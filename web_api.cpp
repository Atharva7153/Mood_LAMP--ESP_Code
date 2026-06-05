/*
 * ============================================================================
 * FILE: web_api.cpp
 * ============================================================================
 * PURPOSE: REST API implementation - handles all HTTP requests from web clients
 *          Processes color changes, preset activations, brightness control, etc.
 * 
 * ARCHITECTURE:
 *   - HTTP Server listens on port 80
 *   - Routes are registered as lambdas (inline functions)
 *   - Each route: parses JSON input → calls lamp control function → returns response
 * 
 * COMMUNICATION FORMAT:
 *   - Protocol: HTTP (standard web protocol)
 *   - Data format: JSON (JavaScript Object Notation)
 *   - Headers: Include CORS for cross-origin requests
 *   - Status codes: 200 (OK), 400 (Bad Request), 204 (No Content)
 * 
 * EXAMPLE REQUEST/RESPONSE:
 *   REQUEST: POST http://192.168.1.100/color/single
 *   BODY: {"r": 255, "g": 0, "b": 0}
 *   RESPONSE: 200 OK "OK"
 *   EFFECT: Lamp turns red
 * 
 * CORS (Cross-Origin Resource Sharing):
 *   - Allows web pages from any domain to control the lamp
 *   - Sends headers allowing: GET, POST, OPTIONS methods
 *   - Accepts Content-Type: application/json header
 * ============================================================================
 */

#include <WebServer.h>       // HTTP server library
#include <ArduinoJson.h>     // JSON parsing library

#include "lamp_control.h"    // LED control functions
#include "mood_presets.h"    // Predefined color moods
#include <vector>            // Dynamic arrays for color lists

extern WebServer server;     // Global server object from main.ino

// ============================================================================
// HELPER FUNCTION - Send HTTP responses with CORS headers
// ============================================================================

/**
 * FUNCTION: sendTextResponse()
 * ==========================================
 * PURPOSE: Send HTTP response with CORS headers enabled
 *          CORS allows web pages to access the API from any domain
 * 
 * PARAMETERS:
 *   - code: HTTP status code (200 = OK, 400 = Bad Request, etc.)
 *   - msg: Text message to send as response body
 * 
 * SENDS HEADERS:
 *   - Access-Control-Allow-Origin: * (allow all domains)
 *   - Access-Control-Allow-Methods: GET, POST, OPTIONS
 *   - Access-Control-Allow-Headers: Content-Type
 * 
 * EXAMPLE USAGE:
 *   sendTextResponse(200, "OK");           // Success
 *   sendTextResponse(400, "Invalid JSON"); // Error
 * 
 * CORS EXPLANATION:
 *   - Web browsers have same-origin policy (security feature)
 *   - CORS headers tell browser it's OK to access from other sites
 *   - Necessary for web UI to control lamp from any domain
 * ==========================================
 */
static void sendTextResponse(int code, const char* msg) {
  // Add CORS headers to allow cross-origin requests
  server.sendHeader("Access-Control-Allow-Origin", "*");          // Allow all domains
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");  // Methods allowed
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");      // Headers allowed
  
  // Send the HTTP response
  server.send(code, "text/plain", msg);
}

// ============================================================================
// FUNCTION: setupRoutes()
// ============================================================================

/**
 * FUNCTION: setupRoutes()
 * ==========================================
 * PURPOSE: Register all HTTP endpoints and configure their handlers
 *          Maps URLs to the functions that process requests
 * 
 * ROUTE CATEGORIES:
 * 1. Color API: /color/* - set colors via JSON
 * 2. Preset Moods: /[mood]/[mode] - activate mood lighting
 * 3. IR Control: /ir/* - enable/disable motion sensing
 * 4. Brightness: /brightness/* - set brightness levels
 * 
 * WHAT IS A ROUTE?
 *   - Maps a URL to a handler function
 *   - Handler processes request and sends response
 *   - Example: GET /color/single → executes handler code
 * 
 * LAMBDA FUNCTIONS:
 *   - Anonymous functions defined inline ([] { ... })
 *   - Execute when matching route receives a request
 *   - Can access variables from outer scope
 * 
 * HTTP METHODS:
 *   - GET: Retrieve data (no body sent)
 *   - POST: Send data (body contains JSON)
 *   - OPTIONS: Preflight request for CORS (browser sends automatically)
 * 
 * JSON PARSING:
 *   - ArduinoJson library parses JSON strings
 *   - Extracts parameters (r, g, b values, etc.)
 *   - Type: StaticJsonDocument<size> - fixed size buffer
 * ==========================================
 */
void setupRoutes() {

  // =========================================================================
  // SECTION 1: CORS PREFLIGHT REQUESTS (HTTP OPTIONS method)
  // =========================================================================
  // CORS preflight: Browser sends OPTIONS request before POST to check permissions
  // These handlers respond to say "yes, you can send POST requests"

  server.on("/color/single", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);  // 204 = No Content (preflight response)
  });

  server.on("/color/multi", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  server.on("/ir/toggle", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  // =========================================================================
  // SECTION 2: COLOR API - Set colors via JSON POST requests
  // =========================================================================

  /*
   * ENDPOINT: POST /color/single
   * ==========================================
   * PURPOSE: Set all LEDs to one solid color
   * 
   * REQUEST FORMAT:
   *   URL: http://192.168.1.100/color/single
   *   Method: POST
   *   Body: {"r": 255, "g": 100, "b": 50}
   *   Content-Type: application/json
   * 
   * PARAMETERS (in JSON body):
   *   - r: Red component (0-255)
   *   - g: Green component (0-255)
   *   - b: Blue component (0-255)
   * 
   * RESPONSE: 200 OK "OK" on success, 400 on JSON parse error
   * 
   * EXAMPLE REQUESTS (via curl):
   *   curl -X POST http://192.168.1.100/color/single -H "Content-Type: application/json" -d '{"r":255,"g":0,"b":0}'
   *   → Sets lamp to RED
   * ==========================================
   */
  server.on("/color/single", HTTP_POST, []() {
    // Get the request body (JSON string)
    String body = server.arg("plain");
    
    // Create JSON document with 200 byte buffer
    StaticJsonDocument<200> doc;
    
    // Parse JSON string into document
    DeserializationError err = deserializeJson(doc, body);
    
    // Check if parsing succeeded
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    
    // Extract RGB values from JSON (default to 0 if not provided)
    int r = doc["r"] | 0;  // | 0 means "use 0 if key doesn't exist"
    int g = doc["g"] | 0;
    int b = doc["b"] | 0;
    
    // Call lamp control function to set color
    setSingleColor(CRGB(r, g, b));
    
    // Send success response
    sendTextResponse(200, "OK");
  });

  /*
   * ENDPOINT: POST /color/multi
   * ==========================================
   * PURPOSE: Set up multi-color animation between multiple colors
   * 
   * REQUEST FORMAT:
   *   URL: http://192.168.1.100/color/multi
   *   Method: POST
   *   Body: {"colors": [{"r":255,"g":0,"b":0}, {"r":0,"g":255,"b":0}]}
   *   Content-Type: application/json
   * 
   * PARAMETERS (in JSON body):
   *   - colors: Array of color objects
   *   - Each color object has: r, g, b (0-255 each)
   *   - Minimum 1 color, up to any number
   * 
   * ANIMATION BEHAVIOR:
   *   - Smoothly transitions between each color
   *   - After last color, goes back to first color
   *   - Speed controlled by /color/speed endpoint
   * 
   * EXAMPLE REQUEST (via curl):
   *   curl -X POST http://192.168.1.100/color/multi -H "Content-Type: application/json" -d '{"colors":[{"r":255,"g":0,"b":0},{"r":0,"g":255,"b":0},{"r":0,"g":0,"b":255}]}'
   *   → Animates through Red → Green → Blue → repeat
   * ==========================================
   */
  server.on("/color/multi", HTTP_POST, []() {
    // Get the request body (JSON string)
    String body = server.arg("plain");
    
    // Create JSON document with 400 byte buffer (larger for color array)
    StaticJsonDocument<400> doc;
    
    // Parse JSON string into document
    DeserializationError err = deserializeJson(doc, body);
    
    // Check if parsing succeeded
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    
    // Extract colors array from JSON
    JsonArray arr = doc["colors"].as<JsonArray>();
    
    // Validate that colors array exists and is not empty
    if (!arr || arr.size() == 0) {
      sendTextResponse(400, "No colors provided");
      return;
    }
    
    // Create vector to store colors
    std::vector<CRGB> colors;
    
    // Loop through each color in the array
    for (JsonVariant v : arr) {
      int r = v["r"] | 0;  // Extract RGB values
      int g = v["g"] | 0;
      int b = v["b"] | 0;
      colors.push_back(CRGB(r, g, b));  // Add to vector
    }
    
    // Call lamp control function to set multi-color animation
    setMultiColors(colors);
    
    // Send success response
    sendTextResponse(200, "OK");
  });

  // CORS preflight for speed endpoint
  server.on("/color/speed", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });

  /*
   * ENDPOINT: POST /color/speed
   * ==========================================
   * PURPOSE: Set the speed of multi-color animations
   * 
   * REQUEST FORMAT:
   *   URL: http://192.168.1.100/color/speed
   *   Method: POST
   *   Body: {"ms": 50}
   *   Content-Type: application/json
   * 
   * PARAMETERS (in JSON body):
   *   - ms or speed: Milliseconds between animation frames
   *   - Range: 3-1000 (clamped by setTransitionDelay)
   *   - Lower = faster, Higher = slower
   * 
   * EFFECT ON ANIMATION:
   *   - 15ms (default): Smooth, moderate speed
   *   - 50ms: Slow, leisurely transitions
   *   - 100ms: Very slow, meditative
   * 
   * EXAMPLE REQUEST:
   *   curl -X POST http://192.168.1.100/color/speed -H "Content-Type: application/json" -d '{"ms":50}'
   *   → Slows down animation to 50ms per frame
   * ==========================================
   */
  server.on("/color/speed", HTTP_POST, []() {
    // Get the request body
    String body = server.arg("plain");
    
    // Create JSON document with 100 byte buffer (small payload)
    StaticJsonDocument<100> doc;
    
    // Parse JSON
    DeserializationError err = deserializeJson(doc, body);
    
    if (err) {
      sendTextResponse(400, "Invalid JSON");
      return;
    }
    
    // Extract speed value (accepts both "ms" and "speed" keys for compatibility)
    unsigned int ms = 0;
    if (doc.containsKey("ms")) ms = doc["ms"].as<unsigned int>();
    else if (doc.containsKey("speed")) ms = doc["speed"].as<unsigned int>();
    
    // Validate that a speed was provided
    if (ms == 0) {
      sendTextResponse(400, "Invalid speed value");
      return;
    }
    
    // Call lamp control function to set animation speed
    setTransitionDelay(ms);
    
    // Send success response
    sendTextResponse(200, "OK");
  });

  // =========================================================================
  // SECTION 3: IR MOTION SENSOR CONTROL
  // =========================================================================

  /*
   * ENDPOINT: GET /ir/on
   * ==========================================
   * PURPOSE: Enable IR motion sensor mode
   * 
   * BEHAVIOR:
   *   - Lamp turns on when motion detected
   *   - Lamp turns off when no motion
   *   - Sensor auto-calibrates to detect baseline
   * 
   * EXAMPLE: http://192.168.1.100/ir/on
   * RESPONSE: 200 OK "IR Mode ON"
   * ==========================================
   */
  server.on("/ir/on", []() {
    setIrMode(true);
    sendTextResponse(200, "IR Mode ON");
  });

  /*
   * ENDPOINT: GET /ir/off
   * ==========================================
   * PURPOSE: Disable IR motion sensor mode
   * 
   * BEHAVIOR:
   *   - Lamp returns to manual control (button or API)
   *   - IR sensor input ignored
   *   - Lamp turns on (restored to previous state)
   * 
   * EXAMPLE: http://192.168.1.100/ir/off
   * RESPONSE: 200 OK "IR Mode OFF"
   * ==========================================
   */
  server.on("/ir/off", []() {
    setIrMode(false);
    sendTextResponse(200, "IR Mode OFF");
  });

  /*
   * ENDPOINT: POST /ir/toggle
   * ==========================================
   * PURPOSE: Toggle IR mode on/off via JSON
   * 
   * REQUEST FORMAT:
   *   URL: http://192.168.1.100/ir/toggle
   *   Method: POST
   *   Body: {"enabled": true}  or {"enabled": false}
   *   Content-Type: application/json
   * 
   * PARAMETERS:
   *   - enabled: true = enable IR mode, false = disable
   * ==========================================
   */
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
      lampEnabled = true;  // Ensure lamp is on when disabling IR
    }
    
    String response = enabled ? "IR Mode ON" : "IR Mode OFF";
    sendTextResponse(200, response.c_str());
  });

  // =========================================================================
  // SECTION 4: MOOD PRESETS - Activate predefined lighting moods
  // =========================================================================
  // Each mood has two endpoints: /[mood]/single and /[mood]/multi
  // - /single: solid color mode
  // - /multi: animation mode (transitions between 3 colors)

  /*
   * MOOD PRESET PATTERN:
   * ==========================================
   * ENDPOINT: GET /[mood]/single or /[mood]/multi
   * 
   * MOODS AVAILABLE:
   *   - /focus/*: Cool blue for concentration
   *   - /cozy/*: Warm orange for relaxation  
   *   - /social/*: Warm peach for social gatherings
   *   - /golden/*: Golden orange for sunset atmosphere
   *   - /latenight/*: Deep purple for bedtime
   *   - /rainy/*: Cool gray-blue for calm mood
   *   - /creative/*: Vibrant purple/cyan for creativity
   *   - /romantic/*: Soft pink for romantic settings
   * 
   * EXAMPLE REQUESTS:
   *   http://192.168.1.100/focus/single    → Blue solid light for work
   *   http://192.168.1.100/cozy/multi      → Warm orange animation for relaxing
   *   http://192.168.1.100/romantic/single → Pink solid light for romance
   * ==========================================
   */

  // FOCUS PRESET (cool blue for concentration)
  server.on("/focus/single", []() {
    setSingleColor(focus.singleColor);
    sendTextResponse(200, "Focus Single");
  });

  server.on("/focus/multi", []() {
    setMultiColors(std::vector<CRGB>{ focus.multi1, focus.multi2, focus.multi3 });
    sendTextResponse(200, "Focus Multi");
  });

  // COZY PRESET (warm orange for relaxation)
  server.on("/cozy/single", []() {
    setSingleColor(cozy.singleColor);
    sendTextResponse(200, "Cozy Single");
  });

  server.on("/cozy/multi", []() {
    setMultiColors(std::vector<CRGB>{ cozy.multi1, cozy.multi2, cozy.multi3 });
    sendTextResponse(200, "Cozy Multi");
  });

  // SOCIAL PRESET (warm peach for social gatherings)
  server.on("/social/single", []() {
    setSingleColor(social.singleColor);
    sendTextResponse(200, "Social Single");
  });

  server.on("/social/multi", []() {
    setMultiColors(std::vector<CRGB>{ social.multi1, social.multi2, social.multi3 });
    sendTextResponse(200, "Social Multi");
  });

  // GOLDEN HOUR PRESET (golden sunset atmosphere)
  server.on("/golden/single", []() {
    setSingleColor(goldenHour.singleColor);
    sendTextResponse(200, "Golden Hour Single");
  });

  server.on("/golden/multi", []() {
    setMultiColors(std::vector<CRGB>{ goldenHour.multi1, goldenHour.multi2, goldenHour.multi3 });
    sendTextResponse(200, "Golden Hour Multi");
  });

  // LATE NIGHT PRESET (deep purple for sleep)
  server.on("/latenight/single", []() {
    setSingleColor(lateNight.singleColor);
    sendTextResponse(200, "Late Night Single");
  });

  server.on("/latenight/multi", []() {
    setMultiColors(std::vector<CRGB>{ lateNight.multi1, lateNight.multi2, lateNight.multi3 });
    sendTextResponse(200, "Late Night Multi");
  });

  // RAINY DAY PRESET (cool gray-blue for calm)
  server.on("/rainy/single", []() {
    setSingleColor(rainyDay.singleColor);
    sendTextResponse(200, "Rainy Day Single");
  });

  server.on("/rainy/multi", []() {
    setMultiColors(std::vector<CRGB>{ rainyDay.multi1, rainyDay.multi2, rainyDay.multi3 });
    sendTextResponse(200, "Rainy Day Multi");
  });

  // CREATIVE FLOW PRESET (vibrant purple/cyan for creativity)
  server.on("/creative/single", []() {
    setSingleColor(creativeFlow.singleColor);
    sendTextResponse(200, "Creative Flow Single");
  });

  server.on("/creative/multi", []() {
    setMultiColors(std::vector<CRGB>{ creativeFlow.multi1, creativeFlow.multi2, creativeFlow.multi3 });
    sendTextResponse(200, "Creative Flow Multi");
  });

  // ROMANTIC PRESET (soft pink for romance)
  server.on("/romantic/single", []() {
    setSingleColor(romantic.singleColor);
    sendTextResponse(200, "Romantic Single");
  });

  server.on("/romantic/multi", []() {
    setMultiColors(std::vector<CRGB>{ romantic.multi1, romantic.multi2, romantic.multi3 });
    sendTextResponse(200, "Romantic Multi");
  });

  // =========================================================================
  // SECTION 5: BRIGHTNESS CONTROL - Set LED brightness levels
  // =========================================================================

  /*
   * BRIGHTNESS PRESET PATTERN:
   * ==========================================
   * ENDPOINT: GET /brightness/[level]
   * 
   * BRIGHTNESS LEVELS:
   *   - /brightness/ultra-low  → 30/255 (12% - for night)
   *   - /brightness/low        → 60/255 (24% - subdued)
   *   - /brightness/medium     → 120/255 (47% - comfortable reading)
   *   - /brightness/high       → 255/255 (100% - full brightness)
   * 
   * EXAMPLE REQUESTS:
   *   http://192.168.1.100/brightness/low      → Soft, dim lighting
   *   http://192.168.1.100/brightness/high     → Full brightness
   * ==========================================
   */

  // ULTRA-LOW BRIGHTNESS (30/255 - night mode, very dim)
  server.on("/brightness/ultra-low", []() {
    setBrightnessLevel(30);
    sendTextResponse(200, "Ultra-Low Brightness");
  });

  // LOW BRIGHTNESS (60/255 - subdued lighting)
  server.on("/brightness/low", []() {
    setBrightnessLevel(60);
    sendTextResponse(200, "Low Brightness");
  });

  // MEDIUM BRIGHTNESS (120/255 - reading/comfortable)
  server.on("/brightness/medium", []() {
    setBrightnessLevel(120);
    sendTextResponse(200, "Medium Brightness");
  });

  // HIGH BRIGHTNESS (255/255 - full brightness)
  server.on("/brightness/high", []() {
    setBrightnessLevel(255);
    sendTextResponse(200, "High Brightness");
  });
}