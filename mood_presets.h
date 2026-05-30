#ifndef MOOD_PRESETS_H
#define MOOD_PRESETS_H

#include <FastLED.h>

struct Mood {
  CRGB singleColor;

  CRGB multi1;
  CRGB multi2;
  CRGB multi3;
};

Mood focus = {
  CRGB(220,235,255),

  CRGB(220,235,255),
  CRGB(180,240,255),
  CRGB(140,190,255)
};

Mood cozy = {
  CRGB(255,170,90),

  CRGB(255,170,90),
  CRGB(255,130,70),
  CRGB(255,210,150)
};

Mood social = {
  CRGB(255,170,140),

  CRGB(255,170,140),
  CRGB(255,120,120),
  CRGB(255,210,100)
};

Mood goldenHour = {
  CRGB(255,140,70),

  CRGB(255,220,120),
  CRGB(255,150,70),
  CRGB(180,90,40)
};

Mood lateNight = {
  CRGB(70,60,160),

  CRGB(70,60,160),
  CRGB(30,40,120),
  CRGB(120,80,200)
};

Mood rainyDay = {
  CRGB(140,160,180),

  CRGB(100,130,180),
  CRGB(160,220,220),
  CRGB(180,180,200)
};

Mood creativeFlow = {
  CRGB(180,80,255),

  CRGB(180,80,255),
  CRGB(80,220,255),
  CRGB(255,80,180)
};

Mood romantic = {
  CRGB(255,120,180),

  CRGB(255,120,180),
  CRGB(255,80,100),
  CRGB(220,150,255)
};

#endif