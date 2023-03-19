#include "tft_proc.h"
#include "ili9341.h"
#include "displ_adc.h"
#include "my.h"

extern char buffTFT[];
extern uint8_t displ_num, oneWire_amount, ds2450_num, fc20H, familycode[][8], newButt, Y_txt, Y_top, Y_bottom, X_left;
extern int16_t result[], fillScreen;
extern int8_t newcorrection, correction[MAX_DEVICE];
const char* pertxt[8]={"6.0","4.0","2.0","1.0","0.8","0.4","0.2","0.1"};
uint8_t Pval, MinVal, MaxVal, period=6;
int16_t X_val=0, X_txt, prescale=0;

void displADC_0(){
 uint8_t item;
// uint16_t color_txt;
// int16_t max_t=-550, min_t=1270, midl_t=0;
  
  if (newButt){
    newButt = 0; X_val = 0;
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,20);  // четыре колонки; одна строка; высота 20
    drawButton(ILI9341_GREEN, 0, "1");
    drawButton(ILI9341_GREEN, 1, "2");
    drawButton(ILI9341_GREEN, 2, "3");
    drawButton(ILI9341_MAGENTA, 3, "налашт");
    ILI9341_WriteString(20, 0, pertxt[period], Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    sprintf(buffTFT,"сек. АЦП #%d",ds2450_num+1);
    ILI9341_WriteString(60, 0, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  }
  Y_txt = 25;
  for (item = 0; item < 4; item++){
    sprintf(buffTFT,"V%d=%1.2f  ",item+1 ,(float)result[ds2450_num*4+item]*VREF/256);
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
    if (item==1||item==3||item==5){
      Y_txt = Y_txt+26+8;
      X_left = 5;
    } 
    else X_left = X_left + 160;
  }
  Y_txt = Y_bottom - 18; X_left=5;
//  sprintf(buffTFT,"MAX=%.1f  MIN=%.1f  MID=%.1f",(float)max_t/10,(float)min_t/10,(float)midl_t/amnt/10);
//  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
}

void displADC_1(){
 uint8_t Y_begin, Y_height, Y_end;
  if (newButt){
    newButt = 0; X_val = 0;
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,20);  // четыре колонки; одна строка; высота 20
    drawButton(ILI9341_GREEN, 0, "1");
    drawButton(ILI9341_GREEN, 1, "2");
    drawButton(ILI9341_GREEN, 2, "3");
    drawButton(ILI9341_MAGENTA, 3, "налашт");
    ILI9341_WriteString(20, 0, pertxt[period], Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    sprintf(buffTFT,"сек./крап. АЦП #%d",ds2450_num+1);
    ILI9341_WriteString(60, 0, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  }
  
  if (++X_val>=ILI9341_WIDTH-1) X_val=1;
  ILI9341_FillRectangle(X_val, Y_top, 5, Y_bottom-Y_top, ILI9341_BLACK);
  
  Y_top = 18;
  X_txt = X_val+5;
  if (X_val+35 >= ILI9341_WIDTH) X_txt = 0;  //X_txt = ILI9341_WIDTH-30
  
  Y_begin = Y_top;
  Y_height = (Y_bottom - Y_top)/4 -1;
  Y_end = Y_begin + Y_height;
  Pval = map(result[ds2450_num*4], 0, 255, Y_begin, Y_end);
  MinVal= map(0, 0, 255, Y_begin, Y_end);
//  MaxVal= map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Pval, ILI9341_GREEN);
  ILI9341_DrawPixel(X_val, MinVal, ILI9341_WHITE);
//  ILI9341_DrawPixel(X_val, MaxVal, ILI9341_WHITE);  
//  Y_txt = Pval;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[ds2450_num*4]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);

  Y_begin = Y_end+1;
  Y_end = Y_begin + Y_height;
  Pval = map(result[ds2450_num*4+1], 0, 255, Y_begin, Y_end);
  MinVal= map(0, 0, 255, Y_begin, Y_end);
//  MaxVal= map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Pval, ILI9341_RED);
  ILI9341_DrawPixel(X_val, MinVal, ILI9341_WHITE);
//  ILI9341_DrawPixel(X_val, MaxVal, ILI9341_WHITE);  
//  Y_txt = Pval;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[ds2450_num*4+1]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);

  Y_begin = Y_end+1;
  Y_end = Y_begin + Y_height;
  Pval = map(result[ds2450_num*4+2], 0, 255, Y_begin, Y_end);
  MinVal= map(0, 0, 255, Y_begin, Y_end);
//  MaxVal= map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Pval, ILI9341_CYAN);
  ILI9341_DrawPixel(X_val, MinVal, ILI9341_WHITE);
//  ILI9341_DrawPixel(X_val, MaxVal, ILI9341_WHITE);
//  Y_txt = Pval;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[ds2450_num*4+2]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);

  Y_begin = Y_end+1;
  Y_end = Y_begin + Y_height;
  Pval = map(result[ds2450_num*4+3], 0, 255, Y_begin, Y_end);
  MinVal= map(0, 0, 255, Y_begin, Y_end);
//  MaxVal= map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Pval, ILI9341_YELLOW);
  ILI9341_DrawPixel(X_val, MinVal, ILI9341_WHITE);
//  ILI9341_DrawPixel(X_val, MaxVal, ILI9341_WHITE);
//  Y_txt = Pval;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[ds2450_num*4+3]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
}

//--------- корекция датчика ----------------------------------
void displADC_2(void){
// uint8_t i;
  Y_txt = Y_top;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,3,45);// четыре колонки; три строки; высота 45
    drawButton(ILI9341_YELLOW, 0, "6.0"); // 34 min.
    drawButton(ILI9341_YELLOW, 1, "4.0"); // 22 min.
    drawButton(ILI9341_YELLOW, 2, "2.0"); // 11 min.
    drawButton(ILI9341_YELLOW, 3, "1.0"); // 5 min.
    drawButton(ILI9341_GREEN, 4, "0.8");  // 4 min 32 sec.
    drawButton(ILI9341_GREEN, 5, "0.4");  // 2 min 16 sec.
    drawButton(ILI9341_GREEN, 6, "0.2");  // 1 min 4 sec.
    drawButton(ILI9341_GREEN, 7, "0.1");  // 32 sec.
    drawButton(ILI9341_CYAN, 8, "but1");
    drawButton(ILI9341_CYAN, 9, "but2");
    drawButton(ILI9341_CYAN,10, "but3");
    drawButton(ILI9341_CYAN,11, "but4");
  }

}

void displADC(void){
  switch (displ_num){
  	case 0: displADC_0(); break;
  	case 1: displADC_1(); break;
    case 2: displADC_2(); break;
//    case 3: displ_as_3(); break;
//    case 4: displ_as_4(); break;
//    case 5: displ_as_5(); break;
  	default: displADC_0();	break;
  }
}
