# Digital Logic Design Project

<p align="center">
  <img src="https://img.shields.io/badge/Proteus-000000?style=for-the-badge&logo=proteus&logoColor=white" />
  <img src="https://img.shields.io/badge/Arduino C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" />
  <img src="https://img.shields.io/badge/Blynk IoT-24D19b?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Digital%20Logic%20Design%20Project-ED8B00?style=for-the-badge" />
</p>

<p align="center">
  A collection of Digital Logic Design (DLD) Proteus simulation files and automated hardware project from my Computer Science & Engineering journey.
</p>

---

# About The Repository

This repository contains all the **Proteus simulation files** and advanced hardware engineering layouts of the Digital Logic Design Lab experiments I performed during my B.Sc. Engineering in CSE. 

Each folder represents a specific lab assignment containing schematic designs, simulations and comprehensive academic project reports.

---

# Topics Covered

- Combinational Logic Circuits
- Sequential Logic Circuits
- Logic Gates Implementation
- Multiplexer & Demultiplexer
- Encoder & Decoder
- Flip-Flops (D, JK, T, SR)
- Counters & Registers
- Arithmetic Circuits

---

# Project: Smart Railway Gate System

## IoT-Driven Smart Railway Crossing with Fail-Safe Sequential Override

The `SentinelGate_Project/` directory hosts an intelligent automated level-crossing network. Driven by an **ESP32 microcontroller**, the firmware models system behaviors through a strict, deterministic, and non-blocking **Finite State Machine (FSM)** to substitute human fallibility with sensory automation.

---

## System Core Architecture & FSM States

The system continuously evaluates physical environmental inputs and maps them into 6 discrete sequential logic states:

* **IDLE:** Road barriers open (Servos at 90°), Green traffic LED active. System loops to scan incoming transits.

* **WARNING_YELLOW:** Triggers when an authorized train unique ID (UID) is verified. The Yellow LED blinks, and the buzzer sounds a 3-second non-blocking clearing buffer for road traffic.

* **GATES_CLOSING:** Red traffic LED turns active, and both Servo motors rotate to 0° to lock physical barriers shut.

* **TRAIN_INSIDE:** System locks down. Actively tracks down-track parameters while waiting for train clearance.

* **EMERGENCY_OBSTACLE:** If a vehicle is trapped on the track ($<7\text{ cm}$ perimeter), the FSM triggers an immediate hardware override—aborting closure, lifting gates back to 90°, flashing the Red LED, sounding a continuous buzzer alarm, and sending push notifications to mobile devices via Blynk Cloud.

* **GATES_OPENING:** Train exits safely via the opposite RFID reader. System unlocks, resets indicators to Green, and loops back to IDLE.

---

## Hardware Pin Mapping Profile

To implement this architecture, the following physical components are routed to the ESP32:

| Component Module | Hardware Pin Name | Connected ESP32 Pin |
| :--- | :--- | :--- |
| **Shared SPI Bus (RFID 1 & 2)** | SCK / MOSI / MISO | GPIO 18 / GPIO 23 / GPIO 19 |
| **RFID Reader 1 (Entry / Exit A)**| SDA_1 / RST_1 | GPIO 5 / GPIO 22 |
| **RFID Reader 2 (Entry / Exit B)**| SDA_2 / RST_2 | GPIO 4 / GPIO 21 |
| **Ultrasonic Sonar Array 1** | TRIG1 / ECHO1 | GPIO 13 / GPIO 14 |
| **Ultrasonic Sonar Array 2** | TRIG2 / ECHO2 | GPIO 25 / GPIO 26 |
| **Gate Servo Motors (1 & 2)** | SERVO1 / SERVO2 | GPIO 27 / GPIO 32 |
| **Traffic Light Signaling LEDs** | RED / YELLOW / GREEN | GPIO 33 / GPIO 15 / GPIO 16 |
| **Acoustic Warning Indicator** | ACTIVE BUZZER | GPIO 17 |

---

## System Implementation & Circuit Schematic

<p align="center">
  <img src="https://github.com/user-attachments/assets/6dce4abb-30f3-4ac1-9e2f-85145fca5e41" alt="IoT-Enabled Railway Crossing System with RFID Authentication" width="850"/>
</p>

<p align="center">
  <i>Graphical hardware implementation layout showcasing the integration of the ESP32 microcontroller with dual RFID readers, servo gates, obstacles tracking sensors, and signaling peripherals.</i>
</p>

---

## Fail-Safe Software Features

* **Asynchronous State Timing:** Utilizes `millis()` clocks within sequential states (such as warning/emergency delays) instead of blocking multi-second `delay()` macros, ensuring the hardware sensor validation loops remain awake and continuously polling.

* **Direction Agnostic Logic:** Automatically registers whichever RFID sub-system is triggered first as the `entryRFID` node. The system evaluates the exit cycle based on the dynamic comparison `detectedReader != entryRFID`, allowing flawless bidirectional train crossing tracking.

* **Resilient Offline Fallback:** Designed with a non-blocking Wi-Fi timeout interface logic. If the local internet routing fails, the system automatically runs in offline mode—maintaining 100% localized safety loops, sensor detection, and servo gate operation.

---

# Project Team & Contributions

This Capstone Project was developed collectively by our lab group:

* **[Rezwan Ahmed](https://github.com/rezwan-ahmed-l7):** RFID tracking matrices, unique 4-byte UID array security filter, and `checkRFIDs()` algorithm design.

* **[Sad Udoy](https://github.com/sadudoy) (Team Leader):** HC-SR04 sonar distance computing logic, error bounds calibration, and safe-distance validations.

* **Sifat:** Physical servo barrier mechanics, Git repository management, and code file merging.

* **Saccha:** Wi-Fi connection pooling, Blynk integration layout, and mobile event signal pushing.

* **Esha:** Audio buzzer alert routines, three-phase LED signal wiring, and shared breadboard common ground management.

---

# Tools & Technologies Used

| Technology       | Purpose                                  |
| ---------------- | ---------------------------------------- |
| Proteus          | Circuit Simulation & Schematic Design    |
| ESP32 DevKit V1  | Core Microcontroller (FSM Software-Driven Engine) |
| MFRC522 RFID     | Secure Bidirectional Train Proximity Authentication |
| HC-SR04 Ultrasonic| Active Track Obstacle Parameter Monitoring |
| Blynk IoT        | Cloud-assisted Telemetry & Emergency Smartphone Alerts |
| Digital Logic    | Core Subject Theory & State Encoding     |

---

# Learning Outcomes

This repository helped me improve my understanding of:

* Digital Logic Fundamentals 
* Sequential Logic Design
* Circuit Design & Simulations 
* Hardware Troubleshooting & Debugging Techniques
* Discrete Automation Architecture via Finite State Machines (FSM)

---

# Author

### Rezwan Ahmed

B.Sc Engg in CSE Student | Aspiring Software Engineer & Learner

---

# Support

🌱 This repository is part of my learning journey as a CSE student. The codes, assignments, and reports are shared for educational purposes and may be useful for students who are learning similar concepts.

If you found this repository useful, consider giving it a ⭐ on GitHub. Thank you.
