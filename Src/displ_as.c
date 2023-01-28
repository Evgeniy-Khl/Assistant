#include "tft_proc.h"
#include "ili9341.h"
#include "ds18b20.h"
#include "displ_as.h"
#include "my.h"

extern char buffTFT[];
extern uint8_t displ_num, oneWire_amount, ds18b20_num, familycode[][8], newButt, Y_txt, X_left, Y_top, Y_bottom;
extern int16_t result[], fillScreen;
extern int8_t newcorrection, correction[MAX_DEVICE];

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
  for (item = 0; item < oneWire_amount; item++){
    if (result[item]<1270) max_t = max(max_t,result[item]);
    if (result[item]>-550) min_t = min(min_t,result[item]);
    if (result[item]<1270) {midl_t = midl_t+result[item]; amnt++;}
    if (result[item]<1000) sprintf(buffTFT,"t%02d=%.1f  ",item+1 ,(float)result[item]/10);
    else if (result[item]<1270) sprintf(buffTFT,"t%02d=%d  ",item+1 , result[item]/10);
    else sprintf(buffTFT,"t%02d=***  ",item+1);
    if (item == ds18b20_num) color_txt = ILI9341_GREEN; else color_txt = ILI9341_WHITE;
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, color_txt, ILI9341_BLACK);
    if (item==2||item==5||item==8||item==11||item==14||item==17){
      Y_txt = Y_txt+18+5;
      X_left = 5;
    }
    else X_left = X_left + 105;
  }
  Y_txt = Y_bottom - 18; X_left=5;
  sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
}

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
    if (item==1||item==3||item==5){
      Y_txt = Y_txt+26+8;
      X_left = 5;
    } 
    else X_left = X_left + 160;
  }
  Y_txt = Y_bottom - 26; X_left=5;
  sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
}
//--------- температуры всех датчиков ----------------------
void displ_as_0(void){
  Y_txt = Y_top; X_left = 5; 
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,45);// четыре колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE, 0, "Полный");
    drawButton(ILI9341_GREEN, 1, "<");
    drawButton(ILI9341_GREEN, 2, ">");
    drawButton(ILI9341_MAGENTA, 3, "Корр.");
  }
  ILI9341_WriteString(X_left+60, Y_txt, "режим АССИСТЕНТ", Font_11x18, ILI9341_YELLOW, fillScreen);
  Y_txt = Y_txt+18+5;
  if (oneWire_amount > 8) displT_11x18(); else displT_16x26();
}
//--------- статус датчика ----------------------------------
void displ_as_1(void){
 uint8_t i = ds18b20_num;
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,45);// четыре колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE, 0, "Кратко");
    drawButton(ILI9341_GREEN, 1, "<");
    drawButton(ILI9341_GREEN, 2, ">");
    drawButton(ILI9341_MAGENTA, 3, "Корр.");
  }
  sprintf(buffTFT,"датчик N%d t=%.1f  ", i+1,(float)result[i]/10);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"коррекция  =%.1f  ",(float)correction[i]/10);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
  Y_txt = Y_txt+36+10;
  sprintf(buffTFT,"hex %02X %02X %02X %02X %02X %02X %02X %02X",
    familycode[i][0], familycode[i][1], familycode[i][2], familycode[i][3],
    familycode[i][4], familycode[i][5], familycode[i][6], familycode[i][7]);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"dec_1  %03u %03u %03u %03u",
    familycode[i][0], familycode[i][1], familycode[i][2], familycode[i][3]);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"dec_2  %03u %03u %03u %03u",
    familycode[i][4], familycode[i][5], familycode[i][6], familycode[i][7]);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  ds18b20_ReadStratcpad(Y_txt, i);
}
//--------- корекция датчика ----------------------------------
void displ_as_2(void){
 uint8_t i = ds18b20_num;
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,45);// четыре колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE, 0, "Отм.");
    drawButton(ILI9341_GREEN, 1, "-");
    drawButton(ILI9341_GREEN, 2, "+");
    drawButton(ILI9341_MAGENTA, 3, "Кор.");
  }
  Y_txt = Y_txt+10;
  sprintf(buffTFT,"датчик N%d", i+1);
  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"t=%.1f  ",(float)(result[i]-correction[i]+newcorrection)/10);
  ILI9341_WriteString(X_left+16, Y_txt, buffTFT, Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+26+8;
  ILI9341_WriteString(X_left+20, Y_txt, "коррекция", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"%.1f  ",(float)newcorrection/10);
  ILI9341_WriteString(X_left+64, Y_txt, buffTFT, Font_16x26, ILI9341_MAGENTA, ILI9341_BLACK);
  Y_txt = Y_txt+26+8;
}
//--------- груповая корекция датчиков ----------------------------------
void displ_as_3(void){
 uint8_t i = ds18b20_num, item, amnt=0;
 uint16_t colorTxt;
 extern int16_t max_t, min_t, midl_t;
  max_t=-550; min_t=1270; midl_t=0;
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0; newcorrection = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,45);// четыре колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE, 0, "Отм.");
    drawButton(ILI9341_GREEN, 1, "v");
    drawButton(ILI9341_GREEN, 2, "^");
    drawButton(ILI9341_MAGENTA, 3, "Кор.");
  }
  for (item = 0; item < oneWire_amount; item++){
    if (result[item]<1270) max_t = max(max_t,result[item]);
    if (result[item]>-550) min_t = min(min_t,result[item]);
    if (result[item]<1270) {midl_t += result[item]; amnt++;}
  }
  Y_txt = Y_txt+10;
  ILI9341_WriteString(X_left+20, Y_txt, "выровнять все датчики :", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"по датчику N%2d   t=%.1f  ", i+1,(float)result[i]/10);
  if (newcorrection==0) colorTxt = ILI9341_MAGENTA; else colorTxt = ILI9341_WHITE;
  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, colorTxt, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"по максимальному t=%.1f  ",(float)max_t/10);
  if (newcorrection==1) colorTxt = ILI9341_MAGENTA; else colorTxt = ILI9341_WHITE;
  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, colorTxt, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"по минимальному  t=%.1f  ",(float)min_t/10);
  if (newcorrection==2) colorTxt = ILI9341_MAGENTA; else colorTxt = ILI9341_WHITE;
  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, colorTxt, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"по среднему      t=%.1f  ",(float)midl_t/amnt/10);
  if (newcorrection==3) colorTxt = ILI9341_MAGENTA; else colorTxt = ILI9341_WHITE;
  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, colorTxt, ILI9341_BLACK);
  Y_txt = Y_txt+18+10;
  if (newcorrection==4) colorTxt = ILI9341_MAGENTA; else colorTxt = ILI9341_WHITE;
  ILI9341_WriteString(X_left+20, Y_txt, "УДАЛИТЬ коррекцию у всех.", Font_11x18, colorTxt, ILI9341_BLACK);
  Y_txt = Y_txt+18+10;
  if (newcorrection==5) colorTxt = ILI9341_MAGENTA; else colorTxt = ILI9341_WHITE;
  ILI9341_WriteString(X_left+50, Y_txt, "режим КЛЕРК включить", Font_11x18, colorTxt, ILI9341_BLACK);
}
//--------- ПОДТВЕРЖДЕНИЕ груповой корекции датчиков ----------------------------------
void displ_as_4(void){
 uint8_t i = ds18b20_num, item, amnt=0;
 extern int16_t max_t, min_t, midl_t;
  max_t=-550; min_t=1270; midl_t=0;
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(2,1,45);// 2 колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE,  0, "Отмена");
    drawButton(ILI9341_RED,   1, "Запись");
  }
  for (item = 0; item < oneWire_amount; item++){
    if (result[item]<1270) max_t = max(max_t,result[item]);
    if (result[item]>-550) min_t = min(min_t,result[item]);
    if (result[item]<1270) {midl_t = midl_t+result[item]; amnt++;}
  }
  midl_t /= amnt;
  Y_txt = Y_txt+10;
  if (newcorrection<4) ILI9341_WriteString(X_left+20, Y_txt, "выровнять все датчики :", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  switch (newcorrection){
    	case 0:
        sprintf(buffTFT,"по датчику N%2d   t=%.1f  ", i+1,(float)result[i]/10);
        ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    		break;
    	case 1:
        sprintf(buffTFT,"по максимальному t=%.1f  ",(float)max_t/10);
        ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    		break;
    	case 2:
        sprintf(buffTFT,"по минимальному  t=%.1f  ",(float)min_t/10);
        ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    		break;
      case 3:
        sprintf(buffTFT,"по среднему      t=%.1f  ",(float)midl_t/10);
        ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    		break;
      case 4:
        ILI9341_WriteString(X_left+20, Y_txt, "УДАЛИТЬ КОРРЕКЦИЮ", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
        Y_txt = Y_txt+18+5;
        ILI9341_WriteString(X_left+20, Y_txt, "У ВСЕХ ДАТЧИКОВ?", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    		break;
    }
}
//--------- ПОДТВЕРЖДЕНИЕ корекции датчика ----------------------------------
void displ_as_5(void){
 uint8_t i = ds18b20_num;
  Y_txt = Y_top; X_left = 5;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(3,1,45);// 3 колонки; одна строка; высота 45
    drawButton(ILI9341_BLUE,  0, "Отмена");
    drawButton(ILI9341_GREEN, 1, "Особый");
    drawButton(ILI9341_RED,   2, "Запись");
  }
  Y_txt = Y_txt+10;
  sprintf(buffTFT,"датчик N%d", i+1);
  ILI9341_WriteString(X_left+20, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"t=%.1f  ",(float)(result[i]+newcorrection)/10);
  ILI9341_WriteString(X_left+16, Y_txt, buffTFT, Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+26+8;
  ILI9341_WriteString(X_left+20, Y_txt, "коррекция", Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  Y_txt = Y_txt+18+5;
  sprintf(buffTFT,"%.1f  ",(float)newcorrection/10);
  ILI9341_WriteString(X_left+64, Y_txt, buffTFT, Font_16x26, ILI9341_MAGENTA, ILI9341_BLACK);
  Y_txt = Y_txt+26+8;
}
void display_as(void){
  switch (displ_num){
  	case 0: displ_as_0(); break;
  	case 1: displ_as_1(); break;
    case 2: displ_as_2(); break;
    case 3: displ_as_3(); break;
    case 4: displ_as_4(); break;
    case 5: displ_as_5(); break;
  	default: displ_as_0();	break;
  }
}
