/* This is the payload code for n3m0 the autonomous boat
 *  MEH 6/2017
 *  Should: log ds18b20 temperature probe
 *          log mavlink data
 *          write lat/lon/time/temp to spiffs file
 *          serve data files on webserver (AP or your wifi)
 *          upload data to web somewhere.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"'

#include <checksum.h>
#include <mavlink.h>
#include <mavlink_helpers.h>
#include <mavlink_types.h>
#include <protocol.h>

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin D1 on the ESP8266 12-E - GPIO 5
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);
char temperatureCString[6];
char temperatureFString[6];

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);

// global variables- MavLink
uint64_t timegps_usec;
uint64_t timesys_usec;
uint64_t timegps_old=0;
uint64_t timegps_boot=0;
uint8_t sysid_from;
uint8_t compid_from;
uint8_t gotmsg=0;
uint8_t notrequested=1;
mavlink_gps_raw_int_t gps_raw;
mavlink_mission_item_reached_t mission_reached;
uint64_t lastprint=0;
uint8_t gotGPS=0;

// logging
uint8_t stoplogging=0, do_log=0;
File logfile;

// wifi stuff
char ssid[32];
char password[32];
String myIP;

ESP8266WebServer server(80);

void setupscreen(void) {
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.clear();
  display.drawString(0, 0, "n3m0 data\r\nConnecting WiFi\r\n....."); 
  display.display();  
}


void setup(void){

  /* Hardware Initializations ------------------- */
  // LED Pin D0
  pinMode(D0,OUTPUT);
  digitalWrite(D0,LOW);
  
  // Initialise LED display
  setupscreen();
  // Initialize serial (57600 for APM telemetry)
  Serial.begin(57600);
  Serial.println("\n\n Welcome n3m0 User!");
  // Initialize temp sensor
  DS18B20.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  DS18B20.setResolution(12); // 12bit=0.11F resolution, 9bit=0.9F

  // Initialize file system
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS init failed");
  } else {
    Serial.println("SPIFFS filesystem init OK");
  }

  // Network & Webserver Init ========================
  // Todo: Check if connected, do AP if not
  //       Add wifimanager to configure
  // load ssid and password from file if exists
  // I think esp8266 remembers it if you send blank ssid..
//  File pwfile = SPIFFS.open("/login.txt", "r");
//  if (pwfile) {
//    pwfile.readBytesUntil('\r',ssid,32);
//    pwfile.readBytesUntil('\r',password,32);
//    pwfile.close();
//  }
  // configure to network or start AP mode
  start_the_wifi();

  Serial.println("\n\nStarting Server...");

  start_the_server();
    
  Serial.println("HTTP server started");

  // End of Network & Server Init.

  // Set up to receive serial stuff
  // sets serial to use alternate pins (not USB)
  Serial.println("Swapping Serial port, goodbye.");
  delay(1000);
  Serial.swap();
  display.clear();
  display.drawString(0, 0, "n3m0 data"); 
 display.drawString(0, 16, "initialization"); 
 display.drawString(0, 48, "complete"); 
 display.display();
 delay(3000);
}

void loop(void){
  server.handleClient();

  // display every second or so, use #define to turn off.
  #define PRINTTEMP 1
  if ((millis() > lastprint+2000)&& PRINTTEMP) {
//    if (DS18B20.getDeviceCount() > 0) {
//      getTemperature();
//      display.clear();
//      display.drawString(0, 0, WiFi.localIP().toString()); 
//      display.drawString(0, 16, "t = " + String(millis()/1000.0)); 
//      display.drawString(0, 48, "T = " + String(temperatureFString) + " F"); 
//      display.display();
//    } else {
//      display.clear();
//      display.drawString(0, 0, "n3m0 data"); 
//      display.drawString(0, 16, "t = " + String(millis()/1000.0)); 
//      display.drawString(0, 48, "No T sensor"); 
//      display.display();
//    }
      display.clear();
      display.drawString(0, 0, myIP); 
      if (timegps_usec >0) {
        display.drawString(0, 16, String(((double)timegps_usec)/1e6)); 
      } else {
        display.drawString(0, 16, "syst = " + String((float)timesys_usec/1000.0)); 
      }
      if (!logfile) {
        if (stoplogging) {
          display.drawString(0, 32, "Log off!  fix:"+String(gps_raw.fix_type));
        } else {
          display.drawString(0, 32, "Log ready.  fix:"+String(gps_raw.fix_type));          
        }
      } else {
        display.drawString(0, 32, "Log on.  fix:"+String(gps_raw.fix_type));
      }
      display.drawString(0,48,String((float)gps_raw.lat/1e7,4)+" " +String((float)gps_raw.lon/1e7,4) );
      display.display();

     sendstatus();
      
    lastprint = millis();
  }

  // get chars from uart, do mavlink
    mavlink_request_comm(); // requests data after first sentence received

  // This is the main mavlink handling routine (polled)
    comm_receive();

  // if at waypoint, take readings
  // log data to file
    log_update();

}


