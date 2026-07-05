from flask import Flask
from config import Config
app = Flask(__name__)
from routers.sports import sports_bp

app.register_blueprint(sports_bp, url_prefix='/api/sports')

@app.route("/")
def ping():
    return "pong"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
    app.run(debug=True)
