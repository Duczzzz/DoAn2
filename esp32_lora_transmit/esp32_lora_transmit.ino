//ban thuong
// void setup() {
//   Serial.begin(9600);
//   pinMode(2,OUTPUT);
//   pinMode(4,OUTPUT);
//   digitalWrite(4,1);
//   Serial2.begin(9600, SERIAL_8N1, 16, 17);
//   Serial.println("ESP32 Receiver ready!");
// }

// void loop() {
//   if (Serial2.available()) {
//     String data = Serial2.readStringUntil('\n');
//     data.trim();
//     Serial.print("Nhận từ ESP32 Master: ");
//     Serial.println(data);
//     if(data == "on") {
//       digitalWrite(2,HIGH);
//       digitalWrite(4,0);
//       Serial2.println("on");
//     }
//     else if(data == "off") {
//       digitalWrite(2,LOW);
//       digitalWrite(4,1);
//       Serial2.println("off");
//     }
//   }
// }

// deepsleep
void sleep() {
  gpio_hold_en((gpio_num_t)4);
  gpio_hold_en((gpio_num_t)2);
  gpio_deep_sleep_hold_en();
  //esp_light_sleep_start();
  esp_deep_sleep_start();
}
#define AuxPin  GPIO_NUM_15
#define cambien  GPIO_NUM_13
void setup() {
    Serial.begin(9600);
    pinMode(2, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(cambien, INPUT);
    digitalWrite(4,1);
    pinMode(AuxPin, INPUT_PULLUP);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    esp_sleep_enable_ext0_wakeup(AuxPin, 0);
    // esp_err_t result = esp_sleep_enable_ext0_wakeup(AuxPin, 0); // 1 = wake up on HIGH signal
    // if (result == ESP_OK) {
    //     Serial.println("EXT0 Wake-Up set successfully as wake-up source.");
    // } else {
    //     Serial.println("Failed to set EXT0 Wake-Up as wake-up source.");
    // }
    delay(100);
    int gtCambien = analogRead(cambien);
    // Serial.print("Cảm biến: ");
    // Serial.print(gtCambien);
    // Serial.print("   ");

    float vol_out = (gtCambien * 3.8) / 4096.0; //3.85 - 3.9 là chuẩn
    // Serial.print("Volt out: ");
    // Serial.print(vol_out);
    // Serial.print("   ");

    float R1 = 47000.0;
    float R2 = 10000.0;
    float vol_in = vol_out / (R2 / (R1 + R2));
    Serial.print("Volt nguồn: ");
    Serial.println(vol_in);
    Serial2.println(vol_in);
}
void loop() {
  gpio_hold_dis(GPIO_NUM_2);
  gpio_hold_dis(GPIO_NUM_4);
  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim();
    // Serial.print("Nhận từ ESP32 Master: ");
    // Serial.println(data);
    if(data == "on") {
      digitalWrite(2,HIGH);
      digitalWrite(4,0);
      Serial2.println("on");
    }
    else if(data == "off") {
      digitalWrite(2,LOW);
      digitalWrite(4,1);
      Serial2.println("off");
    }
    else if(data == "ngu") {
      sleep();
    }
  }
}

// bản lightsleep
// #define AuxPin  GPIO_NUM_15

// void setup() {
//     Serial.begin(9600);
//     pinMode(2, OUTPUT);
//     pinMode(4, OUTPUT);
//     digitalWrite(4,1);
//     pinMode(AuxPin, INPUT_PULLDOWN);
//     Serial2.begin(9600, SERIAL_8N1, 16, 17);
//     esp_err_t result = esp_sleep_enable_ext0_wakeup(AuxPin, 1); // 1 = wake up on HIGH signal
//     if (result == ESP_OK) {
//         Serial.println("EXT0 Wake-Up set successfully as wake-up source.");
//     } else {
//         Serial.println("Failed to set EXT0 Wake-Up as wake-up source.");
//     }
// }
// void loop() {
//   if (Serial2.available()) {
//     String data = Serial2.readStringUntil('\n');
//     data.trim();
//     Serial.print("Nhận từ ESP32 Master: ");
//     Serial.println(data);
//     if(data == "on") {
//       digitalWrite(2,HIGH);
//       digitalWrite(4,0);
//       Serial2.println("on");
//     }
//     else if(data == "off") {
//       digitalWrite(2,LOW);
//       digitalWrite(4,1);
//       Serial2.println("off");
//     }
//   }
//   delay(1000);
//   esp_light_sleep_start();
// }