#include "ds18b20.h"
#include "my.h"

//--------------------------------------------------
uint8_t LastDeviceFlag;
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t ROM_NO[8];
extern char buffTFT[];
extern uint8_t noname, fc20H, fc28H, familycode[8][8];
extern int8_t oneWire_amount, correction[MAX_DEVICE];
extern union {uint8_t data[8]; int16_t val[4];} bufferADC;
extern int16_t result[], pvT, pvRH;

//--------------------------------------------------
void DelayMicro(uint32_t micros) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = micros * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

//--------------------------------------------------
void DHT_port_init(void){
  HAL_GPIO_DeInit(DHT_PORT, DHT_PIN);
  DHT_PORT->CRH |= GPIO_CRH_MODE10;
  DHT_PORT->CRH |= GPIO_CRH_CNF10_0;
  DHT_PORT->CRH &= ~GPIO_CRH_CNF10_1;
}

//--------------------------------------------------
void oneWire_port_init(void){
  HAL_GPIO_DeInit(ONEWIRE_PORT, ONEWIRE_PIN);
  ONEWIRE_PORT->CRH |= GPIO_CRH_MODE11;
  ONEWIRE_PORT->CRH |= GPIO_CRH_CNF11_0;
  ONEWIRE_PORT->CRH &= ~GPIO_CRH_CNF11_1;
}

//-----------------------------------------------
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
		oneWire_WriteByte(0xF0);
	}
	do{
		id_bit = oneWire_ReadBit();
		cmp_id_bit = oneWire_ReadBit();
		if ((id_bit == 1) && (cmp_id_bit == 1))	break;
		else{
			if (id_bit != cmp_id_bit)
				search_direction = id_bit; // bit write value for search
			else{
				if (id_bit_number < LastDiscrepancy)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
				else
					search_direction = (id_bit_number == LastDiscrepancy);
				if (search_direction == 0){
					last_zero = id_bit_number;
					if (last_zero < 9) LastFamilyDiscrepancy = last_zero;
				}
			}
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

//-----------------------------------------------
uint8_t oneWire_Reset(void){
  uint8_t status;
  ONEWIRE_LOW();
  DelayMicro(485);
  ONEWIRE_HIGH();
  DelayMicro(65);
  status = (ONEWIRE_READ() ? 1 : 0);
  DelayMicro(500);
  return (status ? 1 : 0);
}

//--------------------------------------------------
uint8_t oneWire_ReadBit(void){
  uint8_t bit = 0;
  ONEWIRE_LOW();
  DelayMicro(2);
  ONEWIRE_HIGH();
  DelayMicro(13);
  bit = (ONEWIRE_READ() ? 1 : 0);
  DelayMicro(45);
  return bit;
}

//-----------------------------------------------
uint8_t oneWire_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
    data += oneWire_ReadBit() << i;
  return data;
}

//-----------------------------------------------
void oneWire_WriteBit(uint8_t bit){
  ONEWIRE_LOW();
  DelayMicro(bit ? 3 : 65);
  ONEWIRE_HIGH();
  DelayMicro(bit ? 65 : 3);
}

//-----------------------------------------------
void oneWire_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    oneWire_WriteBit(dt >> i & 1);
    DelayMicro(5);
  }
}

//-----------------------------------------------
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

//-----------------------------------------------
void ds18b20_Convert_T(){
  oneWire_Reset();
  oneWire_WriteByte(0xCC);  // SKIP ROM
  oneWire_WriteByte(0x44);  // CONVERT T
}

//----------------------------------------------------------
void ds18b20_ReadStratcpad(uint16_t y_pos, uint8_t DevNum){
  uint8_t i, crc, dt[8];
  oneWire_Reset();
	oneWire_WriteByte(0x55);  // MATCH ROM
	for(i = 0; i < 8; i++){ oneWire_WriteByte(familycode[DevNum][i]); }
  oneWire_WriteByte(0xBE);  // READ SCRATCHPAD
  for(i=0; i<8; i++){ dt[i] = oneWire_ReadByte(); }
  crc = oneWire_ReadByte();
  if (dallas_crc8(dt, 8) == crc){
    sprintf(buffTFT,"PAD %02X %02X %02X %02X %02X %02X %02X %02X",
       dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7]);
    ILI9341_WriteString(5, y_pos, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  }
}

//----------------------------------------------------------
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl){
  uint8_t i;
  oneWire_Reset();
  oneWire_WriteByte(0x55);  // MATCH ROM
  for(i = 0; i < 8; i++){ oneWire_WriteByte(familycode[DevNum][i]); }
  oneWire_WriteByte(0x4E);  // WRITE SCRATCHPAD
  oneWire_WriteByte(th);
  oneWire_WriteByte(tl);
  oneWire_WriteByte(0x7F);  // configuration register
  oneWire_Reset();
  oneWire_WriteByte(0xCC);  // SKIP ROM
  oneWire_WriteByte(0x48);  // COPY SCRATCHPAD
}

//----------------------------------------------------------
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

//----------------------------------------------------------
uint16_t CRC16(uint8_t *pcBlock, uint16_t len){
  uint16_t crc = 0xFFFF;
  uint8_t i;
  while (len--){
    crc ^= *pcBlock++ << 8;
    for (i = 0; i < 8; i++)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}

//----------------------------------------------------------
uint8_t ds18b20_GetSign(uint16_t dt){
  if (dt & (1 << 11)) return 1;
  else return 0;
}

//----------------------------------------------------------
float ds18b20_Convert(uint16_t dt){
  float t;
  t = (float)((dt & 0x07FF) >> 4);
  t += (float)(dt & 0x000F) / 16.0f;
  return t;
}

//----------------------------------------------------------
void temperature_check(){
  uint8_t item, byte, crc, try_cnt=0;
  union {uint8_t data[8]; int16_t val[4];} buffer;
  int16_t valT;
  for (item=0; item < oneWire_amount;){
    oneWire_Reset();
    oneWire_WriteByte(0x55); // MATCH ROM
    for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
    oneWire_WriteByte(0xBE); // READ SCRATCHPAD
    for (byte=0; byte < 8; byte++){
        buffer.data[byte] = oneWire_ReadByte();
    }
    crc = oneWire_ReadByte();
    if (dallas_crc8(buffer.data, 8) == crc){
      try_cnt = 0;
      valT = buffer.val[0];
      if (valT < 0){
        valT = -valT;
        result[item] = -(valT * 10 / 16);
      }
      else result[item] = valT * 10 / 16;
      
      if (buffer.data[2] == TUNING){
          correction[item] = buffer.data[3];
          result[item] += correction[item];
      }
      else correction[item] = 0;
    }
    else if (++try_cnt > 2) {
        try_cnt = 0;
        result[item] = 1990;
    }
    if (try_cnt == 0) item++;
  }
  ds18b20_Convert_T();
}

//------- DHT-21 / DHT-11 ------------------------------------------
uint8_t startDHT(uint8_t cn){
  uint8_t status;
  int16_t flag=0;
  if(cn) DHT_LOW();
  else ONEWIRE_LOW();
  
  HAL_Delay(5);
  
  if(cn) DHT_HIGH();
  else ONEWIRE_HIGH();
  
  DelayMicro(60);
  status = (cn ? DHT_READ() : ONEWIRE_READ()) ? 1 : 0;
  
  if(!status){
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
        if(flag > 1000) return 0; // Timeout
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
          DelayMicro(32);
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
    flag = tem[0] + tem[1] + tem[2] + tem[3];
    if(flag == tem[4]){
      pvRH = (int)tem[0] * 256 + tem[1];
      pvT = (int)tem[2] * 256 + tem[3];
      err = 0;
      if(pvRH > 1000) pvRH = 1000; else if (pvRH < 0) pvRH = 0;
      return 1;
    }
    else if(++err > 3) return 0;
 }
 return 0;
}

//------- DS2450 ------------------------------------------
void DS2450_check(){
  uint8_t item, byte, dev=0;
  union {uint8_t data[8]; uint16_t val[4];} bufferADC;
  for (item=0; item < oneWire_amount; item++){
    if (familycode[item][0] == 0x20){
      oneWire_Reset();
      oneWire_WriteByte(0x55); // MATCH ROM
      for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
      oneWire_WriteByte(0xAA); // Read Memory
      oneWire_WriteByte(0x00); // TA1
      oneWire_WriteByte(0x00); // TA2
      for (byte=0; byte < 8; byte++){
        bufferADC.data[byte] = oneWire_ReadByte();
      }
      oneWire_ReadByte(); // CRC16 Low
      oneWire_ReadByte(); // CRC16 High
      for (byte=0; byte < 4; byte++){
        result[byte + dev * 4] = (bufferADC.val[byte] >> 8);
      }
      dev++;
    }
  }
  oneWire_Reset();
  oneWire_WriteByte(0xCC); // SKIP ROM
  oneWire_WriteByte(0x3C); // Convert ADC
  oneWire_WriteByte(0x0F); // CH-D,C,B,A
  oneWire_WriteByte(0x55); // control byte
  oneWire_ReadByte(); // CRC16 Low
  oneWire_ReadByte(); // CRC16 High
}

uint8_t DS2450_reset(){
  uint8_t j, item, byte, ok, setup=0;
  for (item=0; item < oneWire_amount; item++){
     if (familycode[item][0] == 0x20){
        ok=0;
        oneWire_Reset();
        oneWire_WriteByte(0x55);
        for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
        oneWire_WriteByte(0x55); // Write Memory
        oneWire_WriteByte(0x1C);
        oneWire_WriteByte(0x00);
        oneWire_WriteByte(0x40);
        oneWire_ReadByte(); // CRC16 L
        oneWire_ReadByte(); // CRC16 H
        if (oneWire_ReadByte() == 0x40) ok++;
        
        oneWire_Reset();
        oneWire_WriteByte(0x55);
        for (byte=0; byte < 8; byte++) oneWire_WriteByte(familycode[item][byte]);
        oneWire_WriteByte(0x55); // Write Memory
        oneWire_WriteByte(0x08);
        oneWire_WriteByte(0x00);
        for (j=0; j < 4; j++){
           oneWire_WriteByte(0x08); // 8-bit resolution
           oneWire_ReadByte(); // CRC16 L
           oneWire_ReadByte(); // CRC16 H
           if (oneWire_ReadByte() == 0x08) ok++;
           
           oneWire_WriteByte(0x00); // 2.56V range
           oneWire_ReadByte(); // CRC16 L
           oneWire_ReadByte(); // CRC16 H
           if (oneWire_ReadByte() == 0x00) ok++;
        }
        if (ok == 9) setup++;
     }
  }
  return setup;
}
