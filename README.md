--------------
# maybe online live chart at http://arduino.tk:8888/
--------------------
# super-graphing-data-logger
An arduino sketch for my old super graphing data logger project.

forked from https://github.com/evjrob/super-graphing-data-logger

--------------------------

luberth some changes

on start writes webpage from progmem to sd card /HC.htm (for RTC DS3231 version)

\<meta http-equiv="refresh" content="60"\> reload htm page every 60 seconds

progres bar on bottom of html page to show when page reloads

-

changed time server to time.google.com  because of jumps to 1970 when i log each second

time should not be read every time from time server

sometimes jumps to unixtime 1970 wich mess up the graph == not nice

fixed some compile errors for main ino

    // subtract seventy years: 
    unsigned long epoch = (secsSince1900 - seventyYears)+3600; //time offset 3600seconds for my location amsterdam europe 
    
    // return Unix time:
    
    return epoch;


---------------------



first chart maybe nice for https://github.com/ldijkman/Arduino-Drain-Rain-Irrigation-Measure-weight-system

pee measurement howmuch and when ;-)

<img src="https://github.com/ldijkman/super-graphing-data-logger/blob/master/images/first_chart.jpg" width="50%" heigth="50%">


--------------
# maybe online live chart at http://arduino.tk:8888/
--------------------
