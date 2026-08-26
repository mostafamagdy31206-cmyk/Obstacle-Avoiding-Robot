#include <ESP32Servo.h>
#include <BluetoothSerial.h>

// =====================================================
//                    OBJECTS
// =====================================================
Servo radar;
BluetoothSerial BT;

// =====================================================
//                    MOTOR PINS
// =====================================================

// ---------- Motor A (Left) ----------
#define IN1 12
#define IN2 26
#define ENA 14

// ---------- Motor B (Right) ----------
#define IN3 25
#define IN4 33
#define ENB 32

// =====================================================
//                    ULTRASONIC PINS
// =====================================================
#define TRIG 27
#define ECHO 35

// =====================================================
//                    EXTRA PINS
// =====================================================
#define SERVO_PIN 18
#define BUZZER 19
#define RED_LED 15

// ---------- Extra Lights (Flash / Waiting) ----------
#define FLASH_PIN 21
#define WAITING_PIN 22

// =====================================================
//                    VARIABLES
// =====================================================

char command = 'S';


const int safeDistance = 5;

// ---------- Speed Control ----------
int speedLevel = 5;
int currentSpeed = 127;

bool obstacle = false;

// ---------- Servo Sweep ----------
unsigned long lastServoMove = 0;
const int servoInterval = 50;

int servoAngle = 90;
int servoStep = 5;

const int servoMin = 60;
const int servoMax = 120;

// ---------- Buzzer Timing ----------
unsigned long lastSound = 0;
bool buzzerState = false;


const int readInterval = 60;

long lastDistance = 0;
unsigned long lastReadTime = 0;

// ---------- Safety Timeout ----------
unsigned long lastMoveTime = 0;
const int stopTimeout = 500;

// ---------- Horn ----------
bool hornOn = false;

// ---------- Flash Lights ----------
bool flashOn = false;
unsigned long lastFlash = 0;
bool flashState = false;
const int flashInterval = 300;

// ---------- Waiting Lights ----------
bool waitingOn = false;
unsigned long lastWaitingBlink = 0;
bool waitingBlinkState = false;
const int waitingBlinkInterval = 400;

// =====================================================
//                    SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  BT.begin("RoboTech");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(FLASH_PIN, OUTPUT);
  pinMode(WAITING_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);
  digitalWrite(WAITING_PIN, LOW);

  radar.attach(SERVO_PIN);
  radar.write(servoAngle);

  lastMoveTime = millis();

  stopCar();
}

// =====================================================
//                    MAIN LOOP
// =====================================================

void loop() {

  // ---------- Bluetooth Command Handling ----------
  if (BT.available()) {
    char received = BT.read();

    if (received >= '0' && received <= '9') {
      speedLevel = received - '0';
      currentSpeed = map(speedLevel, 0, 9, 0, 230);
    } else if (received == 'q') {
      speedLevel = 10;
      currentSpeed = 255;
    } else if (received == 'V') {
      hornOn = true;
    } else if (received == 'v') {
      hornOn = false;
      noTone(BUZZER);
    } else if (received == 'X') {
      flashOn = true;
    } else if (received == 'x') {
      flashOn = false;
      digitalWrite(FLASH_PIN, LOW);
    } else if (received == 'U') {
      waitingOn = true;
    } else if (received == 'u') {
      waitingOn = false;
      digitalWrite(WAITING_PIN, LOW);
    }
    else if (received == 'W') {
      digitalWrite(FLASH_PIN, HIGH);
    }
    else if (received == 'w') {
      digitalWrite(FLASH_PIN, LOW);
    }
    else if (received == 'F' || received == 'B' || received == 'L' || received == 'R' ||
             received == 'G' || received == 'I' || received == 'H' || received == 'J' ||
             received == 'S') {
      command = received;
      if (received != 'S') {
        lastMoveTime = millis();
      }
    }
  }

  // ---------- Safety Timeout ----------
  if (millis() - lastMoveTime > stopTimeout) {
    command = 'S';
  }

  // ---------- Servo Sweeping ----------
  if (command == 'F' || command == 'G' || command == 'I') {
    if (servoAngle != 90) {
      servoAngle = 90;
      radar.write(servoAngle);
    }
  } else if (millis() - lastServoMove >= servoInterval) {
    lastServoMove = millis();
    radar.write(servoAngle);
    servoAngle += servoStep;
    if (servoAngle >= servoMax || servoAngle <= servoMin) {
      servoStep = -servoStep;
    }
  }

  // ---------- Distance Read ----------
  long distance = getDistance();

  // ---------- Parking Sensor Timing Interval ----------
  int beepInterval = -1;

  if (distance > 0 && distance <= 5) {
    obstacle = true;
    beepInterval = 0;
  } else if (distance > 5 && distance <= 8) {
    obstacle = false;
    beepInterval = 80;
  } else if (distance > 8 && distance <= 12) {
    obstacle = false;
    beepInterval = 180;
  } else if (distance > 12 && distance <= 20) {
    obstacle = false;
    beepInterval = 350;
  } else {
    obstacle = false;
    beepInterval = -1;
  }

  // ---------- Buzzer & LED Action ----------
  if (hornOn) {
    tone(BUZZER, 3000);
  } else if (beepInterval == 0) {
    tone(BUZZER, 2500);
    digitalWrite(RED_LED, HIGH);
  } else if (beepInterval > 0) {
    if (millis() - lastSound >= beepInterval) {
      lastSound = millis();
      buzzerState = !buzzerState;

      if (buzzerState) {
        tone(BUZZER, 2000);
        digitalWrite(RED_LED, HIGH);
      } else {
        noTone(BUZZER);
        digitalWrite(RED_LED, LOW);
      }
    }
  } else {
    noTone(BUZZER);
    digitalWrite(RED_LED, LOW);
    buzzerState = false;
  }

  // ---------- Flash Light Blinking ----------
  if (flashOn) {
    if (millis() - lastFlash >= flashInterval) {
      lastFlash = millis();
      flashState = !flashState;
      digitalWrite(FLASH_PIN, flashState ? HIGH : LOW);
    }
  }

  // ---------- Waiting Light Blinking ----------
  if (waitingOn) {
    if (millis() - lastWaitingBlink >= waitingBlinkInterval) {
      lastWaitingBlink = millis();
      waitingBlinkState = !waitingBlinkState;
      digitalWrite(WAITING_PIN, waitingBlinkState ? HIGH : LOW);
    }
  }

  // ---------- Car Movement Execution ----------
  if ((command == 'F' || command == 'G' || command == 'I') &&
      distance > 0 && distance <= safeDistance) {
    stopCar();
  } else {
    executeCommand(command);
  }
}

// =====================================================
//                  ULTRASONIC DISTANCE
// =====================================================

long getDistance() {
  if (millis() - lastReadTime < readInterval) {
    return lastDistance;
  }

  lastReadTime = millis();

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);

  if (duration == 0) {
    return lastDistance;
  }

  lastDistance = duration * 0.0343 / 2;
  return lastDistance;
}

// =====================================================
//                    COMMAND CONTROL
// =====================================================

void executeCommand(char cmd) {

  if (cmd == 'F') {
    moveForward();
  }
  else if (cmd == 'B') {
    moveBackward();
  }
  else if (cmd == 'L') {
    turnLeft();
  }
  else if (cmd == 'R') {
    turnRight();
  }
  else if (cmd == 'G') {
    forwardLeft();
  }
  else if (cmd == 'I') {
    forwardRight();
  }
  else if (cmd == 'H') {
    backwardLeft();
  }
  else if (cmd == 'J') {
    backwardRight();
  }
  else if (cmd == 'S') {
    stopCar();
  }
  else {
    stopCar();
  }
}
// =====================================================
//                    FORWARD
// =====================================================

void moveForward() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}


// =====================================================
//                    BACKWARD
// =====================================================

void moveBackward() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}


// =====================================================
//                    HARD LEFT
// =====================================================

void turnLeft() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, currentSpeed);
}


// =====================================================
//                    HARD RIGHT
// =====================================================

void turnRight() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, 0);
}


// =====================================================
//                    FORWARD + LEFT
// =====================================================

void forwardLeft() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, (currentSpeed / 2));
  analogWrite(ENB, currentSpeed);
}


// =====================================================
//                    FORWARD + RIGHT
// =====================================================

void forwardRight() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, (currentSpeed / 2));
}


// =====================================================
//                    BACKWARD + LEFT
// =====================================================

void backwardLeft() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, (currentSpeed / 2));
  analogWrite(ENB, currentSpeed);
}


// =====================================================
//                    BACKWARD + RIGHT
// =====================================================

void backwardRight() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, (currentSpeed / 2));
}


// =====================================================
//                    STOP
// =====================================================

void stopCar() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}





