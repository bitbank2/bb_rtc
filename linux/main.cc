//
// RTC + EEPROM test app
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "../src/bb_rtc.h"

const char *szRTCType[] = {"None", "PCF8563", "DS3231", "RV-3032"};

void ShowHelp(void)
{
	printf("getset_time - gets or sets the time of a attached RTC\n");
	printf("written by Larry Bank\n\n");
	printf("Usage:\n");
	printf("getset_time set - sets the RTC time and date to the system date\n");
	printf("getset_time get - displays the RTC time and date\n");
} /* ShowHelp() */

BBRTC rtc;

int main(int argc, char *argv[])
{
int i;
struct tm *thetime;
time_t tt;

	if (argc != 2)
	{
		ShowHelp();
		return 0;
	}
	        // I2C bus 1 is the default on RPI hardware
        // most other Linux systems expose I2C on bus 0
        i = rtc.init(1, 0); // find a supported RTC
        if (i != RTC_SUCCESS) {
		printf("No supported RTC found\n");
                return -1; // problem - quit
        } else {
           printf("Found RTC, type = %s\n", szRTCType[rtc.getType()]);
	}
        tt = time(NULL);  // get the current system time
        thetime = localtime(&tt);

        if (strcmp(argv[1], "get") == 0) // display RTC time
        {
                rtc.getTime(thetime);
                printf("RTC time = %02d:%02d:%02d\n", thetime->tm_hour, thetime->tm_min, thetime->tm_sec);
                printf("RTC date = %02d/%02d/%04d\n", thetime->tm_mon+1, thetime->tm_mday, thetime->tm_year + 1900);
        }
        else if (strcmp(argv[1], "set") == 0) // set RTC to system time
        {
                rtc.setTime(thetime); // set the current time
                printf("RTC time set to system time\n");
        }
        else
        {
                ShowHelp();
                return 0;
        }

return 0;
} /* main() */
