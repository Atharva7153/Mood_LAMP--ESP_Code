#ifndef LAMP_CONTROL_H
#define LAMP_CONTROL_H

#include <FastLED.h>
#include <vector>

#define LED_PIN 13
#define NUM_LEDS 16
#define IR_PIN 27
#define BUTTON_PIN 26

extern CRGB leds[NUM_LEDS];

extern bool irMode;
extern bool lampEnabled;
extern bool multiMode;
extern int irTriggeredValue; // LOW or HIGH expected when object present

// Enable or disable IR mode; when enabling, the code will auto-detect the active sensor value
void setIrMode(bool enable);

void setBrightnessLevel(int value);

void setupLamp();

void updateLamp();

void setSingleColor(CRGB color);

void setMultiColors(const std::vector<CRGB>& colors);

void turnLampOff();

#endif