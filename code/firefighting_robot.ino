#include <Servo.h> 


const int ENA = 5;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 6;

const int TRIG = 4;
const int ECHO = 2;

const int FL_LEFT  = A0;
const int FL_RIGHT = A1;

const int PUMP  = 3;
const int LED_R = A2;
const int LED_G = A3;
const int LED_B = A4;


int baseSpeed   = 170;   // PWM for forward motion
int turnSpeed   = 150;   // PWM for pivot turns
int avoidDist   = 20;    // Obstacle threshold (cm)
int flameThresh = 700;   // IR analog threshold (lower = fire)
int sprayMs     = 4000;  // Pump on-duration (ms)



void motorsStop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void motorsForward(int spdL, int spdR) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, spdL);
  analogWrite(ENB, spdR);
}

void motorsBackward(int spdL, int spdR) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, spdL);
  analogWrite(ENB, spdR);
}

void motorsTurnLeft(int spd) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void motorsTurnRight(int spd) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}



long readUltrasonicCM() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long dur = pulseIn(ECHO, HIGH, 30000);
  if (dur == 0) return 999;   // no echo / out of range
  return dur / 58;
}



void pumpOn(bool on) {
  digitalWrite(PUMP, on ? LOW : HIGH);
}

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_R, r);
  digitalWrite(LED_G, g);
  digitalWrite(LED_B, b);
}



bool detectFlame() {
  int sumL = 0, sumR = 0;

  for (int i = 0; i < 5; i++) {
    sumL += analogRead(FL_LEFT);
    sumR += analogRead(FL_RIGHT);
    delay(5);
  }

  int avgL = sumL / 5;
  int avgR = sumR / 5;

  return (avgL < flameThresh || avgR < flameThresh);
}



void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(PUMP, OUTPUT);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pumpOn(false);
}



void loop() {

  long dist  = readUltrasonicCM();
  bool flame = detectFlame();

  Serial.print("Distance: ");
  Serial.print(dist);
  Serial.print(" cm  |  Flame: ");
  Serial.println(flame);


  if (flame) {
    setLED(true, false, false);   // RED

    motorsStop();
    delay(200);

    pumpOn(true);
    delay(sprayMs);
    pumpOn(false);

    // Reverse after extinguishing to clear the area
    motorsBackward(150, 150);
    delay(500);

    motorsStop();
    delay(300);
  }


  else if (dist < avoidDist) {
    setLED(false, true, false);   // GREEN

    motorsBackward(150, 150);
    delay(400);

    motorsTurnRight(turnSpeed);
    delay(500);
  }

  else {
    setLED(false, false, true);   // BLUE
    motorsForward(baseSpeed, baseSpeed);
  }

  delay(50);
}
