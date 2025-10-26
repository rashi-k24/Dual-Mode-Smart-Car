#include <Servo.h>

Servo myservo;

// Ultrasonic pins
const int trigPin = 2;
const int echoPin = 4;

// Motor driver pins
// RIGHT MOTOR → OUT1, OUT2
const int ENA = 5;   // PWM Right motor
const int IN1 = 7;
const int IN2 = 8;

// LEFT MOTOR → OUT3, OUT4
const int ENB = 6;   // PWM Left motor
const int IN3 = 9;
const int IN4 = 10;

// Servo pin
const int servoPin = 11;

int distanceThreshold = 60; // cm
int mode = 0; // 0 → AUTO, 1 → MANUAL

// Get distance function with averaging filter
long getDistance() {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 20000);
    long d = duration * 0.034 / 2;
    if (d == 0 || d > 200) d = 200;
    sum += d;
    delay(10);
  }
  return sum / 5;
}

// Motor controls
void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, 180); analogWrite(ENB, 200);
}

void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, 180); analogWrite(ENB, 200);
}

void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void turnLeft() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, 180); analogWrite(ENB, 200);
  delay(350);
  stopCar();
}

void turnRight() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, 180); analogWrite(ENB, 200);
  delay(350);
  stopCar();
}

void setup() {
  Serial.begin(9600); // NodeMCU communication
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  myservo.attach(servoPin); myservo.write(90); // center
}

void loop() {
  // Check for commands from NodeMCU
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); // remove whitespace/newline
    if (cmd == "AUTO") mode = 0;
    else if (cmd == "MANUAL") mode = 1;

    // Manual commands only if mode = 1
    if (mode == 1) {
      if (cmd == "F") forward();
      else if (cmd == "B") backward();
      else if (cmd == "L") turnLeft();
      else if (cmd == "R") turnRight();
      else if (cmd == "S") stopCar();
    }
  } else {
    // If NodeMCU disconnected → auto mode
    mode = 0;
  }

  // AUTO MODE logic
  if (mode == 0) {
    long distance = getDistance();
    if (distance > distanceThreshold) forward();
    else {
      stopCar(); delay(200);
      // Scan left
      myservo.write(150); delay(500); long leftDist = getDistance();
      // Scan right
      myservo.write(30); delay(500); long rightDist = getDistance();
      // Reset to center
      myservo.write(90); delay(200);
      if (leftDist > rightDist) turnLeft();
      else turnRight();
    }
  }
}
