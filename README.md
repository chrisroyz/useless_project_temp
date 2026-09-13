<img width="1280" height="640" alt="git (1)" src="https://github.com/user-attachments/assets/8920b256-2ba8-4988-b824-5351134eb4bd" />

# OTA BASED ROVER 🎯

## Basic Details
### Team Name: POPPINS

### Team Members
- Team Lead: Albin Chacko - Saintgits College of Engineering (Autonomous)
- Member 2: Chris Roy Skariah - Saintgits College of Engineering (Autonomous)

### Project Description
A 4-wheeled rover you drive over WiFi with WASD on your keyboard. No app, no remote, no real purpose — just a laptop, a UDP socket, and a tank-steered box on wheels doing exactly what a $10 RC controller already does, but with extra steps.

### The Problem (that doesn't exist)
Physical RC remotes are far too convenient. A joystick, a trigger, done. Where's the fun in that? Also, "Over The Air" sounded cool and we wanted an excuse to use the term for something that isn't actually updating firmware over the air.

### The Solution (that nobody asked for)
We replaced a perfectly good RC remote with a pygame window on a laptop that has to stay in focus, a UDP packet stream running at 20Hz, and a tiny handshake-and-retry protocol just to toggle a headlight. The rover connects to WiFi, listens for JSON packets, and mixes throttle + steering into tank-drive commands for two motors via an L298N driver. Driving controls are unreliable-but-fast (fire-and-forget streaming), while the headlight toggle gets the "important" treatment with sequence numbers, acknowledgements, and retries — because obviously the lights matter more than not crashing into a wall.

## Technical Details
### Technologies/Components Used
For Software:
- Languages: C++ (Arduino), Python
- Libraries: ESP8266WiFi, WiFiUdp, ArduinoJson (firmware); socket, json, pygame (controller)
- Tools: Arduino IDE, VS Code

For Hardware:
- NodeMCU ESP8266 board
- L298N dual H-bridge motor driver
- 2x DC gear motors (tank/differential drive, 4 wheels)
- Micro servo motor
- 4WD chassis with yellow wheels
- Battery pack + jumper wires

### Implementation
For Software:

**Firmware (`.ino`, flashed to the NodeMCU):**
- Connects to WiFi (`Equinoxi`) and listens on UDP port `5005`
- Parses incoming JSON packets with ArduinoJson
- `"type": "stream"` packets carry `thr` (throttle) and `str` (steering, -100 to 100), which are mixed into left/right wheel speeds (tank steering) and sent to the L298N via PWM
- `"type": "cmd"` packets (e.g. headlight toggle) are sequence-numbered; the rover ACKs every command it receives and only executes new sequence numbers, so a command can't accidentally fire twice

**Controller (`rc.py`, run on a laptop):**
- Opens a small pygame window (needed on Windows to capture key states) titled "RC Car Teleop (WASD)"
- Reads held keys every loop: `W/S` → throttle, `A/D` → steering, streamed to the rover 20 times a second, no acknowledgement needed
- `SPACE` toggles the headlights as a *reliable* command — sent with a sequence number and retried up to 5 times until the rover ACKs it
- `ESC` or closing the window stops the program and sends a zero-throttle command so the rover doesn't drive off into the sunset unattended

# Installation
```bash
# Controller (PC)
pip install pygame

# Firmware (Arduino IDE)
# Install board support: ESP8266 by ESP8266 Community
# Install library: ArduinoJson (by Benoit Blanchon)
# Update ssid / password in the .ino file, then flash to the NodeMCU
```

# Run
```bash
# 1. Power on the rover and note the IP address printed to Serial Monitor
# 2. Update CAR_ADDRESS in rc.py to match that IP
# 3. Run the controller
python rc.py

# Keep the small pygame window focused, then:
# W/A/S/D = drive   SPACE = toggle headlights   ESC = quit
```

### Project Documentation
For Hardware:

# Schematic & Circuit
*Add your circuit diagram here — L298N ENA/IN1/IN2 to D7/D1/D2, ENB/IN3/IN4 to D8/D5/D6 on the NodeMCU.*

# Build Photos
![Rover](rover.jpg)
*The assembled rover: NodeMCU on top of the L298N driver, tank-steered 4-wheel chassis, and a micro servo wired in for future use.*

![Controller](controller.png)
*The WASD teleop controller window — a deliberately tiny, boring pygame window that just needs to stay focused to capture keystrokes.*

### Project Demo
# Video
[Add your demo video link here]
*Demonstrates driving the rover around with WASD and toggling the headlights with SPACE.*

# Additional Demos
[Add any extra demo materials/links]

## Team Contributions
- Albin Chacko: Rover hardware assembly, motor driver wiring, ESP8266 firmware (UDP listener, motor mixing, ACK/retry logic)
- Chris Roy Skariah: Python WASD controller (`rc.py`), UDP streaming/reliable command protocol, testing and integration

---
Made with ❤️ at TinkerHub Useless Projects

![Static Badge](https://img.shields.io/badge/TinkerHub-24?color=%23000000&link=https%3A%2F%2Fwww.tinkerhub.org%2F)
![Static Badge](https://img.shields.io/badge/UselessProjects--26-26?link=https%3A%2F%2Ftinkerhub.org%2Fevents%2F1M8ORET9A1%2Fuseless-projects-3.0)
