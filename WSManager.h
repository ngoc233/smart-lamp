#ifndef WSManager_h
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#define WSManager_h

class WSManager
{
  public:
    WSManager();
    //
    void SETUP();

    //receive in callback function
    void SETCALLBACK();

    void LOOP();
    
    void SERVERHANDLE();
};

extern WSManager wsManager;
// check have IP end point
extern int eIP;
// check blink ,fade,random status
extern bool bStatus,fStatus,rStatus;

// share for main use
extern int red,green,blue;
extern float white;

#endif
