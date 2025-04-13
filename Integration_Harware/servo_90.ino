#include <Servo.h> // Include the Servo library

Servo myServo; // Create a Servo object

void setup() {
  myServo.attach(D1); // Attach the servo to GPIO pin D1 (or another suitable pin)
}

void loop() {
  myServo.write(90); // Move the servo to 90 degrees
  delay(1000);       // Wait for 1 second
  myServo.write(0);  // Move the servo back to 0 degrees
  delay(1000);       // Wait for 1 second
}
