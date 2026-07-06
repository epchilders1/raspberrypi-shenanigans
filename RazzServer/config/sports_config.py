from dataclasses import dataclass, field
from typing import ClassVar, FrozenSet


@dataclass
class SportsConfig:

    ESPN_BASE: ClassVar[str] = "http://site.api.espn.com/apis/site/v2/sports"
    NATIONAL_NETWORKS: ClassVar[FrozenSet[str]] = frozenset({
        "ESPN", "ESPN2", "ABC", "FOX", "CBS", "NBC", "TNT", "TBS", "FS1",
        "TELE", "TELEMUNDO", "PEACOCK",
    })

    sports: list = field(default_factory=lambda: [
        {"label": "NFL",   "path": "football/nfl",                       "team_abbr": "SEA"},
        {"label": "NCAAF", "path": "football/college-football",          "team_abbr": "ALA"},
        {"label": "NCAAB", "path": "basketball/mens-college-basketball",  "team_abbr": "ALA"},
    ])

    team_priority: list = field(default_factory=lambda: [
        {"label": "Alabama Football",  "path": "football/college-football", "team_abbr": "ALA"},
        {"label": "Seattle Seahawks",  "path": "football/nfl",              "team_abbr": "SEA"},
    ])

    # For most-popular game search
    fallback_leagues: dict = field(default_factory=lambda: {
        "NFL":   "football/nfl",
        "NCAAF": "football/college-football",
        "NBA":   "basketball/nba",
        "NCAAB": "basketball/mens-college-basketball",
        "MLB":   "baseball/mlb",
        "NHL":   "hockey/nhl",
        "WORLD_CUP": "soccer/fifa.world",
        "EPL":        "soccer/eng.1",
        "UCL":        "soccer/uefa.champions",
        "LA_LIGA":    "soccer/esp.1",
        "BUNDESLIGA": "soccer/ger.1",
        "SERIE_A":    "soccer/ita.1",
        "MLS":        "soccer/usa.1",
    })