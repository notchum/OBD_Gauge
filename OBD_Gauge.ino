/*************************************************************************
* OBD Data Display
* Works with any Arduino board connected with SH1106 128*64 I2C OLED and
* Freematics OBD-II UART Adapter - https://freematics.com/products
* Author: Morgan Chumbley <github.com/notchum>
*************************************************************************/

// LIBRARIES
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <OBD2UART.h>
#include "bitmaps.h"

// SCREEN CONSTS
#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_MOSI     13
#define OLED_CLK      14
#define OLED_DC       2
#define OLED_CS       15
#define OLED_RESET    12

// DISPLAY CONSTS
#define FONT_DEF_W  6
#define FONT_DEF_H  8
#define FONT_9PT_W  10
#define FONT_9PT_H  14
#define FONT_12PT_W 14
#define FONT_12PT_H 19

// OBD CONSTS
#define PID_AIR_FUEL_RATIO 0x34
#define BTN_LOW_VALUE      1000
#define BTN_HIGH_VALUE     1050
#define BUFFER_SIZE        130
#define N_SCREENS          3
#define N_PIDS             11

// OTHER CONSTS
#define FLOAT_TYPE1 0xA
#define FLOAT_TYPE2 0xB
#define INT_TYPE    0xC
#define UINT_TYPE   0xD
#define DEBUG 1

// GLOBALS
int buttonV ;  // Create button
COBD obd    ;  // Create obd object
Adafruit_SSD1306 display( SCREEN_WIDTH  , // Create display
                          SCREEN_HEIGHT ,
                          OLED_MOSI     , 
                          OLED_CLK      , 
                          OLED_DC       , 
                          OLED_RESET    , 
                          OLED_CS       );

// STRUCTS
typedef struct
{
   byte pid;            // OBD PID
   byte type;           // data type
   byte screen;         // screen index for the data
   int  value;          // value read from ECU
   float Fvalue;        // value read (in float)
   unsigned int Uvalue; // value read (in uint)
   GFXfont font;        // font for the data
   int16_t posX;        // x position on screen
   int16_t posY;        // y position on screen
} DataType;

// FORWARD DECLARATIONS
static void reconnect    ( void );
static void showData     ( byte screen_inx, DataType crt_data );
static void drawBgScreen ( byte screen_inx );

void setup( void )
{
#ifdef DEBUG
   Serial.begin(115200);
   Serial.setDebugOutput(true);
   Serial.println("\n\n\n=============\nOBD Gauge\n=============");
#endif

   // Initialize button 
   pinMode(A0, INPUT_PULLUP);

   // Start screen
   if(!display.begin(SSD1306_SWITCHCAPVCC))
   {
      Serial.println(F("SSD1306 allocation failed"));
      while(1); // Don't proceed, loop forever
   }

   // Draw splash screen
   display.drawBitmap(0, 0, wywh, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE, BLACK);
   display.display();
   delay(100); //+++ change to 5000
   
   // Clear and set font size
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE, BLACK);

#ifndef DEBUG
   // Initialize OBD communitcation
   delay(500);
   obd.begin();
   display.println("\n  Connecting...");
   display.display();

   Serial.print("Connecting to OBD");
   while (!obd.init())
   {
      Serial.print(".");
   }
#endif
   display.clearDisplay();
} // end setup()


void loop( void )
{
   static DataType data[4] = { {PID_AIR_FUEL_RATIO,  FLOAT_TYPE2, 0, 0, 0, 0, FreeSansBold12pt7b, 0, 0},
                                    {PID_INTAKE_TEMP,     INT_TYPE   , 0, 0, 0, 0, FreeSansBold9pt7b , 0, 0},
                                    {PID_ENGINE_OIL_TEMP, INT_TYPE   , 0, 0, 0, 0, FreeSansBold9pt7b , 0, 0},
                                    {PID_BATTERY_VOLTAGE, FLOAT_TYPE1, 0, 0, 0, 1, FreeSansBold12pt7b, 0, 0},
                                    /*{PID_COOLANT_TEMP,    INT_TYPE   , 0, 0, 0, 1, FreeSansBold9pt7b , 0, 0},
                                    {PID_FUEL_PRESSURE,   INT_TYPE   , 0, 0, 0, 1, FreeSansBold9pt7b , 0, 0},
                                    {PID_RPM,             UINT_TYPE  , 0, 0, 0, 2, FreeSansBold9pt7b , 0, 0}, 
                                    {PID_SPEED,           UINT_TYPE  , 0, 0, 0, 2, FreeSansBold9pt7b , 0, 0},
                                    {PID_ENGINE_LOAD,     INT_TYPE   , 0, 0, 0, 2, FreeSansBold9pt7b , 0, 0},
                                    {PID_THROTTLE,        INT_TYPE   , 0, 0, 0, 2, FreeSansBold9pt7b , 0, 0},
                                    {PID_MAF_FLOW,        FLOAT_TYPE1, 0, 0, 0, 2, FreeSansBold12pt7b, 0, 0} */};
   static byte pid_inx = 0, screen_inx = 0;
   drawBgScreen(screen_inx);
//   char data[BUFFER_SIZE];

   // While mode button is not pressed (my button is ~227)
   while ( (analogRead(A0) <= BTN_LOW_VALUE) || (analogRead(A0) >= BTN_HIGH_VALUE) )
   {
      // Send a query to OBD adapter for specified OBD-II pid
      /*if ( PID_ENGINE_OIL_TEMP == data[pid_inx].pid )
      {
         if ( obd.sendCommand("0121\r", data, BUFFER_SIZE) )
         {
            data[pid_inx].value = ((float)strtol(&data[BUFFER_SIZE-1], 0, 16));
            showData(screen_inx, data[pid_inx]);
         }
      }
      else */if ( 1/*obd.readPID(data[pid_inx].pid, data[pid_inx].value)*/ )
      {
         if ( PID_MAF_FLOW == data[pid_inx].pid )
         {
            data[pid_inx].Fvalue = (float(data[7/*speed*/].value)*7.718)/float(data[pid_inx].value);
         }
         showData(screen_inx, data[pid_inx]);
      }

      pid_inx = (pid_inx + 1) % N_PIDS;
#ifndef DEBUG
      // Error checking
      if ( obd.errors >= 2 ) 
      {
         reconnect();
         setup();
      }
#endif
      delay(100);
   }

   // When mode button is pressed
   while ( (analogRead(A0) >= BTN_LOW_VALUE) && (analogRead(A0) <= BTN_HIGH_VALUE) )
   {
      delay(100);
   }

   // Increment mode
   display.clearDisplay();
   screen_inx = (screen_inx + 1) % N_SCREENS;
#ifdef DEBUG
   Serial.print("Current screen: ");
   Serial.println(screen_inx);
#endif
} // end loop()


static void reconnect( void )
{
   display.clearDisplay();
   display.setTextSize(1);
   display.println("\n  Reconnecting...");
   display.display();
   //digitalWrite(SD_CS_PIN, LOW);
   for ( uint16_t i = 0; !obd.init(); i++ )
   {
      if ( 5 == i ) 
         display.clearDisplay();
      delay(3000);
   }
} // end reconnect()


static void showData( byte screen_inx, DataType crt_data )
{
   // Calculate the value of the raw data, x pos, and y pos
   switch ( crt_data.pid ) 
   {
      case PID_AIR_FUEL_RATIO:
         crt_data.Fvalue = (float)crt_data.value*256/32768*14.7;
         crt_data.posX = 33;
         crt_data.posY = (FONT_12PT_H / 2) + 16;
         break;
      case PID_INTAKE_TEMP:
         crt_data.value = crt_data.value*1.8+32;
         crt_data.posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(crt_data.value)) - 1;
         crt_data.posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
         break;
      case PID_ENGINE_OIL_TEMP:
         crt_data.value = crt_data.value*1.8+32;
         crt_data.posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(crt_data.value)) - 1;
         crt_data.posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
         break;
      case PID_BATTERY_VOLTAGE:
         crt_data.Fvalue = (float)crt_data.value/10;
         crt_data.posX = 33;
         crt_data.posY = (FONT_12PT_H / 2) + 16;
         break;
      case PID_COOLANT_TEMP:
         crt_data.value = crt_data.value*1.8+32;
         crt_data.posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(crt_data.value)) - 1;
         crt_data.posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
         break;
      case PID_FUEL_PRESSURE:
         // crt_data.value = 
         crt_data.posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(crt_data.value)) - 1;
         crt_data.posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
         break;
      case PID_RPM:
         crt_data.Uvalue = (unsigned int)crt_data.value % 10000;
         crt_data.posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(crt_data.Uvalue)) - 1;
         crt_data.posY = SCREEN_HEIGHT - 1;
         break;
      case PID_SPEED:
         crt_data.Uvalue = (unsigned int)crt_data.value % 1000;
         crt_data.posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(crt_data.Uvalue)) - 1;
         crt_data.posY = (SCREEN_HEIGHT / 2) + FONT_9PT_H;
         break;
      case PID_ENGINE_LOAD:
         crt_data.posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(crt_data.value)) - 1;
         crt_data.posY = SCREEN_HEIGHT - 1;
         break;
      case PID_THROTTLE:
         crt_data.value = crt_data.value % 100;
         crt_data.posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(crt_data.value)) - 1;
         crt_data.posY = (SCREEN_HEIGHT / 2) + FONT_9PT_H;
         break;
      case PID_MAF_FLOW:
         crt_data.posX = (SCREEN_WIDTH / 2) - (FONT_12PT_W * numDigits(crt_data.Fvalue)) - 1;
         crt_data.posY = SCREEN_HEIGHT / 2 - 3;
         break;
      default:
         // average mpg
         crt_data.posX = (FONT_12PT_W * numDigits(crt_data.Fvalue)) - 1;
         crt_data.posY = SCREEN_HEIGHT / 2 - 3;
         break;
   }

   // Draw the data if screen indices match
   if ( crt_data.screen == screen_inx )
   {
      display.clearDisplay();
      display.setFont(&crt_data.font);
      display.setCursor(crt_data.posX, crt_data.posY);
      switch ( crt_data.type )
      {
         case FLOAT_TYPE1:
            display.print(crt_data.Fvalue, 1);
            break;
         case FLOAT_TYPE2:
            display.print(crt_data.Fvalue, 2);
            break;
         case INT_TYPE:
            display.print(crt_data.value);
            break;
         case UINT_TYPE:
            display.print(crt_data.Uvalue);
            break;
      }
      display.display();
   }
} // end showData()


static void drawBgScreen( byte screen_inx )
{
   switch ( screen_inx ) 
   {
      case 0: // First screen
         display.drawBitmap(0, -1, o2_bits, 32, 32, WHITE, BLACK);
         display.drawBitmap(0, (SCREEN_HEIGHT / 2) + 1, intake_bits, 32, 32, WHITE, BLACK);
         display.drawBitmap((SCREEN_WIDTH / 2) + 1, SCREEN_HEIGHT / 2, oil_bits, 32, 32, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 32, 14, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 25, 4, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 15, 22, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 8, 12, heart_bits, 8, 7, WHITE, BLACK);
         display.drawFastHLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, WHITE);
         display.drawFastVLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_HEIGHT / 2, WHITE);
         break;
      case 1: // Second screen
         display.drawBitmap(0, -1, batt_bits, 32, 32, WHITE, BLACK);
         display.drawBitmap(0, (SCREEN_HEIGHT / 2) + 1, coolant_bits, 32, 32, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 32, 14, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 25, 4, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 15, 22, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 8, 12, heart_bits, 8, 7, WHITE, BLACK);
         display.setCursor(SCREEN_WIDTH / 2 + 2, (SCREEN_HEIGHT / 2) + FONT_DEF_H + 1);
         display.println("FUEL");
         display.setCursor(SCREEN_WIDTH / 2 + 2, (SCREEN_HEIGHT / 2) + (FONT_DEF_H * 2) + 1);
         display.println("PRESS");
         display.drawFastHLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, WHITE);
         display.drawFastVLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_HEIGHT / 2, WHITE);
         break;
      case 2: // Third screen
         display.drawFastHLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, WHITE);
         display.drawFastVLine(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_HEIGHT / 2, WHITE);
         display.setCursor(0, 0);
         display.println("Inst mpg");
         display.setCursor(SCREEN_WIDTH / 2, 0);
         display.println("Avg mpg");
         display.setCursor(0, (SCREEN_HEIGHT / 2) + FONT_DEF_H + 1);
         display.println("THR %");
         display.setCursor(0, SCREEN_HEIGHT - FONT_DEF_H);
         display.println("RPM");
         display.setCursor(SCREEN_WIDTH / 2 + 2, (SCREEN_HEIGHT / 2) + FONT_DEF_H + 1);
         display.println("MPH");
         display.setCursor(SCREEN_WIDTH / 2 + 2, SCREEN_HEIGHT - FONT_DEF_H);
         display.println("LOAD");
         display.drawBitmap(SCREEN_WIDTH / 2 - 14, 1, heart_bits, 8, 7, WHITE, BLACK);
         display.drawBitmap(SCREEN_WIDTH - 16, 1, oval_bits, 16, 7, WHITE, BLACK);
         break;
   }
   display.display();
} // end drawBgScreen()


/*************************
 *    UTILITIES
 ************************/

static int numDigits( int num )
{
   int num_digits = 0;
   while ( num != 0 )
   {
      num /= 10;
      num_digits++;
   }
   return num_digits;
} // end numDigits()


static int numDigits( unsigned int num )
{
   int num_digits = 0;
   while ( num != 0 )
   {
      num /= 10;
      num_digits++;
   }
   return num_digits;
} // end numDigits()


static int numDigits( float num )
{
   num *= 10;
   int num_digits = 0;
   while ( num != 0 )
   {
      num /= 10;
      num_digits++;
   }
   return num_digits;
} // end numDigits()
