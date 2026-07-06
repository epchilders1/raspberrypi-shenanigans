"""
sports_utils.py

Pure data-fetching / parsing helpers for the ESPN scoreboard API.
No Flask, no global state, no display code — just functions that take
inputs and return data, so they're easy to unit test and reuse.
"""

from dataclasses import dataclass, field
from datetime import date, timedelta
from typing import Optional
import time
import requests

from config.sports_config import SportsConfig


@dataclass
class Sport:
    label: str        # e.g. "NCAAB", "NFL"
    path: str          # ESPN API path, e.g. "basketball/mens-college-basketball"
    team_abbr: str      # team abbreviation to track, e.g. "ALA"


@dataclass
class GameInfo:
    found: bool = False
    home_abbr: str = ""
    away_abbr: str = ""
    home_name: str = ""
    away_name: str = ""
    home_score: int = 0
    away_score: int = 0
    state: str = ""            # "pre" | "in" | "post"
    date: str = ""
    short_name: str = ""
    slogan: str = ""
    start_time: str = ""
    situation: str = ""
    status_str: str = ""
    end_time: float = 0.0      # epoch seconds, set when game goes final

    next_found: bool = False
    next_home_abbr: str = ""
    next_away_abbr: str = ""
    next_date: str = ""
    next_status_str: str = ""

    last_found: bool = False
    last_home_abbr: str = ""
    last_away_abbr: str = ""
    last_home_score: int = 0
    last_away_score: int = 0


def fmt_date(d: date) -> str:
    """Equivalent of fmtDate(): YYYYMMDD."""
    return d.strftime("%Y%m%d")


TEAM_SLOGANS = {
    "ALA": "ROLL TIDE",
    "SEA": "GO SEAHAWKS",
}


def get_team_slogan(home: str, away:str) -> str:
    if(home in TEAM_SLOGANS or away in TEAM_SLOGANS):
        return TEAM_SLOGANS[home]
    return {home: "GO TEAM!"}


def find_team_in_events(events: list, abbr: str) -> int:
    """Equivalent of findTeamInEvents(). Returns index or -1."""
    target = abbr.upper()
    for i, event in enumerate(events):
        competitions = event.get("competitions") or []
        if not competitions:
            continue
        competitors = competitions[0].get("competitors") or []
        for c in competitors:
            if c.get("team", {}).get("abbreviation", "").upper() == target:
                return i
    return -1
 
 
def espn_get_team_schedule(sport: Sport, retries: int = 2, timeout: float = 8.0) -> Optional[dict]:
    """
    Fetches a team's full schedule in a single request:
      {ESPN_BASE}/{sport.path}/teams/{team_abbr}/schedule
 
    This replaces the old approach of scanning day-by-day through the
    scoreboard endpoint (which took up to 35 separate requests to find a
    single past NCAAB game). One call here returns the whole season.
 
    Uses sport.team_abbr directly in the URL — ESPN's site API generally
    accepts either a numeric team ID or an abbreviation for this route.
    If a particular league doesn't resolve by abbreviation, look up the
    numeric team ID once via the `/teams` endpoint and swap it in.
    """
    if not sport.team_abbr:
        return None
 
    url = f"{SportsConfig.ESPN_BASE}/{sport.path}/teams/{sport.team_abbr.lower()}/schedule"
    headers = {"Accept": "application/json"}
 
    for attempt in range(retries):
        try:
            resp = requests.get(url, headers=headers, timeout=timeout)
            if resp.status_code == 200:
                return resp.json()
        except requests.RequestException:
            pass
        time.sleep(0.25)
    return None
 
 
def _event_date(event: dict) -> Optional[date]:
    """Pulls the calendar date (ignoring time) out of an event's ISO date string."""
    raw = event.get("date", "")
    if len(raw) < 10:
        return None
    try:
        return date(int(raw[0:4]), int(raw[5:7]), int(raw[8:10]))
    except ValueError:
        return None
 
 
def _event_state(event: dict) -> str:
    comp = (event.get("competitions") or [{}])[0]
    return comp.get("status", {}).get("type", {}).get("state", "")
 
 
# ─────────────────────────────────────────────────────────────────────────────
#  NEXT / LAST GAME SEARCH (schedule-based, sport-agnostic)
# ─────────────────────────────────────────────────────────────────────────────
 
def find_next_game(sport: Sport, today: Optional[date] = None) -> Optional[GameInfo]:
    """
    Returns the tracked team's next upcoming (state == "pre") game after
    `today`, pulled from a single team-schedule fetch. Works the same
    way regardless of sport — no more NCAAB-specific date-range branch.
    """
    today = today or date.today()
    data = espn_get_team_schedule(sport)
    if not data:
        return None
 
    upcoming = []
    for event in data.get("events") or []:
        event_date = _event_date(event)
        if event_date is None or event_date < today:
            continue
        if _event_state(event) == "pre":
            upcoming.append((event_date, event))
 
    if not upcoming:
        return None
    upcoming.sort(key=lambda pair: pair[0])
    return parse_event(upcoming[0][1])
 
 
def find_last_game(sport: Sport, today: Optional[date] = None) -> Optional[GameInfo]:
    """
    Returns the tracked team's most recent completed (state == "post")
    game before `today`, pulled from a single team-schedule fetch.
    Works the same way regardless of sport.
    """
    today = today or date.today()
    data = espn_get_team_schedule(sport)
    if not data:
        return None
 
    completed = []
    for event in data.get("events") or []:
        event_date = _event_date(event)
        if event_date is None or event_date > today:
            continue
        if _event_state(event) == "post":
            completed.append((event_date, event))
 
    if not completed:
        return None
    completed.sort(key=lambda pair: pair[0], reverse=True)
    return parse_event(completed[0][1])

def update_sport_data(sport: Sport, today: Optional[date] = None) -> GameInfo:
    """
    Fetches today's game (if any), and depending on state, the next
    upcoming game and/or the last completed game for the tracked team.
    """
    today = today or date.today()
    info = GameInfo()
 
    data = espn_get_scoreboard(sport, fmt_date(today), limit=0)
    if data:
        events = data.get("events") or []
        idx = find_team_in_events(events, sport.team_abbr)
        if idx >= 0:
            parsed = parse_event(events[idx])
            if parsed:
                info = parsed
                info.found = True
 
    if not info.found or info.state == "post":
        next_info = find_next_game(sport, today)
        if next_info:
            info.next_found = True
            info.next_home_abbr = next_info.home_abbr
            info.next_away_abbr = next_info.away_abbr
            info.next_date = next_info.date[5:10]  # MM-DD slice, matches original
            info.next_status_str = next_info.status_str
 
    last_info = find_last_game(sport, today)
    if last_info:
        info.last_found = True
        info.last_home_abbr = last_info.home_abbr
        info.last_away_abbr = last_info.away_abbr
        info.last_home_score = last_info.home_score
        info.last_away_score = last_info.away_score
 
    return info

def parse_event(event: dict) -> Optional[GameInfo]:
    """Equivalent of parseEvent()."""
    competitions = event.get("competitions") or []
    if not competitions:
        return None
    comp = competitions[0]
    competitors = comp.get("competitors") or []
    status = comp.get("status") or {}
    if len(competitors) < 2 or not status:
        return None
 
    home = next((c for c in competitors if c.get("homeAway") == "home"
                 or c.get("team", {}).get("homeAway") == "home"), None)
    away = next((c for c in competitors if c.get("homeAway") == "away"
                 or c.get("team", {}).get("homeAway") == "away"), None)
    if home is None:
        home = competitors[0]
    if away is None:
        away = competitors[1]
 
    g = GameInfo()
    g.home_abbr = home.get("team", {}).get("abbreviation", "???")
    g.away_abbr = away.get("team", {}).get("abbreviation", "???")
    g.home_name = home.get("team", {}).get("displayName", "?")
    g.away_name = away.get("team", {}).get("displayName", "?")
    g.home_score = int(home.get("score") or 0)
    g.away_score = int(away.get("score") or 0)
    g.state = status.get("type", {}).get("state", "pre")
    g.date = event.get("date", "")
    g.short_name = event.get("shortName", "")
 
    g.slogan = get_team_slogan(g.home_abbr, g.away_abbr)
 
    # Start time: UTC -> local, assumes UTC-6 (same simplification as source)
    raw_date = event.get("date", "")
    if len(raw_date) > 16:
        hour = int(raw_date[11:13])
        minute = int(raw_date[14:16])
        hour = (hour + 18) % 24  # UTC-6 shift
        ampm = "PM" if hour >= 12 else "AM"
        d_hour = 12 if hour % 12 == 0 else hour % 12
        g.start_time = f"{d_hour}:{minute:02d}{ampm}"
 
    # Situation (down/distance or outs), only while live
    g.situation = ""
    if g.state == "in":
        situation = comp.get("situation") or {}
        down_dist = situation.get("downDistanceText") or ""
        if down_dist and down_dist != "null":
            g.situation = down_dist
        else:
            outs = situation.get("outs", -1)
            if outs != -1:
                g.situation = f"{outs} OUTS"
 
    short_detail = status.get("type", {}).get("shortDetail", "")
    clock = status.get("displayClock", "")
    period = status.get("period", 0)
 
    if g.state == "post":
        g.status_str = "FINAL"
        g.end_time = time.time()
    else:
        g.end_time = 0.0
        if g.state == "in":
            g.status_str = short_detail[:8] if short_detail else f"P{period} {clock}"
        else:
            g.status_str = short_detail or "PRE"
 
    return g
def _event_popularity_score(event: dict) -> float:
    """
      - live games >> completed >> upcoming
      - ranked teams involved (lower rank number = bigger bonus)
      - close score margin (nail-biters score higher than blowouts)
      - nationally televised games get a small bump
    """
    score = 0.0
    competitions = event.get("competitions") or [{}]
    comp = competitions[0]
    status = comp.get("status", {})
    state = status.get("type", {}).get("state", "")
 
    if state == "in":
        score += 1000.0
    elif state == "post":
        score += 100.0
 
    competitors = comp.get("competitors") or []
    ranks = []
    point_scores = []
    for c in competitors:
        rank = (c.get("curatedRank") or {}).get("current")
        if isinstance(rank, int) and 0 < rank < 99:  # ESPN uses 99 for "unranked"
            ranks.append(rank)
        try:
            point_scores.append(int(c.get("score") or 0))
        except (TypeError, ValueError):
            point_scores.append(0)
 
    if ranks:
        score += 500.0 / min(ranks)
 
    if state == "in" and len(point_scores) == 2:
        margin = abs(point_scores[0] - point_scores[1])
        score += max(0, 50 - margin)  
 
    networks = set()
    for b in comp.get("broadcasts") or []:
        for name in b.get("names", []):
            networks.add(name.upper())
    if networks & SportsConfig.NATIONAL_NETWORKS:
        score += 50.0
 
    return score
def espn_get_scoreboard(sport: Sport, dates: str, limit: int = 100,
                         retries: int = 2, timeout: float = 8.0) -> Optional[dict]:
    """
    Returns the parsed JSON dict, or None if all attempts fail.
 
    Python/requests gets the full JSON response (no manual field
    filtering needed like the ESP32's buildFilter() — that existed only
    to save RAM on the microcontroller).
    """
    url = f"{SportsConfig.ESPN_BASE}/{sport.path}/scoreboard?dates={dates}"
    if limit > 0:
        url += f"&limit={limit}"
 
    headers = {"Accept": "application/json"}
 
    for attempt in range(retries):
        try:
            resp = requests.get(url, headers=headers, timeout=timeout)
            if resp.status_code == 200:
                return resp.json()
        except requests.RequestException:
            pass
        time.sleep(0.25)
    return None
 
def find_most_popular_game(fallback_leagues: dict, today: Optional[date] = None,
                            require_live: bool = True) -> Optional[GameInfo]:
    """
    League-wide fallback: scans every league in `fallback_leagues`
    (dict of label -> ESPN path, e.g. {"NFL": "football/nfl"}) for
    today's games and returns the single highest-scoring one.
 
    require_live=True only considers games currently in progress; set to
    False if you'd rather also surface a marquee upcoming/finished game
    when nothing is live.
    """
    today = today or date.today()
    best_event, best_score = None, -1.0
 
    for label, path in fallback_leagues.items():
        league_sport = Sport(label=label, path=path, team_abbr=None)
        data = espn_get_scoreboard(league_sport, fmt_date(today), limit=100)
        if not data:
            continue
        for event in data.get("events") or []:
            if require_live:
                comp = (event.get("competitions") or [{}])[0]
                state = comp.get("status", {}).get("type", {}).get("state", "")
                if state != "in":
                    continue
            s = _event_popularity_score(event)
            if s > best_score:
                best_score, best_event = s, event
 
    return parse_event(best_event) if best_event else None
 
 
def resolve_game_hierarchy(team_priority: list, fallback_leagues: dict,
                            today: Optional[date] = None,
                            require_live_for_teams: bool = False,
                            require_live_for_fallback: bool = True):
    """
    1. Walks `team_priority` (an ordered list of Sport objects, each with
       a team_abbr) in order and returns the first team that has a game
       today. Set require_live_for_teams=True to only count a team as
       "active" while its game is actually in progress (state == "in"),
       rather than any time it has a game scheduled/finished today.
    2. If none of the priority teams qualify, scans `fallback_leagues`
       for the most popular live game across those leagues.
 
    Returns a (GameInfo | None, Sport | None) tuple. The Sport is None
    when the result came from the popularity fallback rather than a
    tracked team.
    """
    today = today or date.today()
 
    for sport in team_priority:
        info = update_sport_data(sport, today)
        if info.found and (not require_live_for_teams or info.state == "in"):
            return info, sport
 
    popular = find_most_popular_game(fallback_leagues, today, require_live=require_live_for_fallback)
    return popular, None