// ENCODER CONTROLLER FILE
//  -ROBOCLAW LAU LAB PROJECT

// AUTHORS: Matt Ruffner and Mitch Mullins

// PURPOSE:
//  -To control the Roboclaw Motcor Encoders via an Ion MC Motor Controller

// NOTES:
//  - NONE as of 9/11/17


//See BareMinimum example for a list of library functions

//Includes required to use Roboclaw library
#include <SoftwareSerial.h>
#include <RoboClaw.h>
#include <stdint.h>

//See limitations of Arduino SoftwareSerial
SoftwareSerial serial(10,11);  // These are the Pin Outs on the Arduino for RX and TX
RoboClaw roboclaw(&serial,10000); //Default Roboclaw  setup

#define address 0x80 //Default Roboclaw setup

//Velocity PID coefficients
#define Kp 1.0
#define Ki 0.5
#define Kd 0.25
#define qpps 2000

//FUNCTION: Setup

//PURPOSE: sets up the Arduino for communication to the Roboclaw

void setup() {
  
  //Open Serial and roboclaw serial ports
  Serial.begin(57600); //Serial monitor Communication Rate
  roboclaw.begin(38400); //Baudrate of the Roboclaw
  
  //Set PID Coefficients
  roboclaw.SetM1VelocityPID(address,Kd,Kp,Ki,qpps); //Setting the Velocity PID Coefficients
  roboclaw.SetM2VelocityPID(address,Kd,Kp,Ki,qpps);  
}

uint8_t status1, status2; //Encoder Status Variables
bool valid1, valid2; //Variable for allowing to read encoder
String command; //Command Input variable

//FUNCTION: Main Loop

//PURPOSE: this is the main operating loop for the Roboclaw

void loop() {
  
  while( Serial.available() == 0 ); //Sets loop to run indefinitely

  char c = Serial.read();
  
  uint32_t pulses = Serial.parseInt(); //Fetches input from the Serial Monitor

  //Resetting the Encoders and reading their Status
  roboclaw.ResetEncoders(address);
  int32_t enc1 = roboclaw.ReadEncM1(address, &status1, &valid1);
  int32_t enc2 = roboclaw.ReadEncM2(address, &status2, &valid2);

  //Displays the encoder Values
  Serial.print("intial encoder values:");
  Serial.print(enc1);
  Serial.print(" , ");
  Serial.println(enc2);

  //DECLARATIONS:

  // RoboClaw::ForwardM1(uint8_t address, uint8_t speed) 
  //  NOTE: the address is the same no matter what, encode as "address"
  //        the speed a unsigned 8 bit value. Same for M2.
  //        - "127" is  full speed forward
  //        - "0" is full stop

  
  // RoboClaw::BackwardM1(uint8_t address, uint8_t speed) 
  //  NOTE: the address is the same no matter what, encode as "address"
  //        the speed a unsigned 8 bit value. Same for M2.
  //        - "127" is  full speed forward
  //        - "0" is full stop


  if(c == 'f'){
    Serial.print("Direction Set: Forward ");
    Serial.print(pulses);
    Serial.print(" meters");
    Serial.println();

    pulses = pulses*3341;
  
    //Setting the Robot to move Forward
    roboclaw.ForwardM1(address, 127);
    roboclaw.ForwardM2(address, 127);
  
    do {
      enc1 = roboclaw.ReadEncM1(address, &status1, &valid1);
      enc2 = roboclaw.ReadEncM2(address, &status2, &valid2);
      Serial.print("updated encoder values:");
      Serial.print(enc1);
      Serial.print(" , ");
      Serial.println(enc2);
    } while( abs(enc1) < pulses && abs(enc2) < pulses ); 

    //STOP Command
    roboclaw.ForwardM1(address, 0);
    roboclaw.ForwardM2(address, 0);
    
  }
  else if(c == 'b'){
    Serial.print("Direction Set: Backwards ");
    Serial.print(pulses);
    Serial.print(" meters");
    Serial.println();

    pulses = pulses*3341;

    //Setting the Robot to move Forward
    roboclaw.BackwardM1(address, 127);
    roboclaw.BackwardM2(address, 127);
  
    do {
      enc1 = roboclaw.ReadEncM1(address, &status1, &valid1);
      enc2 = roboclaw.ReadEncM2(address, &status2, &valid2);
      Serial.print("updated encoder values:");
      Serial.print(enc1);
      Serial.print(" , ");
      Serial.println(enc2);
    } while( abs(enc1) < pulses && abs(enc2) < pulses ); 

    //STOP Command
    roboclaw.ForwardM1(address, 0);
    roboclaw.ForwardM2(address, 0);
    
  }
  else if(c == 'r'){
    Serial.print("Right Turn Set: ");
    Serial.print(pulses);
    Serial.print(" degrees");
    Serial.println();

    pulses = pulses/5;
    pulses = pulses*161;

    //Setting the Robot to move Forward
    roboclaw.BackwardM1(address, 127);
    roboclaw.ForwardM2(address, 127);
  
    do {
      enc1 = roboclaw.ReadEncM1(address, &status1, &valid1);
      enc2 = roboclaw.ReadEncM2(address, &status2, &valid2);
      Serial.print("updated encoder values:");
      Serial.print(enc1);
      Serial.print(" , ");
      Serial.println(enc2);
    } while( abs(enc1) < pulses && abs(enc2) < pulses ); 

    //STOP Command
    roboclaw.ForwardM1(address, 0);
    roboclaw.ForwardM2(address, 0);
    
  }
  else if(c == 'l'){
    Serial.print("Left Turn Set: ");
    Serial.print(pulses);
    Serial.print(" degrees");
    Serial.println();

    pulses = pulses/5;
    pulses = pulses*161;

    //Setting the Robot to move Forward
    roboclaw.ForwardM1(address, 127);
    roboclaw.BackwardM2(address, 127);
  
    do {
      enc1 = roboclaw.ReadEncM1(address, &status1, &valid1);
      enc2 = roboclaw.ReadEncM2(address, &status2, &valid2);
      Serial.print("updated encoder values:");
      Serial.print(enc1);
      Serial.print(" , ");
      Serial.println(enc2);
    } while( abs(enc1) < pulses && abs(enc2) < pulses ); 

    //STOP Command
    roboclaw.ForwardM1(address, 0);
    roboclaw.ForwardM2(address, 0);
    
  }
  else if(c == 's'){
    Serial.println();
    Serial.print(" !!!!! EMERGENCY STOP ACTIVATED !!!!! ");
    Serial.println();
    
    //STOP Command
    roboclaw.ForwardM1(address, 0);
    roboclaw.ForwardM2(address, 0);
    
  } 
  else{
    Serial.println();
    Serial.print("INVALID INPUT");
    Serial.println();
  }


  //STOP Command
  roboclaw.ForwardM1(address, 0);
  roboclaw.ForwardM2(address, 0);

}
