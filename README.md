# 📍 Smart Digital ID Card using IoT

## 📖 Overview

This project implements a Smart Digital ID Card using IoT for real-time location tracking and geofencing. It uses an ESP8266 NodeMCU and Neo-6M GPS module to monitor live location and display it on a web-based dashboard.

---

## 🚀 Features

* Real-time GPS tracking
* Geofence boundary detection
* Live location visualization on map
* Web dashboard hosted on ESP8266
* Wi-Fi based communication (no GSM)
* Low-cost and scalable system

---

## 🛠️ Tech Stack

* **Hardware:** ESP8266 NodeMCU, Neo-6M GPS
* **Software:** Arduino IDE (C/C++)
* **Web:** HTML, CSS, JavaScript
* **Libraries:** TinyGPS++, ESP8266WiFi
* **Map Integration:** Leaflet.js

---

## ⚙️ Working

1. GPS module collects real-time coordinates
2. ESP8266 processes the data
3. Geofence logic checks boundary status
4. Web server sends data to dashboard
5. Dashboard displays location and status

---

## 🔌 Hardware Setup

* GPS TX → ESP8266 RX
* GPS RX → ESP8266 TX
* Power via USB / 5V supply

---

1. Open in Arduino IDE
2. Install required libraries
3. Update Wi-Fi credentials
4. Upload code to ESP8266

---

## 🌐 Usage

* Connect device to Wi-Fi
* Open ESP8266 IP address in browser

---

## 📊 Output

* Map visualization
* Geofence status (Inside/Outside)

---

## 🔮 Future Scope

* Cloud integration for data storage
* Mobile application support
* Multi-device tracking
* Improved indoor tracking accuracy

---
