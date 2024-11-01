#ifndef __BB_RTC__
#define __BB_RTC__
//
// BitBank Realtime Clock Library
// written by Larry Bank
//
// Copyright 2023 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================

#ifdef __LINUX__
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>
#include <time.h>
typedef struct _tagbbi2c
{
  int file_i2c;
  uint8_t iSDA, iSCL;
  uint8_t bWire;
} BBI2C;

#else
#include <Arduino.h>
#ifndef __AVR_ATtiny85__
#include <Wire.h>
#endif
#include <BitBang_I2C.h>
#endif // __LINUX__

#define RTC_SUCCESS 0
#define RTC_ERROR 1

// I2C base address of the DS3231 RTC and AT24C32 EEPROM
#define RTC_DS3231_ADDR 0x68
#define RTC_RV3032_ADDR 0x51
#define RTC_PCF8563_ADDR 0x51
#define RTC_PCF85063A_ADDR 0x51

// Status bits
#define STATUS_RUNNING 1
#define STATUS_IRQ1_TRIGGERED 2
#define STATUS_IRQ2_TRIGGERED 4

enum
{
  RTC_UNKNOWN=0,
  RTC_PCF8563,
  RTC_DS3231,
  RTC_RV3032,
  RTC_PCF85063A,
  RTC_TYPE_COUNT
};

//
// Time structure
// modeled after Linux version
//
#if !defined( _TIME_H_ ) && !defined( __LINUX__ )
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
    int getType();
    int getStatus();
    int init(int iSDA=-1, int iSCL=-1, bool bWire = true, uint32_t u32Speed = 100000);
    void logmsg(const char *msg);
    void setFreq(int iFreq);
    void setVBackup(bool bCharge);
    void setAlarm(uint8_t type, struct tm *thetime);
    int getTemp();
    void setTime(struct tm *pTime);
    void getTime(struct tm *pTime);
    void setCountdownAlarm(int iSeconds);
    void clearAlarms(bool bDisable = true);
    uint32_t getEpoch();
    void setEpoch(uint32_t tt);

private:
    int _iRTCType;
    int _iRTCAddr;
    BBI2C _bb;
}; // class BBRTC

#endif // __BB_RTC__

