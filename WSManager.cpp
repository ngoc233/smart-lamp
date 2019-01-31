#include "WSManager.h"
#include "WiFiManager.h"
#include "LedManager.h"

#include <EEPROM.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// check client connect 
uint8_t appClient = 0; 

int eIPManager;

int red,green,blue;
float white;
float brightness; 

bool bStatus,fStatus,rStatus;

WSManager::WSManager(){

}

//convert hex to decimal
unsigned int hexToDec(String hexString) {
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  appClient = num;
  switch(type) {
      case WStype_DISCONNECTED:
          Serial.printf("[%u] Disconnected!\n", num);
          break;
      case WStype_CONNECTED: {
      }
          break;
      case WStype_TEXT:
          Serial.printf("[%u] get Text: %s\n", num, payload);
          if(payload[0] == '{')
          {
            StaticJsonBuffer<500> jsonBuffer;
            JsonObject& clientData = jsonBuffer.parseObject(payload);
            
            // check mode  
            String modeString = clientData["mode"];
            int checkMode = modeString.length();
            
            // make list devices 
            JsonArray& arrayCD = (clientData["id"]);

            for(int i=0; i<arrayCD.size();i++){
               String deviceID = arrayCD[i]["idDevice"];
               Serial.println(deviceID);
               if(deviceID == macID)
               {
                  Serial.println("true ID");
                  if(checkMode >= 0)
                  {
                     switch(int(clientData["mode"]))
                     {
                      case -1:
                      {
                            eIPManager = (int)clientData["eIP"];
                            EEPROM.write(2,eIPManager);
                            EEPROM.commit();
                            Serial.print("save ip to rom :");
                            Serial.println(eIPManager);
                            wsManager.SETUP();
                      }
                      break;
                      case 1:
                            bStatus =false;
                            fStatus =false;
                            rStatus = false;
                            Serial.println("turn off");
                            ledManager.OFFANLED(RED_PIN);
                            ledManager.OFFANLED(GREEN_PIN);
                            ledManager.OFFANLED(BLUE_PIN);
                            ledManager.OFFANLED(WHITE_PIN);
                      break;
                      case 2:
                            bStatus =false;
                            fStatus =false;
                            rStatus = false;
                            Serial.println("turn on");
                            if(red == 2000)
                            {
                              ledManager.ONANLED(WHITE_PIN,white);
                            }
                            else
                            {
                              ledManager.ONANLED(RED_PIN,red); 
                              ledManager.ONANLED(GREEN_PIN,green);
                              ledManager.ONANLED(BLUE_PIN,blue);
                            }
                      break;
                      case 3:
                      {
                            String checkColor = clientData["color"].asString();
                            String hexColor = clientData["color"].asString();
                            // convert hex to dec , but esp8266 PWM max is 0 , min is 1023
                            red = hexToDec(hexColor.substring(0,2)) *4  ;
                            green = hexToDec(hexColor.substring(2,4)) *4 ;
                            blue = hexToDec(hexColor.substring(4,6)) *4 ;
                            bStatus =false;
                            fStatus =false;
                            rStatus = false;
                            // decode rgb data
                            if(red == 2000)
                            {
                              ledManager.ONANLED(WHITE_PIN,white);
                              ledManager.OFFANLED(RED_PIN);
                              ledManager.OFFANLED(GREEN_PIN);
                              ledManager.OFFANLED(BLUE_PIN);
                            }
                            else
                            {
                              ledManager.OFFANLED(WHITE_PIN);
                              ledManager.ONANLED(RED_PIN,red);
                              ledManager.ONANLED(GREEN_PIN,green);
                              ledManager.ONANLED(BLUE_PIN,blue);
                            }
                            break;
                      }
                      case 4:
                            bStatus = false;
                            fStatus = false;
                            rStatus = false;
                            brightness = (int)clientData["brightness"];
                            ledManager.OFFANLED(RED_PIN);
                            ledManager.OFFANLED(GREEN_PIN);
                            ledManager.OFFANLED(BLUE_PIN);
                            // remove rgb
                            red = 2000;
                            white = (10.23 * brightness);
                            ledManager.ONANLED(WHITE_PIN,white);
                            Serial.print("den trang sang voi gia tri");
                            Serial.println(white);
                      break;
                            
                      case 5:
                            bStatus = true;
                            fStatus =false;
                            rStatus = false;
                            Serial.println("blink mode");
                      break;
                            
                      case 6:
                            fStatus = true;
                            bStatus = false;
                            rStatus = false;
                            Serial.println("fade mode");
                            Serial.println(fStatus);
                      break;
    
                      case 7:
                            rStatus = true;
                            bStatus = false;
                            fStatus = false;
                            Serial.println("random mode");
                            Serial.println(rStatus);
                      break;
                      
                      case 8:
                           Serial.println("bat dau hen gio");
                      break;
                      
                     }
                    
                  }
                  else
                  {
                    Serial.println("không có mode");  
                  }
               }
               else{
                Serial.println("khong dung macID");
                Serial.print("mac ID la ");
                Serial.println(macID);
               }
            }
          }else
          {
            Serial.println("k phai json");
          }
          
          break;
  }
}

void WSManager::SETUP(){
  eIPManager = EEPROM.read(2);
  Serial.print("gia tri end point IP la : ");
  Serial.println(eIPManager);
  if(eIPManager != 255 && eIPManager != 0)
  {
       delay(7000);
       Serial.println(eIPManager);
       const IPAddress&  demo = WiFi.localIP();
     
       //setup getway
       int gateway0 = int(demo[0]);
       int gateway1 = int(demo[1]);
       int gateway2 = int(demo[2]);
       int gateway3 = 1;
       IPAddress gateway(gateway0,gateway1,gateway2,gateway3);
      
       //setup ip
       IPAddress ip(gateway0, gateway1, gateway2, eIPManager);
      
       //setup subnet
       IPAddress subnet(255, 255, 255, 0);

       String ssid = WiFi.SSID();
       String pwd  = WiFi.psk();
       
       Serial.println(gateway);
       Serial.println(ip);
       Serial.println(subnet);
       Serial.println(ssid);
       Serial.println(pwd);

       Serial1.println("WiFi delete all config...");
       // delete all existing WiFi stuff in EEPROM
       WiFi.disconnect();
       WiFi.softAPdisconnect();
       WiFi.mode(WIFI_OFF);
       delay(500);

       Serial1.println("WiFi set new config...");
       // connect static
       WiFi.mode(WIFI_STA);
       WiFi.config(ip,gateway,subnet,gateway);
       WiFi.begin(ssid.c_str(),pwd.c_str());

       Serial1.println("Connecting ...");
       while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial1.print(".");
       }

       Serial1.println("");
       Serial1.println("WiFi connected");
       Serial1.println("IP address: ");
       Serial1.println(WiFi.localIP());
  }
  else
  {
    Serial.println("k co last IP");
  }

  delay(500);
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if(MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  
  server.begin();
  
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void WSManager::LOOP(){
  webSocket.loop();
}

void WSManager::SERVERHANDLE(){
  server.handleClient();  
}

WSManager wsManager = WSManager();
int eIP = eIPManager;
