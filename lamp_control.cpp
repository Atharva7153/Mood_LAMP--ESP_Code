/*
 * ============================================================================
 * FILE: lamp_control.cpp
 * ============================================================================
 * PURPOSE: Implementation of LED lamp control functions
 *          Contains all the actual code for controlling 16 addressable RGB LEDs
 * 
 * KEY FEATURES:
 * - Single solid color mode
 * - Multi-color smooth animation mode
 * - Button press detection with debouncing
 * - IR motion sensor support
 * - Brightness control (0-255)
 * - Animation speed adjustment
 * 
 * HARDWARE:
 *   - 16 WS2812B addressable RGB LEDs
 *   - Connected to GPIO 13 for data signal
 *   - Button on GPIO 26 (pull-up resistor)
 *   - IR motion sensor on GPIO 27 (pull-up resistor)
 * ============================================================================
 */

#include "lamp_control.h"

// ============================================================================
// GLOBAL VARIABLES - State and data storage
// ============================================================================

CRGB leds[NUM_LEDS];              // Array to hold RGB color data for all 16 LEDs

// State variables - track current lamp configuration
bool irMode = false;              // true = IR sensor controls lamp, false = manual control
bool lampEnabled = true;          // true = lamp is on, false = lamp is off
bool multiMode = false;           // true = cycling through multiple colors, false = single color

// Button state tracking for debouncing
bool lastButtonState = HIGH;      // Button starts unpressed (HIGH with pull-up resistor)
unsigned long lastButtonPressTime = 0;
const unsigned long DEBOUNCE_TIME = 100; // Minimum 100ms between valid button presses
bool buttonInitialized = false;   // Flag to know when button setup is complete

// Default warm golden color when lamp first turns on
CRGB currentSingle = CRGB(255, 231, 186);  // Warm golden-white

// LED brightness (0-255): 255 = full brightness, 0 = off
uint8_t brightness = 255;

#include <vector>

// Store multiple colors for animation mode
std::vector<CRGB> multiColorsVec;

// IR sensor detection value: either HIGH or LOW when object is present
// Auto-detected by setIrMode() function
int irTriggeredValue = LOW;

// Animation speed: milliseconds to wait between color transition frames
unsigned int transitionDelayMs = 15;  // Default: 15ms (smooth, reasonably fast)

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/*
 * FUNCTION: applyCurrentColors()
 * ==========================================
 * PURPOSE: Apply currently configured colors to all LEDs
 *          Handles both single and multi-color modes
 * 
 * OPERATIONS:
 *   - If lamp disabled: turn off all LEDs
 *   - If multi-color mode: show first color (animation continues in loop)
 *   - If single color mode: show solid color on all LEDs
 * 
 * USAGE: Called when mode changes or lamp turns on
 * ==========================================
 */
void applyCurrentColors() {
  if (!lampEnabled) return;  // If lamp off, don't apply colors
  
  if (multiMode) {
    // Multi-color mode: display first color immediately
    if (!multiColorsVec.empty()) {  // If color list not empty
      CRGB c = multiColorsVec[0];   // Get first color
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = c;  // Set all LEDs to first color
      }
      FastLED.show();  // Update physical LEDs
    }
  } else {
    // Single color mode: show solid color on all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = currentSingle;  // Set all LEDs to current single color
    }
    FastLED.show();  // Update physical LEDs
  }
}



/*
 * FUNCTION: setIrMode()
 * ==========================================
 * PURPOSE: Enable or disable IR motion sensor mode
 *          Auto-detects the baseline sensor value and inverts it for triggered state
 * 
 * WHEN ENABLED (enable = true):
 *   - Samples IR sensor 20 times to establish baseline (idle state)
 *   - Detects if motion = HIGH or motion = LOW (sensor dependent)
 *   - Lamp starts disabled until motion is detected
 *   - LEDs turn off initially
 * 
 * WHEN DISABLED (enable = false):
 *   - IR sensor input ignored
 *   - Lamp returns to manual control (button or API)
 *   - Lamp turns back on with current color
 * 
 * CALIBRATION LOGIC:
 *   - Takes 20 sensor readings (5 with 5ms delays = 100ms total)
 *   - Counts HIGH readings
 *   - If more than 10 HIGH: baseline is HIGH, triggered = LOW
 *   - If more than 10 LOW: baseline is LOW, triggered = HIGH
 *   - Prints result to Serial Monitor for debugging
 * 
 * EXAMPLE OUTPUT:
 *   "IR baseline=LOW => triggered=HIGH"
 *   (This means: baseline is LOW when no motion, becomes HIGH when motion detected)
 * ==========================================
 */
void setIrMode(bool enable) {
  irMode = enable;
  if (enable) {
    // Calibration: detect baseline sensor reading
    const int samples = 20;  // Take 20 samples
    int highCount = 0;       // Counter for HIGH readings
    
    // Sample the sensor 20 times
    for (int i = 0; i < samples; ++i) {
      int v = digitalRead(IR_PIN);  // Read sensor value
      if (v == HIGH) highCount++;    // Count HIGH readings
      delay(5);                      // 5ms between samples
    }
    
    // Determine baseline: if >50% samples were HIGH, baseline is HIGH
    int baseline = (highCount > (samples/2)) ? HIGH : LOW;
    
    // Triggered state is OPPOSITE of baseline
    irTriggeredValue = (baseline == HIGH) ? LOW : HIGH;
    
    // Debug output to Serial Monitor
    Serial.print("IR baseline=");
    Serial.print(baseline == HIGH ? "HIGH" : "LOW");
    Serial.print(" => triggered=");
    Serial.println(irTriggeredValue == HIGH ? "HIGH" : "LOW");
    
    // Start with lamp disabled until motion detected
    lampEnabled = false;
    turnLampOff();  // Turn off all LEDs
  } else {
    // Disable IR mode: restore normal lamp operation
    lampEnabled = true;
    applyCurrentColors();  // Show current color settings
  }
}

/*
 * FUNCTION: setBrightnessLevel()
 * ==========================================
 * PURPOSE: Set the brightness of all LEDs (0-255 scale)
 * 
 * PARAMETERS:
 *   - value: Brightness level (0 = off, 255 = full brightness)
 * 
 * EFFECT: Applies to entire LED strip uniformly
 *         Doesn't change color, only intensity
 * 
 * USAGE EXAMPLES:
 *   setBrightnessLevel(255); // Full brightness (daylight)
 *   setBrightnessLevel(120); // Medium (reading)
 *   setBrightnessLevel(60);  // Low (evening)
 *   setBrightnessLevel(30);  // Very low (night)
 * ==========================================
 */
void setBrightnessLevel(int value) {
  brightness = value;              // Store new brightness value
  FastLED.setBrightness(brightness); // Apply to all LEDs
  FastLED.show();                  // Update physical LEDs
}

/*
 * FUNCTION: setupLamp()
 * ==========================================
 * PURPOSE: Initialize all lamp hardware at boot
 * 
 * INITIALIZATION STEPS:
 * 1. Configure GPIO pins:
 *    - Button pin (INPUT_PULLUP): detects button presses
 *    - IR pin (INPUT_PULLUP): reads motion sensor
 * 2. Initialize FastLED library:
 *    - Configures LED strip on pin 13
 *    - Sets number of LEDs to 16
 *    - Sets LED brightness to default (255)
 * 3. Initialize button state for debouncing
 * 4. Turn on lamp with default warm color
 * 5. Print status to Serial Monitor
 * 
 * CALLED BY: setup() in main.ino
 * ==========================================
 */
void setupLamp() {
  // Configure GPIO pins as inputs with internal pull-up resistors
  // Pull-up means: HIGH when nothing pressed, LOW when button/sensor activated
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Physical button
  pinMode(IR_PIN, INPUT_PULLUP);       // IR motion sensor

  // Initialize FastLED library
  // NEOPIXEL = WS2812B LED type, LED_PIN = GPIO 13, leds = array, NUM_LEDS = 16
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  
  // Set default brightness
  FastLED.setBrightness(brightness);

  // Initialize button state from current reading (used for edge detection)
  lastButtonState = digitalRead(BUTTON_PIN);
  buttonInitialized = true;  // Mark button as ready
  
  // Ensure lamp is ON and showing current color on startup
  lampEnabled = true;
  setSingleColor(currentSingle);
  
  Serial.println("    ✓ Lamp initialized and turned ON");
  Serial.println("    ✓ Button setup complete (Pin 26)");
}

/*
 * FUNCTION: setSingleColor()
 * ==========================================
 * PURPOSE: Set all LEDs to one solid color (no animation)
 * 
 * PARAMETERS:
 *   - color: CRGB value (Red, Green, Blue 0-255 each)
 * 
 * OPERATIONS:
 * 1. Disable multi-color animation mode
 * 2. Store this color as current single color
 * 3. Set all 16 LEDs to this color
 * 4. Update physical LEDs
 * 
 * EXAMPLES:
 *   setSingleColor(CRGB(255, 0, 0));     // Red
 *   setSingleColor(CRGB(255, 231, 186)); // Warm golden (default)
 *   setSingleColor(CRGB(100, 220, 255)); // Cyan (cool blue)
 * ==========================================
 */
void setSingleColor(CRGB color) {
  multiMode = false;               // Disable multi-color mode
  currentSingle = color;           // Store as current single color
  
  // Set all LEDs to this color
  for(int i=0; i<NUM_LEDS; i++) {
    leds[i] = color;
  }
  
  FastLED.show();  // Update physical LEDs to show the color
}

/*
 * FUNCTION: setMultiColors()
 * ==========================================
 * PURPOSE: Set up multi-color animation mode
 *          LEDs smoothly transition between colors
 * 
 * PARAMETERS:
 *   - colors: std::vector of CRGB colors to cycle through
 * 
 * OPERATIONS:
 * 1. Enable multi-color mode
 * 2. Store color list
 * 3. Reset animation state (blend amount and target)
 * 4. Animation will start on next updateLamp() call
 * 
 * ANIMATION FLOW:
 *   - Blends from color[0] → color[1] → ... → color[n] → color[0] (repeat)
 *   - Each blend takes ~255 * transitionDelayMs milliseconds
 *   - With default 15ms: each color transition ~3.8 seconds
 * 
 * EXAMPLE:
 *   std::vector<CRGB> relaxing = {
 *     CRGB(255, 170, 90),   // Orange
 *     CRGB(255, 130, 70),   // Dark orange
 *     CRGB(255, 210, 150)   // Light orange
 *   };
 *   setMultiColors(relaxing);  // Cycle through warm tones
 * ==========================================
 */
void setMultiColors(const std::vector<CRGB>& colors) {
  multiMode = !colors.empty();  // Enable mode only if colors provided
  multiColorsVec = colors;      // Store the color list
  
  // Reset animation state
  extern unsigned long lastChange;
  lastChange = millis();  // Reset animation timer
}



// ============================================================================
// ANIMATION STATE VARIABLES - Track multi-color animation progress
// ============================================================================

static uint8_t blendAmount = 0;      // How far through the current blend (0-255)
static int currentTarget = 0;        // Index of target color we're blending to

/*
 * FUNCTION: animateMulti()
 * ==========================================
 * PURPOSE: Perform one frame of multi-color animation
 *          Smoothly blends from one color to the next
 * 
 * ANIMATION LOGIC:
 * 1. Get current color and next color from color list
 * 2. Blend between them using blendAmount (0-255)
 * 3. Display blended color on all LEDs
 * 4. Increment blendAmount
 * 5. When blendAmount reaches 255, move to next color pair
 * 6. Cycle back to first color when reaching end
 * 7. Delay between frames creates visible animation speed
 * 
 * BLEND CALCULATION:
 *   - blendAmount = 0: show 100% startColor, 0% endColor
 *   - blendAmount = 128: show 50% startColor, 50% endColor
 *   - blendAmount = 255: show 0% startColor, 100% endColor
 * 
 * EXAMPLE:
 *   Color list: [Red, Green, Blue]
 *   Sequence: Red→Red-Green mix→Green→Green-Blue mix→Blue→Blue-Red mix→(repeat)
 * 
 * PERFORMANCE:
 *   - Default transitionDelayMs=15ms means 17 frames per second
 *   - With 256 blend steps: ~15 seconds per color transition
 * ==========================================
 */
void animateMulti() {
  if (multiColorsVec.empty()) return;  // Exit if no colors configured

  // Get current and next colors in the cycle
  CRGB startColor = multiColorsVec[currentTarget % multiColorsVec.size()];
  CRGB endColor = multiColorsVec[(currentTarget + 1) % multiColorsVec.size()];

  // Blend between start and end colors based on blendAmount
  CRGB blended = blend(startColor, endColor, blendAmount);

  // Apply blended color to all LEDs
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = blended;
  }

  FastLED.show();  // Update physical LEDs

  blendAmount++;   // Increment blend progress (0→255)

  // When blend is complete (reached 255), move to next color pair
  if(blendAmount >= 255) {
    blendAmount = 0;  // Reset blend for next transition
    currentTarget = (currentTarget + 1) % multiColorsVec.size();  // Next color
  }

  // Delay between frames controls animation speed
  delay(transitionDelayMs);
}

/*
 * FUNCTION: setTransitionDelay()
 * ==========================================
 * PURPOSE: Adjust the speed of multi-color animations
 * 
 * PARAMETERS:
 *   - ms: Milliseconds delay between animation frames
 * 
 * CLAMPING LIMITS:
 *   - Minimum: 3ms (very fast, hardware limit)
 *   - Maximum: 1000ms (1 second, glacially slow)
 *   - Values outside range are clamped to these limits
 * 
 * EFFECT ON ANIMATION:
 *   - 3ms:    ~340 frames/sec (not visible, too fast)
 *   - 15ms:   ~67 frames/sec (default, smooth)
 *   - 50ms:   ~20 frames/sec (noticeably slower)
 *   - 100ms:  ~10 frames/sec (leisurely)
 *   - 1000ms: ~1 frame/sec (extremely slow)
 * 
 * RELATIONSHIP TO TOTAL COLOR TIME:
 *   - Total time per color = 256 blend steps × transitionDelayMs
 *   - At 15ms: 256 × 15 = 3840ms (3.8 seconds per color)
 *   - At 50ms: 256 × 50 = 12800ms (12.8 seconds per color)
 * ==========================================
 */
void setTransitionDelay(unsigned int ms) {
  // Clamp to reasonable bounds
  if (ms < 3) ms = 3;        // Minimum 3ms (hardware realistic)
  if (ms > 1000) ms = 1000;  // Maximum 1000ms (1 second)
  transitionDelayMs = ms;    // Store the value
}

/*
 * FUNCTION: turnLampOff()
 * ==========================================
 * PURPOSE: Turn off all LEDs (set to black)
 * 
 * OPERATIONS:
 * 1. Set all 16 LEDs to black (0,0,0)
 * 2. Update physical LED strip
 * 
 * USAGE:
 *   - Called when lamp disabled
 *   - Called when user presses button to turn off
 *   - Called during IR mode when no motion detected
 * ==========================================
 */
void turnLampOff() {
  // Set all LEDs to black (no color)
  for(int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::Black;  // Black = (0, 0, 0)
  }
  
  FastLED.show();  // Update physical LEDs
}

/*
 * FUNCTION: updateLamp()
 * ==========================================
 * PURPOSE: Main update function called every frame
 *          Handles input sensing and animation display
 * 
 * OPERATIONS IN ORDER:
 * 1. Check for physical button presses (with debouncing)
 * 2. Check IR sensor input (if IR mode enabled)
 * 3. Turn lamp on/off based on inputs
 * 4. Display animations:
 *    - Multi-color mode: show smooth color transitions
 *    - Single-color mode: show static color
 * 5. Handle lamp enable/disable state transitions
 * 
 * BUTTON BEHAVIOR:
 *   - Detects HIGH→LOW transition (button press)
 *   - 100ms debounce prevents false triggers
 *   - Toggles lampEnabled on each valid press
 * 
 * IR SENSOR BEHAVIOR:
 *   - Continuous monitoring when irMode=true
 *   - Motion detected (matches irTriggeredValue): turn on
 *   - No motion (opposite): turn off
 *   - Manual button overridden by IR in IR mode
 * 
 * STATE MACHINE:
 *   - prevLampEnabled tracks last frame's state
 *   - On enable: restore colors, show animation
 *   - On disable: turn off all LEDs
 *   - While enabled: update animations continuously
 * ==========================================
 */
void updateLamp() {
  static bool prevLampEnabled = lampEnabled;  // Track previous state for edge detection

  // =========================================================================
  // SECTION 1: CHECK PHYSICAL BUTTON INPUT
  // =========================================================================
  if (buttonInitialized) {
    int currentButtonState = digitalRead(BUTTON_PIN);  // Read button GPIO
    
    // Detect falling edge: HIGH → LOW transition (button pressed)
    if (currentButtonState == LOW && lastButtonState == HIGH) {
      unsigned long currentTime = millis();
      
      // Debounce: only accept press if >100ms since last valid press
      if (currentTime - lastButtonPressTime > DEBOUNCE_TIME) {
        lampEnabled = !lampEnabled;  // Toggle lamp on/off
        lastButtonPressTime = currentTime;
        
        // Debug output
        Serial.print("    [BUTTON] Lamp toggled ");
        Serial.println(lampEnabled ? "ON" : "OFF");
      }
    }
    
    lastButtonState = currentButtonState;  // Save state for next frame
  }

  // =========================================================================
  // SECTION 2: CHECK IR MOTION SENSOR INPUT
  // =========================================================================
  if (irMode) {
    int sensor = digitalRead(IR_PIN);  // Read sensor value
    
    // If sensor reads triggered value: motion detected → turn on
    if (sensor == irTriggeredValue) {
      lampEnabled = true;
    } else {
      // No motion detected → turn off
      lampEnabled = false;
    }
  }

  // =========================================================================
  // SECTION 3: HANDLE LAMP BEING TURNED OFF
  // =========================================================================
  if (!lampEnabled) {
    // If just turned off (state changed from on to off)
    if (prevLampEnabled != lampEnabled) {
      turnLampOff();  // Turn off all LEDs
    }
    prevLampEnabled = lampEnabled;
    return;  // Exit function, don't process animations while off
  }

  // =========================================================================
  // SECTION 4: LAMP IS ENABLED - HANDLE STATE TRANSITIONS AND ANIMATIONS
  // =========================================================================
  
  // If just turned on (state changed from off to on)
  if (prevLampEnabled != lampEnabled) {
    applyCurrentColors();  // Restore previous color settings
  }

  prevLampEnabled = lampEnabled;  // Update state for next frame

  // =========================================================================
  // SECTION 5: DISPLAY ANIMATIONS
  // =========================================================================
  if (multiMode) {
    // Multi-color mode: smoothly animate between colors
    animateMulti();
  }
  // Single-color mode: color stays static, no animation needed
}