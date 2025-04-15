#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "DESKTOP-S22TF4N 7629";
const char* password = "W:6a8402";

// DHT11 sensor configuration
#define DHTTYPE DHT11
#define DHTPIN D3  // GPIO pin for DHT sensor

// Soil moisture sensor configuration
const int soilSensorPin = A0;  // Analog pin for soil moisture sensor
const int dryValue = 368;      // Value when soil is completely dry
const int wetValue = 615;      // Value when soil is fully wet

// Ultrasonic sensor configuration
const int trigPin = D1;        // GPIO5
const int echoPin = D2;        // GPIO4
const int MAX_LEVEL = 20;      // Maximum water level (change as needed)
const int MIN_LEVEL = 5;       // Minimum water level (change as needed)

// Pump control configuration
#define PUMP_PIN D5            // Digital pin for pump control

// LED for status indication
#define LED_PIN 2              // Built-in LED

// Initialize sensors
DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

// Sensor data variables
float temperature = 25.0;
float humidity = 55.0;
int moisturePercent = 50;
float waterLevelDistance = 10.0;
float waterLevelPercent = 60.0;
bool pumpStatus = false;

// Function declarations
void handleRoot();
void handleSensorData();
void handleActuatorControl();
void handleNotFound();
void readDHTSensor();
void readSoilMoisture();
void readWaterLevel();
void updateSensors();
String macToString(const uint8_t* mac);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n\n===== ESP8266 Multi-Sensor System with Web Server =====");
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  digitalWrite(LED_PIN, HIGH);   // Turn LED on (indicating power)
  digitalWrite(PUMP_PIN, LOW);   // Ensure pump is off initially
  
  // Initialize DHT sensor
  Serial.println("Initializing DHT11 sensor...");
  dht.begin();
  
  // Connect to WiFi in Station mode (connect to user's WiFi)
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected successfully!");
  Serial.println("******************************");
  Serial.print("SERVER IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  Serial.println("CONNECT TO THIS IP IN YOUR BROWSER");
  Serial.println("******************************");
  
  // Set up server routes
  server.on("/", handleRoot);
  server.on("/data", HTTP_GET, handleSensorData);
  server.on("/control", HTTP_POST, handleActuatorControl);
  server.onNotFound(handleNotFound);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("System initialized and ready to go!");
  Serial.println("=====================================\n");
}

void loop() {
  server.handleClient();  // Handle client requests
  updateSensors();        // Update sensor readings
  
  // Periodically print the IP address for easier access
  static unsigned long lastIpPrint = 0;
  if (millis() - lastIpPrint > 30000) { // Print IP every 30 seconds
    lastIpPrint = millis();
    Serial.println("******************************");
    Serial.print("SERVER IP ADDRESS: ");
    Serial.println(WiFi.localIP());
    Serial.println("CONNECT TO THIS IP IN YOUR BROWSER");
    Serial.println("******************************");
  }
  
  delay(100);             // Short delay to avoid WDT issues
}

// Main function to update all sensor readings
void updateSensors() {
  // Update only every 5 seconds to avoid flooding
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 5000) return;
  lastUpdate = millis();
  
  // Read data from all sensors
  readDHTSensor();
  readSoilMoisture();
  readWaterLevel();
  
  // Auto-control logic for pump based on soil moisture
  if (moisturePercent < 40 && !pumpStatus) {
    // Turn pump on if moisture is low
    digitalWrite(PUMP_PIN, HIGH);
    pumpStatus = true;
    Serial.println("Pump automatically turned ON due to low moisture");
  } else if (moisturePercent > 60 && pumpStatus) {
    // Turn pump off if moisture is high
    digitalWrite(PUMP_PIN, LOW);
    pumpStatus = false;
    Serial.println("Pump automatically turned OFF due to sufficient moisture");
  }
}

// Function to read temperature and humidity from DHT sensor
void readDHTSensor() {
  // Read humidity and temperature
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  // Check if readings are valid
  if (!isnan(newHumidity) && !isnan(newTemperature)) {
    humidity = newHumidity;
    temperature = newTemperature;
    
    // Output to serial monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Error reading data from DHT11 sensor!");
  }
}

// Function to read soil moisture
void readSoilMoisture() {
  int sensorValue = analogRead(soilSensorPin);
  
  // Map the sensor reading to a percentage (0% for wet, 100% for dry)
  moisturePercent = map(sensorValue, wetValue, dryValue, 0, 100);
  
  // Constrain the percentage within 0% to 100% range
  moisturePercent = constrain(moisturePercent, 0, 100);
  
  // Output to serial monitor
  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.print("% | Raw Value: ");
  Serial.println(sensorValue);
  
  // Optional: Add condition-based messages to serial
  if (moisturePercent < 30) {
    Serial.println("Soil is very wet!");
  } else if (moisturePercent > 70) {
    Serial.println("Soil is dry, watering recommended!");
  }
}

// Function to read water level using ultrasonic sensor
void readWaterLevel() {
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send 10us pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo time
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate distance (in cm)
  waterLevelDistance = duration * 0.0343 / 2;
  
  // Convert distance to percentage (inverse relationship)
  // Assuming MAX_LEVEL is when the container is empty and MIN_LEVEL is when full
  waterLevelPercent = map(constrain(waterLevelDistance, MIN_LEVEL, MAX_LEVEL), MAX_LEVEL, MIN_LEVEL, 0, 100);
  
  // Output to serial monitor
  Serial.print("Water Level Distance: ");
  Serial.print(waterLevelDistance);
  Serial.print(" cm | Percentage: ");
  Serial.print(waterLevelPercent);
  Serial.println(" %");
  
  // Check water level status
  if (waterLevelDistance > MAX_LEVEL) {
    Serial.println("WARNING: WATER LEVEL LOW!!");
  } else if (waterLevelDistance < MIN_LEVEL) {
    Serial.println("WARNING: TUB OVERFILLED, REDUCE WATER LEVELS!");
  } else {
    Serial.println("Water level is normal.");
  }
}

// Serve the main HTML page
void handleRoot() {
  String html = R"=====(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Automatic Plant Care</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --sdg-green: #4CAF50;
            --sdg-blue: #2196F3;
            --sdg-dark-blue: #1565C0;
            --sdg-light-blue: #E3F2FD;
            --sdg-yellow: #FFC107;
            --sdg-orange: #FF9800;
            --sdg-white: #FFFFFF;
            --sdg-light-gray: #F5F5F5;
            --sdg-text: #263238;
            --sdg-text-light: #607D8B;
        }

        body {
            font-family: 'Poppins', sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, var(--sdg-light-blue), var(--sdg-white));
            color: var(--sdg-text);
            min-height: 100vh;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
        }

        .header h1 {
            color: var(--sdg-dark-blue);
            margin-bottom: 5px;
            font-weight: 600;
            font-size: 2.2rem;
        }

        .header p {
            color: var(--sdg-text-light);
            margin-top: 0;
            font-weight: 300;
        }

        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 25px;
            max-width: 1400px;
            margin: 0 auto;
        }

        .card {
            background: var(--sdg-white);
            border-radius: 16px;
            box-shadow: 0 8px 20px rgba(0,0,0,0.08);
            padding: 25px;
            text-align: center;
            border: none;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
            position: relative;
            overflow: hidden;
        }

        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 12px 25px rgba(0,0,0,0.15);
        }

        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 5px;
            background: var(--sdg-green);
        }

        .card:nth-child(2)::before {
            background: var(--sdg-orange);
        }

        .card:nth-child(3)::before {
            background: var(--sdg-blue);
        }

        .card:nth-child(4)::before {
            background: var(--sdg-dark-blue);
        }

        .card h2 {
            color: var(--sdg-dark-blue);
            margin-top: 0;
            font-weight: 600;
            font-size: 1.4rem;
        }

        .circle {
            width: 140px;
            height: 140px;
            margin: 0 auto 15px;
            position: relative;
        }

        .circle svg {
            transform: rotate(-90deg);
        }

        .circle .percentage {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-size: 2rem;
            font-weight: 600;
            color: var(--sdg-text);
        }

        .status {
            font-weight: 500;
            margin: 15px 0;
            padding: 8px 12px;
            border-radius: 20px;
            display: inline-block;
            font-size: 0.9rem;
        }

        #pump-status {
            background-color: rgba(16, 185, 129, 0.1);
            color: #10b981;
        }

        #temp-action {
            background-color: rgba(255, 152, 0, 0.1);
            color: var(--sdg-orange);
        }

        #humidity-action {
            background-color: rgba(33, 150, 243, 0.1);
            color: var(--sdg-blue);
        }

        #water-level-status {
            background-color: rgba(21, 101, 192, 0.1);
            color: var(--sdg-dark-blue);
        }

        button {
            margin-top: 10px;
            padding: 12px 24px;
            background: var(--sdg-green);
            border: none;
            border-radius: 8px;
            color: white;
            cursor: pointer;
            transition: all 0.3s ease;
            font-weight: 500;
            font-size: 0.95rem;
            width: 100%;
            max-width: 200px;
        }

        button:hover {
            background: #3d8b40;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }

        .card:nth-child(2) button {
            background: var(--sdg-orange);
        }

        .card:nth-child(2) button:hover {
            background: #e68a00;
        }

        .card:nth-child(3) button {
            background: var(--sdg-blue);
        }

        .card:nth-child(3) button:hover {
            background: #0d8aee;
        }

        .card:nth-child(4) button {
            background: var(--sdg-dark-blue);
        }

        .card:nth-child(4) button:hover {
            background: #0d47a1;
        }

        .alert {
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            font-weight: 500;
        }

        .alert-warning {
            background-color: rgba(255, 152, 0, 0.1);
            color: var(--sdg-orange);
            border-left: 4px solid var(--sdg-orange);
        }

        .alert-success {
            background-color: rgba(16, 185, 129, 0.1);
            color: #10b981;
            border-left: 4px solid #10b981;
        }

        .icon {
            font-size: 2.5rem;
            margin-bottom: 15px;
            display: inline-block;
        }

        .temp-humidity-display {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 10px;
            margin-bottom: 20px;
        }

        .value-display {
            font-size: 2.8rem;
            font-weight: 600;
            color: var(--sdg-text);
        }

        .unit {
            font-size: 1.5rem;
            color: var(--sdg-text-light);
            vertical-align: super;
        }

        .footer {
            text-align: center;
            margin-top: 40px;
            padding: 20px;
            color: var(--sdg-text-light);
            font-size: 0.9rem;
        }

        @media (max-width: 768px) {
            .dashboard {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>

<div class="header">
    <h1>Automatic Plant Care</h1>
    <p>Smart Monitoring System for Healthy Plants</p>
</div>

<div id="dashboard" class="dashboard">
    <div class="card">
        <h2>Soil Moisture</h2>
        <div class="icon">🌱</div>
        <div class="circle">
            <svg width="140" height="140">
                <circle cx="70" cy="70" r="60" stroke="#eee" stroke-width="10" fill="none"/>
                <circle id="moisture-circle" cx="70" cy="70" r="60" stroke="#10b981" stroke-width="10" fill="none" stroke-dasharray="377" stroke-dashoffset="188.5"/>
            </svg>
            <div class="percentage" id="moisture-text">--%</div>
        </div>
        <p class="status" id="pump-status">Pump Status: --</p>
        <div id="moisture-alert"></div>
        <button onclick="controlPump('ON')">Turn ON Pump</button>
        <button onclick="controlPump('OFF')">Turn OFF Pump</button>
    </div>

    <div class="card">
        <h2>Temperature</h2>
        <div class="icon">🌡️</div>
        <div class="temp-humidity-display">
            <span class="value-display" id="temp-value">--</span>
            <span class="unit">°C</span>
        </div>
        <p class="status" id="temp-action">Action: --</p>
        <div id="temp-alert"></div>
    </div>

    <div class="card">
        <h2>Humidity</h2>
        <div class="icon">💧</div>
        <div class="temp-humidity-display">
            <span class="value-display" id="humidity-value">--</span>
            <span class="unit">%</span>
        </div>
        <p class="status" id="humidity-action">Action: --</p>
        <div id="humidity-alert"></div>
    </div>

    <div class="card">
        <h2>Water Level</h2>
        <div class="icon">🚰</div>
        <div class="circle">
            <svg width="140" height="140">
                <circle cx="70" cy="70" r="60" stroke="#eee" stroke-width="10" fill="none"/>
                <circle id="water-level-circle" cx="70" cy="70" r="60" stroke="#1565C0" stroke-width="10" fill="none" stroke-dasharray="377" stroke-dashoffset="75.4"/>
            </svg>
            <div class="percentage" id="water-level-text">--%</div>
        </div>
        <p class="status" id="water-level-status">Level: --</p>
        <div id="water-alert"></div>
    </div>
</div>

<div class="footer">
    <p>Created by <strong>Team Ecotron</strong></p>
    <p id="connection-status">Connected to ESP8266</p>
</div>

<script>
    // Global variables
    let pumpState = false;
    
    // Fetch sensor data every 2 seconds
    function fetchData() {
        fetch('/data')
            .then(response => response.json())
            .then(data => {
                updateUI(data);
                checkAlerts(data);
            })
            .catch(error => {
                console.error('Error fetching data:', error);
                document.getElementById('connection-status').textContent = 'Connection lost. Retrying...';
            });
    }
    
    // Initialize when DOM is loaded
    document.addEventListener('DOMContentLoaded', function() {
        fetchData();
        setInterval(fetchData, 2000);
    });
    
    // Control the water pump
    function controlPump(state) {
        fetch('/control', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ pump: state })
        })
        .then(response => response.json())
        .then(data => {
            console.log('Pump control response:', data);
            document.getElementById('pump-status').textContent = 'Pump Status: ' + data.state;
        })
        .catch(error => {
            console.error('Error controlling pump:', error);
        });
    }
    
    // Check for alert conditions
    function checkAlerts(data) {
        // Moisture alerts
        const moistureAlert = document.getElementById('moisture-alert');
        if (data.moisture < 40) {
            moistureAlert.innerHTML = '<div class="alert alert-warning">Low moisture! Watering recommended.</div>';
        } else if (data.moisture > 80) {
            moistureAlert.innerHTML = '<div class="alert alert-warning">High moisture! Reduce watering.</div>';
        } else {
            moistureAlert.innerHTML = '<div class="alert alert-success">Moisture level is optimal.</div>';
        }
    
        // Temperature alerts
        const tempAlert = document.getElementById('temp-alert');
        if (data.temperature < 20) {
            tempAlert.innerHTML = '<div class="alert alert-warning">Low temperature! Consider warming.</div>';
        } else if (data.temperature > 30) {
            tempAlert.innerHTML = '<div class="alert alert-warning">High temperature! Cooling recommended.</div>';
        } else {
            tempAlert.innerHTML = '<div class="alert alert-success">Temperature is optimal.</div>';
        }
    
        // Humidity alerts
        const humidityAlert = document.getElementById('humidity-alert');
        if (data.humidity < 40) {
            humidityAlert.innerHTML = '<div class="alert alert-warning">Low humidity! Consider humidification.</div>';
        } else if (data.humidity > 70) {
            humidityAlert.innerHTML = '<div class="alert alert-warning">High humidity! Ventilation recommended.</div>';
        } else {
            humidityAlert.innerHTML = '<div class="alert alert-success">Humidity is optimal.</div>';
        }
    
        // Water level alerts
        const waterAlert = document.getElementById('water-alert');
        if (data.waterLevel < 20) {
            waterAlert.innerHTML = '<div class="alert alert-warning">Low water level! Refill required.</div>';
        } else if (data.waterLevel > 80) {
            waterAlert.innerHTML = '<div class="alert alert-warning">Water level is high.</div>';
        } else {
            waterAlert.innerHTML = '<div class="alert alert-success">Water level is adequate.</div>';
        }
    }
    
    // Update the UI with sensor data
    function updateUI(data) {
        // Moisture
        if (data.moisture !== undefined) {
            document.getElementById('moisture-text').textContent = data.moisture.toFixed(0) + '%';
            const moistureOffset = 377 - (data.moisture / 100) * 377;
            document.getElementById('moisture-circle').setAttribute('stroke-dashoffset', moistureOffset);
            document.getElementById('pump-status').textContent = 'Pump Status: ' + (data.pumpStatus ? 'ON' : 'OFF');
        }
    
        // Temperature
        if (data.temperature !== undefined) {
            document.getElementById('temp-value').textContent = data.temperature.toFixed(1);
            const tempAction = data.temperature < 20 ? 'Increase' : data.temperature > 30 ? 'Decrease' : 'Optimal';
            document.getElementById('temp-action').textContent = 'Action: ' + tempAction;
        }
    
        // Humidity
        if (data.humidity !== undefined) {
            document.getElementById('humidity-value').textContent = data.humidity.toFixed(1);
            const humAction = data.humidity < 40 ? 'Increase' : data.humidity > 70 ? 'Decrease' : 'Optimal';
            document.getElementById('humidity-action').textContent = 'Action: ' + humAction;
        }
    
        // Water Level
        if (data.waterLevel !== undefined) {
            document.getElementById('water-level-text').textContent = data.waterLevel.toFixed(0) + '%';
            const waterOffset = 377 - (data.waterLevel / 100) * 377;
            document.getElementById('water-level-circle').setAttribute('stroke-dashoffset', waterOffset);
    
            const waterStatus = data.waterLevel < 20 ? 'Low' : data.waterLevel > 80 ? 'High' : 'Medium';
            document.getElementById('water-level-status').textContent = 'Level: ' + waterStatus;
        }
    }
</script>
</body>
</html>)=====";

  server.send(200, "text/html", html);
}

// Handle JSON data requests
void handleSensorData() {
  StaticJsonDocument<256> jsonDoc;
  
  jsonDoc["moisture"] = moisturePercent;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["waterLevel"] = waterLevelPercent;
  jsonDoc["pumpStatus"] = pumpStatus;
  
  String response;
  serializeJson(jsonDoc, response);
  
  server.send(200, "application/json", response);
}

// Handle actuator control requests
void handleActuatorControl() {
  String postBody = server.arg("plain");
  
  StaticJsonDocument<64> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, postBody);
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Handle pump control
  if (jsonDoc.containsKey("pump")) {
    String pumpCommand = jsonDoc["pump"].as<String>();
    
    if (pumpCommand == "ON") {
      digitalWrite(PUMP_PIN, HIGH);
      pumpStatus = true;
      Serial.println("Pump turned ON via web interface");
    } else if (pumpCommand == "OFF") {
      digitalWrite(PUMP_PIN, LOW);
      pumpStatus = false;
      Serial.println("Pump turned OFF via web interface");
    }
    
    StaticJsonDocument<64> response;
    response["device"] = "pump";
    response["state"] = pumpStatus ? "ON" : "OFF";
    
    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
  } else {
    server.send(400, "application/json", "{\"error\":\"Unknown command\"}");
  }
}

// Handle 404 errors
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// Convert MAC address to string
String macToString(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], HEX);
    if (i < 5) result += ":";
  }
  return result;
}