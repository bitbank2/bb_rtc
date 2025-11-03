//
// Arduino function tests for the bb_rtc library
// written by Larry Bank
//
#include <bb_rtc.h>

BBRTC rtc;
const char *szRTCType[] = {"None", "BM8563", "DS3231", "RV-3032", "PCF85063A"};

void TestPassFail(bool bPass, int *pPass)
{
  if (bPass) {
    (*pPass)++;
    Serial.println(" - PASSED");
  } else {
    Serial.println(" - FAILED");
  }
} /* TestPassFail() */

void setup()
{
  Serial.begin(115200);
  delay(3000); // wait for CDC-Serial to start
  Serial.println("Start of bb_rtc function tests...");
} /* setup() */

void loop()
{
  int i, iStatus, iNewStatus, iTests=0, iPass=0;
  struct tm myTime;

  Serial.println("Uninitialized class tests");
  iTests++;
  Serial.print("getType() should return RTC_UNKNOWN"); // All other BBRTC methods should return RTC_ERROR
  TestPassFail(rtc.getType() == RTC_UNKNOWN, &iPass);

  iTests++;
  Serial.print("getStatus() should return RTC_ERROR");
  TestPassFail(rtc.getStatus() == RTC_ERROR, &iPass);

  iTests++;
  Serial.print("getTemp() should return RTC_ERROR");
  TestPassFail(rtc.getTemp() == RTC_ERROR, &iPass);

  Serial.print("Results for uninitialized tests: ");
  Serial.print(iPass, DEC); Serial.print(" of ");
  Serial.print(iTests, DEC); Serial.println(" passed");

  Serial.println("Initialized class tests");
  iTests = iPass = 0;

  iTests++;
  Serial.print("init() returns RTC_SUCCESS for supported device");
  TestPassFail(rtc.init() == RTC_SUCCESS, &iPass);

  iTests++;
  Serial.print("getType() returns a valid type");
  i = rtc.getType();
  TestPassFail(i > RTC_UNKNOWN && i < RTC_TYPE_COUNT, &iPass);

  iTests++;
  Serial.print("getStatus() returns STATUS_RUNNING");
  TestPassFail(rtc.getStatus() & STATUS_RUNNING, &iPass);

  iTests++;
  Serial.print("Clock chip advances the time");
  memset(&myTime, 0, sizeof(myTime)); // set the current time to 00:00:00
  rtc.setTime(&myTime);
  delay(1000); // wait one second
  rtc.getTime(&myTime);
  TestPassFail(myTime.tm_sec == 1, &iPass);

  iTests++;
  Serial.print("Countdown timer triggers IRQ");
  memset(&myTime, 0, sizeof(myTime));
  rtc.clearAlarms(true); // clear any old alarms
  rtc.setTime(&myTime);
  iStatus = rtc.getStatus(); // check that the IRQ status is clear before the alarm trigggers
  rtc.setCountdownAlarm(3);
  delay(3000);
  iNewStatus = rtc.getStatus();
  iStatus &= (STATUS_IRQ1_TRIGGERED | STATUS_IRQ2_TRIGGERED);
  iNewStatus &= (STATUS_IRQ1_TRIGGERED | STATUS_IRQ2_TRIGGERED);
  TestPassFail(iStatus == 0 && iNewStatus != 0, &iPass);

  iTests++;
  Serial.print("Time match alarm triggers IRQ");
  memset(&myTime, 0, sizeof(myTime));
  rtc.clearAlarms(true); // clear any old alarms
  rtc.setTime(&myTime);
  iStatus = rtc.getStatus(); // check that the IRQ status is clear before the alarm trigggers
  myTime.tm_sec = 3;
  rtc.setAlarm(ALARM_TIME, &myTime);
  delay(3000);
  iNewStatus = rtc.getStatus();
  iStatus &= (STATUS_IRQ1_TRIGGERED | STATUS_IRQ2_TRIGGERED);
  iNewStatus &= (STATUS_IRQ1_TRIGGERED | STATUS_IRQ2_TRIGGERED);
  TestPassFail(iStatus == 0 && iNewStatus != 0, &iPass);

  Serial.print("Results for initialized tests: ");
  Serial.print(iPass, DEC); Serial.print(" of ");
  Serial.print(iTests, DEC); Serial.println(" passed");

  Serial.println("Tests Complete");
  while (1) {
    delay(1); // wait forever
  }
} /* loop() */

