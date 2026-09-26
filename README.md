# Duration-based Smart Attendance System (DSAS)

A smart attendance system using ESP32 and BLE technology to track student attendance based on their presence duration in class.

## Table of Contents
1. [Overview](#overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Connections](#connections)
4. [Software Requirements](#software-requirements)
5. [Code Explanation](#code-explanation)
6. [Setup Instructions](#setup-instructions)
7. [Usage](#usage)

## Overview
The system automatically tracks student attendance by detecting their Bluetooth devices. Students are marked present only if they are detected for at least 80% of the class duration. The system uses an RTC module for accurate timekeeping and Firebase for real-time data storage.

## Hardware Requirements
1. **ESP32 Development Board**
   - Main controller
   - Handles BLE scanning and WiFi connectivity
   - Processes attendance logic

2. **DS3231 RTC Module**
   - Real-Time Clock for accurate timekeeping
   - Includes battery backup (3V coin cell)
   - Maintains time even when main power is off

3. **Power Supply**
   - USB power for ESP32
   - 3V coin cell battery for RTC backup

4. **Connecting Wires**
   - For I2C communication between ESP32 and RTC

## Connections
### ESP32 to DS3231 RTC Module
```
ESP32          DS3231
-----------------------
3.3V    <-->   VCC
GND     <-->   GND
GPIO21  <-->   SDA
GPIO22  <-->   SCL
```

## Software Requirements
1. **Arduino IDE Libraries**
   - WiFi.h - WiFi connectivity
   - Wire.h - I2C communication
   - RTClib.h - RTC module interface
   - BLEDevice.h - BLE functionality
   - Firebase_ESP_Client.h - Firebase integration

2. **Firebase Setup**
   - Real-time database
   - Authentication credentials

## Code Explanation

### 1. Header Files and Constants
```cpp
// Required libraries
#include <WiFi.h>          // WiFi functionality
#include <Wire.h>          // I2C communication
#include <RTClib.h>        // RTC module
#include <BLEDevice.h>     // BLE functionality
#include <Firebase_ESP_Client.h> // Firebase integration

// WiFi and Firebase credentials
#define wifi_ssid "ARYABHATTA"
#define wifi_pswd "iitp@123"
#define fb_host "your_firebase_host"
#define fb_apikey "your_firebase_api_key"
```

### 2. Global Variables and Structures
```cpp
// RTC object
RTC_DS3231 rtc;

// Scanning parameters
const int scan_time = 1;        // 1 second scan duration
const int scan_interval = 5;    // 5 seconds between scans
const int stud_cnt = 4;         // Number of students

// Student structure
struct Student {
  String name;
  String uuid;
  String roll;
};

// Student array with UUID and roll numbers
Student students[stud_cnt] = {
  {"Name1", "UUID1", "Roll1"},
  // ... more students
};
```

### 3. Setup Function
```cpp
void setup() {
  // Initialize Serial and I2C
  Serial.begin(115200);
  Wire.begin();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("❌ Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Connect to WiFi
  WiFi.begin(wifi_ssid, wifi_pswd);
  // ... WiFi connection code ...

  // Initialize Firebase
  // ... Firebase setup code ...

  // Initialize BLE scanner
  BLEDevice::init("DSAS_Scanner");
  // ... BLE setup code ...
}
```

### 4. Main Loop Function
```cpp
void loop() {
  // Get class configuration
  if (!config_received) {
    // Wait for course code, lecture number, duration
    // Format: CS2201,1,60
  }

  // Perform BLE scan every scan_interval seconds
  if (current_time - last_scan_time >= scan_interval) {
    // Scan for devices
    // Match UUIDs with student list
    // Record timestamps
  }

  // Mark attendance when class ends
  if (class_over) {
    // Calculate duration for each student
    // Update Firebase with attendance status
    // Update statistics (attended, total, percentage)
  }
}
```

### 5. Firebase Data Structure
```
Batch27/
├── CS2201/
│   ├── 2301CS06/
│   │   ├── Lec1:2025-04-23: 1
│   │   ├── Lec2:2025-04-23: 0
│   │   └── ...
│   ├── 2301CS16/
│   └── ...
└── CS2202/
    └── ...
```

## Setup Instructions

1. **Hardware Setup**
   - Connect ESP32 to DS3231 following the connection diagram
   - Insert 3V coin cell battery in RTC module
   - Power ESP32 via USB

2. **Software Setup**
   - Install required Arduino libraries
   - Configure Firebase:
     - Create project
     - Set up Real-time Database
     - Get API credentials
   - Update code with:
     - WiFi credentials
     - Firebase credentials
     - Student information (names, UUIDs, roll numbers)

3. **Generate UUIDs**
   - Visit https://www.uuidgenerator.net/
   - Generate unique UUID for each student
   - Update `students` array in code

## Usage

1. **Initial Setup**
   - Upload code to ESP32
   - Open Serial Monitor (115200 baud)
   - RTC will be set to system time
   - System will connect to WiFi and Firebase

2. **Starting a Class**
   - Enter class details in Serial Monitor
   - Format: `[course_code],[lecture_number],[duration_minutes]`
   - Example: `CS2201,1,60`

3. **During Class**
   - System scans for student devices every 5 seconds
   - Entry and exit times are recorded
   - Real-time updates in Serial Monitor

4. **After Class**
   - Attendance marked automatically after duration
   - Results uploaded to Firebase
   - Statistics updated (attended, total, percentage)

## Important Notes

1. **RTC Time**
   - RTC keeps time as long as battery is present
   - No need to upload code before every class
   - Time set from system during upload

2. **BLE Detection**
   - Students must enable Bluetooth
   - Keep devices within range (~10 meters)
   - UUID must match exactly

3. **Attendance Criteria**
   - Present if detected for ≥80% of class duration
   - Continuous tracking every 5 seconds
   - Both entry and exit times recorded

4. **Firebase Updates**
   - Real-time attendance status
   - Individual lecture records
   - Running statistics maintained 