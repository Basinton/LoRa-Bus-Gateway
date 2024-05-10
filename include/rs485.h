#ifndef __RS485_H_
#define __RS485_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Define --------------------------------------------------------------------*/
#define RS485_RX_BUF_SIZE (1024 + 1)
#define RS485_TXD_PIN     (GPIO_NUM_22)
#define RS485_RXD_PIN     (GPIO_NUM_26)
#define RS485_EN          (GPIO_NUM_25)

#define RS485_BAUDRATE    9600

typedef enum {RS485_TRANSMIT = 1, RS485_RECEIVE = 0} RS485_MODE;

/* Variables -----------------------------------------------------------------*/
extern int RS485_RX_length;
extern char RS485_RX_buffer[RS485_RX_BUF_SIZE];
extern RS485_MODE RS485_mode;

extern SoftwareSerial RS485_Serial;

/* Functions -----------------------------------------------------------------*/
void rs485_init();
void rs485_setmode(RS485_MODE RS485_Stt);
void rs485_task(void);

#endif