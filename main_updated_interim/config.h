#ifndef CONFIG_H
#define CONFIG_H

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESPmDNS.h>
#include <time.h>
#include "sports.h"

// ── WiFi ──────────────────────────────────────────────────────────────────────
extern const char* ssid;
extern const char* password;

// ── Location ──────────────────────────────────────────────────────────────────
extern const float LAT;
extern const float LON;

// ── Sleep schedule ─────────────────────────────────────────────────────────────
extern int SLEEP_HOUR;
extern int WAKEUP_HOUR;
extern bool sleepScheduleEnabled;

// ── Plane interrupt settings ──────────────────────────────────────────────────
extern const float PLANE_INTERRUPT_MILES;
extern const float PLANE_SOUTH_MIN_BEARING;
extern const float PLANE_SOUTH_MAX_BEARING;
extern const unsigned long PLANE_SHOW_DURATION;

// ── Sports Globals ────────────────────────────────────────────────────────────
extern const Sport SPORTS[6];
extern const int NUM_SPORTS;
extern int sportsPriority[6];
extern GameInfo sportsGames[6];
extern unsigned long lastSportsUpdate;
extern int sportCheckIndex;
extern int sportsFetchedCount;

// ── Globals ───────────────────────────────────────────────────────────────────
extern WebServer server;
extern MatrixPanel_I2S_DMA *display;

// Modes: 0=idle, 1=weather, 2=planes, 3=scrolling, 4=default(auto), 5=clock, 6=sports
extern int activeMode;

extern bool displayOn;
extern unsigned long lastWeatherUpdate;
extern unsigned long lastPlaneUpdate;
extern unsigned long lastClockUpdate;
extern unsigned long planeInterruptStart;
extern bool          showingPlaneInterrupt;

// ── Weather screen alternation ────────────────────────────────────────────────
extern int           weatherScreen;
extern unsigned long lastWeatherFlip;
extern const unsigned long WEATHER_FLIP_INTERVAL;
extern const unsigned long WEATHER_INTERVAL;
extern const unsigned long PLANE_INTERVAL;
extern const unsigned long CLOCK_INTERVAL;

// ── Weather state ─────────────────────────────────────────────────────────────
extern float tempHigh;
extern float tempLow;
extern float precipitation;
extern int   weatherCode;
extern int   hourCode8am;
extern int   hourCode12pm;
extern int   hourCode3pm;
extern int   hourCode6pm;

extern float uvIndexMax;
extern float hourlyUV[10];
extern float hourlyRain[10];
extern float humidity;
extern float windSpeed;
extern float windGusts;
extern bool  weatherDataReady;

// ── Plane state ───────────────────────────────────────────────────────────────
extern String planeCallsign;
extern String planeIcao24;
extern String planeType;
extern float  planeDistance;
extern float  planeAltitude;
extern float  planeSpeed;
extern float  planeHeading;
extern String planeDirection;
extern bool   planeFound;
extern float  planeBearingFromUs;

// ── Clock state ───────────────────────────────────────────────────────────────
extern uint8_t clockR;
extern uint8_t clockG;
extern uint8_t clockB;
extern int clockMode; // 0=day (morphing), 1=night (red simple)

#endif