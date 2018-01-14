/* This is the web page code for n3m0's payload computer *
 *  M Holden 2017
 *  */
#include <ESP8266HTTPClient.h>
//#include <ArduinoJson.h>

void handleSocial() {
  String message = server.arg(0);
  String dirhtml;

HTTPClient http;
//http.begin("https://api.github.com/gists");
////http.addHeader("Content-Type", "application/json");
//http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//http.POST("{  \"description\": \"#n3m0datatest\",  \"public\": true,  \"files\": {    \"file1.txt\": {      \"content\": \"String file contents\"    }  }}");

//http.begin("http://jsonplaceholder.typicode.com/users/1");
http.begin("http://maker.ifttt.com");
int retval=http.GET();
message=String(retval)+"\r\n";
//message = http.readHeaderValue();
//http.writeToStream(&Serial);
//while (http.available()) {
//  message+=http.read();
//}
message = http.getString();
http.end();  
  
  dirhtml = "Returned:"+String(retval)+"<br><br>";; //"<meta http-equiv=\"refresh\" content=\"5; URL=/\" />";
  dirhtml += message + " <br><br>";
  dirhtml += "--END--";
  server.send(200, "text/html", dirhtml);
}


void handleMap() {
  String message = server.arg(0);
  String tempS;
  String headers[10],latS,lonS; 
  String headhtml, geojson, foothtml;
  String dirhtml;
  int i,row;
  char c;
  
  File dfile = SPIFFS.open(message, "r");

//  while (dfile.available()) {
//    dirhtml += char(dfile.read());
//  }
//  dirhtml += "<br><pre>" ;
  
  //message = dfile.readStringUntil('\r');

  
  // read file one char at a time and parse into labels or values
  // assumes csv format with header line.
  // text is copied exactly (no numbers read)

  // first line sets up header labels for geoJson
  i=0;
  while ((c = dfile.read()) != '\r') {
    if ((c==',')&&(i<9)){ // next header max 10.
      i++;
    } else {
      if (c!=' ') {
        headers[i]+=c;
      }      
    }
  }

  headhtml  = "<!DOCTYPE html>\r\n";
  headhtml += "<html>\r\n";
  headhtml += "<head>\r\n";
  headhtml += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">  \r\n";
  headhtml += "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"docs/images/favicon.ico\" />\r\n";
  headhtml += "<link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.0.3/dist/leaflet.css\" integrity=\"sha512-07I2e+7D8p6he1SIM+1twR5TIrhUQn9+I6yjqD53JQjFiMf8EtC93ty0/5vJTZGF8aAocvHYNEDJajGdNx1IsQ==\" crossorigin=\"\"/>\r\n";
  headhtml += "    <script src=\"https://unpkg.com/leaflet@1.0.3/dist/leaflet.js\" integrity=\"sha512-A7vV8IFfih/D732iSSKi20u/ooOfj/AGehOKq0f4vLT1Zr2Y+RX7C+w8A1gaSasGtRUZpF/NZgzSAu4/Gc41Lg==\" crossorigin=\"\"></script>\r\n";
  headhtml += "</head>\r\n";
  headhtml += "<body><h2>n3m0 data</h2>\r\n";
  headhtml += "<div id=\"mapid\" style=\"width: 90%; height: 600px;\"></div>\r\n";
//  headhtml += "<div id=\"mapid\" style=\"width: 80%; height: 80%;\" ></div>\r\n";
  headhtml += "<script>\r\n";
  headhtml += "var mymap = L.map('mapid').setView([51.505, -0.09], 13);\r\n";
  headhtml += "L.tileLayer('https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw', {\r\n";
  headhtml += "    maxZoom: 25,\r\n";
  headhtml += "    attribution: 'Map data &copy; <a href=\"http://openstreetmap.org\">OpenStreetMap</a> contributors, ' +\r\n";
  headhtml += "      '<a href=\"http://creativecommons.org/licenses/by-sa/2.0/\">CC-BY-SA</a>, ' +\r\n";
  headhtml += "      'Imagery © <a href=\"http://mapbox.com\">Mapbox</a>',\r\n";
  headhtml += "    id: 'mapbox.satellite'\r\n";
  headhtml += "  }).addTo(mymap);\r\n";
  headhtml += "var data = ";
  geojson = "{\r\n";
  geojson += "  \"type\": \"FeatureCollection\",\r\n";
  geojson += "  \"features\": [\r\n";
  
  // then go through the rest of the file printing at commas or eol
//  while (dfile.available()) {
    i=0;
    row=0;
    message="";
    while (dfile.available()) {
      c = dfile.read();
      if ((c=='\n')||(c=='\r')) {
        if (i>0) {
          if (headers[i].indexOf("temp") >= 0) {
            tempS = headers[i]; // save for labels later
          }
          if (headers[i].indexOf("lat") >= 0) {
            latS = message;
          } else {
            if (headers[i].indexOf("lon") >= 0) {
              lonS = message;
            } else {
              geojson += "\"" + headers[i] + "\":\"" + message + "\"},\r\n \"geometry\":{\"type\": \"Point\",\"coordinates\": [" + lonS + "," + latS + "]}}";
              row++;
            }
          }
          i=0; //start over
          message="";
        }
      } else {
        if ((c==',')&&(i<9)){ // next header max 10.
          if (headers[i].indexOf(String("lat")) >= 0) {
            latS = message;
          } else {
            if (headers[i].indexOf(String("lon")) >= 0) {
              lonS = message;
            } else {
              if (i==0) {  // beginning of definition
                if (row==0) { // first row
                  geojson += "{ \"type\":\"Feature\",\"properties\": {";
                } else {  // 2nd rows on need a commma
                  geojson += ",{ \"type\":\"Feature\",\"properties\": {";
                }
              }
              geojson += "\"" + headers[i] + "\":\"" + message + "\",";
            }
          }
          i++;
          message="";
        } else {
          if (c!=' ') {
            message+=c;
          }
        }
      }
    }

    geojson += "]}\r\n";
    foothtml += "myGeoLayer=L.geoJson(data, {\r\n";
    foothtml += "    onEachFeature: function (feature, layer) {\r\n";
    foothtml += "        layer.bindTooltip(\"WP:\"+feature.properties.wpt+\"<br>T:\" + feature.properties[\"" + tempS + "\"] ,\r\n";
    foothtml += "                          {noHide: true,permanent: true,interactive:true,opacity: 0.8, direction: 'top'});\r\n";
    foothtml += "    }\r\n";
    foothtml += "}).addTo(mymap);\r\n";

    foothtml += "mymap.fitBounds(myGeoLayer.getBounds());\r\n";
    foothtml += "    </script>\r\n";
//    foothtml += "<div id=\"dodo\" style=\"width: 600px; height: 400px;\"></div>\r\n";
//    foothtml += "<script>\r\n";
//    foothtml += "document.getElementById(\"dodo\").innerHTML = \"<br>Here is the geojson text: (copy and paste into <a href=\\\"http://geojson.io\\\">geojson.io</a>) <br>\" + JSON.stringify(data) + \"<br>\";\r\n";
//    foothtml += "</script>\r\n";

    foothtml += "</body>\r\n";
    foothtml += "</html>\r\n";

//  }
  // then close file
  dfile.close();

  // WRITE TO DISK
  message = server.arg(0);
  dfile = SPIFFS.open(message+".html","w");
  dfile.print(headhtml+geojson+foothtml);
  dfile.close();

  dfile = SPIFFS.open(message+".geojson","w");
  dfile.print(geojson);
  dfile.close();

  dirhtml = "<meta http-equiv=\"refresh\" content=\"5; URL=/\" />";
  dirhtml += message + " was converted.  Reload in 5 seconds <br>";
  dirhtml += "<a href=\"/\">click to return or wait</a>";
  server.send(200, "text/html", dirhtml);

}

void start_the_server() {
  //Serial.println("IN1");
  server.on("/", handleRoot);
  server.on("/stop", handleStop);
  server.on("/start", handleStart);
  server.on("/format", handleFormat);
  server.on("/dowifi", handleDoWifi);
  server.on("/wifi.php", handleGetWifi);
  server.on("/tomap.php",handleMap);
  server.on("/social.php",handleSocial);
  server.serveStatic("/", SPIFFS, "/");

//  server.on("/inline", [](){
//    server.send(200, "text/plain", "this works as well");
//  });

  server.onNotFound(handleNotFound);

  server.begin();
  //Serial.println("out");
   
}


void handleStop() {
  stoplogging=1;
  if (logfile) {
    logfile.close();
  }
  String dirhtml = "<meta http-equiv=\"refresh\" content=\"0; URL=/\" />";
  server.send(200, "text/html", dirhtml);
}
void handleStart() {
  stoplogging=0;
  String dirhtml = "<meta http-equiv=\"refresh\" content=\"0; URL=/\" />";
  server.send(200, "text/html", dirhtml);
}

void handleDoWifi() {
  //trytodoWifi(300);
  String dirhtml = "</body><!DOCTYPE html><html><body><form action=\"/wifi.php\">SSID:<br><input type=\"text\" name=\"SSID\"><br>Password:<br><input type=\"text\" name=\"Password\">  <br><br><input type=\"submit\" value=\"Submit\"></form></html>";
  server.send(200, "text/html", dirhtml);
}

void handleFormat() {
  stoplogging=1;
  if (logfile) {
    logfile.close();
  }
  SPIFFS.format();
  String dirhtml = "<meta http-equiv=\"refresh\" content=\"0; URL=/\" />";
  server.send(200, "text/html", dirhtml);
}

void handleRoot() {
  display.drawString(0, 48, "sent root"); 
  display.display();
  getTemperature();
  String dirhtml = "<h2>Hello from n3m0!<br>";
  dirhtml += "Temperature is " + String(temperatureFString) + " F</h2><br>";
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  dirhtml += "Total Space:" + String(fs_info.totalBytes) + " bytes<br>";
  dirhtml += "Used:" + String(fs_info.usedBytes) + " bytes<br>";
  dirhtml += "Free:" + String(fs_info.totalBytes-fs_info.usedBytes) + " bytes<br>";
  dirhtml += "<br><a href=/dowifi>Connect Wifi</a><br>";
  if (logfile) {
    dirhtml += "<br>Logging to:" + String(logfile.name());
    dirhtml += "<br><a href=/stop>STOP LOGGING</a><br>";
  } else {
    if (stoplogging) {
      dirhtml += "<br>Not Logging";
      dirhtml += "<br><a href=/start>START LOGGING</a><br>";
    } else {
      dirhtml += "<br>Ready to Log";
      dirhtml += "<br><a href=/stop>STOP LOGGING</a><br>";
    }
 }
  dirhtml += "<a href=/format>FORMAT</a><b> CAREFUL!</b><br>";
  dirhtml += "<br>File Listing:<br>";
  Dir root = SPIFFS.openDir("/");
  while (root.next()) {
    File f = root.openFile("r");
    dirhtml += "<a href=\""+root.fileName()+"\">" 
      + root.fileName() + " " + "</a> " + String(f.size());
      
    if (root.fileName().indexOf("txt")>=0) {
      if (root.fileName().indexOf("html")>=0) {
        // html file options
        dirhtml += "<br>";
      } else {
        if (root.fileName().indexOf("geojson") >= 0) {
          // geojson file options
          dirhtml += "<br>"; // " <a href=/social.php?f="+root.fileName()+">(share to gist and twitter)<br>";
        } else {
          // txt file options
          dirhtml += " <a href=/tomap.php?f="+root.fileName()+">(make map files)<br>";
        }
      }
    }
  }
  server.send(200, "text/html", dirhtml);
}

void handleGetWifi(){
  String message = "wifi setup\n\n";
  message += "SSID: ";
  message += server.arg(0);
  message += "\nPW:";
  message += server.arg(1);
  server.arg(0).toCharArray(ssid,32);
  server.arg(1).toCharArray(password,32);
  WiFi.disconnect();//  stop trying https://github.com/esp8266/Arduino/issues/1615
  if (start_the_wifi()) {
    // ESP chip saves last wifi so I don't need to.
//      if (logfile) {  // only one file at a time?
//        logfile.close();
//      } 
//      File pwfile = SPIFFS.open("/login.txt", "w");
//      if (pwfile) {
//        pwfile.println(server.arg(0));
//        pwfile.println(server.arg(1));
//        display.clear();
//        display.drawString(0, 0, "internet!"); 
//        display.drawString(0, 32, "AP connected"); 
//        display.display();
//        delay(500);
//        pwfile.close();
//      } else {
//        display.clear();
//        display.drawString(0, 0, "internet!"); 
//        display.drawString(0, 32, "no file"); 
//        display.display();
//      }
      delay(1000);
  }
  
  server.send(200, "text/plain", message);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

