#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

//wemos D1 R1
#define RED_PIN 12  //(D1)
#define GREEN_PIN 14 //(D2)  
#define BLUE_PIN 16 //(D3) 
//
//#define RED_PIN 4  //(D1)
//#define GREEN_PIN 5 //(D2)  
//#define BLUE_PIN 0 //(D3) 

int red = 207;
int green = 939;
int blue = 623;

String color = "#ffffff";
String modeLamp  = "blink";

int batteryLife = 5;
  
String ipCheck;
int eIP;

// check client connect 
uint8_t appClient = 0; 

//int getWayArray[1] = {0};

//status setup json
String modeServer = "on";
String colorServer = "#ffffff";
int blinkServer = 5;
int speedServer = 0;
int setBinkTime = 0;

// check blink status
bool bStatus = false;
bool blinkOn = false;
bool blinkOff = false;

// check fade status
bool fStatus = false;

// check status  when led-on or led-off
bool statusOn = false;
bool statusOff = false;

//random status
bool randomStatus = false;

bool statusTimer = false;
//time of timer
int tTimer1;

//time check of timer
int setTTimer1 = 0;

// status of timer
bool sTimer1;
//IPAddress ip(192, 168, 0, 233);

// count total toggle on-off
byte countToggle;
byte toggleAdd = 1;
int timeAmountToggle = 0;

// check setting mode
bool stationStatus = false;

WiFiUDP Udp;


void resetToggle()
{
  EEPROM.write(toggleAdd, 0);
  EEPROM.commit();
  stationStatus = false;
  countToggle = 0;
  Serial.println("reset");
}

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//time schedule for send battery life to client
bool sendBatteryLife() {
  if((unsigned long) (millis() - speedServer) > 30000)
  {
    speedServer = millis();
    return true;  
  }
  return false;
}

//mode blink mode
bool setTimeBlink() {
  if((unsigned long) (millis() - setBinkTime) <= 500)
  {
    return true;  
  }else if((unsigned long) (millis() - setBinkTime) <= 500*2 && (unsigned long) (millis() - setBinkTime) > 500)
  {
    return false;
  }else if((unsigned long) (millis() - setBinkTime) >= 500*2)
  {
    setBinkTime = millis();
  }
}

int setFadeTime = 0;
int childFadeTime = 0;
int redFade,greenFade,blueFade = 0;
bool resetColor = true;

//fade mod function
void fadeColor()
{
  if((unsigned long) (millis() - setFadeTime) <= 3000)
  {
    // từ sáng về tối red -> 0 wemos d1
    // nếu lớn nhỏ hơn red thì cho = red
    if(resetColor)
    {
      redFade = red;
      greenFade = green;
      blueFade = blue;
      resetColor =  !resetColor;
      Serial.println("reset Light");
    }
    if((unsigned long) (millis() - childFadeTime) > 30)
    {
      redFade -= red/100 +1;
      greenFade -= green/100 +1;
      blueFade -= blue/100 +1;
      childFadeTime = millis();
      analogWrite(RED_PIN,redFade);
      analogWrite(GREEN_PIN,greenFade);
      analogWrite(BLUE_PIN,blueFade);
      Serial.println("toi dan");
      Serial.println(redFade);
    }
    
  }else if((unsigned long) (millis() - setFadeTime) <= 3000*2 && (unsigned long) (millis() - setFadeTime) > 3000)
  {
    // từ 1023 về red
    if(!resetColor)
    {
      redFade = 0;
      greenFade = 0;
      blueFade = 0;
      resetColor = !resetColor;
      Serial.println("reset Dark");
    }
    if((unsigned long) (millis() - childFadeTime) > 30)
    {
      redFade +=  red/100 +1;
      greenFade += green/100 +1;
      blueFade += blue/100 +1;
      childFadeTime = millis();
      analogWrite(RED_PIN,redFade);
      analogWrite(GREEN_PIN,greenFade);
      analogWrite(BLUE_PIN,blueFade);
      Serial.println("sang dan");
      Serial.println(redFade);
    }
   
  }else if((unsigned long) (millis() - setFadeTime) >= 3000*2)
  {
    setFadeTime = millis();
  }
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


String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void setupWiFi()
{
  //Mode wifi là station
  WiFi.mode(WIFI_STA);
  //Chờ kết nối
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("bat den");
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
      WiFi.beginSmartConfig();
      while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("bat den");
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("tat den ");
        delay(500);
        //Kiểm tra kết nối thành công in thông báo
        if (WiFi.smartConfigDone()) {
          Serial.println("SmartConfig Success");
          resetToggle();
          setupWS();
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

void setupWS()
{

  eIP = EEPROM.read(2);
  Serial.println(eIP);
  if(eIP != 255 && eIP != 0)
  {
     delay(7000);
//     Serial.print("co last ip");
       Serial.println(eIP);
       const IPAddress&  demo = WiFi.localIP();
     
       //setup getway
       int gateway0 = int(demo[0]);
       int gateway1 = int(demo[1]);
       int gateway2 = int(demo[2]);
       int gateway3 = 1;
       IPAddress gateway(gateway0,gateway1,gateway2,gateway3);
      
       //setup ip
       IPAddress ip(gateway0, gateway1, gateway2, eIP);
      
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
//       WiFi.config(IPAddress(192,168,0,233),IPAddress(192,168,0,1),IPAddress(255,255,255,0),IPAddress(192,168,0,1));
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
  
  // handle index
//  server.on("/", []() {
//    // send index.html
//    server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/></body></html>");
//  });
  
  server.begin();
  
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Setting up... ");

  EEPROM.begin(512);
  
  //Engine channels  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);

  countToggle = EEPROM.read(toggleAdd);
  switch (countToggle)
  {
    case 0 :
      EEPROM.write(toggleAdd, 1);
      EEPROM.commit();
      countToggle = 1;
      break;
    case 1 :
      EEPROM.write(toggleAdd, 2);
      EEPROM.commit();
      countToggle = 2;
      break;
    case 2 :
      EEPROM.write(toggleAdd, 3);
      EEPROM.write(2,eIP);
      EEPROM.commit();
      countToggle = 3;
      stationStatus = true;
      //erases  the saved configuration of wifi in STA mode 
      delay(500);
      WiFi.disconnect(true);
      Serial.println("chinh xac ma");
      break;
    case 3 :
      EEPROM.write(toggleAdd, 4);
      EEPROM.commit();
      countToggle = 4;
      break;
    case 4 :
      EEPROM.write(toggleAdd, 5);
      EEPROM.commit();
      countToggle = 5;
      break;
    default:
      countToggle = 6;
      break;
  }
  setupWS();
  
  //setup interrup
//  pinMode(interruptPin, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(interruptPin), saveRGB, FALLING);

}

//void saveRGB()
//{
//  EEPROM.write(10, redRom );
//  EEPROM.write(11, greenRom);
//  EEPROM.write(12, blueRom);
//  EEPROM.commit();
//}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
    appClient = num;
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            //setup json when user controll using hardware
            StaticJsonBuffer<500> jsonHardWare;
            JsonObject& hardWareControll  = jsonHardWare.createObject();
            hardWareControll["mode"] = modeServer;
            hardWareControll["color"] = colorServer;
            hardWareControll["blink"] =  blinkServer;
            //parse json for send data to client            
            String dataString;
            hardWareControll.printTo(dataString);
            webSocket.sendTXT(num,dataString);  
            
        }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            if(payload[0] == '{')
            {
              StaticJsonBuffer<500> jsonBuffer;
              JsonObject& clientData = jsonBuffer.parseObject(payload);
              String checkColor = clientData["color"].asString();
              if(checkColor.length() > 0)
              {
                Serial.println("co mau");
                String hexColor = clientData["color"].asString();
                // convert hex to dec , but esp8266 PWM max is 0 , min is 1023
                red = hexToDec(hexColor.substring(0,2)) *4  ;
                green = hexToDec(hexColor.substring(2,4)) *4  ;
                blue = hexToDec(hexColor.substring(4,6)) *4  ;
                Serial.println(red);
                Serial.println(green);
                Serial.println(blue);
              }
              else
              {
                Serial.println("don't have color");
              }
              // check mode 
              // if turn on led 
              String modeString = clientData["mode"];
              int checkMode = modeString.length();
              if(checkMode >= 0)
              {
                 switch(int(clientData["mode"]))
                 {
                  case -1:
                        {
                          eIP = (int)clientData["eIP"];
                          EEPROM.write(2,eIP);
                          EEPROM.commit();
                          Serial.println("save ip to rom");
                          setupWS();
                        }
                  break;
                  case 1:
                        bStatus =false;
                        fStatus =false;
                        randomStatus = false;
                        analogWrite(RED_PIN,1023);
                        analogWrite(GREEN_PIN,1023);
                        analogWrite(BLUE_PIN,1023);
                  break;
                  case 2:
                        bStatus =false;
                        fStatus =false;
                        randomStatus = false;
                        analogWrite(RED_PIN,red); 
                        analogWrite(GREEN_PIN,green);
                        analogWrite(BLUE_PIN,blue);
                  break;
                  case 3:
                         bStatus =false;
                         fStatus =false;
                         randomStatus = false;
                         // decode rgb data
                         if(checkColor.length() > 0)
                         {
//                            Serial.println("co mau");
//                            String hexColor = clientData["color"].asString();
//                            red = 1023 -( hexToDec(hexColor.substring(0,2)) *4 ) ;
//                            green = 1023 -( hexToDec(hexColor.substring(2,4)) *4 ) ;
//                            blue = 1023 -( hexToDec(hexColor.substring(4,6)) *4 ) ;
//                            Serial.print("red is ");
//                            Serial.println(red);
//                            Serial.print("green is ");
//                            Serial.println(green);
//                            Serial.print("blue is ");
//                            Serial.println(blue);
                            analogWrite(RED_PIN,red); 
                            analogWrite(GREEN_PIN,green);
                            analogWrite(BLUE_PIN,blue);
                         }
                         else 
                         {
                          Serial.println("khong co mau");
                          Serial.println((const char*)clientData["color"]);
                         }
                  break;
                  
                  case 4:
                        bStatus = true;
                        fStatus =false;
                        randomStatus = false;
                  break;
                        
                  case 5:
                        fStatus = true;
                        bStatus = false;
                        randomStatus = false;
                  break;

                  case 6:
                        randomStatus = true;
                        bStatus = false;
                        fStatus = false;
                  break;
                  
                  case 7:
                       tTimer1 = (int)clientData["timer"]["minutes"];
                       sTimer1 = (bool)clientData["timer"]["status"];
                       statusTimer = true;
                       Serial.println("bat dau hen gio");
                  break;
                  
                 }
                
              }
              else
              {
                Serial.println("không có mode");  
              }
             
            }else
            {
              Serial.println("k phai json");
            }
            
            break;
    }
}


void loop()
{
  webSocket.loop();
  if(sendBatteryLife())
  {
    StaticJsonBuffer<500> jsonBattery;
    JsonObject& batteryLife  = jsonBattery.createObject();
    batteryLife["battery"] = 25;
    String batteryString;
    batteryLife.printTo(batteryString);
    webSocket.sendTXT(appClient,batteryString);   
  }

  // have blink
  if(bStatus)
  {
    if(setTimeBlink())
    {
      if(!blinkOn)
      {
        Serial.println("sang");
        Serial.println(red);
        analogWrite(RED_PIN,red);
        analogWrite(GREEN_PIN,green);
        analogWrite(BLUE_PIN,blue);
        blinkOn = true;
        blinkOff = false;
      }
    }
    else{
      if(!blinkOff)
      {
        Serial.println("toi");
        analogWrite(RED_PIN,0);
        analogWrite(GREEN_PIN,0);
        analogWrite(BLUE_PIN,0);
        blinkOn = false;
        blinkOff = true;
      }
    }
  }

  // have fade
  if(fStatus)
  {
    fadeColor();
  }

  // if over 3 seconds
  if ((unsigned long) (millis() - timeAmountToggle) > 5000 )
  {
    if (countToggle >= 1)
    {
      if (countToggle == 3)
      {
        // do notthing now
      }
      else
      {
        resetToggle();
        countToggle = 0 ;
      }
    }

  }
    
  if (stationStatus)
  {
    setupWiFi();
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
  server.handleClient();  
}
