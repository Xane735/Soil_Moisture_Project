//Simple Arduino code to predict volumetric
//Water content in soil using a capacitive soil moisture sensor
int soil_pin = A0; // AOUT pin on sensor

float slope =2.48;// Slope from linear fit
float intercept = -0.72;

void setup() {
  Serial.begin(9600); //Serial port setup
  analogReference(EXTERNAL); // Set the analog reference to 3.3V
}

void loop() {
  float voltage,vol_water_cont; // Preallocate to approx, voltage and theta_v
  Serial.print("\n Voltage:");
  voltage = (float(analogRead(soil_pin))/1023.0)*3.3;
  Serial.print(voltage); //Read Sensor
  Serial.print("V, Theta_v:");
  vol_water_cont = ((1.0/voltage)*slope)+intercept; //Calculation of theta_v (vol. water content)
  Serial.print(vol_water_cont);
  Serial.print("cm^3/cm^3"); 
  delay(100); //Slight delay between readings
}
