BitBank Realtime Clock Library<br>
------------------------------
Copyright (c) 2023 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
I've been working with several different I2C RTC modules and decided to create a single library which can auto-detect them and have a simple and consistent API.<br>
This library currently supports the DS3231, RV-3032 and PCF8563 RTC modules. Each has slightly different capabilities, but I was able to create a simple API which covers all of the important features of each.<br>

The interrupt pin (usually open-collector with pull-up resistor) is enabled for the alarms and countdown timer functions. It's up to you to act on the changing state of the pin.<br>


