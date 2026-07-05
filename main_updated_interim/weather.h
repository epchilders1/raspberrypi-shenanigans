#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include "config.h"

void getWeatherData();
int weatherIconType(int code);
void drawWeatherIcon(int x, int y, int type);
String getTodayString();
void displayWeatherScreen0();
void displayWeatherScreen1();
void displayWeather();

#endif