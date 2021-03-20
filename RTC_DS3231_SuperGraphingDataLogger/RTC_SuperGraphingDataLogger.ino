

// my arduino mega maybe online at http://arduino.tk:8888/

// added i2c BME280 sensor for temp humid pressure
// NTP time  removed
// now i2c RTC DS3231 for time
// removed the eepromanything new log file each day


/* ************************************************************************
 * ***                    Super Graphing Data Logger                    ***
 * ************************************************************************
   Everett Robinson, December 2016. More at: http://everettsprojects.com

  forked and changes made by luberth dijkman
  http://Arduino.tk
  https://github.com/ldijkman/super-graphing-data-logger
  low cost pee measurement system
  https://github.com/ldijkman/Arduino-Drain-Rain-Irrigation-Measure-weight-system



   SDcard
   HC.htm        file in the root directory
   /data/       directory where the csv log files will be stored.

   This sketch combines the functionality of an existing fileserver example
   which can be found at http://www.ladyada.net/learn/arduino/ethfiles.html
   with the Datalogger example that comes with the new SD library from 1.0,


   serve up the datafiles in conjunction with a page which uses highcharts JS to graph it.

   highcharts.js files externally
   It should work just fine to have the highcharts.js file
   on the arduino's SD card,
   though loading the page will be painfully slow.

   Some of the code this was derived from may or may not be under a GPL
   licence; I'm not entirely sure. I suppose anyone using this should treat
   it like it is too
   Also if one intends to use this for commercial applications, it may be
   necessary to purchase a license for Highcharts.

*/

// webpage is overwritten to sdcard on sdcard on startup in setup
// webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage  webpage
const char Web_page[] PROGMEM = R"=====(

<!DOCTYPE HTML>
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
        <!--<meta http-equiv="refresh" content="60">-->
        <meta name="copyright" content="Copyright 2021 Dirk Luberth Dijkman Bangert 30 1619GJ Andijk The Netherlands" />
        <title>Super Graphing Data Logger! you are now inside my arduino mega 2560 with ethernet shield and SD-Card</title>

        <script src="https://code.jquery.com/jquery-3.1.1.min.js"
    integrity="sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8="
    crossorigin="anonymous"></script>
        <script type="text/javascript">
function getDataFilename(str){
    point = str.lastIndexOf("file=")+4;

    tempString = str.substring(point+1,str.length)
    if (tempString.indexOf("&") == -1){
    return(tempString);
    }
    else{
        return tempString.substring(0,tempString.indexOf("&"));
    }
        
}

query  = window.location.search;

var dataFilePath = "/data/"+getDataFilename(query);

$(function () {
    var chart;
    $(document).ready(function() {
    
        // define the options
        var options = {
    
            chart: {
                renderTo: 'container',
                zoomType: 'x',
                spacingRight: 20
            },
    
            title: {
                text: 'Arduino mega2560 logs light LDR BME280 temperature, humidity, milibar'
            },
    
            subtitle: {
                text: 'Click and drag in the plot area to zoom in'
            },
    
            xAxis: {
                type: 'datetime',
                maxZoom: 1 * 3600000
            },
    
            yAxis: {
                title: {
                    text: 'Light Levels (0 - 1024)'
                },
                min: 0,
                startOnTick: false,
                showFirstLabel: false
            },
    
            legend: {
                enabled: true
            },
    
            tooltip: {
                formatter: function() {
                        return '<b>'+this.y + ' ' + this.series.name +'</b><br/>'+
                        Highcharts.dateFormat('%H:%M:%S - %e %b %Y', this.x);
                }
                
            },
    
            plotOptions: {
             
                series: {
                    cursor: 'pointer',
                    lineWidth: 1.0,
                    
                    point: {
                        events: {
                            click: function() {
                                hs.htmlExpand(null, {
                                    pageOrigin: {
                                        x: this.pageX,
                                        y: this.pageY
                                    },
                                    headingText: this.series.name,
                                    maincontentText: Highcharts.dateFormat('%H:%M - %b %e, %Y', this.x) +':<br/> '+
                                        this.y,
                                    width: 200
                                });
                            }
                        }
                    },
                }
            },
    
            series: [{
             name: 'LDR Light Sensor',
             marker: {radius: 4}      
            },{
             name: '°C Temperature',
             marker: {radius: 4}
            },{
             name: '% Humidity',
             marker: {radius: 4}
            },{
             name: 'Milibar',
             marker: {radius: 4}
            }]
          
        };
    
    
        // Load data asynchronously using jQuery. On success, add the data
        // to the options and initiate the chart.
        // This data is obtained by exporting a GA custom report to TSV.
        // http://api.jquery.com/jQuery.get/
        jQuery.get(dataFilePath, null, function(csv, state, xhr) {
            var lines = [],
                date,
    
                // set up the two data series
                lightLevels = [];
                Temperature = [];
                Humidity = [];
                Pressure = [];
    
            // inconsistency
            if (typeof csv !== 'string') {
                csv = xhr.responseText;
            }
    
            // split the data return into lines and parse them
            csv = csv.split(/\n/g);
            jQuery.each(csv, function(i, line) {
    
                // all data lines start with a double quote
                line = line.split(',');
                date = parseInt(line[0], 10)*1000;
    
                lightLevels.push([
                    date,
                    parseInt(line[1], 10)
                ]);

               Temperature.push([
                   date,
                  parseFloat(line[2], 10)
               ]);

               Humidity.push([
                    date,
                    parseFloat(line[3], 10)
                ]);

                Pressure.push([
                   date,
                    parseFloat(line[4], 10)
                ]);
               
                
            });
    
            options.series[0].data = lightLevels;   
           options.series[1].data = Temperature;
           options.series[2].data = Humidity;
           options.series[3].data = Pressure;
         
           
            chart = new Highcharts.Chart(options);
        });
    });
    
});
        </script>
    </head>
    <body>
<center>ereyesterday<label id="ereyesterday"></label> Yesterday <label id="yesterday"></label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Today <label id="today"></label></center>
<script>
var today = new Date();
var yesterday = new Date((new Date()).valueOf() - 1000*60*60*24);
var ereyesterday = new Date((new Date()).valueOf() - 1000*60*60*24*2);
document.getElementById("ereyesterday").innerHTML = "<a href=\"HC.htm?file="+ereyesterday.getDate()+ "-"+(ereyesterday.getMonth()+1)+ "-"+(ereyesterday.getFullYear().toString().substr(2, 2))+ ".CSV\">"+ereyesterday.getDate()+ "-"+(ereyesterday.getMonth()+1)+ "-"+(ereyesterday.getFullYear().toString().substr(2, 2))+ ".CSV<a>";

document.getElementById("yesterday").innerHTML = "<a href=\"HC.htm?file="+yesterday.getDate()+ "-"+(yesterday.getMonth()+1)+ "-"+(yesterday.getFullYear().toString().substr(2, 2))+ ".CSV\">"+yesterday.getDate()+ "-"+(yesterday.getMonth()+1)+ "-"+(yesterday.getFullYear().toString().substr(2, 2))+ ".CSV<a>";

document.getElementById("today").innerHTML ="<a href=\"HC.htm?file="+today.getDate()+ "-"+(today.getMonth()+1)+ "-"+(today.getFullYear().toString().substr(2, 2))+ ".CSV\">"+today.getDate()+ "-"+(today.getMonth()+1)+ "-"+(today.getFullYear().toString().substr(2, 2))+ ".CSV<a>";
</script>
        <!--<p style="text-align:center;">Please allow the chart to load, it may take up to 30 seconds </p>-->
        <hr/>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highcharts/5.0.6/highcharts.js"></script>

<!-- Additional files for the Highslide popup effect -->
<script type="text/javascript" src="http://www.highcharts.com/highslide/highslide-full.min.js"></script>
<script type="text/javascript" src="http://www.highcharts.com/highslide/highslide.config.js" charset="utf-8"></script>
<link rel="stylesheet" type="text/css" href="http://www.highcharts.com/highslide/highslide.css" />

<div id="container" style="min-width: 400px; height: 400px; margin: 0 auto"><center><img src="https://media1.tenor.com/images/72b72ff0c392a16c6b12e80bbe3473c5/tenor.gif?itemid=4989541" width="25%" height="25%"><center></div>
 <script>
var timeleft = 60;
var downloadTimer = setInterval(function(){
timeleft--;
  document.getElementById("progressBar").value = 60-timeleft;
  document.getElementById("CountDown").innerHTML = (timeleft);
  if((timeleft)<=0){window.location.href = window.location.href;}
  var x = document.getElementById("myBtn");
  if (x.innerHTML == "Start AutoReload!") {
    timeleft=timeleft+1;
  }
if(timeleft <= 0) 
    clearInterval(downloadTimer);
},1000);
</script><center><label id="CountDown"></label>
<progress value="0" max="60" id="progressBar"style="height:12px;width:100%"> ></progress>
<button id="myBtn" value="myvalue" onclick="myFunction()">Stop AutoReload</button><center>


<script>
function myFunction() {
  var x = document.getElementById("myBtn");
  if (x.innerHTML == "Stop AutoReload") {
    x.innerHTML = "Start AutoReload!";
  } else {
    x.innerHTML = "Stop AutoReload";
  }
}
</script>
    </body>
</html>
)=====";

// end webpage  end webpage  end webpage  end webpage  end webpage  end webpage  end webpage  end webpage  end webpage  end webpage  end webpage  end webpage 



#include <SD.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <string.h>
#include <Time.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>                  // for watchdogtimer
#include <Wire.h>

#include "RTClib.h"                   // https://github.com/adafruit/RTClib

RTC_DS3231 rtc;

byte second_now;
byte last_second;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C


/************ ETHERNET STUFF ************/
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4C, 0x64 };
byte ip[] = { 192, 168, 178, 10 };

EthernetServer server(80);



/*** DATA LOGGER AND TIMER CONTROLS ****/
const int analogPin = 0;

unsigned long lastIntervalTime = 0; //The time the last measurement occured.
#define MEASURE_INTERVAL 30000     //30 seonds intervals between measurements (in ms)


// How big our line buffer should be for sending the files over the ethernet.
// 75 has worked fine for me so far.
#define BUFSIZ 75






// A function for easy printing of the headers
void HtmlHeaderOK(EthernetClient client) {

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");

}




// error 404 header function
void HtmlHeader404(EthernetClient client) {

  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<h2>File Not Found!</h2>");

}





//**************************************************************************************
void setup() {

  Serial.begin(115200);

  pinMode(10, OUTPUT);          // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);       // but turn off the W5100 chip!


  if (! rtc.begin()) {
    Serial.println("Could not find i2c RTC DS3231");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }



  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    Serial.println("SD Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("SD card initialized.");

  // The SD card is working, start the server and ethernet related stuff!

  Ethernet.begin(mac, ip);
  // Serial.println("Failed to configure Ethernet using DHCP");
  // don't do anything more:
  //return;

  Serial.println("DHCP configured!");
  Serial.print("IP-address: ");
  Serial.println(Ethernet.localIP());

  server.begin();
    
    
    unsigned status;
    
    // default settings
    status = bme.begin();  
    // You can also pass in a Wire library object like &Wire2
     status = bme.begin(0x76);
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
    }


#define PMTXT(textarray) (reinterpret_cast<const __FlashStringHelper*>(textarray))
 SD.remove("/HC.htm"); //delete html page we create a new one from progmem to sdcard
 File myFile;
myFile = SD.open("/HC.htm", FILE_WRITE);                  // if yes open it
      
      if (myFile) {                                                 // looks like println allready seeks end of file, where to append
        myFile.print( PMTXT(Web_page) );                                 // print string to sdcard log file
        myFile.close();
      }
//Serial.print( PMTXT(Web_page) );



wdt_enable(WDTO_8S);             // enable watchdogtimer if not reset in 8seconds reboot (reset in begin main loop)
  

}

// A function that takes care of the listing of files for the
// main page one sees when they first connect to the arduino.
// it only lists the files in the /data/ folder. Make sure this
// exists on your SD card.
void ListFiles(EthernetClient client) {

  File workingDir = SD.open("/data/");

  client.println("<ul>");

  while (true) {
    File entry =  workingDir.openNextFile();
    if (! entry) {
      break;
    }
    client.print("<li><a href=\"/HC.htm?file=");
    client.print(entry.name());
    client.print("\">Graphic ");
    client.print(entry.name());
    client.println("&nbsp;&nbsp;</a>");
    client.print("<a href=\"/data/");
    client.print(entry.name());
    client.print("\">Text ");
    client.print(entry.name());
    client.println("</a></li>");
    entry.close();
  }
  client.println("</ul>");
  workingDir.close();
  client.println("<br>This Page Served from an arduino mega ethernet sdcard, lightsensor LDR on analog A0, CSV log files saved on sdcard<br><br>maybe will switch hardware to 8 Euro WT32-ETH01 ESP32 with RJ45 Wired Ethernet<br>http://www.wireless-tag.com/portfolio/wt32-eth01/<br>");
  client.println("<br><a href=\"https://github.com/ldijkman/Arduino-Drain-Rain-Irrigation-Measure-weight-system\" target=\"new\">https://github.com/ldijkman/Arduino-Drain-Rain-Irrigation-Measure-weight-system</a><br>");
  client.println("<a href=\"https://github.com/ldijkman/Arduino_Plant_Watering_System\" target=\"new\">https://github.com/ldijkman/Arduino_Plant_Watering_System</a><br>");
  client.println("<br><a href=\"https://github.com/ldijkman/super-graphing-data-logger/tree/master/RTC_DS3231_SuperGraphingDataLogger\" target=\"new\">https://github.com/ldijkman/super-graphing-data-logger</a><br>");
  client.println("Changed NTP time to i2c RTC DS3231 time for standalone use if there is no internet<br>");
  client.println("removed the eepromanything == just a new log file each day<br> ");
  client.println("added BME280 sensor for temperature humidity millibar only vissible in csv log text data for now<br> ");
  client.print("<br><br>ip ");
  client.print(client.remoteIP());
  client.print("<br>Copyright 2021 Dirk Luberth Dijkman Bangert 30 1619GJ Andijk The Netherlands");
}














//***********************************************************************************
void loop() {

  wdt_reset();   // feed the Dog, kick the Dog  reset watchdog timer if not done reboots
  //while(1==1){Serial.println("watchdog test 8 seconds"); delay(500);}

  DateTime now = rtc.now();

  second_now = now.second();
  if (last_second != second_now) {       // only do this once each second

    last_second = second_now;

    // print unixtime
    //    Serial.print("rtc = ");
    //    Serial.println(now.unixtime());

    // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    // Serial.print(" ");
    // Serial.print(now.day(), DEC);
    // Serial.print("-");
    // Serial.print(now.month(), DEC);
    // Serial.print("-");
    // Serial.print(now.year(), DEC);
    // Serial.println(' ');
    // Serial.print(now.hour(), DEC);
    // Serial.print(':');
    // Serial.print(now.minute(), DEC);
    // Serial.print(':');
    // Serial.print(now.second(), DEC);
    // Serial.println();

    // Serial.print("Temperature: ");
    // Serial.print(rtc.getTemperature());
    // Serial.println(" C");
/*
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    */
  }  // end do this only once each second











  if ((millis() % lastIntervalTime) >= MEASURE_INTERVAL) { //Is it time for a new measurement?

    //char dataString[20] = "";
    int count = 0;
    unsigned long rawTime;
    rawTime = now.unixtime();           // getTime();


    File myFile;
    byte errorflag = 0;

    // filename must be 8.3 size CSV file 5 december 2021 makes 5-12-21.CSV
    String DateStampFile = "data/" + String(now.day()) + "-" + String(now.month()) + "-" + String(now.year() - 2000) + ".CSV";
    // String LogFileHeader = "time, dry1, wett1, dry2, wett2, sensor1, sensor2, averageinprocent, moisturestartprocent, starthour, endhour, temperature, jobcounter, maxjobs, wateringduration, pauzeduration, lastwateringtime, ValveStatus, watergifttimer, pauzetimer, outputread, errorflag,";

    if (SD.exists(DateStampFile)) {                                 // does the file exist on sdcard?
      // Serial.print("File exists. "); Serial.println(DateStampFile);

      myFile = SD.open(DateStampFile, FILE_WRITE);                  // if yes open it

      if (myFile) {                                                 // looks like println allready seeks end of file, where to append
        // myFile.println(dataString);                                 // print string to sdcard log file
         myFile.close();
        // Serial.println(dataString);                                 // print to the serial port too:
        // lcd.setCursor(9, 0);
        // lcd.print("SDok");
        // errorflag = 0;
      } else {
        Serial.print("# error opening ");  Serial.println(myFile);    // if the file isn't open, pop up an error:
        // lcd.setCursor(9, 0);
        // lcd.print("SD=X");
        // errorflag = 1;
      }

    }
    else
    {
      Serial.print(DateStampFile); Serial.println("# Does not exist.");
      Serial.print(DateStampFile); Serial.println("# Creating File.");
      myFile = SD.open(DateStampFile, FILE_WRITE);                  // create file with datestamp.txt MUST BE 8.3 SIZE
      //myFile.println(LogFileHeader);                                // print header to file for spreadsheet or chartmaker
      myFile.close();
      // lcd.setCursor(9, 0);
      // lcd.print("SD=X");
      // errorflag = 1;
    }












    //get the values and setup the string we want to write to the file
    int sensor = analogRead(analogPin);
 
    String dataString = "";
    dataString += String(rawTime);
    dataString += ",";
    dataString += String(sensor);
    dataString += ",";
    dataString += String(bme.readTemperature());
    dataString += ",";
    dataString += String(bme.readHumidity());
    dataString += ",";
    dataString += String((bme.readPressure() / 100.0F)); 
       
    //Serial.print("timeStr "); Serial.println(timeStr);
    //Serial.print("rawTime "); Serial.println(rawTime);
    //open the file we'll be writing to.
    File dataFile = SD.open(DateStampFile, FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {

      dataFile.println(dataString);

      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("Error opening datafile for writing");
    }

    //Update the time of the last measurment to the current timer value
    lastIntervalTime = millis();
  }








// do not understand next part yet
// think it could / should  be simpler eassier to understand // do not understand next part yet
// think it could / should  be simpler eassier to understand // do not understand next part yet
// think it could / should  be simpler eassier to understand // do not understand next part yet
// think it could / should  be simpler eassier to understand 


  char clientline[BUFSIZ];
  int index = 0;

  EthernetClient client = server.available();
  if (client) {
    Serial.print("visitors ip ");
    Serial.println(client.remoteIP());
    // an http request ends with a blank line
    boolean current_line_is_blank = true;

    // reset the input buffer
    index = 0;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c != '\n' && c != '\r') {                              // If it isn't a new line, add the character to the buffer
          clientline[index] = c;
          index++;
          if (index >= BUFSIZ)                                     // are we too big for the buffer? start tossing out data
            index = BUFSIZ - 1;          
          continue;                                                // continue to read more data!
        }

        clientline[index] = 0;                                     // got a \n or \r new line, which means the string is done

       
        Serial.println(clientline);                                      // Print it out for debugging
 
                                                                          // Look for substring such as a request to get the root file
        if (strstr(clientline, "GET / ") != 0) {                          // strstr = Find the first occurrence of a string
                                                                          // send a standard http response header
          HtmlHeaderOK(client);
                                                                          // print all the data files, use a helper to keep it clean
          client.println("<h2>View data for the week of (dd-mm-yy):</h2>");
          ListFiles(client);
        }
        else if (strstr(clientline, "GET /") != 0) {                    // strstr = Find the first occurrence of a string
                                                                        // this time no space after the /, so a sub-file!
          char *filename;

          filename = strtok(clientline + 5, "?");                       // strok = split str into tokens
                                                                        // look after the "GET /" (5 chars) but before
                                                                        // the "?" if a data file has been specified. A little trick, look for the " HTTP/1.1"
                                                                        // string and turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;

          
          Serial.println(filename);                                     // print the file we want
          File file = SD.open(filename, FILE_READ);
          if (!file) {
            HtmlHeader404(client);
            break;
          }

          Serial.println("Opened!");

          HtmlHeaderOK(client);

          while (file.available()) {
            int num_bytes_read;
            uint8_t byte_buffer[64];

            num_bytes_read = file.read(byte_buffer, 64);
            client.write(byte_buffer, num_bytes_read);
          }
          //client.println("<h2>View data for the week of (dd-mm-yy):</h2>");

          file.close();
          ListFiles(client);

        }
        else {
         
          HtmlHeader404(client);                                               // everything else is a 404
        }
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    client.stop();
  }

}
