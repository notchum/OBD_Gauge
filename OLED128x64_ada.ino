#include "bitmaps.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <OBD2UART.h>

#define OLED_RESET 4
#define buttonPin 8

typedef enum {AFR, OilTemp, Battery, CoolantTemp, IntakeTemp} modes;

Adafruit_SH1106 display(OLED_RESET); // Create display

COBD obd; // Create obd member

double previousReading[] = {0,0,0,0,0};
double curValue[] = {0,215,0,150,100};
static byte pids[] = {PID_ENGINE_OIL_TEMP, PID_AIR_FUEL_EQUIV_RATIO, PID_HYBRID_BATTERY_PERCENTAGE, 
                      PID_COOLANT_TEMP, PID_INTAKE_TEMP};
modes mode = AFR;
int modeNum = 0;
bool  buttonPressed = false;

// Function declarations
void getVal();
void updateVal();
void drawIcon();
void updateMode();

void setup() 
{
  delay(100);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.display();

  // start communication with OBD-II adapter
  obd.begin(); 

  // init button mode
  pinMode(buttonPin, INPUT_PULLUP);\

  // Draw the first mode
  drawIcon();
  //getVal();
  //updateVal();
}

void loop() 
{
//    display.setCursor(20 , 22);
//    display.print(" 14.7");
//    display.display();     

   //display the value here
   //only print if it changes to avoid flickering
    //getVal();
    if (abs(curValue[mode] - previousReading[mode]) > 0 )
    {
     updateVal();
    }

    // State Machine for debounced button press
    while (digitalRead(buttonPin) == HIGH){
      buttonPressed = true;
    }
    if(buttonPressed){
      // When a button is pressed, change the mode once
      updateMode();
      buttonPressed = false;
    }

    // draws the current mode icon
    drawIcon();

    //getVal();
    updateVal(); //if you don't update here and the value hasn't changed you get blank value
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                  Helper Functions
//
/////////////////////////////////////////////////////////////////////////////////////////

void updateMode(){
  if(modeNum == 4)
    modeNum = 0;
  else
    modeNum++;
  
  switch(mode){
  case AFR:
    mode = OilTemp; 
    break;   
  case OilTemp:
    mode = Battery;
    break;
  case Battery:
    mode = CoolantTemp;
    break;
  case CoolantTemp:
    mode = IntakeTemp;
    break;
  case IntakeTemp:
    mode = AFR;
    break;
  }
}

// Gets the value by reading the PID corresponding to the current mode
void getVal(){
  switch(mode){
  case AFR:
    obd.readPID(PID_AIR_FUEL_EQUIV_RATIO, curValue[mode]); 
    break;   
  case OilTemp:
    obd.readPID(PID_ENGINE_OIL_TEMP, curValue[mode]);
    break;
  case Battery:
    obd.readPID(PID_HYBRID_BATTERY_PERCENTAGE, curValue[mode]);
    break;
  case CoolantTemp:
    obd.readPID(PID_COOLANT_TEMP, curValue[mode]);
    break;
  case IntakeTemp:
    obd.readPID(PID_INTAKE_TEMP, curValue[mode]);
    break;
  }
}

 // 
void updateVal(){
  //draw old value
  display.setTextColor(BLACK);
  display.setCursor(40, 22);
  display.print(previousReading[modeNum]); 
  display.display();
  //draw new value
  display.setTextColor(WHITE);
  display.setCursor(40, 22); 
  if ( (modeNum == 0) || (modeNum == 2) ){ //AFR should show the decimal (others could be added to this list)
    display.print((double)curValue[modeNum]);
  }
  else { //everything else should print an int value
    display.print(curValue[modeNum]);
  }
  display.display();
  previousReading[modeNum] = curValue[modeNum];  
  return;
}

void drawIcon(){
  
  display.clearDisplay();
  
  switch(mode){
  case AFR:
    display.drawBitmap(2, 16, o2_bits, 32, 32, 1);
    display.display();
    break;   
  case OilTemp:
    display.drawBitmap(2, 16, oil_bits, 32, 32, 1);
    display.display();  
    break;
  case Battery:
    display.drawBitmap(2, 16, batt_bits, 32, 32, 1); 
    display.display();
    break;
  case CoolantTemp:
    display.drawBitmap(2, 16, coolant_bits, 32, 32, 1); 
    display.display();
    break;
  case IntakeTemp:
    display.drawBitmap(2, 16, intake_bits, 32, 32, 1); 
    display.display();
    break;
  }
}
 
