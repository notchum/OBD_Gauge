// original sketch with 3 buttons here: https://github.com/stirobot/arduinoModularTFTgauges/blob/master/oledOBDgaugesSmallIrvinedLib/oledOBDgaugesSmallIrvinedLib.ino
#include "bitmaps.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <ELM327.h>

#define ELM_TIMEOUT 9000
#define ELM_BAUD_RATE 9600
#define ELM_PORT Serial

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

int buttonV;

Elm327 Elm;
byte Status;

int warnLevels[] = {100, 220, 15, 220, 300};
int warnSign[] = {1,1,1,1,1};  //1 for high, 0 for low (in cases like oil pressure)
float peaks[] = {0,0,0,0,0};
float curValue[] = {0,0,0,0,0};
float previousReading[] = {0,0,0,0,0};
int mode = 0;
int modes = 4;  //0 is the first of the array


void setup() {
  pinMode(A0, INPUT_PULLUP);
  Status=Elm.begin();

  //3 buttons connected as such: http://tronixstuff.com/2011/01/11/tutorial-using-analog-input-for-multiple-buttons/
  //use 15 Kohm resistors
  //no buttons is 1010
  //button nearest gnd is 300
  //middle button is 454
  //last button is 555
  //Chum: I just have one button, it is about 227

  //Serial.begin(9600);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  display.clearDisplay();
  display.display();

  //show flash screen
  display.drawBitmap(0, 0, wywh, 128, 64, 1);
  display.display();
  delay(2500);

  getOBDIIvalue("obdvolts"); //do the first reading here because it takes a sec or two  display.fillRect(8, 21, 2, 2, BLACK);
  delay(2500);

  //put first mode icon and unit here
  display.clearDisplay();
  display.drawBitmap(2, 16, batt_bits, 32, 32, 1);
  display.display();
}

void loop() {

  buttonV = analogRead(A0);

  while ( (analogRead(A0) <= 200) || (analogRead(A0) >= 270) ){ //not mode button (not between 430 and 490)
    buttonV = analogRead(A0);
    if ( (buttonV >= 500) && (buttonV <= 600) ){ //hold down the peaks button to show the peaks of this mode (555ish)
      while ( (buttonV >= 500) && (buttonV <= 600) ){ //debounce
        //display peaks for this "mode" here
        display.fillRect(48,0,80,32,BLACK); //black area between icon and unit
        display.setTextSize(3);
        display.setTextColor(WHITE);
        display.setCursor(48,10);
        display.println(peaks[mode]);
        display.display();
        buttonV = analogRead(A0);
        delay(50);
      } 

      display.fillRect(48,0,80,32,BLACK); //black area between icon and unit
      getVal();
      updateVal(); //just incase it doesn't draw below because the values stay the same
      display.display();
      buttonV = analogRead(A0);
    }

    //hold reset button here to reset peak of specific mode
    if ( (buttonV <= 325) && (buttonV >= 285) ){ //hold down the reset button to reseet the peaks of this mode (300ish)
      while ( (buttonV <= 325) && (buttonV >= 285) ){ //debounce
        display.fillRect(48,0,80,32,BLACK); //black area between icon and unit
        display.setTextSize(3);
        display.setCursor(48,10);
        display.setTextColor(WHITE);
        display.println("RST");
        display.display();
        buttonV = analogRead(A0);
        delay(50);
      } 

      //reset peak specific to this mode
      display.fillRect(48,0,80,32,BLACK); //black area between icon and unit
      getVal();
      updateVal(); //just incase it doesn't draw below because the values stay the same
      display.display();
      peaks[mode] = 0;
      buttonV = analogRead(A0);
    }

    //display the value here...no conditionals...just print the value
    //only print if it changes to avoid flickering
    getVal();

    if ( abs(curValue[mode]-previousReading[mode]) > 0 ){
      updateVal();
    }

    //TODO: fix this...not working
    //check for warning values here (for this mode only)
    //if ( ( (curValue[mode] > warnLevels[mode]) && (warnSign[mode] == 1) ) || ( (curValue[mode] < warnLevels[mode]) && (warnSign[mode] == 0) ) ){

    if (curValue[mode] >= warnLevels[mode]){
      //do a warning thing here
      display.println("WARN");
      display.display();
      warn();
    }
  }

  while ( (analogRead(A0) >= 200) && (analogRead(A0) <= 270) ){ //mode button
  }

  if (mode == modes){
    mode = 0;
  }
  else {
    mode++;
  }

  //upon switching modes blank screen and display that mode's icon
  //also print the unit of measurement if used
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);

  if (mode == 0){//oil temp
    display.drawBitmap(2, 16, batt_bits, 32, 32, 1);
    display.display();
  }

  if (mode == 1){//AFR
    display.drawBitmap(2, 16, o2_bits, 32, 32, 1);
    display.display();
  }

  if (mode == 2){//Volts
    display.drawBitmap(2, 16, oil_bits, 32, 32, 1);
    display.display();
  }

  if (mode == 3){//Coolant
    display.drawBitmap(2, 16, coolant_bits, 32, 32, 1);
    display.display();
  }

  if (mode == 4){
    display.drawBitmap(2, 16, intake_bits, 32, 32, 1);
    display.display();
  }

  getVal();
  updateVal(); //if you don't update here and the value hasn't changed you get blank value
}

void warn(){
  //a border around the screen flashes as a warning
  for (int a=1;a>=3;a++){
    display.drawRect(0,0,127,63,WHITE);
    delay(100);
    display.drawRect(0,0,127,63,BLACK);
  }
  return;
} 

void getVal(){
  if (mode == 0){
    curValue[mode] = getOBDIIvalue("obdvolts");//modeList[mode]);
  }

  else if (mode == 1){
    curValue[mode] = getOBDIIvalue("obdafr");
  }

  else if (mode == 2){
    curValue[mode] = getOBDIIvalue("obdbrzoiltempf");
  }

  else if (mode == 3){
    curValue[mode] = getOBDIIvalue("obdcoolantf");
  }

  else if (mode == 4){
    curValue[mode] = getOBDIIvalue("obdiat");
  }

  if (curValue[mode] > peaks[mode]){
    peaks[mode] = curValue[mode];
  }
}

void updateVal(){
  display.setTextSize(3);

  //draw old value
  display.setTextColor(BLACK);
  display.setCursor(50,16);

  if ( (mode == 0) || (mode == 1) ){ //AFR should show the decimal (others could be added to this list)
    display.println(previousReading[mode]);
  }

  else { //everything else should print an int value
    display.println((int)previousReading[mode]);
  }

  display.display();

  //draw new value
  display.setTextColor(WHITE);
  display.setCursor(50,16);

  if ( (mode == 0) || (mode == 1) ){ //AFR should show the decimal (others could be added to this list)
    display.println(curValue[mode]);
  }

  else { //everything else should print an int value
    display.println((int)curValue[mode]);
  }

  display.display();
  previousReading[mode] = curValue[mode]; 
  //delay(100); 
  return;
}

float getOBDIIvalue(String whichSensor){

  //Serial.flush();
  float value = 0;
  char data[130];

  if (whichSensor.indexOf("obdcoolantf") >=0){
    //Elm.coolantTemperature(value);
    value=value*1.8+32;
  }
  
  if (whichSensor.indexOf("obdafr") >= 0){
    //Status = Elm.runCommand("0134",data,20);
    value = ((float)(strtol(&data[6],0,16)*256)+strtol(&data[9],0,16))/32768*14.7;  //(A*256+B)/32768*14.7
  }

  if (whichSensor.indexOf("obdiat") >= 0){ //USED
    //Elm.intakeAirTemperature(value);
    value=value*1.8+32;
  }

  if (whichSensor.indexOf("obdvolts") >= 0){ //USED
    Elm.getVoltage(value);
  }

  if (whichSensor.indexOf("obdbrzoiltempf") >= 0){ //works and USED
    //Status = Elm.runCommand("2101",data,130);
    value = ((float)strtol(&data[109],0,16) - 40) * 1.8 + 32;
  }

  //delay(100);  
  return value; 
}
