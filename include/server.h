#ifndef __SERVER_H__
#define __SERVER_H__

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <uri/UriBraces.h>

#include "global.h"

extern bool isBus50Ready;
extern bool isBus08Ready;

void server_init();
void requestBus(String stopCode, String routeCode);
void updateRequestBus(String stopCode, String routeCode, String newStatus);
void requestAnotherAPI(String parameter);

#endif