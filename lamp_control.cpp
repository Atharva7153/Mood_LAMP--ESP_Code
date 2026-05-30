#include "lamp_control.h"

CRGB leds[NUM_LEDS];

bool irMode = false;
bool lampEnabled = true;
bool multiMode = false;

// default warm golden color when lamp turns on
CRGB currentSingle = CRGB(255, 231, 186);
uint8_t brightness = 120;

#include <vector>

std::vector<CRGB> multiColorsVec;

int irTriggeredValue = LOW;

// transition delay used by animateMulti (ms)
unsigned int transitionDelayMs = 15;

void applyCurrentColors() {
  if (!lampEnabled) return;
  if (multiMode) {
    // show first multi color as an immediate state, animation continues in loop
    if (!multiColorsVec.empty()) {
      CRGB c = multiColorsVec[0];
      for (int i = 0; i < NUM_LEDS; i++) leds[i] = c;
      FastLED.show();
    }
  } else {
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = currentSingle;
    FastLED.show();
  }
}

void setIrMode(bool enable) {
  irMode = enable;
  if (enable) {
    // detect baseline and assume triggered state is opposite
    const int samples = 20;
    int highCount = 0;
    for (int i = 0; i < samples; ++i) {
      int v = digitalRead(IR_PIN);
      if (v == HIGH) highCount++;
      delay(5);
    }
    int baseline = (highCount > (samples/2)) ? HIGH : LOW;
    irTriggeredValue = (baseline == HIGH) ? LOW : HIGH;
    Serial.print("IR baseline=");
    Serial.print(baseline == HIGH ? "HIGH" : "LOW");
    Serial.print(" => triggered=");
    Serial.println(irTriggeredValue == HIGH ? "HIGH" : "LOW");
    // start with lamp disabled until sensor triggers
    lampEnabled = false;
    // ensure LEDs off initially
    turnLampOff();
  } else {
    // disable IR mode, restore lamp
    lampEnabled = true;
    applyCurrentColors();
  }
}

unsigned long lastChange = 0;
int colorIndex = 0;

void setBrightnessLevel(int value) {

  brightness = value;

  FastLED.setBrightness(brightness);

  FastLED.show();
}

void setupLamp() {

  pinMode(IR_PIN, INPUT_PULLUP);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);

  setSingleColor(currentSingle);
}

void setSingleColor(CRGB color) {

  multiMode = false;

  currentSingle = color;

  for(int i=0; i<NUM_LEDS; i++) {
    leds[i] = color;
  }

  FastLED.show();
}

void setMultiColors(const std::vector<CRGB>& colors) {

  multiMode = !colors.empty();

  multiColorsVec = colors;

  // reset animation state
  extern unsigned long lastChange;
  lastChange = millis();
}

static uint8_t blendAmount = 0;
static int currentTarget = 0;

void animateMulti() {

  if (multiColorsVec.empty()) return;

  CRGB startColor = multiColorsVec[currentTarget % multiColorsVec.size()];
  CRGB endColor = multiColorsVec[(currentTarget + 1) % multiColorsVec.size()];

  CRGB blended = blend(startColor, endColor, blendAmount);

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = blended;
  }

  FastLED.show();

  blendAmount++;

  if(blendAmount >= 255) {
    blendAmount = 0;
    currentTarget = (currentTarget + 1) % multiColorsVec.size();
  }

  // wait between frames; adjustable via setTransitionDelay
  delay(transitionDelayMs);
}

void setTransitionDelay(unsigned int ms) {
  // clamp to decent bounds
  if (ms < 3) ms = 3;
  if (ms > 1000) ms = 1000;
  transitionDelayMs = ms;
}

void turnLampOff() {

  for(int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  FastLED.show();
}

void updateLamp() {

  static bool prevLampEnabled = lampEnabled;

  if (irMode) {
    int sensor = digitalRead(IR_PIN);
    if (sensor == irTriggeredValue) {
      lampEnabled = true;
    } else {
      lampEnabled = false;
    }
  }

  if (!lampEnabled) {
    if (prevLampEnabled != lampEnabled) {
      // turning off now
      turnLampOff();
    }
    prevLampEnabled = lampEnabled;
    return;
  }

  // lampEnabled is true
  if (prevLampEnabled != lampEnabled) {
    // just turned on — restore colors
    applyCurrentColors();
  }

  prevLampEnabled = lampEnabled;

  if (multiMode) {
    animateMulti();
  }
}