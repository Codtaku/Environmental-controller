#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "globals.h"
extern TaskHandle_t encoderTaskHandle;
void setupInput();               // Initializes pins for encoder and button
void IRAM_ATTR readEncoderISR(); // ISR to handle encoder rotation (attach to pins A and B)
void checkEncoderButton();       // Checks button state (press, long press) - called in main loop
void handleEncoderRotation();    // Processes encoder changes - called in main loop if encoderUpdated is true
void beepEncoder();              // Short beep for feedback
void encoderTask(void *parameter);

// Task function for reading encoder if using a task instead of ISR
// void encoderTask(void * parameter);

#endif // INPUT_HANDLER_H