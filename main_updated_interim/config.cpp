#include "config.h"

// ── WiFi ──────────────────────────────────────────────────────────────────────
const char* ssid = "KF Brownsville Chapter";
const char* password = "monkeysarefunny";

// ── Location ──────────────────────────────────────────────────────────────────
const float LAT = 25.958694;
const float LON = -97.480888;

// ── Sleep schedule ─────────────────────────────────────────────────────────────
int SLEEP_HOUR  = 22;
int WAKEUP_HOUR =  7;
bool sleepScheduleEnabled = true;

// ── Plane interrupt settings ──────────────────────────────────────────────────
const float PLANE_INTERRUPT_MILES   = 50.0;
const float PLANE_SOUTH_MIN_BEARING = 135.0;
const float PLANE_SOUTH_MAX_BEARING = 225.0;
const unsigned long PLANE_SHOW_DURATION = 15000UL;

// ── Sports Configuration ──────────────────────────────────────────────────────
// Note: The 'Sport' and 'GameInfo' structs are defined in sports.h
const Sport SPORTS[6] = {
  { "NFL",   "football/nfl",                       "SEA" },
  { "MLB",   "baseball/mlb",                       "SEA" },
  { "NHL",   "hockey/nhl",                         "SEA" },
  { "MLS",   "soccer/usa.1",                       "SEA" },
  { "NCAAF", "football/college-football",          "ALA" },
  { "NCAAB", "basketball/mens-college-basketball", "ALA" }
};

const int NUM_SPORTS = 6;
int sportsPriority[6] = {0, 1, 2, 3, 4, 5}; 

// ── Sports Globals ────────────────────────────────────────────────────────────
GameInfo sportsGames[6] = {}; 
unsigned long lastSportsUpdate = 0;
unsigned long lastSportsDraw = 0;
int sportsDisplayIndex = 0;
int sportCheckIndex = 0;
int sportsFetchedCount = 0;

// ── Globals ───────────────────────────────────────────────────────────────────
WebServer server(80);
MatrixPanel_I2S_DMA *display = nullptr;

int activeMode = 0;

bool displayOn = true;
unsigned long lastWeatherUpdate   = 0;
unsigned long lastPlaneUpdate     = 0;
unsigned long lastClockUpdate     = 0;
unsigned long planeInterruptStart = 0;
bool          showingPlaneInterrupt = false;

int           weatherScreen       = 0;
unsigned long lastWeatherFlip     = 0;
const unsigned long WEATHER_FLIP_INTERVAL = 5000UL; // 5 seconds
const unsigned long WEATHER_INTERVAL = 1800000UL; // Updates weather every 30 minutes (1800000 milliseconds)
const unsigned long PLANE_INTERVAL   =   10000UL; // Updates plane info every 10 seconds when active
const unsigned long CLOCK_INTERVAL   =    1000UL; // Updates clock every second

float tempHigh      = 0;
float tempLow       = 0;
float precipitation = 0;
int   weatherCode   = 0;
int   hourCode8am   = 0;
int   hourCode12pm  = 0;
int   hourCode3pm   = 0;
int   hourCode6pm   = 0;

float uvIndexMax    = 0;
float hourlyUV[10]  = {0};
float hourlyRain[10] = {0};
float humidity      = 0;
float windSpeed     = 0;
float windGusts     = 0;
bool  weatherDataReady = false;

String planeCallsign  = "";
String planeIcao24    = "";
String planeType      = "";
float  planeDistance  = 0;
float  planeAltitude  = 0;
float  planeSpeed     = 0;
float  planeHeading   = 0;
String planeDirection = "";
bool   planeFound     = false;
float  planeBearingFromUs = 0;

uint8_t clockR = 0;
uint8_t clockG = 255;
uint8_t clockB = 160;
int clockMode = 0;