#include "sports.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ESPN_BASE  = "http://site.api.espn.com/apis/site/v2/sports";

// ═════════════════════════════════════════════════════════════════════════════
//  MAIN LOGIC
// ═════════════════════════════════════════════════════════════════════════════

bool fetchScoreboardFromApi(GameInfo& out) {
  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(15000);

  String url = "http://192.168.1.112:5000/api/sports/get-scoreboard";

  if (!http.begin(client, url)) return false;
  http.addHeader("Accept", "application/json");

  int code = http.GET();
  if (code != 200) { http.end(); return false; }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) return false;

  if (!doc["found"]) return false;

  JsonObject g = doc["game"];
  out.found      = true;
  out.homeAbbr   = g["home_abbr"]   | "???";
  out.awayAbbr   = g["away_abbr"]   | "???";
  out.homeScore  = g["home_score"]  | 0;
  out.awayScore  = g["away_score"]  | 0;
  out.state      = g["state"]       | "pre";
  out.statusStr  = g["status_str"]  | "";
  out.situation  = g["situation"]   | "";
  out.startTime  = g["start_time"]  | "";
  out.slogan     = g["slogan"]      | "";

  out.nextFound     = g["next_found"];
  out.nextHomeAbbr  = g["next_home_abbr"] | "";
  out.nextAwayAbbr  = g["next_away_abbr"] | "";
  out.nextDate      = g["next_date"]      | "";

  out.lastFound      = g["last_found"];
  out.lastHomeAbbr   = g["last_home_abbr"] | "";
  out.lastAwayAbbr   = g["last_away_abbr"] | "";
  out.lastHomeScore  = g["last_home_score"] | 0;
  out.lastAwayScore  = g["last_away_score"] | 0;

  return true;
}

void updateSportsData(bool forceRefresh) {
  unsigned long now = millis();
  if (!forceRefresh && (now - lastSportsUpdate < 10000) && lastSportsUpdate != 0) return;

  GameInfo info;
  if (fetchScoreboardFromApi(info)) {
    if (info.state == "post") {
      info.endTime = (sportsGames[0].state == "post") ? sportsGames[0].endTime : now;
    }
    sportsGames[0] = info;
    sportsFetchedCount = 1;
  }
  lastSportsUpdate = now;
}

int getActiveSportIndex() {
  // 1. Priority: Live games (in-progress)
  for (int i = 0; i < NUM_SPORTS; i++) {
    int sIdx = sportsPriority[i];
    if (sportsGames[sIdx].found && sportsGames[sIdx].state == "in") return sIdx;
  }

  // 2. Priority: Games that finished in the last hour
  for (int i = 0; i < NUM_SPORTS; i++) {
    int sIdx = sportsPriority[i];
    if (sportsGames[sIdx].found && sportsGames[sIdx].state == "post") {
      // Check if it's been less than 60 minutes (3,600,000 ms)
      if (millis() - sportsGames[sIdx].endTime < 3600000UL) return sIdx;
    }
  }

  // 3. Priority: Upcoming games for today (Pregame)
  for (int i = 0; i < NUM_SPORTS; i++) {
    int sIdx = sportsPriority[i];
    if (sportsGames[sIdx].found && sportsGames[sIdx].state == "pre") return sIdx;
  }

  // 4. Default: Show the next game in the calendar
  for (int i = 0; i < NUM_SPORTS; i++) {
    int sIdx = sportsPriority[i];
    if (sportsGames[sIdx].nextFound) return sIdx;
  }

  return sportsPriority[0];
}

void displaySports() {
  int targetIdx = getActiveSportIndex();
  GameInfo& g = sportsGames[targetIdx];
  const Sport& s = SPORTS[targetIdx];

  display->clearScreen();

  // PREGAME LAYOUT
  if (g.found && g.state == "pre") {
    display->setCursor(0, 0);
    display->setTextColor(display->color565(255, 255, 255));
    display->printf("%s VS. %s", g.awayAbbr.c_str(), g.homeAbbr.c_str());

    display->setCursor(0, 12);
    display->setTextColor(display->color565(0, 200, 255));
    display->printf("STARTS: %s", g.startTime.c_str());

    display->setCursor(0, 24);
    display->setTextColor(display->color565(0, 255, 100));
    display->print(g.slogan);
  }
  // ACTIVE OR RECENTLY FINAL (1 Hour Window)
  else if (g.found && (g.state == "in" || (g.state == "post" && (millis() - g.endTime < 3600000UL)))) {
    display->setCursor(0, 0);
    display->setTextColor(display->color565(255, 255, 255));
    display->printf("%-4s %d", g.awayAbbr.c_str(), g.awayScore);
    
    display->setCursor(0, 10);
    display->printf("%-4s %d", g.homeAbbr.c_str(), g.homeScore);

    display->setCursor(0, 22);
    display->setTextColor(display->color565(255, 215, 0)); 
    display->print(g.statusStr);

    display->setCursor(32, 22);
    display->setTextColor(display->color565(150, 150, 150));
    display->print(g.situation);

  } else if (g.nextFound) {
    display->setCursor(0, 0);
    display->setTextColor(display->color565(0, 200, 255));
    display->print(s.label);
    
    display->setCursor(0, 10);
    display->setTextColor(display->color565(150, 150, 150));
    display->print("NEXT:");
    
    display->setCursor(0, 18);
    display->setTextColor(display->color565(255, 255, 255));
    display->printf("%s @ %s", g.nextAwayAbbr.c_str(), g.nextHomeAbbr.c_str());

    display->setCursor(0, 26);
    display->setTextColor(display->color565(255, 215, 0));
    display->print(g.nextDate);
  } else {
    display->setCursor(0, 16);
    display->setTextColor(display->color565(100, 100, 100));
    display->print("NO GAMES");
  }
}

String sportsDataJson() {
  JsonDocument doc;
  doc["ready"] = (sportsFetchedCount >= 1);
  doc["fetched"] = sportsFetchedCount;
  
  JsonArray gamesArr = doc["games"].to<JsonArray>();

  for (int i=0; i<NUM_SPORTS; i++) {
    JsonObject g = gamesArr.add<JsonObject>();
    g["sport"] = SPORTS[i].label;
    
    g["found"] = sportsGames[i].found;
    g["home"] = sportsGames[i].homeAbbr;
    g["away"] = sportsGames[i].awayAbbr;
    g["homeScore"] = sportsGames[i].homeScore;
    g["awayScore"] = sportsGames[i].awayScore;
    g["status"] = sportsGames[i].statusStr;

    g["nextFound"] = sportsGames[i].nextFound;
    g["nextHome"] = sportsGames[i].nextHomeAbbr;
    g["nextAway"] = sportsGames[i].nextAwayAbbr;
    g["nextDate"] = sportsGames[i].nextDate;

    g["lastFound"] = sportsGames[i].lastFound;
    g["lastHome"] = sportsGames[i].lastHomeAbbr;
    g["lastAway"] = sportsGames[i].lastAwayAbbr;
    g["lastHomeScore"] = sportsGames[i].lastHomeScore;
    g["lastAwayScore"] = sportsGames[i].lastAwayScore;
  }
  
  String out;
  serializeJson(doc, out);
  return out;
}