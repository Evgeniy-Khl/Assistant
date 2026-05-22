#ifndef DS18B20_H_
#define DS18B20_H_

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "tft_proc.h"

//--------------------------------------------------
#define ONEWIRE_PORT  GPIOB
#define ONEWIRE_PIN   GPIO_PIN_11
#define DHT_PORT      GPIOB
#define DHT_PIN       GPIO_PIN_10

#define ONEWIRE_LOW()   ONEWIRE_PORT->BSRR = GPIO_BSRR_BR11
#define ONEWIRE_HIGH()  ONEWIRE_PORT->BSRR = GPIO_BSRR_BS11
#define ONEWIRE_READ()  (ONEWIRE_PORT->IDR & GPIO_IDR_IDR11)

#define DHT_LOW()       DHT_PORT->BSRR = GPIO_BSRR_BR10
#define DHT_HIGH()      DHT_PORT->BSRR = GPIO_BSRR_BS10
#define DHT_READ()      (DHT_PORT->IDR & GPIO_IDR_IDR10)

#define TUNING        170
#define SKIP_ROM      0
#define NO_SKIP_ROM   1
#define AM2301_IDR    GPIO_IDR_IDR10
#define OneWR_IDR     GPIO_IDR_IDR11
#define AM2301_RESET  GPIO_BSRR_BR10
#define OneWR_RESET   GPIO_BSRR_BR11
//--------------------------------------------------
void DelayMicro(__IO uint32_t micros);
uint8_t oneWire_Reset(void);
void DHT_port_init(void);
void oneWire_port_init(void);
uint8_t oneWire_SearhRom(uint8_t *Addr);
uint8_t oneWire_ReadBit(void);
uint8_t oneWire_ReadByte(void);
void oneWire_WriteBit(uint8_t bit);
void oneWire_WriteByte(uint8_t dt);
void oneWire_count(uint8_t amount);
void ds18b20_Convert_T(void);
void ds18b20_ReadStratcpad(uint16_t y_pos, uint8_t DevNum);
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl);
uint16_t CRC16(uint8_t *pcBlock, uint16_t len);
uint8_t ds18b20_GetSign(uint16_t dt);
float ds18b20_Convert(uint16_t dt);
void displ_status(void);
void displ_T(void);
void temperature_check(void);
uint8_t dallas_crc8(uint8_t * data, uint8_t size);
uint8_t startDHT(uint8_t cn);
uint8_t readDHT(uint8_t cn);
void DS2450_check(void);
uint8_t DS2450_reset(void);
#endif /* DS18B20_H_ */
