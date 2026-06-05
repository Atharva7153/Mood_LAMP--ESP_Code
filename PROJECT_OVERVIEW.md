# MOOD LAMP - Smart IoT RGB Lighting System
## Complete Project Overview for Viva

---

## 📋 PROJECT SUMMARY

**Mood Lamp** is an IoT-based smart lighting system that allows users to control 16 addressable RGB LEDs via WiFi. The system provides customizable mood presets, animations, and brightness control through a REST API accessible from web browsers and mobile apps.

### Key Capabilities:
- ✅ WiFi-connected ESP32 microcontroller
- ✅ 16 addressable RGB LEDs (WS2812B)
- ✅ 8 preset mood lighting modes
- ✅ Single color and multi-color animation modes
- ✅ Adjustable animation speed
- ✅ 4-level brightness control
- ✅ Physical button control (GPIO 26)
- ✅ IR motion sensor support (GPIO 27)
- ✅ REST API with CORS support
- ✅ Web server on port 80

---

## 🏗️ ARCHITECTURE OVERVIEW

### Hardware Stack:
```
┌─────────────────────────────────┐
│      ESP32 Microcontroller      │ ← Main processor
├─────────────────────────────────┤
│  WiFi Module (built-in)         │ ← Network connectivity
├─────────────────────────────────┤
│  GPIO 13: WS2812B LED Strip     │ ← 16 RGB LEDs
│  GPIO 26: Push Button           │ ← Manual control
│  GPIO 27: IR Motion Sensor      │ ← Automatic detection
└─────────────────────────────────┘
```

### Software Stack:
```
main.ino
  ├─ WiFi connection & authentication
  ├─ Web server initialization
  ├─ LED animation loop
  └─ Network scanning & feedback

lamp_control.h/cpp
  ├─ LED driver functions
  ├─ Color & brightness management
  ├─ Button input handling
  ├─ IR sensor processing
  └─ Animation blending

web_api.h/cpp
  ├─ HTTP route handlers
  ├─ JSON request parsing
  ├─ CORS support
  └─ Response formatting

mood_presets.h
  └─ 8 predefined mood settings

wifi_config.h
  └─ Network credentials
```

---

## 📁 FILE DESCRIPTIONS

### 1. **main.ino** - Main Controller
**Purpose:** Entry point and system orchestrator

**Key Functions:**
- `setup()`: One-time initialization (called at boot)
  - Initializes serial communication
  - Sets up LED system
  - Connects to WiFi with visual feedback
  - Configures API routes
  - Starts web server
  
- `loop()`: Main execution loop (runs continuously)
  - Handles HTTP requests via `server.handleClient()`
  - Updates LED state via `updateLamp()`

**Visual Feedback Animations:**
- `breatheConnecting()`: Cyan breathing effect during WiFi connection
  - Uses sine wave for smooth breathing (1.6s cycle)
  - Brightness oscillates from 80-255
  - Provides user feedback that connection is in progress
  
- `fadeOutConnected()`: Green fade-out animation on connection success
  - Green light fades over 1.5 seconds
  - Indicates successful WiFi connection
  - Then restores default warm color

**Networking Functions:**
- `wifiStatusToString()`: Converts WiFi status codes to readable strings
  - Displays: WL_CONNECTED, WL_CONNECT_FAILED, WL_CONNECTION_LOST, etc.
  - Useful for debugging connection issues
  
- `scanNearbyNetworks()`: Scans and displays available WiFi networks
  - Shows network names (SSIDs)
  - Shows signal strength (dBm)
  - Marks target network with indicator

---

### 2. **lamp_control.h** - LED Control Interface
**Purpose:** Header file defining the public API for LED control

**Key Declarations:**

| Function | Purpose | Parameters |
|----------|---------|-----------|
| `setIrMode(bool)` | Enable/disable motion detection | true=on, false=off |
| `setBrightnessLevel(int)` | Set LED brightness | 0-255 scale |
| `setupLamp()` | Initialize LED hardware | None |
| `updateLamp()` | Update animations/inputs | None (runs every frame) |
| `setSingleColor(CRGB)` | Set solid color | RGB color |
| `setMultiColors(vector)` | Set animation colors | Vector of CRGB |
| `turnLampOff()` | Turn all LEDs off | None |
| `setTransitionDelay(int)` | Set animation speed | Milliseconds per frame |

**Global Variables:**
- `leds[NUM_LEDS]`: Array storing RGB values for 16 LEDs
- `irMode`: Motion sensor enabled flag
- `lampEnabled`: Lamp on/off state
- `multiMode`: Animation mode flag
- `transitionDelayMs`: Animation speed (default 15ms)

---

### 3. **lamp_control.cpp** - LED Control Implementation
**Purpose:** Contains all LED control logic and hardware interfacing

**Major Functions:**

#### `setupLamp()`
1. Configures GPIO pins with pull-up resistors
2. Initializes FastLED library for WS2812B control
3. Sets default brightness (255)
4. Initializes button state
5. Turns lamp on with default warm color

#### `setSingleColor(CRGB color)`
- Disables multi-color mode
- Sets all 16 LEDs to specified color
- Immediately displays on hardware

#### `setMultiColors(vector<CRGB>)`
- Enables multi-color animation mode
- Stores color list for smooth transitions
- Resets animation state

#### `animateMulti()`
**Animation Algorithm:**
1. Gets current and next colors from array
2. Calculates blend amount (0-255)
3. Blends between colors: `blended = blend(start, end, amount)`
4. Updates all LEDs with blended color
5. Increments blend amount
6. When blend reaches 255, moves to next color pair
7. Cycles back to first color at end

**Example:** With 3 colors and 15ms transition:
- Red → Red-Green mix → Green → Green-Blue mix → Blue → Blue-Red mix → (repeat)
- Total cycle time: 3 colors × 255 blends × 15ms = ~11.5 seconds

#### `setTransitionDelay(unsigned int ms)`
- Sets speed of multi-color animations
- Range: 3-1000ms (clamped)
- Lower = faster, Higher = slower

#### `updateLamp()` - State Machine Logic
**Execution Order Each Frame:**

1. **Button Input Processing:**
   - Reads GPIO 26
   - Detects falling edge (HIGH → LOW)
   - Debounces for 100ms
   - Toggles lampEnabled on press

2. **IR Sensor Processing:**
   - If IR mode enabled, reads GPIO 27
   - Compares to irTriggeredValue
   - Turns lamp on/off based on motion

3. **State Transitions:**
   - Tracks previous enabled state
   - On disable: calls `turnLampOff()`
   - On enable: calls `applyCurrentColors()`

4. **Animation Display:**
   - If multi-color mode: calls `animateMulti()`
   - If single-color: displays static color

**State Diagram:**
```
        ┌─────────────────┐
        │   LAMP OFF      │
        │ (LEDs black)    │
        └────────┬────────┘
                 ↑
    Button press │    IR detection │ API request
                 ↓                  ↓
        ┌─────────────────┐
        │   LAMP ON       │
        │ Display colors/ │
        │  animations     │
        └─────────────────┘
```

#### `setIrMode(bool enable)`
**Calibration Process:**
1. Takes 20 sensor readings with 5ms intervals
2. Counts HIGH readings
3. Determines baseline (LOW or HIGH)
4. Sets triggered value = opposite of baseline
5. Prints result for debugging

**Example Output:**
```
IR baseline=LOW => triggered=HIGH
(Means: Normally LOW, becomes HIGH when motion detected)
```

---

### 4. **mood_presets.h** - Lighting Presets
**Purpose:** Defines 8 preset lighting modes optimized for different activities

**Preset Structure:**
```cpp
struct Mood {
  CRGB singleColor;  // For solid color mode
  CRGB multi1;       // For animation mode - Color 1
  CRGB multi2;       // For animation mode - Color 2
  CRGB multi3;       // For animation mode - Color 3
};
```

### 8 Mood Presets:

| Mood | Best For | Single Color | Characteristics |
|------|----------|--------------|-----------------|
| **Focus** | Study, work, concentration | Cool blue (220,235,255) | Enhances alertness, reduces fatigue |
| **Cozy** | Relaxation, comfort | Warm orange (255,170,90) | Mimics firelight, very soothing |
| **Social** | Parties, gatherings | Warm peach (255,170,140) | Flattering skin tones, welcoming |
| **Golden Hour** | Evenings, sunset atmosphere | Rich golden (255,140,70) | Replicates sunset lighting, nostalgic |
| **Late Night** | Bedtime, sleep prep | Deep purple (70,60,160) | Minimal melatonin suppression |
| **Rainy Day** | Introspection, calm focus | Gray-blue (140,160,180) | Atmospheric, promotes calm thinking |
| **Creative Flow** | Creative work, brainstorming | Vibrant purple (180,80,255) | Stimulates imagination and creativity |
| **Romantic** | Date night, intimacy | Soft pink (255,120,180) | Passionate, flattering, intimate |

**RGB Color Space Explanation:**
- Each component: 0-255
- (0, 0, 0) = Black (all off)
- (255, 255, 255) = White (all full)
- (255, 0, 0) = Red (only red at full)
- (255, 231, 186) = Warm white/golden

---

### 5. **wifi_config.h** - Network Configuration
**Purpose:** Stores WiFi credentials (SSID and password)

**Security Note:**
⚠️ **NEVER share this file publicly or commit to GitHub!**
- Contains WiFi password
- Anyone with this info can connect to your network
- If compromised, change WiFi password immediately

**Current Configuration:**
```cpp
WIFI_SSID = "Vivo v29 "
WIFI_PASSWORD = "ah7kkzdv"
```

**Connection Process:**
1. ESP32 starts in Station (STA) mode
2. Attempts to connect to specified SSID
3. Authenticates with password
4. Obtains local IP address from router
5. Becomes accessible at http://[IP_ADDRESS]:80

---

### 6. **web_api.h** - API Interface Declaration
**Purpose:** Declares the public API setup function

**Single Function:** `setupRoutes()`
- Called during `setup()`
- Registers all HTTP endpoints
- Maps URLs to handler functions
- Sets up CORS headers

---

### 7. **web_api.cpp** - REST API Implementation
**Purpose:** Implements all HTTP endpoints for remote control

**Architecture:**
- HTTP Server on port 80
- JSON request/response format
- CORS enabled (works from any domain)
- Lambda functions as route handlers

### API Endpoints:

#### Color Control
```
POST /color/single
  Body: {"r": 255, "g": 100, "b": 50}
  Effect: Sets solid color
  Response: 200 OK

POST /color/multi
  Body: {"colors": [{"r":255,"g":0,"b":0}, {"r":0,"g":255,"b":0}]}
  Effect: Animates between colors
  Response: 200 OK

POST /color/speed
  Body: {"ms": 50}
  Effect: Sets animation speed to 50ms per frame
  Response: 200 OK
```

#### Preset Moods
```
GET /[mood]/single    → Solid color mode
GET /[mood]/multi     → Animation mode

Available moods: focus, cozy, social, golden, latenight, rainy, creative, romantic

Examples:
  http://192.168.1.100/focus/single     → Blue focus mode
  http://192.168.1.100/cozy/multi       → Warm cozy animation
  http://192.168.1.100/romantic/single  → Pink romantic mode
```

#### IR Motion Sensor
```
GET /ir/on           → Enable motion detection
GET /ir/off          → Disable motion detection
POST /ir/toggle      → Toggle with JSON {"enabled": true/false}
```

#### Brightness Control
```
GET /brightness/ultra-low  → Brightness 30/255 (12%)
GET /brightness/low        → Brightness 60/255 (24%)
GET /brightness/medium     → Brightness 120/255 (47%)
GET /brightness/high       → Brightness 255/255 (100%)
```

### CORS (Cross-Origin Resource Sharing)
- Allows web UI from any domain to control lamp
- Sends headers: `Access-Control-Allow-Origin: *`
- Enables: GET, POST, OPTIONS methods
- Supports: Content-Type: application/json

### Error Handling
- Invalid JSON: Returns 400 "Invalid JSON"
- Missing parameters: Returns 400 "No colors provided"
- Success: Returns 200 "OK"

**Helper Function:** `sendTextResponse(int code, const char* msg)`
- Adds CORS headers to response
- Sends HTTP status code and message
- Used by all endpoints

---

## 🔄 PROGRAM FLOW

### Startup Sequence (First 30 seconds):
```
1. Serial communication initialized (115200 baud)
   ↓
2. LED system setup (FastLED library, GPIO pins)
   ↓
3. WiFi configuration
   - Scans nearby networks
   - Attempts connection to configured SSID
   - Shows cyan breathing animation while connecting
   ↓
4. Connection result:
   - Success: Green fade-out animation, show status
   - Failure: Show warm color, display error code
   ↓
5. API routes configured (all endpoints registered)
   ↓
6. Web server started on port 80
   ↓
7. System ready for requests!
```

### Runtime Loop (Continuous):
```
loop() {
  1. server.handleClient()    // Check for HTTP requests
  2. updateLamp()             // Update animations, read inputs
  3. Repeat ~1000x per second
}
```

### Color Mode Behavior:
```
Single Color Mode:
  - Static: display color on all LEDs
  - No animation, no delay

Multi-Color Mode:
  - Frame 0: Show color[0] (100% C0, 0% C1)
  - Frame 128: Show 50% C0, 50% C1 (blend)
  - Frame 256: Show C1 (0% C0, 100% C1)
  - Delay transitionDelayMs between frames
  - After reaching C[n], move to blend C[n]→C[n+1]
  - After last color, wrap to first
  - Repeat forever
```

---

## 💡 KEY FEATURES EXPLAINED

### 1. **Multi-Color Animation**
Uses FastLED's `blend()` function to smoothly transition between colors.

**Why smooth?** Without blending (just switching), transitions look jarring.

**Math:** `blended = blend(startColor, endColor, blendAmount)`
- blendAmount goes 0→255 over multiple frames
- Lower transition delay = smoother, faster animation
- Higher transition delay = slower, more leisurely effect

### 2. **Button Debouncing**
Prevents multiple triggers from electrical noise or vibration.

**Strategy:** Only accept button press if >100ms since last press.

**Why?** Button contacts can bounce (vibrate) for ~5-20ms, causing multiple false presses.

### 3. **IR Motion Sensor Auto-Calibration**
Detects baseline (idle) state, then inverts it for trigger state.

**Why auto-calibrate?** Different sensors have different idle states:
- Some sensors: LOW when no motion, HIGH when motion
- Others: HIGH when no motion, LOW when motion

**Solution:** Sample 20 times, count HIGH, determine baseline, set triggered = opposite.

### 4. **WiFi Connection Feedback**
Shows breathing animation while connecting (user knows something is happening).

**Breathing Algorithm:**
```
brightness = sin(progress × 2π)  // -1 to +1
brightness = map to 80-255       // Convert to visible range
Update all LEDs every 50ms
```

Result: Smooth pulsing effect that's calming and indicates processing.

### 5. **REST API with JSON**
Allows any web browser or app to control lamp.

**Example curl command:**
```bash
curl -X POST http://192.168.1.100/color/single \
  -H "Content-Type: application/json" \
  -d '{"r":255,"g":0,"b":0}'
```

This turns lamp red.

---

## 🎯 KEY CONCEPTS FOR VIVA

### 1. **WiFi Connectivity**
- ESP32 has built-in WiFi module
- Connects to home WiFi network
- Gets local IP address
- Accessible within local network

### 2. **Addressable LEDs**
- Each LED individually controllable
- WS2812B (NeoPixel) standard
- Requires precise timing for data signal
- FastLED library handles complexity

### 3. **Animations**
- Frame-based animation (like video)
- Each frame updates colors
- Delay between frames controls speed
- Blending makes transitions smooth

### 4. **Input Processing**
- Button: Detects falling edge (HIGH→LOW)
- IR Sensor: Detects when sensor value matches triggered state
- Both update lampEnabled variable

### 5. **State Management**
- lampEnabled: Is lamp on or off?
- multiMode: Animation or solid color?
- irMode: Motion sensor active?
- Track previous state for edge detection

### 6. **Communication**
- WiFi: Network access
- HTTP: Web protocol (request/response)
- JSON: Data format (text representation of objects)
- REST: API style (resource-based, method-based operations)

---

## 📊 SYSTEM SPECIFICATIONS

| Component | Specification |
|-----------|---------------|
| **Microcontroller** | ESP32 (32-bit ARM Cortex-M3) |
| **RAM** | 160KB internal SRAM |
| **Flash Storage** | 4MB |
| **WiFi** | 802.11 b/g/n, 2.4GHz |
| **GPIO Pins Used** | 13 (LED data), 26 (button), 27 (IR sensor) |
| **LED Count** | 16 addressable RGB |
| **LED Type** | WS2812B (NeoPixel) |
| **LED Voltage** | 5V |
| **Brightness Levels** | 256 (0-255) |
| **Color Values** | 16.7 million (256³ RGB combinations) |
| **Update Rate** | ~1000 Hz (1ms response time) |
| **HTTP Port** | 80 (standard web port) |
| **API Format** | JSON over HTTP |

---

## 🔧 LIBRARIES USED

| Library | Purpose |
|---------|---------|
| **WiFi.h** | ESP32 WiFi connectivity |
| **WebServer.h** | HTTP server for handling requests |
| **FastLED.h** | Addressable LED control (WS2812B) |
| **ArduinoJson.h** | Parse JSON from HTTP requests |
| **vector** | Store multiple colors for animation |

---

## 🎓 VIVA TALKING POINTS

### When asked about system overview:
"This is an IoT-based smart lighting system. An ESP32 microcontroller controls 16 addressable RGB LEDs via WiFi. Users can control the lamp through a REST API by sending HTTP requests with JSON data. The system provides 8 preset moods, custom color control, animations, and motion detection."

### When asked about hardware:
"We use an ESP32 microcontroller which has built-in WiFi capability. The LED strip consists of 16 WS2812B addressable LEDs connected to GPIO 13 for data signal. We also have a physical button on GPIO 26 for manual control and an IR motion sensor on GPIO 27 for automatic activation."

### When asked about software architecture:
"The main.ino file is the entry point that handles WiFi connection and web server setup. The lamp_control module manages all LED operations including color, brightness, and animations. The web_api module implements REST endpoints that clients can call via HTTP. Mood presets define color combinations optimized for different activities."

### When asked about animation:
"We use FastLED's blend function to smoothly transition between colors. For multi-color mode, we blend from color[0] to color[1], then to color[2], and cycle back. Each blend step increments over 256 frames with a configurable delay between frames. Lower delay creates faster, smoother animations."

### When asked about API:
"The REST API uses HTTP protocol on port 80. Clients send POST requests with JSON bodies containing color values or mood names. All responses include CORS headers to allow access from any domain. For example, posting {'r':255,'g':0,'b':0} to /color/single turns the lamp red."

### When asked about motion detection:
"The IR sensor auto-calibrates by taking 20 samples and determining baseline. If more than 50% of samples are HIGH, the baseline is HIGH, and triggered value becomes LOW. When the sensor reads the triggered value, the lamp turns on automatically."

---

## 🚀 FUTURE ENHANCEMENTS

1. **Mobile App:** Native Android/iOS app for better UX
2. **Voice Control:** Integration with Alexa/Google Home
3. **Scheduling:** Set mood changes at specific times
4. **Automation:** Trigger moods based on time of day
5. **More LEDs:** Extend to 32, 64, or more LEDs
6. **Color History:** Save and recall favorite colors
7. **Music Sync:** Change colors based on music beat
8. **Temperature Sensor:** Auto-adjust based on room temperature
9. **Sunrise/Sunset:** Mimic natural sunrise/sunset cycles
10. **Energy Monitoring:** Track LED power consumption

---

**Document Version:** 1.0  
**Last Updated:** 2026-06-05  
**For:** Viva Preparation
