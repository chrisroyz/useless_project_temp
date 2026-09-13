# RC Car Teleoperation System

A Python and ESP8266-based RC car teleoperation system with a Skid-Steer (Tank Drive) setup.

## 🎥 Demo
[Watch the RC Car in action!](https://drive.google.com/file/d/1298T6hJaPL8kEGePPkBc9YoJcaaSTVhx/view?usp=drive_link)

## 🏗️ Architecture & Workflow

The system uses a UDP-based protocol where a Python Pygame application reads keyboard input and sends JSON payloads to an ESP8266 (NodeMCU) over Wi-Fi.

```mermaid
sequenceDiagram
    participant User
    participant PC as Python Client (Pygame)
    participant ESP as ESP8266 NodeMCU
    participant MotorA as L298N (Left Wheels)
    participant MotorB as L298N (Right Wheels)

    User->>PC: Presses W/A/S/D
    PC->>ESP: UDP Stream `{"type": "stream", "thr": 100, "str": 0}`
    ESP->>MotorA: PWM & Direction Pins (Speed: 100)
    ESP->>MotorB: PWM & Direction Pins (Speed: 100)
    Note over MotorA, MotorB: Differential Steering Mixed on ESP
    
    User->>PC: Presses Spacebar
    PC->>ESP: UDP Reliable `{"type": "cmd", "seq": 1, "val": "HEADLIGHTS_TOGGLE"}`
    ESP-->>PC: ACK `{"ack_seq": 1}`
```

```mermaid
graph LR
    A[Python Teleop Script] -->|UDP Port 5005| B(ESP8266 NodeMCU)
    B -->|ENA, IN1, IN2| C[L298N Motor Driver]
    B -->|ENB, IN3, IN4| C
    C -->|Motor A| D[Left Wheels]
    C -->|Motor B| E[Right Wheels]
```

## 🛠️ Technicals
*(Note: Be sure to upload your images (UI, IDE, hardware) into an `images/` folder in the repository so these display correctly once pushed to GitHub.)*

### Teleop UI
![Teleop UI](images/ui.png)

### Arduino IDE Code
![Arduino IDE](images/ide.png)

### Hardware Setup
![Hardware](images/hardware.png)

## 🚀 Running the System
1. Flash `sketch_sep12a.ino` to your NodeMCU.
2. Connect to the same network.
3. Run the Python client: `python rc.py`
4. Make sure the Pygame window is focused and use `WASD` to drive.
