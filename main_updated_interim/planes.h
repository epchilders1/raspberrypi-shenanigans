#ifndef PLANES_H
#define PLANES_H

#include <Arduino.h>
#include "config.h"

float calculateDistance(float lat1, float lon1, float lat2, float lon2);
float calculateBearing(float lat1, float lon1, float lat2, float lon2);
String getCardinalDirection(float bearing);
bool isInSouthernView(float bearing);
float kmToMiles(float km);
String simplifyAircraftType(String raw);
void getAircraftType();
void getPlaneData();
void displayPlane();

#endif