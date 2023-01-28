#include "tft_proc.h"
#include "ili9341.h"
#include "my.h"

extern char buffTFT[];
extern uint8_t displ_num, oneWire_amount, ds18b20_num, fc20H, familycode[][8], newButt, Y_txt, X_left, Y_top, Y_bottom;
extern int16_t result[], fillScreen;
extern int8_t newcorrection, correction[MAX_DEVICE];
uint8_t Y_val, X0axis, MaxAxis;
int16_t X_val=0, X_txt;

void displADC(uint8_t dev){
 uint8_t item, begin, ending, Y_begin, Y_height, Y_end;
  switch (dev){
  	case 1: begin=0; ending=4; break;
  	case 2: begin=4; ending=8; break;
    case 3: begin=8; ending=12; break;
  	default: begin=0; ending=4; break;
  }
  X_left = 0;
  if (newButt){
    newButt = 0;
    ILI9341_FillRectangle(0, Y_txt, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
    initializeButtons(4,1,20);  // четыре колонки; одна строка; высота 20
    drawButton(ILI9341_GREEN, 0, "1");
    drawButton(ILI9341_GREEN, 1, "2");
    drawButton(ILI9341_GREEN, 2, "3");
    drawButton(ILI9341_MAGENTA, 3, "налашт");
  }
  for (item = begin; item < ending; item++){
    sprintf(buffTFT,"ADC%02d=%4d  ",item+1 , result[item]);
    ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
    X_left = X_left+80;
  }
  if (++X_val>=ILI9341_WIDTH-1) X_val=1;
  ILI9341_FillRectangle(X_val, Y_top, 5, Y_bottom-Y_top, ILI9341_BLACK);
  
  Y_top = Y_txt+10+5;
  X_txt = X_val+5;
  if (X_val+35 >= ILI9341_WIDTH) X_txt = 0;  //X_txt = ILI9341_WIDTH-30
  
  Y_begin = Y_top;
  Y_height = (Y_bottom - Y_top)/4 -1;
  Y_end = Y_begin + Y_height;
  Y_val = map(result[0], 0, 255, Y_begin, Y_end);
  X0axis= map(0, 0, 255, Y_begin, Y_end);
  MaxAxis=map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Y_val, ILI9341_CYAN);
  ILI9341_DrawPixel(X_val, X0axis, ILI9341_WHITE);
//  Y_txt = Y_val;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[0]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
//  ILI9341_DrawPixel(X_val, MaxAxis, ILI9341_WHITE);
  
  Y_begin = Y_end+1;
  Y_end = Y_begin + Y_height;
  Y_val = map(result[1], 0, 255, Y_begin, Y_end);
  X0axis= map(0, 0, 255, Y_begin, Y_end);
  MaxAxis=map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Y_val, ILI9341_RED);
  ILI9341_DrawPixel(X_val, X0axis, ILI9341_WHITE);
//  Y_txt = Y_val;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[1]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
//  ILI9341_DrawPixel(X_val, MaxAxis, ILI9341_WHITE);
  
  Y_begin = Y_end+1;
  Y_end = Y_begin + Y_height;
  Y_val = map(result[2], 0, 255, Y_begin, Y_end);
  X0axis= map(0, 0, 255, Y_begin, Y_end);
  MaxAxis=map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Y_val, ILI9341_CYAN);
  ILI9341_DrawPixel(X_val, X0axis, ILI9341_WHITE);
//  Y_txt = Y_val;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[2]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
//  ILI9341_DrawPixel(X_val, MaxAxis, ILI9341_WHITE);
  
  Y_begin = Y_end+1;
  Y_end = Y_begin + Y_height;
  Y_val = map(result[3], 0, 255, Y_begin, Y_end);
  X0axis= map(0, 0, 255, Y_begin, Y_end);
  MaxAxis=map(255, 0, 255, Y_begin, Y_end);
  ILI9341_DrawPixel(X_val, Y_val, ILI9341_RED);
  ILI9341_DrawPixel(X_val, X0axis, ILI9341_WHITE);
//  Y_txt = Y_val;
  Y_txt = Y_begin + Y_height/2;
  if (Y_txt+10 >= Y_end) Y_txt = Y_end-12;
  sprintf(buffTFT,"%4d", result[3]);
  ILI9341_WriteString(X_txt, Y_txt, buffTFT, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
//  ILI9341_DrawPixel(X_val, MaxAxis, ILI9341_WHITE);
}

