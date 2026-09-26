# Duration-based Smart Attendance System (DSAS)

## Table of Contents
1. [Introduction](#introduction)
2. [Motivation](#motivation)
3. [Innovation/Uniqueness](#innovationuniqueness)
4. [Hardware Requirements](#hardware-requirements)
5. [Project Workflow](#project-workflow)
6. [Circuit Diagram](#circuit-diagram)
7. [Code Details](#code-details)
8. [Project Outcome](#project-outcome)
9. [Potential System Exploits and Vulnerabilities](#potential-system-exploits-and-vulnerabilities)
10. [Individual Contributions](#individual-contributions)

## Introduction
The Duration-based Smart Attendance System (DSAS) is an automated attendance tracking system that uses Bluetooth Low Energy (BLE) technology to detect student presence. The system consists of an ESP32 microcontroller and a Real-Time Clock (RTC) module, which work together to track student attendance based on their physical presence in the classroom for a specified duration.

## Motivation
- **Manual Attendance Issues**: Traditional roll-call methods are time-consuming, especially in large classes (100+ students), and are prone to proxy attendance.
- **Digital Attendance Limitations**: While digital methods like Google Forms are faster, they are vulnerable to proxy attendance as students can easily share form links.
- **Project Goals**:
  - Automate the attendance process to save time
  - Use BLE detection to verify actual student presence
  - Implement duration-based attendance (students must be present for 80% of class time)
  - Provide real-time attendance tracking and reporting

## Innovation/Uniqueness
- **Duration-Based Tracking**: Students are marked present only if they are detected for at least 80% of the class duration
- **Real-Time Monitoring**: System scans every 5 seconds to verify continuous presence
- **Offline Time Accuracy**: RTC module ensures accurate timekeeping independent of network connectivity
- **Firebase Integration**: Real-time attendance data storage and retrieval
- **Cost-Effective**: Uses minimal hardware components (ESP32 + RTC module)
- **Scalable**: Can be easily deployed across multiple classrooms

## Hardware Requirements
- **ESP32 Dev Module**: For BLE scanning and WiFi connectivity
- **DS3231 RTC Module**: For accurate timekeeping
- **Power Supply**: 
  - USB power for ESP32
  - 3V coin cell battery for RTC backup
- **Connecting Wires**: For I2C communication between ESP32 and RTC

## Project Workflow
1. **Initialization**:
   - ESP32 connects to WiFi
   - RTC sets current time
   - Firebase authentication
   - BLE scanner initialization

2. **Class Setup**:
   - Instructor inputs course code, lecture number, and duration
   - System starts tracking time

3. **Attendance Tracking**:
   - BLE scans every 5 seconds
   - Detects student devices using UUID
   - Records entry and last seen timestamps
   - Updates Firebase in real-time

4. **Attendance Marking**:
   - After class duration ends
   - Calculates presence duration for each student
   - Marks attendance based on 80% threshold
   - Updates attendance statistics

## Circuit Diagram
```
ESP32 <--I2C--> DS3231 RTC
  |
  |--USB Power
  |--WiFi Connection
  |--BLE Scanning
```

## Code Details
The system uses several key components:

1. **RTC Time Management**:
   - Automatic time setting from system
   - Accurate timestamp recording
   - Duration calculation

2. **BLE Scanning**:
   - 5-second scan intervals
   - UUID-based student identification
   - Continuous presence tracking

3. **Firebase Integration**:
   - Real-time database updates
   - Attendance statistics tracking
   - Secure authentication

4. **Attendance Logic**:
   - Entry time recording
   - Duration calculation
   - 80% presence threshold
   - Automatic status updates

## Project Outcome
- **Successful Implementation**:
  - Accurate BLE device detection
  - Reliable time tracking
  - Real-time Firebase updates
  - Duration-based attendance marking

- **Technical Specifications**:
  - BLE detection range: ~10 meters
  - Scan interval: 5 seconds
  - Minimum attendance duration: 80% of class time
  - Real-time data updates

- **Challenges Overcome**:
  - BLE signal stability
  - RTC time synchronization
  - Firebase connection management
  - Memory optimization

## Potential System Exploits and Vulnerabilities
1. **Device Sharing**: Students could share their devices to mark proxy attendance
   - Mitigation: Regular physical verification by instructors

2. **Bluetooth Disabling**: Students might disable Bluetooth to avoid detection
   - Mitigation: Make Bluetooth usage mandatory for attendance

3. **Signal Range**: BLE signals can be affected by physical obstacles
   - Mitigation: Strategic placement of ESP32 in classroom

4. **Battery Issues**: RTC backup battery failure could affect timekeeping
   - Mitigation: Regular battery checks and replacements

5. **Network Dependence**: Firebase connectivity issues
   - Mitigation: Local storage with sync when connection resumes
