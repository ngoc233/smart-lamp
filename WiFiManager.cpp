#include "WiFiManager.h"
#include "LedManager.h"
#include "WSManager.h"

WiFiUDP Udp;

const uint8_t timeLimit = 10;
String macID = macID;
bool toggleStatus;

SetupWiFi::SetupWiFi(){
  
}

String SetupWiFi::GETMACID(){
  //get mac address
  macID = WiFi.macAddress();
  //remove ":" paramater
  macID.replace(":","");
  Serial.println(macID);
  return macID;
}

void SetupWiFi::SETUP(){
  ledManager.SETUP();
}

void SetupWiFi::RUN(){
  //Mode wifi is station
  WiFi.mode(WIFI_STA);
  //Chờ kết nối
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.beginSmartConfig();
    while (1) {
      ledManager.ONDILED(WHITE_PIN);
      delay(500);
      ledManager.OFFDILED(WHITE_PIN);
      delay(500);
      //Kiểm tra kết nối thành công in thông báo
      if (WiFi.smartConfigDone()) {
        ledManager.ONDILED(WHITE_PIN);
        Serial.println("SmartConfig Success");
        wifiManager.GETMACID();
        toggleStatus = true;
        wsManager.SETUP();
        break;
      }
    }

    Serial.println("");

    WiFi.printDiag(Serial);

    // Khởi tạo server
    Udp.begin(49999);
    Serial.println("Server started");

    // In địa chỉ IP
    Serial.println(WiFi.localIP());

  }

}

// SETUP UDP
void SetupWiFi::RECONNECT(){
    // Nhận gói tin gửi từ ESPTouch
    Udp.parsePacket();
    //In IP của ESP8266
    while (Udp.available()) {
      Serial.println(Udp.remoteIP());
      Udp.flush();
      delay(5);
    }
    // Kiểm tra kết nối
}

bool SetupWiFi::CONNECTION(){
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }
  else{
    return false;
  }
  
}


SetupWiFi wifiManager = SetupWiFi();
