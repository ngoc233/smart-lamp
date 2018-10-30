
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>

#define RED_PIN 5  //(D1)
#define GREEN_PIN 4 //(D2) 
#define BLUE_PIN 0  //(D3)
byte red, green, blue;
byte redBlink,greenBlink,blueBlink;

String color = "#ffffff";
String modeLamp  = "blink";
int blinkTime = 3;

int batteryLife = 5;
long rgbColor = 0;

int setUpWifi = 0;
String ipCheck;

// check client connect 
uint8_t appClient = 0; 

const char WiFiAPPSK[] = "rgb12345";      
int ipDefault = 233;
 
const char* ssid;
const char* password;

//int getWayArray[1] = {0};

//status setup json
String modeServer = "on";
String colorServer = "#ffffff";
int blinkServer = 5;
int speedServer = 0;
int setBinkTime = 3;
int speedBink = 3;
bool statusOn = false;
bool statusOff = false;
int setEffectTime = 0;

//IPAddress ip(192, 168, 0, 233);

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

//set time blink
bool setTimeBlink() {
  if((unsigned long) (millis() - setBinkTime) <= blinkTime*1000)
  {
    if(speedBink >= 1)
    {
      if((unsigned long) (millis() - setEffectTime) >= blinkTime*1000/3)
      {
        redBlink+= red - red/speedBink;
        greenBlink+= green - green/speedBink;
        blueBlink+=  blue -blue/speedBink;
        speedBink--;
        setEffectTime = millis();
      }
    }
    return true;  
  }else if((unsigned long) (millis() - setBinkTime) <= blinkTime*1000*2 && (unsigned long) (millis() - setBinkTime) > blinkTime*1000)
  {
    if(speedBink <= 0 )
    {
      speedBink = 1;
      if((unsigned long) (millis() - setEffectTime) >= blinkTime*1000/speedBink)
      {
        redBlink+= red/speedBink;
        greenBlink+= green/speedBink;
        blueBlink+= blue/speedBink;
        speedBink++;
        setEffectTime = millis();
      }
    }
    return false;
  }else if((unsigned long) (millis() - setBinkTime) >= blinkTime*1000*2)
  {
    setBinkTime = millis();
  }
  
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
  if(setUpWifi != 0)
  {
   WiFi.mode(WIFI_AP);
   WiFi.softAP("ESP8266-RGB", WiFiAPPSK);
  }
  else{      
   WiFi.mode(WIFI_STA);
   ssid = "iOTech";
   password = "iotech.vn";
   WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  

// set up subnet
//  if(int(demo[0]) == 192)
//  {
//    IPAddress subnet(255, 255, 255, 0);
//  }else if(int(demo[0]) == 172)
//  {
//    IPAddress subnet(255, 255, 0, 0);
//  }else if(int(demo[0]) == 10){
//    IPAddress subnet(255, 0, 0, 0);
//  }else
//  {
//    Serial.println("The LocalIP is not invalid ");
//  }

 const IPAddress&  demo = WiFi.localIP();
 
 //setup getway
 int gateway0 = int(demo[0]);
 int gateway1 = int(demo[1]);
 int gateway2 = int(demo[2]);
 int gateway3 = 1;
 IPAddress gateway(gateway0,gateway1,gateway2,gateway3);

 //setup ip
 IPAddress ip(gateway0, gateway1, gateway2, 233);

 //setup subnet
 IPAddress subnet(255, 255, 255, 0);
 WiFi.config(ip,gateway,subnet);
 Serial.println("");
 Serial.println("WiFi connected");
//  /Serial.println(WiFi.localIP());
   ipCheck = IpAddress2String(WiFi.localIP()); 
  } 
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Setting up... ");

  //Engine channels  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  analogWrite(RED_PIN, 1023);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  delay(1000);//wait for a second
  
  setupWiFi();

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  if(MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  
  // handle index
  server.on("/", []() {
    // send index.html
    server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/></body></html>");
  });
  
  server.begin();
  
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 1023);
  analogWrite(BLUE_PIN, 0);
}

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
            hardWareControll["speed"] = speedServer;
            //parse json for send data to client            
            String dataString;
            hardWareControll.printTo(dataString);
            webSocket.sendTXT(num,dataString);  
            
        }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            if(payload[0] == '#') {
                // we get RGB data

                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

                analogWrite(RED_PIN,    ((rgb >> 16) & 0xFF) <<2); //PWM on a ESP8266 is from 0 to 1024, so the last <<2 (we are multiplying by 4)
                analogWrite(GREEN_PIN,  ((rgb >> 8) & 0xFF) <<2);
                analogWrite(BLUE_PIN,   ((rgb >> 0) & 0xFF) <<2);                
            }else if(payload[0] == '{')
            {
              StaticJsonBuffer<500> jsonBuffer;
              JsonObject& clientData = jsonBuffer.parseObject(payload);
              
              
              if(clientData["ssid"] != NULL)
              {
                webSocket.sendTXT(num,"ok");
                ssid = clientData["ssid"];
                password = clientData["pwd"];
                ipDefault = int(clientData["ip"]);
                Serial.println(String(ipDefault));
                setUpWifi = 1; 
                setupWiFi();
              }
              else
              {
                Serial.println("khong hieu noi");
              }
              // check mode 
              // if turn on led 
              if(clientData["mode"] != NULL)
              {
                 // decode rgb data
                 if(clientData["color"].asString() != "")
                 {
                    Serial.println("co mau");
                    String hexColor = clientData["color"].asString();
                    rgbColor = (long) strtol( &hexColor[1], NULL, 16);
                    red = rgbColor >> 16;
                    green = rgbColor >> 8 & 0xFF;
                    blue = rgbColor & 0xFF;
                    Serial.print("red is ");
                    Serial.println(red);
                    Serial.print("green is ");
                    Serial.println(green);
                    Serial.print("blue is ");
                    Serial.println(blue);
                 }
                 else 
                 {
                  Serial.println("khong co mau");
                  Serial.println((const char*)clientData["color"]);
                 }
                 switch(int(clientData["mode"]))
                 {
                  case 1:
                        analogWrite(RED_PIN,255); 
                        analogWrite(GREEN_PIN,0);
                        analogWrite(BLUE_PIN,0);
                  break;
                  case 2:
                        analogWrite(RED_PIN,0);
                        analogWrite(GREEN_PIN,0);
                        analogWrite(BLUE_PIN,0);
                  break;
                  case 3:
                        // check have color
                        if(rgbColor != 0)
                        {
                          Serial.println("Hien thi mau");
                          analogWrite(RED_PIN,red);
                          analogWrite(GREEN_PIN,green);
                          analogWrite(BLUE_PIN,blue);    
                        }
                        else
                        {
                          Serial.println("không có màu RGB");
                        }

                        //check have blink
                        if(int(clientData["blink"]) != 0)
                        {
                          blinkTime = int(clientData["blink"]);
                          Serial.print("blink la : ");
                          Serial.print(blinkTime);
                          Serial.println();
                        }
                        else
                        {
                          Serial.println("không có blink");
                        }
                        // check have speed
                        if(int(clientData["speed"]) != 0)
                        {
                          
                          speedBink = int(clientData["speed"]);
                          Serial.print("speed la : ");
                          Serial.print(speedBink);
                          Serial.println();
                        }
                        else
                        {
                          speedBink = 0;
                          Serial.println("không có Speed");
                        }
                  break;
                  case 4:
                        
                  break;
                  case 5:
                  break;
                  case 6:
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

  if(blinkTime == 0)
  {
    setBinkTime = 0;
  }
  else
  {
    if(setTimeBlink())
    {
        if(!statusOn)
        {
          if(speedBink >=1 )
          {
            Serial.println("fade sang");
            Serial.println(green);
            analogWrite(RED_PIN,red);
            analogWrite(GREEN_PIN,green);
            analogWrite(BLUE_PIN,blue);
          }else{
            Serial.println("bat den");
            analogWrite(RED_PIN,red);
            analogWrite(GREEN_PIN,green);
            analogWrite(BLUE_PIN,blue);
          }
          statusOn = true;
          statusOff =false;
        }
    }
    else{
        if(!statusOff)
        {
          if(speedBink >=1 )
          {
            Serial.println("fade toi");
            Serial.println(green);
            analogWrite(RED_PIN,red);
            analogWrite(GREEN_PIN,green);
            analogWrite(BLUE_PIN,blue);
          }else{
            Serial.println("tat den");
            analogWrite(RED_PIN,LOW);
            analogWrite(GREEN_PIN,LOW);
            analogWrite(BLUE_PIN,LOW);
          }
          statusOff = true;
          statusOn = false;
        }
        
    }
    
  }
  
  
  
  server.handleClient();  
}
