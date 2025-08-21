import os
import signal
import sys
from pathlib import Path
from multiprocessing import Process

STATIC_FOLDER = os.path.join(Path(__file__).parent.parent, 'simulation-frontend/dist')

def create_app():
    from flask import Flask, send_from_directory, send_file
    app = Flask(__name__)

    @app.route('/')
    def serve_react_app():
        return send_file(os.path.join(STATIC_FOLDER, 'index.html'))

    @app.route('/<path:path>')
    def serve_static_files(path):
        if os.path.exists(os.path.join(STATIC_FOLDER, path)):
            return send_from_directory(STATIC_FOLDER, path)
        return send_file(os.path.join(STATIC_FOLDER, 'index.html'))

    return app

def run_server():
    app = create_app()
    app.run(host='0.0.0.0', port=8082)

def start_simulation_framework(source, target, env):
    env.Execute("$PYTHONEXE -m pip install flask")
    print("Starting simulation framework")

    p = Process(target=run_server)
    p.start()

    try:
        p.join()
    except KeyboardInterrupt:
        print("Stopping simulation framework...")
        p.terminate()
        p.join()
