/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "tft_proc.h"
#include "ili9341_touch.h"
#include "ds18b20.h"
#include "displ_as.h"
#include "displ_adc.h"
#include "buttons.h"
#include "my.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
const float A1=1.6, A2=0.64, A3=0.04;  // ������� a=0.8 (A1=2a; A2=a^2; A3=(1-a)^2)
char txt[10], buffTFT[60];
uint8_t displ_num=0, newButt=1, ticTimer, ticTouch, show, showADC, Y_txt=5, X_left=5, Y_top, Y_bottom=ILI9341_HEIGHT-22, buttonAmount, secTick;
uint8_t noname, fc20H, fc28H, familycode[MAX_DEVICE][8]={0};
int8_t oneWire_amount, ds18b20_num, ds2450_num, numSet=0, numDate=0, newDate=0, resetDispl=0, newcorrection, correction[MAX_DEVICE];
int16_t result[MAX_DEVICE]={199}, max_t, min_t, midl_t, val_t, pvT, pvRH;
uint16_t touch_x, touch_y;
uint16_t fillScreen = ILI9341_BLACK;
extern uint16_t prescale;
uint32_t checkButt;
float PVold1, PVold2;
struct ram_structure {int x,y; char w,h;} buttons[4];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_RTC_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void home_screen(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if(htim->Instance == TIM1){ //check if the interrupt comes from TIM1 (10 ms)
    checkButt++;
    if (ticTouch){ --ticTouch; HAL_GPIO_WritePin(Touch_GPIO_Port, Touch_Pin, GPIO_PIN_SET);}// ��������� �������
    else {HAL_GPIO_WritePin(Touch_GPIO_Port, Touch_Pin, GPIO_PIN_RESET);}
//    if (ticTimer){ --ticTimer;
//      if (set[3]&1) HAL_GPIO_WritePin(Alarm_GPIO_Port, Alarm_Pin, GPIO_PIN_SET); // �������� �������
//    }
//    else HAL_GPIO_WritePin(Alarm_GPIO_Port, Alarm_Pin, GPIO_PIN_RESET);
  }
  if(htim->Instance == TIM1){
    if(prescale){__HAL_TIM_SET_PRESCALER(&htim2, prescale); prescale = 0;}
  }
}
uint16_t LowPassF2(uint16_t pv){
float val;
  val = A1*PVold1-A2*PVold2+A3*pv;
  PVold2 = PVold1;
  PVold1 = val;
  return val;
};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
//------------------------------------- ��������� ���������� -----------
  uint8_t item;
  uint16_t adcVal;
  float uVal;
//------------------------------------- ������ ���������� -----------
  HAL_TIM_Base_Start_IT(&htim1);          /* ------  ������ 100��.  ������  10 ��.  ----*/
  HAL_TIM_Base_Start_IT(&htim2);          /* ------  ������  5��.   ������ 200 ��.  ----*/
  HAL_RTCEx_SetSecond_IT(&hrtc);          // Sets Interrupt for second
  HAL_ADCEx_Calibration_Start(&hadc1);    // ���������a ���
  //HAL_GPIO_WritePin(GPIOA, Alarm_Pin, GPIO_PIN_SET);  // LED_PC13=OFF
  TFT_init();
  home_screen();                      // �������� ������������ ��������
  ILI9341_FillScreen(fillScreen);     // ������� ������
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  secTick = 0;
  show = 0;
  while (1){
    Y_txt = 5; X_left = 5;        // Y_top = Y_txt;
    if(fc20H){                    // DS2450
        //------------------------------------- �������� ---------------------------
        if (ILI9341_TouchPressed()&& checkButt>40){
//          ILI9341_WriteString(X_left, Y_bottom - 22, "TouchPressed!", Font_11x18, ILI9341_MAGENTA, fillScreen);
          if(ILI9341_TouchGetCoordinates(&touch_x, &touch_y)){
              for (item=0; item<buttonAmount; item++){
                  if(contains(touch_x, touch_y, item)) break; // �������� ��������� ����� ���������� � ������� ������
              }
              checkButtons_ADC(item); // �������� ������� ������               
              displADC();     // ����������� ���������� ���
          }
          checkButt = 0;
        }
        if (showADC){
            showADC = 0;
            DS2450_check();
            displADC();     // ����������� ���������� ��� 
        }
    }
    else if(show){
        show = 0;
        if(fc28H){                        // DS18b20
            //------------------------------------- �������� ---------------------------
            if(ILI9341_TouchPressed()&& checkButt>40){
              //ILI9341_WriteString(X_left, Y_bottom - 22, "TouchPressed!", Font_11x18, ILI9341_MAGENTA, fillScreen);
              if(ILI9341_TouchGetCoordinates(&touch_x, &touch_y)){
                  for (item=0; item<buttonAmount; item++){
                      if(contains(touch_x, touch_y, item)) break; // �������� ��������� ����� ���������� � ������� ������
                  }
                  checkButtons_as(item); // �������� ������� ������               
                  if (displ_num) resetDispl=60; else resetDispl = 0;
              }
              checkButt = 0;
            }
            //------------------------------------- �������� ����� ---------------------------
            //-------- show, secTick are changed in function handles RTC global interrupt -> {void RTC_IRQHandler(void)} in file "stm32f1xx_it.c"
            temperature_check();  // ��������� �����������
            display_as();         // ����������� ����������
        }
        else {
            item = readDHT(0);        // DHT-21 ��������� �� ����� 1 Wire
            if(item){
                sprintf(buffTFT,"1Wt=%.1f  Rh=%.1f%% ",(float)pvT/10,(float)pvRH/10);
                ILI9341_WriteString(5, Y_txt, buffTFT, Font_16x26, ILI9341_CYAN, ILI9341_BLACK);
                Y_txt = Y_txt+26+8;
            }
        }
        if(fc28H < 9 && displ_num == 0){
            item = readDHT(1);            // DHT-21 ��������� �� ����� AM2301
            if (item){
              sprintf(buffTFT,"Amt=%.1f  Rh=%.1f%% ",(float)pvT/10,(float)pvRH/10);
              ILI9341_WriteString(5, Y_txt, buffTFT, Font_16x26, ILI9341_CYAN, ILI9341_BLACK);
              Y_txt = Y_txt+26+8;
            }

            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1,100);
            adcVal = ((float)HAL_ADC_GetValue(&hadc1));
            HAL_ADC_Stop(&hadc1);              
            if (adcVal > 500){
              if (PVold1==0) PVold2 = PVold1 = adcVal;
              else adcVal = LowPassF2(adcVal);
              uVal = (float)adcVal*3.35/4096;
              
              if (uVal > 0.8) sprintf(buffTFT,"V=%.3f   Rh=%.1f%% ",uVal,(uVal/5-0.1515)/0.00636);
              else sprintf(buffTFT,"V=%.3f   RH=0  ",uVal);
              ILI9341_WriteString(5, Y_txt, buffTFT, Font_16x26, ILI9341_CYAN, ILI9341_BLACK);
              Y_txt = Y_txt+26+8;
            }
            else if(adcVal > 10) {
              sprintf(buffTFT,"ADC=%4i",adcVal);
              ILI9341_WriteString(85, Y_txt, buffTFT, Font_16x26, ILI9341_CYAN, ILI9341_BLACK);
              Y_txt = Y_txt+26+8;
            }
            if(Y_txt==5){
              ILI9341_FillScreen(fillScreen);
              ILI9341_WriteString(45, 100, "������� ���������!", Font_11x18, ILI9341_RED, ILI9341_BLACK);
              Y_txt = Y_txt+18+5;
              if(fc28H==0){
                item = oneWire_count(MAX_DEVICE);       // ��������� ������� �������� ���� item = 0 ������� �������
                ILI9341_WriteString(145, 100, "...", Font_11x18, ILI9341_RED, ILI9341_BLACK);
                if(item){
                  newButt = 1;
                  sprintf(buffTFT,"���������� 1-Wire: %d ��.",oneWire_amount);
                  ILI9341_WriteString(5, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
                  Y_txt = Y_txt+18+5;
                  HAL_Delay(2000);
                }
              }
            }
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 719;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 7199;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, T_CS_Pin|Alarm_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TFT_CS_Pin|TFT_DC_Pin|Touch_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : T_IRQ_Pin */
  GPIO_InitStruct.Pin = T_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(T_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : T_CS_Pin TFT_CS_Pin TFT_DC_Pin */
  GPIO_InitStruct.Pin = T_CS_Pin|TFT_CS_Pin|TFT_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : AM2301_Pin OneWR_Pin */
  GPIO_InitStruct.Pin = AM2301_Pin|OneWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_RST_Pin */
  GPIO_InitStruct.Pin = TFT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TFT_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Touch_Pin Alarm_Pin */
  GPIO_InitStruct.Pin = Touch_Pin|Alarm_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
