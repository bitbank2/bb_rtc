#include <bb_rtc.h>
#include <WiFi.h>
#include <NTPClient.h>           //https://github.com/taranais/NTPClient
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "esp_wifi.h"
static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, "pool.ntp.org");
struct tm myTime;

BBRTC rtc;
const char *szRTCType[] = {"None", "PCF8563", "DS3231", "RV-3032"};
const char* ssid     = "your_ssid";
const char* password = "your_password";

//
// This function uses the ipapi.co website to convert
// a public IP address into a time zone offset (HHMM)
// It returns the offset in seconds from GMT
//
int GetTimeOffset(char *szIP)
{
  HTTPClient http;
  int httpCode = -1;
  char szTemp[256];

  //format -> https://ipapi.co/<your public ip>/utc_offset/
  sprintf(szTemp, "https://ipapi.co/%s/utc_offset/", szIP);
  http.begin(szTemp);
  httpCode = http.GET();  //send GET request
  if (httpCode != 200) {
     http.end();
     return -1;
  } else {
     const char *s;
     int i;
     String payload = http.getString();
     http.end();
     s = payload.c_str();
    // Get the raw HTTP response text (+HHMM)
    // and convert the time zone offset (HHMM) into seconds
    Serial.print("TZ offset ");
    Serial.println(s);
    i = ((s[1]-'0') * 10) + (s[2]-'0'); // hour
    i *= 60;
    i += ((s[3]-'0') * 10) + (s[4]-'0'); // minute
    if (s[0] == '-')
      i = -i; // negative offset
    return (i*60); // return seconds
  } // if successfully connected
  return -1;
} /* GetTimeOffset() */
//
// Get our external IP from ipify.org
// Copy it into the given string variable
// in the form (a.b.c.d)
// Returns true for success
//
bool GetExternalIP(char *szIP)
{
  WiFiClient client;

  if (!client.connect("api.ipify.org", 80)) {
    Serial.println("api.ipify.org failed!");
    return false;
  }
  else {
    int timeout = millis() + 5000;
    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println("Client Timeout!");
        client.stop();
        return false;
      }
    }
    // Get the raw HTTP+JSON response text
    // and parse out just the IP address
    int i, j, size, offset = 0;
    char szTemp[256];
    while ((size = client.available()) > 0) {
      if (size+offset > 256) size = 256-offset;
      size = client.read((uint8_t *)&szTemp[offset], size);
      offset += size;
    } // while data left to read

    // parse the IP address we want
    for (i=0; i<offset; i++) {
      if (memcmp(&szTemp[i],"{\"ip\":\"", 7) == 0) {
        for (j=i+7; j<offset && szTemp[j] != '\"'; j++) {
          szIP[j-(i+7)] = szTemp[j];
        } // for j
        szIP[j-(i+7)] = 0; // zero terminate it
        return true;
      } // if found start of IP
    } // for i
  } // if successfully connected
  return false;
} /* GetExternalIP() */

void setup() {
  int i, iTimeout;
  char szIP[32], szTemp[128];

  Serial.begin(115200);
  delay(3000); // wait for CDC serial to start
  Serial.println("Starting...");
  i = rtc.init(21, 22);
  if (i == RTC_SUCCESS) {
    Serial.println("Success");
    Serial.printf("RTC type = %s\n", szRTCType[rtc.getType()]);
  } else {
    Serial.println("Failure, stopping...");
    while (1) {};
  }
  rtc.getTime(&myTime);
  if (myTime.tm_year > 122) {
    Serial.println("Already has the correct date/time");
    return;
  }
  Serial.println("Connect to WiFi");
  WiFi.begin(ssid, password);
  iTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && iTimeout < 20) {
    delay(500);
    iTimeout++;
    Serial.print(".");
   }
if (WiFi.status() == WL_CONNECTED) {
   Serial.println("\nCONNECTED!");
} else {
   Serial.println("\nFailed!");
   Serial.println("Press reset to try again");
   while (1) {};
}
  if (GetExternalIP(szIP)) {
    int iTimeOffset; // offset in seconds
    Serial.println("My IP:");
    Serial.println(szIP);
    // Get our time zone offset (including daylight saving time)
    iTimeOffset = GetTimeOffset(szIP);
    if (iTimeOffset != -1) {
    // Initialize a NTPClient to get time
      timeClient.begin();
      timeClient.setTimeOffset(iTimeOffset);
      timeClient.update();
      unsigned long epochTime = timeClient.getEpochTime();
  //Get a time structure
      struct tm *ptm = gmtime ((time_t *)&epochTime);
      rtc.setTime(ptm); // set it into our RTC chip
      timeClient.end(); // don't need it any more
    } else {
      Serial.println("Time Zone info failed");
    }
  }
  WiFi.disconnect(true); // disconnect and turn off the WiFi radio

} /* setup() */

void loop() {
  // put your main code here, to run repeatedly:
  rtc.getTime(&myTime);
  Serial.printf("%02d:%02d:%02d\n", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
  delay(1000);
}
