#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi credentials
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

// MQTT Broker settings (use the same as in your JavaScript)
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_client_id = "esp8266-plantcare";
const char *publish_topic = "plantcare/sensors";
const char *subscribe_topic = "plantcare/actuators";

// Sensor pins
#define MOISTURE_PIN A0    // Soil moisture sensor analog pin
#define WATER_LEVEL_PIN D1 // Water level sensor digital pin
#define DHT_PIN D2         // DHT temperature & humidity sensor pin
#define DHT_TYPE DHT22     // DHT22 (AM2302) or DHT11
#define PUMP_PIN D5        // Water pump control pin

// Sensor objects
DHT dht(DHT_PIN, DHT_TYPE);

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Variables
unsigned long lastMsg = 0;
const long interval = 30000; // Send data every 30 seconds

void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    // Create a buffer for the payload
    char message[length + 1];
    for (int i = 0; i < length; i++)
    {
        message[i] = (char)payload[i];
    }
    message[length] = '\0';
    Serial.println(message);

    // Parse JSON command
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, message);

    if (!error)
    {
        // Check if pump control command received
        if (doc.containsKey("pump"))
        {
            String pumpState = doc["pump"].as<String>();
            if (pumpState == "ON")
            {
                digitalWrite(PUMP_PIN, HIGH);
                Serial.println("Pump turned ON");
            }
            else if (pumpState == "OFF")
            {
                digitalWrite(PUMP_PIN, LOW);
                Serial.println("Pump turned OFF");
            }
        }
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(mqtt_client_id))
        {
            Serial.println("connected");
            client.subscribe(subscribe_topic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup()
{
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW); // Make sure pump is off initially

    Serial.begin(115200);
    setup_wifi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    dht.begin();
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > interval)
    {
        lastMsg = now;

        // Read sensor values
        float moisture = readMoistureSensor();
        float waterLevel = readWaterLevelSensor();
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        // Check if any readings failed
        if (isnan(humidity) || isnan(temperature))
        {
            Serial.println("Failed to read from DHT sensor!");
        }
        else
        {
            // Create JSON document
            DynamicJsonDocument doc(256);
            doc["moisture"] = moisture;
            doc["temperature"] = temperature;
            doc["humidity"] = humidity;
            doc["waterLevel"] = waterLevel;
            doc["timestamp"] = millis();

            // Serialize JSON to string
            char jsonBuffer[256];
            serializeJson(doc, jsonBuffer);

            // Publish to MQTT
            Serial.print("Publishing: ");
            Serial.println(jsonBuffer);
            client.publish(publish_topic, jsonBuffer);
        }
    }
}

float readMoistureSensor()
{
    // Read raw value (adapt based on your actual sensor)
    int rawValue = analogRead(MOISTURE_PIN);

    // Convert to percentage (adjust these values based on your sensor's min/max)
    // Typically soil moisture sensors give low values when wet and high when dry
    float moisturePercent = map(rawValue, 1023, 0, 0, 100);

    // Constrain to 0-100% range
    moisturePercent = constrain(moisturePercent, 0, 100);

    return moisturePercent;
}

float readWaterLevelSensor()
{
    // Example implementation - modify based on your actual sensor
    // For analog water level sensors:
    int rawValue = analogRead(WATER_LEVEL_PIN);
    float levelPercent = map(rawValue, 0, 1023, 0, 100);

    // For ultrasonic sensors, you would measure distance instead
    // levelPercent = 100 - (distance_to_water / total_height * 100);

    return levelPercent;
}