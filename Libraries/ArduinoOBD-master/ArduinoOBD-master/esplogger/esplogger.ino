/*************************************************************************
* OBD-II Data Logger based on ESP8266 or ESP32
* Distributed under GPL v2.0
* Developed by Stanley Huang <support@freematics.com.au>
*************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <OBD2UART.h>
#include "SH1106.h"
#include "images.h"
#include "config.h"
#include "datalogger.h"

// logger states
#define STATE_SD_READY 0x1
#define STATE_OBD_READY 0x2
#define STATE_GPS_FOUND 0x4
#define STATE_GPS_READY 0x8
#define STATE_ACC_READY 0x10
#define STATE_SLEEPING 0x20

static uint32_t lastFileSize = 0;
static int lastSpeed = -1;
static int speed = 0;
static uint32_t distance = 0;
static uint16_t fileIndex = 0;
static uint32_t startTime = 0;
static uint8_t lastPid = 0;
static int lastValue = 0;

static byte pidTier1[]= {PID_RPM, PID_SPEED, PID_ENGINE_LOAD, PID_THROTTLE};
static byte pidTier2[] = {PID_INTAKE_MAP, PID_MAF_FLOW, PID_TIMING_ADVANCE};
static byte pidTier3[] = {PID_COOLANT_TEMP, PID_INTAKE_TEMP, PID_AMBIENT_TEMP, PID_FUEL_LEVEL};

#define TIER_NUM1 sizeof(pidTier1)
#define TIER_NUM2 sizeof(pidTier2)
#define TIER_NUM3 sizeof(pidTier3)

byte pidValue[TIER_NUM1];

#ifdef ESP32
HardwareSerial Serial1(1);
#endif

class COBDLogger : public COBD, public CDataLogger
{
public:
    COBDLogger():state(0) {}
    void setup()
    {
        pinMode(8, OUTPUT);
        digitalWrite(8, LOW);
        
        showStates();

#if USE_MPU6050
        Wire.begin();
        if (MPU6050_init() == 0) {
            state |= STATE_ACC_READY;
            showStates();
        }
#endif

        do {
            showStates();
        } while (!init());

        state |= STATE_OBD_READY;

        showStates();

#if ENABLE_DATA_LOG
        // open file for logging
        if (!(state & STATE_SD_READY)) {
            if (checkSD()) {
                state |= STATE_SD_READY;
                showStates();
            }
        }

        if (state & STATE_SD_READY) {
          uint16_t index = openFile();
          lcd.setFontSize(FONT_SIZE_SMALL);
          lcd.setCursor(86, 0);
          if (index) {
              lcd.write('[');
              lcd.setFlags(FLAG_PAD_ZERO);
              lcd.printInt(index, 5);
              lcd.setFlags(0);
              lcd.write(']');
          } else {
              lcd.print("NO LOG");
          }
          delay(100);
        }
#endif

        initLoggerScreen();
    }
    void loop()
    {
        static byte index = 0;
        static byte index2 = 0;
        static byte index3 = 0;

        // poll OBD-II PIDs
        logOBDData(pidTier1[index++]);
        if (index == TIER_NUM1) {
            index = 0;
            if (index2 == TIER_NUM2) {
                index2 = 0;
                logOBDData(pidTier3[index3]);
                index3 = (index3 + 1) % TIER_NUM3;
            } else {
                logOBDData(pidTier2[index2++]);
            }
        }

        // display distance travelled (GPS)
        char buf[10];
        sprintf(buf, "%4ukm", (uint16_t)(distance / 1000));
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(92, 6);
        lcd.print(buf);

#if USE_MPU6050
        if (state & STATE_ACC_READY) {
            processAccelerometer();
        }
#endif

#if ENABLE_DATA_LOG
        // flush SD data every 1KB
        if (dataSize - lastFileSize >= 1024) {
            flushFile();
            lastFileSize = dataSize;
            // display logged data size
            char buf[7];
            sprintf(buf, "%4uKB", (int)(dataSize >> 10));
            lcd.setFontSize(FONT_SIZE_SMALL);
            lcd.setCursor(92, 7);
            lcd.print(buf);
        }
#endif

        if (errors >= 2) {
            reconnect();
        }
    }
#if ENABLE_DATA_LOG
    bool checkSD()
    {
        state &= ~STATE_SD_READY;
        if (!SD.begin()) {
          lcd.print("SD ");
          lcd.draw(cross, 16, 16);
          lcd.println();
          return false;
        }

        uint8_t cardType = SD.cardType();
        if(cardType == CARD_NONE){
          lcd.println("No SD card");
          return false;
        }

        if(cardType == CARD_MMC){
            Serial.print("MMC ");
        } else if(cardType == CARD_SD){
            Serial.print("SDSC ");
        } else if(cardType == CARD_SDHC){
            Serial.print("SDHC ");
        } else {
            Serial.print("SD ");
        }
        unsigned int cardSize = (unsigned int)SD.cardSize() >> 30;
        lcd.print(cardSize);
        lcd.println("G");

        state |= STATE_SD_READY;
        return true;
    }
#endif
private:
    void dataIdleLoop()
    {
        if (lastPid) {
            showLoggerData(lastPid, lastValue);
            lastPid = 0;
        }
    }
#if USE_MPU6050
    void processAccelerometer()
    {
        accel_t_gyro_union data;
        MPU6050_readout(&data);
        dataTime = millis();
        // log x/y/z of accelerometer
        logData(PID_ACC, data.value.x_accel, data.value.y_accel, data.value.z_accel);
        //showGForce(data.value.y_accel);
        // log x/y/z of gyro meter
        logData(PID_GYRO, data.value.x_gyro, data.value.y_gyro, data.value.z_gyro);
    }
#endif
    int logOBDData(byte pid)
    {
        int value = 0;
        // send a query to OBD adapter for specified OBD-II pid
        if (readPID(pid, value)) {
            dataTime = millis();
            // log data to SD card
            logData(0x100 | pid, value);
            lastValue = value;
            lastPid = pid;
        }
        return value;
    }
    void reconnect()
    {
#if ENABLE_DATA_LOG
        closeFile();
#endif
        lcd.clear();
        lcd.setFontSize(FONT_SIZE_MEDIUM);
        lcd.print("Reconnecting");
        startTime = millis();
        state &= ~(STATE_OBD_READY | STATE_ACC_READY);
        state |= STATE_SLEEPING;
        //digitalWrite(SD_CS_PIN, LOW);
        for (uint16_t i = 0; ; i++) {
            if (i == 5) {
                lcd.backlight(false);
                lcd.clear();
            }
            if (init()) {
                int value;
                if (readPID(PID_RPM, value) && value > 0)
                    break;
            }
        }
        state &= ~STATE_SLEEPING;
        fileIndex++;
        recover();
        setup();
    }
    byte state;

    void showTickCross(bool yes)
    {
        lcd.draw(yes ? tick : cross, 16, 16);
    }
    // screen layout related stuff
    void showStates()
    {
        lcd.setFontSize(FONT_SIZE_MEDIUM);
        lcd.setCursor(0, 4);
        lcd.print("OBD ");
        showTickCross(state & STATE_OBD_READY);
        lcd.setCursor(0, 6);
        lcd.print("ACC ");
        showTickCross(state & STATE_ACC_READY);
    }
    void showLoggerData(byte pid, int value)
    {
        char buf[8];
        switch (pid) {
        case PID_RPM:
            lcd.setCursor(64, 0);
            lcd.setFontSize(FONT_SIZE_XLARGE);
            lcd.printInt((unsigned int)value % 10000, 4);
            break;
        case PID_SPEED:
            if (lastSpeed != value) {
                lcd.setCursor(0, 0);
                lcd.setFontSize(FONT_SIZE_XLARGE);
                lcd.printInt((unsigned int)value % 1000, 3);
                lastSpeed = value;
            }
            break;
        case PID_THROTTLE:
            lcd.setCursor(24, 5);
            lcd.setFontSize(FONT_SIZE_SMALL);
            lcd.print(value % 100);
            break;
        case PID_INTAKE_TEMP:
            if (value < 1000) {
                lcd.setCursor(102, 5);
                lcd.setFontSize(FONT_SIZE_SMALL);
                lcd.print(value);
            }
            break;
        }
    }
#if USE_MPU6050
    void showGForce(int g)
    {
        byte n;
        /* 0~1.5g -> 0~8 */
        g /= 85 * 25;
        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(0, 3);
        if (g == 0) {
            lcd.clearLine(1);
        } else if (g < 0 && g >= -10) {
            for (n = 0; n < 10 + g; n++) {
                lcd.write(' ');
            }
            for (; n < 10; n++) {
                lcd.write('<');
            }
            lcd.print("        ");
        } else if (g > 0 && g < 10) {
            lcd.print("        ");
            for (n = 0; n < g; n++) {
                lcd.write('>');
            }
            for (; n < 10; n++) {
                lcd.write(' ');
            }
        }
    }
#endif
    void initLoggerScreen()
    {
        lcd.clear();
        lcd.backlight(true);
        lcd.setFontSize(FONT_SIZE_SMALL);
        lcd.setCursor(24, 3);
        lcd.print("km/h");
        lcd.setCursor(110, 3);
        lcd.print("rpm");
        lcd.setCursor(0, 5);
        lcd.print("THR:");
        lcd.setCursor(80, 5);
        lcd.print("AIR:");
    }
};

static COBDLogger logger;

void setup()
{
    Serial.begin(115200);
    Serial.println("Freematics");
    lcd.begin();
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.println("ESPLogger");
    delay(1000);

    logger.begin();

#if ENABLE_DATA_LOG
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.setCursor(0, 2);
    logger.checkSD();
#endif
    
    logger.setup();


}

void loop()
{
    logger.loop();
}
