from flask import Blueprint, request, jsonify
from config.app_config import AppConfig
from config.sports_config import SportsConfig

from dataclasses import asdict
from datetime import date
from threading import Lock

from utils.sports_utils import (
    Sport,
    update_sport_data,
    resolve_game_hierarchy,
)

sports_bp = Blueprint('sports', __name__)
cfg = SportsConfig()

SPORTS = [Sport(**s) for s in cfg.sports]
TEAM_PRIORITY = [Sport(**s) for s in cfg.team_priority]
FALLBACK_LEAGUES = cfg.fallback_leagues
SPORTS_PRIORITY = list(range(len(SPORTS)))
REFRESH_THROTTLE_SECONDS = AppConfig.REFRESH_THROTTLE_SECONDS
 
_state_lock = Lock()
_games_cache: dict = {}
_last_update_ts: float = 0.0
 
 
def _refresh_all(force: bool = False) -> None:
    global _last_update_ts
    import time
 
    with _state_lock:
        if not force and _games_cache and (time.time() - _last_update_ts) < REFRESH_THROTTLE_SECONDS:
            return
 
        today = date.today()
        for i, sport in enumerate(SPORTS):
            _games_cache[i] = update_sport_data(sport, today)
        _last_update_ts = time.time()


@sports_bp.route('/get-scoreboard', methods=['GET'])
def get_hierarchy_game():
    require_live_teams = request.args.get("live_teams", "true").lower() == "true"
    require_live_fallback = request.args.get("live_fallback", "true").lower() == "true"
 
    info, matched_sport = resolve_game_hierarchy(
        SPORTS, FALLBACK_LEAGUES,
        require_live_for_teams=require_live_teams,
        require_live_for_fallback=require_live_fallback,
    )
    if info is None:
        return jsonify({"found": False, "source": "none", "game": None})
    return jsonify({
        "found": True,
        "source": "priority_team" if matched_sport else "popularity_fallback",
        "sport": matched_sport.label if matched_sport else None,
        "game": asdict(info),
    })
 