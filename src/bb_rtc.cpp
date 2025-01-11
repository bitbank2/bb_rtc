//
// BitBang RealTime Clock library (bb_rtc)
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
#include "bb_rtc.h"

#define LOGGING

void BBRTC::logmsg(const char *msg)
{
#ifdef LOGGING
#ifdef __LINUX__
  printf(msg);
  printf("\n");
#else
  Serial.println(msg);
#endif
#endif
} /* logmsg() */

// Implement I2C wrapper functions for Linux
#ifdef __LINUX__
void delay(uint32_t u32)
{
	usleep(u32*1000);
} /* delay() */

void I2CInit(BBI2C *pI2C, uint32_t iClock)
{
char filename[32];
int iChannel = pI2C->iSDA;

	sprintf(filename, "/dev/i2c-%d", iChannel);
        if ((pI2C->file_i2c = open(filename, O_RDWR)) < 0)
        {       
                fprintf(stderr, "Failed to open the i2c bus\n");
        }               
} /* I2CInit() */
uint8_t I2CTest(BBI2C *pI2C, uint8_t addr)
{
uint8_t response = 0;
    if (ioctl(pI2C->file_i2c, I2C_SLAVE, addr) >= 0) {
            // probe this address
        uint8_t ucTemp;
        if (read(pI2C->file_i2c, &ucTemp, 1) >= 0)
            response = 1;
    }
    return response;
} /* I2CTest() */

int I2CRead(BBI2C *pI2C, uint8_t iAddr, uint8_t *pData, int iLen)
{
int rc;
	ioctl(pI2C->file_i2c, I2C_SLAVE, iAddr);
	rc = read(pI2C->file_i2c, pData, iLen);
	return rc;
} /* I2CRead() */
int I2CReadRegister(BBI2C *pI2C, uint8_t iAddr, uint8_t u8Register, uint8_t *pData, int iLen)
{
int rc;
        // Reading from an I2C device involves first writing the 8-bit register
        // followed by reading the data
	ioctl(pI2C->file_i2c, I2C_SLAVE, iAddr);
        rc = write(pI2C->file_i2c, &u8Register, 1); // write the register value
        if (rc == 1)
        {
                rc = read(pI2C->file_i2c, pData, iLen);
        }
        return rc;

} /* I2CReadRegister() */

int I2CWrite(BBI2C *pI2C, uint8_t iAddr, uint8_t *pData, int iLen)
{
int rc;
	ioctl(pI2C->file_i2c, I2C_SLAVE, iAddr);
	rc = write(pI2C->file_i2c, pData, iLen);
	return rc;
} /* I2CWrite() */
#endif // __LINUX__

//
// Return the RTC chip type
//
int BBRTC::getType(void)
{
    return _iRTCType;
} /* getType() */

//
// Enable or disable trickle charging
// of the backup battery source
//
void BBRTC::setVBackup(bool bCharge)
{
uint8_t ucTemp[4];

    if (_iRTCType != RTC_RV3032) return; // only supported on RVxxxx devices

    ucTemp[0] = 0x11;
    ucTemp[1] = 0x4; // event interrupt enabled
    I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);
    ucTemp[0] = 0x15;
    ucTemp[1] = 0x0; // event filter off
    I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);
    ucTemp[0] = 0x10; // control 1
    ucTemp[1] = 0x04; // EERD is disabled to allow modifying EEPROM values
    I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);

    if (bCharge) { // enable trickle charging on VBAT pin
         ucTemp[0] = 0x3d; // EEADDR, EEDATA
         ucTemp[1] = 0xc0; // eeprom PMU register
         ucTemp[2] = 0x11; // enable trickle charger and direct switching mode
         I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 3);
    } else { // disable trickle charging on VBAT pin (default)
         ucTemp[0] = 0x3d; // EEADDR, EEDATA
         ucTemp[1] = 0xc0; // eeprom PMU register
         ucTemp[2] = 0x00; // disable trickle charger and DSM (default value)
         I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 3);   
    } // disable trickle charging
 // write the changed byte into EEPROM, then copy all EEPROM registers to RAM
    ucTemp[0] = 0x3f; // eeprom command
    ucTemp[1] = 0x21; // write 1 byte of EEPROM data
    I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);
    delay(10); // doc says 5-9ms to write one byte
    ucTemp[0] = 0x3f; // eeprom command
    ucTemp[1] = 0x12; // copy EEPROM to RAM backup registers
    I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);
    delay(64);
} /* setVBackup() */

//
// Turn on the RTC
// returns 1 for success, 0 for failure
//
int BBRTC::init(int iSDA, int iSCL, bool bWire, uint32_t u32Speed)
{
uint8_t ucTemp[4];

  logmsg("Entering init");
  memset(&_bb,0,sizeof(_bb));
  _bb.iSDA = iSDA;
  _bb.iSCL = iSCL;
  _bb.bWire = bWire;
  I2CInit(&_bb, u32Speed); // initialize the bit bang library
  _iRTCType = -1;
  if (I2CTest(&_bb, RTC_DS3231_ADDR)) {
     // Make sure it's really a DS3231 because other I2C devices
     // use the same address (0x68)
      I2CReadRegister(&_bb, RTC_DS3231_ADDR, 0x12, ucTemp, 1); // read temp reg
      if ((ucTemp[0] & 0x3f) == 0) {
	logmsg("Found DS3231");
         _iRTCAddr = RTC_DS3231_ADDR;
         _iRTCType = RTC_DS3231;
      }
  }
  if (_iRTCType == -1 && I2CTest(&_bb, RTC_RV3032_ADDR)) {
     // The PCF85063A, PCF8563 and RV3032 all use the same I2C address (0x51)
     // Try to write to the temperature threshold register to see
     // which one is connected
     ucTemp[0] = 0x17; // temp threshold high register
     ucTemp[1] = 0x55; // random value to write
     I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);
     I2CReadRegister(&_bb, RTC_RV3032_ADDR, 0x17, ucTemp, 1);
     if (ucTemp[0] == 0x55) {
        logmsg("Found RV3032");
        _iRTCAddr = RTC_RV3032_ADDR;
        _iRTCType = RTC_RV3032;
     } else { // it must be the PCF8563 or PCF85063A
        ucTemp[0] = 0x3; // 1 byte of RAM in PCF85063A, minutes reg in BM8563
        ucTemp[1] = 0xaa;
        I2CWrite(&_bb, RTC_RV3032_ADDR, ucTemp, 2);
        I2CReadRegister(&_bb, RTC_RV3032_ADDR, 0x03, ucTemp, 1);
        if (ucTemp[0] == 0xaa) {
            logmsg("Found PCF85063A");
            _iRTCAddr = RTC_PCF85063A_ADDR;
            _iRTCType = RTC_PCF85063A;
        } else {
            logmsg("Found PCF8563");
            _iRTCAddr = RTC_PCF8563_ADDR;
            _iRTCType = RTC_PCF8563;
        }
     }
  }
  if (_iRTCType == -1) { // not found
     logmsg("no supported device found");
     return RTC_ERROR;
  }

  if (_iRTCType == RTC_DS3231) {
    ucTemp[0] = 0xe; // control register
    ucTemp[1] = 0x1c; // enable main oscillator and interrupt mode for alarms
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
  } else if (_iRTCType == RTC_RV3032) {
    // Enable direct switchover mode to the backup battery (disabled on delivery)
    ucTemp[0] = 0xc0; // EEPROM PMU
    ucTemp[1] = 0x10; // enable direct VBACKUP switchover, disable trickle charge
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 2); 
  } else { // PCF8563 and PCF85063A
    ucTemp[0] = 0; // control_status_1
    ucTemp[1] = 0; // normal mode, clock on, power-on-reset disabled
    ucTemp[2] = 0; // disable all alarms
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
  }
  return RTC_SUCCESS;
} /* init() */
//
// Enable/set the CLKOUT frequency (-1 = disable)
//
void BBRTC::setFreq(int iFreq)
{
uint8_t c, ucTemp[4];
int i;

   if (_iRTCType == RTC_RV3032) {
      if (iFreq == -1) { // disable it
          I2CReadRegister(&_bb, _iRTCAddr, 0xc0, &ucTemp[1], 1); // read control register
          ucTemp[0] = 0xc0; // write it back with NCLKE set to disable CLKOUT
          ucTemp[1] |= 0x40; // set NCLKE
          I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
      } else { // enable clock
          I2CReadRegister(&_bb, _iRTCAddr, 0xc0, &ucTemp[1], 1); // read control register
          ucTemp[0] = 0xc0; // write it back with NCLKE set to disable CLKOUT
          ucTemp[1] &= ~0x40; // clear NCLKE
          I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
          c = 0; // default = 32768
          if (iFreq <= 32768) { // low speed
             ucTemp[0] = 0xc3; // CLKOUT control
             if (iFreq == 1024) c = 1;
             else if (iFreq == 64) c = 2;
             else if (iFreq == 1) c = 3; // all other values will stay at 32k
             ucTemp[1] = c << 5; // bits 5+6 in 32k mode
             I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
          } else { // high speed
             ucTemp[0] = 0xc2; // HFD + CLKOUT control
             i = (iFreq / 8192000) - 1;
             if (i < 0) i = 0;
             else if (i > 8191) i = 8191; // top 13 bits of freq up to 67Mhz
             ucTemp[1] = (uint8_t)(i & 0xff);
             ucTemp[2] = (uint8_t)(0x80 | ((i >> 8) & 0x1f));
             I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
          }
      }
   } else if (_iRTCType == RTC_DS3231) {
       if (iFreq == -1) { // disable CLKOUT (allow interrupts)
          ucTemp[0] = 0xe;// control register
          ucTemp[1] = 0x4; // disable SQW and enable interrupts 
       } else { // enable CLKOUT (disable interrupts)
          ucTemp[0] = 0xe;
          c = 3; // assume 8192Hz (default
          if (iFreq == 1) c = 0;
          else if (iFreq == 1024) c = 1;
          else if (iFreq == 4096) c = 2;
          else if (iFreq == 8192) c = 3;
          ucTemp[1] = (c << 3); // enable SQW, disable interrupts
       }
       I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
   } else if (_iRTCType == RTC_PCF8563) {
       ucTemp[0] = 0xd; // CLKOUT control
       if (iFreq == -1)  { // disable CLKOUT
          ucTemp[1] = 0x0; // disable CLKOUT
       } else { // enable CLKOUT at the given frequency
          if (iFreq == 32768)
             ucTemp[1] = 0x80;
          else if (iFreq == 1024)
             ucTemp[1] = 0x81;
          else if (iFreq == 32)
             ucTemp[1] = 0x82;
          else ucTemp[1] = 0x83; // assume 1Hz
       }
       I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
   }
} /* setFreq() */
//
// Retrieve the current power & irq status
//
int BBRTC::getStatus(void)
{
int iStatus = 0;
uint8_t ucTemp[4];

  if (_iRTCType == RTC_DS3231) {
     I2CReadRegister(&_bb, _iRTCAddr, 0xf, ucTemp, 1); // read the status register
     if (!(ucTemp[0] & 0x80)) // oscillator running/stopped
        iStatus |= STATUS_RUNNING;
     if (ucTemp[0] & 2)
        iStatus |= STATUS_IRQ2_TRIGGERED;
     if (ucTemp[0] & 1)
        iStatus |= STATUS_IRQ1_TRIGGERED;
  } else if (_iRTCType == RTC_RV3032) {
     iStatus |= STATUS_RUNNING; // oscillator is always running
     I2CReadRegister(&_bb, _iRTCAddr, 0xd, ucTemp, 1); // read the status register
     if (ucTemp[0] & 8) // alarm fired
        iStatus |= STATUS_IRQ1_TRIGGERED;
  } else if (_iRTCType == RTC_PCF8563) {
     I2CReadRegister(&_bb, _iRTCAddr, 0x00, ucTemp, 2); // read control regs 1 & 2
     if (!(ucTemp[0] & 0x20))
        iStatus |= STATUS_RUNNING;
     if (ucTemp[1] & 8)
        iStatus |= STATUS_IRQ1_TRIGGERED; 
  }
  return iStatus;
} /* getStatus() */
//
// Get the UNIX epoch time
// (only available on the RV-3032-C7
//
uint32_t BBRTC::getEpoch(void)
{
uint32_t tt = 0;

   if (_iRTCType != RTC_RV3032)
      return tt;
   I2CReadRegister(&_bb, _iRTCAddr, 0x1b, (uint8_t *)&tt, sizeof(tt)); 
   return tt;
} /* getEpoch() */
//
// Set the UNIX epoch time
// (only available on the RV-3032-C7
//
void BBRTC::setEpoch(uint32_t tt)
{
uint8_t ucTemp[8];

  if (_iRTCType == RTC_RV3032) {
    I2CReadRegister(&_bb, _iRTCAddr, 0x10, &ucTemp[1], 1); // read control register 2
    ucTemp[0] = 0x10;
    ucTemp[1] |= 1; // set RESET BIT
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 2); // do a reset of seconds and prescaler
    ucTemp[0] = 0x1b;
    memcpy(&ucTemp[1], (uint8_t *)&tt, sizeof(tt));
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 1+sizeof(tt)); // set time
  }
} /* setEpoch() */

//
// Set Alarm for:
// ALARM_SECOND = Once every second
// ALARM_MINUTE = Once every minute
// ALARM_HOUR = for a specific hour
// ALARM_TIME = When a specific hour:second match
// ALARM_DAY = When a specific day of the week and time match
// ALARM_DATE = When a specific day of the month and time match
//
void BBRTC::setAlarm(uint8_t type, struct tm *pTime)
{
uint8_t ucTemp[8];

  if (_iRTCType == RTC_DS3231)
  {
    switch (type)
    {
      case ALARM_SECOND: // turn on repeating alarm for every second
        ucTemp[0] = 0xe; // control register
        ucTemp[1] = 0x1d; // enable alarm1 interrupt
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0x7; // starting register for alarm 1
        ucTemp[1] = 0x80; // set bit 7 in the 4 registers to tell it a repeating alarm
        ucTemp[2] = 0x80;
        ucTemp[3] = 0x80;
        ucTemp[4] = 0x80;
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 5);
        break;
      case ALARM_MINUTE: // turn on repeating alarm for every minute
        ucTemp[0] = 0xe; // control register
        ucTemp[1] = 0x1e; // enable alarm2 interrupt
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0xb; // starting register for alarm 2
        ucTemp[1] = 0x80; // set bit 7 in the 3 registers to tell it a repeating alarm
        ucTemp[2] = 0x80;
        ucTemp[3] = 0x80;
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 4);
        break;
      case ALARM_TIME: // turn on alarm to match a specific time
      case ALARM_DAY: // turn on alarm for a specific day of the week
      case ALARM_DATE: // turn on alarm for a specific date
// Values are stored as BCD
        ucTemp[0] = 0x7; // start at register 7
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        if (type == ALARM_DAY)
           ucTemp[4] = pTime->tm_wday + 1;
        // day of the month
        else if (type == ALARM_DATE) {
          ucTemp[4] = (pTime->tm_mday / 10) << 4;
          ucTemp[4] |= (pTime->tm_mday % 10);
        } else {
          ucTemp[4] = 0;
        }
        // set the A1Mx bits (high bits of the 4 registers)
        // for the specific type of alarm
        ucTemp[1] &= 0x7f; // make sure A1M1 & A1M2 are set to 0
        ucTemp[2] &= 0x7f;
        if (type == ALARM_TIME) // A1Mx bits should be x1000
        {
          ucTemp[3] &= 0x7f;
          ucTemp[4] |= 0x80;
        }
        else if (type == ALARM_DAY) // A1Mx bits should be 10000
        {
          ucTemp[3] &= 0x7f;
          ucTemp[4] &= 0x7f;
          ucTemp[4] |= 0x40; // DY/DT bit
        }
        // for matching the date, all bits are left as 0's (00000)
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 5);
        ucTemp[0] = 0xe; // control register
        ucTemp[1] = 0x5; // enable alarm1 interrupt
        ucTemp[2] = 0x00; // reset alarm status bits
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
        break;
     } // switch on type
  }
  else if (_iRTCType == RTC_PCF8563)
  {
    switch (type)
    {
      case ALARM_SECOND: // turn on repeating alarm for every second
        ucTemp[0] = 0x1; // control_status_2
        ucTemp[1] = 0x5; // enable timer & interrupt
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0xe; // timer control
        ucTemp[1] = 0x81; // enable timer for 1/64 second interval
        ucTemp[2] = 0x40; // timer count value (64 = 1 second)
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
        break;
      case ALARM_MINUTE: // turn on repeating timer for every minute
        ucTemp[0] = 0x1; // control_status_2
        ucTemp[1] = 0x5; // enable timer & interrupt
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0xe; // timer control
        ucTemp[1] = 0x82; // enable timer for 1 hz interval
        ucTemp[2] = 0x3c; // 60 = 1 minute
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
        break;
      case ALARM_TIME: // turn on alarm to match a specific time
      case ALARM_DAY: // turn on alarm for a specific day of the week
      case ALARM_DATE: // turn on alarm for a specific date
        ucTemp[0] = 0x1; // control_status_2
        ucTemp[1] = 0xa; // enable alarm & interrupt
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
// Values are stored as BCD
        ucTemp[0] = 0x9; // start at register 9
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        ucTemp[1] |= 0x80; // disable
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        ucTemp[2] |= 0x80; // disable
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        ucTemp[3] |= 0x80; // disable
        // day of the week
        ucTemp[5] = pTime->tm_wday + 1;
        ucTemp[5] = 0x80; // disable
        // day of the month
        ucTemp[4] = (pTime->tm_mday / 10) << 4;
        ucTemp[4] |= (pTime->tm_mday % 10);
        ucTemp[4] |= 0x80; // disable
        // clear high bits of the 4 registers
        // for the specific type of alarm
        if (type == ALARM_TIME)
        {
          ucTemp[1] &= 0x7f;
          ucTemp[2] &= 0x7f;
          ucTemp[3] &= 0x7f;
        }
        else if (type == ALARM_DAY)
        {
          ucTemp[5] &= 0x7f;
        }
        else if (type == ALARM_DATE)
        {
          ucTemp[4] &= 0x7f;
        }
        I2CWrite(&_bb, _iRTCAddr, ucTemp, 6);
        break;
     } // switch on alarm type
   } // PCF8563
   else if (_iRTCType == RTC_RV3032) {
      switch (type)
      {
         //case ALARM_SECOND: // not supported 
         case ALARM_MINUTE: // repeats on a specific minute
            ucTemp[0] = 0x08; // minutes alarm
            if (pTime == NULL) { // set repeating alarm every minute
               ucTemp[1] = 0x80; // all disabled = repeating alarm
            } else { // wake at a specific minute
               ucTemp[1] = ((pTime->tm_min / 10) << 4);
               ucTemp[1] |= (pTime->tm_min % 10); // first 7 bits hold BCD minutes
            }
            ucTemp[2] = 0x80; // disable hours alarm
            ucTemp[3] = 0x80; // disable date alarm
            I2CWrite(&_bb, _iRTCAddr, ucTemp, 4);
            break;
         case ALARM_HOUR: // repeats on a specific hour
            ucTemp[0] = 0x08; // minutes alarm
            ucTemp[1] = 0x80; // disable minutes alarm
            ucTemp[2] = ((pTime->tm_hour / 10) << 4);
            ucTemp[2] |= (pTime->tm_hour % 10);
            ucTemp[3] = 0x80; // disable date alarm
            I2CWrite(&_bb, _iRTCAddr, ucTemp, 4);
            break;
         case ALARM_TIME:
         //case ALARM_DAY: // not supported
         case ALARM_DATE:
            ucTemp[0] = 0x08; // minutes alarm
            ucTemp[1] = ((pTime->tm_min / 10) << 4);
            ucTemp[1] |= (pTime->tm_min % 10); // first 7 bits hold BCD minutes
            ucTemp[2] = ((pTime->tm_hour / 10) << 4);
            ucTemp[2] |= (pTime->tm_hour % 10);
            if (type == ALARM_TIME) {
               ucTemp[3] = 0x80; // disable date alarm
            } else {
               ucTemp[3] = ((pTime->tm_mday+1) / 10) << 4;
               ucTemp[3] |= ((pTime->tm_mday+1) % 10);
            }
            I2CWrite(&_bb, _iRTCAddr, ucTemp, 4);
            break;
      } // switch on alarm type
      ucTemp[0] = 0x11; // Control 2
      ucTemp[1] = 0x08; // enable time interrupt and disable other int functions
      I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
   } // RV3032
} /* setAlarm() */

//
// Set a countdown alarm for N seconds
//
void BBRTC::setCountdownAlarm(int iSeconds)
{
uint8_t ucTemp[4];

  if (_iRTCType == RTC_RV3032) {
     ucTemp[0] = 0xb; // low byte of countdown timer
     ucTemp[1] = (uint8_t)iSeconds;
     ucTemp[2] = (uint8_t)(iSeconds >> 8);
     I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
     // set up the clock frequency to use seconds as the period
     I2CReadRegister(&_bb, _iRTCAddr, 0x10, &ucTemp[1], 3); // control reg 1/2/3
     ucTemp[1] &= 0xfc; // control 1
     ucTemp[1] |= 0x0a; // enable TE (period countdown timer), set TD = 10 = 1Hz
     ucTemp[2] &= ~0x2c; // disable periodic/alarm and external interrupts
     ucTemp[2] |= 0x10; // enable countdown interrupt
     ucTemp[3] = 0; // disable backup switchover and all temperature interrupts
     ucTemp[0] = 0x10; // write all 3 control registers back
     I2CWrite(&_bb, _iRTCAddr, ucTemp, 4); // start countdown timer
  } else if (_iRTCType == RTC_DS3231) {
  // The DS3231 doesn't have a countdown timer, but we can set an alarm
  // to match hr/min/sec (unlike the RV3032)
     struct tm theTime;
     int iSecs; // seconds since midnight
     getTime(&theTime);
     iSecs = theTime.tm_hour*3600;
     iSecs += (theTime.tm_min*60);
     iSecs += theTime.tm_sec;
     iSecs += iSeconds; // add alarm delay period
     if (iSecs >= 24*60*60) // seconds in a day
        iSecs -= 24*60*60;
     // Put the wake time back in the time structure
     theTime.tm_hour = iSecs / 3600;
     iSecs %= 3600;
     theTime.tm_min = iSecs / 60;
     iSecs %= 60;
     theTime.tm_sec = iSecs;
     setAlarm(ALARM_TIME, &theTime);
  }
} /* setCountdownAlarm() */
//
// Read the current internal temperature
// Value is celcius * 4 (resolution of 0.25C)
//
int BBRTC::getTemp(void)
{
unsigned char ucTemp[2];
int iTemp = 0;

  if (_iRTCType == RTC_DS3231) {
    I2CReadRegister(&_bb, _iRTCAddr, 0x11, ucTemp, 2); // MSB location
    iTemp = ucTemp[0] << 8; // high byte
    iTemp |= ucTemp[1]; // low byte
    iTemp >>= 6; // lower 2 bits are fraction; upper 8 bits = integer part
  } else if (_iRTCType == RTC_RV3032) {
    I2CReadRegister(&_bb, _iRTCAddr, 0x0e, ucTemp, 2); // LSB, then MSB
    iTemp = ucTemp[0] | (ucTemp[1] << 8);
    iTemp >>= 6; // lower 2 bits are fraction upper 8 are integer
  }
  return iTemp; // no temperature sensor
} /* getTemp() */
//
// Set the current time/date
//
void BBRTC::setTime(struct tm *pTime)
{
unsigned char ucTemp[20];
uint8_t i;

   if (_iRTCType == RTC_DS3231) {
// Values are stored as BCD
        ucTemp[0] = 0; // start at register 0
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        ucTemp[4] = pTime->tm_wday + 1;
        // day of the month
        ucTemp[5] = (pTime->tm_mday / 10) << 4;
        ucTemp[5] |= (pTime->tm_mday % 10);
        // month + century
        i = pTime->tm_mon+1; // 1-12 on the RTC
        ucTemp[6] = (i / 10) << 4;
        ucTemp[6] |= (i % 10);
        if (pTime->tm_year >= 100)
           ucTemp[6] |= 0x80; // century bit
        // year
        ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
        ucTemp[7] |= (pTime->tm_year % 10);
    } else if (_iRTCType == RTC_PCF8563 || _iRTCType == RTC_PCF85063A) {
        ucTemp[0] = (_iRTCType == RTC_PCF8563) ? 2:4; // start at register 2/4
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        ucTemp[5] = pTime->tm_wday + 1;
        // day of the month
        ucTemp[4] = (pTime->tm_mday / 10) << 4;
        ucTemp[4] |= (pTime->tm_mday % 10);
        // month + century
        i = pTime->tm_mon+1; // 1-12 on the RTC
        ucTemp[6] = (i / 10) << 4;
        ucTemp[6] |= (i % 10);
        if (pTime->tm_year >= 100 && _iRTCType == RTC_PCF8563)
           ucTemp[6] |= 0x80; // century bit
        // year
        ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
        ucTemp[7] |= (pTime->tm_year % 10);
    } else if (_iRTCType == RTC_RV3032) {
// Values are stored as BCD
        ucTemp[0] = 1; // start at register 1
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        ucTemp[4] = pTime->tm_wday;
        // day of the month
        ucTemp[5] = (pTime->tm_mday / 10) << 4;
        ucTemp[5] |= (pTime->tm_mday % 10);
        // month
        i = pTime->tm_mon+1; // 1-12 on the RTC
        ucTemp[6] = (i / 10) << 4;
        ucTemp[6] |= (i % 10);
        // year
        ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
        ucTemp[7] |= (pTime->tm_year % 10);
    }
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 8);

} /* setTime() */

//
// Read the current time/date
//
void BBRTC::getTime(struct tm *pTime)
{
unsigned char ucTemp[20];

  if (_iRTCType == RTC_DS3231) {
        I2CReadRegister(&_bb, _iRTCAddr, 0, ucTemp, 7); // start of data registers
        memset(pTime, 0, sizeof(struct tm));
        // convert numbers from BCD
        pTime->tm_sec = ((ucTemp[0] >> 4) * 10) + (ucTemp[0] & 0xf);
        pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
        // hours are stored in 24-hour format in the tm struct
        if (ucTemp[2] & 64) // 12 hour format
        {
                pTime->tm_hour = ucTemp[2] & 0xf;
                pTime->tm_hour += ((ucTemp[2] >> 4) & 1) * 10;
                pTime->tm_hour += ((ucTemp[2] >> 5) & 1) * 12; // AM/PM
        }
        else // 24 hour format
        {
                pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);
        }
        pTime->tm_wday = ucTemp[3] - 1; // day of the week (0-6)
        // day of the month
        pTime->tm_mday = ((ucTemp[4] >> 4) * 10) + (ucTemp[4] & 0xf);
        // month
        pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11
        pTime->tm_year = (ucTemp[5] >> 7) * 100; // century
        pTime->tm_year += ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);
  } else if (_iRTCType == RTC_PCF8563 || _iRTCType == RTC_PCF85063A) {
        I2CReadRegister(&_bb, _iRTCAddr, (_iRTCType == RTC_PCF8563) ? 2 : 4, ucTemp, 7); // start of data registers
        memset(pTime, 0, sizeof(struct tm));
        // convert numbers from BCD
        pTime->tm_sec = (((ucTemp[0] >> 4) & 7) * 10) + (ucTemp[0] & 0xf);
        pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
        // hours are stored in 24-hour format in the tm struct
        pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);
        pTime->tm_wday = ucTemp[4] - 1; // day of the week (0-6)
        // day of the month
        pTime->tm_mday = ((ucTemp[3] >> 4) * 10) + (ucTemp[3] & 0xf);
        // month
        pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11
        if (_iRTCType == RTC_PCF8563) {
            pTime->tm_year = (ucTemp[5] >> 7) * 100; // century
        } else { // assume 20th century
            pTime->tm_year = 100;
        }
        pTime->tm_year += ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);
  } else if (_iRTCType == RTC_RV3032) {
        I2CReadRegister(&_bb, _iRTCAddr, 0x01, ucTemp, 7); // start of data registers
        memset(pTime, 0, sizeof(struct tm));
        // convert numbers from BCD
        pTime->tm_sec = ((ucTemp[0] >> 4) * 10) + (ucTemp[0] & 0xf);
        pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
        pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);
        pTime->tm_wday = (ucTemp[3] & 7); // day of the week (0-6)
        // day of the month
        pTime->tm_mday = ((ucTemp[4] >> 4) * 10) + (ucTemp[4] & 0xf);
        // month
        pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11     
        pTime->tm_year = 100 + ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);
  }
} /* getTime() */
//
// Reset the "fired" bits for Alarm 1 and 2
// Interrupts will not occur until these bits are cleared
//
void BBRTC::clearAlarms(bool bDisable)
{
uint8_t ucTemp[4];

  if (_iRTCType == RTC_DS3231)
  {
    ucTemp[0] = 0xe; // control register
    ucTemp[1] = 0x4; // disable alarm interrupt bits
    ucTemp[2] = 0x0; // clear A1F & A2F (alarm 1 or 2 fired) bit to allow it to fire again
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 3);
  }
  else if (_iRTCType == RTC_PCF8563)
  {
    ucTemp[0] = 1; // control_status_2
    ucTemp[1] = 0; // disable all alarms
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
  }
  else if (_iRTCType == RTC_PCF85063A)
  {
      I2CReadRegister(&_bb, _iRTCAddr, 1, &ucTemp[1], 1); // read reg value first
      ucTemp[1] &= 7; // disable all alarm flags while leaving clockout bits
      ucTemp[0] = 1; // control_status_2
      I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
  }
  else if (_iRTCType == RTC_RV3032)
  {
    if (bDisable) {
       I2CReadRegister(&_bb, _iRTCAddr, 0x11, &ucTemp[1], 1);
       ucTemp[0] = 0x11; // control 2
       ucTemp[1] &= 0x81; // disable all alarms
       I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
    }
    ucTemp[0] = 0x0d; // status register
    ucTemp[1] = 0x00; // clear all flags
    I2CWrite(&_bb, _iRTCAddr, ucTemp, 2);
  }
} /* clearAlarms() */

