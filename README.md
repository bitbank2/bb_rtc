BitBank Realtime Clock Library<br>
------------------------------
Copyright (c) 2023 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
My focus lately has been to make embedded programming as simple as possible (for myself and everyone else). One of my ideas to simplify project code is to create device libraries that work as a category instead of for a specific device. What this means in this case is that there are several popular I2C realtime clock chips that share a lot of common functionality. By combining the code for all of them into a single library with a simple and consistent API, the device-specific details can be managed without the user worrying about them. This library currently supports the DS3231, RV-3032 and PCF8563 RTC modules; the three most popular devices currently in use. Each has slightly different capabilities, but I was able to create a simple API which covers all of the important features of each.<br>

The interrupt pin (usually open-collector with pull-up resistor) is enabled for the alarms and countdown timer functions. It's up to you to act on the changing state of the pin.<br>

The photo below shows the Arduino Nano 33 BLE example sketch running on my Nano+Feather breakout PCB (https://github.com/bitbank2/KiCad_Projects) <br>
<br>
![bb_rtc](/bb_rtc_demo.jpg?raw=true "bb_rtc")
<br>

If you find this code useful, please consider sending a donation or becoming a Github sponsor.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

