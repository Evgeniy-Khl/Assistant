#include "my.h"
#include "buttons.h"

//extern int16_t set[MAX_SET], newval[MAX_SET];
extern uint8_t displ_num, newButt, Y_txt, X_left, Y_top, Y_bottom, buttonAmount;
extern int8_t oneWire_amount, ds18b20_num, numSet, numDate, newDate, tiimeDispl, newcorrection, correction[MAX_DEVICE];
extern uint16_t fillScreen;
extern int16_t result[], max_t, min_t, midl_t, val_t;
//extern RTC_HandleTypeDef hrtc;
//extern RTC_TimeTypeDef sTime;
//extern RTC_DateTypeDef sDate;

void checkButtons_as(uint8_t item){
    switch (displ_num){
    	case 0://--------- температуры всех датчиков ----------------------
        switch (item){
        	case 0: displ_num = 1; newButt = 1; break;
          case 1: if (--ds18b20_num<0) ds18b20_num = 0;	break;
      	  case 2: if (++ds18b20_num>oneWire_amount-1) ds18b20_num = oneWire_amount-1;	break;
          case 3: displ_num = 3; newButt = 1; break;
        }
        item = 10;
    	break;
    	case 1://--------- статус датчика ----------------------------------
        switch (item){
        	case 0: displ_num = 0; newButt = 1; break;
          case 1: if (--ds18b20_num<0) ds18b20_num = oneWire_amount-1;	break;
        	case 2: if (++ds18b20_num>oneWire_amount-1) ds18b20_num = 0;	break;
          case 3: displ_num = 2; newButt = 1; newcorrection = correction[ds18b20_num]; break;
        }
        item = 10;
    	break;
      case 2://--------- коррекция датчика ----------------------------------
        switch (item){
      	  case 0: displ_num = 1; newButt = 1; break;
          case 1: if (--newcorrection<-12) newcorrection =-12;	break;
        	case 2: if (++newcorrection >12) newcorrection = 12;	break;
          case 3: displ_num = 5; newButt = 1; break;
        }
        item = 10;
     	break;
      case 3://--------- груповая корекция датчиков -------------------------
        switch (item){
        	case 0: displ_num = 0; newButt = 1; break;
          case 1: if (++newcorrection>5) newcorrection = 0;	break;
        	case 2: if (--newcorrection<0) newcorrection = 5;	break;
          case 3: 
            newButt = 1;
            if (newcorrection == 5) displ_num = 0; else displ_num = 4;
          break;
        }
        item = 10;
    	break;
      case 4://--------- ПОДТВЕРЖДЕНИЕ груповой корекции датчиков ----------
        switch (item){
        	case 0: displ_num = 3; newButt = 1; break;
          case 1:
            ILI9341_FillRectangle(0, Y_top, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
            Y_txt = Y_top+40; X_left = 45;
            ILI9341_WriteString(X_left, Y_txt, "ВЫПОЛНЯЮ КОРРЕКЦИЮ!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
            Y_txt = Y_txt+18+5; X_left = 5;
            switch (newcorrection){
            	case 0: val_t = result[ds18b20_num];	break;
            	case 1: val_t = max_t; break;
              case 2: val_t = min_t; break;
              case 3: val_t = midl_t; break;
              case 4: val_t = -100; break;
            }
            if (val_t == -100){
              for (item=0; item<oneWire_amount; item++) ds18b20_WriteScratchpad(item,70,0);
            }
            else{
              for (item=0; item<oneWire_amount; item++){
                newcorrection =(int8_t)(val_t-result[item]);
           //   sprintf(buffTFT,"t%02d=%3d t=%3d  nC=%3d",item+1 ,result[item], val_t, newcorrection);
           //   ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
           //   Y_txt = Y_txt+18+5;
           //   HAL_Delay(5000);
                ds18b20_WriteScratchpad(item, TUNING, newcorrection);
              }
            //HAL_Delay(10000);
            }
            HAL_Delay(1000);
            displ_num = 0; newButt = 1; item = 10;
            break;
        }
      break;
      case 5://--------- ПОДТВЕРЖДЕНИЕ коррекции датчика ----------------------------------
        switch (item){
          case 1: newButt=TUNING+1;	break;
          case 2: newButt=TUNING;  break;
        }
        if (item){
          ILI9341_FillRectangle(0, Y_top, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
          Y_txt = Y_top+40; X_left = 45;
          ILI9341_WriteString(X_left, Y_txt, "ВЫПОЛНЯЮ  КОРРЕКЦИЮ!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
          ds18b20_WriteScratchpad(ds18b20_num, newButt, newcorrection);
          HAL_Delay(1000);
        }
        displ_num = 1; newButt = 1; item = 10;
     	break;
      default: displ_num = 0; newButt = 1; break;
    }
}
