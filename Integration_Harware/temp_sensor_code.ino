#include <DHT.h>

// Define the DHT sensor type and the pin it's connected to
#define DHTTYPE DHT11
#define DHTPIN 2 // DHT11 sensor connected to pin 2

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing DHT11 Sensor...");
  dht.begin(); // Start the DHT sensor
}

void loop() {
  // Wait a few seconds between measurements
  delay(2000);

  float humidity = dht.readHumidity();    // Read humidity
  float temperature = dht.readTemperature(); // Read temperature in Celsius

  // Check if the readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error reading data from DHT sensor!");
    return;
  }

  // Print data to the Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
}
