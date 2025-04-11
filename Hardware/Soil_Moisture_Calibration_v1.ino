//Simple code for Measuring Voltage from
// Capacitive soil moisture sensor

int soil_pin =A0; //AOUT pin on sensor

void setup() {
  Serial.begin(9600); // Serial port setup
  analogReference(EXTERNAL);
}

void loop(){
  Serial.print("Soil Moisture Sensor Voltage:");
  Serial.print((float(analogRead(soil_pin))/1023.0)*3.3); //Read sensor
  Serial.println("V");
  delay(100); //Slight delay between readings
}
