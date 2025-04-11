// Simple Arduino code to estimate Soil Moisture (%) 
// using a capacitive soil moisture sensor

int soil_pin = A0; // AOUT pin on sensor

float slope = 2.48;      // Slope from linear fit
float intercept = -0.72; // Intercept from linear fit

void setup() {
  Serial.begin(9600);              // Serial port setup
  analogReference(EXTERNAL);      // Set the analog reference to 3.3V
}

void loop() {
  float voltage, vol_water_cont, moisture_percent;

  voltage = (analogRead(soil_pin) / 1023.0) * 3.3;

  // Calculate volumetric water content (theta_v)
  vol_water_cont = ((1.0 / voltage) * slope) + intercept;

  // Convert to soil moisture percentage
  moisture_percent = vol_water_cont * 100;

  // Clamp moisture percentage between 0 and 100
  if (moisture_percent < 0) moisture_percent = 0;
  if (moisture_percent > 100) moisture_percent = 100;

  // Print values to Serial Monitor
  Serial.print("Voltage: ");
  Serial.print(voltage, 2);
  Serial.print(" V | Moisture: ");
  Serial.print(moisture_percent, 2);
  Serial.println(" %");

  delay(1000); // 1 second delay between readings
}
