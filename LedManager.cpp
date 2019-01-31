#include "LedManager.h"
#include "Arduino.h"

int RED_PIN = 4;
int GREEN_PIN = 12;
int BLUE_PIN = 14 ;
int WHITE_PIN = 5 ;

int setFadeTime = 0;
int childFadeTime = 0;
int redFade,greenFade,blueFade = 0;
float whiteFade = 0;
bool resetColor = true;
//Just on or off one time
bool blinkOn = false;
bool blinkOff = false;
// update time for prepare blink mode
int setBinkTime = 0;

LedManager::LedManager(){

}

//setup pin for device
void LedManager::SETUP(){
  pinMode(RED_PIN,OUTPUT);
  pinMode(GREEN_PIN,OUTPUT);
  pinMode(BLUE_PIN,OUTPUT);
  pinMode(WHITE_PIN,OUTPUT);
}

//turn on with digital type
void LedManager::ONDILED(int gpio){
  digitalWrite(gpio,HIGH);
}

//turn off with digital type
void LedManager::OFFDILED(int gpio){
  digitalWrite(gpio,LOW);
}

// turn on with analog type
void LedManager::ONANLED(int gpio,int value){
  analogWrite(gpio,value);
}

// turn off with analog type              
void LedManager::OFFANLED(int gpio){
  analogWrite(gpio,0);
}

void LedManager::BLINK(int red,int green,int blue,int white){
  if((unsigned long) (millis() - setBinkTime) <= 500)
  {
    if(!blinkOn)
    {
      if(red != 2000)
      {
        analogWrite(RED_PIN,red);
        analogWrite(GREEN_PIN,green);
        analogWrite(BLUE_PIN,blue); 
      }
      else
      {
        analogWrite(WHITE_PIN,white);
      }
                                                                
      blinkOn = true;
      blinkOff = false;
    }               
  }else if((unsigned long) (millis() - setBinkTime) <= 500*2 && (unsigned long) (millis() - setBinkTime) > 500)
  {
    if(!blinkOff)
    {
      if(red != 2000)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
      {
        analogWrite(RED_PIN,0);
        analogWrite(GREEN_PIN,0);
        analogWrite(BLUE_PIN,0);
      }
      else
      {
        analogWrite(WHITE_PIN,0);
      }
      blinkOn = false;
      blinkOff = true;
    }
  }else if((unsigned long) (millis() - setBinkTime) >= 500*2)
  {
    setBinkTime = millis();
  }
}

void LedManager::FADE(int red, int green,int blue,int white){
  if((unsigned long) (millis() - setFadeTime) <= 3000)
  {
    // từ sáng về tối red -> 0 wemos d1
    // nếu lớn nhỏ hơn red thì cho = red
    if(resetColor)
    {
      if(red != 2000)
      {
        redFade = red;
        greenFade = green;
        blueFade = blue;
      }
      else
      {
        whiteFade = white;
      }
      resetColor =  !resetColor;
      Serial.println("reset Light");
    }
    if((unsigned long) (millis() - childFadeTime) > 30)
    {
      if(red != 2000)
      {
        if(redFade >= 0)
        {
          redFade -= red/100 +1;
        }
        if(greenFade >= green)
        {
          greenFade -= green/100 +1;
        }
        if(blueFade >= blue)
        {
          blueFade -= blue/100 +1;
        }
        analogWrite(RED_PIN,redFade);
        analogWrite(GREEN_PIN,greenFade);
        analogWrite(BLUE_PIN,blueFade); 
      }
      else
      {
        if(whiteFade >= 0)
        {
          whiteFade -= white/100 +1;
        }
        analogWrite(WHITE_PIN,whiteFade);
      }
      childFadeTime = millis();
    }
    
  }else if((unsigned long) (millis() - setFadeTime) <= 3000*2 && (unsigned long) (millis() - setFadeTime) > 3000)
  {
    // từ 1023 về red
    if(!resetColor)
    {
      if(red != 2000)
      {
        redFade = 0;
        greenFade = 0;
        blueFade = 0;
      }
      else
      { 
        whiteFade = 0;
      }
      resetColor = !resetColor;
      Serial.println("reset Dark");
    }
    if((unsigned long) (millis() - childFadeTime) > 30)
    {
      if(red != 2000)
      {
        if(redFade <= red)
        {
          redFade +=  red/100 +1;
        }
        if(greenFade <= green)
        {
          greenFade += green/100 +1;
        }
        if(blueFade <= blue)
        {
          blueFade += blue/100 +1;
        }
        analogWrite(RED_PIN,redFade);
        analogWrite(GREEN_PIN,greenFade);
        analogWrite(BLUE_PIN,blueFade); 
      }
      else
      {
        if(whiteFade <= white)
        {
          whiteFade +=  white/100 +1;
        }
        analogWrite(WHITE_PIN,whiteFade);
      }
      childFadeTime = millis();
    }
   
  }else if((unsigned long) (millis() - setFadeTime) >= 3000*2)
  {
    setFadeTime = millis();
  }
}

LedManager ledManager = LedManager();
