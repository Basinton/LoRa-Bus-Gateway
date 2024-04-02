/* Includes ------------------------------------------------------------------*/
#include "rs485.h"

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <SoftwareSerial.h>

#include "station.h"
#include "crc16.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
TaskHandle_t rs485TaskHandle = NULL;

int RS485_RX_length                     = 0;
char RS485_RX_buffer[RS485_RX_BUF_SIZE] = {0};

RS485_MODE RS485_mode = RS485_RECEIVE;
SoftwareSerial RS485_Serial(RS485_RXD_PIN, RS485_TXD_PIN);

/* Functions -----------------------------------------------------------------*/
uint8_t CRC16_check(char *buffer, int length)
{
    uint16_t calculated_crc = CRC16(buffer, length - 2);

    // Extract the 2-byte CRC from the end of the buffer
    uint16_t received_crc = buffer[length - 1] << 8 | (buffer[length - 2]);

    // Compare the calculated CRC with the received CRC
    if (calculated_crc == received_crc)
    {
        return 1;
    }

    // CRC check failed
    Serial.printf("CRC16: Calculated 0x%04X --- Recieved 0x%04X\n", calculated_crc, received_crc);

    return 0;
}

void rs485_setmode(RS485_MODE RS485_Stt)
{
    if (RS485_Stt == RS485_TRANSMIT)
    {
        digitalWrite(RS485_EN, HIGH);
    }
    else
    {
        while (!RS485_Serial.availableForWrite())
            ;

        digitalWrite(RS485_EN, LOW);
    }
}

void rs485_task(void)
{
    static int BOARD_WINDOW_LENGTH = 4;

    RS485_RX_length = RS485_Serial.readBytes(RS485_RX_buffer, RS485_RX_BUF_SIZE);
    for (int responseIndex = 0; responseIndex < RS485_RX_length / BOARD_WINDOW_LENGTH; ++responseIndex)
    {
        int startIndex = responseIndex * BOARD_WINDOW_LENGTH;

        char *currentResponse = RS485_RX_buffer + startIndex;

        // Serial.print("rs485: \t ");
        // for (int i = 0; i < BOARD_WINDOW_LENGTH; i++)
        // {
        //     Serial.print(currentResponse[i], HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();

        if (CRC16_check(currentResponse, BOARD_WINDOW_LENGTH))
        {
            switch (RS485_RX_buffer[0])
            {
                case REQUEST_TO_STATION:
                    if (busHandleState == WAITING)
                    {
                        isThereRequest  = 1;
                    }
                    else
                    {
                        isBoardReAckStationAccept = 1;
                    }
                    break;

                case BUS_ACCEPT:
                    if (busHandleState == STATION_NOTIFY_BUS_ACCEPT_TO_BOARD)
                    {
                        isNotifyBusAcceptAck = 1;
                    }

                    break;

                case BUS_PASS:
                    if (busHandleState == STATION_NOTIFY_BUS_PASS_TO_BOARD)
                    {
                        isNotifyBusPassAck = 1;
                    }

                    break;

                case DRIVER_CANCEL:
                    if (busHandleState == STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD)
                    {
                        isNotifyBusCancelAck = 1;
                    }

                    break;

                case PASSENGER_CANCEL:
                    if (busHandleState != INIT && busHandleState != WAITING && busHandleState != STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD && busHandleState != DRIVER_CANCEL && busHandleState != BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION && busHandleState != PASSENGER_CANCEL)
                    {
                        isPassengerCancel = 1;

                        Serial.println("rs485: \t [passenger cancel]");
                    }
                    else
                    {
                        isBoardReAckPassengerCancel = 1;
                    }

                    break;

                default:
                    break;
            }
        }
    }
}

void rs485_init(void)
{
    RS485_Serial.begin(RS485_BAUDRATE);
    pinMode(RS485_EN, OUTPUT);
    rs485_setmode(RS485_RECEIVE);

    // xTaskCreate(rs485_task, "rs485 Task", 8192, NULL, configMAX_PRIORITIES, &rs485TaskHandle);

    Serial.println("rs485: \t [init]");
}