// -- essentials --
#include <Arduino.h>
// wifi
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
// our files
#include "PLR_config.h"
#include "PhysicalLightRemote.h"
#include "PLR_string.h"
#include "PLR_network.h"
#include "PLR_light.h"
#include "PLR_button.h"
#include "PLR_menu.h"


// Global Variables
namespace Global
{
    Button_Map Buttons;
    Light_Collection LightCollection;
    Network_Clients NetworkClients;
    Menu_State Menu;
    WiFiUDP Udp;
    IPAddress IpMulticast(239, 255, 255, 250);
    uint32_t CycleCounter;

    uint32_t UdpRefreshPeriodMs = 1000;
    bool MulticastMessageSent;
    uint32_t LastUdpMessageSentCycle;
    uint32_t LastButtonMeasurementCycle;
    uint32_t LastAnalogCalculationCycle;
 

    uint32_t DebugLastCycle;
    uint32_t DebugFrameCount;
}



#define TimeElapsed(LastTime) (currentTimestamp - (LastTime))
void loop() 
{
    using namespace Global;
    uint32_t currentTimestamp = millis();
    CycleCounter += 1;
    DebugFrameCount += 1;

    // periodic udp mutlicast task
    {
        if (TimeElapsed(LastUdpMessageSentCycle) > (UdpRefreshPeriodMs - 1000) && !MulticastMessageSent)
        {
            SendMulticastMessage(&Udp, &IpMulticast);
            MulticastMessageSent = true;
        }

        if (TimeElapsed(LastUdpMessageSentCycle) > UdpRefreshPeriodMs)
        {
            UdpReadMultipleMessages(&Udp, &LightCollection);

            MulticastMessageSent = false;
            LastUdpMessageSentCycle = currentTimestamp;
            UdpRefreshPeriodMs += (1000 * LightCollection.currentLightCount);

            // debug
            if (DebugFrameCount > 0)
            {
                Print("Average frame time: ");
                Print(TimeElapsed(DebugLastCycle) / DebugFrameCount);
                Print("ms [(");
                Print(currentTimestamp);
                Print(" - ");
                Print(DebugLastCycle);
                Print(") / ");
                Print(DebugFrameCount);
                PrintN("]");

                DebugLastCycle = currentTimestamp;
                DebugFrameCount = 0;
            }
        }
    }
    
    // button handling
    {
        if (TimeElapsed(LastButtonMeasurementCycle) > 2)
        {
            LastButtonMeasurementCycle = currentTimestamp;
            ReadButtons(&Buttons, currentTimestamp);
            CollectAnalogSamples(&Buttons.stick);
        }

        if(DigitalButtonComparison(&Buttons.buttonA, currentTimestamp))
        {
            SetMode(&Menu, ModeA);
        }

        if(DigitalButtonComparison(&Buttons.buttonB, currentTimestamp))
        {
            SetMode(&Menu, ModeB);
        }

        if(DigitalButtonComparison(&Buttons.buttonC, currentTimestamp))
        {
            SetMode(&Menu, ModeC);
        }

        if(DigitalButtonComparison(&Buttons.buttonD, currentTimestamp))
        {
            SetMode(&Menu, ModeD);
        }

        if (TimeElapsed(LastAnalogCalculationCycle) > 125)
        {
            LastAnalogCalculationCycle = currentTimestamp;

            CalculateAnalogValue(&Buttons.stick);
            if (AnalogButtonComparison(&Buttons.stick))
            {
                Print("Analog button change: ");
                PrintN(Buttons.stick.value);
                ProcessAnalogChange(&Menu, &LightCollection, 
                                    &NetworkClients, Buttons.stick.value);
            }
        }
    }
}



void setup()
{
    using namespace Global;

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
        Buttons.buttonA.key = D1;
        Buttons.buttonB.key = D2;
        Buttons.buttonC.key = D3;
        Buttons.buttonD.key = D4;
        Buttons.stick.key = A0;

        pinMode(Buttons.buttonA.key, INPUT_PULLUP);
        pinMode(Buttons.buttonB.key, INPUT_PULLUP);
        pinMode(Buttons.buttonC.key, INPUT_PULLUP);
        pinMode(Buttons.buttonD.key, INPUT_PULLUP);
        pinMode(Buttons.stick.key, INPUT);

        uint32_t timestamp = millis();
        ReadButtons(&Buttons, timestamp);
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // delay for DHCP
    delay(1000); 


#if CREATE_SERVER_TO_CONFIG_WIFI
    WiFiManager wifiManager; 
    wifiManager.autoConnect("ESP-CONF");  
#endif


    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the WiFi network");

    uint32_t currentTime = millis();
    LastUdpMessageSentCycle = currentTime;
    LastAnalogCalculationCycle = currentTime;
    LastButtonMeasurementCycle = currentTime;
}
