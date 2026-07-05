#ifndef API_H
#define API_H

#include <Arduino.h>
#include "config.h"
#include "sports.h"

void setCORS();
void handleRoot();
void checkSleepSchedule();
void handleDefaultMode();
void handleSendMessage();
void handleGetMessages();
void handleDisplayMessage_Inbox();
void programA();
void programB();
void programDefault();
void programClock();
void programSports();
void boardOff();
void handleSetSleep();
void handleSportsData();

#endif