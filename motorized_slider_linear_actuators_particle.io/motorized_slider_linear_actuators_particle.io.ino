// This #include statement was automatically added by the Particle IDE.
#include <captouch.h>

//Arduino Pin Assignments
int arraySize = 15;
const int motorDown    = D3;   //H-Bridge control to make the motor go down
const int motorUp      = D5;   //H-Bridge control to make the motor go up

//Inputs
const int wiper        = A0;   //Position of fader relative to GND (Analog 0)
const int touchSend    = D4;   //Send pin for Capacitance Sensing Circuit (Digital 7)
const int touchReceive = D7;   //Receive pin for Capacitance Sensing Circuit (Digital 8)

#define PIN_SERVO (D1)
#define PIN_SERVO2 (D2)
#define PIN_SERVO3 (D0)
#define PIN_SERVO4 (A4)

Servo myServo;
Servo myServo2;
Servo myServo3;
Servo myServo4;

//Variables
int   faderMax        = 3800;     //Value read by fader's maximum position (0-1023)
int   faderMin        = 100;     //Value read by fader's minimum position (0-1023)

int reversedMap;
int mappedMin = 0;
int mappedMax = 14;
int millisBetweenSteps = 10000;
int curMillis;
int prevStepMillis;

bool positionUpdated = true;


int g = 0;
int i = 0;

int testArray[15];
int testArray2[15] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

float personHR1[15] = {61, 67, 90, 80, 61, 61, 67, 90, 80, 61, 61, 67, 90, 80, 61};
float personHR2[15] = {80, 89, 110, 90, 100, 80, 89, 110, 90, 100, 80, 89, 110, 90, 100};
float personHR3[15] = {100, 107, 70, 95, 80, 100, 107, 70, 95, 80, 100, 107, 70, 95, 80};
float personHR4[15] = {119, 97, 65, 80, 120, 119, 97, 65, 80, 120, 119, 97, 65, 80, 120};

float persone1Mapped[15];
float persone2Mapped[15];
float persone3Mapped[15];
float persone4Mapped[15];
float minHR = 60;
float maxHR = 120;
int mappedHR;

//Linear stroke lengths
float minStroke = 5;
float maxStroke =95;

int m = 0;
int position2;

bool isTouched = false;

int startPos;


//From capacitiy library too measure if the slider is touched
CapTouch Touch(touchSend, touchReceive);

void setup() {
    
    pinMode (motorUp, OUTPUT);
    pinMode (motorDown, OUTPUT);
    
    myServo.attach(PIN_SERVO);
    myServo2.attach(PIN_SERVO2);
    myServo3.attach(PIN_SERVO3);
    myServo4.attach(PIN_SERVO4);
    
    minstrokToSlide();
    Particle.variable("wiperValue", position2);
    Particle.variable("arrayPos", g);
    Particle.variable("bool", isTouched);
    //For capacitor library
    Touch.setup();
    Serial.begin(9600);
    
    HRtoLinearActPos(personHR1, persone1Mapped);
    HRtoLinearActPos(personHR2, persone2Mapped);
    HRtoLinearActPos(personHR3, persone3Mapped);
    HRtoLinearActPos(personHR4, persone4Mapped);
    //calibrateFader();
    g = map(faderPosition(), faderMin, faderMax, mappedMin, mappedMax );

}

void loop() {
    
    curMillis = millis();
    position2 = analogRead(A0);
    touchListerner();
    isTheSliderTouched();
    delay(100);
    
}

void minstrokToSlide(){
    for(m = 0; m < arraySize; m++){
        reversedMap = map(testArray2[m], mappedMin, mappedMax, faderMin, faderMax);
        testArray[m] = reversedMap;
        //float testarray = map(testArray2[m], (float) mappedMin, (float) mappedMax, (float) slideMin, (float) slideMax);
        //testArray2[m] = testarray;
    }

}
void isTheSliderTouched(){
    //if the slider is not touched it will slide from side to side

    if (isTouched == false) {
    
        updateFader(testArray[g]);
        SetStrokePerc4(persone4Mapped[g]);
        SetStrokePerc(persone1Mapped[g]);
        SetStrokePerc2(persone2Mapped[g]);
        SetStrokePerc3(persone3Mapped[g]);
         singleStep();
        
        if(g >= arraySize){
            g = 0;
        
        }
            
    }else if(isTouched == true){
        
        g = map(faderPosition(), faderMin, faderMax, mappedMin, mappedMax);
        digitalWrite(motorUp,LOW);
        digitalWrite(motorDown,LOW);
        singleStep2();
        
    }
}

void updateFader(int position) {
    if (position < analogRead(wiper) - 10 && position > faderMin && !isTouched) {
        digitalWrite(motorDown, HIGH);
        while (position < analogRead(wiper) - 10 && !isTouched) {};  //Loops until motor is done moving
        digitalWrite(motorDown, LOW);
    }
    else if (position > analogRead(wiper) + 10 && position < faderMax && !isTouched) {
        digitalWrite(motorUp, HIGH);
        
        
        while (position > analogRead(wiper) + 10 && !isTouched) {}; //Loops until motor is done moving
        digitalWrite(motorUp, LOW);
    }

}

void touchListerner(){
    //Instead of 10 m ohm only use 100k
    CapTouch::Event touchEvent = Touch.getEvent();
    if (touchEvent == CapTouch::TouchEvent) {
        isTouched = true;
    } 
    else if (touchEvent == CapTouch::ReleaseEvent) {
        
    isTouched = false;
    
    }
}

int faderPosition() {
    int position = analogRead(wiper);
    int returnValue = 0;

    if (position <= faderMin) {
        returnValue = 100;
    }
    else if (position >= faderMax) {
        returnValue = 3800;
    }
    else {
        returnValue = analogRead(wiper);
    }

    return returnValue;
}
void HRtoLinearActPos(float HRarray[], float mappedArray[]){
    for(i = 0; i < arraySize; i++ ){
        float mappedValues = map(HRarray[i],  minHR,  maxHR, minStroke, maxStroke);
        mappedArray[i] = mappedValues;
    }
}


void singleStep() {
 if (curMillis - prevStepMillis >= millisBetweenSteps) {
 prevStepMillis += millisBetweenSteps;
 //digitalWrite(motorUp, LOW);
    g++;                
 }
}
void singleStep2() {
 if (curMillis - prevStepMillis >= millisBetweenSteps) {
 prevStepMillis += millisBetweenSteps;
 //digitalWrite(motorUp, LOW);
                  
 }
}


//The storke for linear actuator (this is in procentage)
void SetStrokePerc(float strokePercentage)
{
  if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
  {
    int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
    myServo.writeMicroseconds((int) usec);
  }
}

void SetStrokePerc2(float strokePercentage)
{
  if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
  {
    int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
    myServo2.writeMicroseconds((int) usec);
  }
}
void SetStrokePerc3(float strokePercentage)
{
  if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
  {
    int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
    myServo3.writeMicroseconds((int) usec);
  }
}
void SetStrokePerc4(float strokePercentage)
{
  if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
  {
    int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
    myServo4.writeMicroseconds((int)usec);
  }
}
