// This #include statement was automatically added by the Particle IDE.
#include <captouch.h>

//the size of the arrays (15, but goes from 0-14)
int arraySize = 15;

const int motorDown    = D3;   //H-Bridge control to make the motor go down
const int motorUp      = D5;   //H-Bridge control to make the motor go up

//motorized lisder
const int wiper        = A0;   //Position of fader relative to GND (Analog 0)
const int touchSend    = D4;   //Send pin for Capacitance Sensing Circuit (Digital 4)
const int touchReceive = D7;   //Receive pin for Capacitance Sensing Circuit (Digital 7)

//linear actuators
#define PIN_SERVO (D1)
#define PIN_SERVO2 (D2)
#define PIN_SERVO3 (D0)
#define PIN_SERVO4 (A4)

Servo myServo;
Servo myServo2;
Servo myServo3;
Servo myServo4;


int   faderMax        = 3800;     //Value read by fader's maximum position (0-1023)
int   faderMin        = 100;     //Value read by fader's minimum position (0-1023)

//used for singlestep();
int millisBetweenSteps = 10000;
int curMillis;
int prevStepMillis;


//g controls the movement of slider/fader and linear actuator positions
int g = 0;

//for loop
int i = 0;

//it convert the position from 0-14 to fader/slider positioner
int testArray[15];
//array for the fader/slider
int testArray2[15] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

//for convertions
int reversedMap;
int mappedMin = 0;
int mappedMax = 14;


//for hear rate
float personHR1[15] = {61, 67, 90, 80, 61, 61, 67, 90, 80, 61, 61, 67, 90, 80, 61};
float personHR2[15] = {80, 89, 110, 90, 100, 80, 89, 110, 90, 100, 80, 89, 110, 90, 100};
float personHR3[15] = {100, 107, 70, 95, 80, 100, 107, 70, 95, 80, 100, 107, 70, 95, 80};
float personHR4[15] = {119, 97, 65, 80, 120, 119, 97, 65, 80, 120, 119, 97, 65, 80, 120};

//here is the hr converted to stroke position on linear actuator
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
    //for the slider
    pinMode (motorUp, OUTPUT);
    pinMode (motorDown, OUTPUT);
    
    //for the linear actuator
    myServo.attach(PIN_SERVO);
    myServo2.attach(PIN_SERVO2);
    myServo3.attach(PIN_SERVO3);
    myServo4.attach(PIN_SERVO4);
    
    minstrokToSlide();
    
    //For capacitor library
    Touch.setup();
    
    Serial.begin(9600);
    
    //converts hr to linear actuator position
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

//converts the steps from 0-14 to fader position to act as the time line
void minstrokToSlide(){
    for(m = 0; m < arraySize; m++){
        reversedMap = map(testArray2[m], mappedMin, mappedMax, faderMin, faderMax);
        testArray[m] = reversedMap;
    }

}

//
void isTheSliderTouched(){
    
    //if the slider is not touched, then the slider/fader will updates its position and move the linear actuators accordingly to the slider.
    if (isTouched == false) {
        
        //update fader/slider
        updateFader(testArray[g]);
        
        //set strok position on linear actuators
        SetStrokePerc4(persone4Mapped[g]);
        SetStrokePerc(persone1Mapped[g]);
        SetStrokePerc2(persone2Mapped[g]);
        SetStrokePerc3(persone3Mapped[g]);
        
        //takes a step and wait
        singleStep();
        
        //this is depende on which way the fader/slider should move. Remeber to also look at singleStep() to shit g-- to g++ if the other way is chosen
        if(g < 0){
            g = 14;
        }
        /*
        //reset the slider when it hits 15
        if(g >= arraySize){
            g = 0;
        
        }*/
    
    //if the slider is touch, the slider will not move and the position it is moved to, is the new starting point.
    }else if(isTouched == true){
        
        g = map(faderPosition(), faderMin, faderMax, mappedMin, mappedMax);
        digitalWrite(motorUp,LOW);
        digitalWrite(motorDown,LOW);
        singleStep2();
        
    }
}

//updates the fader position
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


//check is the slider is touched. From captouch lib
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

//gets fader position
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


//converts HR to linear actuator position
void HRtoLinearActPos(float HRarray[], float mappedArray[]){
    for(i = 0; i < arraySize; i++ ){
        float mappedValues = map(HRarray[i],  minHR,  maxHR, minStroke, maxStroke);
        mappedArray[i] = mappedValues;
    }
}

//ensure to pause between the steps and take a new step
void singleStep() {
 if (curMillis - prevStepMillis >= millisBetweenSteps) {
    prevStepMillis += millisBetweenSteps;
    //g++;
    g--;
 }
}


//pause without taking a new step
void singleStep2() {
 if (curMillis - prevStepMillis >= millisBetweenSteps) {
        prevStepMillis += millisBetweenSteps;
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
