/*
 * ============================================================================
 * FILE: lamp_control.h
 * ============================================================================
 * PURPOSE: Header file for LED lamp control module
 *          Declares functions and variables for controlling the RGB LED system
 * 
 * SCOPE: This header defines the interface between main.ino / web_api.cpp and
 *        the actual lamp control implementation (lamp_control.cpp)
 * 
 * KEY RESPONSIBILITIES:
 * - LED configuration (pins, number of LEDs, animation modes)
 * - Declare control functions (color, brightness, IR mode, animations)
 * - Declare global variables shared across modules
 * 
 * COMPILATION: This file is included in other .cpp files to access LED control
 *              functions without exposing implementation details
 * ============================================================================
 */

#ifndef LAMP_CONTROL_H
#define LAMP_CONTROL_H

#include <FastLED.h>  // Library for controlling addressable RGB LEDs (WS2812B)
#include <vector>     // C++ standard library for dynamic arrays

// ============================================================================
// HARDWARE PIN CONFIGURATION
// ============================================================================

#define LED_PIN 13     // GPIO 13: Connected to data line of WS2812B LED strip
#define NUM_LEDS 16    // Total number of addressable RGB LEDs in the strip
#define IR_PIN 27      // GPIO 27: Connected to IR motion sensor (HIGH when motion, LOW when idle)
#define BUTTON_PIN 26  // GPIO 26: Physical button for manual on/off control

// ============================================================================
// GLOBAL LED ARRAY - Shared memory for LED color data
// ============================================================================

// External declaration: Actual array defined in lamp_control.cpp
// Contains RGB color values for each of the 16 LEDs
// Type: CRGB = Compact RGB (8 bits each for Red, Green, Blue = 24 bits total per LED)
extern CRGB leds[NUM_LEDS];

// ============================================================================
// GLOBAL STATE VARIABLES - Control current lamp behavior
// ============================================================================

extern bool irMode;              // true = IR motion sensor mode, false = normal mode
extern bool lampEnabled;         // true = lamp is ON, false = lamp is OFF
extern bool multiMode;           // true = multi-color animation, false = single solid color
extern int irTriggeredValue;     // IR sensor value when motion detected (LOW or HIGH)

// ============================================================================
// FUNCTION DECLARATIONS - Core lamp control operations
// ============================================================================

/**
 * FUNCTION: setIrMode()
 * ==========================================
 * PURPOSE: Enable or disable IR motion sensor control
 * 
 * PARAMETERS:
 *   - enable: true to enable IR mode, false to disable
 * 
 * BEHAVIOR WHEN ENABLED:
 *   - Lamp automatically turns ON when motion is detected
 *   - Lamp automatically turns OFF when no motion for a period
 *   - Automatically detects if sensor uses HIGH or LOW for detection
 *   - Disables manual button control (IR sensor takes priority)
 * 
 * BEHAVIOR WHEN DISABLED:
 *   - Lamp returns to manual control (button or API commands)
 *   - IR sensor input ignored
 * 
 * EXAMPLE:
 *   setIrMode(true);  // Turn on IR motion detection
 *   setIrMode(false); // Turn off IR mode, use manual control
 * ==========================================
 */
void setIrMode(bool enable);

/**
 * FUNCTION: setBrightnessLevel()
 * ==========================================
 * PURPOSE: Set the overall brightness level of all LEDs
 * 
 * PARAMETERS:
 *   - value: Brightness level from 0 to 255
 *      0 = completely off (black)
 *      128 = 50% brightness
 *      255 = full brightness (default)
 * 
 * EFFECT: Applies to all LEDs uniformly, whether in single or multi-color mode
 * 
 * EXAMPLE:
 *   setBrightnessLevel(255); // Full brightness
 *   setBrightnessLevel(60);  // Low brightness
 *   setBrightnessLevel(30);  // Ultra-low for night mode
 * ==========================================
 */
void setBrightnessLevel(int value);

/**
 * FUNCTION: setupLamp()
 * ==========================================
 * PURPOSE: Initialize the LED system at startup
 * 
 * CALLED BY: setup() in main.ino during boot sequence
 * 
 * INITIALIZATION TASKS:
 * 1. Configure GPIO pins (button input, IR input, LED output)
 * 2. Initialize FastLED library with correct LED count and pin
 * 3. Set initial brightness level
 * 4. Set default color (warm golden light)
 * 5. Enable lamp in ready state
 * 
 * NO PARAMETERS - uses global #define values for configuration
 * ==========================================
 */
void setupLamp();

/**
 * FUNCTION: updateLamp()
 * ==========================================
 * PURPOSE: Update lamp state based on inputs and display animations
 * 
 * CALLED BY: loop() - runs continuously (many times per second)
 * 
 * OPERATIONS:
 * 1. Read button state and detect presses (with debouncing)
 * 2. Read IR sensor if in IR mode
 * 3. Update multi-color animations (blend between colors)
 * 4. Apply current color/animation to physical LEDs
 * 
 * BUTTON BEHAVIOR:
 *   - Press button to toggle lamp ON/OFF
 *   - Debouncing prevents accidental multiple presses
 * 
 * IR BEHAVIOR (when enabled):
 *   - Motion detected → lamp ON
 *   - No motion → lamp OFF
 * ==========================================
 */
void updateLamp();

/**
 * FUNCTION: setSingleColor()
 * ==========================================
 * PURPOSE: Set all LEDs to one solid color (static, no animation)
 * 
 * PARAMETERS:
 *   - color: CRGB color value (Red 0-255, Green 0-255, Blue 0-255)
 * 
 * MODE SWITCHING: Automatically disables multi-color animation mode
 * 
 * EXAMPLE COLORS:
 *   CRGB(255, 0, 0)     = Red
 *   CRGB(0, 255, 0)     = Green
 *   CRGB(0, 0, 255)     = Blue
 *   CRGB(255, 255, 0)   = Yellow
 *   CRGB(255, 165, 0)   = Orange (warm light)
 *   CRGB(255, 231, 186) = Warm white/golden (default)
 * 
 * USAGE:
 *   setSingleColor(CRGB(255, 100, 50)); // Set to warm orange
 * ==========================================
 */
void setSingleColor(CRGB color);

/**
 * FUNCTION: setMultiColors()
 * ==========================================
 * PURPOSE: Set up multi-color animation mode
 *          LEDs smoothly transition between colors in sequence
 * 
 * PARAMETERS:
 *   - colors: std::vector of CRGB colors to cycle through
 * 
 * MODE SWITCHING: Enables multi-color animation mode
 * 
 * ANIMATION BEHAVIOR:
 *   - Smooth blend from color 1 → color 2 → color 3 → ... → color 1 (repeat)
 *   - Speed adjustable via setTransitionDelay()
 *   - Each transition takes ~3-400ms (depending on transitionDelayMs)
 * 
 * EXAMPLE:
 *   std::vector<CRGB> colors = {
 *     CRGB(255, 0, 0),    // Red
 *     CRGB(0, 255, 0),    // Green
 *     CRGB(0, 0, 255)     // Blue
 *   };
 *   setMultiColors(colors);  // Cycle through RGB
 * ==========================================
 */
void setMultiColors(const std::vector<CRGB>& colors);

/**
 * FUNCTION: turnLampOff()
 * ==========================================
 * PURPOSE: Turn off all LEDs (set to black/no color)
 * 
 * EFFECT: Sets all 16 LEDs to black and updates physical display
 * 
 * USAGE: Called when lamp is disabled or in sleep mode
 * ==========================================
 */
void turnLampOff();

// ============================================================================
// ANIMATION CONTROL VARIABLES
// ============================================================================

extern unsigned int transitionDelayMs;  // Delay (ms) between animation frames
                                        // Lower = faster animation, Higher = slower

// ============================================================================
// ANIMATION CONTROL FUNCTIONS
// ============================================================================

/**
 * FUNCTION: setTransitionDelay()
 * ==========================================
 * PURPOSE: Set the speed of multi-color animations
 * 
 * PARAMETERS:
 *   - ms: Milliseconds delay between animation frames
 *         Range: 3ms to 1000ms (clamped by function)
 * 
 * EFFECT ON ANIMATION:
 *   - 3ms   = Very fast (rapid color transitions)
 *   - 15ms  = Default (smooth but brisk)
 *   - 50ms  = Slower (more leisurely transitions)
 *   - 100ms = Very slow (glacial color changes)
 * 
 * EXAMPLE:
 *   setTransitionDelay(15);   // Fast animation
 *   setTransitionDelay(100);  // Slow, relaxing animation
 * ==========================================
 */
void setTransitionDelay(unsigned int ms);

#endif
