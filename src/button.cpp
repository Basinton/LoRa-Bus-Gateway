#include "button.h"

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint32_t keyCode = 0;
uint32_t keyTimeBeforeReleasing = 0;

/* Task handles */
TaskHandle_t buttonTaskHandle = NULL;

/* Functions -----------------------------------------------------------------*/
void button_task(void *pvParameters)
{
    unsigned long previousMillis = 0;

    while (1)
    {
        if (digitalRead(BUTTON_PIN) == LOW)
        {
            keyTimeBeforeReleasing = 0;
            keyCode++;

            if (keyCode == 1000)
            {
                keyCode = 1000;
            }
        }
        else
        {
            keyTimeBeforeReleasing = keyCode;
            keyCode = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void button_init()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    keyCode = 0;
    keyTimeBeforeReleasing = 0;

    // Create the button task
    xTaskCreate(button_task, "Button Task", 1024, NULL, configMAX_PRIORITIES - 1, &buttonTaskHandle);

    Serial.println("but: \t [init]");
}

uint32_t button_keycode()
{
    return keyCode;
}

uint32_t button_keycode_release()
{
    return keyTimeBeforeReleasing;
}