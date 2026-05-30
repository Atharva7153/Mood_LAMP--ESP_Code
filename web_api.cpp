#include <WebServer.h>
#include <ArduinoJson.h>

#include "lamp_control.h"
#include "mood_presets.h"
#include <vector>

extern WebServer server;

static void sendTextResponse(int code, const char* msg) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "text/plain", msg);
}

void setupRoutes() {

  // CORS preflight for color endpoints
  server.on("/color/single", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  server.on("/color/multi", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  // CORS preflight for IR toggle endpoint
  server.on("/ir/toggle", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  // POST single color as JSON { r,g,b }
  server.on("/color/single", HTTP_POST, []() {
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    int r = doc["r"] | 0;
    int g = doc["g"] | 0;
    int b = doc["b"] | 0;
    setSingleColor(CRGB(r, g, b));
    sendTextResponse(200, "OK");
  });

  // POST multi colors as JSON { colors: [{r,g,b}, ...] }
  server.on("/color/multi", HTTP_POST, []() {
    String body = server.arg("plain");
    StaticJsonDocument<400> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    JsonArray arr = doc["colors"].as<JsonArray>();
    if (!arr || arr.size() == 0) {
      sendTextResponse(400, "No colors provided");
      return;
    }
    std::vector<CRGB> colors;
    for (JsonVariant v : arr) {
      int r = v["r"] | 0;
      int g = v["g"] | 0;
      int b = v["b"] | 0;
      colors.push_back(CRGB(r, g, b));
    }
    setMultiColors(colors);
    sendTextResponse(200, "OK");
  });

  server.on("/color/speed", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  // POST transition speed as JSON { ms: <number> } (milliseconds per frame)
  server.on("/color/speed", HTTP_POST, []() {
    String body = server.arg("plain");
    StaticJsonDocument<100> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    unsigned int ms = 0;
    if (doc.containsKey("ms")) ms = doc["ms"].as<unsigned int>();
    else if (doc.containsKey("speed")) ms = doc["speed"].as<unsigned int>();
    if (ms == 0) {
      sendTextResponse(400, "Invalid speed value");
      return;
    }
    setTransitionDelay(ms);
    sendTextResponse(200, "OK");
  });


  server.on("/ir/on", []() {
    setIrMode(true);
    sendTextResponse(200, "IR Mode ON");
  });

  server.on("/ir/off", []() {
    setIrMode(false);
    sendTextResponse(200, "IR Mode OFF");
  });

  // POST IR toggle with JSON { enabled: true/false }
  server.on("/ir/toggle", HTTP_POST, []() {
    String body = server.arg("plain");
    StaticJsonDocument<100> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    bool enabled = doc["enabled"] | false;
    irMode = enabled;
    if (!enabled) {
      lampEnabled = true;
    }
    String response = enabled ? "IR Mode ON" : "IR Mode OFF";
    sendTextResponse(200, response.c_str());
  });

  server.on("/focus/single", []() {

    setSingleColor(focus.singleColor);

    sendTextResponse(200, "Focus Single");
  });

  server.on("/focus/multi", []() {

    setMultiColors(std::vector<CRGB>{ focus.multi1, focus.multi2, focus.multi3 });

    sendTextResponse(200, "Focus Multi");
  });

  server.on("/cozy/single", []() {

    setSingleColor(cozy.singleColor);

    sendTextResponse(200, "Cozy Single");
  });

  server.on("/cozy/multi", []() {

    setMultiColors(std::vector<CRGB>{ cozy.multi1, cozy.multi2, cozy.multi3 });

    sendTextResponse(200, "Cozy Multi");
  });

  server.on("/social/single", []() {

    setSingleColor(social.singleColor);

    sendTextResponse(200, "Social Single");
  });

  server.on("/social/multi", []() {

    setMultiColors(std::vector<CRGB>{ social.multi1, social.multi2, social.multi3 });

    sendTextResponse(200, "Social Multi");
  });

  server.on("/golden/single", []() {

    setSingleColor(goldenHour.singleColor);

    sendTextResponse(200, "Golden Hour Single");
  });

  server.on("/golden/multi", []() {

    setMultiColors(std::vector<CRGB>{ goldenHour.multi1, goldenHour.multi2, goldenHour.multi3 });

    sendTextResponse(200, "Golden Hour Multi");
  });

  server.on("/latenight/single", []() {

    setSingleColor(lateNight.singleColor);

    sendTextResponse(200, "Late Night Single");
  });

  server.on("/latenight/multi", []() {

    setMultiColors(std::vector<CRGB>{ lateNight.multi1, lateNight.multi2, lateNight.multi3 });

    sendTextResponse(200, "Late Night Multi");
  });

  server.on("/rainy/single", []() {

    setSingleColor(rainyDay.singleColor);

    sendTextResponse(200, "Rainy Day Single");
  });

  server.on("/rainy/multi", []() {

    setMultiColors(std::vector<CRGB>{ rainyDay.multi1, rainyDay.multi2, rainyDay.multi3 });

    sendTextResponse(200, "Rainy Day Multi");
  });

  server.on("/creative/single", []() {

    setSingleColor(creativeFlow.singleColor);

    sendTextResponse(200, "Creative Flow Single");
  });

  server.on("/creative/multi", []() {

    setMultiColors(std::vector<CRGB>{ creativeFlow.multi1, creativeFlow.multi2, creativeFlow.multi3 });

    sendTextResponse(200, "Creative Flow Multi");
  });

  server.on("/romantic/single", []() {

    setSingleColor(romantic.singleColor);

    sendTextResponse(200, "Romantic Single");
  });

  server.on("/romantic/multi", []() {

    setMultiColors(std::vector<CRGB>{ romantic.multi1, romantic.multi2, romantic.multi3 });

    sendTextResponse(200, "Romantic Multi");
  });
  server.on("/brightness/low", []() {

  setBrightnessLevel(60);

  sendTextResponse(200, "Low Brightness");
});

server.on("/brightness/medium", []() {

  setBrightnessLevel(120);

  sendTextResponse(200, "Medium Brightness");
});

server.on("/brightness/high", []() {

  setBrightnessLevel(255);

  sendTextResponse(200, "High Brightness");
});
}