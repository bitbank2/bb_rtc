//
// Test setting an alarm and triggering an interrupt on a pin
//
// GPIO definitions for the Waveshare ESP32-S3 1.54" e-Paper
#define SDA_PIN 47
#define SCL_PIN 48
#define IRQ_PIN 5

#include <bb_rtc.h>
BBRTC rtc;
struct tm myTime;
const char *szRTCType[] = {"None", "BM8563", "DS3231", "RV-3032", "PCF85063A", "RX8130"};

void setup()
{
  int rc;
  long l;
  pinMode(IRQ_PIN, INPUT_PULLUP); // open collector
  Serial.begin(115200);
  delay(3000); // allow time for CDC-UART to start
  Serial.println("Starting...");
  rc = rtc.init(SDA_PIN, SCL_PIN);
  if (rc == RTC_SUCCESS) {
    Serial.println("Success");
    Serial.printf("RTC type = %s\n", szRTCType[rtc.getType()]);
  } else {
    Serial.println("Failure, stopping...");
    while (1) {};
  }
  myTime.tm_hour = 1; myTime.tm_min = 0; myTime.tm_sec = 0;
  rtc.setTime(&myTime); // set it to a known time
  rtc.enableIRQ(true);
  myTime.tm_min = 1; // set an alarm 1 minute later
  rtc.setAlarm(ALARM_TIME, &myTime); // set the alarm
  Serial.println("Waiting for alarm to trigger one minute from now...");
  l = millis();
  while (digitalRead(IRQ_PIN) == HIGH && (millis() - l) < 61000) {
    delay(10);
  }
  l = millis() - l;
  if (l >= 61000) {
    Serial.println("Alarm failed to trigger IRQ");
  } else {
    Serial.println("IRQ triggered, success!");
  }
 // Try disabling the IRQ
  myTime.tm_hour = 1; myTime.tm_min = 0; myTime.tm_sec = 0;
  rtc.setTime(&myTime); // set it to a known time
  rtc.enableIRQ(false); // <-- disable the IRQ and see what happens
  myTime.tm_min = 1; // set an alarm 1 minute later
  rtc.setAlarm(ALARM_TIME, &myTime); // set the alarm
  Serial.println("Waiting for alarm to not trigger one minute from now...");
  l = millis();
  while (digitalRead(IRQ_PIN) == HIGH && (millis() - l) < 61000) {
    delay(10);
  }
  l = millis() - l;
  if (l >= 61000) {
    Serial.println("Alarm failed to trigger, success!");
  } else {
    Serial.println("IRQ triggered; something went wrong!");
  }  
}

void loop()
{
}
