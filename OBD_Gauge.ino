/*************************************************************************
* OBD Data Display
* Works with NodeMCU board connected with SSD1306 128*64 SPI OLED and
* Freematics OBD-II UART Adapter.
* Author: Morgan Chumbley <github.com/notchum>
*************************************************************************/
// #define _DEBUG_

// LIBRARIES
#include <SPI.h>
#include <Wire.h>
#include <TaskScheduler.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <OBD2UART.h>
#include "bitmaps.h"

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#endif

// DISPLAY CONSTS
#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_MOSI     13
#define OLED_CLK      14
#define OLED_DC       2
#define OLED_CS       15
#define OLED_RESET    12

// FONT CONSTS
#define FONT_DEF_W  6
#define FONT_DEF_H  8
#define FONT_9PT_W  10
#define FONT_9PT_H  14
#define FONT_12PT_W 14
#define FONT_12PT_H 17

// OBD CONSTS
#define PID_AIR_FUEL_RATIO 0x34
#define BUFFER_SIZE        130
#define N_PIDS             11
#define N_SCREENS          3

// OTHER CONSTS
#define INT_TYPE   0xA
#define FLOAT_TYPE 0xB
#define UINT_TYPE  0xC

// GLOBALS
Scheduler runner   ; // Create scheduler object
COBD obd           ; // Create obd object
byte pid_inx = 0   ; // PID index
byte screen_inx = 0; // Current screen
Adafruit_SSD1306 display( SCREEN_WIDTH  , // Create display
                          SCREEN_HEIGHT ,
                          OLED_MOSI     , 
                          OLED_CLK      , 
                          OLED_DC       , 
                          OLED_RESET    , 
                          OLED_CS       );
const int BUTTON_PIN = 5         ; // Button pin
volatile bool btnPinHandled = true;

// STRUCTS
typedef struct
{
   byte pid;            // OBD PID
   byte type;           // data type
   byte screen;         // screen index for the data
   int raw_data;        // value read from ECU
   int Ivalue;          // processed data (in int)
   float Fvalue;        // processed data (in float)
   unsigned int Uvalue; // processed data (in uint)
} DataType;

DataType data[N_PIDS] = { {PID_AIR_FUEL_RATIO,  FLOAT_TYPE, 0, 0, 0, 0, 0},
                          {PID_INTAKE_TEMP,     INT_TYPE  , 0, 0, 0, 0, 0},
                          {PID_ENGINE_OIL_TEMP, INT_TYPE  , 0, 0, 0, 0, 0},
                          {PID_BATTERY_VOLTAGE, FLOAT_TYPE, 1, 0, 0, 0, 0},
                          {PID_COOLANT_TEMP,    INT_TYPE  , 1, 0, 0, 0, 0},
                          {PID_FUEL_PRESSURE,   INT_TYPE  , 1, 0, 0, 0, 0},
                          {PID_RPM,             UINT_TYPE , 2, 0, 0, 0, 0}, 
                          {PID_SPEED,           UINT_TYPE , 2, 0, 0, 0, 0},
                          {PID_ENGINE_LOAD,     INT_TYPE  , 2, 0, 0, 0, 0},
                          {PID_THROTTLE,        INT_TYPE  , 2, 0, 0, 0, 0},
                          {PID_MAF_FLOW,        FLOAT_TYPE, 2, 0, 0, 0, 0} };

// CALLBACK PROTOTYPES
void readObdDataCB ( void );
void drawScreenCB  ( void );
void btnHandleCB   ( void );

// TASK CREATIONS
Task tReadDataOBD ( 10 , TASK_FOREVER, &readObdDataCB );
Task tDrawScreen  ( 60 , TASK_FOREVER, &drawScreenCB  );
Task tBtnHandle   ( 500, TASK_ONCE   , &btnHandleCB   );


/*************************
 *    FUNCTIONS
 ************************/


void ICACHE_RAM_ATTR buttonPressed( void )
{
   if ( btnPinHandled )
   {
      detachInterrupt(BUTTON_PIN);
      tBtnHandle.restartDelayed();
      btnPinHandled = false;
   }
} // end buttonPressed()


void btnHandleCB( void )
{
   _PP(millis()); _PP(": btnHandleCB");

   // Increment mode
   screen_inx = (screen_inx + 1) % N_SCREENS;
   _PP(" | Screen: "); _PL(screen_inx + 1);

   attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, RISING);
   btnPinHandled = true;
} // end btnHandleCB()


void readObdDataCB( void )
{
   _PP(millis()); _PL(": readObdDataCB");
   
   /*char data[BUFFER_SIZE];
   // Send a query to OBD adapter for specified OBD-II pid
   if ( PID_ENGINE_OIL_TEMP == data[pid_inx].pid )
   {
      if ( obd.sendCommand("0121\r", data, BUFFER_SIZE) )
      {
         data[pid_inx].value = ((float)strtol(&data[BUFFER_SIZE-1], 0, 16));
         showData(screen_inx, data[pid_inx]);
      }
   }
   else */if ( obd.readPID(data[pid_inx].pid, data[pid_inx].raw_data) )
   {
#ifdef _DEBUG_
      data[pid_inx].raw_data = random(10, 299);
#endif
      if ( PID_MAF_FLOW == data[pid_inx].pid )
      {
         data[pid_inx].Fvalue = (float(data[7/*speed*/].raw_data)*7.718)/float(data[pid_inx].raw_data);
      }
   }

   processData();
   pid_inx = (pid_inx + 1) % N_PIDS;

#ifndef _DEBUG_
   // Error checking
   if ( obd.errors >= 2 ) 
   {
      reconnect();
      setup();
   }
#endif
} // end readObdDataCB()


void drawScreenCB( void )
{
   _PP(millis()); _PL(": drawScreenCB");
   // Clear and draw the background and data
   display.clearDisplay();
   drawBgScreen();
   showData();
   display.display();
} // end drawScreenCB()


/*************************
 *    DISPLAY FUNCTIONS
 ************************/


void reconnect( void )
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


void processData( void )
{
   // Calculate value from raw data
   switch ( data[pid_inx].pid )
   {
      case PID_AIR_FUEL_RATIO:
         data[pid_inx].Fvalue = (float)data[pid_inx].raw_data*256/32768*14.7;
         break;
      case PID_INTAKE_TEMP:
         data[pid_inx].Ivalue = data[pid_inx].raw_data*1.8+32;
         break;
      case PID_ENGINE_OIL_TEMP:
         data[pid_inx].Ivalue = data[pid_inx].raw_data*1.8+32;
         break;
      case PID_BATTERY_VOLTAGE:
         data[pid_inx].Fvalue = (float)data[pid_inx].raw_data/10;
         break;
      case PID_COOLANT_TEMP:
         data[pid_inx].Ivalue = data[pid_inx].raw_data*1.8+32;
         break;
      case PID_FUEL_PRESSURE:
         data[pid_inx].Ivalue = data[pid_inx].raw_data;
         break;
      case PID_RPM:
         data[pid_inx].Uvalue = (unsigned int)data[pid_inx].raw_data % 10000;
         break;
      case PID_SPEED:
         data[pid_inx].Uvalue = (unsigned int)data[pid_inx].raw_data % 1000;
         break;
      case PID_ENGINE_LOAD:
         data[pid_inx].Ivalue = data[pid_inx].raw_data;
         break;
      case PID_THROTTLE:
         data[pid_inx].Ivalue = data[pid_inx].raw_data % 100;
         break;
      case PID_MAF_FLOW:
         break;
      default:
         // average mpg
         data[pid_inx].Fvalue = (float)data[pid_inx].raw_data;
         break;
   }
} // end processData()


void showData( void )
{
   uint16_t posX = 0, posY = 0;

   // Calculate the x pos and y pos (if the PID is currently on-screen)
   for ( int i = 0; i < N_PIDS; i++ )
   {
      if ( data[i].screen == screen_inx )
      {
         switch ( data[i].pid )
         {
            case PID_AIR_FUEL_RATIO:
               posX = 33;
               posY = (FONT_12PT_H / 2) + 10;
               break;
            case PID_INTAKE_TEMP:
               posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(data[i].Ivalue)) - 1;
               posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
               break;
            case PID_ENGINE_OIL_TEMP:
               posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(data[i].Ivalue)) - 1;
               posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
               break;
            case PID_BATTERY_VOLTAGE:
               posX = 33;
               posY = (FONT_12PT_H / 2) + 10;
               break;
            case PID_COOLANT_TEMP:
               posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(data[i].Ivalue)) - 1;
               posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
               break;
            case PID_FUEL_PRESSURE:
               posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(data[i].Ivalue)) - 1;
               posY = (SCREEN_HEIGHT / 2) + (FONT_9PT_H / 2) + 16;
               break;
            case PID_RPM:
               posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(data[i].Uvalue)) - 1;
               posY = SCREEN_HEIGHT - 1 - (FONT_9PT_H / 2);
               break;
            case PID_SPEED:
               posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(data[i].Uvalue)) - 1;
               posY = (SCREEN_HEIGHT / 2) + FONT_9PT_H;
               break;
            case PID_ENGINE_LOAD:
               posX = SCREEN_WIDTH - (FONT_9PT_W * numDigits(data[i].Ivalue)) - 1;
               posY = SCREEN_HEIGHT - 1;
               break;
            case PID_THROTTLE:
               posX = (SCREEN_WIDTH / 2) - (FONT_9PT_W * numDigits(data[i].Ivalue)) - 1;
               posY = (SCREEN_HEIGHT / 2) + FONT_9PT_H;
               break;
            case PID_MAF_FLOW:
               posX = (SCREEN_WIDTH / 2) - (FONT_12PT_W * numDigits(data[i].Fvalue)) - 1;
               posY = SCREEN_HEIGHT / 2 - 3;
               break;
            default:
               // AVERAGE MPG
               posX = (FONT_12PT_W * numDigits(data[i].Fvalue)) - 1;
               posY = SCREEN_HEIGHT / 2 - 3;
               break;
         }
         drawDataByPos(posX, posY, i);
      }
   }
} // end showData()


void drawDataByPos( uint16_t posX, uint16_t posY, byte inx )
{
   display.setCursor(posX, posY);
   switch ( data[inx].type )
   {
      case FLOAT_TYPE:
         display.setFont(&FreeSansBold12pt7b);
         display.print(data[inx].Fvalue, 1);
         break;
      case INT_TYPE:
         display.setFont(&FreeSansBold9pt7b);
         display.print(data[inx].Ivalue);
         break;
      case UINT_TYPE:
         display.setFont(&FreeSansBold9pt7b);
         display.print(data[inx].Uvalue);
         break;
   }
} // end drawDataByPos()


void drawBgScreen( void )
{
   display.setFont();
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
} // end drawBgScreen()


/*************************
 *    ARDUINO FUNCTIONS
 ************************/


void setup( void )
{
#ifdef _DEBUG_
   randomSeed(millis());
   Serial.begin(115200);
   _PL("\n\n\n=============\nOBD Gauge\n=============");
#endif
 
   // Initialize button 
   // pinMode(BUTTON_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, RISING);

   // Initialize screen
   if( !display.begin(SSD1306_SWITCHCAPVCC) )
   {
      _PP(millis()); _PL(F(": SSD1306 allocation failed"));
      while(1); // Don't proceed, loop forever
   }

   // Initialize scheduler
   runner.init();
   _PP(millis()); _PL(": Initialized scheduler");
   runner.addTask(tReadDataOBD);
   runner.addTask(tDrawScreen);
   runner.addTask(tBtnHandle);

   // Draw splash screen
   display.drawBitmap(0, 0, wywh, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE, BLACK);
   display.display();
   delay(5000);

   // Enable the scheduler tasks
   tReadDataOBD.enable();
   _PP(millis()); _PL(": tReadDataOBD enabled");
   tDrawScreen.enable();
   _PP(millis()); _PL(": tDrawScreen enabled");
   
   // Clear and set font size
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE, BLACK);

#ifndef _DEBUG_
   // Initialize OBD communitcation
   delay(500);
   obd.begin();
   display.println("\n  Connecting...");
   display.display();

   _PP(millis()); _PP(": Connecting to OBD");
   while (!obd.init())
   {
      _PP(".");
   }
   _PP("\n");
#endif
   display.clearDisplay();
} // end setup()


void loop( void )
{
   runner.execute();
} // end loop()


/*************************
 *    UTILITIES
 ************************/


int numDigits( int num )
{
   int num_digits = 0;
   while ( num != 0 )
   {
      num /= 10;
      num_digits++;
   }
   return num_digits;
} // end numDigits()


int numDigits( unsigned int num )
{
   int num_digits = 0;
   while ( num != 0 )
   {
      num /= 10;
      num_digits++;
   }
   return num_digits;
} // end numDigits()


int numDigits( float num )
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
