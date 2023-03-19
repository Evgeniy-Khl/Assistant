#include "my.h"

#include "tft_proc.h"
#include "ili9341_touch.h"

extern uint8_t displ_num, ticTouch, Y_txt, X_left, Y_top, Y_bottom, buttonAmount;
extern uint16_t fillScreen;
extern struct ram_structure {int x,y; char w,h;} buttons[];

void TFT_init(){
  ILI9341_Unselect();
  ILI9341_TouchUnselect();
  ILI9341_Init();
  ILI9341_FillScreen(fillScreen);
  Y_txt = 5; X_left = 5;
  ILI9341_WriteString(X_left+25, Y_txt, "Программа Ассистент v 0.3", Font_11x18, ILI9341_YELLOW, fillScreen);
  Y_txt = Y_txt+18+5;
}

void WindowDraw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t setcolor, const char* str){
 uint16_t strColor, bordColor; 
  switch (setcolor)
  {
  	case ILI9341_BLACK: strColor=ILI9341_WHITE; bordColor = ILI9341_WHITE; break;
    case ILI9341_BLUE:  strColor=ILI9341_WHITE; bordColor = ILI9341_WHITE; break;
  	default: strColor=ILI9341_BLACK; bordColor = ILI9341_BLACK; break;
  }
  ILI9341_FillRectangle(x, y, w, h, setcolor);
  for(int i = x; i < x+w+1; i++){
     ILI9341_DrawPixel(i, y-1, bordColor);
     ILI9341_DrawPixel(i, y+h+1, bordColor);
  }
  for(int i = y; i < y+h+1; i++){
     ILI9341_DrawPixel(x-1, i, bordColor);
     ILI9341_DrawPixel(x+w+1, i, bordColor);
  }
  ILI9341_WriteString(x+(w-11)/2, y+(h-18)/2, str, Font_11x18, strColor, setcolor);
}

void initializeButtons(uint8_t col, uint8_t row, uint8_t h) // h -> высота кнопки
{
  uint8_t i,j,indx;
  uint16_t x, y, w;
  switch (col)                  // ширина кнопки зависит от кол-ва кнопок в строке
   {
    case 4: w = 72; break;
    case 3: w = 100; break;
    case 2: w = 150; break;   
    default: w = ILI9341_WIDTH-6;
   };
  if(h<20) h=20;
  y = ILI9341_HEIGHT - h - 4;      // верхний контур кнопки
  indx = 0;
  for (j=0; j<row; j++){
    x = 4;// начало 1 кнопки
    for (i=0; i<col; i++){
        buttons[indx].x = x+i*(w+8);// интервал между кнопками по горизонтали
        buttons[indx].w = w;
        buttons[indx].h = h;
        buttons[indx].y = y;
        indx++;
    }
    y -= (h+4);// интервал между кнопками по вертикали
  }
  Y_bottom = y;// нижняя граница до которой можно закрашивать экран
  buttonAmount = col * row;// обшее количество кнопок
}
//-------------------- цвет фона --- цвет рамки -- цвет текста --- номер --- текст ---------
void drawButton(uint16_t setcolor, uint8_t b, char *str)
{
  uint16_t x, y, w, h;
  w = buttons[b].w;      // ширина кнопки
  h = buttons[b].h;      // высота кнопки
  x = buttons[b].x;      // начало контура кнопки
  y = buttons[b].y;      // начало контура кнопки

  ILI9341_FillRectangle(x, y, w, h, setcolor);

  uint16_t strColor, bordColor;
  if (fillScreen == ILI9341_BLACK) bordColor=ILI9341_WHITE;
  else bordColor=ILI9341_BLACK;
  switch (setcolor){
  	case ILI9341_BLACK: strColor=ILI9341_WHITE; break;
    case ILI9341_BLUE:  strColor=ILI9341_WHITE; break;
  	default: strColor=ILI9341_BLACK; break;
  }
  ILI9341_FillRectangle(x, y, w, h, setcolor);
  for(uint16_t i = x; i < x+w+1; i++){
     ILI9341_DrawPixel(i, y-1, bordColor);
     ILI9341_DrawPixel(i, y+h+1, bordColor);
  }
  for(uint16_t i = y; i < y+h+1; i++){
     ILI9341_DrawPixel(x-1, i, bordColor);
     ILI9341_DrawPixel(x+w+1, i, bordColor);
  }

  x = x + w/2 - strlen(str)*11/2;  // ширина символа 11
  y = y + h/2 - 9;                 // высота символа 18 / 2
  
  ILI9341_WriteString(x, y, str, Font_11x18, strColor, setcolor);
//  sprintf(buffTFT,"sizeof=%d",strlen(str));
//  ILI9341_WriteString(5, ILI9341_HEIGHT-(45+18+5+(18+5)*b), buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK); 
}

// проверка попадания пересчитаной координаты в область кнопки.
uint8_t contains(uint16_t touch_X, uint16_t touch_Y, uint8_t b){
 uint16_t beg, end;
   beg = buttons[b].x;
   end = beg + buttons[b].w-3;
   if ((touch_X < beg)||(touch_X > end)) return 0;
   beg = buttons[b].y;
   end = beg + buttons[b].h-3;
   if ((touch_Y < beg)||(touch_Y > end)) return 0;
   else ticTouch = 30;
   return 1;
 }

