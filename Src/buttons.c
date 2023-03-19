#include "my.h"
#include "buttons.h"

//extern int16_t set[MAX_SET], newval[MAX_SET];
extern uint8_t displ_num, newButt, Y_txt, X_left, Y_top, Y_bottom, buttonAmount;
extern int8_t oneWire_amount, ds18b20_num, ds2450_num, numSet, numDate, newDate, tiimeDispl, newcorrection, correction[MAX_DEVICE];
extern uint8_t period;
extern uint16_t fillScreen, prescale;
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
      	  case 2: if (++ds18b20_num>oneWire_amount-1) ds18b20_num = oneWire_amount-1; break;
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
            ILI9341_WriteString(45, 100, "ВЫПОЛНЯЮ КОРРЕКЦИЮ!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
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
           //   Y_txt = 5; X_left = 5;   
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
          ILI9341_WriteString(45, 100, "ВЫПОЛНЯЮ  КОРРЕКЦИЮ!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
          ds18b20_WriteScratchpad(ds18b20_num, newButt, newcorrection);
          HAL_Delay(1000);
        }
        displ_num = 1; newButt = 1; item = 10;
     	break;
      default: displ_num = 0; newButt = 1; break;
    }
}

void checkButtons_ADC(uint8_t item){
    switch (displ_num){
    	case 0://--------- Значение АЦП ----------------------
        switch (item){
        	case 0: ds2450_num = 0; newButt = 1; break;
          case 1: ds2450_num = 1; newButt = 1; break;
      	  case 2: ds2450_num = 2; newButt = 1; break;
          case 3: displ_num = 2;  newButt = 1; break;
        }
        item = 10;
    	break;
      case 1://--------- Графики АЦП ----------------------
        switch (item){
        	case 0: ds2450_num = 0; newButt = 1; break;
          case 1: ds2450_num = 1; newButt = 1; break;
      	  case 2: ds2450_num = 2; newButt = 1; break;
          case 3: displ_num = 2;  newButt = 1; break;
        }
        item = 10;
    	break;
      case 2://--------- коррекция развертки ----------------------------------
        newButt = 1;
        if(item<8){period=item; displ_num = 1;}
        else {displ_num = 0; prescale=1999; period=6;}
        switch (item){
      	  case 0: prescale=59999; break;  // "6.0"
          case 1: prescale=39999; break;  // "4.0"
        	case 2: prescale=19999; break;  // "2.0"
          case 3: prescale=9999;  break;  // "1.0"
          case 4: prescale=4999;  break;  // "0.8"
          case 5: prescale=3999;  break;  // "0.4"
        	case 6: prescale=1999;  break;  // "0.2"
          case 7: prescale= 999;  break;  // "0.1"
        }
        item = 10;
     	break;  
    }
  
}

