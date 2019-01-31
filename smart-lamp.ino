#include <EEPROM.h>

#include "WiFiManager.h"
#include "LedManager.h"
#include "WSManager.h"

// count total toggle on-off
byte countToggle;
byte toggleAdd = 1;
int timeAmountToggle = 0;
// check setting mode
bool stationStatus = false;

void resetToggle()
{
  EEPROM.write(toggleAdd, 0);
  EEPROM.commit();
  stationStatus = false;
  countToggle = 0;
  Serial.println("reset");
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Setting up... ");

  EEPROM.begin(512);

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
//      EEPROM.write(2,eIP);
      EEPROM.commit();
      countToggle = 3;
      stationStatus = true;
      //remove  the saved configuration of wifi in STA mode 
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
  wifiManager.SETUP();
  wifiManager.GETMACID();
  wsManager.SETUP();
}

void loop()
{
  wsManager.LOOP();

  // have blink
  if(bStatus)
  {
    ledManager.BLINK(red,green,blue,white);
  }

  // have fade
  if(fStatus)
  {
    ledManager.FADE(red,green,blue,white);
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

  // if smartconfig done -> reset toggle
  if(toggleStatus)
  {
    toggleStatus = false;
    resetToggle();
  }
    
  // check had smart config
  if(stationStatus)
  {
    wifiManager.RUN();
    wifiManager.RECONNECT();
  }

  wsManager.SERVERHANDLE();
}
