

void LoadStateFromMemory(Save_State *save, Menu_State *menu)
{
    EEPROM.get(0, *save);
    menu->smoothness = Clamp<int>(save->smoothness, 1, 1000);

    PrintN("Loaded from EEPROM");
}

void SaveCurrentStateToMemory(Save_State *save, Menu_State *menu)
{
    if (Abs(save->smoothness - menu->smoothness) > 200)
    {
        save->smoothness  = menu->smoothness;

        EEPROM.put(0, *save);
        EEPROM.commit();
        PrintN("Saved to EEPROM");
    }
}

void StopLightShows(Light_Collection *lightCollection, 
                    Network_Clients *networkClients, Long_Effect *longEffect)
{
    *longEffect = {0};

    CommandStopCf(lightCollection, networkClients);
}


void ChangePage(Save_State *save, Menu_State *menu,  
                Light_Collection *lightCollection, Network_Clients *networkClients, 
                Long_Effect *longEffect, bool forward)
{
    menu->mode = ModeNone;

    StopLightShows(lightCollection, networkClients, longEffect);

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

    Print("ChangePage() -> current page: ");
    PrintN(menu->page);

    SaveCurrentStateToMemory(save, menu);
}

void IncreaseColorMode(Menu_State *menu)
{
    menu->colorChangeMode = (ColorChangeMode)((int)menu->colorChangeMode + 1);
    if (menu->colorChangeMode > ColorB)
    {
        menu->colorChangeMode = ColorRGB;
    }
}

void SetMode(Save_State *save, Menu_State *menu, 
             Light_Collection *lightCollection, Network_Clients *networkClients,
             Long_Effect *longEffect, Mode mode)
{
    StopLightShows(lightCollection, networkClients, longEffect);

    if (menu->page == 0)
    {
        menu->mode = mode;
        if (menu->mode == ModeA)
        {
            IncreaseColorMode(menu);
        }
        else
        {
            menu->colorChangeMode = ColorNone;
        }

        Print("Mode changed to ");
        PrintN(mode);
    }
    else if (menu->page == 1)
    {
        if (mode == ModeA)
        {
            PrintN("ModeA action, page 1 [CF Police]");
            CommandStartColorFlowLoop(lightCollection, networkClients, 
                                      menu, CF_Police);
        }
    }
    else
    {
        PrintN("Only Page 0 and 1 are implemented");
    }
}


void SetSmoothness(Menu_State *menu, float analogValue)
{
    menu->smoothness = (int)(analogValue * 1000);
    Print("New smoothness: ");
    PrintN(menu->smoothness);
}


float NormalizeAnalogValue(int rawAnalogValue)
{
    float normalized = (float)rawAnalogValue / 1024.0f;
    normalized = Clamp<float>(normalized, 0.0f, 1.0f);
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
            Print("ModeA action, page 0 [color change, mode: ");
            Print(menu->colorChangeMode);
            PrintN("]");

            CommandChangeColor(lightCollection, networkClients, 
                               menu, normalized);
        }
        else if (menu->mode == ModeB)
        {
            PrintN("ModeB action, page 0 [set brightness]");
            CommandChangeBrightness(lightCollection, networkClients, 
                                    menu->smoothness, normalized);
        }
        else if (menu->mode == ModeC)
        {
            PrintN("ModeC action, page 0 [set color temperature]");
            CommandChangeColorTemperature(lightCollection, networkClients,
                                          menu->smoothness, normalized);
        }
        else if (menu->mode == ModeD)
        {
            PrintN("ModeD action, page 0 [set power]");
            CommandChangePowerState(lightCollection, networkClients, 
                                    menu->smoothness, normalized);
        }
    }
    else
    {
        SetSmoothness(menu, normalized);
    }
}

