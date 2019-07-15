/*************************************************************************
* Simple OBD Data Display
* Works with any Arduino board connected with SH1106 128*64 I2C OLED and
* Freematics OBD-II UART Adapter - https://freematics.com/products
* Distributed under public domain
* Written by Stanley Huang <stanley@freematics.com.au>
*************************************************************************/

#include <Wire.h>
#include <OBD2UART.h>
#include <MicroLCD.h>

LCD_SH1106 lcd;
COBD obd;

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
  case PID_RPM:
    lcd.setCursor(64, 0);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt((unsigned int)value % 10000, 4);
    break;
  case PID_BATTERY_VOLTAGE:
    lcd.setCursor(0, 0);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt((unsigned int)value % 1000, 3);
    break;
  case PID_INTAKE_TEMP:
    lcd.setCursor(88, 5);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.printInt(value % 100, 3);
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.print(" F");
    break;
  case PID_AIR_FUEL_EQUIV_RATIO:
    lcd.setCursor(12, 5);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.printInt((unsigned int)value % 1000, 2);
    break;
  }
}

void initScreen()
{
  lcd.clear();
  lcd.setFontSize(FONT_SIZE_SMALL);
  lcd.setCursor(24, 3);
  lcd.print("VOLTS");
  lcd.setCursor(110, 3);
  lcd.print("RPM");
  lcd.setCursor(10, 7);
  lcd.print("AFR");
  lcd.setCursor(70, 7);
  lcd.print("IN. TEMP");
}

void setup()
{
  lcd.begin();
  lcd.setFontSize(FONT_SIZE_MEDIUM);
  lcd.println("OBD DISPLAY");

  delay(500);
  obd.begin();

  lcd.println();
  lcd.println("Connecting...");
  while (!obd.init());
  initScreen();
}

void loop()
{
  static byte pids[]= {PID_RPM, PID_BATTERY_VOLTAGE, PID_AIR_FUEL_EQUIV_RATIO, PID_INTAKE_TEMP};
  static byte index = 0;
  byte pid = pids[index];
  int value;
  // send a query to OBD adapter for specified OBD-II pid
  if (obd.readPID(pid, value)) {
      showData(pid, value);
  }
  index = (index + 1) % sizeof(pids);

  if (obd.errors >= 2) {
      reconnect();
      setup();
  }
}
