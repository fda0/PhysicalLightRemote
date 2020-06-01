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
#define PrintRaw(Text, Len) Serial.write((char *)(Text), (Len))
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


namespace Yeelight
{
    const char SetPower[] = "set_power";
    const char SetBright[] = "set_bright"; 
};



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


enum Light_Type
{
    Mono,
    Color
};

struct Features
{
    bool setPower;
    bool setBright;
};

struct Light
{
    Light_Type type;
    char ipAddress[16];
    Features features;
    bool isPowered;
};

struct Light_Collection
{
    Light lights[32];
    int lightCount;
};


// Global Variables
Button_Map ButtonsID;
Button_Map CurrentButtons;
Button_Map LastButtons;
Light_Collection LightCollection;
uint32_t CycleCounter = 0xFFFF0000;


WiFiUDP GlobalUdp;
WiFiClient GlobalClient;
IPAddress IpMulticast(239, 255, 255, 250);

#define MediumBufferSize 512
#define BigBufferSize 1024
char BigBuffer[BigBufferSize];


int FindFirstOf(const char* inputStr, const char* searchStr, int maxSearchRange = 2048)
{
    int input = 0;
    while ((inputStr[input] != 0))
    {
        int inputCopy = input;
        int search = 0;

        while (inputStr[inputCopy] == searchStr[search])
        {
            ++inputCopy;
            ++search;

            if (searchStr[search] == 0)
            {
                return input;
            }
        }

        if (inputCopy > maxSearchRange)
        {
            return -1;
        }
        
        ++input;
    }
    return -1;
}

void CatString(char *output, const char *source, int startPos, int length)
{
    for (int i = startPos; i < (startPos + length); ++i)
    {
        output[i] = source[i];
    }

    if (output[length - 1] != 0)
    {
        output[length] = 0;
    }
}

void CopyString(char *output, const char *source)
{
    while (*source != 0)
    {
        *output = *source;
        ++output;
        ++source;
    }
    *output = 0;
}


bool AreStringIdentical(const char *a, const char *b)
{
    while (*a == *b)
    {
        if (*a == 0)
        {
            return true;
        }
        
        ++a;
        ++b;
    }

    return false;
}


void FillLightData(int lightIndex, const char *ipBuffer, Features *featuresBuffer, bool isPowered)
{
    using namespace Yeelight;    
    Light *light = &LightCollection.lights[lightIndex];
    
    CopyString(light->ipAddress, ipBuffer);
    light->features = *featuresBuffer;
    light->isPowered = isPowered;

    Print("Adding new light with IP: ");
    Print(light->ipAddress);
    Print(", features: [");
    if (light->features.setPower)
    {
        Print(SetPower);
        Print(", ");   
    }
    if (light->features.setBright)
    {
        Print(SetBright);
        Print(", ");   
    }
    Print("], Power state: ");
    PrintN(light->isPowered);
}

void ParseUdpRead(const char *buffer)
{
    const char yeelightTag[] = "yeelight://";
    const char colonTag[] = ":";
    const char supportTag[] = "support:";
    const char powerTag[] = "power:";

    int addressOffset = FindFirstOf(buffer, yeelightTag);
    if (addressOffset != -1)
    {
        buffer += addressOffset + sizeof(yeelightTag) - 1;

        int colonOffset = FindFirstOf(buffer, colonTag);
        if (colonOffset != -1 && colonOffset < 16)
        {
            char ipBuffer[16];
            CatString(ipBuffer, buffer, 0, colonOffset);

            Print("Ip buffer: ");
            PrintN(ipBuffer);

            for (int lightIndex = 0; 
                 lightIndex < LightCollection.lightCount;
                 ++lightIndex)
            {
                if (AreStringIdentical(ipBuffer, LightCollection.lights[lightIndex].ipAddress))
                {
                    return;
                }
            }

            int supportOffset = FindFirstOf(buffer, supportTag);
            if (supportOffset != -1)
            {
                buffer += supportOffset + sizeof(supportTag) - 1;
                int powerOffset = FindFirstOf(buffer, powerTag);

                if (powerOffset != -1)
                {
                    using namespace Yeelight;

                    Features featuresBuffer = {0};
                    featuresBuffer.setPower = (FindFirstOf(buffer, SetPower, powerOffset) != -1);
                    featuresBuffer.setBright = (FindFirstOf(buffer, SetBright, powerOffset) != -1);

                    buffer += powerOffset + sizeof(powerTag) + 1;
                    bool isPowered = (*buffer == 'n');
                    FillLightData(LightCollection.lightCount, ipBuffer, &featuresBuffer, isPowered);
                    ++LightCollection.lightCount;
                }

            }

        }
    }
}

int UdpRead()
{
    int packetSize = GlobalUdp.parsePacket();
    Print("Packet size: ");
    PrintN(packetSize);
    if (packetSize) 
    {
        int readLength = GlobalUdp.read(BigBuffer, BigBufferSize);
        if (readLength > 0)
        {
            BigBuffer[readLength] = 0;
            ParseUdpRead(BigBuffer);
        }
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

void SendMulticastMessage()
{
    PrintN("Sending multicast discover message");
    GlobalUdp.beginMulticast(WiFi.localIP(), IpMulticast, 1982);
    GlobalUdp.beginPacketMulticast(IpMulticast, 1982, WiFi.localIP());
    GlobalUdp.print("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1982\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb");
    GlobalUdp.endPacket();
    GlobalUdp.begin(1982);
}


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
    sprintf(BigBuffer, "{\"id\": 1, \"method\": \"%s\", \"params\":[%s]}", method, params);

    if (GlobalClient.connect(light->ipAddress, (uint16_t)55443)) 
    {
        GlobalClient.println(BigBuffer);
    }

    String result = "";
    while (GlobalClient.connected()) 
    {
        result = GlobalClient.readStringUntil('\r');
        Print("result test = ");
        PrintN(result);
        GlobalClient.stop();
    }
}

void ButtonToggleLights()
{
    for (int lightIndex = 0;
         lightIndex < LightCollection.lightCount;
         ++lightIndex)
    {
        Light *light = &LightCollection.lights[lightIndex];
        
        char paramBuffer[MediumBufferSize];
        sprintf(paramBuffer, "\"%s\", \"smooth\", 500", (light->isPowered ? "off" : "on"));
        SendCommand(light, Yeelight::SetPower, paramBuffer);
        light->isPowered = !(light->isPowered); 
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
            SendMulticastMessage();
        }

        if (((CycleCounter - Seconds(1)) % Seconds(5)) == 0)
        {
            UdpReadMultipleMessages();
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

    SendMulticastMessage();
    delay(Seconds(1));
    UdpReadMultipleMessages();
}
