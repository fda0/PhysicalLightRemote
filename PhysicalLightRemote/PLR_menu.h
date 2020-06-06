//
// commands
//

void GetSmoothnessString(char *output, int smoothness)
{
    if (30 > smoothness)
    {
        sprintf(output, "\"sudden\", 1");
    }
    else
    {
        sprintf(output, "\"smooth\", %d", smoothness);
    }
}


char* GetZeroPointer(char *input)
{
    while (*input != 0)
    {
        input += 1;
    }

    return input;
}

void CommandStartColorFlowLoop(Light_Collection *lightCollection, 
                               Network_Clients *networkClients, 
                               Menu_State *menu, ColorFlow colorFlow)
{
    const int32_t *loop = cfloop_police;
    int arrayCount = ArrayCount(cfloop_police);
    int rowCount = arrayCount / 4;

    MegaBuffer[0] = 0;
    char *bufferPart = MegaBuffer;
    sprintf(MegaBuffer, "0, 0, \"");

    for (int row = 0; row < rowCount; ++row)
    {
        bufferPart = GetZeroPointer(bufferPart);
        int rowOffset = row * 4;
        
        // type
        int type = (int32_t)pgm_read_dword(loop + rowOffset + 1);

        // delay
        float speedRatio = Clamp<int>((float)menu->smoothness, 10, 1000) / 1000.0f;
        int maxDelay = (int32_t)pgm_read_dword(loop + rowOffset);
        int delay = (int)((float)maxDelay * speedRatio);
        if ((type == 7) && (delay < 50))
        {
            continue;
        }
        else if (delay < 50)
        {
            delay = 50;
        }

        // brightness
        int brightness = (int32_t)pgm_read_dword(loop + rowOffset + 3);
        if (brightness == -1 && (lightCollection->currentLightCount > 0))
        {
            brightness = lightCollection->lights[0].brightness;
        }

        sprintf(bufferPart, "%d,%d,%d,%d,", 
                delay,
                type, 
                (int32_t)pgm_read_dword(loop + rowOffset + 2), 
                brightness);
    }

    int endZeroPos = GetZeroPosition(MegaBuffer);
    MegaBuffer[endZeroPos - 1] = '\"';

    for (int lightIndex = 0;
     lightIndex < lightCollection->currentLightCount;
     ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        if (light->features.setRgb && light->features.startCf)
        {
            SendCommand(networkClients, light->ipAddress, Yeelight::StartCf, MegaBuffer);
        }
    }

    CloseConnections(networkClients);
}




float NormalizeFromStepRange(float value, float stepRange, int step)
{
    float output = (value - (stepRange * (step - 1))) / (stepRange);
    return output;
}

void CommandChangeColor(Light_Collection *lightCollection, 
                        Network_Clients *networkClients, 
                        Menu_State *menu, float analogValue)
{
    if (menu->colorChangeMode == ColorRGB)
    {
        float singleStep = 1.0f / 6.0f;

        if (analogValue < (singleStep * 1))
        {
            // R -> RG
            menu->color.r = 1;
            menu->color.g = NormalizeFromStepRange(analogValue, singleStep, 1);
            menu->color.b = 0;
        }
        else if (analogValue < (singleStep * 2))
        {
            // RG -> G
            menu->color.r = 1.0f - NormalizeFromStepRange(analogValue, singleStep, 2);
            menu->color.g = 1;
            menu->color.b = 0;
        }
        else if (analogValue < (singleStep * 3))
        {
            // G -> GB
            menu->color.r = 0;
            menu->color.g = 1;
            menu->color.b = NormalizeFromStepRange(analogValue, singleStep, 3);
        }
        else if (analogValue < (singleStep * 4))
        {
            // GB -> B
            menu->color.r = 0;
            menu->color.g = 1.0f - NormalizeFromStepRange(analogValue, singleStep, 4);
            menu->color.b = 1;
        }
        else if (analogValue < (singleStep * 5))
        {
            // B -> RB
            menu->color.r = NormalizeFromStepRange(analogValue, singleStep, 5);
            menu->color.g = 0;
            menu->color.b = 1;
        }
        else
        {
            // RB -> R
            menu->color.r = 1;
            menu->color.g = 0;
            menu->color.b = 1.0f - NormalizeFromStepRange(analogValue, singleStep, 6);
        }
        
    }
    else if (menu->colorChangeMode == ColorR)
    {
        menu->color.r = analogValue;
    }
    else if (menu->colorChangeMode == ColorG)
    {
        menu->color.g = analogValue;
    }
    else
    {
        menu->color.b = analogValue;
    }

    uint32_t red = (uint32_t)(menu->color.r * 255.0f);
    uint32_t green = (uint32_t)(menu->color.g * 255.0f);
    uint32_t blue = (uint32_t)(menu->color.b * 255.0f);
    uint32_t rgb = (red << 16) | (green << 8) | blue;

    // Print("R: ");
    // Print(red);
    // Print(", G: ");
    // Print(green);
    // Print(", B: ");
    // Print(blue);
    // Print(", RGB: ");
    // PrintN(rgb);

    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        if (light->features.setRgb)
        {
            char smoothnessBuffer[SmallBufferSize];
            GetSmoothnessString(smoothnessBuffer, menu->smoothness);

            char paramBuffer[MediumBufferSize];
            sprintf(paramBuffer, "%d, %s", rgb, smoothnessBuffer);
            
            SendCommand(networkClients, light->ipAddress, Yeelight::SetRgb, paramBuffer);
        }
    }

    CloseConnections(networkClients);
}


void CommandChangeColorTemperature(Light_Collection *lightCollection, 
                                   Network_Clients *networkClients, 
                                   int smoothness, float analogValue)
{
    int temperature = (analogValue * (6500 - 1700)) + 1700;

    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        if (light->features.setCtAbx)
        {
            char smoothnessBuffer[SmallBufferSize];
            GetSmoothnessString(smoothnessBuffer, smoothness);

            char paramBuffer[MediumBufferSize];
            sprintf(paramBuffer, "%d, %s", temperature, smoothnessBuffer);
            
            SendCommand(networkClients, light->ipAddress, Yeelight::SetCtAbx, paramBuffer);
        }
    }

    CloseConnections(networkClients);
}


void CommandChangeBrightness(Light_Collection *lightCollection, 
                             Network_Clients *networkClients, 
                             int smoothness, float analogValue)
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

        if (light->features.setBright)
        {
            char smoothnessBuffer[SmallBufferSize];
            GetSmoothnessString(smoothnessBuffer, smoothness);

            char paramBuffer[MediumBufferSize];
            sprintf(paramBuffer, "%d, %s", brightness, smoothnessBuffer);
            
            SendCommand(networkClients, light->ipAddress, Yeelight::SetBright, paramBuffer);
            light->brightness = brightness;
        }
    }

    CloseConnections(networkClients);
}


void CommandChangePowerState(Light_Collection *lightCollection, 
                      Network_Clients *networkClients, 
                      int smoothness, float analogValue)
{
    bool turnOffWhite = analogValue < 0.66f;
    bool turnOffColor = analogValue < 0.33f; 

    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        if (light->features.setPower)
        {
            char smoothnessBuffer[SmallBufferSize];
            GetSmoothnessString(smoothnessBuffer, smoothness);

            bool powerOn = !(turnOffWhite) || ((!turnOffColor) && light->features.setRgb);

            char paramBuffer[MediumBufferSize];
            sprintf(paramBuffer, "\"%s\", %s", 
                    (powerOn ? "on" : "off"), smoothnessBuffer);

            SendCommand(networkClients, light->ipAddress, Yeelight::SetPower, paramBuffer);
            light->isPowered = powerOn;
        }
    }

    CloseConnections(networkClients);
}




//
// menu
//


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


void ChangePage(Save_State *save, Menu_State *menu, bool forward)
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
             Mode mode)
{
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

