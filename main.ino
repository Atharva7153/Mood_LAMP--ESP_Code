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

void setup() {

  Serial.begin(115200);

  setupLamp();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting");

  unsigned long start = millis();
  const unsigned long timeout = 20000; // 20s

  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Failed to connect, status: ");
    Serial.print(WiFi.status());
    Serial.print(" (");
    Serial.print(wifiStatusToString(WiFi.status()));
    Serial.println(")");
  } else {
    Serial.println("WiFi Connected");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }

  setupRoutes();

  server.begin();

  Serial.println("Server Started");
}

void loop() {

  server.handleClient();

  updateLamp();
}