#include <Arduino.h>

#include <Servo.h>
#include "Configuration.h"
#include "Commands.h"

#define  VERSION "1.0.0"

// put function declarations here:
int myFunction(int, int);
Servo *thisServo;
Commands *commandProcessor;

void setup() {
  Serial.begin(119500); 
  delay(2);
  Serial.print("ServoTester version "); Serial.println(VERSION);
  delay(2000);
  pinMode(LED_BUILTIN, OUTPUT);
  thisServo = new Servo();
  thisServo->attach(SERVO_PIN_NO, MIN_PW_DEFAULT, MAX_PW_DEFAULT);

  commandProcessor  = new  Commands(thisServo);
}

bool curState=false;
unsigned long lastTime = millis();

void loop()
{
  if ((millis() - lastTime) > 1000)
  { // once per period
    if (curState)
      digitalWrite(LED_BUILTIN, HIGH);
    else
      digitalWrite(LED_BUILTIN, LOW);
    curState = !curState;
  }
  commandProcessor->nextRead(); // read (and decode) any command
  commandProcessor->loop();     // run the loop...
}
