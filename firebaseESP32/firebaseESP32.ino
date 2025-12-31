//backup đồ án 2 có lora + wifi + ds1307 + xử lý nút bấm local
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <time.h>
#include <EEPROM.h>
#include <RTClib.h>
//#include <C:\Users\Duc\Documents\Arduino\ds1307_i2c\ds1307_i2c.h>

String ssid = "DUC";
String pass = "14042004";
String ssidnew = "";
String passnew = "";
const char* ntpServer1 = "time.google.com";
//const char* ntpServer2 = "time.cloudflare.com";
//const char* ntpServer3 = "asia.pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;

#define DATABASE_URL "https://testesp8266-15723-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "TURc5Td0kl6roHU0jB27iadFsW5FLv8RpPgChQ03"
#define led 2

FirebaseData fbdo,fbdo_wf,fbdo_dt;
FirebaseAuth auth;
FirebaseConfig config;
RTC_DS1307 rtc;

String data;
int power;
int demwifi=0;
int dem = 0;
int demtg = 0;
int mode  = 0;
int currentHour = 0;
int currentMin = 0;
int hoursta = 0;
int minsta = 0;
int hoursto = 0;
int minsto = 0;
int modewf;
bool ledState = 0;
int autoStateLocal = 0;
int lastAutoStateLocal = -1;
int autoState = 0;
int lastAutoState = -1;
int statuswf = -1;
bool btn1 = false;
bool btn2 = false;
void IRAM_ATTR ISR() {
  if(digitalRead(32) == 0) {
    btn1 = true;
  }
}

void IRAM_ATTR ISR2() {
  if(digitalRead(33) == 0) {
    btn2 = true;
  }
}
void send_on() {
  digitalWrite(led, HIGH);
  Serial2.println("on");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial2.println("on");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial2.println("ngu");
}
void send_off() {
  Serial2.println("off");
  digitalWrite(led, LOW);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial2.println("off");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial2.println("ngu");  
}
void doctime_ds1307() {
  // get_time();
  // currentHour = gio.toInt();
  // currentMin = phut.toInt();

  DateTime now = rtc.now();
  currentHour = now.hour();
  currentMin  = now.minute();
  //Serial.printf("Đang get time từ DS1307: %d:%d\n", currentHour, currentMin);
}
void xulynutbam() {
  if (btn1) {
    btn1 = false;
    if(WiFi.status() == WL_CONNECTED) {
      Firebase.setInt(fbdo,"/data/power", 1);
    }
    send_on();
  }
  else if(btn2) {
    btn2 = false;
    if(WiFi.status() == WL_CONNECTED) {
      Firebase.setInt(fbdo,"/data/power", 0);
    }
    send_off();
  }
}
void xuly_local() {
  doctime_ds1307();
  Serial.printf("⏱ Start: %02d:%02d - Stop: %02d:%02d\n", hoursta, minsta, hoursto, minsto);     
  Serial.printf("🕒 DS1307 Time: %02d:%02d\n", currentHour, currentMin);
  if ((currentHour > hoursta || 
      (currentHour == hoursta && currentMin >= minsta)) &&
      (currentHour < hoursto || 
      (currentHour == hoursto && currentMin < minsto)) &&
      (mode == 1)) 
  {
    autoStateLocal = 1;
  } 
  else if ((currentHour < hoursta || 
           (currentHour == hoursto && currentMin >= minsto)) &&
           (mode == 1)) 
  {
    autoStateLocal = 0;
  }
  if (autoStateLocal != lastAutoStateLocal) {
    if (autoStateLocal == 1) {
      send_on();
    } else if (autoStateLocal == 0) {
      send_off();
    }
    lastAutoStateLocal = autoStateLocal;
  }
  delay(1000);
}

void updateStatus() {
  if (data == "on") {
    Firebase.setString(fbdo, "/Status", "Đã bật");
  }
  else if(data == "off") {
    Firebase.setString(fbdo, "/Status", "Đã tắt");
  }
  else if(data != "on" || data != "off") {
    float pin = data.toFloat();
    Firebase.setFloat(fbdo, "/pin", pin);
  }
}

void thucong() {
  if (power == 1 && mode == 0) {
    send_on();
  }
  else if (power == 0 && mode == 0) {
    send_off();
  }
}
void xulytudong() {    
  if (mode != 1) {
    lastAutoState = -1;
    return;
  }

  if ((currentHour > hoursta || (currentHour == hoursta && currentMin >= minsta)) &&
      (currentHour < hoursto || (currentHour == hoursto && currentMin < minsto)) &&
      mode == 1) {
    autoState = 1;
  } 
  else if ((currentHour < hoursta || (currentHour == hoursto && currentMin >= minsto)) &&
           mode == 1) {
    autoState = 0;
  }

  if (autoState != lastAutoState) {
    if (autoState == 1) {
      send_on();
    } else if (autoState == 0) {
      send_off();
    }
    lastAutoState = autoState;
  }
}

void streamdata() {
    if (!Firebase.readStream(fbdo_dt)) {
        Serial.printf("Stream data begin error: %s\n", fbdo_dt.errorReason().c_str());
    }
    if (fbdo_dt.streamAvailable()) {
        if (Firebase.getInt(fbdo, "/data/auto")) {
            mode = fbdo.intData();
            EEPROM.writeInt(260, mode);
            EEPROM.commit();
            if (mode == 0) {
              //Serial.printf("\n Mode hien tai: %d", mode);
              if (Firebase.getInt(fbdo, "/data/power")) {
                power = fbdo.intData();
                thucong();
              }
            } else if (mode == 1) {
                if (Firebase.getInt(fbdo, "/data/hoursta")) hoursta = fbdo.intData();
                if (Firebase.getInt(fbdo, "/data/minsta")) minsta = fbdo.intData();
                if (Firebase.getInt(fbdo, "/data/hoursto")) hoursto = fbdo.intData();
                if (Firebase.getInt(fbdo, "/data/minsto")) minsto = fbdo.intData();
                EEPROM.writeInt(132, hoursta);
                EEPROM.writeInt(164, hoursto);
                EEPROM.writeInt(196, minsta);
                EEPROM.writeInt(228, minsto);
                EEPROM.commit();
                Serial.printf("⏱ Start: %02d:%02d - Stop: %02d:%02d\n", hoursta, minsta, hoursto, minsto);
            }
        }
    }
}
void updatewifi() {
  if (modewf == 1) {
    if (Firebase.getString(fbdo, "/wifi/ssid")) ssidnew = fbdo.stringData();
    if (Firebase.getString(fbdo, "/wifi/pass")) passnew = fbdo.stringData();
    WiFi.begin(ssidnew, passnew);
    while (WiFi.status() != WL_CONNECTED) {
      demwifi++;
      xulytudong();
      xulynutbam();
      digitalWrite(2, (ledState = ledState ? 0 : 1));
      if (demwifi == 60) {
        WiFi.begin(ssid, pass);
        demwifi = 0;
      }
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    delay(2000);
    fbdo.clear();
    digitalWrite(2, 0);
    if(WiFi.status() == WL_CONNECTED) {
      if(WiFi.SSID() == ssidnew) {
        ssid = ssidnew;
        pass = passnew;
        Serial.println("\nConnected with IP: " + WiFi.localIP().toString());
        EEPROM.writeString(0, ssid);
        EEPROM.writeString(100, pass);
        EEPROM.commit();
        fbdo.clear();
        modewf = 0;
        statuswf = 1;
        Firebase.setInt(fbdo, "/wifi/statuswf", statuswf);
        Firebase.setInt(fbdo, "/wifi/modewf", modewf);
      }
      else {
        modewf = 0;
        statuswf = 0;
        Firebase.setInt(fbdo, "/wifi/statuswf", statuswf);
        Firebase.setInt(fbdo, "/wifi/modewf", modewf);
        Firebase.setString(fbdo, "/wifi/ssid", ssid);
        Firebase.setString(fbdo, "/wifi/pass", pass);
      }
    }
    delay(3000);
    Firebase.setInt(fbdo, "/wifi/statuswf", -1);
  }
  else return;
}

void streamwifi() {
    if (!Firebase.readStream(fbdo_wf)) {
        Serial.printf("Stream error: %s\n", fbdo_wf.errorReason().c_str());
    }
    if (fbdo_wf.streamAvailable()) {
        if (Firebase.getInt(fbdo, "wifi/modewf")) {
          modewf = fbdo.intData();
      }
    }
}


void init_firebase() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  Firebase.begin(&config, &auth);
  Firebase.setString(fbdo, "/wifi/ssid", ssid);
  Firebase.setString(fbdo, "/wifi/pass", pass);
  if( Firebase.getInt(fbdo,"/data/mode")) mode =  fbdo.intData();
  if( Firebase.getInt(fbdo,"/data/power")) power =  fbdo.intData();
  Firebase.setInt(fbdo, "/wifi/statuswf", statuswf);
  Serial.printf("biến power: %d, biến mode: %d\n", power,mode);
  if(!Firebase.beginStream(fbdo_wf, "/wifi"))
    Serial.printf("stream wifi begin error, %s\n\n", fbdo_wf.errorReason().c_str());
  if(!Firebase.beginStream(fbdo_dt, "/data"))
    Serial.printf("stream data begin error, %s\n\n", fbdo_wf.errorReason().c_str());
}
void init_wifi() {
  Serial.println("WiFi SSID: " + ssid);
  Serial.println("WiFi PASS: " + pass);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    xuly_local();
    xulynutbam();
    digitalWrite(2, (ledState = ledState ? 0 : 1));
    delay(100);
    // Serial.println(".");
    // delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
void init_eeprom() {
  ssid = EEPROM.readString(0);
  pass = EEPROM.readString(100);
  //EEPROM.writeString(0, ssid);
  //EEPROM.writeString(100, pass);
  hoursta = EEPROM.readInt(132);
  hoursto = EEPROM.readInt(164);
  minsta  = EEPROM.readInt(196);
  minsto  = EEPROM.readInt(228);
  mode    = EEPROM.readInt(260);
}
void setup() 
{
  pinMode(32, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  pinMode(led,OUTPUT);
  attachInterrupt(32, ISR, FALLING);
  attachInterrupt(33, ISR2, FALLING);
  EEPROM.begin(512);
  init_eeprom();
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.begin(9600);
  //Wire.begin();
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  init_wifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
  init_firebase();
  xTaskCreatePinnedToCore(read_send_dt, "data Task", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(gettime, "doc tine", 4096, NULL, 0, NULL, 1);
}

void read_send_dt(void *param) {
  while (1) {
    streamdata();
    xulytudong();
    streamwifi();
    updatewifi();
    xulynutbam();
    if(WiFi.status() != WL_CONNECTED){
      dem++;
      digitalWrite(2, (ledState = ledState ? 0 : 1));
      if(dem == 30) {
        WiFi.begin(ssid,pass);
        dem = 0;
      }
    }
    if(WiFi.status() == WL_CONNECTED) {
      updateStatus();
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
void gettime(void *param ) {
  while(1) {
    if(WiFi.status() == WL_CONNECTED) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        currentHour = timeinfo.tm_hour;
        currentMin  = timeinfo.tm_min;
        DateTime now = rtc.now();
        if((timeinfo.tm_hour != now.hour() && timeinfo.tm_min == now.minute())|| (timeinfo.tm_hour == now.hour() && timeinfo.tm_min != now.minute())) {
          rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
          Serial.println("Đã đồng bộ time");
        }
        // demtg++;
        // if(demtg == 60) {
        //   //Serial.printf("thời gian NTP sever: %d %d", currentHour,currentMin);
        //   demtg = 0;
        // }
      } else {
        //Serial.println("Không load được NTP");
        doctime_ds1307();
      }
    } else {
      doctime_ds1307();
    }
    if(Serial2.available()){
      data = Serial2.readStringUntil('\n');
      data.trim();
      Serial.print("Nhận từ ESP32 Slave: ");
      Serial.println(data);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void loop()
{
}