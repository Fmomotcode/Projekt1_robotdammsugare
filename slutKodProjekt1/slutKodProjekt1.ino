/*
*Name: Robotdammsugare
*Author: Filip Momot
*Date:
*Description: This project uses motors, sensors and raspberry pi zero W with a camera to find garbage.
*/

//Libraries
  //#include <InfraredTypes.h>
  #include <Servo.h>
  #include <NewPing.h>

//constants 
  const int motorPins[] = {7, 9, 10, 8};
  const int ENLeft = 3;
  const int ENRight = 4;

  const int fram = 90;
  const int hoger = 30;
  const int vanster = 150;

  const long interval = 50; 
  const int analogThreshold = 200;
// Global variables
  int triggerPin = 12; 
  int echoPin = 13;  
  int MaxDistans = 80;

  int digitalSensorPinLeft = 2; 
  int analogSensorPinLeft = A0; 

  int digitalSensorPinRight = 11;
  int analogSensorPinRight = A1;
  unsigned long lastTime = 0;

  String currentAction = "FRAM"; // vad den ska göra
  unsigned long actionStartTime = 0; 
  const unsigned long turnTime = 150;

// Construct objects
  NewPing sonar(triggerPin, echoPin, MaxDistans);
  Servo esc;
  Servo myservo; 
  
void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(motorPins[i], OUTPUT);
  }
  pinMode(ENLeft, OUTPUT);
  pinMode(ENRight, OUTPUT);
  
  pinMode(digitalSensorPinLeft, INPUT);
  pinMode(analogSensorPinLeft, INPUT);
  pinMode(digitalSensorPinRight, INPUT);
  pinMode(analogSensorPinRight, INPUT);

  esc.attach(5);
  esc.writeMicroseconds(1000); // MIN
  delay(4000);

  myservo.attach(6);
// commusnitaction 
  Serial.begin(9600); 
//settings 
}

void loop() {
  brushless();  // Always take in air

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "VÄNSTER" || cmd == "HÖGER") {
      currentAction = cmd;
      actionStartTime = millis();
    }
    else if (cmd == "FRAM") {
      currentAction = "FRAM";
    }
  }

  irSensor();

  int avstand = ultrasonic();
  if (avstand > 0 && avstand <= MaxDistans) {
    stopMotor();
    delay(200);
    servo();
    return;
  }

  if (currentAction == "VÄNSTER") {
    vansterMotor();
    if (millis() - actionStartTime >= turnTime) {
      currentAction = "FRAM";
    }
  }
  else if (currentAction == "HÖGER") {
    hogerMotor();
    if (millis() - actionStartTime >= turnTime) {
      currentAction = "FRAM";
    }
  }
  else {
    framMotor();
  }
}


//This function makes the vaccum cleaner drive forward
void framMotor() {
  digitalWrite(motorPins[0], HIGH);
  digitalWrite(motorPins[1], LOW);
  digitalWrite(motorPins[2], HIGH);
  digitalWrite(motorPins[3], LOW);

  analogWrite(ENLeft, 200);
  analogWrite(ENRight, 200);
}

//This function makes the vaccum cleaner turn right
void hogerMotor() {
  digitalWrite(motorPins[0], HIGH);
  digitalWrite(motorPins[1], LOW);

  digitalWrite(motorPins[2], HIGH);
  digitalWrite(motorPins[3], LOW);

  analogWrite(ENLeft, 120);
  analogWrite(ENRight, 200);
}

//This function makes the vaccum cleaner turn left
void vansterMotor() {
  digitalWrite(motorPins[0], HIGH);
  digitalWrite(motorPins[1], LOW);

  digitalWrite(motorPins[2], HIGH);
  digitalWrite(motorPins[3], LOW);

  analogWrite(ENRight, 120);
  analogWrite(ENLeft, 200);
}

void stopMotor() {
  digitalWrite(motorPins[0], LOW);
  digitalWrite(motorPins[1], LOW);

  digitalWrite(motorPins[2], LOW);
  digitalWrite(motorPins[3], LOW);

  analogWrite(ENRight, 0);
  analogWrite(ENLeft, 0);
}

//This function takes the data from the ultrasonic sensor
int ultrasonic() {
  delayMicroseconds(10); // lagom paus mellan pingar

  int distans = sonar.ping_cm(); // pinga EN gång

  Serial.print("Avstånd: ");
  Serial.print(distans);
  Serial.println("cm");

  if (distans > 0 && distans <= MaxDistans) {
    Serial.println("För nära!!!");
  }
  return distans;
}

int readStableSensor(int pin, int samples = 5) {
  int sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += digitalRead(pin);
    delay(5);
  }
  return (sum > samples / 2) ? HIGH : LOW;
}

//This function takes the data from irSensor and tells if
// the vaccum cleaner is going to fall
void irSensor() {
  if (millis() - lastTime > interval) {
    lastTime = millis();

    int digitalValueLeft  = readStableSensor(digitalSensorPinLeft);
    int analogValueLeft   = analogRead(analogSensorPinLeft);
    int digitalValueRight = readStableSensor(digitalSensorPinRight);
    int analogValueRight  = analogRead(analogSensorPinRight);

    bool leftCliff  = (digitalValueLeft == LOW && analogValueLeft < analogThreshold);
    bool rightCliff = (digitalValueRight == LOW && analogValueRight < analogThreshold);

    if (leftCliff) {
      Serial.println("Vänster kant!");
      currentAction = "HÖGER";
      actionStartTime = millis();
    }
    else if (rightCliff) {
      Serial.println("Höger kant!");
      currentAction = "VÄNSTER";
      actionStartTime = millis();
    }
    else {
      currentAction = "FRAM";
    }
  }
}


//This function turns on the brushless motor to suck in the air 
void brushless() {
  static int speed = 1000; 
  const int maxSpeed = 2000;
  const int addSpeed = 5;      
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate > 50) { 
    lastUpdate = now;
    if (speed < maxSpeed) {
      speed = speed + addSpeed;  
    }
    esc.writeMicroseconds(speed);
  }
}

//This function makes the servo move to check diffrent angles 
void servo() {
  myservo.write(fram);
  delay(250);
  int framDist = ultrasonic();

  myservo.write(hoger);
  delay(300);
  int hogerDist = ultrasonic();

  myservo.write(vanster);
  delay(300);
  int vansterDist = ultrasonic();

  if (hogerDist > 0 && hogerDist < MaxDistans) {
    currentAction = "VÄNSTER";
    actionStartTime = millis();
  }
  else if (vansterDist > 0 && vansterDist < MaxDistans) {
    currentAction = "HÖGER";
    actionStartTime = millis();
  }
  else {
    currentAction = "FRAM";
  }
}