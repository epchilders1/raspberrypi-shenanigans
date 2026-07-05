from flask import Blueprint, request, jsonify

sports_bp = Blueprint('sports', __name__)

@sports_bp.route('/test-route', methods=['GET'])
def test():
    pass