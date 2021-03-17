# arduino mega maybe online http://arduino.tk:8888/

ntp time removed

i2c RTC DS3231 for time now

for standalone use if there is no internet connection

 removed the eeprom part eeprom anything == just create daily a new csv file

on start deletes /HC.htm webpage from SDcard

on start writes creates webpage from progmem to sd card /HC.htm (for RTC DS3231 version)

Easier for me to make changes to html page

and if someone inserts a card with no htm file it gets created
