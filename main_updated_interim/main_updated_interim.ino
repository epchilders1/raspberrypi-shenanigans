#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESPmDNS.h>
#include <time.h>
#include "webpage.h"
#include "transitions.h"
#include "config.h"
#include "inbox.h"
#include "weather.h"
#include "planes.h"
#include "sports.h"
#include "display.h"
#include "api.h"
#include <Preferences.h>
Preferences prefs;

// ═════════════════════════════════════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1000);

  HUB75_I2S_CFG mxconfig(64, 32, 1);
  mxconfig.gpio.r1  =  4; mxconfig.gpio.g1  =  5; mxconfig.gpio.b1  =  6;
  mxconfig.gpio.r2  =  7; mxconfig.gpio.g2  = 15; mxconfig.gpio.b2  = 16;
  mxconfig.gpio.a   = 18; mxconfig.gpio.b   =  8; mxconfig.gpio.c   =  3;
  mxconfig.gpio.d   = 46; mxconfig.gpio.clk = 9; mxconfig.gpio.lat = 10;
  mxconfig.gpio.oe  = 11; mxconfig.clkphase = false;

  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();
  display->setBrightness8(90);
  display->clearScreen();

  display->setCursor(2, 2);
  display->setTextColor(display->color565(0, 255, 0));
  display->print("WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  configTime(-6 * 3600, 3600, "pool.ntp.org");

  display->clearScreen();
  display->setCursor(0, 2);
  display->setTextColor(display->color565(0, 255, 100));
  display->print("IP:");
  display->setCursor(0, 12);
  display->setTextColor(display->color565(255, 255, 255));
  display->print(WiFi.localIP().toString());
  delay(3000);

  // Load saved sports priority
  prefs.begin("board_settings", false);
  for (int i = 0; i < 6; i++) {
    sportsPriority[i] = prefs.getInt(("prio" + String(i)).c_str(), i);
  }

  // New endpoint to save priority from the web app
  server.on("/api/setPriority", HTTP_GET, []() {
    String p = server.arg("p"); // Expected format: "0,2,1,4,3,5"
    if (p.length() > 0) {
      int idx = 0;
      int start = 0;
      int end = p.indexOf(',');
      while (end != -1 && idx < 6) {
        sportsPriority[idx] = p.substring(start, end).toInt();
        prefs.putInt(("prio" + String(idx)).c_str(), sportsPriority[idx]);
        start = end + 1;
        end = p.indexOf(',', start);
        idx++;
      }
      if (idx < 6) {
        sportsPriority[idx] = p.substring(start).toInt();
        prefs.putInt(("prio" + String(idx)).c_str(), sportsPriority[idx]);
      }
    }
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  });

  server.on("/",                     HTTP_GET, handleRoot);
  server.on("/api/programA",         HTTP_GET, programA);
  server.on("/api/programB",         HTTP_GET, programB);
  server.on("/api/programDefault",   HTTP_GET, programDefault);
  server.on("/api/clock",            HTTP_GET, programClock);
  server.on("/api/programSports",    HTTP_GET, programSports);
  server.on("/api/sportsData",       HTTP_GET, handleSportsData);
  server.on("/api/display",          HTTP_GET, handleDisplayMessage);
  server.on("/api/boardOff",         HTTP_GET, boardOff);
  server.on("/api/setSleep",         HTTP_GET, handleSetSleep);
  server.on("/api/sendMessage",      HTTP_GET, handleSendMessage);
  server.on("/api/messages",         HTTP_GET, handleGetMessages);
  server.on("/api/displayMessage",   HTTP_GET, handleDisplayMessage_Inbox);
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) { setCORS(); server.send(204); }
    else server.send(404);
  });

  //Update Sports data
  updateSportsData(true);

  server.begin();
  MDNS.begin("esp32");
  Serial.println("Ready → http://esp32.local  /  http://" + WiFi.localIP().toString());
}

// ═════════════════════════════════════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════════════════════════════════════
void loop() {
  server.handleClient();

  //Rechecks the sports info every new day
  static int lastDay = -1;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
      if (timeinfo.tm_mday != lastDay) {
          updateSportsData(true); // Force a full refresh for the new day
          lastDay = timeinfo.tm_mday;
      }
  }

  static unsigned long lastSleepCheck = 0;
  if (millis() - lastSleepCheck >= 60000UL) {
    checkSleepSchedule();
    lastSleepCheck = millis();
  }

  if (!displayOn) return;

  unsigned long now = millis();

  // ── Mode 1: Weather only ──────────────────────────────────────────────────
  if (activeMode == 1 && !showingPlaneInterrupt) {
    if (now - lastWeatherUpdate >= WEATHER_INTERVAL) {
      getWeatherData();
      displayWeather();
    }
    if (now - lastWeatherFlip >= WEATHER_FLIP_INTERVAL) {
      weatherScreen = 1 - weatherScreen;
      displayWeather();
      lastWeatherFlip = now;
    }
  }

  // ── Sports fetch (runs in background for mode 4 and 6) ───────────────────
  if (activeMode == 4 || activeMode == 6) {
    updateSportsData();
  }

  // ── Mode 2: Planes ────────────────────────────────────────────────────────
  if (activeMode == 2 && now - lastPlaneUpdate >= PLANE_INTERVAL) {
    getPlaneData();
    displayPlane();
  }

  // ── Mode 4: Default (weather + planes + sports) ───────────────────────────
  if (activeMode == 4) {
    handleDefaultMode();
  }

  // ── Mode 5: Clock ─────────────────────────────────────────────────────────
  if (activeMode == 5 && now - lastClockUpdate >= CLOCK_INTERVAL) {
    displayClock();
    lastClockUpdate = now;
  }

  // ── Mode 6: Sports ────────────────────────────────────────────────────────
  if (activeMode == 6) {
    displaySports();
  }
}
