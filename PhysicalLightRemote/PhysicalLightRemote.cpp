// -- essentials --
#include <Arduino.h>
// wifi
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
// internet
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
// our config
#include "build_config.h"


#if DEV_MODE
#define PrintN(Text) Serial.println((Text))
#define Print(Text) Serial.print((Text))
#define PrintRaw(Text, Len) Serial.write((Text), (Len))
#else
#define PrintN(Text)
#define Print(Text)
#define PrintRaw(Text, Len)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Button_Read(Value) (!digitalRead(Value))


struct Button_Map 
{
    union
    {
        int buttons[5];
        struct
        {
            int buttonA;
            int buttonB;
            int buttonC;
            int buttonD;
            int analogStick;
        };
    };
};

Button_Map ButtonsID;
Button_Map CurrentButtons;
Button_Map LastButtons;
uint32_t GlobalCounter = 0xFFFF0000;

void ReadButtons(Button_Map *mapOutput, Button_Map *mapID)
{
    for (int buttonIndex = 0;
         buttonIndex < ArrayCount(mapID->buttons);
         ++buttonIndex)
    {
        mapOutput->buttons[buttonIndex] = Button_Read(mapID->buttons[buttonIndex]);
    }
}

int ButtonComparison(int currentState, int lastState)
{
    return currentState != lastState;
}


void loop() 
{
    if (++GlobalCounter > 60 * 5)
    {
        PrintN("---loop---");
        GlobalCounter = 0;
    }
    
    ReadButtons(&CurrentButtons, &ButtonsID);

    if(ButtonComparison(CurrentButtons.buttonA, LastButtons.buttonA))
    {
        Print("Button one change, analog = ");
        PrintN(analogRead(ButtonsID.analogStick));
    }

    if(ButtonComparison(CurrentButtons.buttonB, LastButtons.buttonB))
    {
        Print("Button two change, analog = ");
        PrintN(analogRead(ButtonsID.analogStick));
    }

    if(ButtonComparison(CurrentButtons.buttonC, LastButtons.buttonC))
    {
        Print("Button three change, analog = ");
        PrintN(analogRead(ButtonsID.analogStick));
    }

    if(ButtonComparison(CurrentButtons.buttonD, LastButtons.buttonD))
    {
        Print("Button four change, analog = ");
        PrintN(analogRead(ButtonsID.analogStick));
    }

    LastButtons = CurrentButtons;
    // limit to 60 fps
    delay(16); 
}


void setup()
{
    Serial.begin(9600);
    
#if DEV_MODE
    for (int i = 0; i < 8; i++)
    {
        Print("Setup() DEV_MODE delay: ");
        PrintN(i);
        delay(1000);
    }
#endif


    // init buttons
    {
        ButtonsID.buttonA = D1;
        ButtonsID.buttonB = D2;
        ButtonsID.buttonC = D3;
        ButtonsID.buttonD = D4;
        ButtonsID.analogStick = A0;

        pinMode(ButtonsID.buttonA, INPUT_PULLUP);
        pinMode(ButtonsID.buttonB, INPUT_PULLUP);
        pinMode(ButtonsID.buttonC, INPUT_PULLUP);
        pinMode(ButtonsID.buttonD, INPUT_PULLUP);
        pinMode(ButtonsID.analogStick, INPUT);

        ReadButtons(&CurrentButtons, &ButtonsID);
        LastButtons = CurrentButtons;
    }


#if CREATE_SERVER_TO_CONFIG_WIFI
    WiFiManager wifiManager; 
    wifiManager.autoConnect("ESP-CONF");  
#endif
}
