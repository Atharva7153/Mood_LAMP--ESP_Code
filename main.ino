/*
 * ============================================================================
 * PROJECT: MOOD LAMP - IoT Smart RGB LED Control System
 * ============================================================================
 * 
 * PURPOSE:
 * This is the main controller for a WiFi-connected mood lamp that uses 
 * addressable RGB LEDs (WS2812B NeoPixels). It allows users to control 
 * lamp colors, brightness, and animations via REST API endpoints from a 
 * web interface.
 * 
 * KEY FEATURES:
 * - WiFi connectivity for remote control
 * - 8 preset mood lighting modes (Focus, Cozy, Social, Golden Hour, Late Night, 
 *   Rainy Day, Creative Flow, Romantic)
 * - Single color and multi-color animation modes
 * - Adjustable animation speed (transition delay)
 * - Brightness control (4 levels: ultra-low, low, medium, high)
 * - Button control (physical toggle on/off)
 * - IR motion sensor support (auto on/off when motion detected)
 * - REST API for web-based control
 * - Visual feedback animations during WiFi connection
 * 
 * HARDWARE COMPONENTS:
 * - ESP32 microcontroller
 * - 16 addressable RGB LEDs (WS2812B/NeoPixel)
 * - Push button (GPIO 26) - physical on/off control
 * - IR motion sensor (GPIO 27) - motion detection
 * - WiFi module (built into ESP32)
 * ============================================================================
 */

#include <WiFi.h>           // WiFi connectivity library
#include <WebServer.h>      // HTTP server to handle API requests

#include "wifi_config.h"    // Contains WiFi credentials (SSID & password)
#include "lamp_control.h"   // LED control functions
#include "web_api.h"        // REST API endpoint definitions

// Create a web server object that listens on port 80 (standard HTTP port)
WebServer server(80);

/*
 * FUNCTION: wifiStatusToString()
 * ==========================================
 * PURPOSE: Convert WiFi connection status codes to readable strings for debugging
 * 
 * PARAMETERS:
 *   - s: WiFi status enumeration value (wl_status_t)
 * 
 * RETURNS: String representation of the status (e.g., "WL_CONNECTED")
 * 
 * USAGE: Displays in Serial Monitor to show why WiFi connection succeeded/failed
 * 
 * EXAMPLE STATUS CODES:
 *   - WL_CONNECTED: Successfully connected to WiFi network
 *   - WL_CONNECT_FAILED: Connection attempt failed (wrong password, network not found)
 *   - WL_CONNECTION_LOST: Previously connected but lost connection
 *   - WL_DISCONNECTED: Not connected (default initial state)
 * ==========================================
 */
const char* wifiStatusToString(wl_status_t s) {
  switch(s) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";           // WiFi shield not detected
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";       // Idle, not searching
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";   // Target network not found
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED"; // Network scan finished
    case WL_CONNECTED: return "WL_CONNECTED";           // Connected to network
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED"; // Connection failed
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST"; // Connection was lost
    case WL_DISCONNECTED: return "WL_DISCONNECTED";     // Disconnected
    default: return "WL_UNKNOWN";                       // Unknown status
  }
}

/*
 * FUNCTION: scanNearbyNetworks()
 * ==========================================
 * PURPOSE: Scan for all available WiFi networks in the area and display them
 *          to the user in Serial Monitor. Also identifies if target network found.
 * 
 * PARAMETERS: None
 * 
 * RETURNS: Void (outputs to Serial Monitor)
 * 
 * FUNCTIONALITY:
 * 1. Initiates WiFi network scan (passive scanning)
 * 2. Waits for scan to complete
 * 3. Displays all found networks with:
 *    - Network name (SSID)
 *    - Signal strength in dBm (the higher, the stronger signal)
 *    - Marks the target network that ESP32 is trying to connect to
 * 
 * DEBUGGING AID: Helps troubleshoot connection issues by showing:
 *   - If target network is available in range
 *   - Signal strength of target network
 *   - Other networks that might interfere
 * 
 * EXAMPLE OUTPUT:
 * "Found 5 network(s):
 *  [1] Vivo v29 (-45 dBm) ← TARGET NETWORK
 *  [2] Other_Network (-62 dBm)
 *  [3] Guest_WiFi (-78 dBm)"
 * ==========================================
 */
void scanNearbyNetworks() {
  Serial.println("\n    Scanning nearby WiFi networks...");
  
  // Initiate WiFi network scan. Returns number of networks found
  int numNetworks = WiFi.scanNetworks();
  
  Serial.print("    Found ");
  Serial.print(numNetworks);
  Serial.println(" network(s):\n");
  
  if (numNetworks > 0) {
    // Loop through all networks found
    for (int i = 0; i < numNetworks; i++) {
      Serial.print("    [");
      Serial.print(i + 1);
      Serial.print("] ");
      Serial.print(WiFi.SSID(i));           // Print network name
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));           // Print signal strength (dBm)
      Serial.print(" dBm)");
      
      // Compare this network to our target network (configured in wifi_config.h)
      if (strcmp(WiFi.SSID(i).c_str(), WIFI_SSID) == 0) {
        Serial.print(" ← TARGET NETWORK");  // Mark our target network
      }
      Serial.println("");
    }
  } else {
    Serial.println("    No networks found!");  // No WiFi networks detected
  }
  Serial.println("");
}



/*
 * FUNCTION: breatheConnecting()
 * ==========================================
 * PURPOSE: Create a smooth "breathing" animation effect while WiFi is connecting
 *          Shows soft cyan light that pulses gently to indicate connection in progress
 * 
 * PARAMETERS:
 *   - elapsedMs: Time elapsed (in milliseconds) since connection attempt started
 *               Used to calculate animation progress
 * 
 * ANIMATION DETAILS:
 *   - Color: Soft cyan (light blue) - calming color
 *   - Pattern: Uses sine wave for smooth breathing effect
 *   - Brightness: Oscillates smoothly from 80 to 255 (about 30% to 100%)
 *   - Speed: 1 complete breath cycle takes 1.6 seconds
 *   - Duration: Continues until WiFi connects or timeout reached
 * 
 * HOW IT WORKS:
 *   1. Uses mathematical sine wave to create smooth brightness variation
 *   2. Maps sine output (-1 to +1) to brightness range (80 to 255)
 *   3. Updates all 16 LEDs with the calculated brightness
 *   4. Creates natural "breathing" illusion similar to device sleep indicators
 * 
 * EXAMPLE VISUALIZATION:
 * Brightness over time:
 *     ╱─╲      ╱─╲      ╱─╲
 *   ╱   ╲    ╱   ╲    ╱   ╲    (cyan breathing pattern)
 * ==========================================
 */
void breatheConnecting(unsigned long elapsedMs) {
  // Define the cyan color used for breathing animation
  CRGB connectingColor = CRGB(100, 220, 255); // Bright Cyan (R, G, B)
  
  unsigned long breathePeriod = 1600; // 1.6 seconds for one complete breath cycle
  
  // Calculate where we are in the current breath cycle (0 to breathePeriod)
  unsigned long cycleMs = elapsedMs % breathePeriod;
  
  // Convert cycle time to 0.0 to 1.0 progress value (normalized)
  float progress = (float)cycleMs / breathePeriod;
  
  // Apply sine wave to progress: converts linear progress to smooth breathing curve
  // sin(progress * 2π) creates a full sine wave from 0 to 2π radians
  // Result: smooth oscillation from -1 to +1
  float sine = sin(progress * 2 * 3.14159);
  
  // Map sine output (-1 to +1) to brightness range (80 to 255)
  // -1 → 80 brightness (dim), +1 → 255 brightness (bright), 0 → ~167 (mid)
  uint8_t brightness = map((int)((sine + 1) * 127.5), 0, 255, 80, 255);
  
  // Create a copy of the cyan color to apply brightness to
  CRGB breathed = connectingColor;
  
  // Adjust brightness: fadeLightBy reduces brightness by (255 - brightness) amount
  breathed.fadeLightBy(255 - brightness);
  
  // Apply the calculated breathing color to all 16 LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = breathed;
  }
  
  // Update the physical LEDs to show the new colors
  FastLED.show();
}

/*
 * FUNCTION: fadeOutConnected()
 * ==========================================
 * PURPOSE: Show a success animation after WiFi connects
 *          Green light fades smoothly to off over 1.5 seconds
 * 
 * ANIMATION DETAILS:
 *   - Color: Bright green - indicates success/good status
 *   - Pattern: Fade out (brightness gradually decreases to 0)
 *   - Duration: 1.5 seconds for complete fade
 *   - Updates: 50ms intervals for smooth animation (30 frames total)
 * 
 * HOW IT WORKS:
 *   1. Records start time when function is called
 *   2. Loops for 1.5 seconds, calculating fade progress each frame
 *   3. Maps elapsed time to brightness (255 down to 0)
 *   4. Updates LEDs every 20ms for smooth animation
 *   5. Function ends after 1.5 seconds (user doesn't manually call)
 * 
 * EXAMPLE VISUALIZATION:
 * Brightness over time (1500ms total):
 * 255 ████████████████ Start
 * 200 ████████████
 * 150 ████████
 * 100 ████
 * 50  ██
 * 0   (end)
 * ==========================================
 */
void fadeOutConnected() {
  // Define the success green color
  CRGB connectedColor = CRGB(100, 255, 150); // Bright Green (R, G, B)
  
  unsigned long fadeDuration = 1500;  // Total fade duration: 1.5 seconds
  unsigned long fadeStart = millis();  // Record the start time
  
  // Loop while less than 1.5 seconds has elapsed
  while (millis() - fadeStart < fadeDuration) {
    // Calculate how much time has passed since start (0 to 1500ms)
    unsigned long elapsed = millis() - fadeStart;
    
    // Map elapsed time to brightness: 255 (full) → 0 (off)
    // At 0ms: brightness = 255
    // At 1500ms: brightness = 0
    uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
    
    // Create a copy of green color to apply fading brightness to
    CRGB faded = connectedColor;
    
    // Apply the calculated brightness to the color
    faded.fadeLightBy(255 - brightness);
    
    // Apply faded color to all 16 LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = faded;
    }
    
    // Update physical LEDs with fade effect
    FastLED.show();
    
    delay(20);  // Wait 20ms before next frame (~50 frames per second, smooth animation)
  }
}

/*
 * FUNCTION: setup()
 * ==========================================
 * PURPOSE: Initialize all system components - runs once at boot
 *          This is the main initialization function for the ESP32
 * 
 * INITIALIZATION SEQUENCE:
 * 1. Serial communication setup (for debugging/logging to Serial Monitor)
 * 2. LED system initialization (FastLED library, pin configuration)
 * 3. WiFi scanning and connection setup
 * 4. API route configuration (REST endpoints)
 * 5. Web server startup
 * 
 * EXECUTION TIMELINE:
 *   ~0-2s: Serial & LED setup
 *   ~2-5s: WiFi scanning
 *   ~5-25s: WiFi connection attempt (with breathing animation)
 *   ~25-26s: API routes setup
 *   ~26s+: Server ready to receive requests
 * 
 * VISUAL FEEDBACK:
 *   - Cyan breathing during WiFi connection
 *   - Green fade-out on successful connection
 *   - Default warm color after all systems ready
 * ==========================================
 */
void setup() {

  // Initialize serial communication at 115200 baud rate
  // This allows debugging via Serial Monitor in Arduino IDE
  Serial.begin(115200);
  delay(500);  // Wait for serial port to stabilize

  // Print startup banner to Serial Monitor for debugging
  Serial.println("\n\n================================");
  Serial.println("  MOOD LAMP - STARTUP SEQUENCE");
  Serial.println("================================\n");

  // STEP 1: Initialize the LED system
  Serial.println("[1] Initializing LED System...");
  setupLamp();  // Calls function from lamp_control.cpp
  Serial.println("    ✓ LED System initialized\n");

  // Record startup time for WiFi connection visual feedback
  unsigned long wifiStartTime = millis();

  // STEP 2: WiFi Configuration
  Serial.println("[2] WiFi Configuration:");
  Serial.print("    Target SSID: ");
  Serial.println(WIFI_SSID);  // Show which network to connect to
  
  // Scan for available WiFi networks (debugging aid)
  scanNearbyNetworks();
  
  Serial.println("    Attempting connection...");

  // Set WiFi mode to Station (STA) - connects to an existing network
  // Alternative: WIFI_AP (Access Point) or WIFI_AP_STA (both)
  WiFi.mode(WIFI_STA);
  
  // Start WiFi connection using credentials from wifi_config.h
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  const unsigned long timeout = 20000; // Give 20 seconds to connect before timeout

  // Show soft cyan breathing animation while connecting
  // Loop continues until WiFi connected OR timeout reached
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
    unsigned long elapsedMs = millis() - wifiStartTime;
    breatheConnecting(elapsedMs);  // Show breathing animation
    delay(50);  // Small delay to update animation smoothly
    Serial.print(".");  // Print dots showing connection attempts
  }

  Serial.println("\n");

  // Check if WiFi connection was successful
  if (WiFi.status() != WL_CONNECTED) {
    // Connection failed - display error info
    Serial.print("    ✗ CONNECTION FAILED - Status: ");
    Serial.print(WiFi.status());
    Serial.print(" (");
    Serial.print(wifiStatusToString(WiFi.status()));
    Serial.println(")");
    // Set lamp to warm color (default) even though WiFi failed
    setSingleColor(CRGB(255, 231, 186));
  } else {
    // WiFi connection successful!
    Serial.println("    ✓ Successfully Connected!");
    Serial.print("    Connected SSID: ");
    Serial.println(WiFi.SSID());        // Show connected network name
    Serial.print("    IP Address: ");
    Serial.println(WiFi.localIP());     // Show assigned IP (needed for API access)
    Serial.print("    Gateway: ");
    Serial.println(WiFi.gatewayIP());   // Show gateway/router IP
    Serial.print("    Signal Strength: ");
    Serial.print(WiFi.RSSI());          // Show signal strength in dBm
    Serial.println(" dBm");
    
    // Show green fade-out animation to indicate success
    fadeOutConnected();
    
    // Restore default warm light after successful connection
    setSingleColor(CRGB(255, 231, 186));
  }

  // STEP 3: Setup Web Server Routes (API endpoints)
  Serial.println("\n[3] Setting up API Routes...");
  setupRoutes();  // Calls function from web_api.cpp - registers all endpoints
  Serial.println("    ✓ Routes configured\n");

  // STEP 4: Start the HTTP Web Server
  Serial.println("[4] Starting Web Server...");
  server.begin();  // Start listening for HTTP requests on port 80
  Serial.println("    ✓ Server running on port 80\n");

  // Print completion message
  Serial.println("================================");
  Serial.println("  SYSTEM READY - ACCEPTING REQUESTS");
  Serial.println("================================\n");
}


/*
 * FUNCTION: loop()
 * ==========================================
 * PURPOSE: Main program loop - runs continuously after setup() completes
 *          Handles HTTP requests and updates LED animations in real-time
 * 
 * EXECUTION FLOW (repeats continuously):
 * 1. Check for incoming HTTP requests from web clients
 * 2. Process any received requests (color changes, brightness, etc.)
 * 3. Update LED animations (if multi-color mode active)
 * 4. Repeat ~1000 times per second
 * 
 * KEY OPERATIONS:
 *   - server.handleClient(): Checks for new HTTP requests and processes them
 *   - updateLamp(): Updates animations, checks button/sensor inputs
 * 
 * HOW ANIMATIONS WORK:
 *   - Single color: Updates to solid color immediately
 *   - Multi-color: Smooth transition between colors over time
 *   - Button: Physical button press toggles lamp on/off
 *   - IR sensor: Motion detection turns lamp on/off automatically
 * 
 * TIMING:
 *   - loop() runs continuously without blocking
 *   - API requests processed immediately when received
 *   - Animations happen smoothly due to frequent updates
 * ==========================================
 */
void loop() {

  // Handle incoming HTTP requests from web clients
  // Checks for GET/POST requests to API endpoints like /color/single, /focus/multi, etc.
  // If request received, executes the corresponding handler function
  server.handleClient();

  // Update LED system state
  // This function:
  //   - Checks physical button for on/off toggling
  //   - Reads IR sensor for motion detection
  //   - Updates multi-color animations (if active)
  // Called frequently to ensure smooth animations
  updateLamp();
}