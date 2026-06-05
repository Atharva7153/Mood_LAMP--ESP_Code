/*
 * ============================================================================
 * FILE: web_api.h
 * ============================================================================
 * PURPOSE: Header file for REST API implementation
 *          Declares the function that sets up all HTTP endpoints
 * 
 * WHAT IS AN API (Application Programming Interface)?
 *   - A set of rules allowing different software to communicate
 *   - REST API: Uses HTTP requests (GET/POST) over the web
 *   - Allows web browsers and mobile apps to control the lamp
 * 
 * ENDPOINTS PROVIDED:
 *   - Color control: /color/single, /color/multi, /color/speed
 *   - Mood presets: /focus/*, /cozy/*, /social/*, etc.
 *   - IR control: /ir/on, /ir/off, /ir/toggle
 *   - Brightness: /brightness/ultra-low, /brightness/low, etc.
 * 
 * COMMUNICATION PROTOCOL:
 *   - HTTP (HyperText Transfer Protocol)
 *   - Port 80 (standard web port)
 *   - JSON format for data exchange
 *   - CORS enabled (works with web clients from any origin)
 * ============================================================================
 */

#ifndef WEB_API_H
#define WEB_API_H

/**
 * FUNCTION: setupRoutes()
 * ==========================================
 * PURPOSE: Register all HTTP endpoints and their handler functions
 *          Maps URLs to the functions that handle requests
 * 
 * CALLED BY: setup() in main.ino during initialization
 * 
 * ROUTES REGISTERED (examples):
 *   POST /color/single        → Set single solid color from JSON
 *   POST /color/multi         → Set multi-color animation from JSON
 *   POST /color/speed         → Set animation speed from JSON
 *   GET /focus/single         → Activate focus preset (single mode)
 *   GET /focus/multi          → Activate focus preset (animation mode)
 *   POST /ir/toggle           → Enable/disable IR mode from JSON
 *   GET /brightness/high      → Set brightness to maximum (255)
 *   ... and many more
 * 
 * HANDLER FUNCTIONS:
 *   - Each route has a lambda function (anonymous function)
 *   - Lambda extracts parameters, calls lamp functions, returns response
 *   - All responses include CORS headers for cross-origin requests
 * 
 * NO PARAMETERS - uses global server object defined in main.ino
 * ==========================================
 */
void setupRoutes();

#endif