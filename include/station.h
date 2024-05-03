#ifndef __STATION_H_
#define __STATION_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Define --------------------------------------------------------------------*/
#define STATION_ID 1

const uint16_t GATEWAY_ADDRESS = 0x0001;
const uint16_t GATEWAY_CHANNEL = 1;

const uint16_t BUS_ADDRESS[BUS_COUNT] = {0x0001, 0x0010, 0x0011, 0x0100, 0x0101}; //Node's Addresses
const uint16_t BUS_CHANEL[BUS_COUNT] = {11, 22, 33, 44, 55};                      //Node's Channels

#define REQUESTED_BUS_DIRECTION 1
#define REQUESTED_BUS_NUMBER 50

#define STATION_LAT 10.773762
#define STATION_LNG 106.657042

#define BUS_NUMBER  1
#define ID_INDEX 2
#define ADDRESS_HI_INDEX 3
#define ADDRESS_LO_INDEX 4
#define STATE_INDEX 5

#define REQUEST_TIMEOUT 8000     // Timeout = 160 * 50 = 8000 ms

typedef enum
{
    ERROR_TIMEOUT = -1,
    INIT = 0,
    WAITING = 1,
    REQUEST_TO_STATION = 2,
    STATION_NOTIFY_ACCEPT_TO_BOARD = 3,
    REQUEST_TO_BUS = 4,
    STATION_NOTIFY_BUS_ACCEPT_TO_BOARD = 5,
    BUS_ACCEPT = 6,
    STATION_NOTIFY_BUS_PASS_TO_BOARD = 7,
    BUS_PASS = 8,
    STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD = 9,
    DRIVER_CANCEL = 10,
    BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION = 11,
    PASSENGER_CANCEL = 12,
    FINISHED = 13
} SYSTEM_STATE;

/* Variables -----------------------------------------------------------------*/
extern uint8_t isThereRequest[BUS_COUNT];
extern uint8_t isThereBusCancel[BUS_COUNT];
extern uint8_t isBusAccept[BUS_COUNT];
extern uint8_t isBusPass[BUS_COUNT];

extern uint8_t isBusReAckBusPass[BUS_COUNT];
extern uint8_t isBusReAckBusCancel[BUS_COUNT];

extern uint8_t isBoardReAckStationAccept[BUS_COUNT];
extern uint8_t isNotifyBusAcceptAck[BUS_COUNT];
extern uint8_t isNotifyBusPassAck[BUS_COUNT];
extern uint8_t isNotifyBusCancelAck[BUS_COUNT];
extern uint8_t isBoardReAckPassengerCancel[BUS_COUNT];

extern uint8_t isBoardReAckPassengerCancel[BUS_COUNT];
extern uint8_t isPassengerCancel[BUS_COUNT];
extern uint8_t isPassengerCancelAck[BUS_COUNT];
extern uint8_t isBusReAckPassengerCancel[BUS_COUNT];

extern int32_t stationLAT;
extern int32_t stationLNG;

struct BUS_RESPONSE
{
    uint8_t id;

    uint8_t addressHI;
    uint8_t addressLO;
};
extern BUS_RESPONSE busResponse[BUS_COUNT];

extern SYSTEM_STATE busHandleState[BUS_COUNT];

/* Functions -----------------------------------------------------------------*/
void stationInit(void);

#endif