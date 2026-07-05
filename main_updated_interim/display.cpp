#include "display.h"
#include "inbox.h"
#include "transitions.h"
#include "api.h"
#include "Digit.h"
#include "Digitsec.h"
#include "weather.h"

void parseHexColor(String hex, uint8_t &r, uint8_t &g, uint8_t &b) {
  hex.trim();
  if (hex.length() < 6) { r=255; g=255; b=255; return; }
  r = (uint8_t) strtol(hex.substring(0, 2).c_str(), NULL, 16);
  g = (uint8_t) strtol(hex.substring(2, 4).c_str(), NULL, 16);
  b = (uint8_t) strtol(hex.substring(4, 6).c_str(), NULL, 16);
}

void displayClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 500)) return;

  int hour   = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  bool isPM  = (hour >= 12);
  int  hour12 = hour % 12;
  if (hour12 == 0) hour12 = 12;

  if (clockMode == 0) {
    // ── Date line at top ─────────────────────────────────────────────────
    String dateStr = getTodayString(); // reuse from weather.h
    display->setCursor(max(0, (64 - (int)dateStr.length() * 6) / 2), 0);
    display->setTextColor(display->color565(130, 130, 160));
    display->print(dateStr);
    //display->drawFastHLine(0, 7, 64, display->color565(40, 40, 60));

    // ── Morphing digits – centered in rows 8–31 ──────────────────────────
    // Each big digit: segWidth=6 → rendered width = segWidth+2 = 8px
    // Layout: H1(x=1) H2(x=10) colon(x=19) M1(x=22) M2(x=31)  total≈39px, center offset = (64-39)/2 = 12
    // Seconds (small): rendered width ~10px, placed right edge
    static Digit d0(display, 0,  1+12-8, 9, display->color565(255,255,255));
    static Digit d1(display, 0, 10+12-8, 9, display->color565(255,255,255));
    static Digit d2(display, 0, 22+12-8, 9, display->color565(255,255,255));
    static Digit d3(display, 0, 31+12-8, 9, display->color565(255,255,255));
    static Digitsec s0(display, 0, 45, 15, display->color565(255,255,255));
    static Digitsec s1(display, 0, 54, 15, display->color565(255,255,255));

    static bool firstRun = true;

    int h1 = hour12 / 10, h2 = hour12 % 10;
    int m1 = minute  / 10, m2 = minute  % 10;
    int sec1 = timeinfo.tm_sec / 10, sec2 = timeinfo.tm_sec % 10;

    if (firstRun) {
        // On first call, draw directly without morphing
        d0.Draw(h1); d1.Draw(h2);
        d2.Draw(m1); d3.Draw(m2);
        s0.Draw(sec1); s1.Draw(sec2);
        firstRun = false;
    } else {
        if (h1 != d0.Value()) d0.Morph(h1); else d0.Draw(d0.Value());
        if (h2 != d1.Value()) d1.Morph(h2); else d1.Draw(d1.Value());
        if (m1 != d2.Value()) d2.Morph(m1); else d2.Draw(d2.Value());
        if (m2 != d3.Value()) d3.Morph(m2); else d3.Draw(d3.Value());
        if (sec1 != s0.Value()) s0.Morph(sec1); else s0.Draw(s0.Value());
        if (sec2 != s1.Value()) s1.Morph(sec2); else s1.Draw(s1.Value());
    }

    // Colon between hours and minutes (x=20, rows 10–24)
    display->fillRect(20+11-8, 11, 2, 3, display->color565(255,255,255));
    display->fillRect(20+11-8, 17, 2, 3, display->color565(255,255,255));


    //display->drawFastHLine(0, 18, 64, display->color565(60, 60, 60));

    const char* labels[] = { "9", "N", "3", "6" };
    int         codes[]  = { hourCode8am, hourCode12pm, hourCode3pm, hourCode6pm };
    for (int i = 0; i < 4; i++) {
      int colX   = i * 16;
      display->setCursor(colX, 24);
      display->setTextColor(display->color565(100, 100, 100));
      display->print(labels[i]);
      drawWeatherIcon(colX + (16 - 7) / 2 + 3, 24, weatherIconType(codes[i]));
    }
  }

  else { // Night mode: simple red clock, hour and minute only
    display->clearScreen();

    char timeBuf[6];
    snprintf(timeBuf, sizeof(timeBuf), "%d:%02d", hour12, minute);
    int timeLen = strlen(timeBuf);
    int timeX   = max(0, (64 - timeLen * 12) / 2);

    display->setTextSize(2);
    display->setTextColor(display->color565(255, 0, 0)); // Red
    display->setCursor(timeX, 8);
    display->print(timeBuf);
    display->setTextSize(1);
  }

  drawInboxPixels();
}

int wordWrapLines(const String& msg, int maxChars, String lines[], int maxLines) {
  int lineCount = 0, start = 0, len = msg.length();
  while (start < len && lineCount < maxLines) {
    if ((len - start) <= maxChars) { lines[lineCount++] = msg.substring(start); break; }
    int end = start + maxChars, spacePos = -1;
    for (int i = end; i >= start; i--) { if (msg.charAt(i) == ' ') { spacePos = i; break; } }
    if (spacePos == -1) { lines[lineCount++] = msg.substring(start, end); start = end; }
    else { lines[lineCount++] = msg.substring(start, spacePos); start = spacePos + 1; }
  }
  return lineCount;
}

void scrollMessage(String msg) {
  int msgWidth = msg.length() * 6, startX = 64, endX = -msgWidth;
  for (int x = startX; x > endX; x -= 1) {
    server.handleClient();
    if (activeMode != 3) return;
    display->clearScreen();
    display->setTextColor(display->color565(255, 255, 255));
    display->setCursor(x, 12);
    display->print(msg);
    drawInboxPixels();
    delay(45);
  }
  display->clearScreen();
  activeMode = 0;
}

void handleDisplayMessage() {
  setCORS();
  activeMode = 0;
  String msg = server.arg("message");
  Serial.println("Display: " + msg);
  display->clearScreen();
  display->setTextColor(display->color565(255, 255, 255));

  const int CHARS_PER_LINE = 10;
  if (msg.length() <= 5) {
    display->setTextSize(2);
    int x = max(0, (64 - (int)msg.length() * 12) / 2);
    display->setCursor(x, 8); display->print(msg);
    display->setTextSize(1);
  } else if (msg.length() <= CHARS_PER_LINE) {
    int x = max(0, (64 - (int)msg.length() * 6) / 2);
    display->setCursor(x, 12); display->print(msg);
  } else if (msg.length() <= CHARS_PER_LINE * 3) {
    String lines[3];
    int nLines = wordWrapLines(msg, CHARS_PER_LINE, lines, 3);
    int startY = max(0, (32 - nLines * 8) / 2);
    for (int i = 0; i < nLines; i++) {
      int x = max(0, (64 - (int)lines[i].length() * 6) / 2);
      display->setCursor(x, startY + i * 8); display->print(lines[i]);
    }
  } else {
    activeMode = 3;
    server.send(200, "application/json", "{\"status\":\"message scrolling\"}");
    scrollMessage(msg);
    return;
  }
  drawInboxPixels();
  server.send(200, "application/json", "{\"status\":\"message received\"}");
}