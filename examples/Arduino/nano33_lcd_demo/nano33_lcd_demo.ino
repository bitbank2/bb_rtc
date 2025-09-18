//
// Example of displaying the current time
// on a color LCD with the bb_rtc library
// in combination with the bb_spi_lcd library
// written by Larry Bank
//

#include <bb_spi_lcd.h>
#include <bb_rtc.h>
BBRTC rtc;
BB_SPI_LCD lcd;

//
// These GPIO pin definitions are for my Nano+Feather LCD breakout board (https://github.com/bitbank2/KiCad_Projects)
// Check your pin numbers; they will probably be different
//
#define CS_PIN 4
#define RST_PIN 2
#define LCD_PIN 5
#define DC_PIN 3
//
// Defining these values as -1 tells the bb_spi_lcd library
// to use the default pins defined in the Arduino variants header file
//
#define MISO_PIN -1
#define MOSI_PIN -1
#define CLK_PIN -1

void setup() {
  rtc.init(); // initialize the RTC that's found (amongst 3 supported) on the default I2C pins for this target board

  //    int begin(int iType, int iFlags, int iFreq, int iCSPin, int iDCPin, int iResetPin, int iLEDPin, int iMISOPin, int iMOSIPin, int iCLKPin);
  // For this demo I used the 'blue' PCB version of a ST7735 128x128 1.44" LCD
  lcd.begin(LCD_ST7735_128, FLAGS_NONE, 8000000, CS_PIN, DC_PIN, RST_PIN, LCD_PIN, MISO_PIN, MOSI_PIN, CLK_PIN);
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(FONT_12x16);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.println("Nano 33");
  lcd.println("LCD+RTC");
  lcd.println("test rig");
  lcd.setTextColor(TFT_BLUE, TFT_BLACK);
  lcd.setFont(FONT_8x8);
  lcd.println("Look ma,");
  lcd.println("no soldering!");

}

void loop() {
struct tm myTime;
char szTemp[32];

  rtc.getTime(&myTime); // Read the current time from the RTC into our time structure
  sprintf(szTemp, "%02d:%02d:%02d", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
  lcd.setCursor(0,96);
  lcd.setFont(FONT_12x16);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.print(szTemp);
  delay(1000); // update the time once every second
}
