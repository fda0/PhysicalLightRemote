// -- essentials --
#include <Arduino.h>
// wifi
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
// internet
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
// our files
#include "PLR_config.h"
#include "PhysicalLightRemote.h"
#include "PLR_string.h"
#include "PLR_network.h"
#include "PLR_light.h"


// Global Variables
Button_Map ButtonsID;
Button_Map CurrentButtons;
Button_Map LastButtons;
Light_Collection GlobalLightCollection;
uint32_t CycleCounter = 0xFFFF0000;

WiFiUDP GlobalUdp;
Network_Clients GlobalNetworkClients;
IPAddress GlobalIpMulticast(239, 255, 255, 250);




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
    return (currentState != lastState) && currentState;
}

void SendCommand(Light *light, const char *method, const char *params)
{
    char buffer[MEDIUM_BUFFER_SIZE];
    sprintf(buffer, "{\"id\": 1, \"method\": \"%s\", \"params\":[%s]}", method, params);

    // if (GlobalClient.connect(light->ipAddress, (uint16_t)55443)) 
    // {
    //     GlobalClient.println(buffer);
    // }
    
    PrintN("MESSAGE 1");
}

void ButtonToggleLights()
{
    for (int lightIndex = 0;
         lightIndex < GlobalLightCollection.currentLightCount;
         ++lightIndex)
    {
        Light *light = &GlobalLightCollection.lights[lightIndex];

        char paramBuffer[MEDIUM_BUFFER_SIZE];
        sprintf(paramBuffer, "\"%s\", \"smooth\", 500", (light->isPowered ? "off" : "on"));
        SendCommand(light, Yeelight::SetPower, paramBuffer);

        light->isPowered = !(light->isPowered);

        // while (GlobalClient.connected()) 
        // {
        //     PrintN("CLOSE 1");
        //     GlobalClient.stop();
        // }

    }
}

void loop() 
{
    // periodic tasks
    {
        ++CycleCounter;

        if ((CycleCounter % Seconds(1)) == 0)
        {
            PrintN("---loop---");
        }

        if ((CycleCounter % Seconds(5)) == 0)
        {
            SendMulticastMessage(&GlobalUdp, &GlobalIpMulticast);
        }

        if (((CycleCounter - Seconds(1)) % Seconds(5)) == 0)
        {
            UdpReadMultipleMessages(&GlobalUdp, &GlobalLightCollection);
        }
    }
    

    // button handling
    {
        ReadButtons(&CurrentButtons, &ButtonsID);

        if(ButtonComparison(CurrentButtons.buttonA, LastButtons.buttonA))
        {
            ButtonToggleLights();
            Print("[LIGHT TOGGLE] Button one change, analog = ");
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
    }

    LastButtons = CurrentButtons;
    delay(FrameDelay); 
}



void setup()
{
    Serial.begin(9600);
    
#if DEV_SLOW
    for (int i = 0; i < 8; i++)
    {
        Print("Setup() DEV_SLOW delay: ");
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

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // delay for DHCP
    delay(1000); 


#if CREATE_SERVER_TO_CONFIG_WIFI
    WiFiManager wifiManager; 
    wifiManager.autoConnect("ESP-CONF");  
#endif


    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the WiFi network");

    SendMulticastMessage(&GlobalUdp, &GlobalIpMulticast);
    delay(Seconds(1));
    UdpReadMultipleMessages(&GlobalUdp, &GlobalLightCollection);
}
