#include "ds18b20.h"
#include "my.h"

//----------------------------------------------------------------------------------
// Глобальные переменные для управления поиском устройств на шине 1-Wire.
// 1-Wire использует алгоритм обхода двоичного дерева адресов для идентификации всех 
// датчиков на одной линии без конфликтов.
//----------------------------------------------------------------------------------
uint8_t LastDeviceFlag;           // Указывает, что поиск завершен и больше устройств нет
uint8_t LastDiscrepancy;          // Хранит позицию последнего бита, где был выбор между 0 и 1
uint8_t LastFamilyDiscrepancy;    // Используется для поиска устройств конкретного типа
uint8_t ROM_NO[8];                // Буфер для формирования текущего 64-битного ID

// Внешние переменные, используемые для вывода данных и коррекции значений
extern char buffTFT[];
extern uint8_t noname, fc20H, fc28H, familycode[8][8];
extern int8_t oneWire_amount, correction[MAX_DEVICE];
extern union {uint8_t data[8]; int16_t val[4];} bufferADC;
extern int16_t result[], pvT, pvRH;

//----------------------------------------------------------------------------------
// Реализация точной микросекундной задержки через аппаратный счетчик циклов DWT.
// Важно: на STM32 задержки через пустые циклы 'while(micros--)' могут быть 
// удалены компилятором при включении оптимизации (-Os, -O3), что приведет 
// к полной неработоспособности протоколов 1-Wire и DHT.
//----------------------------------------------------------------------------------
void DelayMicro(uint32_t micros) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = micros * (SystemCoreClock / 1000000);
    // Использование разности без знака корректно обрабатывает переполнение счетчика
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

//----------------------------------------------------------------------------------
// Инициализация порта для DHT-датчиков. Настраивается как Open-Drain выход.
// Это необходимо, так как датчик сам подтягивает линию к "1" через внешний резистор 
// и "просаживает" ее до "0" при ответе.
//----------------------------------------------------------------------------------
void DHT_port_init(void){
  HAL_GPIO_DeInit(DHT_PORT, DHT_PIN);
  DHT_PORT->CRH |= GPIO_CRH_MODE10;          // Output Mode, 50MHz
  DHT_PORT->CRH |= GPIO_CRH_CNF10_0;         // Open-Drain
  DHT_PORT->CRH &= ~GPIO_CRH_CNF10_1;        
}

//----------------------------------------------------------------------------------
// Инициализация порта для 1-Wire. Аналогично Open-Drain.
//----------------------------------------------------------------------------------
void oneWire_port_init(void){
  HAL_GPIO_DeInit(ONEWIRE_PORT, ONEWIRE_PIN);
  ONEWIRE_PORT->CRH |= GPIO_CRH_MODE11;          // Output Mode, 50MHz
  ONEWIRE_PORT->CRH |= GPIO_CRH_CNF11_0;         // Open-Drain
  ONEWIRE_PORT->CRH &= ~GPIO_CRH_CNF11_1;        
}

//----------------------------------------------------------------------------------
// Алгоритм поиска ROM ID устройств. 
// Позволяет получить уникальный 8-байтный номер каждого устройства в сети.
// 1-й байт - Family Code (0x28 для DS18B20, 0x20 для DS2450), 
// следующие 6 байт - серийный номер, 8-й байт - CRC.
//----------------------------------------------------------------------------------
uint8_t oneWire_SearhRom(uint8_t *Addr){
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;

  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;

	if (!LastDeviceFlag){
		oneWire_Reset();
		oneWire_WriteByte(0xF0); // Команда начала поиска
	}

	do {
    // Каждое устройство передает бит адреса, затем инвертированный бит адреса.
    // Если на шине и 0, и 1 - значит есть устройства с разными битами (коллизия).
		id_bit = oneWire_ReadBit();
		cmp_id_bit = oneWire_ReadBit();

		if ((id_bit == 1) && (cmp_id_bit == 1))	break; // Нет ответа от устройств
		else {
			if (id_bit != cmp_id_bit)
				search_direction = id_bit; // Все ответившие имеют одинаковый бит в этой позиции
			else {
        // Есть коллизия. Выбираем направление на основе данных предыдущего прохода.
				if (id_bit_number < LastDiscrepancy)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
				else
					search_direction = (id_bit_number == LastDiscrepancy);
				if (search_direction == 0){
					last_zero = id_bit_number;
					if (last_zero < 9) LastFamilyDiscrepancy = last_zero;
				}
			}
      
      // Подтверждаем мастеру бит адреса (те устройства, чей бит не совпал, замолчат до сброса)
			if (search_direction == 1) ROM_NO[rom_byte_number] |= rom_byte_mask;
			else ROM_NO[rom_byte_number] &= ~rom_byte_mask;
      
			oneWire_WriteBit(search_direction);
			id_bit_number++;
			rom_byte_mask <<= 1;
			if (rom_byte_mask == 0){
				rom_byte_number++;
				rom_byte_mask = 1;
			}
		}
  } while(rom_byte_number < 8);

	if (!(id_bit_number < 65)){
		LastDiscrepancy = last_zero;
		if (LastDiscrepancy == 0)	LastDeviceFlag = 1;
		search_result = 1;	
  }

	if (!search_result || !ROM_NO[0]){
		LastDiscrepancy = 0;
		LastDeviceFlag = 0;
		LastFamilyDiscrepancy = 0;
		search_result = 0;
	}
	else{
    for (int i = 0; i < 8; i++) Addr[i] = ROM_NO[i];
  }	
  return search_result;
}

//----------------------------------------------------------------------------------
// Сброс и Presence Pulse. Шина 1-Wire управляется только мастером.
// Любая транзакция должна начинаться с импульса сброса (минимум 480 мкс).
//----------------------------------------------------------------------------------
uint8_t oneWire_Reset(void){
  uint8_t status;
  ONEWIRE_LOW(); 
  DelayMicro(485);    // Импульс сброса (Reset pulse)
  ONEWIRE_HIGH(); 
  DelayMicro(65);     // Окно для ответа Presence Pulse (60-240 мкс отпускания)
  status = (ONEWIRE_READ() ? 1 : 0); // Датчик притягивает линию к 0 в ответ
  DelayMicro(500);    // Завершение тайм-слота сброса
  return (status ? 1 : 0);
}

//----------------------------------------------------------------------------------
// Чтение бита. Мастер инициирует чтение импульсом 0 (минимум 1 мкс).
// Данные от датчика валидны через 15 мкс после начала импульса.
//----------------------------------------------------------------------------------
uint8_t oneWire_ReadBit(void){
  uint8_t bit = 0;
  ONEWIRE_LOW();
  DelayMicro(2);      // Краткий импульс старта
  ONEWIRE_HIGH();
  DelayMicro(13);     // Пауза до момента замера (сэмплирование в районе 15-й мкс)
  bit = (ONEWIRE_READ() ? 1 : 0);
  DelayMicro(45);     // Дожидаемся конца тайм-слота (60 мкс всего)
  return bit;
}

//----------------------------------------------------------------------------------
// Считывание целого байта. 1-Wire передает данные LSB (младший бит) вперед.
//----------------------------------------------------------------------------------
uint8_t oneWire_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
    data += oneWire_ReadBit() << i;
  return data;
}

//----------------------------------------------------------------------------------
// Запись бита. '1' и '0' различаются длительностью удержания линии в 0.
//----------------------------------------------------------------------------------
void oneWire_WriteBit(uint8_t bit){
  ONEWIRE_LOW();
  DelayMicro(bit ? 3 : 65); // Для 0 линия держится в 0 долго
  ONEWIRE_HIGH();
  DelayMicro(bit ? 65 : 3);
}

//----------------------------------------------------------------------------------
// Запись байта
//----------------------------------------------------------------------------------
void oneWire_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    oneWire_WriteBit(dt >> i & 1);
    DelayMicro(5); // Пауза для восстановления заряда на линии
  }
}

//----------------------------------------------------------------------------------
// Пересчет количества датчиков. Обнуляет счетчики перед поиском.
//----------------------------------------------------------------------------------
void oneWire_count(uint8_t amount){
  uint8_t i, dt[8];
  oneWire_amount = 0;
  fc20H = 0; fc28H = 0; noname = 0;
  for(i = 0; i < amount; i++){
    if(oneWire_SearhRom(dt)){
      memcpy(familycode[oneWire_amount], dt, 8);
      oneWire_amount++;
      switch (dt[0]){
      	case 0x20: fc20H++; break;  // DS2450
      	case 0x28: fc28H++;	break;  // DS18B20
      	default:  noname++; break;
      }
    }
    else break;
  }
}

//----------------------------------------------------------------------------------
// Глобальный запуск преобразования для всех датчиков.
// После этой команды датчики начинают замер (длится до 750 мс при 12-битах).
//----------------------------------------------------------------------------------
void ds18b20_Convert_T(){
  oneWire_Reset();
  oneWire_WriteByte(0xCC);  // SKIP ROM: команда всем датчикам на линии
  oneWire_WriteByte(0x44);  // CONVERT T: команда начать замер
}

//----------------------------------------------------------------------------------
// Чтение данных (Scratchpad) конкретного датчика.
// Используется для получения серийника и отладки данных в памяти датчика.
//----------------------------------------------------------------------------------
void ds18b20_ReadStratcpad(uint16_t y_pos, uint8_t DevNum){
  uint8_t i, crc, dt[8];
  oneWire_Reset();
	oneWire_WriteByte(0x55);  // MATCH ROM: выбор конкретного ID
	for(i = 0; i < 8; i++){ oneWire_WriteByte(familycode[DevNum][i]); }
  oneWire_WriteByte(0xBE);  // READ SCRATCHPAD: чтение оперативной памяти
  for(i=0; i<8; i++){ dt[i] = oneWire_ReadByte(); }
  crc = oneWire_ReadByte(); // Последним идет байт CRC
  
  if (dallas_crc8(dt, 8) == crc){
    sprintf(buffTFT,"PAD %02X %02X %02X %02X %02X %02X %02X %02X",
       dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7]);
    ILI9341_WriteString(5, y_pos, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  }
}

//----------------------------------------------------------------------------------
// Сохранение конфигурации. 
// Записываем пороги в Scratchpad и копируем их в энергонезависимую память (EEPROM) датчика.
//----------------------------------------------------------------------------------
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl){
  uint8_t i;
  oneWire_Reset();
  oneWire_WriteByte(0x55);  // MATCH ROM
  for(i = 0; i < 8; i++){ oneWire_WriteByte(familycode[DevNum][i]); }
  oneWire_WriteByte(0x4E);  // WRITE SCRATCHPAD
  oneWire_WriteByte(th);    // Байт TH (High Trigger). Можно использовать как TUNING метку.
  oneWire_WriteByte(tl);    // Байт TL (Low Trigger). Здесь храним коррекцию.
  oneWire_WriteByte(0x7F);  // 12 бит разрешение (0.0625°C шаг)
  
  // Важно: данные в Scratchpad пропадут при сбросе питания, если не сделать COPY.
  oneWire_Reset();
  oneWire_WriteByte(0xCC);
  oneWire_WriteByte(0x48);  // COPY SCRATCHPAD: перенос из RAM в EEPROM
}

//----------------------------------------------------------------------------------
// Стандартный алгоритм Dallas CRC8 для валидации данных.
// Если данные повреждены помехой на линии, CRC не совпадет.
//----------------------------------------------------------------------------------
uint8_t dallas_crc8(uint8_t * data, uint8_t size){
  uint8_t crc = 0;
  for (uint8_t i = 0; i < size; ++i){
    uint8_t inbyte = data[i];
    for (uint8_t j = 0; j < 8; ++j){
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

//----------------------------------------------------------------------------------
// Проверка знака температуры (формат DS18B20 - старшие 5 бит отвечают за знак).
//----------------------------------------------------------------------------------
uint8_t ds18b20_GetSign(uint16_t dt){
  if (dt & (1 << 11)) return 1; // Отрицательная
  else return 0;                // Положительная
}

//----------------------------------------------------------------------------------
// Перевод сырых данных в градусы Цельсия (float).
// Сырое значение - это число с фиксированной запятой, где 4 младших бита - дробь.
//----------------------------------------------------------------------------------
float ds18b20_Convert(uint16_t dt){
  float t;
  t = (float)((dt & 0x07FF) >> 4); // Выделяем целую часть
  t += (float)(dt & 0x000F) / 16.0f; // Выделяем дробную часть (1/16 = 0.0625)
  return t;
}

//----------------------------------------------------------------------------------
// Основная процедура опроса. Опрашивает датчики последовательно.
//----------------------------------------------------------------------------------
void temperature_check(){
  uint8_t item, byte, crc, try_cnt=0;
  union {uint8_t data[8]; int16_t val[4];} buffer;
  int16_t valT;
  
  for (item=0; item < oneWire_amount;){
    oneWire_Reset();
    oneWire_WriteByte(0x55); // Адресация конкретного устройства
    for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
    oneWire_WriteByte(0xBE); // Читаем Scratchpad
    for (byte=0; byte < 8; byte++){
        buffer.data[byte] = oneWire_ReadByte();
    }
    crc = oneWire_ReadByte();
    
    // Если CRC совпал - данные достоверны
    if (dallas_crc8(buffer.data, 8) == crc){
      try_cnt = 0;
      valT = buffer.val[0]; // Температура (младший и старший байты)
      
      // Расчет: умножаем на 10 для хранения одного знака после запятой в int16_t
      // Делим на 16, так как разрешение датчика - 4 бита на дробь (2^4=16).
      if (valT < 0){
        valT = -valT;
        result[item] = -(valT * 10 / 16); // Исправлено: корректный индекс item
      }
      else result[item] = valT * 10 / 16;
      
      // Логика калибровки: если в EEPROM датчика (байт 2) записан наш TUNING флаг,
      // значит в байте 3 лежит значение индивидуальной коррекции.
      if (buffer.data[2] == TUNING){
          correction[item] = buffer.data[3];
          result[item] += correction[item];
      }
      else correction[item] = 0;
    }
    else if (++try_cnt > 2) {
        // Если 3 раза подряд ошибка CRC - считаем датчик отключенным/неисправным
        try_cnt = 0;
        result[item] = 1990; // Специальный код для отображения ошибки
    }
    
    if (try_cnt == 0) item++; // Переход к следующему датчику только при успехе или лимите попыток
  }
  
  // После сбора данных запускаем замер на следующий цикл
  ds18b20_Convert_T();
}

//------- DHT-21 / DHT-11 ----------------------------------------------------------
// Датчики DHT используют нестандартный протокол с длительными старт-импульсами.
//----------------------------------------------------------------------------------
uint8_t startDHT(uint8_t cn){
  uint8_t status;
  int16_t flag=0;
  if(cn) DHT_LOW();
  else ONEWIRE_LOW();
  
  HAL_Delay(5); // Удерживаем линию в 0 минимум 1-18мс для пробуждения датчика
  
  if(cn) DHT_HIGH();
  else ONEWIRE_HIGH();
  
  DelayMicro(60); // Датчик должен ответить низким уровнем
  status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
  
  if(!status){
      // Ожидаем ответный импульс датчика (80мкс в 0, затем 80мкс в 1)
      while(!status){
        flag++;
        status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
        if(flag > 1000) return 0; // Timeout
      }
      if(flag < 10) return 0;
      
      flag = 0;
      while(status){
        flag++;
        status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
        if(flag > 1000) return 0;
      }
      if(flag < 10) return 0;
      else return 1;
  }
  return 0;
}

uint8_t readDHT(uint8_t cn){
 uint8_t i, j, flag=0, status, tem[5];
 static uint8_t err;
 if(startDHT(cn)){
    for(i=0; i<5; i++){
       tem[i]=0;
       for(j=0; j<8; j++){
          tem[i] <<= 1;
          status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
          while(!status){
            status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
          }
          DelayMicro(32); // Если через 32мкс линия всё еще 1 - значит передан бит '1'
          flag = 0;
          status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
          while(status){
            flag++;
            status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
            if(flag > 1000) break;
          }
          if(flag > 10) tem[i] |= 1;
       }
    }
    // Контрольная сумма DHT: сумма первых 4-х байт должна быть равна 5-му
    flag = tem[0] + tem[1] + tem[2] + tem[3];
    if(flag == tem[4]){
      pvRH = (int)tem[0] * 256 + tem[1]; // Целое + дробное (для DHT21/22)
      pvT = (int)tem[2] * 256 + tem[3];  // Температура
      err = 0;
      if(pvRH > 1000) pvRH = 1000; else if (pvRH < 0) pvRH = 0;
      return 1;
    }
    else if(++err > 3) return 0;
 }
 return 0;
}

//------- DS2450 (4-канальный АЦП на шине 1-Wire) ----------------------------------
// Позволяет измерять напряжения удаленно по той же линии, что и термометры.
//----------------------------------------------------------------------------------
void DS2450_check(){
  uint8_t item, byte, dev=0;
  union {uint8_t data[8]; uint16_t val[4];} bufferADC;
  for (item=0; item < oneWire_amount; item++){
    if (familycode[item][0] == 0x20){
      oneWire_Reset();
      oneWire_WriteByte(0x55); 
      for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
      oneWire_WriteByte(0xAA); // READ MEMORY: читаем результаты замера
      oneWire_WriteByte(0x00); // Начальный адрес TA1
      oneWire_WriteByte(0x00); // TA2
      for (byte=0; byte < 8; byte++){
        bufferADC.data[byte] = oneWire_ReadByte();
      }
      oneWire_ReadByte(); // Пропускаем CRC16
      oneWire_ReadByte();
      for (byte=0; byte < 4; byte++){
        result[byte + dev * 4] = (bufferADC.val[byte] >> 8); // Сохраняем 8-битный результат
      }
      dev++;
    }
  }
  // Запуск преобразования для следующего цикла опроса
  oneWire_Reset();
  oneWire_WriteByte(0xCC); 
  oneWire_WriteByte(0x3C); // CONVERT
  oneWire_WriteByte(0x0F); // Маска каналов (A, B, C, D)
  oneWire_WriteByte(0x55); // Output control
  oneWire_ReadByte(); // Пропуск CRC16 ответа на команду
  oneWire_ReadByte();
}

//----------------------------------------------------------------------------------
// Инициализация DS2450: настройка разрядности и диапазона входного напряжения.
//----------------------------------------------------------------------------------
uint8_t DS2450_reset(){
  uint8_t j, item, byte, ok, setup=0;
  for (item=0; item < oneWire_amount; item++){
     if (familycode[item][0] == 0x20){
        ok=0;
        oneWire_Reset();
        oneWire_WriteByte(0x55);
        for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
        oneWire_WriteByte(0x55); // WRITE MEMORY
        oneWire_WriteByte(0x1C); // Адрес регистра VCC/питания
        oneWire_WriteByte(0x00);
        oneWire_WriteByte(0x40); // Активируем аналоговую часть
        oneWire_ReadByte(); // CRC16
        oneWire_ReadByte();
        if (oneWire_ReadByte() == 0x40) ok++; // Проверка записи
        
        oneWire_Reset();
        oneWire_WriteByte(0x55);
        for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
        oneWire_WriteByte(0x55);
        oneWire_WriteByte(0x08); // Адрес конфигурации каналов
        oneWire_WriteByte(0x00);
        for (j=0; j < 4; j++){
           oneWire_WriteByte(0x08); // 8-бит разрешение
           oneWire_ReadByte(); // CRC16
           oneWire_ReadByte();
           if (oneWire_ReadByte() == 0x08) ok++;
           
           oneWire_WriteByte(0x00); // IR=0: диапазон 2.56V (для 5.12V нужно 0x01)
           oneWire_ReadByte(); // CRC16
           oneWire_ReadByte();
           if (oneWire_ReadByte() == 0x00) ok++;
        }
        if (ok == 9) setup++; // Если все 9 байт конфигурации записаны верно
     }
  }
  return setup;
}
