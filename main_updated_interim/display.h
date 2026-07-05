#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "config.h"

void parseHexColor(String hex, uint8_t &r, uint8_t &g, uint8_t &b);
void displayClock();
int wordWrapLines(const String& msg, int maxChars, String lines[], int maxLines);
void scrollMessage(String msg);
void handleDisplayMessage();

#endif