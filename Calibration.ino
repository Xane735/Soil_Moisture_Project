#include <DHT.h>
#define DHTPIN 2      // DHT data pin connected to D2
#define DHTTYPE DHT11 // Use DHT22 if you have that

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    Serial.begin(9600);
    dht.begin();
}

void loop()
{
    // Read humidity and temperature
    float humidity = dht.readHumidity();
    float temp = dht.readTemperature(); // Celsius

    // Check if readings failed (common if wiring is wrong)
    if (isnan(humidity) || isnan(temp))
    {
        Serial.println("ERROR: Check DHT wiring or sensor!");
        delay(2000);
        return; // Skip the rest of the loop
    }

    // Check for impossible values (DHT11 range: 20-90% humidity, 0-50°C)
    if (humidity < 0 || humidity > 100 || temp < -40 || temp > 80)
    {
        Serial.println("ERROR: Invalid values! Check sensor or power.");
        delay(2000);
        return;
    }

    // Print valid data
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    delay(2000); // Wait 2 seconds
}
