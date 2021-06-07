/*************************************************************************
* OBD Data Display
* Works with any Arduino board connected with SH1106 128*64 I2C OLED and
* Freematics OBD-II UART Adapter - https://freematics.com/products
* Author: Morgan Chumbley <github.com/notchum>
*************************************************************************/

// LIBRARIES
#include <Wire.h>
#include <OBD2UART.h>
#include <MicroLCD.h>
#include "bitmaps.h"

// CONSTS
#define PID_AIR_FUEL_RATIO 0x34
#define BTN_LOW_VALUE      200
#define BTN_HIGH_VALUE     270
#define BUFFER_SIZE        130
#define N_PIDS             5
//#define TEST  1
//#define DEBUG 1

// GLOBALS
int buttonV    ;  // Create button
LCD_SH1106 lcd ;  // Create display
COBD obd       ;  // Create obd member

// FORWARD DECLARATIONS
static void reconnect( void );
static void showData ( byte pid, int value );
static void drawIcon ( byte pid );

void setup()
{  
   // initialize button 
   pinMode(A0, INPUT_PULLUP);

   // Start screen
   lcd.begin();  
#ifdef DEBUG
   Serial.begin(9600);
   Serial.println("OBD Gauge");
#endif
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
#ifndef TEST
   while (!obd.init());
#endif
   lcd.clear();
} // end setup()


void loop( void )
{
   static byte pids[N_PIDS]= {PID_AIR_FUEL_RATIO, PID_BATTERY_VOLTAGE, PID_ENGINE_OIL_TEMP, PID_COOLANT_TEMP, PID_INTAKE_TEMP};
   static byte index = 0;
   byte pid = pids[index];
   int value;
   char data[BUFFER_SIZE];

   // Get the reading of our mode button
   buttonV = analogRead(A0);

   // While mode button is not pressed (my button is ~227)
   while ( (analogRead(A0) <= BTN_LOW_VALUE) || (analogRead(A0) >= BTN_HIGH_VALUE) )
   { 
      buttonV = analogRead(A0);
      drawIcon(pid);

      // Send a query to OBD adapter for specified OBD-II pid
      if ( PID_ENGINE_OIL_TEMP == pid )
      {
         if ( obd.sendCommand("2101\r", data, BUFFER_SIZE) )
         {
            value = ((float)strtol(&data[BUFFER_SIZE-1], 0, 16));
            showData(pid, value);
         }
      }
      else if ( obd.readPID(pid, value) )
      {
         showData(pid, value);
      }
#ifndef TEST
      // Error checking
      if ( obd.errors >= 2 ) 
      {
         reconnect();
         setup();
      }
#endif
   }

   // When mode button is pressed
   while ( (analogRead(A0) >= BTN_LOW_VALUE) && (analogRead(A0) <= BTN_HIGH_VALUE) );

   // Increment mode
   lcd.clear();
   index = (index + 1) % N_PIDS;
#ifdef DEBUG
   Serial.print("Current mode: ");
   Serial.println(index);
#endif
} // end loop()


static void reconnect( void )
{
   lcd.clear();
   lcd.setFontSize(FONT_SIZE_MEDIUM);
   lcd.print("Reconnecting");
   //digitalWrite(SD_CS_PIN, LOW);
   for ( uint16_t i = 0; !obd.init(); i++ )
   {
      if ( 5 == i ) 
         lcd.clear();
      delay(3000);
   }
} // end reconnect()


static void showData( byte pid, int value )
{
   switch ( pid ) 
   {
      case PID_AIR_FUEL_RATIO:
         lcd.setCursor(55, 3);
         lcd.setFontSize(10);
         lcd.print((float)value*256/32768*14.7, 2);
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
      case PID_COOLANT_TEMP:
         lcd.setCursor(55, 3);
         lcd.setFontSize(FONT_SIZE_XLARGE);
         lcd.printInt(value*1.8+32, 3);
         break;
      case PID_INTAKE_TEMP:
         lcd.setCursor(55, 3);
         lcd.setFontSize(FONT_SIZE_XLARGE);
         lcd.printInt(value*1.8+32, 3);
         break;
      // +++TODO: Add a screen to display all four values below at once
      // +++left here as placeholder
      // lcd.setFontSize(FONT_SIZE_SMALL);
      // lcd.setCursor(24, 3);
      // lcd.print("km/h");
      // lcd.setCursor(110, 3);
      // lcd.print("rpm");
      // lcd.setCursor(0, 7);
      // lcd.print("ENGINE LOAD");
      // lcd.setCursor(80, 7);
      // lcd.print("THROTTLE");
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
} // end showData()


static void drawIcon( byte pid )
{
   lcd.setCursor(2, 2);
   switch ( pid ) 
   {
      case PID_AIR_FUEL_RATIO:
         lcd.draw(o2_bits, 32, 32);
         break;
      case PID_BATTERY_VOLTAGE:
         lcd.draw(batt_bits, 32, 32);
         break;
      case PID_ENGINE_OIL_TEMP:
         lcd.draw(oil_bits, 32, 32);
         break;
      case PID_COOLANT_TEMP:
         lcd.draw(coolant_bits, 32, 32);
         break;
      case PID_INTAKE_TEMP:
         lcd.draw(intake_bits, 32, 32);
         break;
   }
} // end drawIcon()
