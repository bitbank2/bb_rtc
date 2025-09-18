# BitBank Realtime Clock Library

Copyright (c) 2023-2025 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
My focus lately has been to make embedded programming as simple as possible (for myself and everyone else). One of my ideas to simplify project code is to create device libraries that work as a category instead of for a specific device. What this means in this case is that there are several popular I2C realtime clock chips that share a lot of common functionality. By combining the code for all of them into a single library with a simple and consistent API, the device-specific details can be managed without the user worrying about them. This library currently supports the DS3231, RV-3032, PCF85063A and PCF8563 RTC modules; the four most popular devices currently in use. Each has slightly different capabilities, but I was able to create a single API which covers all of the important features of each. This code is basically portable C++, but needs a bit of target-specific code for the I2C communication. It currently includes wrapper functions to allow it to work on Arduino, esp-idf and Linux. Examples are included for each system. To use it on other hardware (e.g. STM32), is simply a matter of writing wrapper functions for I2C init, test, read and write.

## Capabilities
The common capabilities of I2C realtime clock chips are the following:
- Keep the current time and date in 1 second increments
- Maintain accurate time over periods of weeks or months (< 30 parts per million error)
- Offer a digital square wave output signal which can generate different frequencies (shared with the interrupt pin on some)
- Set wakeup alarms as a countdown or at a specific time and date (not all support all options)
- Internal temperature sensor (which can be read) to reduce temperature induced frequency deviations (not all support this feature)

## Auto-detection...how?
I2C devices normally adhere to a de-facto standard of using a set of internal registers and having a fixed address. With this information, and sometimes a bit of probing of the register behavior, it is possible to automatically find and identify them. With bb_rtc, you only need to specify the GPIO pins used and if you will be using hardware I2C support or bit banging (provided by my BitBank_I2C libraru). With this information the library can identify and use your RTC. This feature also gives you the freedmom to change devices while developing your project without having to modify your software. If your RTC is not covered by this library, please let me know and I'll add support for it.

## The bb_rtc API
The library defines the BBRTC class which makes use of the POSIX 'struct tm' aka broken-out time structure. This allows you to specify the individual time and date fields as member variables. You also can use 32-bit epoch time (seconds since January 1, 1970). For a detailed look at the API, see the Wiki. The class  methods (overview):
- <b>init</b> Detect and turn on the RTC. If no parameters are passed, it assumes that the I2C bus has already been initialized by other code
- <b>getType</b> Returns the specific type of RTC (e.g. DS3231)
- <b>getStatus</b> Returns the current alarm and interrupt status
- <b>getBB</b> Returns a pointer to the internal I2C structure to share with other libraries
- <b>setBB</b> Allows setting the internal I2C structure
- <b>setFreq</b> Enable/disable and set the square wave output frequency
- <b>setVBackup</b> Enable or disable the built-in charge circuitry for backup batteries or capacitors
- <b>setAlarm</b> Set the type and time of an alarm
- <b>getTemp</b> Read the current ambient temperature
- <b>setTime</b> Set the current time and date from a tm structure
- <b>getTime</b> Get the current time and date into a tm structure
- <b>setCountdownAlarm</b> Set a countdown alarm in seconds
- <b>clearAlarms</b> Clear any pending alarm
- <b>getEpoch</b> Get the time as a 32-bit epoch value
- <b>setEpoch</b> Set the time as a 32-bit epoch value
- <b>stop</b> Stop the clock for low power standby
  
## Alarms and Interrupts
The interrupt pin (normally open-collector and used with a pull-up resistor) is enabled for the alarms and countdown timer functions. It's up to you to act on the changing state of the pin. When you set an alarm, the IRQ feature is enabled and when you disable an alarm, it's disabled. You can also read the status register to see if an alarm caused your MCU to awaken.<br>

The photo below shows the Arduino Nano 33 BLE example sketch running on my Nano+Feather breakout PCB (https://github.com/bitbank2/KiCad_Projects) <br>
<br>
![bb_rtc](/bb_rtc_demo.jpg?raw=true "bb_rtc")
<br>

If you find this code useful, please consider sending a donation or becoming a Github sponsor.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

