#include "my.h"
#include "buttons.h"

// Внешние переменные для управления состоянием интерфейса
extern uint8_t displ_num, newButt, Y_txt, X_left, Y_top, Y_bottom, buttonAmount;
extern int8_t oneWire_amount, ds18b20_num, ds2450_num, numSet, numDate, newDate, tiimeDispl, newcorrection, correction[MAX_DEVICE];
extern uint8_t period;
extern uint16_t fillScreen, prescale;
extern int16_t result[], max_t, min_t, midl_t, val_t;

//----------------------------------------------------------------------------------
// Обработка нажатий кнопок для экранов DS18B20 (Основной функционал)
// Используется логика конечного автомата, где displ_num - текущее состояние (экран)
//----------------------------------------------------------------------------------
void checkButtons_as(uint8_t item){
    switch (displ_num){
    	case 0:// Экран 0: Общий список температур всех датчиков
        switch (item){
        	case 0: displ_num = 1; newButt = 1; break; // Перейти в детальный просмотр
          case 1: if (--ds18b20_num<0) ds18b20_num = 0;	break; // Выбрать предыдущий датчик
      	  case 2: if (++ds18b20_num>oneWire_amount-1) ds18b20_num = oneWire_amount-1; break; // Следующий
          case 3: displ_num = 3; newButt = 1; break; // Перейти в меню калибровки
        }
        item = 10; // Сброс индекса кнопки после обработки
    	break;
    	case 1:// Экран 1: Детальная информация об одном датчике (ROM ID, Scratchpad)
        switch (item){
        	case 0: displ_num = 0; newButt = 1; break; // Вернуться к списку
          case 1: if (--ds18b20_num<0) ds18b20_num = oneWire_amount-1;	break; // Циклическая прокрутка
        	case 2: if (++ds18b20_num>oneWire_amount-1) ds18b20_num = 0;	break;
          case 3: displ_num = 2; newButt = 1; newcorrection = correction[ds18b20_num]; break; // Ввод коррекции
        }
        item = 10;
    	break;
      case 2:// Экран 2: Настройка индивидуальной коррекции датчика
        switch (item){
      	  case 0: displ_num = 1; newButt = 1; break;
          case 1: if (--newcorrection<-12) newcorrection =-12;	break; // Уменьшить на 0.1°C
        	case 2: if (++newcorrection >12) newcorrection = 12;	break; // Увеличить на 0.1°C
          case 3: displ_num = 5; newButt = 1; break; // Перейти к подтверждению сохранения
        }
        item = 10;
     	break;
      case 3:// Экран 3: Выбор метода групповой калибровки (по среднему, макс, мин)
        switch (item){
        	case 0: displ_num = 0; newButt = 1; break;
          case 1: if (++newcorrection>4) newcorrection = 0;	break; 
        	case 2: if (--newcorrection<0) newcorrection = 4;	break; 
          case 3: 
            newButt = 1;
            if (newcorrection == 5) displ_num = 0; else displ_num = 4;
          break;
        }
        item = 10;
    	break;
      case 4:// Экран 4: Подтверждение записи калибровки во все датчики сразу
        switch (item){
        	case 0: displ_num = 3; newButt = 1; break;
          case 1:
            // Процесс записи калибровки в EEPROM датчиков
            ILI9341_FillRectangle(0, Y_top, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
            ILI9341_WriteString(45, 120, "Запись данных!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
            switch (newcorrection){
            	case 0: val_t = result[ds18b20_num];	break;
            	case 1: val_t = max_t; break;
              case 2: val_t = min_t; break;
              case 3: val_t = midl_t; break;
              case 4: val_t = -100; break; // Сброс всех коррекций
            }
            if (val_t == -100){
              // Обнуляем Scratchpad (байты TH/TL)
              for (item=0; item<oneWire_amount; item++) ds18b20_WriteScratchpad(item,0,0);
            }
            else{
              // Рассчитываем разницу и пишем её в каждый датчик
              for (item=0; item<oneWire_amount; item++){
                newcorrection =(int8_t)(val_t-result[item]);
                ds18b20_WriteScratchpad(item, TUNING, newcorrection);
              }
            }
            HAL_Delay(1000);
            displ_num = 0; newButt = 1; item = 10;
            break;
        }
      break;
      case 5:// Экран 5: Подтверждение записи коррекции для одного датчика
        if (item){
          ILI9341_FillRectangle(0, Y_top, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
          ILI9341_WriteString(45, 120, "Запись данных!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
          // Сохраняем метку TUNING и значение коррекции в EEPROM датчика
          ds18b20_WriteScratchpad(ds18b20_num, TUNING, newcorrection);
          HAL_Delay(1000);
        }
        displ_num = 1; newButt = 1; item = 10;
     	break;
      default: displ_num = 0; newButt = 1; break;
    }
}

//----------------------------------------------------------------------------------
// Обработка нажатий кнопок для экранов АЦП (DS2450)
//----------------------------------------------------------------------------------
void checkButtons_ADC(uint8_t item){
    switch (displ_num){
    	case 0:
        switch (item){
        	case 0: ds2450_num = 0; newButt = 1; break; // Выбрать 1-й модуль АЦП
          case 1: ds2450_num = 1; newButt = 1; break; // Выбрать 2-й модуль
      	  case 2: ds2450_num = 2; newButt = 1; break; // Выбрать 3-й модуль
          case 3: displ_num = 2;  newButt = 1; break; // Настройка периода опроса
        }
        item = 10;
    	break;
      case 1:
        switch (item){
        	case 0: ds2450_num = 0; newButt = 1; break;
          case 1: ds2450_num = 1; newButt = 1; break;
      	  case 2: ds2450_num = 2; newButt = 1; break;
          case 3: displ_num = 2;  newButt = 1; break;
        }
        item = 10;
    	break;
      case 2:// Настройка частоты опроса АЦП через изменение прескалера таймера
        newButt = 1;
        if(item<8){period=item; displ_num = 1;}
        else {displ_num = 0; prescale=1999; period=6;}
        switch (item){
      	  case 0: prescale=59999; break;  // 6.0 сек
          case 1: prescale=39999; break;  // 4.0 сек
        	case 2: prescale=19999; break;  // 2.0 сек
          case 3: prescale=9999;  break;  // 1.0 сек
          case 4: prescale=4999;  break;  // 0.8 сек
          case 5: prescale=3999;  break;  // 0.4 сек
        	case 6: prescale=1999;  break;  // 0.2 сек
          case 7: prescale= 999;  break;  // 0.1 сек
        }
        item = 10;
     	break;  
    }
}
