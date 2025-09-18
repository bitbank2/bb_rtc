//
// RTC set alarm example
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <bb_rtc.h>

const char *szRTCType[] = {"None", "PCF8563", "DS3231", "RV-3032", "PCF85063A"};

BBRTC rtc;

int main(int argc, char *argv[])
{
int i;
struct tm *thetime, myTime;
time_t tt;
int iSec, iTimeout;

    printf("bb_rtc set alarm example\n");
    printf("Initializes, then sets an alarm time.\nThe time will be confirmed with the internal timer.\n");
	// I2C bus 1 is the default on RPI hardware
        // Other Linux systems can use any number from 0 to 10 (usually)
        i = rtc.init(4); // find a supported RTC
        if (i != RTC_SUCCESS) {
            printf("NB: by default your system may require root access for I2C\n");
	    printf("No supported RTC found\n");
            return -1; // problem - quit
        } else {
            printf("RTC detected and initialized\n");
            printf("device type = %s\n", szRTCType[rtc.getType()]);
	}
	printf("Setting the RTC time to the Linux system time\n");
        tt = time(NULL);  // get the current system time
        thetime = localtime(&tt);
        rtc.setTime(thetime); // set the current time
        printf("RTC time = %02d:%02d:%02d\n", thetime->tm_hour, thetime->tm_min, thetime->tm_sec);
        printf("RTC date = %02d/%02d/%04d\n", thetime->tm_mon+1, thetime->tm_mday, thetime->tm_year + 1900);
 
    printf("Setting alarm for 10 seconds from now...\n");
    rtc.clearAlarms();
    // The alarm will trigger a status change and the INT pin will be held low
    if (rtc.getType() == RTC_RV3032) { // RV3032 doesn't support direct time alarms with seconds resolution; use a countdown timer instead
        rtc.setCountdownAlarm(10);
    } else {
        tt += 10;
        thetime = localtime(&tt);
        rtc.setAlarm(ALARM_TIME, thetime);
    }
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
            if (iTimeout >= 90 && iTimeout < 110) {
                printf("Alarm triggered at the correct time!\n");
            } else {
                printf("Alarm triggered at the wrong time! %d\n", iTimeout);
            }
	    return -1;
        }
        if (iTimeout > 150) {
            printf("Alarm never triggered!\n");
            return -1;
        }
        iTimeout++;
        usleep(100000); // about 100 milliseconds
    } // while (1)

return 0;
} /* main() */
