void ChangePage(Menu_State *menu, bool forward)
{
    menu->mode = ModeNone;

    if (forward)
    {
        menu->page += 1;
    }
    else
    {
        menu-> page -= 1;
    }

    if (menu->page < 0)
    {
        menu->page = 0;
    }

    if (menu->page >= MenuPageCount)
    {
        menu->page = MenuPageCount - 1;
    }
}


void SetMode(Menu_State *menu, Mode mode)
{
    menu->mode = mode;
    Print("Mode changed to ");
    PrintN(mode);
}


void SetSpeed(Menu_State *menu, float analogValue)
{
    menu->speed = (int)(analogValue * 1000);

    if (menu->speed < 1)
    {
        menu->speed = 1;
    }
}


void CommandChangeBrightness(Light_Collection *lightCollection, 
                             Network_Clients *networkClients, 
                             int speed, float analogValue)
{
    int brightness = analogValue * 100;
    if (brightness < 1)
    {
        brightness = 1;
    }

    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        char paramBuffer[MediumBufferSize];
        sprintf(paramBuffer, "%d, \"smooth\", %d", 
                brightness, speed);
        
        SendCommand(networkClients, light->ipAddress, Yeelight::SetBright, paramBuffer);
    }

    CloseConnections(networkClients);
}


void CommandChangePowerState(Light_Collection *lightCollection, 
                      Network_Clients *networkClients, 
                      int speed, float analogValue)
{
    bool turnOffWhite = analogValue < 0.66f;
    bool turnOffColor = analogValue < 0.33f; 

    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];
        bool powerOn = !(turnOffWhite) || ((!turnOffColor) && light->features.setRgb);

        char paramBuffer[MediumBufferSize];
        sprintf(paramBuffer, "\"%s\", \"smooth\", %d", 
                (powerOn ? "on" : "off"), speed);

        SendCommand(networkClients, light->ipAddress, Yeelight::SetPower, paramBuffer);

        light->isPowered = powerOn;
    }

    CloseConnections(networkClients);
}


float NormalizeAnalogValue(int rawAnalogValue)
{
    float normalized = (float)rawAnalogValue / 1024.0f;

    if (normalized < 0.0f)
    {
        normalized = 0.0f;
    }

    if (normalized > 1.0f)
    {
        normalized = 1.0f;
    }

    return normalized;
}

void ProcessAnalogChange(Menu_State *menu, Light_Collection *lightCollection,
                         Network_Clients *networkClients, int rawAnalogValue)
{
    float normalized = NormalizeAnalogValue(rawAnalogValue);

    if (menu->page == 0)
    {
        if (menu->mode == ModeA)
        {
            PrintN("ModeA action, page 0 [set speed, TEMP]");
            SetSpeed(menu, normalized);
        }
        else if (menu->mode == ModeB)
        {
            PrintN("ModeB action, page 0 [set brightness]");
            CommandChangeBrightness(lightCollection, networkClients, 
                                    menu->speed, normalized);
        }
        else if (menu->mode == ModeC)
        {
            PrintN("ModeC action, page 0");
        }
        else if (menu->mode == ModeD)
        {
            PrintN("ModeD action, page 0 [set power]");
            CommandChangePowerState(lightCollection, networkClients, 
                                    menu->speed, normalized);
        }
    }
    else if (menu->page == 1)
    {
        if (menu->mode == ModeA)
        {
            PrintN("ModeA action, page 1");
        }
        else if (menu->mode == ModeB)
        {
            PrintN("ModeB action, page 1");
        }
        else if (menu->mode == ModeC)
        {
            PrintN("ModeC action, page 1 [set speed]");
            SetSpeed(menu, normalized);
        }
        else if (menu->mode == ModeD)
        {
            PrintN("ModeD action, page 1");
        }
    }
    else
    {
        PrintN("Only pages 0 and 1 are implemented");

    }
}

