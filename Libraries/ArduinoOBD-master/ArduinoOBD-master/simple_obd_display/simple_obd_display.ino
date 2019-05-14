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

static const PROGMEM uint8_t intake_bits []  = {
  0x00, 0x00, 0x00, 0x00, 0x6f, 0x80, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x10, 0x00, 
  0x0f, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x3f, 0xe0, 0x0c, 0x00, 0x3f, 0xe0, 
  0x0c, 0xe0, 0x3f, 0xe0, 0x01, 0x10, 0x38, 0x00, 0x00, 0x10, 0x38, 0x00, 0x00, 0x10, 0x3f, 0xe0, 
  0x7f, 0xf3, 0x3f, 0xe0, 0x7f, 0xe4, 0xbf, 0xe0, 0x00, 0x00, 0xb8, 0x00, 0x1f, 0xff, 0xb8, 0x00, 
  0x1f, 0xff, 0x3f, 0xe0, 0x00, 0x00, 0x3f, 0xe0, 0x7f, 0xfe, 0x3f, 0xe0, 0x7f, 0xff, 0x38, 0x00, 
  0x00, 0x01, 0x38, 0x00, 0x00, 0x09, 0x7c, 0x00, 0x00, 0x06, 0x7c, 0x00, 0x00, 0x00, 0x7c, 0x00, 
  0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                                                                 
};

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

void showData(byte pid, double value)
{
  switch (pid) {
  case PID_RPM:
    lcd.setCursor(64, 0);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt((unsigned int)value % 10000, 4);
    break;
  case PID_SPEED:
    lcd.setCursor(0, 0);
    lcd.setFontSize(FONT_SIZE_XLARGE);
    lcd.printInt((unsigned int)value % 1000, 3);
    break;
  case PID_THROTTLE:
    lcd.setCursor(88, 5);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.printInt((int)value % 100, 3);
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.print(" %");
    break;
  case PID_ENGINE_LOAD:
    lcd.setCursor(12, 5);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.printInt((int)value, 3);
    lcd.setFontSize(FONT_SIZE_SMALL);
    lcd.print(" %");
    break;
  }
}

void initScreen()
{
  lcd.clear();
  lcd.setFontSize(FONT_SIZE_SMALL);
  lcd.setCursor(24, 3);
  lcd.print("km/h");
  lcd.setCursor(110, 3);
  lcd.print("rpm");
  lcd.setCursor(0, 7);
  lcd.print("ENGINE LOAD");
  lcd.setCursor(80, 7);
  lcd.print("THROTTLE");
}

void setup()
{
  lcd.begin();
  
  lcd.draw(intake_bits, 16, 16);
  delay(2000);
  lcd.clear();
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
  static byte pids[]= {PID_RPM, PID_SPEED, PID_ENGINE_LOAD, PID_THROTTLE};
  static byte index = 0;
  byte pid = pids[index];
  double value;
  // send a query to OBD adapter for specified OBD-II pid
  if (obd.readPID(pid, (int&)value)) {
      showData(pid, value);
  }
  index = (index + 1) % sizeof(pids);

  if (obd.errors >= 2) {
      reconnect();
      setup();
  }
}
