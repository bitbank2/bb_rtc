//
// Set the time of an I2C Realtime Clock (RTC) chip using 2 buttons and an OLED display
// written by Larry Bank (bitbank@pobox.com)
// Project started 1/12/2025
//
// This project requires one of the RTC chips supported by bb_rtc as well as 2 pushbuttons
// and a SSD1306 128x64 OLED
// The purpose is to allow you to easily set the time and date manually using 2 buttons
// The first button selects which field to change and the second button changes the field's value
// Holding the second button will cause it to repeat at a rate of about 4 values per second
//
#include <OneBitDisplay.h>
#include <bb_rtc.h>
BBRTC rtc;
ONE_BIT_DISPLAY oled;
int iField = 0; // currently selected field/digit
// Number of ticks before a held button starts repeating (about 1 second)
#define REPEAT_DURATION 32
// These are for Martin Fasani's ESP32-C6 epaper PCB
#define SDA_PIN 7
#define SCL_PIN 6
#define BUTTON0 18
#define BUTTON1 19
struct tm myTime;
const char *szDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *szMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                          "Aug", "Sep", "Oct", "Nov", "Dec"};
// Enumeration of the field names that the user can change
enum {
  DAY_FIELD = 0,
  MONTH_FIELD,
  DATE_FIELD,
  YEAR_FIELD,
  HOUR_FIELD,
  MINUTE_FIELD,
  SECOND_FIELD
};
//
// Returns the current button states as the lower 2 bits of a uint8_t
// 1 = pressed, 0 = not pressed (opposite of the GPIO state)
//
uint8_t GetButtons(void)
{
uint8_t u8 = 0;
  if (digitalRead(BUTTON0) == 0) u8 |= 1;
  if (digitalRead(BUTTON1) == 0) u8 |= 2;
  return u8;
} /* GetButtons() */
//
// Display the current time and date in an attractive format
//
void ShowTime(void)
{
  char szTemp[16];

  rtc.getTime(&myTime); // get the current time
  oled.setFont(FONT_8x8);
  oled.setCursor(0,0);
  oled.setTextColor((iField == DAY_FIELD) ? OBD_WHITE : OBD_BLACK);
  oled.print(szDays[myTime.tm_wday]); // day of the week
  oled.setTextColor(OBD_BLACK);
  oled.print(" ");
  oled.setTextColor((iField == MONTH_FIELD) ? OBD_WHITE : OBD_BLACK);
  oled.print(szMonths[myTime.tm_mon - 1]); // month
  oled.setTextColor(OBD_BLACK);
  oled.print(" ");
  oled.setTextColor((iField == DATE_FIELD) ? OBD_WHITE : OBD_BLACK);
  oled.print(myTime.tm_mday, DEC);
  oled.setTextColor(OBD_BLACK);
  oled.println("    "); // erase any old info as the length changes
  oled.setTextColor((iField == YEAR_FIELD) ? OBD_WHITE : OBD_BLACK);
  sprintf(szTemp, "%04d\n", myTime.tm_year + 1900);
  oled.println(szTemp);
  // use a larger font to display the time
  oled.setFont(FONT_12x16);
  oled.setTextColor((iField == HOUR_FIELD) ? OBD_WHITE : OBD_BLACK);
  sprintf(szTemp, "%02d", myTime.tm_hour);
  oled.print(szTemp);
  oled.setTextColor(OBD_BLACK);
  oled.print(":");
  oled.setTextColor((iField == MINUTE_FIELD) ? OBD_WHITE : OBD_BLACK);
  sprintf(szTemp, "%02d", myTime.tm_min);
  oled.print(szTemp);
  oled.setTextColor(OBD_BLACK);
  oled.print(":");
  oled.setTextColor((iField == SECOND_FIELD) ? OBD_WHITE : OBD_BLACK);
  sprintf(szTemp, "%02d", myTime.tm_sec);
  oled.print(szTemp);
} /* ShowTime() */

void setup() {
//  Serial.begin(115200);
  oled.setI2CPins(SDA_PIN, SCL_PIN);
  oled.I2Cbegin(OLED_128x64); // common SSD1306 128x64 1-bit display
  rtc.init(SDA_PIN, SCL_PIN);
  rtc.setVBackup(true); // Allow trickle charge of supercapacitor
  oled.fillScreen(OBD_WHITE);
  pinMode(BUTTON0, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);
} /* setup() */

void loop() {
  int iDelta; // indicates if the value should be changed during this loop
  int iCount = 0; // loop count
  int iRepeat; // used for knowing if a button is being held down
  uint8_t iButtons, iOldButtons = 0; // previous button state
  bool bChanged = true; // boolean indicating that the display needs to be updated

  while (1) { // loop forever
    iButtons = GetButtons();
    if (iButtons != 0) { // manage button presses
      if ((iButtons & 1) && !(iOldButtons & 1)) { // button 0 newly pressed, change field
        iField++;
        bChanged = true;
        if (iField > SECOND_FIELD) iField = DAY_FIELD; // only 7 unique fields, wrap around
      } // button 0 pressed
      if (iButtons & 2) { // button 1 is being pressed, change value
          iDelta = 0; // assume no change needed in value
          if (!(iOldButtons & 2)) { // button 1 was just pressed, start repeat timer
              iRepeat = iCount; // mark the time to know when it was first pressed
              iDelta = 1; // first press, change the value
          } else if ((iCount - iRepeat) >= REPEAT_DURATION && (iCount & 0x7) == 0) {
            // pressed long enough to start repeating?
              iDelta = 1;
          }
          if (iDelta) { // we need to change the field value
            switch (iField) { // each field behaves slightly differently
                case DAY_FIELD:
                   myTime.tm_mday++;
                   if (myTime.tm_mday > 6) myTime.tm_mday = 0; // wrap around to Sunday
                   break;
                case MONTH_FIELD:
                   myTime.tm_mon++;
                   if (myTime.tm_mon > 12) myTime.tm_mon = 1; // wrap around to January
                   break;
                case DATE_FIELD:
                   myTime.tm_mday++;
                   if (myTime.tm_mday > 31) myTime.tm_mday = 1;
                   break;
                case YEAR_FIELD:
                   myTime.tm_year++;
                   if (myTime.tm_year > 199) myTime.tm_year = 125; // don't allow going back before 2025 
                   break;
                case HOUR_FIELD:
                   myTime.tm_hour++;
                   if (myTime.tm_hour > 23) myTime.tm_hour = 0;
                   break;
                case MINUTE_FIELD:
                   myTime.tm_min++;
                   if (myTime.tm_min > 59) myTime.tm_min = 0;
                   break;
                case SECOND_FIELD:
                   myTime.tm_sec++;
                   if (myTime.tm_sec > 59) myTime.tm_sec = 0;
                   break;
            } // switch on field
            rtc.setTime(&myTime); // set the new time
            bChanged = true;
          }
      } // button 1 pressed
    } // a button was/is being pressed
    iOldButtons = iButtons; // new->old
    if (bChanged || (iCount & 0x1f) == 0) { // Update the time every second or if the user changed it
        ShowTime();
        bChanged = false;
    }
    delay(31); // about 32 ticks per second, and an easy way to debounce the them too :)
    iCount++;
  } // while (1)
} /* loop() */
