#include <BluetoothSerial.h>
#include <ESP32Servo.h> // Include the ESP32Servo library

// BluetoothSerial object
BluetoothSerial SerialBT;

// Local variables to track the current and previous states of the fan and light
int fanState = 2;
int lightState = 2;
int prevFanState = 2;
int prevLightState = 2;

// Servo objects
Servo servo1; // For light control
Servo servo2; // For fan control

// Pins for the servos
const int servo1Pin = 14; // Replace with the correct pin for servo 1
const int servo2Pin = 32; // Replace with the correct pin for servo 2

// Built-in LED pin
const int ledPin = 2;

// Function to blink the LED a specified number of times
void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(delayMs);
    digitalWrite(ledPin, LOW);
    delay(delayMs);
  }
}

// Setup Bluetooth connection and servos
void setup() {
  Serial.begin(115200);

  // Configure built-in LED pin as output
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Turn LED off initially

  // Attach servos to their respective pins
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  // Set initial positions to 90° (neutral position)
  servo1.write(90);
  servo2.write(90);
  delay(500);

  // Start Bluetooth communication
  SerialBT.begin("ESP32_Device"); // Set Bluetooth device name
  SerialBT.setPin("2727", 4);
  Serial.println("Bluetooth started! Waiting for pairing...");
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

// Function to process Bluetooth commands
void processBluetoothCommand(const String& command) {
  if (command == "F1") {
    fanState = 1; // Turn fan ON
  } else if (command == "F0") {
    fanState = 0; // Turn fan OFF
  } else if (command == "L1") {
    lightState = 1; // Turn light ON
  } else if (command == "L0") {
    lightState = 0; // Turn light OFF
  } else if (command == "F") {
    fanState = !fanState; // Toggle fan state
  } else if (command == "L") {
    lightState = !lightState; // Toggle light state
  } else {
    Serial.println("Invalid command received.");
  }

  // Control servos based on the updated states
  controlServos();
}

// Loop to handle Bluetooth communication
void loop() {
  // // Check if Bluetooth is connected
  if (SerialBT.hasClient()) {
    // Turn on the LED if Bluetooth is connected
    digitalWrite(ledPin, HIGH);
  } else {
    // Turn off the LED if Bluetooth is not connected
    digitalWrite(ledPin, LOW);
  }

  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n'); // Read incoming command
    command.trim(); // Remove any trailing spaces or newline characters
    Serial.println("Received command: " + command);
    processBluetoothCommand(command); // Process the command
  }

  delay(100); // Short delay to avoid overwhelming the loop
}
