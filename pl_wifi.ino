/* This is the wifi code for n3m0's payload *
 *  M Holden 2017 *
 *  
 *  */


char start_the_wifi() {
  WiFi.begin(ssid, password);
  // Wait for connection
  unsigned long timenow;
  char worked=0;
  timenow = millis();
  while ((WiFi.status() != WL_CONNECTED)&&(millis() < timenow + 30000)) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.mode(WIFI_STA); // remove AP https://github.com/esp8266/Arduino/issues/1615
    display.clear();
    //display.drawString(0, 0, "Humidity: " + String(h) + "%\t"); 
    display.drawString(0, 0, "n3m0 data"); 
    myIP = WiFi.localIP().toString();
    display.drawString(0, 16, myIP); 
    display.drawString(0, 32, "connected"); 
    display.drawString(0, 48, "123456789123456789"); 
    display.display();
    worked = 1;

  } else {
    WiFi.disconnect();//  stop trying https://github.com/esp8266/Arduino/issues/1615
    WiFi.softAP("n3m0_wifi");
    myIP = WiFi.softAPIP().toString() +"(AP)";
    //Serial.print("AP IP address: ");
    display.clear();
    //display.drawString(0, 0, "Humidity: " + String(h) + "%\t"); 
    display.drawString(0, 0, "n3m0 data"); 
    display.drawString(0, 32, "AP connected"); 
    display.display();
    delay(1000);
  }
  
  // Connect to http://n3m0.local/
  // takes a few seconds to propagate
  // doesn't work on every network.
  if (MDNS.begin("n3m0")) {
    Serial.println("MDNS responder started");
  }
  return worked;
}

