# arduino mega maybe online http://arduino.tk:8888/

my version == single file 

added i2c BME280 sensor for temperature humidity millibar only vissible in csv log text data for now

added watchdog 8 seconds

#include <avr/wdt.h>                  // for watchdogtimer

wdt_enable(WDTO_8S);             // enable watchdogtimer if not reset in 8seconds reboot (reset in begin main loop)

wdt_reset();   // feed the Dog, kick the Dog  reset watchdog timer if not done reboots

//while(1==1){Serial.println("watchdog test 8 seconds"); delay(500);}

ntp time removed

i2c RTC DS3231 for time now

for standalone use if there is no internet connection

 removed the eeprom part eeprom anything == just create daily a new csv file

on start deletes /HC.htm webpage from SDcard

on start writes creates webpage from progmem to sd card /HC.htm (for RTC DS3231 version)

Easier for me to make changes to html page

and if someone inserts a card with no HC.htm file it gets created
