#include "weather.h"
#include "inbox.h"

void getWeatherData() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = "http://api.open-meteo.com/v1/forecast?latitude=" +
               String(LAT, 4) + "&longitude=" + String(LON, 4) +
               "&daily=temperature_2m_max,temperature_2m_min,precipitation_sum,weather_code,uv_index_max,wind_speed_10m_max,wind_gusts_10m_max"
               "&hourly=weather_code,relative_humidity_2m,wind_speed_10m,uv_index,precipitation"
               "&temperature_unit=fahrenheit&precipitation_unit=inch"
               "&wind_speed_unit=mph"
               "&timezone=America/Chicago";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      tempHigh      = doc["daily"]["temperature_2m_max"][0];
      tempLow       = doc["daily"]["temperature_2m_min"][0];
      precipitation = doc["daily"]["precipitation_sum"][0];
      weatherCode   = doc["daily"]["weather_code"][0];
      uvIndexMax    = doc["daily"]["uv_index_max"][0];
      windSpeed     = doc["daily"]["wind_speed_10m_max"][0];
      windGusts     = doc["daily"]["wind_gusts_10m_max"][0];

      hourCode8am   = doc["hourly"]["weather_code"][8];
      hourCode12pm  = doc["hourly"]["weather_code"][12];
      hourCode3pm   = doc["hourly"]["weather_code"][15];
      hourCode6pm   = doc["hourly"]["weather_code"][18];
      
      for (int i = 0; i < 10; i++) {
        hourlyUV[i] = doc["hourly"]["uv_index"][9 + i];  // 9AM to 6PM (indices 9 to 18)
        hourlyRain[i] = doc["hourly"]["precipitation"][9 + i];
      }
      
      humidity      = doc["hourly"]["relative_humidity_2m"][15];
      weatherDataReady = true;

      Serial.printf("Weather → Hi:%.1f Lo:%.1f Precip:%.2f Code:%d UV:%.1f Wind:%.0fmph Gusts:%.0fmph Hum:%.0f%%\n",
                    tempHigh, tempLow, precipitation, weatherCode,
                    uvIndexMax, windSpeed, windGusts, humidity);
    }
  }
  http.end();
  lastWeatherUpdate = millis();
}

int weatherIconType(int code) {
  if (code == 0)                return 0;
  if (code == 1 || code == 2)   return 1;
  if (code == 3)                return 2;
  if (code >= 51 && code <= 57) return 6;
  if (code >= 61 && code <= 67) return 3;
  if (code >= 71 && code <= 77) return 4;
  if (code >= 80 && code <= 82) return 3;
  if (code >= 85 && code <= 86) return 4;
  if (code >= 95)               return 5;
  return 2;
}

void drawWeatherIcon(int x, int y, int type) {
  uint16_t yellow = display->color565(255, 210,   0);
  uint16_t white  = display->color565(220, 220, 220);
  uint16_t grey   = display->color565(140, 140, 140);
  uint16_t blue   = display->color565( 80, 160, 255);
  uint16_t lblue  = display->color565(160, 210, 255);
  uint16_t ltyel  = display->color565(255, 240, 120);

  switch (type) {
    case 0:
      display->drawPixel(x+3, y+0, yellow);
      display->drawPixel(x+6, y+1, yellow);
      display->drawPixel(x+6, y+5, yellow);
      display->drawPixel(x+3, y+6, yellow);
      display->drawPixel(x+0, y+5, yellow);
      display->drawPixel(x+0, y+1, yellow);
      display->drawFastHLine(x+2, y+2, 3, yellow);
      display->drawFastHLine(x+1, y+3, 5, yellow);
      display->drawFastHLine(x+2, y+4, 3, yellow);
      break;
    case 1:
      display->drawPixel(x+5, y+0, ltyel);
      display->drawFastHLine(x+4, y+1, 3, ltyel);
      display->drawPixel(x+5, y+2, ltyel);
      display->drawFastHLine(x+1, y+4, 5, white);
      display->drawFastHLine(x+0, y+5, 6, white);
      display->drawFastHLine(x+1, y+6, 5, white);
      break;
    case 2:
      display->drawFastHLine(x+2, y+1, 3, white);
      display->drawFastHLine(x+1, y+2, 5, white);
      display->drawFastHLine(x+0, y+3, 7, white);
      display->drawFastHLine(x+0, y+4, 7, grey);
      display->drawFastHLine(x+1, y+5, 5, grey);
      break;
    case 3:
      display->drawFastHLine(x+1, y+0, 5, grey);
      display->drawFastHLine(x+0, y+1, 7, grey);
      display->drawFastHLine(x+0, y+2, 7, grey);
      display->drawFastHLine(x+1, y+3, 5, grey);
      display->drawPixel(x+1, y+5, blue);
      display->drawPixel(x+3, y+4, blue);
      display->drawPixel(x+5, y+5, blue);
      display->drawPixel(x+2, y+6, blue);
      display->drawPixel(x+4, y+6, blue);
      break;
    case 4:
      display->drawFastHLine(x+1, y+0, 5, grey);
      display->drawFastHLine(x+0, y+1, 7, grey);
      display->drawFastHLine(x+0, y+2, 7, grey);
      display->drawFastHLine(x+1, y+3, 5, grey);
      display->drawPixel(x+1, y+5, white);
      display->drawPixel(x+3, y+4, white);
      display->drawPixel(x+5, y+5, white);
      display->drawPixel(x+2, y+6, white);
      display->drawPixel(x+4, y+6, white);
      break;
    case 5:
      display->drawFastHLine(x+1, y+0, 5, grey);
      display->drawFastHLine(x+0, y+1, 7, grey);
      display->drawFastHLine(x+0, y+2, 7, grey);
      display->drawFastHLine(x+1, y+3, 5, grey);
      display->drawFastHLine(x+2, y+4, 3, yellow);
      display->drawFastHLine(x+1, y+5, 3, yellow);
      display->drawFastHLine(x+2, y+6, 2, yellow);
      break;
    case 6:
      display->drawFastHLine(x+1, y+0, 5, grey);
      display->drawFastHLine(x+0, y+1, 7, grey);
      display->drawFastHLine(x+0, y+2, 7, grey);
      display->drawFastHLine(x+1, y+3, 5, grey);
      display->drawPixel(x+1, y+5, lblue);
      display->drawPixel(x+4, y+4, lblue);
      display->drawPixel(x+2, y+6, lblue);
      display->drawPixel(x+5, y+6, lblue);
      break;
  }
}

String getTodayString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 2000)) return "TODAY";
  const char* days[]   = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
  const char* months[] = {"JAN","FEB","MAR","APR","MAY","JUN",
                          "JUL","AUG","SEP","OCT","NOV","DEC"};

  char buf[16];
  snprintf(buf, sizeof(buf), "%s %s %d",
           days[timeinfo.tm_wday], months[timeinfo.tm_mon], timeinfo.tm_mday);
  return String(buf);
}

void displayWeatherScreen0() {
  display->clearScreen();
  String dateStr = getTodayString();
  int dateX = max(0, (64 - (int)dateStr.length() * 6) / 2);
  display->setCursor(dateX, 0);
  display->setTextColor(display->color565(255, 255, 255));
  display->print(dateStr);
  display->drawFastHLine(0, 8, 64, display->color565(80, 80, 80));

  display->setCursor(0, 10);
  display->setTextColor(display->color565(255, 0, 0));
  display->print("HI");
  display->setCursor(8, 10); display->print(":");
  display->setCursor(12, 10);
  display->setTextColor(display->color565(255, 0, 0));
  display->print((int)tempHigh);

  display->setCursor(34, 10);
  display->setTextColor(display->color565(80, 160, 255));
  display->print("LO");
  display->setCursor(44, 10); display->print(":");
  display->setCursor(48, 10);
  display->setTextColor(display->color565(80, 160, 255));
  display->print((int)tempLow);

  display->drawFastHLine(0, 18, 64, display->color565(60, 60, 60));

  const char* labels[] = { "9", "N", "3", "6" };
  int         codes[]  = { hourCode8am, hourCode12pm, hourCode3pm, hourCode6pm };
  for (int i = 0; i < 4; i++) {
    int colX   = i * 16;
    display->setCursor(colX, 19);
    display->setTextColor(display->color565(100, 100, 100));
    display->print(labels[i]);
    drawWeatherIcon(colX + (16 - 7) / 2 + 3, 24, weatherIconType(codes[i]));
  }

  drawInboxPixels();
}

void displayWeatherScreen1() {

  display->clearScreen();

  uint16_t divCol   = display->color565(45, 50, 75);
  uint16_t labelCol = display->color565(110, 115, 140);

  // ── UV bars (left) ────────────────────────────────────────────────────────
  display->setCursor(2, 4);
  display->setTextColor(labelCol);
  display->print("UV");
  display->drawFastVLine(14, 0, 12, divCol);
  display->drawFastHLine(14, 11, 12, divCol);
  uint16_t uvCol = display->color565(255, 255, 0);
  for (int i = 0; i < 10; i++) {
    int height = min(11, (int)hourlyUV[i]);
    if (height > 0) display->fillRect(15 + i, 11 - height, 1, height, uvCol);
  }

  // ── Rain bars (right) ─────────────────────────────────────────────────────
  display->setCursor(27, 4);
  display->setTextColor(labelCol);
  display->print("RA");
  display->setCursor(38, 4);
  display->print("I"); 
  display->setCursor(42, 4);
  display->print("N");  
  display->drawFastVLine(49, 0, 12, divCol);
  display->drawFastHLine(49, 11, 12, divCol);
  uint16_t rainCol = display->color565(0, 150, 255);
  for (int i = 0; i < 10; i++) {
    int height = min(11, (int)round(hourlyRain[i] * 55));
    if (height > 0) display->fillRect(50 + i, 11 - height, 1, height, rainCol);
  }

  display->drawFastHLine(0, 13, 64, divCol);

  // ── Humidity & Wind ───────────────────────────────────────────────────────
  display->setCursor(0, 15);
  display->setTextColor(labelCol);
  display->print("HUM");
  display->setCursor(18, 15);
  display->setTextColor(display->color565(100, 180, 255));
  display->print((int)humidity);
  display->print("%");

  display->setCursor(36, 15);
  display->setTextColor(labelCol);
  display->print("WND");
  char wbuf[8];
  snprintf(wbuf, sizeof(wbuf), "%d", (int)round(windSpeed));
  display->setCursor(53, 15);
  display->setTextColor(display->color565(150, 200, 255));
  display->print(wbuf);

  // ── Gusts & Daily Rain ────────────────────────────────────────────────────
  display->setCursor(2, 25);
  display->setTextColor(labelCol);
  display->print("GST");
  snprintf(wbuf, sizeof(wbuf), "%d", (int)round(windGusts));
  display->setCursor(20, 25);
  display->setTextColor(display->color565(255, 160, 60));
  display->print(wbuf);

  display->setCursor(35, 25);
  display->setTextColor(labelCol);
  display->print("RAIN");
  display->setCursor(56, 25);
  if (precipitation < 0.01) {
    display->setTextColor(display->color565(70, 70, 90));
    display->print("--");
  } else {
    display->setTextColor(display->color565(120, 190, 255));
    char rbuf[6];
    snprintf(rbuf, sizeof(rbuf), "%.2f", precipitation);
    display->print(rbuf);
  }

  drawInboxPixels();
}

void displayWeather() {
  if (weatherScreen == 0) displayWeatherScreen0();
  else                    displayWeatherScreen1();
}