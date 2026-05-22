#include "tft_proc.h"
#include "ili9341.h"
#include "ds18b20.h"
#include "displ_as.h"
#include "my.h"

// Внешние переменные для отрисовки данных
extern char buffTFT[];
extern uint8_t displ_num, oneWire_amount, fc28H, ds18b20_num, familycode[][8], newButt, Y_txt, X_left, Y_top, Y_bottom;
extern int16_t result[], fillScreen;
extern int8_t newcorrection, correction[MAX_DEVICE];

// Вспомогательные функции для математических операций
int16_t min(int16_t a, int16_t b ) { return a < b ? a : b; }
int16_t max(int16_t a, int16_t b ) { return a > b ? a : b; }

//----------------------------------------------------------------------------------
// Отрисовка списка температур мелким шрифтом (11x18).
// Используется, если датчиков больше 8, чтобы все уместились на экране.
//----------------------------------------------------------------------------------
void displT_11x18(){
 uint8_t item, amnt=0;
 uint16_t color_txt;
 int16_t max_t=-550, min_t=1270, midl_t=0;
 
  for (item = 0; item < oneWire_amount; item++){
    // Статистика (Макс, Мин, Среднее)
    if (result[item]<1270) max_t = max(max_t,result[item]);
    if (result[item]>-550) min_t = min(min_t,result[item]);
    if (result[item]<1270) {midl_t = midl_t+result[item]; amnt++;}
    
    // Форматирование вывода (один знак после запятой через целочисленное деление)
    if (result[item]<1000) sprintf(buffTFT,"t%02d=%.1f  ",item+1 ,(float)result[item]/10);
    else if (result[item]<1270) sprintf(buffTFT,"t%02d=%d  ",item+1 , result[item]/10);
    else sprintf(buffTFT,"t%02d=***  ",item+1); // Ошибка датчика
    
    // Выделение выбранного датчика зеленым цветом
    if (item == ds18b20_num) color_txt = ILI9341_GREEN; else color_txt = ILI9341_WHITE;
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, color_txt, ILI9341_BLACK);
    
    // Переход на новую строку (сетка 3 столбца)
    if (item==2||item==5||item==8||item==11||item==14||item==17||item==20){
      Y_txt = Y_txt+18+4;
      X_left = 5;
    }
    else X_left = X_left + 105;
  }
  
  if(min_t==1270) fc28H=0; // Если ни одного замера не успешно
  if(fc28H){
    // Вывод итоговой статистики внизу экрана
    sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
    ILI9341_WriteString(5, Y_bottom-3, buffTFT, Font_11x18, ILI9341_YELLOW, ILI9341_BLACK);
  }
}

//----------------------------------------------------------------------------------
// Отрисовка списка температур крупным шрифтом (16x26).
// Используется для комфортного чтения, если датчиков мало (до 8).
//----------------------------------------------------------------------------------
void displT_16x26(){
 uint8_t item, amnt=0;
 uint16_t color_txt;
 int16_t max_t=-550, min_t=1270, midl_t=0;
 
  for (item = 0; item < oneWire_amount; item++){
    if (result[item]<1270) max_t = max(max_t,result[item]);
    if (result[item]>-550) min_t = min(min_t,result[item]);
    if (result[item]<1270) {midl_t = midl_t+result[item]; amnt++;}
    
    if (result[item]<1000) sprintf(buffTFT,"t%02d=%.1f  ",item+1 ,(float)result[item]/10);
    else if (result[item]<1270) sprintf(buffTFT,"t%d=%d  ",item+1 , result[item]/10);
    else sprintf(buffTFT,"t%d=***  ",item+1);
    
    if (item == ds18b20_num) color_txt = ILI9341_GREEN; else color_txt = ILI9341_WHITE;
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_16x26, color_txt, ILI9341_BLACK);
    
    // Сетка 2 столбца
    if (item==1||item==3||item==5){
      Y_txt = Y_txt+26+8;
      X_left = 5;
    } 
    else X_left = X_left + 160;
  }
  
  if(min_t==1270) fc28H=0;
  if(fc28H){
    sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
    ILI9341_WriteString(5, Y_bottom-3, buffTFT, Font_11x18, ILI9341_YELLOW, ILI9341_BLACK);
  }
}

// Экран 0: Обзор датчиков
void displ_as_0(void){ 
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,25); // Рисуем 4 кнопки управления
    drawButton(ILI9341_BLUE, 0, "Датчик");
    drawButton(ILI9341_GREEN, 1, "<");
    drawButton(ILI9341_GREEN, 2, ">");
    drawButton(ILI9341_MAGENTA, 3, "Калибровка");
  }
  // Выбор шрифта в зависимости от плотности данных
  if (oneWire_amount > 8) displT_11x18(); else displT_16x26();
}

// Экран 1: Подробно о датчике
void displ_as_1(void){
 uint8_t i = ds18b20_num;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,25);
    drawButton(ILI9341_BLUE, 0, "Список");
    drawButton(ILI9341_GREEN, 1, "<");
    drawButton(ILI9341_GREEN, 2, ">");
    drawButton(ILI9341_MAGENTA, 3, "Коррекция");
  }
  // Вывод текущей температуры и серийного номера (ROM ID) датчика
  sprintf(buffTFT,"Датчик N%d t=%.1f  ", i+1,(float)result[i]/10);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"   Коррекция= %.1f  ",(float)correction[i]/10);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
  Y_txt = Y_txt+36+10;
  
  // Отображение ID в HEX и DEC форматах (для удобства пользователя)
  sprintf(buffTFT,"hex %02X %02X %02X %02X %02X %02X %02X %02X",
    familycode[i][0], familycode[i][1], familycode[i][2], familycode[i][3],
    familycode[i][4], familycode[i][5], familycode[i][6], familycode[i][7]);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  ds18b20_ReadStratcpad(Y_txt, i); // Чтение содержимого Scratchpad
}

// Функции-заглушки для выбора активного экрана отрисовки
void display_as(void){
  switch (displ_num){
  	case 0: displ_as_0(); break;
  	case 1: displ_as_1(); break;
    // ... остальные кейсы
  	default: displ_as_0();	break;
  }
}
