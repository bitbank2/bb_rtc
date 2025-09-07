#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <bb_rtc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

BBRTC rtc;
// These are the GPIO pins on the M5Stack M5StickC PLUS2
// The address and type of RTC will be auto-detected
#define SDA_PIN 21
#define SCL_PIN 22
// Declare ASCII names for each of the supported RTC types
const char *szType[] = {"Unknown", "PCF8563", "DS3231", "RV3032", "PCF85063A"};

extern "C" {
void app_main();
}

void app_main(void)
{
int rc;
struct tm myTime;

    printf("bb_rtc basic usage example\n");
    printf("Initializes, then sets and reads the time.\n");
    rc = rtc.init(SDA_PIN, SCL_PIN);
    if (rc != RTC_SUCCESS) {
        printf("Error initializing the RTC; stopping...\n");
        while (1) {
            vTaskDelay(1);
        }
    }
    printf("RTC detected and initialized\n");
    printf("device type = %s\n", szType[rtc.getType()]);
    printf("Setting time to 12:34:56\n");
    myTime.tm_hour = 12;
    myTime.tm_min = 34;
    myTime.tm_sec = 56;
    rtc.setTime(&myTime);

    while (1) { // show the time forever...
        rtc.getTime(&myTime);
        // Print backspace characters to keep the time from scrolling
        // in the serial terminal
        printf("\b\b\b\b\b\b\b\b%02d:%02d:%02d", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
        vTaskDelay(10); // update about once a second
    }
} /* app_main() */
