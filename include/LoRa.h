#ifndef __LORA_H_
#define __LORA_H_

/*
 * LoRa E32-TTL-100
 * Set configuration.
 * https://www.mischianti.org/2019/10/29/lora-e32-device-for-arduino-esp32-or-esp8266-configuration-part-3/
 *
 * E32-TTL-100----- Arduino UNO
 * M0         ----- 3.3v
 * M1         ----- 3.3v
 * TX         ----- PIN 2 (PullUP)
 * RX         ----- PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Define --------------------------------------------------------------------*/
#define LORA_PACKAGE_SIZE_SEND    8
#define LORA_PACKAGE_SIZE_RECIEVE 6

#define PIN_M0                    18
#define PIN_M1                    5
#define PIN_AUX                   4

/* Variables -----------------------------------------------------------------*/
extern uint8_t lora_receive[1000];
extern uint32_t lora_receive_cnt;

extern LoRa_E32 e32ttl100;

/* Functions -----------------------------------------------------------------*/
uint8_t checkSum(uint8_t *message, int size);
void checkDataReceive(void);
void lora_process(void);

void accessModeConfig(void);
void accessModeTransmit(void);
void setConfig(uint16_t address, uint16_t channel, uint8_t airRate, uint8_t power);

void printParameters(struct Configuration configuration);

void lora_init(void);

#endif