const int sensorPin = A0; // Analog pin connected to the sensor
int sensorValue = 0; 
int moisturePercent = 0;

const int dryValue = 368; // Value when the soil is completely dry
const int wetValue = 615;  // Value when the soil is fully wet

void setup() {
  Serial.begin(9600); // Start the Serial Monitor
}

void loop() {
  sensorValue = analogRead(sensorPin); // Read the analog value from the sensor

  // Map the sensor reading to a percentage (0% for wet, 100% for dry)
  moisturePercent = map(sensorValue, wetValue, dryValue, 0, 100);

  // Constrain the percentage within 0% to 100% range
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println(" %");

  delay(5000); // Wait for 1 second
}
