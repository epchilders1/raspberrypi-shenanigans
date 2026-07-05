#ifndef SPORTS_H
#define SPORTS_H

#include <Arduino.h>

// Move Sport struct here so it's defined before config.h uses it
struct Sport {
  const char* label;
  const char* path;
  const char* teamAbbr;
};

struct GameInfo {
    bool   found;
    String homeAbbr, awayAbbr;
    String homeName, awayName;
    int    homeScore, awayScore;
    String state;
    String statusStr;
    String date;
    String shortName;

    // Step 2 Fields
    String startTime;      
    String slogan;         
    String situation;      
    unsigned long endTime; 

    bool   nextFound;
    String nextHomeAbbr, nextAwayAbbr;
    String nextDate; 
    String nextStatusStr;
    String nextState;

    bool   lastFound;
    String lastHomeAbbr, lastAwayAbbr;
    int    lastHomeScore, lastAwayScore;
};

void updateSportsData(bool forceRefresh = false);
void displaySports();
int getActiveSportIndex();
String sportsDataJson();

#endif