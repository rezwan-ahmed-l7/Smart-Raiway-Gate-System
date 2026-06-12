// --- BLYNK CREDENTIALS (REPLACE WITH YOURS) ---
#define BLYNK_TEMPLATE_ID "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "SentinelGate"
#define BLYNK_AUTH_TOKEN "YourAuthToken"

#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// --- WI-FI SETTINGS ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- PIN DEFINITIONS ---
#define SCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19

// RFID 1 (Entry/Exit A)
#define SDA_1_PIN 5
#define RST_1_PIN 22

// RFID 2 (Entry/Exit B)
#define SDA_2_PIN 4
#define RST_2_PIN 21

// Sonar Sensors
#define TRIG1_PIN 13
#define ECHO1_PIN 14
#define TRIG2_PIN 25
#define ECHO2_PIN 26

// Servos
#define SERVO1_PIN 27
#define SERVO2_PIN 32

// Indicators
#define RED_LED 33
#define YELLOW_LED 15 
#define GREEN_LED 16
#define BUZZER 17

// --- SETTINGS ---
const int GATE_OPEN_ANGLE = 90;
const int GATE_CLOSE_ANGLE = 0;
const int OBSTACLE_DISTANCE_CM = 7; 

// --- OBJECTS ---
MFRC522 rfid1(SDA_1_PIN, RST_1_PIN);
MFRC522 rfid2(SDA_2_PIN, RST_2_PIN);
Servo gate1;
Servo gate2;

// --- UIDs ---
byte train1UID[] = {0xA4, 0xD7, 0xA4, 0xE5};
byte train2UID[] = {0x2A, 0x77, 0x02, 0xE1};

// --- SYSTEM STATES ---
enum SystemState {
  IDLE,
  WARNING_YELLOW,
  GATES_CLOSING,
  TRAIN_INSIDE,
  EMERGENCY_OBSTACLE,
  GATES_OPENING
};

SystemState currentState = IDLE;
int entryRFID = 0; 
unsigned long stateTimer = 0;
bool alertSentThisEmergency = false; // Prevents spamming your phone

void setup() {
  Serial.begin(115200);
  
  // Initialize Wi-Fi and Blynk
  connectWiFi();

  // Initialize SPI & RFIDs
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
  rfid1.PCD_Init();
  rfid2.PCD_Init();

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Initialize Servos
  gate1.setPeriodHertz(50); 
  gate2.setPeriodHertz(50);
  gate1.attach(SERVO1_PIN, 500, 2400); 
  gate2.attach(SERVO2_PIN, 500, 2400);
  
  gate1.write(GATE_OPEN_ANGLE);
  gate2.write(GATE_OPEN_ANGLE);

  // Initialize IO
  pinMode(TRIG1_PIN, OUTPUT);
  pinMode(ECHO1_PIN, INPUT);
  pinMode(TRIG2_PIN, OUTPUT);
  pinMode(ECHO2_PIN, INPUT);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  setTrafficLights(LOW, LOW, HIGH); // Start Green
  Serial.println("SentinelGate IoT System Ready.");
}

void loop() {
  // Handle Blynk background tasks safely
  if (Blynk.connected()) {
    Blynk.run();
  }

  switch (currentState) {
    case IDLE: handleIdleState(); break;
    case WARNING_YELLOW: handleWarningState(); break;
    case GATES_CLOSING: handleGatesClosingState(); break;
    case TRAIN_INSIDE: handleTrainInsideState(); break;
    case EMERGENCY_OBSTACLE: handleEmergencyState(); break;
    case GATES_OPENING: handleGatesOpeningState(); break;
  }
}

// --- STATE HANDLERS ---

void handleIdleState() {
  alertSentThisEmergency = false; // Reset alert flag for the next train passage
  
  int detectedReader = checkRFIDs();
  if (detectedReader > 0) {
    Serial.print("Train Detected at Reader: ");
    Serial.println(detectedReader);
    
    entryRFID = detectedReader;
    currentState = WARNING_YELLOW;
    stateTimer = millis();
    
    setTrafficLights(LOW, HIGH, LOW);
    digitalWrite(BUZZER, HIGH); 
    delay(200);
    digitalWrite(BUZZER, LOW);
  }
}

void handleWarningState() {
  if (millis() - stateTimer > 3000) {
    currentState = GATES_CLOSING;
    setTrafficLights(HIGH, LOW, LOW); 
  }
}

void handleGatesClosingState() {
  gate1.write(GATE_CLOSE_ANGLE);
  gate2.write(GATE_CLOSE_ANGLE);
  
  if (isObstacleDetected()) {
    triggerEmergency();
    return;
  }
  
  currentState = TRAIN_INSIDE;
}

void handleTrainInsideState() {
  if (isObstacleDetected()) {
    triggerEmergency();
    return;
  }

  int detectedReader = checkRFIDs();
  if (detectedReader > 0 && detectedReader != entryRFID) {
    Serial.println("Train exiting via opposite reader.");
    currentState = GATES_OPENING;
  }
}

void handleEmergencyState() {
  bool toggle = (millis() / 250) % 2;
  digitalWrite(RED_LED, toggle);
  digitalWrite(BUZZER, toggle);
  
  if (!isObstacleDetected()) {
    delay(2000); 
    if (!isObstacleDetected()) {
      Serial.println("Track clear. Resuming closure.");
      digitalWrite(BUZZER, LOW);
      setTrafficLights(HIGH, LOW, LOW); 
      currentState = GATES_CLOSING;
    }
  }
}

void handleGatesOpeningState() {
  gate1.write(GATE_OPEN_ANGLE);
  gate2.write(GATE_OPEN_ANGLE);
  setTrafficLights(LOW, LOW, HIGH); 
  entryRFID = 0;
  currentState = IDLE;
}

void triggerEmergency() {
  Serial.println("EMERGENCY: Obstacle on track! Opening gates.");
  
  // Open gates IMMEDIATELY before doing anything else
  gate1.write(GATE_OPEN_ANGLE); 
  gate2.write(GATE_OPEN_ANGLE);
  currentState = EMERGENCY_OBSTACLE;

  // Send Wi-Fi alert only once per emergency to avoid phone spam
  if (!alertSentThisEmergency) {
    sendPhoneAlert();
    alertSentThisEmergency = true;
  }
}

// --- HELPER FUNCTIONS ---

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  
  // Try to connect for 10 seconds max so it doesn't freeze the setup forever
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connected!");
    // Configure and connect Blynk non-blockingly if Wi-Fi is available
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
  } else {
    Serial.println("\nWi-Fi Failed. Running in offline mode.");
  }
}

void sendPhoneAlert() {
  if (Blynk.connected()) {
    Serial.println("Sending Emergency Alert to Phone via Blynk...");
    Blynk.logEvent("crossing_emergency");
  } else {
    Serial.println("Cannot send alert. Blynk not connected.");
  }
}

void setTrafficLights(bool red, bool yellow, bool green) {
  digitalWrite(RED_LED, red);
  digitalWrite(YELLOW_LED, yellow);
  digitalWrite(GREEN_LED, green);
}

int checkRFIDs() {
  if (rfid1.PICC_IsNewCardPresent() && rfid1.PICC_ReadCardSerial()) {
    if (isValidTrain(rfid1.uid.uidByte, rfid1.uid.size)) {
      rfid1.PICC_HaltA(); 
      return 1;
    }
  }
  if (rfid2.PICC_IsNewCardPresent() && rfid2.PICC_ReadCardSerial()) {
    if (isValidTrain(rfid2.uid.uidByte, rfid2.uid.size)) {
      rfid2.PICC_HaltA(); 
      return 2;
    }
  }
  return 0; 
}

bool isValidTrain(byte *uid, byte uidSize) {
  if (uidSize != 4) return false;
  bool isTrain1 = true, isTrain2 = true;
  for (int i = 0; i < 4; i++) {
    if (uid[i] != train1UID[i]) isTrain1 = false;
    if (uid[i] != train2UID[i]) isTrain2 = false;
  }
  return isTrain1 || isTrain2;
}

bool isObstacleDetected() {
  float dist1 = getDistance(TRIG1_PIN, ECHO1_PIN);
  float dist2 = getDistance(TRIG2_PIN, ECHO2_PIN);
  
  if (dist1 > 0 && dist1 < OBSTACLE_DISTANCE_CM) return true;
  if (dist2 > 0 && dist2 < OBSTACLE_DISTANCE_CM) return true;
  
  return false;
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); 
  if (duration == 0) return 999.0; 
  return duration * 0.034 / 2;
}
