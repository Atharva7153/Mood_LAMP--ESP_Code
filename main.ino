#include <WiFi.h>
#include <WebServer.h>

#include "wifi_config.h"
#include "lamp_control.h"
#include "web_api.h"

WebServer server(80);

const char* wifiStatusToString(wl_status_t s) {
  switch(s) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "WL_UNKNOWN";
  }
}

void scanNearbyNetworks() {
  Serial.println("\n    Scanning nearby WiFi networks...");
  
  int numNetworks = WiFi.scanNetworks();
  
  Serial.print("    Found ");
  Serial.print(numNetworks);
  Serial.println(" network(s):\n");
  
  if (numNetworks > 0) {
    for (int i = 0; i < numNetworks; i++) {
      Serial.print("    [");
      Serial.print(i + 1);
      Serial.print("] ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm)");
      
      // Check if this is our target network
      if (strcmp(WiFi.SSID(i).c_str(), WIFI_SSID) == 0) {
        Serial.print(" ← TARGET NETWORK");
      }
      Serial.println("");
    }
  } else {
    Serial.println("    No networks found!");
  }
  Serial.println("");
}

// WiFi Connection Visual Feedback - Soft cyan breathing animation
void breatheConnecting(unsigned long elapsedMs) {
  CRGB connectingColor = CRGB(100, 220, 255); // Bright Cyan
  unsigned long breathePeriod = 1600; // 1.6s for smooth breathing
  unsigned long cycleMs = elapsedMs % breathePeriod;
  
  // Smooth sine wave breathing: goes from 50 to 255 brightness
  float progress = (float)cycleMs / breathePeriod; // 0.0 to 1.0
  float sine = sin(progress * 2 * 3.14159); // -1 to 1
  uint8_t brightness = map((int)((sine + 1) * 127.5), 0, 255, 80, 255);
  
  CRGB breathed = connectingColor;
  breathed.fadeLightBy(255 - brightness);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = breathed;
  }
  FastLED.show();
}

// WiFi Connection Success Visual Feedback - Bright green fade out
void fadeOutConnected() {
  CRGB connectedColor = CRGB(100, 255, 150); // Bright Green
  unsigned long fadeDuration = 1500; // 1.5 seconds fade
  unsigned long fadeStart = millis();
  
  while (millis() - fadeStart < fadeDuration) {
    unsigned long elapsed = millis() - fadeStart;
    uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
    
    CRGB faded = connectedColor;
    faded.fadeLightBy(255 - brightness);
    
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = faded;
    }
    FastLED.show();
    delay(20);
  }
}


void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.println("\n\n================================");
  Serial.println("  MOOD LAMP - STARTUP SEQUENCE");
  Serial.println("================================\n");

  Serial.println("[1] Initializing LED System...");
  setupLamp();
  Serial.println("    ✓ LED System initialized\n");

  // Start breathing animation immediately - WiFi connecting visual
  unsigned long wifiStartTime = millis();

  Serial.println("[2] WiFi Configuration:");
  Serial.print("    Target SSID: ");
  Serial.println(WIFI_SSID);
  
  // Scan for nearby networks
  scanNearbyNetworks();
  
  Serial.println("    Attempting connection...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  const unsigned long timeout = 20000; // 20s

  // Show soft cyan breathing while connecting to WiFi
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
    unsigned long elapsedMs = millis() - wifiStartTime;
    breatheConnecting(elapsedMs);
    delay(50); // Small delay for breathing animation
    Serial.print(".");
  }

  Serial.println("\n");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("    ✗ CONNECTION FAILED - Status: ");
    Serial.print(WiFi.status());
    Serial.print(" (");
    Serial.print(wifiStatusToString(WiFi.status()));
    Serial.println(")");
    // Restore default light on connection failure
    setSingleColor(CRGB(255, 231, 186));
  } else {
    Serial.println("    ✓ Successfully Connected!");
    Serial.print("    Connected SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("    IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("    Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("    Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Show bright green fade-out animation on successful connection
    fadeOutConnected();
    
    // Restore default light after fade-out
    setSingleColor(CRGB(255, 231, 186));
  }

  Serial.println("\n[3] Setting up API Routes...");
  setupRoutes();
  Serial.println("    ✓ Routes configured\n");

  Serial.println("[4] Starting Web Server...");
  server.begin();
  Serial.println("    ✓ Server running on port 80\n");

  Serial.println("================================");
  Serial.println("  SYSTEM READY - ACCEPTING REQUESTS");
  Serial.println("================================\n");
}

void loop() {

  server.handleClient();

  updateLamp();
}