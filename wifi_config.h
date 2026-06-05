/*
 * ============================================================================
 * FILE: wifi_config.h
 * ============================================================================
 * PURPOSE: WiFi credentials and network configuration
 *          IMPORTANT: This file contains sensitive WiFi password
 * 
 * SECURITY WARNING:
 *   - This file should NEVER be shared publicly or committed to GitHub
 *   - Anyone with these credentials can connect to your WiFi
 *   - If credentials compromised, change WiFi password immediately
 * 
 * CONFIGURATION:
 *   - WIFI_SSID: Name of the WiFi network to connect to
 *   - WIFI_PASSWORD: Password for the WiFi network
 * 
 * USAGE:
 *   - Included by main.ino during WiFi connection setup
 *   - Also used by scanNearbyNetworks() to identify target network
 * ============================================================================
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// The WiFi network name (SSID) that ESP32 will attempt to connect to
// This is the name you see when scanning available WiFi networks
const char* WIFI_SSID = "Vivo v29 ";

// The password for the above WiFi network
// Used for authentication when connecting
const char* WIFI_PASSWORD = "ah7kkzdv";

#endif