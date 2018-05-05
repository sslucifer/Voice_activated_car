#include <AFMotor.h>      //add Adafruit Motor Shield library
#include <Servo.h>        //add Servo Motor library            
#include <NewPing.h>      //add Ultrasonic sensor library
//#include "pitches.h"
//notes in the melody
//int melody[]={NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
//note durations. 4=quarter note / 8=eighth note
//int noteDurations[]={4, 8, 8, 4, 4, 4, 4, 4};

#define TRIG_PIN A0 // Pin A0 on the Motor Drive Shield soldered to the ultrasonic sensor
#define ECHO_PIN A1 // Pin A1 on the Motor Drive Shield soldered to the ultrasonic sensor
#define MAX_DISTANCE 300 // sets maximum useable sensor measuring distance to 300cm
#define MAX_SPEED 160 // sets speed of DC traction motors to 150/250 or about 70% of full speed - to get power drain down.
//#define MAX_SPEED_OFFSET 40 // this sets offset to allow for differences between the two DC traction motors
#define COLL_DIST 30 // sets distance at which robot stops and reverses to 30cm
#define TURN_DIST COLL_DIST+20 // sets distance at which robot veers away from object
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE); // sets up sensor library to use the correct pins to measure distance.

AF_DCMotor leftMotor1(1, MOTOR12_1KHZ); // create motor #1 using M1 output on Motor Drive Shield, set to 1kHz PWM frequency
AF_DCMotor leftMotor2(2, MOTOR12_1KHZ); // create motor #2, using M2 output, set to 1kHz PWM frequency
AF_DCMotor rightMotor1(3, MOTOR34_1KHZ);// create motor #3, using M3 output, set to 1kHz PWM frequency
AF_DCMotor rightMotor2(4, MOTOR34_1KHZ);// create motor #4, using M4 output, set to 1kHz PWM frequency
Servo myservo;  // create servo object to control a servo 

int calibrationTime = 30;
long unsigned int lowIn;        
long unsigned int pause = 5000; 
 
boolean lockLow = true;
boolean takeLowTime; 
int pirPin = A5;    //the digital pin connected to the PIR sensor's output

int leftDistance, rightDistance; //distances on either side
int curDist = 0;
String motorSet = "";
String voice;
//String voice1;
int speedSet = 0;
int LED1 = A2; //define RED pin
int LED2 = A3; //define GREEN pin
int LED3 = A4; //define BLUE pin
//int buzzerPin = A5; //define buzzer pin
int avg,avgr,avgl;
long duration;
int distance;


//-------------------------------------------- SETUP LOOP ----------------------------------------------------------------------------
void setup() {
  pinMode(LED1, OUTPUT); //A2 is output pin
  pinMode(LED2, OUTPUT); //A3 is output pin
  pinMode(LED3, OUTPUT); //A4 is output pin
  pinMode(pirPin, INPUT);//A5 is input pin 
  //pinMode(buzzerPin, OUTPUT); 
  Serial.begin(9600);
  myservo.attach(10);  // attaches the servo on pin 10 (SERVO_1 on the Motor Drive Shield to the servo object 
  LEDOFF();
  digitalWrite(pirPin, LOW);
 
  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
  delay(1000); // delay for one seconds
 }
//------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------MAIN LOOP ------------------------------------------------------------------------------
void loop() {
  myservo.write(90); // tells the servo to position at 90-degrees ie. facing forward.
  while (Serial.available()>0){ //Check if there is an available byte to read
     delay(10); //Delay added to make thing stable 
    char c = Serial.read(); //Conduct a serial read
    if (c == '#') {break;} //Exit the loop when the # is detected after the word
    voice += c; //Shorthand for voice = voice + c
   }
  if (voice.length() > 0){
    if(voice == "*jarvis go ahead"){
      forward_car();
      }
    else if(voice == "*jarvis go back"){
      back_car();
      }
    else if(voice == "*jarvis turn right") {
      right_car();
    }
    else if(voice == "*jarvis turn left") {
      left_car();
    }
    else if(voice == "*Jarvis power up"||voice == "*Jarvis wake up"||voice == "*Jarvis on") {
       CRED();        
    }
    else if(voice == "*Jarvis power down"||voice == "*Jarvis sleep"||voice == "*Jarvis hibernate"||voice == "*power down") {
      LEDOFF();
    }
    /*else if(voice == "*Jarvis say hello"||voice == "*say hello to everyone") {
       CPURPLE();
      buzzer_on();
    }*/
    else if(voice == "*Jarvis sense")
    {
      LEDOFF();
      sense();
    }
    els
    e if(voice == "*Jarvis stop"||voice == "*Jarvis take a break"||voice == "*Jarvis careful") {
      moveStop();
    }
    else if(voice == "*jarvis check your status"||voice == "*Jarvis what is your status"||voice == "*Jarvis status"){
         CPURPLE();
         forward_car();
         back_car();
         right_car();
         left_car();
         moveStop();
         CRED();
    }  
    else if(voice == "*Jarvis Disco")
    {
      mix();
    }
    else if(voice == "*Jarvis autopilot on"||voice == "*Jarvis drive yourself"||voice == "*Jarvis self drive")
    {
        CGREEN();
        moveForward();  // move forward
        delay(500);
    }  
     voice=""; //Reset the variable after initiating
  }   
 }
//-------------------------------------------------------------------------------------------------------------------------------------

void changePath() {
  avgr=avgl=0;
  moveStop();   // stop forward movement
  myservo.write(36);  // check distance to the right
    delay(500);
    /*for(int i=0;i<20;i++){
    avgr = avgr+readPing(); //set right distance
    delayMicroseconds(10);
    }*/
    rightDistance=readPing();
    Serial.print("\nrightDistance: ");
    Serial.print(rightDistance);
    delay(500);
    myservo.write(144);  // check distace to the left
    delay(500);
    /*for(int j=0;j<20;j++){
      avgl=avgl+readPing();
      delayMicroseconds(10);
    }*/
    leftDistance = readPing(); //set left distance
    Serial.print("\nleftDistance: ");
    Serial.print(leftDistance);
    delay(500);
    myservo.write(90); //return to center
    delay(100);
    compareDistance();
  }

  
void compareDistance()   // find the longest distance
{
  if (leftDistance>rightDistance) //if left is less obstructed 
  {
    turnLeft();
  }
  else if (rightDistance>leftDistance) //if right is less obstructed
  {
    turnRight();
  }
   else //if they are equally obstructed
  {
    turnAround();
  }
}


//-------------------------------------------------------------------------------------------------------------------------------------

int readPing() { // read the ultrasonic sensor distance
  delay(70);   
  /*unsigned int uS = sonar.ping();
  int cm = uS/US_ROUNDTRIP_CM;
  return cm;*/
  digitalWrite(TRIG_PIN,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);
  duration=pulseIn(ECHO_PIN,HIGH);
  distance=(duration*0.034)/2;
  return distance;

}
//-------------------------------------------------------------------------------------------------------------------------------------
void moveStop() {leftMotor1.run(RELEASE); leftMotor2.run(RELEASE); rightMotor1.run(RELEASE); rightMotor2.run(RELEASE);}  // stop the motors.
//-------------------------------------------------------------------------------------------------------------------------------------
void moveForward() {
   
    while(!Serial.available()){
     delay(10); 
     myservo.write(90);  // move eyes forward
        delay(90);
        curDist=readPing();
        Serial.print("\ncurDist: ");
        Serial.print(curDist);
      if (curDist < COLL_DIST) {changePath();}
      else{
    motorSet = "FORWARD";speedSet=170;
    leftMotor1.run(FORWARD);      // turn it on going forward
    leftMotor2.run(FORWARD);      // turn it on going forward
    rightMotor1.run(FORWARD);     // turn it on going forward
    rightMotor2.run(FORWARD);     // turn it on going forward
  /*for (speedSet = 0; speedSet < MAX_SPEED; speedSet +=2) // slowly bring the speed up to avoid loading down the batteries too quickly
  {
    myservo.write(90);  // move eyes forward
        delay(90);
        /*for(int a=0;a<20;a++){
        avg =avg+ readPing();   // read distance
        delayMicroseconds(10);
       ///////// }
        curDist=readPing();
        Serial.print("\ncurDist: ");
        Serial.print(curDist);
        if (curDist < COLL_DIST) {changePath();}  // if forward is blocked change direction
    else{
    leftMotor1.setSpeed(speedSet);
    leftMotor2.setSpeed(speedSet);
    rightMotor1.setSpeed(speedSet); 
    rightMotor2.setSpeed(speedSet);}*/
    leftMotor1.setSpeed(170);
    leftMotor2.setSpeed(170);
    rightMotor1.setSpeed(170); 
    rightMotor2.setSpeed(170);
    delay(50);
  }
 }
 moveStop();
}//-------------------------------------------------------------------------------------------------------------------------------------
void moveBackward() {
    motorSet = "BACKWARD";
    leftMotor1.run(BACKWARD);     // turn it on going backward
    leftMotor2.run(BACKWARD);     // turn it on going backward
    rightMotor1.run(BACKWARD);    // turn it on going backward
    rightMotor2.run(BACKWARD);    // turn it on going backward
  for (speedSet = 0; speedSet < MAX_SPEED; speedSet +=2) // slowly bring the speed up to avoid loading down the batteries too quickly
  {
    leftMotor1.setSpeed(speedSet);
    leftMotor2.setSpeed(speedSet);
    rightMotor1.setSpeed(speedSet); 
    rightMotor2.setSpeed(speedSet); 
    delay(5);
  }
}  
//-------------------------------------------------------------------------------------------------------------------------------------
void turnRight() {
  motorSet = "RIGHT";
  leftMotor1.run(FORWARD);      // turn motor 1 forward
  leftMotor2.run(FORWARD);      // turn motor 2 forward
  //rightMotor1.run(BACKWARD);    // turn motor 3 backward
  //rightMotor2.run(BACKWARD);    // turn motor 4 backward
  //rightMotor1.setSpeed(speedSet+MAX_SPEED_OFFSET);      
  //rightMotor2.setSpeed(speedSet+MAX_SPEED_OFFSET);     
  leftMotor1.setSpeed(speedSet);      
  leftMotor2.setSpeed(speedSet);
  delay(1500); // run motors this way for 1500        
  motorSet = "FORWARD";
  moveForward();
}  
//-------------------------------------------------------------------------------------------------------------------------------------
void turnLeft() {
  motorSet = "LEFT";
  //leftMotor1.run(BACKWARD);      // turn motor 1 backward
  //leftMotor2.run(BACKWARD);      // turn motor 2 backward
  //leftMotor1.setSpeed(speedSet+MAX_SPEED_OFFSET);     
  //leftMotor2.setSpeed(speedSet+MAX_SPEED_OFFSET);    
  rightMotor1.run(FORWARD);     // turn motor 3 forward
  rightMotor2.run(FORWARD);     // turn motor 4 forward
  rightMotor1.setSpeed(speedSet);
  rightMotor1.setSpeed(speedSet);
  delay(1500); // run motors this way for 1500  
  motorSet = "FORWARD";
  moveForward();  
}  
//-------------------------------------------------------------------------------------------------------------------------------------
void turnAround() {
  motorSet = "RIGHT";
  leftMotor1.run(FORWARD);      // turn motor 1 forward
  leftMotor2.run(FORWARD);      // turn motor 2 forward
  rightMotor1.run(BACKWARD);    // turn motor 3 backward
  rightMotor2.run(BACKWARD);    // turn motor 4 backward
  /*rightMotor1.setSpeed(speedSet+MAX_SPEED_OFFSET);      
  rightMotor2.setSpeed(speedSet+MAX_SPEED_OFFSET);*/
  rightMotor1.setSpeed(speedSet);      
  rightMotor2.setSpeed(speedSet);
  leftMotor1.setSpeed(speedSet);
  leftMotor1.setSpeed(speedSet);
  delay(1700); // run motors this way for 1700
  motorSet = "FORWARD";
  moveForward();
}  
//----------------------------------------------------LED INTERFACE---------------------------------------------------------------------------
void setcolor(int cred,int cgreen,int cblue)
{
  analogWrite(LED1,cred);
  analogWrite(LED2,cgreen);
  analogWrite(LED3,cblue);
}
void CRED()
{
    setcolor(255,0,0);
}
void CGREEN()
{
     setcolor(0,255,0);
}
void CPURPLE()
{
     setcolor(170,0,255);
} 
void LEDOFF()
{
      setcolor(0,0,0);         
}
//--------------------------------------------------------BUZZER SOUND------------------------------------------------------------------------
/*void buzzer_on()
{
 /* tone(buzzerPin, 100);
  delay(800);
  noTone(buzzerPin);
  //iterate over the notes of the melody
    for (int thisNote=0; thisNote <8; thisNote++){
  //to calculate the note duration, take one second. Divided by the note type
    int noteDuration = 1000 / noteDurations [thisNote];
    tone(buzzerPin, melody [thisNote], noteDuration);
  //to distinguish the notes, set a minimum time between them
  //the note's duration +30% seems to work well
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    //stop the tone playing
     noTone(buzzerPin);
}
}*/
//--------------------------------------------------------SMALL MOVES BY CAR-----------------------------------------------------------------
void forward_car()
{
  CGREEN();
  leftMotor1.run(FORWARD);      // turn it on going forward
  leftMotor2.run(FORWARD);      // turn it on going forward
  rightMotor1.run(FORWARD);     // turn it on going forward
  rightMotor2.run(FORWARD);     // turn it on going forward
  leftMotor1.setSpeed(170);
  leftMotor2.setSpeed(170);
  rightMotor1.setSpeed(170); 
  rightMotor2.setSpeed(170);
  delay(2000);
  leftMotor1.run(RELEASE); 
  leftMotor2.run(RELEASE);
  rightMotor1.run(RELEASE);
  rightMotor2.run(RELEASE);
  LEDOFF();
}
  
void back_car()
{
  CGREEN();
  leftMotor1.run(BACKWARD);      // turn it on going backward
  leftMotor2.run(BACKWARD);      // turn it on going backward
  rightMotor1.run(BACKWARD);     // turn it on going backward
  rightMotor2.run(BACKWARD);    // turn it on going backward
  leftMotor1.setSpeed(170);
  leftMotor2.setSpeed(170);
  rightMotor1.setSpeed(170); 
  rightMotor2.setSpeed(170);
  delay(2000);
  leftMotor1.run(RELEASE); 
  leftMotor2.run(RELEASE);
  rightMotor1.run(RELEASE);
  rightMotor2.run(RELEASE);
  LEDOFF();
}
  
void right_car()
{
  CGREEN();
  myservo.write(0);
  delay(1000);
  myservo.write(90);
  delay(1000);
  leftMotor1.run(FORWARD);      // turn it on going forward
  leftMotor2.run(FORWARD);      // turn it on going forward
  //rightMotor1.run(BACKWARD);     // turn it on going backward 
  //rightMotor2.run(BACKWARD);     // turn it on going backward
  leftMotor1.setSpeed(170);
  leftMotor2.setSpeed(170);
  rightMotor1.setSpeed(170);
  rightMotor2.setSpeed(170);
  delay(1000);
  leftMotor1.run(RELEASE); 
  leftMotor2.run(RELEASE);
  rightMotor1.run(RELEASE);
  rightMotor2.run(RELEASE);
  LEDOFF();
}
  
void left_car()
{
  CGREEN();
  myservo.write(180);
  delay(1000);
  myservo.write(90);
  delay(1000);
 // leftMotor1.run(BACKWARD);      // turn it on going backward
  //leftMotor2.run(BACKWARD);      // turn it on going backward
  rightMotor1.run(FORWARD);     // turn it on going forward 
  rightMotor2.run(FORWARD);     // turn it on going forward
  leftMotor1.setSpeed(170);
  leftMotor2.setSpeed(170);
  rightMotor1.setSpeed(170);
  rightMotor2.setSpeed(170);
  delay(1000);
  //leftMotor1.run(RELEASE); 
  //leftMotor2.run(RELEASE);
  rightMotor1.run(RELEASE);
  rightMotor2.run(RELEASE);
  LEDOFF();
}
void mix()
{
 
 for(int x=1;x<=10;x++)
 {
   //medium blue 
   if(x==1)
   setcolor(0,0,205);         
   //golden 
   if(x==2)
   setcolor(255,215,0);
   //orange 
   if(x==3)
   setcolor(255,165,0);
   //yellow 
   if(x==4)
   setcolor(255,255,0);
   //hot pink 
   if(x==5)
   setcolor(255,105,180);
   //medium voilet red 
   if(x==6)
   setcolor(199,21,133);
   //indigo
   if(x==7)
   setcolor(75,0,130);
   //light green 
   if(x==8)
   setcolor(144,238,144);
   //maroon 
   if(x==9)
   setcolor(128,0,0);
   //magenta
   if(x==10)
   setcolor(255,0,255);
   delay(1000); 
 }  
} 
void sense(){
   while(!Serial.available())
   {
     if(digitalRead(pirPin) == HIGH){
       back_car();
       if(lockLow){ 
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;           
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(millis()/1000);
         Serial.println(" sec");
         delay(50);
         }        
         takeLowTime = true;
       }
 
     if(digitalRead(pirPin) == LOW){      
       moveStop();
 
       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause,
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause){ 
           //makes sure this block of code is only executed again after
           //a new motion sequence has been detected
           lockLow = true;                       
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause)/1000);
           Serial.println(" sec");
           delay(50);
           }
       }
   }
   CRED();
}
