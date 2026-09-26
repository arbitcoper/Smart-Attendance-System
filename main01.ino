#include <WiFi.h>
#include <Wire.h>
#include <RTClib.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#include <BLEUtils.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Include sensitive credentials
// The secrets.h file defines the following macros:
// wifi_ssid, wifi_pswd, fb_host, fb_apikey, fb_email, fb_pswd
#include "secrets.h"

// firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// RTC object
RTC_DS3231 rtc;

const int scan_time = 1;        // BLE scan will be done for 1 second
const int scan_interval = 5;    // gap between two scans has been set to 5 seconds
const int stud_cnt = 4;         // Total number of students in class

// class config
String course_code = "";
int lec_num = 0;
String lec_date = "";
unsigned long class_duration = 0;
unsigned long req_duration = 0;
unsigned long last_scan_time = 0;

// formatting lecture number with date in format "LecX:YYYY-MM-DD"
String get_lecture_key(int lec_num, String date) {
  return "Lec" + String(lec_num) + ":" + date;
}

// structure for students info
struct Student {
  String name;
  String uuid;
  String roll;
};

// List of students to be put in this before uploading to esp32
// Format : {Name, UUID, Roll No.}
// UUID generated from https://www.uuidgenerator.net/
Student students[stud_cnt] = {
  {"Ankesh", "4da64075-887a-4eea-9121-1b0438ce4f89", "2301CS06"},
  {"Eshan", "88fe8911-8a91-4673-bf1c-f04360531436", "2301CS16"},
  {"Krishan", "bfb8e01f-c86e-4716-9c83-be0619d1daf1", "2301CS25"},
  {"Chaitanya", "8cdbd53a-1b57-4b09-b955-5f6ce441e1bf", "2301CS77"}
};

bool attendance_marked = false;
unsigned long class_start_time;
bool config_received = false;

// convert timestamp to HH:MM:SS
String time_to_string(unsigned long timestamp) {
  DateTime t(timestamp);
  char buffer[10];
  sprintf(buffer, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
  return String(buffer);
}

// BLE scan
BLEScan* pBLEScan;
unsigned long entry_timestamps[stud_cnt] = {0};
unsigned long last_seen_timestamps[stud_cnt] = {0};

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // initialize and set rtc time
  if (!rtc.begin()) {
    Serial.println("❌ Couldn't find RTC");
    while (1);
  }
  
  // setting rtc to system time
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  DateTime now = rtc.now();
  Serial.print("\nRTC Time: ");
  Serial.print(now.year());
  Serial.print("-");
  if(now.month() < 10) Serial.print("0");
  Serial.print(now.month());
  Serial.print("-");
  if(now.day() < 10) Serial.print("0");
  Serial.print(now.day());
  Serial.print(" ");
  if(now.hour() < 10) Serial.print("0");
  Serial.print(now.hour());
  Serial.print(":");
  if(now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");
  if(now.second() < 10) Serial.print("0");
  Serial.println(now.second());

  WiFi.begin(wifi_ssid, wifi_pswd);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected");
  }
  else {
    Serial.println("\n❌ Failed to connect to WiFi!");
    while (1);
  }

  // initializing Firebase
  config.api_key = fb_apikey;
  config.database_url = fb_host;
  auth.user.email = fb_email;
  auth.user.password = fb_pswd;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // wait for authentication with timeout
  Serial.print("Authenticating with Firebase");
  attempts = 0;
  while (auth.token.uid == "" && attempts < 20) {
    Serial.print(".");
    delay(1000);
    attempts++;
  }

  if (auth.token.uid != "") {
    Serial.println("\n☁️ Firebase connected.");
  }
  else {
    Serial.println("\n❌ Failed to connect to Firebase!");
    while (1);
  }

  // initialize BLE
  if (!BLEDevice::getInitialized()) {
    BLEDevice::init("DSAS_Scanner");
  }
  
  pBLEScan = BLEDevice::getScan();
  if (pBLEScan == nullptr) {
    Serial.println("❌ Failed to initialize BLE scan!");
    while (1);
  }
  
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  Serial.println("\n⏳ Enter course code, lecture number and duration in the format: [course_code],[lec_num],[duration_minutes]");
  Serial.println("Example: CS2201,4,60");
}

void loop() {
  if (!config_received) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      
      // Expected format: [course_code],[lec_num],[duration_minutes]
      // Example: CS2201,4,60
      int first_comma = input.indexOf(',');
      int second_comma = input.indexOf(',', first_comma + 1);
      
      if (first_comma > 0 && second_comma > first_comma) {
        course_code = input.substring(0, first_comma);
        String lec_num_str = input.substring(first_comma + 1, second_comma);
        String duration_str = input.substring(second_comma + 1);
        
        lec_num = lec_num_str.toInt();
        int duration_minutes = duration_str.toInt();
        
        if (course_code.length() > 0 && lec_num > 0 && duration_minutes > 0) {
          class_duration = duration_minutes * 60; // Convert minutes to seconds
          req_duration = 0.8 * class_duration;
          
          DateTime now = rtc.now();
          int year = now.year();
          int month = now.month();
          int day = now.day();
          char date_buffer[11];
          sprintf(date_buffer, "%04d-%02d-%02d", year, month, day);
          lec_date = String(date_buffer);

          config_received = true;
          class_start_time = rtc.now().unixtime();
          Serial.printf("\n📚 Class tracking started for %s, Lecture %d on %s.\n", course_code.c_str(), lec_num, lec_date.c_str());
          Serial.printf("⏱️ Class duration: %d minutes\n", duration_minutes);
        }
        else {
          Serial.println("⚠️ Invalid input format. Please use: [course_code],[lec_num],[duration_minutes]");
          Serial.println("Example: CS2201,4,60");
        }
      }
      else {
        Serial.println("⚠️ Invalid input format. Please use: [course_code],[lec_num],[duration_minutes]");
        Serial.println("Example: CS2201,4,60");
      }
    }
    delay(100);
    return;
  }

  DateTime now = rtc.now();
  unsigned long current_time = now.unixtime();

  // Only scan every scan_interval seconds
  if (current_time - last_scan_time >= scan_interval) {
    BLEScanResults found_devices = *pBLEScan->start(scan_time, false);
    last_scan_time = current_time;

    // Reset presence flags for this scan
    bool present_in_this_scan[stud_cnt] = {false};

    for (int i = 0; i < found_devices.getCount(); i++) {
      BLEAdvertisedDevice device = found_devices.getDevice(i);
      if (device.haveServiceUUID()) {
        String uuid = device.getServiceUUID().toString().c_str();

        for (int j = 0; j < stud_cnt; j++) {
          if (uuid.equalsIgnoreCase(students[j].uuid)) {
            if (entry_timestamps[j] == 0) {
              entry_timestamps[j] = current_time;
              Serial.printf("🟢 %s entered at %s\n", students[j].roll.c_str(), time_to_string(current_time).c_str());
            }
            present_in_this_scan[j] = true;
            last_seen_timestamps[j] = current_time;
          }
        }
      }
    }

    // Update duration for students present in this scan
    for (int j = 0; j < stud_cnt; j++) {
      if (present_in_this_scan[j] && entry_timestamps[j] != 0) {
        // Add scan_interval seconds to their duration
        last_seen_timestamps[j] = current_time;
      }
    }

    pBLEScan->clearResults();
  }

  // Class over → Mark attendance
  if (!attendance_marked && (current_time - class_start_time >= class_duration)) {
    attendance_marked = true;
    Serial.printf("🛑 Class ended. Marking attendance for %s, Lecture %d on %s...\n", course_code.c_str(), lec_num, lec_date.c_str());

    String lecture_key = get_lecture_key(lec_num, lec_date);

    for (int i = 0; i < stud_cnt; i++) {
      String base_path = "/Batch27/" + course_code + "/" + students[i].roll;
      String lec_path = base_path + "/" + lecture_key;

      unsigned long duration = 0;
      int status = 0;

      if (entry_timestamps[i] != 0) {
        duration = last_seen_timestamps[i] - entry_timestamps[i];
        if (duration >= req_duration) status = 1;
        
        // Add error handling for Firebase operations
        if (!Firebase.RTDB.setInt(&fbdo, lec_path, status)) {
          Serial.printf("❌ Failed to set attendance for %s\n", students[i].roll.c_str());
          continue;
        }
      } else {
        if (!Firebase.RTDB.setInt(&fbdo, lec_path, 0)) {
          Serial.printf("❌ Failed to set attendance for %s\n", students[i].roll.c_str());
          continue;
        }
      }

      // Update attendance stats with error handling
      if (Firebase.RTDB.getInt(&fbdo, base_path + "/attended")) {
        int attended = fbdo.intData();
        if (Firebase.RTDB.getInt(&fbdo, base_path + "/total")) {
          int total = fbdo.intData();
          
          if (status == 1) 
            attended++;
          total++;

          float percent = (total > 0) ? (attended * 100.0 / total) : 0.0;
          
          if (!Firebase.RTDB.setFloat(&fbdo, base_path + "/percentage", percent) ||
              !Firebase.RTDB.setInt(&fbdo, base_path + "/attended", attended) ||
              !Firebase.RTDB.setInt(&fbdo, base_path + "/total", total)) {
            Serial.printf("❌ Failed to update stats for %s\n", students[i].roll.c_str());
          }
        }
      }

      Serial.printf("📤 %s: %s (%lu secs) for %s\n", students[i].roll.c_str(), status ? "Present" : "Absent", duration, lecture_key.c_str());
    }
    Serial.printf("✅ Attendance upload complete for %s, Lecture %d on %s.\n", course_code.c_str(), lec_num, lec_date.c_str());
    while (true);     // stops after marking attendance for this lecture
  }

  delay(2000);
}