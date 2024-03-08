#include <Arduino.h>
#include <Preferences.h>

// project includes
#include "console.h"
#include "pinout.h"
#include "conf.h"
#include "macros.h"
#include "display.h"
#include "utilities.h"

// libs
#include <RotaryEncoder.h>
#include <FastLED.h>

// main.cpp globals
STATE state, previousState;

Preferences preferences;

COLOR_PICKER_TYPE current_CPT, previous_CPT;
uint16_t currentColorHueIndex, previousColorHueIndex;
uint16_t currentColorTemperatureIndex, previousColorTemperatureIndex;
uint8_t currentBrightness, previousBrightness;

uint8_t previousSwitch_1, previousSwitch_2;
long previous_encoder_1_position, previous_encoder_2_position;

CRGB LED_stripArray[LED_STRIP_LED_COUNT];

RotaryEncoder encoder_1 = RotaryEncoder(RE_1_IN1_PIN, RE_1_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2 = RotaryEncoder(RE_2_IN1_PIN, RE_2_IN2_PIN, RotaryEncoder::LatchMode::TWO03);

void loadPreferences()
{
    bool firstTimeRun = false;

    CONSOLE("\r\nLoading preferences: ")
    preferences.begin("app", false);

    if(preferences.getUInt("firstRun", 0) != DEFAULT_ID)
    {
        firstTimeRun = true;
        preferences.putUInt("firstRun", DEFAULT_ID);
        preferences.putUChar("CPT", (uint8_t)CPT_COLOR_TEMPERATURE);
        preferences.putUInt("color-hue", 0);
        preferences.putUInt("color-t", 0);
        preferences.putUChar("brightness", DEFAULT_BRIGHTNESS);
    }

    current_CPT = (COLOR_PICKER_TYPE)preferences.getUChar("CPT", (uint8_t)CPT_NONE);  
    previous_CPT = current_CPT;
    currentColorHueIndex = preferences.getUInt("color-hue", 0); 
    previousColorHueIndex = currentColorHueIndex;
    currentColorTemperatureIndex = preferences.getUInt("color-t", 0); 
    previousColorTemperatureIndex = currentColorTemperatureIndex;
    currentBrightness = preferences.getUChar("brightness", DEFAULT_BRIGHTNESS);
    previousBrightness = currentBrightness;    
    
    CONSOLE_CRLF("OK")

    CONSOLE("  |-- first time run: ") 
    CONSOLE_CRLF(firstTimeRun ? "Yes" : "No")

    CONSOLE("  |-- current color picker type: ") 
    CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])

    CONSOLE("  |-- current color hue index: ") 
    CONSOLE_CRLF(currentColorHueIndex);

    CONSOLE("  |-- current color temperature index: ") 
    CONSOLE_CRLF(currentColorTemperatureIndex);
}

void update_LED_strip()
{
    for(uint16_t i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        LED_stripArray[i] = CRGB(0, 0, 0);    
    }

    // zone 1 + 3
    if(previousSwitch_1 == LOW)
    {
        for(uint16_t i = ZONE_1_LED_START_INDEX; i <= ZONE_1_LED_END_INDEX; i++)
        {
            LED_stripArray[i] = (current_CPT == CPT_COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);     
        }

        for(uint16_t i = ZONE_3_LED_START_INDEX; i <= ZONE_3_LED_END_INDEX; i++)
        {
            LED_stripArray[i] = (current_CPT == CPT_COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);    
        }
    }

    // zone 2
    if(previousSwitch_2 == LOW)
    {
        for(uint16_t i = ZONE_2_LED_START_INDEX; i <= ZONE_2_LED_END_INDEX; i++)
        {
            LED_stripArray[i] = (current_CPT == CPT_COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);     
        }
    }

    FastLED.show();
}

void LED_strip_load_animation()
{
    for(uint16_t i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        for(uint16_t j = 0; j < i; j++)
        {
            LED_stripArray[j] = ColorFromPalette(RainbowColors_p, (uint8_t)(j/2), DEFAULT_BRIGHTNESS, COLOR_BLENDING);   
        }

        for(uint16_t j = i; j < LED_STRIP_LED_COUNT; j++)
        {
            LED_stripArray[j] = CRGB(0, 0, 0);   
        }  

        FastLED.show();
        delay(6);
    }
}   

void setup_LED_strip()
{
    CONSOLE("\r\nLED strip: ")
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, LED_STRIP_LED_COUNT).setCorrection(TypicalLEDStrip);

    #ifndef DEVELOPMENT   
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    LED_strip_load_animation();
    #endif

    FastLED.setBrightness(currentBrightness);
    update_LED_strip();

    CONSOLE_CRLF("OK")
}

void checkRotaryEncoderPosition_1()
{
    encoder_1.tick();
}

void checkRotaryEncoderPosition_2()
{
    encoder_2.tick();
}

void setupRotaryEncoders()
{
    CONSOLE("\r\nRotary encoder #1: ")
    attachInterrupt(digitalPinToInterrupt(RE_1_IN1_PIN), checkRotaryEncoderPosition_1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RE_1_IN2_PIN), checkRotaryEncoderPosition_1, CHANGE);
    pinMode(RE_1_SW_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK")

    CONSOLE("Rotary encoder #2: ")
    attachInterrupt(digitalPinToInterrupt(RE_2_IN1_PIN), checkRotaryEncoderPosition_2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RE_2_IN2_PIN), checkRotaryEncoderPosition_2, CHANGE);
    pinMode(RE_2_SW_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK")
}

void setupSwitches()
{
    CONSOLE("\r\nSwitches: ")   
    pinMode(SWITCH_1_PIN, INPUT_PULLUP);
    pinMode(SWITCH_2_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK") 

    CONSOLE("  |-- Switch state #1: ") 
    CONSOLE_CRLF(digitalRead(SWITCH_1_PIN) == LOW ? "ON" : "OFF")

    CONSOLE("  |-- Switch state #2: ") 
    CONSOLE_CRLF(digitalRead(SWITCH_2_PIN) == LOW ? "ON" : "OFF")
}

void loadDefaultValues()
{
    CONSOLE("\r\nLoading default values: ")
    state = STATE_MAIN;
    previousState = STATE_NONE;
    previousSwitch_1 = digitalRead(SWITCH_1_PIN);
    previousSwitch_2 = digitalRead(SWITCH_2_PIN);
    previous_encoder_1_position = 0;
    previous_encoder_2_position = 0;
    CONSOLE_CRLF("OK")
}

void checkSwitches()
{
    bool switch_1 = digitalRead(SWITCH_1_PIN);
    bool switch_2 = digitalRead(SWITCH_2_PIN);
    static uint32_t switch_1_debounce_timer = 0;
    static uint32_t switch_2_debounce_timer = 0;

    if(switch_1 != previousSwitch_1 && millis() - switch_1_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_1_debounce_timer = millis();
        previousSwitch_1 = switch_1;

        update_LED_strip();

        CONSOLE("\r\nSWITCH_1 STATE CHANGE: ");
        CONSOLE_CRLF(switch_1 == LOW ? "ON" : "OFF");
    }

    if(switch_2 != previousSwitch_2 && millis() - switch_2_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_2_debounce_timer = millis();
        previousSwitch_2 = switch_2;

        update_LED_strip();

        CONSOLE("\r\nSWITCH_2 STATE CHANGE: ");
        CONSOLE_CRLF(switch_2 == LOW ? "ON" : "OFF");
    }
}

void updateBrightness(int direction)
{
    int32_t tempBrightness = currentBrightness + (direction * BRIGHTNESS_STEP);

    if(tempBrightness < 0)
    {
        currentBrightness = 0;
    }
    else if(tempBrightness > 255)
    {
        currentBrightness = 255;
    }
    else 
    {
        currentBrightness = (uint8_t)tempBrightness;     
    }

    CONSOLE_CRLF("\r\nBRIGHTNESS UPDATE")
    CONSOLE("  |-- previous value: ")
    CONSOLE_CRLF(previousBrightness)
    CONSOLE("  |-- new value: ")
    CONSOLE_CRLF(currentBrightness)

    FastLED.setBrightness(currentBrightness);
    FastLED.show();

    if(previousBrightness != currentBrightness)
    {
        updateDisplayBrightness(currentBrightness);
    }

    previousBrightness = currentBrightness;
}

void updateColorHue(int direction)
{   
    int32_t tempColorHueIndex = currentColorHueIndex + (direction * COLOR_HUE_INDEX_STEP);

    if(tempColorHueIndex < 0)
    {
        currentColorHueIndex = PICKER_WIDTH - 1;
    }
    else if(tempColorHueIndex >= PICKER_WIDTH)
    {
        currentColorHueIndex = 0;
    }
    else 
    {
        currentColorHueIndex = (uint16_t)tempColorHueIndex;     
    }

    CRGB currentColor = calculateColorHueFromPickerPosition(currentColorHueIndex);
    CRGB previousColor = calculateColorHueFromPickerPosition(previousColorHueIndex);
    

    CONSOLE_CRLF("\r\nCOLOR HUE UPDATE")
    CONSOLE("  |-- previous picker value: ")
    CONSOLE_CRLF(previousColorHueIndex)
    CONSOLE("  |-- previous color value: ")
    CONSOLE("[R: ")
    CONSOLE(previousColor.r)
    CONSOLE("| G: ")
    CONSOLE(previousColor.g)
    CONSOLE("| B: ")
    CONSOLE(previousColor.b)
    CONSOLE_CRLF("]")
    CONSOLE("  |-- new picker value: ")
    CONSOLE_CRLF(currentColorHueIndex)
    CONSOLE("  |-- new color value: ")
    CONSOLE("[R: ")
    CONSOLE(currentColor.r)
    CONSOLE("| G: ")
    CONSOLE(currentColor.g)
    CONSOLE("| B: ")
    CONSOLE(currentColor.b)
    CONSOLE_CRLF("]")

    update_LED_strip();

    if(previousColorHueIndex != currentColorHueIndex)
    {
        updateDisplayColorHue(currentColorHueIndex, previousColorHueIndex); 
    }
     
    previousColorHueIndex = currentColorHueIndex;
}

void updateColorTemperature(int direction)
{   
    int32_t tempColorTemperatureIndex = currentColorTemperatureIndex + (direction * COLOR_TEMPERATURE_INDEX_STEP);

    if(tempColorTemperatureIndex < 0)
    {
        currentColorTemperatureIndex = 0;
    }
    else if(tempColorTemperatureIndex >= PICKER_WIDTH)
    {
        currentColorTemperatureIndex = PICKER_WIDTH - 1;
    }
    else 
    {
        currentColorTemperatureIndex = (uint16_t)tempColorTemperatureIndex;     
    }

    CRGB currentColor = calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);
    CRGB previousColor = calculateColorTemperatureFromPickerPosition(previousColorTemperatureIndex);
    
    CONSOLE_CRLF("\r\nCOLOR TEMPERATURE UPDATE")
    CONSOLE("  |-- previous picker value: ")
    CONSOLE_CRLF(previousColorTemperatureIndex)
    CONSOLE("  |-- previous color value: ")
    CONSOLE("[R: ")
    CONSOLE(previousColor.r)
    CONSOLE("| G: ")
    CONSOLE(previousColor.g)
    CONSOLE("| B: ")
    CONSOLE(previousColor.b)
    CONSOLE_CRLF("]")
    CONSOLE("  |-- new picker value: ")
    CONSOLE_CRLF(currentColorTemperatureIndex)
    CONSOLE("  |-- new color value: ")
    CONSOLE("[R: ")
    CONSOLE(currentColor.r)
    CONSOLE("| G: ")
    CONSOLE(currentColor.g)
    CONSOLE("| B: ")
    CONSOLE(currentColor.b)
    CONSOLE_CRLF("]")

    update_LED_strip();

    if(previousColorTemperatureIndex != currentColorTemperatureIndex)
    {
        updateDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex); 
    }
     
    previousColorTemperatureIndex = currentColorTemperatureIndex;
}

void checkRotaryEncoders(uint32_t *rotary_encoder_timer)
{
    int8_t encoder_1_direction, encoder_2_direction;

    encoder_1.tick();
    encoder_2.tick();

    long encoder_1_position = encoder_1.getPosition();
    long encoder_2_position = encoder_2.getPosition();

    uint8_t encoder_1_switch = digitalRead(RE_1_SW_PIN);
    uint8_t encoder_2_switch = digitalRead(RE_2_SW_PIN);

    static uint32_t encoder_1_switch_debounce_timer = 0;
    static uint32_t encoder_2_switch_debounce_timer = 0;

    if( previous_encoder_1_position != encoder_1_position)
    {
        *rotary_encoder_timer = millis();
        previous_encoder_1_position = encoder_1_position;
        encoder_1_direction = (int)(encoder_1.getDirection());

        CONSOLE_CRLF("\r\nROTARY ENCODER 1 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_1_position)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_1_direction)

        if(state != STATE_BRIGHTNESS)
        {
            state = STATE_BRIGHTNESS;
        }    
        else
        {
            updateBrightness(encoder_1_direction);
        }
    }
   
    if(previous_encoder_2_position != encoder_2_position)
    {
        *rotary_encoder_timer = millis();
        previous_encoder_2_position = encoder_2_position;
        encoder_2_direction = (int)(encoder_2.getDirection());

        CONSOLE_CRLF("\r\nROTARY ENCODER 2 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_2_position)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_2_direction)

        if(state != STATE_COLOR)
        {
            state = STATE_COLOR;
        } 
        else
        {
            if(current_CPT == CPT_COLOR_TEMPERATURE)
            {
                updateColorTemperature(encoder_2_direction);
            }
            else if(current_CPT == CPT_COLOR_HUE)
            {
                updateColorHue(encoder_2_direction);
            }
        } 
    }

    if(encoder_1_switch == LOW && millis() - encoder_1_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
    {
        encoder_1_switch_debounce_timer = millis();
        *rotary_encoder_timer = millis();

        if(state == STATE_BRIGHTNESS)
        {
            // unused - reserved
        }
        else if(state == STATE_MAIN || state == STATE_COLOR)
        {
            state = STATE_BRIGHTNESS;
        }      
    }

    if(encoder_2_switch == LOW && millis() - encoder_2_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
    {
        encoder_2_switch_debounce_timer = millis();
        *rotary_encoder_timer = millis();

        if(state == STATE_COLOR)
        {
            current_CPT = (current_CPT == CPT_COLOR_TEMPERATURE) ? CPT_COLOR_HUE : CPT_COLOR_TEMPERATURE;  
            CONSOLE("\r\nCOLOR PICKER TYPE CHANGE: ")  
            CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])  
        }  
        else if(state == STATE_MAIN || state == STATE_BRIGHTNESS)
        {
            state = STATE_COLOR;
        }   
    }
}

void updatePreferences()
{
    if(current_CPT != (COLOR_PICKER_TYPE)preferences.getUChar("CPT"))
    {
        preferences.putUChar("CPT", (uint8_t)current_CPT);
    }

    if(currentColorHueIndex != preferences.getUInt("color-hue"))
    {
        preferences.putUInt("color-hue", currentColorHueIndex);
    }

    if(currentColorTemperatureIndex != preferences.getUInt("color-t"))
    {
        preferences.putUInt("color-t", currentColorTemperatureIndex);
    }

    if(currentBrightness != preferences.getUChar("brightness"))
    {
        preferences.putUChar("brightness", currentBrightness);
    }
}

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);
    CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);
    CONSOLE_CRLF("~~~ SETUP ~~~")

    loadDefaultValues();
    loadPreferences();
    setupDisplay();
    setupRotaryEncoders();
    setupSwitches();
    setup_LED_strip();
    CONSOLE_CRLF()

    state = STATE_MAIN;

    CONSOLE_CRLF("~~~ LOOP ~~~")
}

void loop() 
{
    static uint32_t mainScreenTimer = 0;
    static uint32_t rotary_encoder_timer = 0;

    // handle inputs
    checkSwitches();
    checkRotaryEncoders(&rotary_encoder_timer);

    if((state == STATE_BRIGHTNESS || state == STATE_COLOR) && millis() - rotary_encoder_timer > ANY_SETTING_SCREEN_TIMER_MS)
    {
        state = STATE_MAIN;
    }

    // handle state change
    if(state != previousState)
    {
        previousState = state;

        CONSOLE("\r\nSTATE CHANGE: ");
        CONSOLE_CRLF(stateString[(uint8_t)state])

        if(state == STATE_COLOR)
        {
            CONSOLE("  |-- color picker type: ")
            CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT]);
        }

        if(state == STATE_MAIN)
        {
            // in case there has been any changes to preferences
            updatePreferences();

            clearDisplay();
            mainScreenTimer = millis();
            updateMainScreen(true, 88, 88, 88, 4, 8888, 88.88, 3);   
        }
        else if(state == STATE_BRIGHTNESS)
        {
            clearDisplay();
            loadDisplayBrightness(currentBrightness);  
        }
        else if(state == STATE_COLOR) // handle transition from "state = STATE_MAIN" to "state == STATE_COLOR" 
        {
            clearDisplay();

            if(current_CPT == CPT_COLOR_TEMPERATURE)
            {
                loadDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
            }
            else if(current_CPT == CPT_COLOR_HUE)
            {
                loadDisplayColorHue(currentColorHueIndex, previousColorHueIndex);  
            }
        }
    }
    else if(current_CPT != previous_CPT && state == STATE_COLOR) // handle CPT change during state == STATE_COLOR
    {
        previous_CPT = current_CPT;

        CONSOLE("\r\nCPT CHANGE: ");
        CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])

        clearDisplay(); 

        if(current_CPT == CPT_COLOR_TEMPERATURE)
        {
            loadDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
            updateColorTemperature(0); // force color change without direction from encoder
        }
        else if(current_CPT == CPT_COLOR_HUE)
        {
            loadDisplayColorHue(currentColorHueIndex, previousColorHueIndex);
            updateColorHue(0); // force color change without direction from encoder  
        }                
    }

    // update screen once a second
    if(millis() - mainScreenTimer > MAIN_SCREEN_TIMER_MS && state == STATE_MAIN)
    {
        mainScreenTimer = millis();
        updateMainScreen(false, 88, 88, 88, 4, 8888, 88.88, 3);
    }
}