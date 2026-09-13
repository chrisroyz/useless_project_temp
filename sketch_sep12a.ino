#include <ESP8266WiFi.h> 
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char* ssid = "Equinoxi";
const char* password = "password";
const int udpPort = 5005;

WiFiUDP udp;
long last_processed_seq = -1;

// --- L298N MOTOR DRIVER PIN DEFINITIONS ---
// Motor A (Throttle / Drive)
#define ENA D7   // GPIO13 - PWM speed
#define IN1 D1   // GPIO5  - Direction
#define IN2 D2   // GPIO4  - Direction

// Motor B (Steering)
#define ENB D8   // GPIO15 - PWM speed
#define IN3 D5   // GPIO14 - Direction
#define IN4 D6   // GPIO12 - Direction

// --- MOTOR CONTROL FUNCTION ---
// Takes a value from -100 to 100 and drives a motor via L298N
// Positive = Forward/Right, Negative = Reverse/Left, 0 = Stop
void driveMotor(int enPin, int in1Pin, int in2Pin, int value) {
  // Clamp to -100..100
  value = constrain(value, -100, 100);

  // Map absolute speed (0-100) to PWM range (0-1023 on ESP8266)
  int pwmSpeed = map(abs(value), 0, 100, 0, 1023);

  if (value > 0) {
    // Forward / Right
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  } else if (value < 0) {
    // Reverse / Left
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  } else {
    // Stop - brake
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    pwmSpeed = 0;
  }

  analogWrite(enPin, pwmSpeed);
}

void setup() {
  Serial.begin(115200);

  // Initialize motor pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Start with motors stopped
  driveMotor(ENA, IN1, IN2, 0);
  driveMotor(ENB, IN3, IN4, 0);
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP()); 

  udp.begin(udpPort);
  Serial.printf("Listening on UDP port %d\n", udpPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("!!! RAW UDP PACKET RECEIVED !!! Size: ");
    Serial.println(packetSize);
    char packetBuffer[255];
    int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) packetBuffer[len] = '\0'; 

    JsonDocument doc; 
    DeserializationError error = deserializeJson(doc, packetBuffer);

    if (error) {
      Serial.println("JSON parse failed!");
      return;
    }

    const char* type = doc["type"];

    // --- FAILSAFE: Prevent crash if "type" is missing ---
    if (type == nullptr) {
      Serial.println("[ERROR] Packet missing 'type' field. Ignoring.");
      return; 
    }

    // ==========================================
    // HANDLE RELIABLE COMMANDS (Requires ACK)
    // ==========================================
    if (strcmp(type, "cmd") == 0) {
      long seq = doc["seq"];
      const char* cmdVal = doc["val"];

      // 1. Send ACK immediately
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      JsonDocument ackDoc;
      ackDoc["ack_seq"] = seq;
      serializeJson(ackDoc, udp); 
      udp.endPacket();

      // 2. Execute if it's a new sequence
      if (seq > last_processed_seq) {
        Serial.printf("[CMD] Executing: %s (Seq: %ld)\n", cmdVal, seq);
        // Add your switch/if statements here for headlights, modes, etc.
        last_processed_seq = seq;
      }
    } 
    // ==========================================
    // HANDLE CONTINUOUS STREAM (No ACK)
    // ==========================================
    else if (strcmp(type, "stream") == 0) {
      int throttle = doc["thr"];
      int steering = doc["str"];
      
      Serial.printf("[STREAM] Throttle: %d | Steering: %d\n", throttle, steering);

      // Differential Steering Mixing (Tank Drive)
      int leftSpeed = throttle + steering;
      int rightSpeed = throttle - steering;

      // Drive the motors!
      driveMotor(ENA, IN1, IN2, leftSpeed);   // Motor A = Left wheels
      driveMotor(ENB, IN3, IN4, rightSpeed);  // Motor B = Right wheels
    }
  }
}
