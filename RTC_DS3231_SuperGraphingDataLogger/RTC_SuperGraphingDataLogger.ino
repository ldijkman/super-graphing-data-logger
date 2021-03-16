
// NTP time  removed
// now RTC DS3231 for time
// removed the eepromanything new log file each day


/* ************************************************************************
 * ***                    Super Graphing Data Logger                    ***
 * ************************************************************************
   Everett Robinson, December 2016. More at: http://everettsprojects.com

   This sketch relies on the SD and ethernet libraries in arduino 1.0 or newer.
   The following extra non standard libraries were also used, and will need to
   be added to the libraries folder:
   - Time: http://playground.arduino.cc/Code/Time
   



   You must also ensure that you have the HC.htm file in the root directory
   of your SD card, as well as a data directory where the datafiles will be
   stored.

   This sketch combines the functionality of an existing fileserver example
   which can be found at http://www.ladyada.net/learn/arduino/ethfiles.html
   with the Datalogger example that comes with the new SD library from 1.0,
   as well as some code from the UdpNtpClient example that cones with the
   ethernet library.

   Added to all of these are some tricks to make it manage and serve up the
   datafiles in conjunction with a page which uses highcharts JS to graph it.
   This is basically accomplished using the arduino by itself. Because I
   actually host the highcharts.js files externally, this is true more in
   theory than in actual practice, but oh well. It should work just fine to
   have the highcharts.js file on the arduino's SD card, though loading the
   page will be painfully slow.

   Some of the code this was derived from may or may not be under a GPL
   licence; I'm not entirely sure. I suppose anyone using this should treat
   it like it is too, but I don't really care too much.
   Also if one intends to use this for commercial applications, it may be
   necessary to purchase a license for Highcharts.

*/

#include <SD.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <string.h>
#include <Time.h>
#include <avr/pgmspace.h>

#include "RTClib.h"                   // https://github.com/adafruit/RTClib

RTC_DS3231 rtc;

byte second_now;
byte last_second;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};



/************ ETHERNET STUFF ************/
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4C, 0x64 };
byte ip[] = { 192, 168, 178, 10 };
EthernetServer server(80);



/*** DATA LOGGER AND TIMER CONTROLS ****/
const int analogPin = 0;
unsigned long lastIntervalTime = 0; //The time the last measurement occured.
#define MEASURE_INTERVAL 10000     //10 minute intervals between measurements (in ms)
unsigned long newFileTime;          //The time at which we should create a new week's file
#define FILE_INTERVAL 86400        //One day worth of seconds

// How big our line buffer should be for sending the files over the ethernet.
// 75 has worked fine for me so far.
#define BUFSIZ 75



//A structure that stores file config variables from EEPROM
typedef struct {
  unsigned long newFileTime;      //Keeps track of when a newfile should be made.
  char workingFilename[19];       //The path and filename of the current week's file
} configuration;

configuration config;               //Actually make our config struct


// Strings stored in flash mem for the Html Header (saves ram)
const char HeaderOK_0[] PROGMEM = "HTTP/1.1 200 OK";            //
const char HeaderOK_1[] PROGMEM = "Content-Type: text/html";    //
const char HeaderOK_2[] PROGMEM = "";                           //

// A table of pointers to the flash memory strings for the header
const char* const HeaderOK_table[] PROGMEM = {
  HeaderOK_0,
  HeaderOK_1,
  HeaderOK_2
};

// A function for reasy printing of the headers
void HtmlHeaderOK(EthernetClient client) {

  char buffer[30]; //A character array to hold the strings from the flash mem

  for (int i = 0; i < 3; i++) {
    strcpy_P(buffer, (char*)pgm_read_word(&(HeaderOK_table[i])));
    client.println( buffer );
  }
}


// Strings stored in flash mem for the Html 404 Header
const char Header404_0[] PROGMEM = "HTTP/1.1 404 Not Found";     //
const char Header404_1[] PROGMEM = "Content-Type: text/html";    //
const char Header404_2[] PROGMEM = "";                           //
const char Header404_3[] PROGMEM = "<h2>File Not Found!</h2>";

//const char HeaderOK_0[] PROGMEM = "HTTP/1.1 200 OK";


// A table of pointers to the flash memory strings for the header
const char* const Header404_table[] PROGMEM = {
  Header404_0,
  Header404_1,
  Header404_2,
  Header404_3
};

// Easy peasy 404 header function
void HtmlHeader404(EthernetClient client) {

  char buffer[30]; //A character array to hold the strings from the flash mem

  for (int i = 0; i < 4; i++) {
    strcpy_P(buffer, (char*)pgm_read_word(&(Header404_table[i])));
    client.println( buffer );
  }
}





//**************************************************************************************
void setup() {
  Serial.begin(115200);

  pinMode(10, OUTPUT);          // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);       // but turn off the W5100 chip!


  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }



  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // The SD card is working, start the server and ethernet related stuff!
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // don't do anything more:
    return;
  }
  Serial.println("DHCP configured!");
  Serial.print("IP-address: ");
  Serial.println(Ethernet.localIP());

  server.begin();

 
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
    client.print("\">Graphic View ");
    client.print(entry.name());
    client.println(" ------ </a>");
    client.print("<a href=\"/data/");
    client.print(entry.name());
    client.print("\">Text CSV File ");
    client.print(entry.name());
    client.println("</a></li>");
    entry.close();
  }
  client.println("</ul>");
  workingDir.close();
  client.println("<br>This Page Served from an arduino mega ethernet sdcard, lightsensor LDR on analog A0, CSV log files saved on sdcard<br>maybe will switch hardware to 8 Euro WT32-ETH01 ESP32 with RJ45 Wired Ethernet<br>");
  client.println("<br><a href=\"https://github.com/ldijkman/Arduino-Drain-Rain-Irrigation-Measure-weight-system\" target=\"new\">https://github.com/ldijkman/Arduino-Drain-Rain-Irrigation-Measure-weight-system</a><br>");
  client.println("<a href=\"https://github.com/ldijkman/Arduino_Plant_Watering_System\" target=\"new\">https://github.com/ldijkman/Arduino_Plant_Watering_System</a><br>");
  client.println("<a href=\"https://github.com/ldijkman/super-graphing-data-logger\" target=\"new\">https://github.com/ldijkman/super-graphing-data-logger</a><br>");
  client.println("Changed NTP time to i2c RTC DS3231 time for standalone use if there is no internet<br> ");
  client.println("");
}














//***********************************************************************************
void loop() {

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


  }  // end do this only once each second











  if ((millis() % lastIntervalTime) >= MEASURE_INTERVAL) { //Is it time for a new measurement?

    char dataString[20] = "";
    int count = 0;
    unsigned long rawTime;
    rawTime = now.unixtime();           // getTime();


 File myFile;
  byte errorflag = 0;

  // filename must be 8.3 size CSV file 5 december 2021 makes 5_12_21.CSV
  String DateStampFile = "data/" + String(now.day()) + "-" + String(now.month()) + "-" + String(now.year() - 2000) + ".CSV";
  //String LogFileHeader = "time, dry1, wett1, dry2, wett2, sensor1, sensor2, averageinprocent, moisturestartprocent, starthour, endhour, temperature, jobcounter, maxjobs, wateringduration, pauzeduration, lastwateringtime, ValveStatus, watergifttimer, pauzetimer, outputread, errorflag,";
  // must be a units header here?, but cannot find info about that

  if (SD.exists(DateStampFile)) {                                 // does the file exist on sdcard?
   // Serial.print("File exists. "); Serial.println(DateStampFile);

    myFile = SD.open(DateStampFile, FILE_WRITE);                  // if yes open it

    if (myFile) {                                                 // looks like println allready seeks end of file, where to append
      //myFile.println(dataString);                                 // print string to sdcard log file
      // myFile.close();
      // Serial.println(dataString);                                 // print to the serial port too:
      //lcd.setCursor(9, 0);
      // lcd.print("SDok");
      // errorflag = 0;
    } else {
      Serial.print("# error opening ");  Serial.println(myFile);    // if the file isn't open, pop up an error:
      //lcd.setCursor(9, 0);
      //lcd.print("SD=X");
      //  errorflag = 1;
    }

  }
  else
  {
    Serial.print(DateStampFile); Serial.println("# Does not exist.");
    Serial.print(DateStampFile); Serial.println("# Creating File.");
    myFile = SD.open(DateStampFile, FILE_WRITE);                  // create file with datestamp.txt MUST BE 8.3 SIZE
    //myFile.println(LogFileHeader);                                // print header to file for spreadsheet or chartmaker
    myFile.close();
    //lcd.setCursor(9, 0);
    //lcd.print("SD=X");
    // /errorflag = 1;
  }












    //get the values and setup the string we want to write to the file
    int sensor = analogRead(analogPin);
    char timeStr[12];
    char sensorStr[6];

    ultoa(rawTime, timeStr, 10);
    itoa(sensor, sensorStr, 10);

    strcat(dataString, timeStr);
    strcat(dataString, ",");
    strcat(dataString, sensorStr);
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



 






  char clientline[BUFSIZ];
  int index = 0;

  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean current_line_is_blank = true;

    // reset the input buffer
    index = 0;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZ)
            index = BUFSIZ - 1;

          // continue to read more data!
          continue;
        }

        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;

        // Print it out for debugging
        Serial.println(clientline);

        // Look for substring such as a request to get the root file
        if (strstr(clientline, "GET / ") != 0) {
          // send a standard http response header
          HtmlHeaderOK(client);
          // print all the data files, use a helper to keep it clean
          client.println("<h2>View data for the week of (dd-mm-yy):</h2>");
          ListFiles(client);
        }
        else if (strstr(clientline, "GET /") != 0) {
          // this time no space after the /, so a sub-file!
          char *filename;

          filename = strtok(clientline + 5, "?"); // look after the "GET /" (5 chars) but before
          // the "?" if a data file has been specified. A little trick, look for the " HTTP/1.1"
          // string and turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;

          // print the file we want
          Serial.println(filename);
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
          // everything else is a 404
          HtmlHeader404(client);
        }
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    client.stop();
  }

}
