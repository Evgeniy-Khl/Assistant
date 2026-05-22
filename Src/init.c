#include "main.h"
#include "ds18b20.h"
#include "my.h"

extern char buffTFT[];
extern uint8_t noname, oneWire_amount, Y_txt, fc20H, fc28H;
extern int16_t pvT, pvRH;
extern ADC_HandleTypeDef hadc1;

//----------------------------------------------------------------------------------
// Приветственный экран и первичная диагностика датчиков при старте.
// Здесь происходит сканирование шины 1-Wire и определение типов устройств.
//----------------------------------------------------------------------------------
void home_screen(void){
  int8_t item;
  uint16_t adcVal;

  Y_txt = Y_txt+18+5;
  oneWire_port_init(); // Перевод пина в режим работы с 1-Wire
  
  // 1. Поиск всех устройств на шине
  oneWire_count(MAX_DEVICE); 
  
  if(oneWire_amount){
    // Отчет о найденных устройствах 1-Wire
    sprintf(buffTFT,"Найдено 1-Wire: %d шт.",oneWire_amount);
    ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;

    if(fc28H){
      sprintf(buffTFT,"Термометры DS18B20: %d шт.",fc28H);
      ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
      ds18b20_Convert_T(); // Запуск первого преобразования (результат будет через 750мс)
    }
    
    if(fc20H){
      // Сброс и настройка АЦП DS2450
      item = DS2450_reset();
      uint16_t color = (fc20H == item) ? ILI9341_GREEN : ILI9341_RED;
      sprintf(buffTFT,"АЦП DS2450: %d / %d OK", fc20H, item);
      ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, color, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
    }
  }
  else {
    ILI9341_WriteString(5, Y_txt, "Шина 1-Wire пуста!", Font_11x18, ILI9341_RED, ILI9341_BLACK);       
    Y_txt = Y_txt+18+5;
  }

  // 2. Попытка инициализации датчика DHT, если шина 1-Wire пуста (совмещенный режим)
  if(oneWire_amount==0){
    HAL_Delay(500);
    if(startDHT(0)){
      ILI9341_WriteString(5, Y_txt, "Найден DHT на 1-Wire!", Font_11x18, ILI9341_CYAN, ILI9341_BLACK);       
      Y_txt = Y_txt+18+5;
    }
  }

  // 3. Проверка аналогового датчика влажности HIH-5030 через внутренний АЦП STM32
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK){
    adcVal = HAL_ADC_GetValue(&hadc1);
    if (adcVal > 500){ // Датчик подключен, если напряжение выше порога
      ILI9341_WriteString(5, Y_txt, "Датчик HIH-5030 ОК", Font_11x18, ILI9341_GREEN, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
    }
  }
  HAL_ADC_Stop(&hadc1);

  HAL_Delay(3000); // Даем время пользователю прочитать лог загрузки
}
