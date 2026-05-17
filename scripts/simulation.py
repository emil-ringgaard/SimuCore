import os
import sys
import time
import asyncio
import subprocess
import tempfile
import threading
from pathlib import Path
from simucore_simulation import SimuCoreSystem

STATIC_FOLDER = os.path.join(Path(__file__).parent.parent, "simucore-frontend/dist")

# Server script as string
SERVER_SCRIPT = """
import os
import sys
from flask import Flask, send_from_directory, send_file

STATIC_FOLDER = r"{static_folder}"

app = Flask(__name__)

@app.route("/")
def serve_react_app():
    return send_file(os.path.join(STATIC_FOLDER, "index.html"))

@app.route("/<path:path>")
def serve_static_files(path):
    if os.path.exists(os.path.join(STATIC_FOLDER, path)):
        return send_from_directory(STATIC_FOLDER, path)
    return send_file(os.path.join(STATIC_FOLDER, "index.html"))

if __name__ == "__main__":
    print("Flask server starting...")
    try:
        app.run(host="0.0.0.0", port=8085, use_reloader=False)
    except KeyboardInterrupt:
        print("Flask server shutting down...")
        sys.exit(0)
""".format(static_folder=STATIC_FOLDER)


def start_simulation_framework(source, target, env):
    env.Execute("$PYTHONEXE -m pip install flask websockets tenacity")
    print("Starting simulation framework")
    # Write server script to temporary file
    with tempfile.NamedTemporaryFile(mode="w", suffix=".py", delete=False) as f:
        f.write(SERVER_SCRIPT)
        server_script_path = f.name

    try:
        # Start subprocess - this is more reliable for signal handling
        process = subprocess.Popen(
            [sys.executable, server_script_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        print("Press Ctrl+C to stop...")
        embedded_simucore_system = subprocess.Popen(
            Path(env["PROJECT_BUILD_DIR"])
            .joinpath(Path(env["PIOENV"]))
            .joinpath("program")
        )

        def run_simucore():
            asyncio.run(SimuCoreSystem().start())

        simucore_thread = threading.Thread(target=run_simucore, daemon=True)
        simucore_thread.start()
        while process.poll() is None:
            pass

    except KeyboardInterrupt:
        print(f"\nStopping server process {process.pid}...")
        try:
            # loop.close()
            process.terminate()
            process.wait(timeout=5)
            embedded_simucore_system.terminate()
            embedded_simucore_system.wait(timeout=5)
        except subprocess.TimeoutExpired:
            print("Force killing server...")
            process.kill()
            # loop.close()
            process.wait()
            embedded_simucore_system.kill()
            embedded_simucore_system.wait()

    except Exception as e:
        print(f"Error: {e}")
        if process.poll() is None:
            process.terminate()
            # loop.close()
            embedded_simucore_system.kill()

    finally:
        # Clean up temporary file
        try:
            os.unlink(server_script_path)
        except OSError:
            pass

    print("Server stopped")
