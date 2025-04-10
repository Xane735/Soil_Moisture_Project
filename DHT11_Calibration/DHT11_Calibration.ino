#include <DHT.h>

#define DHTPIN 2       // Datenpin des DHT11-Sensors
#define DHTTYPE DHT11  // Sensor-Typ DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);  // Serial Communication Started
  dht.begin();         // DHT-Sensor initialization
  delay(1000);         // Sensor is slow, wait 1 second
  Serial.println("DHT11 Temperature and Humidity Measurement Started.");
}

void loop() {
  // Temperatur und Luftfeuchtigkeit auslesen
  float temperature = dht.readTemperature();  // Temperature in Celsius
  float humidity = dht.readHumidity();        // Humidity in %

  // Check if sensor is providing valid data
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error reading sensor !");
    delay(2000);  // Wait 2 seconds and trying again
    return;
  }

  // Printing Temperature on Serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  // Luftfeuchtigkeit im seriellen Monitor ausgeben
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.println("-----------------------------");
  delay(2000);  // 2 Sekunden warten, bevor die Werte erneut ausgelesen werden
}