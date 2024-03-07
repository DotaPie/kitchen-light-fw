#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <FastLED.h>

enum MOVEMENT_DIRECTION {DIRECTION_LEFT, DIRECTION_RIGHT};

extern Adafruit_ST7789 display;

CRGB calculateColorHueFromPickerPosition(uint16_t pickerPosition);
uint16_t calculatePickerPositionFromColorHue(CRGB color);

void loadDisplayBrightness(uint8_t brightness);
void updateDisplayBrightness(uint8_t brightness);

void loadDisplayColorHue(uint16_t currentColorHueIndex, uint16_t previousColorHueIndex);
void updateDisplayColorHue(uint16_t currentColorHueIndex, uint16_t previousColorHueIndex);

void updateMainScreen(bool forceAll, uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, float temperature, uint8_t wifiSingal);
void clearDisplay();

#endif