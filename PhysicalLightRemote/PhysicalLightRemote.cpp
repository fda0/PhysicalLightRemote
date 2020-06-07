// -- essentials --
#include <Arduino.h>
// wifi
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <EEPROM.h>
// our files
#include "PLR_config.h"
#include "PhysicalLightRemote.h"
#include "PLR_string.h"
#include "PLR_network.h"
#include "PLR_light.h"
#include "PLR_button.h"
#include "PLR_color_flow.h"
#include "PLR_command.h"
#include "PLR_menu.h"


// Global Variables
namespace Global
{
    Button_Map Buttons;
    Light_Collection LightCollection;
    Network_Clients NetworkClients;
    Menu_State Menu;
    WiFiUDP Udp;
    Save_State Save;
    Long_Effect LongEffect;
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
            ReadButtons(&Buttons, currentTimestamp, true);
            CollectAnalogSamples(&Buttons.stick);
        }
        else
        {
            ReadButtons(&Buttons, currentTimestamp, false);
        }

        if(DigitalButtonComparison(&Buttons.arrowA, currentTimestamp))
        {
            ChangePage(&Save, &Menu, &LightCollection, 
                       &NetworkClients, &LongEffect, false);
        }

        if(DigitalButtonComparison(&Buttons.arrowB, currentTimestamp))
        {
            ChangePage(&Save, &Menu, &LightCollection, 
                       &NetworkClients, &LongEffect, true);
        }

        if(DigitalButtonComparison(&Buttons.buttonA, currentTimestamp))
        {
            // color change for page 0
            SetMode(&Save, &Menu, &LightCollection, 
                    &NetworkClients, &LongEffect, ModeA);
        }

        if(DigitalButtonComparison(&Buttons.buttonB, currentTimestamp))
        {
            // brightness for page 0
            SetMode(&Save, &Menu, &LightCollection,  
                    &NetworkClients, &LongEffect, ModeB);
        }

        if(DigitalButtonComparison(&Buttons.buttonC, currentTimestamp))
        {
            // color temp for page 0
            SetMode(&Save, &Menu, &LightCollection,   
                    &NetworkClients, &LongEffect, ModeC);
        }

        if(DigitalButtonComparison(&Buttons.buttonD, currentTimestamp))
        {
            // power for page 0

            if ((Menu.page == 0) &&
                (LightCollection.currentLightCount > 0) &&
                ((!LightCollection.hasAnyWhiteLight) || (Menu.mode == ModeD)))
            {
                float simulatedAnalog = 
                    LightCollection.lights[0].isPowered ? 0.0f : 1.0f;

                CommandChangePowerState(&LightCollection, &NetworkClients, 
                                        Menu.smoothness, simulatedAnalog);
            }

            SetMode(&Save, &Menu, &LightCollection,   
                    &NetworkClients, &LongEffect, ModeD);
        }

        if (TimeElapsed(LastAnalogCalculationCycle) > 125)
        {
            LastAnalogCalculationCycle = currentTimestamp;

            CalculateAnalogValue(&Buttons.stick);
            if (AnalogButtonComparison(&Buttons.stick, &Menu))
            {
                Print("Analog button change: ");
                PrintN(Buttons.stick.value);
                ProcessAnalogChange(&Menu, &LightCollection, 
                                    &NetworkClients, Buttons.stick.value);
            }
        }
    }




    // long effects
    if (TimeElapsed(LongEffect.lastCycleTimestamp) > LongColorStepMS)
    {
        LongEffect.lastCycleTimestamp = currentTimestamp;

        switch (LongEffect.active)
        {
            case EffectRandomColors:
            {
                CommandProcessRandomColorsStep(&LightCollection, &NetworkClients, 
                                               &Menu, &LongEffect);
            } break;
        }
    }


}



void setup()
{
    using namespace Global;

    Serial.begin(9600);
    EEPROM.begin(sizeof(Save_State));
    
#if DEV_SLOW
    for (int i = 0; i < 8; i++)
    {
        Print("Setup() DEV_SLOW delay: ");
        PrintN(i);
        delay(1000);
    }
#endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // delay for DHCP
    delay(1000); 


#if CREATE_SERVER_TO_CONFIG_WIFI
    WiFiManager wifiManager; 
    wifiManager.autoConnect("YEELIGHT-REMOTE-CONF");  
#endif

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected to the WiFi network");

    // init buttons
    {
        Buttons.arrowA.key  = D2;
        Buttons.arrowB.key  = D1; 
        Buttons.buttonA.key = D5;
        Buttons.buttonB.key = D7;
        Buttons.buttonC.key = D4;
        Buttons.buttonD.key = D6;
        Buttons.stick.key   = A0;

        pinMode(Buttons.arrowA.key, INPUT_PULLUP);
        pinMode(Buttons.arrowB.key, INPUT_PULLUP);
        pinMode(Buttons.buttonA.key, INPUT_PULLUP);
        pinMode(Buttons.buttonB.key, INPUT_PULLUP);
        pinMode(Buttons.buttonC.key, INPUT_PULLUP);
        pinMode(Buttons.buttonD.key, INPUT_PULLUP);
        pinMode(Buttons.stick.key, INPUT);

        uint32_t timestamp = millis();
        ReadButtons(&Buttons, timestamp, true);
        CollectAnalogSamples(&Buttons.stick);
        CalculateAnalogValue(&Buttons.stick);
    }

    randomSeed(Buttons.stick.value);

    uint32_t currentTime = millis();
    LastUdpMessageSentCycle = currentTime;
    LastAnalogCalculationCycle = currentTime;
    LastButtonMeasurementCycle = currentTime;

    LoadStateFromMemory(&Save, &Menu);
    if (Save.firstTime != 0xCAFECAFE)
    {
        Print("Loaded first EEPROM config, ");
        PrintN(Save.firstTime);

        Save.firstTime = 0xCAFECAFE;
        Menu.smoothness = 500;
    }
}
