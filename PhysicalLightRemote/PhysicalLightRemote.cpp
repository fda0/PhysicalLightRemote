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

#define Framerate 20
#define FrameDelay (1000 / Framerate)
#define Seconds(Value) (Framerate * (Value))
#define Minutes(Value) (Seconds((Value)) * Framerate)






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


// Global Variables
Button_Map ButtonsID;
Button_Map CurrentButtons;
Button_Map LastButtons;
uint32_t CycleCounter = 0xFFFF0000;
uint32_t DiscoveryDelay = 5;

WiFiUDP Udp;
IPAddress IpMulticast(239, 255, 255, 250);
char BigBuffer[1099];






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

void SendMulticastMessage()
{
    PrintN("Sending multicast discover message");
    Udp.beginMulticast(WiFi.localIP(), IpMulticast, 1982);
    Udp.beginPacketMulticast(IpMulticast, 1982, WiFi.localIP());
    Udp.print("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1982\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb");
    Udp.endPacket();
    Udp.begin(1982);
}

int UdpRead()
{
    int packetSize = Udp.parsePacket();
    Print("Packet size: ");
    PrintN(packetSize);
    if (packetSize) 
    {
        int len = Udp.read(BigBuffer, 1099);
        if (len > 0) 
        {
            BigBuffer[len] = 0;
        }
        Print("Output >>");
        PrintRaw(BigBuffer, len);
        PrintN("<<");
    }
    return packetSize;
}

void UdpReadMultipleMessages()
{
    for (int networkReadIndex = 0; 
     networkReadIndex < 10;
     ++networkReadIndex)
    {
        if (!UdpRead())
        {
            break;
        }
    }
}


void loop() 
{
    // periodic tasks
    {
        ++CycleCounter;
        ++DiscoveryDelay;

        if (CycleCounter % Seconds(1) == 0)
        {
            PrintN("---loop---");
        }

        if (CycleCounter % Seconds(DiscoveryDelay) == 0)
        {
            SendMulticastMessage();
        }

        if (CycleCounter % Seconds(DiscoveryDelay + 1) == 0)
        {
            UdpReadMultipleMessages();
        }
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
    delay(FrameDelay); 
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
}
