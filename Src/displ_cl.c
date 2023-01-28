#include "tft_proc.h"
#include "ili9341.h"
#include "ds18b20.h"
#include "displ_cl.h"
#include "my.h"

extern char buffTFT[];
extern uint8_t displ_num, ds18b20_amount, ds18b20_num, familycode[][8], newButt, Y_txt, X_left, Y_top, Y_bottom, card, newDate, ticTimer, assistent;
extern int16_t ds18b20_val[], fillScreen/*, set[], newval[]*/;
extern int8_t numSet, numDate;
extern int32_t UnixTime;
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

int16_t min(int16_t a, int16_t b ) {
   return a < b ? a : b;
}

int16_t max(int16_t a, int16_t b ) {
   return a > b ? a : b;
}

void displT_11x18(){
 uint8_t item, amnt=0;
 uint16_t color_txt;
 int16_t max_t=-550, min_t=1270, midl_t=0;
  for (item = 0; item < ds18b20_amount; item++){
    if (ds18b20_val[item]<1270) max_t = max(max_t,ds18b20_val[item]);
    if (ds18b20_val[item]>-550) min_t = min(min_t,ds18b20_val[item]);
    if (ds18b20_val[item]<1270) {midl_t = midl_t+ds18b20_val[item]; amnt++;}
    if (ds18b20_val[item]<1000) sprintf(buffTFT,"t%02d=%.1f  ",item+1 ,(float)ds18b20_val[item]/10);
    else if (ds18b20_val[item]<1270) sprintf(buffTFT,"t%02d=%d  ",item+1 , ds18b20_val[item]/10);
    else sprintf(buffTFT,"t%02d=***  ",item+1);
    if (assistent){
        if (item == ds18b20_num) color_txt = ILI9341_GREEN; else color_txt = ILI9341_WHITE;
    }
    else {
//        if (ds18b20_val[item]>=set[0]) {color_txt = ILI9341_MAGENTA; ticTimer+=5;}
//        else if (ds18b20_val[item]<=set[1]) {color_txt = ILI9341_CYAN; ticTimer+=5;}
//        else color_txt = ILI9341_WHITE;
    }
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, color_txt, ILI9341_BLACK);
    if (item==2||item==5||item==8||item==11||item==14||item==17||item==20){
      Y_txt = Y_txt+18+5;
      X_left = 5;
    }
    else X_left = X_left + 105;
  }
  X_left=5;
  sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
  ILI9341_WriteString(X_left, Y_bottom - 22, buffTFT, Font_11x18, ILI9341_YELLOW, ILI9341_BLACK);
}

void displT_16x26(){
 uint8_t item, amnt=0;
 uint16_t color_txt;
 int16_t max_t=-550, min_t=1270, midl_t=0;
  for (item = 0; item < ds18b20_amount; item++){
    if (ds18b20_val[item]<1270) max_t = max(max_t,ds18b20_val[item]);
    if (ds18b20_val[item]>-550) min_t = min(min_t,ds18b20_val[item]);
    if (ds18b20_val[item]<1270) {midl_t = midl_t+ds18b20_val[item]; amnt++;}
    if (ds18b20_val[item]<1000) sprintf(buffTFT,"t%02d=%.1f  ",item+1 ,(float)ds18b20_val[item]/10);
    else if (ds18b20_val[item]<1270) sprintf(buffTFT,"t%02d=%d  ",item+1 , ds18b20_val[item]/10);
    else sprintf(buffTFT,"t%02d=***  ",item+1);
    if (assistent){
        if (item == ds18b20_num) color_txt = ILI9341_GREEN; else color_txt = ILI9341_WHITE;
    }
    else {
//        if (ds18b20_val[item]>=set[0]) {color_txt = ILI9341_MAGENTA; ticTimer+=5;}
//        else if (ds18b20_val[item]<=set[1]) {color_txt = ILI9341_CYAN; ticTimer+=5;}
//        else color_txt = ILI9341_WHITE;
    }
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_16x26, color_txt, ILI9341_BLACK);
    if (item==1||item==3||item==5||item==7){
      Y_txt = Y_txt+26+8;
      X_left = 5;
    } 
    else X_left = X_left + 160;
  }
  X_left=5;
  sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
  ILI9341_WriteString(X_left, Y_bottom - 22, buffTFT, Font_11x18, ILI9341_YELLOW, ILI9341_BLACK);
}
//--------- температуры всех датчиков ----------------------
void displ_cl_0(void){
  Y_txt = Y_top; X_left = 5; 
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(3,1,25);// 3 колонки; одна строка; высота 25
    if (card) drawButton(ILI9341_MAGENTA, 0, "Запись");
    else drawButton(ILI9341_WHITE, 0, "нет SD");
    drawButton(ILI9341_CYAN, 1, "Время");
    drawButton(ILI9341_GREEN, 2, "Устан.");
  }

  Y_txt = Y_txt+18+5;
  if (ds18b20_amount > 10) displT_11x18(); else displT_16x26();
}

//--------- информация о файле SD ----------------------------------
void displ_cl_1(void){
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(2,1,25);// две колонки; одна строка; высота 25
    drawButton(ILI9341_BLUE, 0, "Выход");
    drawButton(ILI9341_BLACK, 1, "");
  }

}

//--------- Установки ----------------------------------
void displ_cl_2(void){
//  uint8_t item;
//  uint16_t color_txt;

  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,45);// четыре колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE, 0, "Выход");
    drawButton(ILI9341_GREEN, 1, "^");
    drawButton(ILI9341_GREEN, 2, "v");
    drawButton(ILI9341_MAGENTA, 3, "Кор.");
  }
  Y_txt = Y_txt+10;
 
}

//--------- корекция Установки ----------------------------------
void displ_cl_3(void){
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,45);// четыре колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE, 0, "Отм.");
    drawButton(ILI9341_GREEN, 1, "+");
    drawButton(ILI9341_GREEN, 2, "-");
    drawButton(ILI9341_MAGENTA, 3, "Зап.");
  }
  Y_txt = Y_txt+45;

}

void display_cl(void){
  switch (displ_num){
  	case 0: displ_cl_0(); break;
  	case 1: displ_cl_1(); break;
    case 2: displ_cl_2(); break;
    case 3: displ_cl_3(); break;
  	default: displ_cl_0();	break;
  }
}
