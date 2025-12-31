#include <WiFi.h>
#include <time.h>
#include <RTClib.h>  // For DS3231 or DS1307 RTC module

// Enter your Wi-Fi credentials
const char* ssid = "DUC";
const char* password = "14042004";

// NTP server
const char* ntpServer1 = "time.google.com";
//const char* ntpServer2 = "time.cloudflare.com";
//const char* ntpServer3 = "asia.pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;

// RTC object (change to RTC_DS1307 for DS1307 module)
RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Track last sync time
struct tm timeinfo;
unsigned long lastSyncMillis = 0;  // Last sync time in milliseconds

void setup() {
  Serial.begin(9600);
  setupWiFi();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);  // Stop if RTC is not found
  }
  syncTime();
}

void loop() {
  checkTimeAndSync();  // Check if 1 hour has passed and sync if necessary

  // Get current time from RTC
  DateTime now = rtc.now();

  // Format the time as strings for easier readability
  String yearStr = String(now.year());
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month());
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day());
  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour());
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute());
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second());
  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  // Construct and print the formatted time
  String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;
  Serial.println(formattedTime);

  delay(10000);  // Wait 10 seconds before printing again
}

void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void syncTime() {
  // Configure time with NTP server and get time info
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Configure timezone for DST using POSIX rule
  Serial.println("Configuring timezone and dailight saving time");
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain adjusted time");
    return;
  }

  Serial.println("\n Time synchronized with NTP server with timezone and DST.");
  Serial.print("Current time: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  // Update the RTC with NTP time
  rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));

  lastSyncMillis = millis();  // Update last sync time
}

void checkTimeAndSync() {
  // Sync with NTP server every 1 hour (3600000 ms)
  if (millis() - lastSyncMillis >= 3600000) {
    Serial.println("Synchronizing time with NTP...");
    syncTime();
  }
}