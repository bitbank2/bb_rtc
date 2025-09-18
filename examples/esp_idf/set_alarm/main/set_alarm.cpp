//
// Set an alarm on the PCF8563 RTC of the M5Stack StickCPlus2
// This example shows how to enable the alarm feature of the RTC
// to generate an interrupt and status change at a specific time
// The alarm feature of the PCF8563 only allows setting the wakeup
// date+time down to the minute
// The RTC IRQ signal will be generated, but requires additional programming
// of the power management chip of the StickCPlus2 and is beyond the scope
// of this example.
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <bb_rtc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

BBRTC rtc;
// These are the GPIO pins on M5Stack StickCPlus2
// The address and type of RTC will be auto-detected
#define RTC_SDA 21
#define RTC_SCL 22
// Declare ASCII names for each of the supported RTC types
const char *szType[] = {"Unknown", "PCF8563", "DS3231", "RV3032", "PCF85063A"};

extern "C" {
void app_main();
}
//
// Main esp-idf program entry point
//
void app_main(void)
{
int rc;
struct tm myTime;
int iSec, iTimeout;
    
    printf("bb_rtc set alarm example\n");
    printf("Initializes, then sets an alarm time.\nThe time will be confirmed with the internal timer.\n");
    rc = rtc.init(RTC_SDA, RTC_SCL);
    if (rc != RTC_SUCCESS) {
        printf("Error initializing the RTC; stopping...\n");
        while (1) {
            vTaskDelay(1);
        }
    }
    printf("RTC detected and initialized\n");
    printf("device type = %s\n", szType[rtc.getType()]);
    printf("Setting time to 12:34:50\n");
    memset(&myTime, 0, sizeof(myTime));
    myTime.tm_hour = 12;
    myTime.tm_min = 34;
    myTime.tm_sec = 50;
    rtc.setTime(&myTime);

    printf("Setting alarm for 12:35:00, the PCF8563 only\nallows resolution of 1 minute, so it will fire in 10 seconds.\n");
    rtc.clearAlarms();
    myTime.tm_min = 35;
    // The alarm will trigger a status change and the INT pin will be held low
    // when the clock reaches 35 minutes and 00 seconds (10 seconds from now)
    rtc.setAlarm(ALARM_TIME, &myTime);
    iTimeout = 0;
    iSec = -1;
    while (1) {
        rtc.getTime(&myTime);
        if (iSec != myTime.tm_sec) {
            // Show the time passing once per second
            printf("time: %02d:%02d:%02d\n", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
            iSec = myTime.tm_sec;
        }
        uint8_t u8Status = rtc.getStatus(); // poll the alarm flag
        if (u8Status & STATUS_IRQ1_TRIGGERED) {
            if (iTimeout >= 99 && iTimeout < 110) {
                printf("Alarm triggered at the correct time!\n");
            } else {
                printf("Alarm triggered at the wrong time! %d\n", iTimeout);
            }
            while (1) {
                vTaskDelay(1); // wait here forever
            }
        }
        if (iTimeout > 150) {
            printf("Alarm never triggered!\n");
            while (1) {
                vTaskDelay(1);
            }
        }
        iTimeout++;
        vTaskDelay(10); // about 100 milliseconds
    } // while (1)
} /* app_main() */
