#ifndef __BB_RTC__
#define __BB_RTC__

#include <Arduino.h>
#ifndef __AVR_ATtiny85__
#include <Wire.h>
#endif
#include <BitBang_I2C.h>

// I2C base address of the DS3231 RTC and AT24C32 EEPROM
#define RTC_DS3231_ADDR 0x68
#define EEPROM_ADDR 0x57
#define RTC_RV3032_ADDR 0x51
#define RTC_PCF8563_ADDR 0x51

// Status bits
#define STATUS_RUNNING 1
#define STATUS_IRQ1_TRIGGERED 2
#define STATUS_IRQ2_TRIGGERED 4

// Wire types
enum
{
  RTC_SW_WIRE=0, // Bit Bang
  RTC_HW_WIRE,
  RTC_HAS_WIRE
};

enum
{
  RTC_UNKNOWN=0,
  RTC_PCF8563,
  RTC_DS3231,
  RTC_RV3032,
  RTC_TYPE_COUNT
};

//
// Time structure
// modeled after Linux version
//
#ifndef _TIME_H_
struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};
#endif

// Alarm types
enum {
  ALARM_SECOND=0,
  ALARM_MINUTE,
  ALARM_HOUR,
  ALARM_TIME,
  ALARM_DAY,
  ALARM_DATE
};
class BBRTC
{
public:
    BBRTC() {}
    ~BBRTC() {};
    int getStatus();
    void setbb(BBI2C *pBB);
    int init(int iSDA=-1, int iSCL=-1, bool bWire = true);
    void setFreq(int iFreq);
    void setAlarm(uint8_t type, struct tm *thetime);
    int getTemp();
    void setTime(struct tm *pTime);
    void getTime(struct tm *pTime);
    setCountdownAlarm(int iSeconds);
    void clearAlarms();
    uint32_t getEpoch();
    void setEpoch(uint32_t tt);

private:
    int _iRTCType;
    int _iRTCAddr;
    BBI2C _bb;

}; // class BBRTC

#endif // __BB_RTC__

