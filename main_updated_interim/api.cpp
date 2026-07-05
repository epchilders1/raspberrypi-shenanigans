#include "api.h"
#include "webpage.h"
#include "inbox.h"
#include "weather.h"
#include "planes.h"
#include "sports.h"
#include "display.h"
#include "transitions.h"

void setCORS() {
  server.sendHeader("Access-Control-Allow-Origin",  "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleRoot() {
  Serial.printf("Free heap before serve: %d\n", ESP.getFreeHeap());
  WiFiClient client = server.client();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  const char* p = INDEX_HTML;
  size_t remaining = strlen_P(INDEX_HTML);
  char buf[512];

  while (remaining > 0) {
    size_t toSend = min(remaining, sizeof(buf));
    memcpy_P(buf, p, toSend);
    client.write(buf, toSend);
    p += toSend;
    remaining -= toSend;
  }

  server.client().stop();
}

void checkSleepSchedule() {
  if (!sleepScheduleEnabled) return;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 500)) return;

  int hour = timeinfo.tm_hour;
  bool shouldSleep = (hour >= SLEEP_HOUR || hour < WAKEUP_HOUR);

  if (shouldSleep && displayOn) {
    display->clearScreen();
    display->setBrightness8(0);
    displayOn = false;
    Serial.println("Display sleeping");
  } else if (!shouldSleep && !displayOn) {
    display->setBrightness8(90);
    displayOn = true;
    activeMode = 4; // Always return to default mode (weather + plane interrupts + live sports) on wake
    Serial.println("Display waking up → default mode");
    if (weatherDataReady) displayWeather();
    else { getWeatherData(); displayWeather(); }
    drawInboxPixels();
  }
}

void handleDefaultMode() {
  unsigned long now = millis();
  
  if (showingPlaneInterrupt) {
    if (now - planeInterruptStart >= PLANE_SHOW_DURATION) {
      showingPlaneInterrupt = false;
      Serial.println("Plane interrupt ended, returning");
      weatherScreen = 0;
      bool gameActive = false;
      for(int i=0; i<6; i++) if(sportsGames[i].found && sportsGames[i].state == "in") gameActive = true;
      if (!gameActive) displayWeather(); 
    }
    return;
  }
  
  bool gameActive = false;
  for(int i=0; i<6; i++) {
      if(sportsGames[i].found && sportsGames[i].state == "in") { gameActive = true; break; }
  }

  if (gameActive) {
      displaySports(); 
  } else {
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

  if (now - lastPlaneUpdate >= PLANE_INTERVAL) {
    getPlaneData();
    if (planeFound) {
      float distMiles = kmToMiles(planeDistance);
      bool  inView    = isInSouthernView(planeBearingFromUs);
      if (distMiles <= PLANE_INTERRUPT_MILES && inView) {
        showingPlaneInterrupt = true;
        planeInterruptStart   = now;
        transitionToPlane(display);
      }
    }
  }
}

void handleSendMessage() {
  setCORS();
  String name = server.arg("name");
  String msg  = server.arg("message");
  name.trim(); msg.trim();
  if (name.length() == 0 || msg.length() == 0) {
    server.send(400, "application/json", "{\"status\":\"missing name or message\"}");
    return;
  }

  if (name.length() > 30)  name = name.substring(0, 30);
  if (msg.length()  > 120) msg  = msg.substring(0, 120);

  String timeStr = "";
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 500)) {
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    timeStr = String(buf);
  } else {
    timeStr = "??:??";
  }

  addInboxMessage(name, msg, timeStr);
  server.send(200, "application/json", "{\"status\":\"Message received\"}");
}

void handleGetMessages() {
  setCORS();
  String json = "{\"messages\":[";
  for (int i = 0; i < inboxCount; i++) {
    if (i > 0) json += ",";
    String safeName = inbox[i].name;
    safeName.replace("\"", "\\\"");
    json += "{\"id\":" + String(inbox[i].id) +
            ",\"name\":\"" + safeName + "\"" +
            ",\"time\":\"" + inbox[i].timeStr + "\"" +
            ",\"viewed\":" + (inbox[i].viewed ? "true" : "false") + "}";
  }
  json += "],\"unviewed\":" + String(unviewedCount) + "}";
  server.send(200, "application/json", json);
}

void handleDisplayMessage_Inbox() {
  setCORS();
  if (!server.hasArg("id")) {
    server.send(400, "application/json", "{\"status\":\"missing id\"}");
    return;
  }
  int id = server.arg("id").toInt();

  int idx = -1;
  for (int i = 0; i < inboxCount; i++) {
    if (inbox[i].id == id) { idx = i; break; }
  }
  if (idx < 0) {
    server.send(404, "application/json", "{\"status\":\"not found\"}");
    return;
  }

  viewMessage(id);
  server.send(200, "application/json", "{\"status\":\"displaying\"}");

  String scrollStr = inbox[idx].name + ": " + inbox[idx].text;
  activeMode = 3;
  int msgWidth = scrollStr.length() * 6, startX = 64, endX = -msgWidth;

  for (int x = startX; x > endX; x -= 1) {
    server.handleClient();
    if (activeMode != 3) break;
    display->clearScreen();
    display->setTextColor(display->color565(255, 112, 192)); 
    display->setCursor(x, 12);
    display->print(scrollStr);
    drawInboxPixels(); 
    delay(45);
  }
  display->clearScreen();
  activeMode = 0;

  if (weatherDataReady) displayWeather();
  drawInboxPixels();
}

void programA() {
  setCORS();
  activeMode = 1;
  showingPlaneInterrupt = false;
  weatherScreen = 0;
  displayOn = true;
  display->setBrightness8(90);
  Serial.println("Program A: Weather mode");

  if (!weatherDataReady || (millis() - lastWeatherUpdate >= WEATHER_INTERVAL)) {
    display->clearScreen();
    display->setCursor(2, 2);
    display->setTextColor(display->color565(0, 255, 0));
    display->print("Getting");
    display->setCursor(2, 12);
    display->print("Weather");
    getWeatherData();
  }

  lastWeatherFlip = millis();
  transitionToWeather(display);
  drawInboxPixels();
  server.send(200, "application/json", "{\"status\":\"Weather displayed\"}");
}

void programB() {
  setCORS(); activeMode = 2; showingPlaneInterrupt = false;
  displayOn = true; display->setBrightness8(90);
  Serial.println("Program B: Plane tracking mode");
  display->clearScreen();
  display->setCursor(2, 2); display->setTextColor(display->color565(0, 255, 0));
  display->print("Tracking"); display->setCursor(2, 12); display->print("Planes");
  getPlaneData();
  transitionToPlane(display);
  drawInboxPixels();
  server.send(200, "application/json", "{\"status\":\"Plane tracking active\"}");
}

void programDefault() {
  setCORS(); activeMode = 4; showingPlaneInterrupt = false;
  weatherScreen = 0;
  displayOn = true; display->setBrightness8(90);
  Serial.println("Default mode: weather + plane interrupt + sports");
  
  if (!weatherDataReady || (millis() - lastWeatherUpdate >= WEATHER_INTERVAL)) {
    display->clearScreen();
    display->setCursor(2, 2); display->setTextColor(display->color565(0, 255, 0));
    display->print("Getting");
    display->setCursor(2, 12);
    display->print("Weather");
    getWeatherData();
  }

  lastWeatherFlip = millis();
  displayWeather();
  drawInboxPixels();
  server.send(200, "application/json", "{\"status\":\"Default mode active\"}");
}

void programClock() {
  setCORS();
  showingPlaneInterrupt = false;
  displayOn = true;
  display->setBrightness8(90);
  if (server.hasArg("mode")) {
    clockMode = server.arg("mode").toInt();
  }

  activeMode = 5;
  Serial.printf("Clock mode → %s\n", clockMode == 0 ? "Day (Morphing)" : "Night (Red)");
  displayClock();
  server.send(200, "application/json", "{\"status\":\"Clock active\"}");
}

void programSports() {
  setCORS();
  activeMode = 6;
  showingPlaneInterrupt = false;
  displayOn = true;
  display->setBrightness8(90);
  lastSportsUpdate = 0;    // Force immediate fetch next loop cycle
  sportsFetchedCount = 0;  // Reset so webpage waits for all 6 sports to re-fetch
  sportCheckIndex = 0;     // Start from the first sport again
  server.send(200, "application/json", "{\"status\":\"Sports active\"}");
}

void boardOff() {
  setCORS();
  activeMode = 0; showingPlaneInterrupt = false;
  display->clearScreen(); display->setBrightness8(0); displayOn = false;
  Serial.println("Board off via API");
  server.send(200, "application/json", "{\"status\":\"Board off\"}");
}

void handleSetSleep() {
  setCORS();
  bool enabled = (server.arg("enabled") == "1");
  sleepScheduleEnabled = enabled;

  if (enabled) {
    if (server.hasArg("sleepH")) SLEEP_HOUR  = server.arg("sleepH").toInt();
    if (server.hasArg("wakeH"))  WAKEUP_HOUR = server.arg("wakeH").toInt();

    SLEEP_HOUR  = constrain(SLEEP_HOUR,  0, 23);
    WAKEUP_HOUR = constrain(WAKEUP_HOUR, 0, 23);

    Serial.printf("Sleep schedule updated → sleep:%dh  wake:%dh\n", SLEEP_HOUR, WAKEUP_HOUR);
    server.send(200, "application/json", "{\"status\":\"Schedule updated\"}");

  } else {
    if (!displayOn) { display->setBrightness8(90); displayOn = true; }
    Serial.println("Sleep schedule disabled");
    server.send(200, "application/json", "{\"status\":\"Schedule disabled\"}");
  }
}
void handleSportsData() {
    setCORS();
    server.send(200, "application/json", sportsDataJson());
}