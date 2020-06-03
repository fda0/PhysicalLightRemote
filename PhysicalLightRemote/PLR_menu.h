void ChangePage(Menu_State *menu, bool forward)
{
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
    if (menu->speed > 1000)
    {
        menu->speed = 1000;
    }

    if (menu->speed < 1)
    {
        menu->speed = 1;
    }
}


void CommandChangeBrightness(Light_Collection *lightCollection, 
                             Network_Clients *networkClients, float analogValue)
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
        sprintf(paramBuffer, "\"%s\", \"smooth\", 500", (powerOn ? "on" : "off"));
        SendCommand(networkClients, light->ipAddress, Yeelight::SetPower, paramBuffer);

        light->isPowered = powerOn;
    }

    CloseConnections(networkClients);
}


void CommandChangePowerState(Light_Collection *lightCollection, 
                      Network_Clients *networkClients, float analogValue)
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
        sprintf(paramBuffer, "\"%s\", \"smooth\", 500", (powerOn ? "on" : "off"));
        SendCommand(networkClients, light->ipAddress, Yeelight::SetPower, paramBuffer);

        light->isPowered = powerOn;
    }

    CloseConnections(networkClients);
}


void ProcessAnalogChange(Menu_State *menu, Light_Collection *lightCollection,
                         Network_Clients *networkClients, int rawAnalogValue)
{
    float normalized = (float)rawAnalogValue / 1024.0f;

    if (menu->page == 0)
    {
        if (menu->mode == ModeA)
        {
            PrintN("ModeA action, page 0");
        }
        else if (menu->mode == ModeB)
        {
            PrintN("ModeB action, page 0");
        }
        else if (menu->mode == ModeC)
        {
            PrintN("ModeC action, page 0");
        }
        else
        {
            PrintN("ModeD action, page 0 [set power]");
            CommandChangePowerState(lightCollection, networkClients, normalized);
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
        else
        {
            PrintN("ModeD action, page 1");
        }
    }
    else
    {
        PrintN("Only pages 0 and 1 are implemented");

    }
}

