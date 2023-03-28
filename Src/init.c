
#include "ds18b20.h"
#include "my.h"

extern char buffTFT[];
extern uint8_t noname, oneWire_amount, Y_txt, fc20H, fc28H;
extern int16_t pvT, pvRH;
extern ADC_HandleTypeDef hadc1;
//extern int16_t result[];
//------------------------------------- НАЧАЛЬНЫЙ экран ---------------- 
void home_screen(void){
  int8_t item, amount=0;
  uint16_t adcVal;
  
  Y_txt = Y_txt+18+5;
  oneWire_port_init();
  item = oneWire_count(MAX_DEVICE);       // проверяем наличие датчиков если item = 0 датчики найдены
//  oneWire_amount = 24;
  if(oneWire_amount){
    amount++;
    sprintf(buffTFT,"Количество 1-Wire: %d шт.",oneWire_amount);
    ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;
//    for (item=0; item<oneWire_amount; item++){
//      sprintf(buffTFT,"%2x %2x %2x %2x %2x %2x %2x %2x",familycode[item][0],familycode[item][1],familycode[item][2],familycode[item][3],familycode[item][4],familycode[item][5],familycode[item][6],familycode[item][7]);
//      ILI9341_WriteString(5, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
//      Y_txt = Y_txt+10+5;
//    }
//    HAL_Delay(5000);
    if(fc28H){
      sprintf(buffTFT,"Количество ds18b20: %d шт.",fc28H);
      ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
      ds18b20_Convert_T();        // первое измерение температуры
    }
    if(fc20H){
      item = DS2450_reset();
      adcVal = ILI9341_GREEN;
      sprintf(buffTFT,"Количество ds2450: %d / %d шт.",fc20H,item);
      if(fc20H-item>0) adcVal = ILI9341_RED; 
      ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, adcVal, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
    }
    if(noname){
      sprintf(buffTFT,"Количество NONAME: %d шт.",noname);
      ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_CYAN, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
    }
  }
  else {
    ILI9341_WriteString(5, Y_txt, "Датчики 1-Wire не найдены!", Font_11x18, ILI9341_RED, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;
  }
//---------------------------- линия 1-Wire ----------------------------------  
  if(oneWire_amount==0){
    HAL_Delay(500);
    item = startDHT(0);
    if(item){
      amount++;  
      ILI9341_WriteString(5, Y_txt, "DHT-21 на линии 1-Wire.", Font_11x18, ILI9341_CYAN, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
    }
    else {
      ILI9341_WriteString(5, Y_txt, "DHT-21 не подключен к 1-Wire", Font_11x18, ILI9341_RED, ILI9341_BLACK);
      Y_txt = Y_txt+18+5;
    }
  }
//---------------------------- линия AM2301 ----------------------------------- 
  DHT_port_init();
  item = startDHT(1);
  if(item){
    amount++;  
    ILI9341_WriteString(5, Y_txt, "DHT-21 на линии AM2301.", Font_11x18, ILI9341_CYAN, ILI9341_BLACK);
//    sprintf(buffTFT,"DHT-21 f0=%d; f1=%d",result[0], result[1]);
//    ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;
  }
  else {
    ILI9341_WriteString(5, Y_txt, "DHT-21 не подключен к AM2301", Font_11x18, ILI9341_RED, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;
  }
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1,100);
  adcVal = ((float)HAL_ADC_GetValue(&hadc1));
  HAL_ADC_Stop(&hadc1);              
  if (adcVal > 10){
    amount++;  
    ILI9341_WriteString(5, Y_txt, "Подключен HIH-5030.", Font_11x18, ILI9341_CYAN, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;
  }
  else {
    ILI9341_WriteString(5, Y_txt, "HIH-5030 не подключен!", Font_11x18, ILI9341_RED, ILI9341_BLACK);
    Y_txt = Y_txt+18+5;
  }
  HAL_Delay(5000);
  if(amount==0) while (1){}
}
