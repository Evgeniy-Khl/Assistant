/* vim: set ai et ts=4 sw=4: */
#ifndef __ILI9341_H__
#define __ILI9341_H__

#include "main.h"
#include "fonts.h"
#include <stdbool.h>

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

/*** Redefine if necessary ***/
#define ILI9341_SPI_PORT hspi2
extern SPI_HandleTypeDef ILI9341_SPI_PORT;

#define ILI9341_RES_Pin       TFT_RST_Pin
#define ILI9341_RES_GPIO_Port TFT_RST_GPIO_Port
#define ILI9341_CS_Pin        TFT_CS_Pin
#define ILI9341_CS_GPIO_Port  GPIOA
#define ILI9341_DC_Pin        TFT_DC_Pin
#define ILI9341_DC_GPIO_Port  GPIOA

// default orientation
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)
*/
// rotate right

#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)


// rotate left
/*
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
*/

// upside down
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)
*/

/****************************/
// Color definitions
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
//#define ILI9341_NAVY 0x000F        ///<   0,   0, 123   бнеммн-лнпяйни
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0   релмн-гекемши
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123   релмн-цнксани
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0   релмн-анпднбши
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123   тхнкернбши
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0   нкхбйнбши
//#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198   яберкн-яепши
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123   релмн-яепши
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255   
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0   
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255   
#define ILI9341_RED 0xF800         ///< 255,   0,   0   
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255   оспоспмши
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0   
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255   
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0   
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41   фекрн-гекемши
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198   пнгнбши

// Color definitions
//#define	ILI9341_BLACK   0x0000
//#define	ILI9341_BLUE    0x001F
//#define	ILI9341_RED     0xF800
//#define	ILI9341_GREEN   0x07E0
//#define ILI9341_CYAN    0x07FF
//#define ILI9341_MAGENTA 0xF81F
//#define ILI9341_YELLOW  0xFFE0
//#define ILI9341_WHITE   0xFFFF
//#define ILI9341_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

// call before initializing any SPI devices
void ILI9341_Unselect(void);

void ILI9341_Init(void);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_InvertColors(bool invert);
uint8_t map(uint8_t val, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);
#endif // __ILI9341_H__
