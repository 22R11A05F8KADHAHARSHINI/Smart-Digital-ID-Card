#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include "index.h"

const char* ntpServer = "asia.pool.ntp.org";
const long  gmtOffset_sec = 5 * 3600 + 30 * 60; 
const int   daylightOffset_sec = 0;
time_t now;
struct tm timeinfo;

const double fences[1][10][2] = {{
  {17.521800, 78.632200},
  {17.521500, 78.630000},
  {17.520500, 78.629500},
  {17.519000, 78.630000},
  {17.518800, 78.631000},
  {17.519200, 78.631800},
  {17.520000, 78.632500},
  {17.521000, 78.632800},
  {17.521500, 78.632500},
  {17.521800, 78.632000}
}};


const char* hardcoded_ssid = "Evad bey nuvu";
const char* hardcoded_password = "ikgg6778";

TinyGPSPlus gps;
SoftwareSerial gpsSerial(D5, D6); 

double latitude = 0.0, longitude = 0.0;
int sat = 0;
String date_time = "N/A";

char lati[12] = "0.0";
char longi[12] = "0.0";
int targetStatus = 0;
int fence = 1;
char cumulativeAngle[12] = "0.0";

ESP8266WebServer gpsServer(80);

void connectWifi();
void generateRandomLocation();
void readGpsOrFallback();
void pip();
void handleRoot();
void fenceSelect();
void gps_data();
void updateNtpTime();

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  connectWifi();
  updateNtpTime();

  randomSeed(analogRead(A0));

  gpsServer.on("/", handleRoot);
  gpsServer.on("/status", fenceSelect);
  gpsServer.on("/values", gps_data);
  gpsServer.begin();

  Serial.print("Web server started at IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  readGpsOrFallback();
  pip();

  gpsServer.handleClient();
}

void updateNtpTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time from NTP");
  } else {
    char time_output[30];
    strftime(time_output, 30, "%F %T", &timeinfo);
    Serial.print("NTP Time Synced: ");
    Serial.println(time_output);
  }
}

void readGpsOrFallback() {
  if (gps.location.isValid() && gps.location.isUpdated()) {
    sat = gps.satellites.value();
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    date_time = String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year()) + " " +
                String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());

    Serial.print("REAL GPS: ");
  } else {
    static unsigned long lastFakeUpdate = 0;
    if (millis() - lastFakeUpdate > 5000) {
      generateRandomLocation();
      lastFakeUpdate = millis();
      sat = 0;

      if (getLocalTime(&timeinfo)) {
        char time_output[30];
        strftime(time_output, 30, "%d/%m/%Y %H:%M:%S", &timeinfo);
        date_time = String(time_output);
      } else {
        date_time = "Time Error";
      }

      Serial.print("FALLBACK: ");
    } else {
      return;
    }
  }

  dtostrf(latitude, 9, 7, lati);
  dtostrf(longitude, 9, 7, longi);

  Serial.print("LAT: ");
  Serial.print(latitude, 6);
  Serial.print(" | LONG: ");
  Serial.println(longitude, 6);
}


void connectWifi() {
  WiFi.begin(hardcoded_ssid, hardcoded_password);
  Serial.println("------------------------------------------------");
  Serial.print("Connecting to Access Point ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(hardcoded_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("------------------------------------------------");
}

void generateRandomLocation() {
  double baseLat = 17.520550;
  double baseLon = 78.631027;

  int mode = random(100) < 80 ? 0 : 1;

  double newLat, newLon;

  if (mode == 0) {
    newLat = baseLat + random(-500, 500) * 0.000001;
    newLon = baseLon + random(-500, 500) * 0.000001;
  } else {
    newLat = baseLat + random(1, 2) * (random(2) == 0 ? -1 : 1) * random(2000, 5000) * 0.000001;
    newLon = baseLon + random(1, 2) * (random(2) == 0 ? -1 : 1) * random(2000, 5000) * 0.000001;
  }

  latitude = newLat;
  longitude = newLon;
}

void pip() {
  if (fence < 1 || fence > 1) {
    targetStatus = 0;
    return;
  }

  int fenceSize = sizeof(fences[fence - 1]) / sizeof(fences[fence - 1][0]);
  double vectors[fenceSize][2];
  for (int i = 0; i < fenceSize; i++) {
    vectors[i][0] = fences[fence - 1][i][0] - latitude;
    vectors[i][1] = fences[fence - 1][i][1] - longitude;
  }

  double angle = 0;
  for (int i = 0; i < fenceSize; i++) {
    double num = (vectors[i][0]) * (vectors[(i + 1) % fenceSize][0]) + (vectors[i][1]) * (vectors[(i + 1) % fenceSize][1]);
    double den = (sqrt(pow(vectors[i][0], 2) + pow(vectors[i][1], 2))) * (sqrt(pow(vectors[(i + 1) % fenceSize][0], 2) + pow(vectors[(i + 1) % fenceSize][1], 2)));

    if (den == 0.0) {
      angle = 360.0;
      break;
    }

    double acos_arg = num / den;
    if (acos_arg > 1.0) acos_arg = 1.0;
    else if (acos_arg < -1.0) acos_arg = -1.0;

    angle += (180 * acos(acos_arg) / PI);
  }

  dtostrf(angle, 9, 7, cumulativeAngle);

  if (angle > 180.0) {
    targetStatus = 1;
    Serial.println("STATUS: INSIDE Geofence.");
  } else {
    targetStatus = 0;
    Serial.println("STATUS: OUTSIDE Geofence!");
  }
}


void handleRoot() {
  String s = webpage;
  gpsServer.send(200, "text/html", s);
}

void fenceSelect() {
  fence = gpsServer.arg("fenceValue").toInt();
  gpsServer.send(200, "text/plane", String(fence));
}

void gps_data() {
  String payload = String(sat) + "#" + date_time + "#" + lati + "#" + longi;

  if (targetStatus == 0) payload += "#outside";
  else payload += "#inside";

  payload += "#" + String(cumulativeAngle);
  payload += "#ONLINE";

  gpsServer.send(200, "text/plane", payload);
}
