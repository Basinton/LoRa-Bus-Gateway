#pragma once
#ifndef __STATION_H_
#define __STATION_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

/* Define --------------------------------------------------------------------*/
#define STATION_ID 1

const uint16_t GATEWAY_ADDRESS = 0x0001;
const uint16_t BUS_ADDRESS     = 0x0800; // 0xFFFF / 2
const uint16_t GATEWAY_CHANNEL = 1;
const uint16_t BUS_50_CHANEL   = 11;

#define REQUESTED_BUS_DIRECTION 1
#define REQUESTED_BUS_NUMBER    50

#define STATION_LAT             10.773762
#define STATION_LNG             106.657042

#define ID_INDEX                1
#define ADDRESS_HI_INDEX        2
#define ADDRESS_LO_INDEX        3
#define STATE_INDEX             4

typedef enum
{
    INIT                                     = 0,
    WAITING                                  = 1,
    REQUEST_TO_STATION                       = 2,
    STATION_NOTIFY_ACCEPT_TO_BOARD           = 3,
    REQUEST_TO_BUS                           = 4,
    STATION_NOTIFY_BUS_ACCEPT_TO_BOARD       = 5,
    BUS_ACCEPT                               = 6,
    STATION_NOTIFY_BUS_PASS_TO_BOARD         = 7,
    BUS_PASS                                 = 8,
    STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD    = 9,
    DRIVER_CANCEL                            = 10,
    BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION = 11,
    PASSENGER_CANCEL                         = 12,
    FINISHED                                 = 13
} SYSTEM_STATE;

/* Variables -----------------------------------------------------------------*/
extern uint8_t isThereRequest;
extern uint8_t isThereBusCancel;
extern uint8_t isBusAccept;
extern uint8_t isBusPass;

extern uint8_t isBusReAckBusPass;
extern uint8_t isBusReAckBusCancel;

extern uint8_t isBoardReAckStationAccept;
extern uint8_t isNotifyBusAcceptAck;
extern uint8_t isNotifyBusPassAck;
extern uint8_t isNotifyBusCancelAck;
extern uint8_t isBoardReAckPassengerCancel;

extern uint8_t isBoardReAckPassengerCancel;
extern uint8_t isPassengerCancel;
extern uint8_t isPassengerCancelAck;
extern uint8_t isBusReAckPassengerCancel;

extern int32_t stationLAT;
extern int32_t stationLNG;

struct BUS_RESPONSE
{
    uint8_t id;

    uint8_t addressHI;
    uint8_t addressLO;
};
extern BUS_RESPONSE busResponse;

extern SYSTEM_STATE busHandleState;

/* Functions -----------------------------------------------------------------*/
void stationInit(void);

#endif