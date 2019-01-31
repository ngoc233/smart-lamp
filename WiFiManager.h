#ifndef WiFiManager_h
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#define WiFiManager_h

class SetupWiFi
{
  public:
    SetupWiFi();
    //setup gpio for effects when smartconfig 
    void SETUP();
    
    // run smart config
    void RUN();
    
    // receive udp
    void RECONNECT();
    
    // return status of wifi
    bool CONNECTION();
    
    String GETMACID();
};

extern SetupWiFi wifiManager;
extern String macID;
extern bool toggleStatus;

#endif
