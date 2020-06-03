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


void ChangePowerState(Light_Collection *lightCollection, 
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
            PrintN("ModeA action");
        }
        else if (menu->mode == ModeB)
        {
            PrintN("ModeB action");
        }
        else if (menu->mode == ModeC)
        {
            PrintN("ModeC action");
        }
        else
        {
            PrintN("ModeD action");
            ChangePowerState(lightCollection, networkClients, normalized);
        }
    }
    else
    {
        PrintN("Only page index 0 is implemented");
    }
}

