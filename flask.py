#This code works like a server for connecting the raspberry pi zero with my website to send over orders, for example turn on the camera
from flask import Flask, request
import subprocess
import signal
import os

app = Flask(__name__)
process = None

@app.route('/start', methods=['POST'])
def start():
    global process
    if process is None:
        process = subprocess.Popen(['python3', '/home/filipmomot/robot/kamera.py'])
        return 'Startad', 200
    return 'Är på', 400

@app.route('/stop', methods=['POST'])
def stop():
    global process
    if process is not None:
        os.kill(process.pid, signal.SIGTERM)
        process = None
        return 'Stop', 200
    return 'Har avlsutats', 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
