#include <WiFi.h>
#include <Arduino.h>

#include "button.h"
#include "lora.h"
#include "ota.h"
#include "led.h"
#include "rs485.h"
#include "station.h"

void setup()
{
    delay(1000);

    Serial.begin(115200);

    lora_init();
    led_init();
    button_init();

    stationInit();
    rs485_init();
}

uint32_t startCycle = 0;
void loop()
{
    while (millis() - startCycle < 50);
    startCycle = millis();

    rs485_task();
}