// #include <HardwareSerial.h>

// HardwareSerial ESP32Serial2(2);  // UART2

// void setup() {
//   Serial.begin(9600);  // Debug
//   ESP32Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17
//   Serial.println("ESP32 Sender ready!");
// }

// void loop() {
//   ESP32Serial2.println("on");
//   delay(1000);
//   ESP32Serial2.println("off");
//   delay(1000);
//   // if (ESP32Serial2.available()) {
//   //   String data = ESP32Serial2.readStringUntil('\n');
//   //   Serial.print("Nhận từ ESP32 Slave: ");
//   //   data.trim();
//   //   Serial.println(data);
//   // }
// }
void setup() {
  Serial.begin(9600);  // Debug
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17
  Serial.println("ESP32 Sender ready!");
}

void loop() {
  Serial2.println("on");
  delay(1000);
  Serial2.println("off");
  delay(1000);
  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    Serial.print("Nhận từ ESP32 Slave: ");
    data.trim();
    Serial.println(data);
  }
}