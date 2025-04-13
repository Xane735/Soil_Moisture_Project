const int trigPin = 9;
const int echoPin = 10;
const int MAX_LEVEL = 20; // Change according to Tub length
const int MIN_LEVEL = 5;  // Change according to sensor minimum range

long duration;
float distanceCm;

void setup()
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send 10us pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo time
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance (in cm)
  distanceCm = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");
  if (distanceCm > MAX_LEVEL)
  {
    Serial.print("WARNING, WATER LEVEL LOW!!");
  }
  else if (distanceCm < MIN_LEVEL)
  {
    Serial.print("WARNING, TUB OVERFILLED, REDUCE WATER LEVELS!");
  }

  delay(1000);
}
