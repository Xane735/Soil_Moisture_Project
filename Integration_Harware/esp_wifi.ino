#include <ESP8266WiFi.h>

// Replace with your WiFi credentials
const char* ssid = "Galaxy A13 28C5";
const char* password = "vdwu1706";

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Nothing here
}
