#pragma once
#ifndef __BUTTON_H_
#define __BUTTON_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Define --------------------------------------------------------------------*/
#define BUTTON_PIN 34

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
void button_init();
void button_process();             // Process button state
uint32_t button_keycode();         // Get the key code
uint32_t button_keycode_release(); // Get key time before releasing

#endif