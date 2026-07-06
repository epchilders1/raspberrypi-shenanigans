#include "sports.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ESPN_BASE  = "http://site.api.espn.com/apis/site/v2/sports";

// ═════════════════════════════════════════════════════════════════════════════
//  HELPER FUNCTIONS
// ═════════════════════════════════════════════════════════════════════════════

bool isNcaab(const Sport& s) { return strcmp(s.label, "NCAAB") == 0; }

String fmtDate(const struct tm& t) {
  char buf[9];
  strftime(buf, sizeof(buf), "%Y%m%d", &t);
  return String(buf);
}

struct tm addDays(struct tm base, int days) {
  time_t tt = mktime(&base);
  tt += (time_t)days * 86400;
  struct tm out;
  localtime_r(&tt, &out);
  return out;
}

// Logic for team-specific slogans based on abbreviations
String getTeamSlogan(String abbr) {
  if (abbr == "ALA") return "ROLL TIDE";
  if (abbr == "SEA") return "GO SEAHAWKS"; // Default Seattle slogan
  return "GO TEAM!";
}

JsonDocument buildFilter() {
  JsonDocument f;
  JsonObject ev   = f["events"][0].to<JsonObject>();
  ev["date"]      = true;
  ev["shortName"] = true;
  JsonObject comp = ev["competitions"][0].to<JsonObject>();
  
  // Added situation for outs/downs
  comp["situation"]["downDistanceText"] = true;
  comp["situation"]["outs"]             = true;

  JsonObject c0   = comp["competitors"][0].to<JsonObject>();
  JsonObject c1   = comp["competitors"][1].to<JsonObject>();
  c0["team"]["abbreviation"] = true;
  c0["team"]["displayName"]  = true;
  c0["team"]["homeAway"]     = true;
  c0["score"]                = true;
  c1["team"]["abbreviation"] = true;
  c1["team"]["displayName"]  = true;
  c1["team"]["homeAway"]     = true;
  c1["score"]                = true;
  
  JsonObject st = comp["status"].to<JsonObject>();
  st["displayClock"]        = true;
  st["period"]              = true;
  st["type"]["state"]       = true;
  st["type"]["shortDetail"] = true;
  return f;
}

bool espnGetScoreboard(const Sport& s, const String& dates, JsonDocument& doc, int limit = 100) {
  String url = String(ESPN_BASE) + "/" + s.path + "/scoreboard?dates=" + dates;
  if (isNcaab(s)) {
    url += "&groups=50";
    if (limit < 200) limit = 200;
  }
  if (limit > 0) url += "&limit=" + String(limit);

  for (int attempt = 0; attempt < 2; attempt++) {
    WiFiClient client;
    HTTPClient http;
    http.useHTTP10(true);
    http.setReuse(false);
    http.setConnectTimeout(8000);
    http.setTimeout(15000);

    if (!http.begin(client, url)) continue;
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");

    int code = http.GET();
    if (code <= 0 || code != 200) { http.end(); delay(250); continue; }

    doc.clear();
    JsonDocument filter = buildFilter();
    DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(25));
    http.end();

    if (!err) return true;
    delay(250);
  }
  return false;
}

bool parseEvent(JsonObject event, GameInfo& out) {
  JsonObject comp = event["competitions"][0];
  if (comp.isNull()) return false;
  JsonArray competitors = comp["competitors"].as<JsonArray>();
  JsonObject st = comp["status"];
  if (competitors.isNull() || competitors.size() < 2 || st.isNull()) return false;

  JsonObject home, away;
  for (JsonObject c : competitors) {
    String ha = c["team"]["homeAway"] | c["homeAway"] | "";
    if (ha == "home") home = c;
    else if (ha == "away") away = c;
  }
  if (home.isNull()) home = competitors[0];
  if (away.isNull()) away = competitors[1];

  out.homeAbbr  = home["team"]["abbreviation"] | "???";
  out.awayAbbr  = away["team"]["abbreviation"] | "???";
  out.homeName  = home["team"]["displayName"]  | "?";
  out.awayName  = away["team"]["displayName"]  | "?";
  out.homeScore = String(home["score"] | "0").toInt();
  out.awayScore = String(away["score"] | "0").toInt();
  out.state     = st["type"]["state"] | "pre";
  out.date      = event["date"] | "";
  out.shortName = event["shortName"] | "";

  // 1. Slogan Logic
  out.slogan = getTeamSlogan(out.homeAbbr == "ALA" || out.awayAbbr == "ALA" ? "ALA" : "SEA");

  // 2. Start Time Parsing (UTC to Local - assumes -6 for Alabama)
  String rawDate = event["date"] | "";
  if (rawDate.length() > 16) {
    int hour = rawDate.substring(11, 13).toInt();
    int min  = rawDate.substring(14, 16).toInt();
    hour = (hour + 18) % 24; // Simple UTC-6 conversion
    String ampm = (hour >= 12) ? "PM" : "AM";
    int dHour = (hour % 12 == 0) ? 12 : hour % 12;
    out.startTime = String(dHour) + ":" + (min < 10 ? "0" : "") + String(min) + ampm;
  }

  // 3. Situation Logic (Downs/Outs)
  out.situation = "";
  if (out.state == "in" && !comp["situation"].isNull()) {
    String downDist = comp["situation"]["downDistanceText"] | "";
    if (downDist != "" && downDist != "null") {
      out.situation = downDist;
    } else {
      int outs = comp["situation"]["outs"] | -1;
      if (outs != -1) out.situation = String(outs) + " OUTS";
    }
  }

  String shortDetail = st["type"]["shortDetail"] | "";
  String clock       = st["displayClock"] | "";
  int    period      = st["period"] | 0;

  if (out.state == "post") {
    out.statusStr = "FINAL";
    if (out.endTime == 0) out.endTime = millis();
  } else {
    out.endTime = 0; // Reset if game is active or pre
    if (out.state == "in") {
      out.statusStr = shortDetail.length() ? shortDetail.substring(0, 8) : ("P" + String(period) + " " + clock);
    } else {
      out.statusStr = shortDetail.length() ? shortDetail : "PRE";
    }
  }
  return true;
}

int findTeamInEvents(JsonArray events, const char* abbr) {
  String target = String(abbr);
  target.toUpperCase();
  for (int i = 0; i < (int)events.size(); i++) {
    JsonObject comp = events[i]["competitions"][0];
    if (comp.isNull()) continue;
    JsonArray competitors = comp["competitors"].as<JsonArray>();
    if (competitors.isNull()) continue;
    for (JsonObject c : competitors) {
      String a = c["team"]["abbreviation"] | "";
      a.toUpperCase();
      if (a == target) return i;
    }
  }
  return -1;
}

bool findNextBySingleDay(const Sport& s, int daysAhead, struct tm g_today, GameInfo& out) {
  for (int d = 1; d <= daysAhead; d++) {
    struct tm day = addDays(g_today, d);
    JsonDocument doc;
    if (!espnGetScoreboard(s, fmtDate(day), doc, 100)) continue;
    JsonArray events = doc["events"].as<JsonArray>();
    if (events.isNull() || events.size() == 0) continue;
    int idx = findTeamInEvents(events, s.teamAbbr);
    if (idx >= 0 && parseEvent(events[idx].as<JsonObject>(), out)) return true;
  }
  return false;
}

bool findLastBySingleDay(const Sport& s, int daysBack, struct tm g_today, GameInfo& out) {
  for (int d = 1; d <= daysBack; d++) {
    struct tm day = addDays(g_today, -d);
    JsonDocument doc;
    if (!(s, fmtDate(day), doc, 100)) continue;
    JsonArray events = doc["events"].as<JsonArray>();
    if (events.isNull() || events.size() == 0) continue;
    for (int i = (int)events.size() - 1; i >= 0; i--) {
      JsonObject ev = events[i].as<JsonObject>();
      JsonArray competitors = ev["competitions"][0]["competitors"].as<JsonArray>();
      if (competitors.isNull()) continue;
      bool hasTeam = false;
      for (JsonObject c : competitors) {
        String a = c["team"]["abbreviation"] | "";
        a.toUpperCase();
        String t = String(s.teamAbbr);
        t.toUpperCase();
        if (a == t) { hasTeam = true; break; }
      }
      if (!hasTeam) continue;
      String state = ev["competitions"][0]["status"]["type"]["state"] | "";
      if (state != "post") continue;
      if (parseEvent(ev, out)) return true;
    }
  }
  return false;
}


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
    sportsGames[0] = info; 
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
  doc["ready"] = (sportsFetchedCount >= NUM_SPORTS);
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