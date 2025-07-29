#include <BluetoothSerial.h>
#include <ESP32Servo.h> // Include the ESP32Servo library

BluetoothSerial SerialBT;

// Servo objects
Servo servo1; // For light control
Servo servo2; // For fan control

// Pins for the servos
const int servo1Pin = 14; // Replace with the correct pin for servo 1
const int servo2Pin = 32; // Replace with the correct pin for servo 2
const int ledPin = 2;     // Built-in LED pin

// States for fan and light
int fanState = 2;
int lightState = 2;
int prevFanState = 2;
int prevLightState = 2;

// Time variables
unsigned long prevMillis = 0;
unsigned long millisOffset = 0;
int currentHour = 0;
int currentMinute = 0;

// Variables to store the alarm time
int alarmHour = 7;    // Default alarm hour
int alarmMinute = 0;  // Default alarm minute

// Alarm Light and Fan states
int alarmLightState = 1;
int alarmFanState = 0;

// Function to blink the LED a specified number of times
void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(delayMs);
    digitalWrite(ledPin, LOW);
    delay(delayMs);
  }
}

void updateTime() {
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - prevMillis;

  if (elapsedMillis >= 60000) { // Only update when a full minute has passed
    prevMillis += elapsedMillis; // Move forward the previous millis marker
    unsigned long elapsedMinutes = elapsedMillis / 60000;

    currentMinute += elapsedMinutes;
    if (currentMinute >= 60) {
      currentMinute -= 60;
      currentHour++;
    }

    if (currentHour >= 24) {
      currentHour = 0;
    }

    Serial.printf("Time updated to %02d:%02d\n", currentHour, currentMinute);
  }
}

void processBluetoothCommand(const String &command) {
  if (command.startsWith("F") || command.startsWith("L")) {
    if (command.length() >= 5) {
      int receivedHour = command.substring(1, 3).toInt();
      int receivedMinute = command.substring(3, 5).toInt();

      if (receivedHour >= 0 && receivedHour < 24 && receivedMinute >= 0 && receivedMinute < 60) {
        currentHour = receivedHour;
        currentMinute = receivedMinute;
        prevMillis = millis();
        millisOffset = 0;
        Serial.printf("Time set to %02d:%02d\n", currentHour, currentMinute);
      } else {
        Serial.println("Invalid time format.");
      }
    }

    if (command.startsWith("F")) {
      fanState = !fanState;
    } else if (command.startsWith("L")) {
      lightState = !lightState;
    }

    controlServos();
  } else if (command.startsWith("SET")) {
    if (command.length() == 7) {
      int alarmHH = command.substring(3, 5).toInt();
      int alarmMM = command.substring(5, 7).toInt();

      if (alarmHH >= 0 && alarmHH < 24 && alarmMM >= 0 && alarmMM < 60) {
        alarmHour = alarmHH;
        alarmMinute = alarmMM;
        Serial.printf("Alarm set for %02d:%02d\n", alarmHour, alarmMinute);
      } else {
        Serial.println("Invalid alarm time format.");
      }
    } else if(command.length() == 9) {
      int alarmHH = command.substring(3, 5).toInt();
      int alarmMM = command.substring(5, 7).toInt();

      int alarmLight = command.substring(7, 8).toInt();  // Light state (7th digit)
      int alarmFan = command.substring(8, 9).toInt();    // Fan state (8th digit)

      if (alarmHH >= 0 && alarmHH < 24 && alarmMM >= 0 && alarmMM < 60) {
        alarmHour = alarmHH;
        alarmMinute = alarmMM;

        alarmLightState = alarmLight;
        alarmFanState = alarmFan;

        Serial.printf("Alarm set for %02d:%02d\n", alarmHour, alarmMinute);
      } else {
        Serial.println("Invalid alarm time format.");
      }
    } else {
      Serial.println("Invalid SET command format. Use SETHHMMLF.");
    }
  } else if (command == "STATUS") {
    // Send status details
    String statusMessage = String("Current Time: ") + String(currentHour) + ":" + 
                           String(currentMinute) + "\n" +
                           String("Alarm Time: ") + String(alarmHour) + ":" + 
                           String(alarmMinute) + "\n" +
                           String("Fan State: ") + (fanState ? "ON" : "OFF") + "\n" +
                           String("Light State: ") + (lightState ? "ON" : "OFF") + "\n" +
                           String("Alam States: ") + ", Light" + (alarmLightState ? "ON" : "OFF") + "Fan " + (alarmFanState ? "ON" : "OFF");
    SerialBT.println(statusMessage);
    Serial.println("STATUS command processed, data sent.");
  } else {
    Serial.println("Invalid command received.");
  }
}

// Move Servos
void moveServo(int servo, int angle) {
  if (servo == 1) {
    servo1.write(angle);
    delay(500);
    servo1.write(90);
    delay(500);
  } else if (servo == 2) {
    servo2.write(angle);
    delay(500);
    servo2.write(90);
    delay(500);
  }
}

// Function to control the servos based on state changes
void controlServos() {
  // Control servo 1 (light)
  if (lightState != prevLightState) {
    if (lightState == 1) {
      moveServo(2, 45);
      blinkLED(2, 200);
      Serial.println("Light ON: Servo 2 at 45°");
    } else if (lightState == 0) {
      moveServo(1, 135);
      blinkLED(2, 200);
      Serial.println("Light OFF: Servo 1 at 135");
    }
  }

  // Control servo 2 (fan)
  if (fanState != prevFanState) {
    if (fanState == 1) {
      moveServo(2, 135);
      blinkLED(2, 200);
      Serial.println("Fan ON: Servo 2 at 135");
    } else if (fanState == 0) {
      moveServo(1, 45);
      blinkLED(2, 200);
      Serial.println("Fan OFF: Servo 1 at 45");
    }
  }

  // Update previous states
  prevLightState = lightState;
  prevFanState = fanState;
}

// Check and trigger alarms
void checkAlarms() {
  // Turn light ON and fan OFF at alarm time.
  if (currentHour == alarmHour && currentMinute == alarmMinute) {
    lightState = alarmLightState; // Light ON
    fanState = alarmFanState;   // Fan OFF
    controlServos();
    Serial.println("Alarm: Light ON, Fan OFF");
  }
}

// Setup Bluetooth and servos
void setup() {
  Serial.begin(115200);

  // Initialize LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Attach servos
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  // Default servo positions
  servo1.write(90);
  servo2.write(90);

  // Start Bluetooth
  SerialBT.begin("ESP_BT");
  Serial.println("Bluetooth started! Waiting for pairing...");
}

// Main loop
void loop() {
  // Update time
  updateTime();

  // Process Bluetooth commands
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();
    Serial.println("Received command: " + command);
    processBluetoothCommand(command);
  }

  // Check for alarms
  checkAlarms();

  // Indicate Bluetooth connection status with LED
  digitalWrite(ledPin, SerialBT.hasClient() ? HIGH : LOW);

  delay(100); // Short delay
}
