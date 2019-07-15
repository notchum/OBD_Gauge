/*************************************************************************
* OBD Data Display
* Works with any Arduino board connected with SH1106 128*64 I2C OLED and
* Freematics OBD-II UART Adapter - https://freematics.com/products
* Author: Morgan Chumbley <github.com/notchum>
*************************************************************************/

#include "bitmaps.h"
#define PID_AIR_FUEL_RATIO 0x34

int buttonV;

LCD_SH1106 lcd;  // Create display
COBD obd;       // Create obd member

void reconnect();
void showData(byte pid, int value);
void initScreen();
void drawIcon(byte pid);
void debounce();

void setup()
{  
  // initialize button 
  pinMode(A0, INPUT_PULLUP);
  
  // Start screen
  lcd.begin();  
  //Serial.begin(9600);
  //Serial.println("Begin");

  // Draw intro screen
  lcd.draw(wywh, 128, 64);
  delay(5000);

  // Clear and set font size
  lcd.clear();
  lcd.setFontSize(FONT_SIZE_MEDIUM);

  // Initialize OBD communitcation
  delay(500);
  obd.begin();

  // Print a connecting screen while waiting for connection
  lcd.println("\nConnecting...");
  while (!obd.init());
  lcd.clear();
}

void loop()
{
  static byte pids[]= {PID_AIR_FUEL_RATIO, PID_BATTERY_VOLTAGE, PID_ENGINE_OIL_TEMP, PID_COOLANT_TEMP, PID_INTAKE_TEMP};
  static byte index = 0;
  byte pid = pids[index];
  int value;
  char data[130];

  buttonV = analogRead(A0);

  // While mode button is not pressed
  while ( (analogRead(A0) <= 200) || (analogRead(A0) >= 270) ){ 
    buttonV = analogRead(A0);
    drawIcon(pid);

    // send a query to OBD adapter for specified OBD-II pid
    if(pid == PID_ENGINE_OIL_TEMP){
      if(obd.sendCommand("2101\r", data, 130)){
        value = ((float)strtol(&data[129],0,16));
        showData(pids[2], value);
      }
    }
    else{
      if (obd.readPID(pid, value)) {
          showData(pid, value);
      }
    }
    if (obd.errors >= 2) {
      reconnect();
      setup();
    }
  }

  // When mode button is pressed
  while ( (analogRead(A0) >= 200) && (analogRead(A0) <= 270) ){ 
  }

  // Increment mode
  lcd.clear();
  if(index == (sizeof(pids)-1))
    index = 0;
  else
    index++;
}

void reconnect()
{
  lcd.clear();
  lcd.setFontSize(FONT_SIZE_MEDIUM);
  lcd.print("Reconnecting");
  //digitalWrite(SD_CS_PIN, LOW);
  for (uint16_t i = 0; !obd.init(); i++) {
    if (i == 5) {
      lcd.clear();
    }
    delay(3000);
  }
}

void showData(byte pid, int value)
{
  switch (pid) {
  case PID_INTAKE_TEMP:
    lcd.setCursor(55, 3);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt(value*1.8+32, 3);
    break;
  case PID_COOLANT_TEMP:
    lcd.setCursor(55, 3);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt(value*1.8+32, 3);
    break;
  case PID_BATTERY_VOLTAGE:
    lcd.setCursor(55, 3);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.print((float)value/10, 2);
    break;
  case PID_ENGINE_OIL_TEMP:
    lcd.setCursor(55, 3);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.print((value-40)*1.8+32, 2);
    break;
  case PID_AIR_FUEL_RATIO:
    lcd.setCursor(55, 3);
    lcd.setFontSize(10);
    lcd.print((float)value*256/32768*14.7, 2);
    break;
  }
}

void drawIcon(byte pid){
  
  lcd.setCursor(2, 2);
  switch (pid) {
  case PID_INTAKE_TEMP:
    lcd.draw(intake_bits, 32, 32);
    break;
  case PID_COOLANT_TEMP:
    lcd.draw(coolant_bits, 32, 32);
    break;
  case PID_BATTERY_VOLTAGE:
    lcd.draw(batt_bits, 32, 32);
    break;
  case PID_ENGINE_OIL_TEMP:
    lcd.draw(oil_bits, 32, 32);
    break;
  case PID_AIR_FUEL_RATIO:
    lcd.draw(o2_bits, 32, 32);
    break;
  }
}
