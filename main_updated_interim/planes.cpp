#include "planes.h"
#include "inbox.h"

float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0;
  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);
  float a = sin(dLat/2)*sin(dLat/2) +
            cos(radians(lat1))*cos(radians(lat2))*sin(dLon/2)*sin(dLon/2);

  return R * 2 * atan2(sqrt(a), sqrt(1-a));
}

float calculateBearing(float lat1, float lon1, float lat2, float lon2) {
  float dLon = radians(lon2 - lon1);
  float y = sin(dLon) * cos(radians(lat2));
  float x = cos(radians(lat1))*sin(radians(lat2)) -
            sin(radians(lat1))*cos(radians(lat2))*cos(dLon);
  return fmod((atan2(y, x) * 180.0 / PI + 360.0), 360.0);
}

String getCardinalDirection(float bearing) {
  if (bearing >= 337.5 || bearing < 22.5)  return "N";
  if (bearing < 67.5)  return "NE";
  if (bearing < 112.5) return "E";
  if (bearing < 157.5) return "SE";
  if (bearing < 202.5) return "S";
  if (bearing < 247.5) return "SW";
  if (bearing < 292.5) return "W";
  return "NW";
}

bool isInSouthernView(float bearing) {
  return (bearing >= PLANE_SOUTH_MIN_BEARING && bearing <= PLANE_SOUTH_MAX_BEARING);
}

float kmToMiles(float km) { return km * 0.621371; }

String simplifyAircraftType(String raw) {
  raw.toUpperCase(); raw.trim();
  if (raw.indexOf("737") >= 0) return "B737";
  if (raw.indexOf("747") >= 0) return "B747";
  if (raw.indexOf("757") >= 0) return "B757";
  if (raw.indexOf("767") >= 0) return "B767";
  if (raw.indexOf("777") >= 0) return "B777";
  if (raw.indexOf("787") >= 0) return "B787";
  if (raw.indexOf("A320") >= 0 || raw.indexOf("A319") >= 0 || raw.indexOf("A321") >= 0) return "A320";
  if (raw.indexOf("A330") >= 0) return "A330";
  if (raw.indexOf("A340") >= 0) return "A340";
  if (raw.indexOf("A350") >= 0) return "A350";
  if (raw.indexOf("A380") >= 0) return "A380";
  if (raw.indexOf("CRJ") >= 0 || raw.indexOf("CANADAIR") >= 0) return "CRJ";
  if (raw.indexOf("ERJ") >= 0 || raw.indexOf("EMBRAER") >= 0)  return "ERJ";
  if (raw.indexOf("GULFSTREAM") >= 0 || raw.indexOf("G650") >= 0 || raw.indexOf("G550") >= 0) return "G-JET";
  if (raw.indexOf("CITATION") >= 0)   return "CITE";
  if (raw.indexOf("CHALLENGER") >= 0) return "CHLGR";
  if (raw.indexOf("LEARJET") >= 0 || raw.indexOf("LEAR") >= 0) return "LEAR";
  if (raw.indexOf("FALCON") >= 0)  return "FLCN";
  if (raw.indexOf("HAWKER") >= 0)  return "HWKR";
  if (raw.indexOf("PHENOM") >= 0)  return "PHNM";
  if (raw.indexOf("LEGACY") >= 0)  return "LGCY";
  if (raw.indexOf("CESSNA") >= 0) {
    if (raw.indexOf("172") >= 0) return "C172";
    if (raw.indexOf("182") >= 0) return "C182";
    if (raw.indexOf("208") >= 0) return "C208";
    return "CESSNA";
  }
  if (raw.indexOf("PIPER") >= 0)   return "PIPR";
  if (raw.indexOf("CIRRUS") >= 0)  return "CIRS";
  if (raw.indexOf("BEECH") >= 0 || raw.indexOf("BONANZA") >= 0) return "BEECH";
  if (raw.indexOf("MD11") >= 0 || raw.indexOf("MD-11") >= 0) return "MD11";
  if (raw.length() > 0 && raw.length() <= 8) return raw.substring(0, min(6, (int)raw.length()));
  return "UNK";
}

void getAircraftType() {
  planeType = "UNK";
  if (WiFi.status() != WL_CONNECTED || planeIcao24.length() == 0) return;
  HTTPClient http;
  http.begin("https://opensky-network.org/api/metadata/aircraft/icao/" + planeIcao24);
  http.setTimeout(10000);
  if (http.GET() > 0) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      String model    = doc["model"]    | "";
      String typecode = doc["typecode"] | "";
      if      (model.length()    > 0) planeType = simplifyAircraftType(model);
      else if (typecode.length() > 0) planeType = simplifyAircraftType(typecode);
    }
  }
  http.end();
}

void getPlaneData() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  float lamin = LAT - 0.45, lamax = LAT + 0.45;
  float lomin = LON - 0.45, lomax = LON + 0.45;

  String url = "https://opensky-network.org/api/states/all?lamin=" +
               String(lamin, 4) + "&lomin=" + String(lomin, 4) +
               "&lamax=" + String(lamax, 4) + "&lomax=" + String(lomax, 4);

  http.begin(url);
  http.setTimeout(15000);
  int httpCode = http.GET();
  if (httpCode > 0) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      JsonArray states = doc["states"];
      if (states && states.size() > 0) {
        float minDistance = 999999;
        planeFound = false;
        for (JsonArray state : states) {
          float lon      = state[5] | 0.0f;
          float lat      = state[6] | 0.0f;
          bool  onGround = state[8] | false;
          if (onGround || lat == 0.0 || lon == 0.0) continue;

          float dist = calculateDistance(LAT, LON, lat, lon);
          if (dist < minDistance) {
            minDistance        = dist;
            planeIcao24        = state[0] | "";
            planeCallsign      = String(state[1] | "N/A"); planeCallsign.trim();
            planeDistance      = dist;
            planeAltitude      = (state[7] | 0.0f) * 3.28084f;
            planeSpeed         = (state[9] | 0.0f) * 1.94384f;
            planeHeading       = state[10] | 0.0f;
            planeBearingFromUs = calculateBearing(LAT, LON, lat, lon);
            planeDirection     = getCardinalDirection(planeBearingFromUs);
            planeFound         = true;
          }
        }
        if (planeFound) {
          getAircraftType();
          Serial.printf("Plane → %s  %s  %s  %.1fkm (%.1fmi)  %.0fft  %.0fkts  bearing:%.0f\n",
                        planeCallsign.c_str(), planeType.c_str(), planeDirection.c_str(),
                        planeDistance, kmToMiles(planeDistance),
                        planeAltitude, planeSpeed, planeBearingFromUs);
        }
      } else {
        planeFound = false;
        Serial.println("No planes detected");
      }
    }
  }
  http.end();
  lastPlaneUpdate = millis();
}

void displayPlane() {
  display->clearScreen();
  if (!planeFound) {
    display->setCursor(8, 12);
    display->setTextColor(display->color565(80, 80, 80));
    display->print("no planes nearby");
    drawInboxPixels();
    return;
  }

  display->drawFastHLine(0,  9, 64, display->color565(60, 60, 60));
  display->drawFastHLine(0, 20, 64, display->color565(60, 60, 60));

  display->setCursor(0, 1);
  display->setTextColor(display->color565(255, 255, 255));
  display->print(planeCallsign.length() > 6 ? planeCallsign.substring(0,6) : planeCallsign);

  int altBar = constrain((int)(planeAltitude / 45000.0 * 30), 1, 30);
  for (int i = 0; i < altBar; i++) {
    uint16_t barColor = (i < 10) ? display->color565(0,200,0)
                      : (i < 20) ? display->color565(255,200,0)
                                 : display->color565(255,50,50);
    display->drawPixel(63, 31 - i, barColor);
  }

  display->setCursor(0, 11);
  display->setTextColor(display->color565(0, 200, 255));
  display->print(planeType.length() > 4 ? planeType.substring(0,4) : planeType);

  display->setCursor(40, 11);
  display->setTextColor(display->color565(200, 200, 200));
  display->print((int)round(kmToMiles(planeDistance)));
  display->print("mi ");
  display->setCursor(50, 1);
  display->print(planeDirection);

  display->setCursor(0, 22);
  display->setTextColor(display->color565(255, 150, 0));
  display->print((int)round(planeSpeed));
  display->print("kt");
  display->setCursor(32, 22);
  display->setTextColor(display->color565(100, 255, 100));
  display->print((int)round(planeAltitude / 100) * 100);

  drawInboxPixels();
}