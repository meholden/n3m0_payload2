/* This is the data logging code for n3m0's payload computer *
 *  M Holden 2017
 */
void log_update() {

  if (do_log) {
      do_log=0;
      // open file if needed.
      if ((!stoplogging) && (!logfile)) {
        int ii=0;
        String fname = "/sentences" + String(ii) + ".txt";
        while (SPIFFS.exists(fname) || ii>=100) {
          ii++;
          fname = "/sentences" + String(ii) + ".txt";
        }
        logfile = SPIFFS.open(fname, "w");
        if (!logfile) {
          stoplogging=1;
        }    
        logfile.println("loggertime(s), unixtime(s), wpt, lat, lon, speed(m/s), heading(deg), temperature(c)");
      }

      // take measurement
      if (DS18B20.getDeviceCount() > 0) {
        getTemperature();
        display.clear();
        display.drawString(0, 0, WiFi.localIP().toString()); 
        display.drawString(0, 16, "t = " + String(((double)timegps_usec)/1e6)); 
        display.drawString(0, 32, "wpt: " + String(mission_reached.seq));
        display.drawString(0, 48, "T = " + String(temperatureFString) + " F"); 
        display.display();
      } else {
        display.clear();
        display.drawString(0, 0, "n3m0 data"); 
        display.drawString(0, 16, "t = " + String(((float)timegps_usec)/1e6)); 
        display.drawString(0, 48, "No T sensor"); 
        display.display();
      }

      // save things to the file
      // loggertime, unixtime, wpt, lat, lon, speed, heading, temperature
      if (logfile) {
        logfile.print(((float)millis())/1000.0);
        logfile.print(",");
        logfile.print(((float)timegps_usec)/1e6);
        logfile.print(",");
        logfile.print(mission_reached.seq);
        logfile.print(",");
        logfile.print(String((float)gps_raw.lat/1e7,6));
        logfile.print(",");
        logfile.print(String((float)gps_raw.lon/1e7,6));
        logfile.print(",");
        logfile.print(String((float)gps_raw.vel/100.0,2));
        logfile.print(",");
        logfile.print(String((float)gps_raw.cog/100.0,2));
        logfile.print(",");
        logfile.println(temperatureCString);
        //logfile.print(",");        
      }
  
  }
  
}
void getTemperature() {
  float tempC;
  float tempF;
//  do {
    DS18B20.requestTemperatures(); 
    tempC = DS18B20.getTempCByIndex(0);
    dtostrf(tempC, 2, 2, temperatureCString);
    tempF = DS18B20.getTempFByIndex(0);
    dtostrf(tempF, 3, 2, temperatureFString);
    //delay(100);
//  } while (tempC == 85.0 || tempC == (-127.0));
}

void sendstatus() {
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t len;
      // request streams
//      mavlink_msg_heartbeat_pack(1,2,&msg,10,3,0,0,0);
//      //mavlink_msg_set_mode_pack(0,0,&msg, 1,193,2); //custom mode 2=learn: 0 mavlink_heartbeat_t custom_mode 2 type 10 autopilot 3 base_mode 193 system_status 4 mavlink_version 3  sig  Len 17
//      len = mavlink_msg_to_send_buffer(buf, &msg);+
//      Serial.write(buf, len);
//      delay(500);      
      //mavlink_msg_statustext_pack(0, 0, &msg, 9, "hello world");
//     char msgText[MAVLINK_MSG_ID_STATUSTEXT_LEN];
//     strncpy(msgText, "hello world", MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN);
//     mavlink_msg_statustext_pack(88, 88, &msg, 1, msgText);
     mavlink_msg_statustext_pack(1, 2, &msg, 6, "hello world");
      // Copy the message to the send buffer
      len = mavlink_msg_to_send_buffer(buf, &msg);
 
      // Send the message with the standard UART send function
      Serial.write(buf, len);
      delay(1000);      
//      display.clear();
      display.drawString(110, 0, "=="); 
      display.display();
      gotmsg=1; //hack!

}

void mavlink_request_comm() {
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    if ((gotmsg) && (notrequested)) {
      // request streams
//      mavlink_msg_request_data_stream_pack(0, 1, &msg, sysid_from, compid_from, 0, 1, 1);
      mavlink_msg_request_data_stream_pack(1,2, &msg, sysid_from, compid_from, 0, 1, 1);
 // uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, uint8_t target_system, uint8_t target_component, uint8_t req_stream_id, uint16_t req_message_rate, uint8_t start_stop
      
      // Copy the message to the send buffer
      uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
 
      // Send the message with the standard UART send function
      Serial.write(buf, len);
      
      display.clear();
      display.drawString(0, 0, "Start data req"); 
      display.display();
      
      delay(1000);
      notrequested=0;
    }   
}


void comm_receive() {
 
  mavlink_message_t msg;
  mavlink_status_t status;
  
  while(Serial.available() > 0 ) 
  {
    digitalWrite(D0,(!(digitalRead(D0))));
    uint8_t c = Serial.read();
//      display.clear();
//      display.drawString(0, 0, "yyyy"); 
//      display.drawString(0, 16, String(c)); 
//      display.display();
    // Try to get a new message
    if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      // Handle message
     // see the .h file for each message in C:\Users\Mike\Documents\Arduino\libraries\mavlinkinclude\common
     // there are functions to decode all the sent data, individually or as a data structure.

//      Serial.print ("msg: ");
//      Serial.print(msg.msgid);
//      Serial.print("  SysID: ");
//      Serial.print(msg.sysid);
//      Serial.print("  CMP ID:  ");
//      Serial.println(msg.compid);
//      display.clear();
//      display.drawString(0, 0, "msg:" + String(msg.msgid)); 
//      display.drawString(0, 16, String(millis())); 
//      display.display();
//      delay(2000);
      gotmsg=1; // got a message
      sysid_from = msg.sysid; // who it is from
      compid_from = msg.compid;

      switch(msg.msgid)
      {
         case MAVLINK_MSG_ID_HEARTBEAT:
               // E.g. read GCS heartbeat and go into
               // comm lost mode if timer times out
//               gotmsg=1; // got a message
//               sysid_from = msg.sysid; // who it is from
//               compid_from = msg.compid;
              break;
         case MAVLINK_MSG_ID_COMMAND_LONG:
              // EXECUTE ACTION
              break;
         case MAVLINK_MSG_ID_ATTITUDE:
              // 30
              break;
         case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
              // 46
              // GOT TO WAYPOINT?
              mavlink_msg_mission_item_reached_decode(&msg, &mission_reached);
              do_log=1; // flag to write data
              break;
         case MAVLINK_MSG_ID_SYSTEM_TIME:
              // 2
              //timesys_usec = mavlink_msg_system_time_get_time_unix_usec(&msg);
              timesys_usec = mavlink_msg_system_time_get_time_boot_ms(&msg);
              timegps_usec = mavlink_msg_system_time_get_time_unix_usec(&msg);
//                display.clear();
//                display.drawString(0, 0, "upT:" + String(float(timesys_usec))); 
////              Serial.print("\r\n upTIME: ");
////              Serial.print((float)timesys_usec/1000.0);
////              Serial.print(", sysTIME: ");
////              Serial.println((float)timesys_usec);
//                display.drawString(0, 16, "gpT:" + String(float(timegps_usec))); 
//                display.display();
              break;
         case MAVLINK_MSG_ID_GPS_RAW_INT:
              mavlink_msg_gps_raw_int_decode(&msg, &gps_raw); 
              gotGPS=1;
              //timegps_old = timegps_usec;
              //timegps_usec = mavlink_msg_gps_raw_int_get_time_usec(&msg);
//              display.clear();
//              display.drawString(0, 0, "msg:" + String(msg.msgid)); 
//              display.drawString(0, 16, String(millis())); 
//              display.display();
//              break;
         default:
              //Do nothing
              break;
      }

//      if (logfile) {
//        logfile.print(millis());
//        logfile.print(",");
//        logfile.print(msg.msgid);
//        if (msg.msgid==MAVLINK_MSG_ID_MISSION_ITEM_REACHED) {
//          logfile.print(",");
//          logfile.println(mission_reached.seq);
//        } else {
//          logfile.print("\r\n");
//        }
//      }
//
    }
 
    // And get the next one
  }
}
 
