# 🚗 ESP32 Bluetooth Smart Robotic Car with Dynamic Radar & Parking Assist

An embedded, non-blocking robotic car powered by the ESP32 microcontroller. Controlled wirelessly via Bluetooth (`RoboTech`), this system features dynamic PWM speed mapping, intelligent obstacle braking, a servo-guided radar sweep, multi-frequency proximity alert beeps, and an automatic safety timeout mechanism.

---

## 📌 Features & Highlights

- **Non-Blocking Architecture:** Built with `millis()` timing execution—zero `delay()` calls in main control loops to prevent UI/sensor freezing.
- **Safety Emergency Braking:** Automatic vehicle shutdown when forward distance is $\le 5\text{ cm}$.
- **Dynamic Servo Radar Sweep:** 
  - Centers automatically to $90^\circ$ during forward motion (`F`, `G`, `I`).
  - Actively sweeps between $60^\circ$ and $120^\circ$ when idle or turning to scan surrounding clearance.
- **Audible & Visual Proximity Alerts:** Variable buzzer pitch/frequency and Red LED blinking rates based on real-time distance zones ($>12-20\text{ cm}$, $>8-12\text{ cm}$, $>5-8\text{ cm}$, $\le 5\text{ cm}$).
- **Safety Failsafe Timeout:** Auto-stops motors if no Bluetooth signal is received within $500\text{ ms}$.
- **Multi-Mode Lighting & Horn:** Independent toggle controls for Horn (`tone(3000)`), Flash lights, and Hazard/Waiting blinkers.

---

## 🔌 Hardware Pin Mapping

| Component | ESP32 Pin | Function |
| :--- | :--- | :--- |
| **Motor A (Left)** | GPIO 12 (IN1) / GPIO 26 (IN2) | Direction Control |
| **Motor A Speed** | GPIO 14 (ENA) | PWM Speed Modulation |
| **Motor B (Right)** | GPIO 25 (IN3) / GPIO 33 (IN4) | Direction Control |
| **Motor B Speed** | GPIO 32 (ENB) | PWM Speed Modulation |
| **Ultrasonic Sensor**| GPIO 27 (TRIG) / GPIO 35 (ECHO)| Distance Scanning |
| **Radar Servo** | GPIO 18 | Radar Sweep Actuator |
| **Buzzer & Alert LED**| GPIO 19 (Buzzer) / GPIO 15 (Red LED) | Proximity Audio/Visual Warning |
| **Extra Lights** | GPIO 21 (Flash) / GPIO 22 (Waiting) | Flashers & Hazard Lights |

---

## 🕹️ Bluetooth Protocol (Device Name: `RoboTech`)

### Commands Matrix

- **Directions:** `F` (Forward), `B` (Backward), `L` (Left), `R` (Right), `G` (Forward-Left), `I` (Forward-Right), `H` (Backward-Left), `J` (Backward-Right), `S` (Stop).
- **Speed Control:** Keys `'0'` to `'9'` map dynamically to PWM values (0–230). Key `'q'` sets Turbo Speed (255 PWM).
- **Toggles:** 
  - `V` / `v`: Horn ON / OFF
  - `X` / `x` or `W` / `w`: Flash Lights Control
  - `U` / `u`: Hazard / Waiting Lights Control

---

## 🚀 Getting Started

1. **Hardware Setup:** Connect the components according to the Pin Mapping table.
2. **Flash the Code:** Open the repository in Arduino IDE / VS Code (PlatformIO), select your ESP32 board, set upload speed to 115200 baud, and upload.
3. **Connect Mobile App:** Open any Bluetooth Serial Terminal app, search for **`RoboTech`**, connect, and start driving.

---


