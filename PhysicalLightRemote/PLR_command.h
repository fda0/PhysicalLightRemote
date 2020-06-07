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

void RandomColorsGetRandomTargets(Random_Color_Struct *rc, bool firstInit)
{
    rc->stolenAmmount = 0;

    int skipColor = random(0, colorCount - 1);
    int direction = (random(0, 1) == 1) ? 1 : -1;

    int victimIndex = Wrap(skipColor + direction, colorCount);
    int thiefIndex = Wrap(victimIndex + direction, colorCount);
    rc->victimColor = rc->color.array[victimIndex];
    rc->thiefColor = rc->color.array[thiefIndex];

    if (firstInit)
    {
        int biggerColor = random(127, 255);
        *victimColor = biggerColor;
        *thiefColor = maxColorSum - biggerColor;
    }

    int maxSteal = Min(*rc->victimColor, *rc->thiefColor);
    rc->stealTargetAmmout = random((maxSteal / 2), maxSteal);
}

void CommandProcessRandomColorsStep(Light_Collection *lightCollection, 
                                    Network_Clients *networkClients, 
                                    Menu_State *menu, Long_Effect *effect)
{
    const int colorCount = ArrayCount(effect->rc.color.array);
    const int maxColorSum = 255;

    Random_Color_Struct *rc = effect->randomColor;

    if (!effect->isRunning)
    {
        effect->isRunning = true;
        RandomColorsGetRandomTargets(rc, true);
    }

    effect->timeSinceLastSingal += LongColorStepMS;

    int maxDelay = 5000;
    float speedRatio = Clamp<float>((float)menu->smoothness, 10, 1000) / 1000.0f;
    int currentDelay = Clamp<int>((int)((float)maxDelay * speedRatio), 1, 1000);
    
    if (currentDelay <= effect->timeSinceLastSingal)
    {
        effect->timeSinceLastSingal = 0;

        float inverseRatio = 1.0f - speedRatio;

        Print("Color step without clamp: ");
        PrintN((int)(20.0f * inverseRatio));

        int colorStep = Clamp<int>((int)(20.0f * inverseRatio), 1, 255);
        int leftToSteal = rc->stealTargetAmmout - rc->stolenAmmount;
        bool lastSteal = (colorStep > leftToSteal) 
        if (lastSteal)
        {
            colorStep = leftToSteal;
        }

        rc->victimColor -= colorStep;
        rc->thiefColor += colorStep;
    }


}

void CommandStartColorFlowLoop(Light_Collection *lightCollection, 
                               Network_Clients *networkClients, 
                               Menu_State *menu, ColorFlow colorFlow)
{
    const int32_t *loop = cfloop_police;
    int arrayCount = ArrayCount(cfloop_police);

    switch (colorFlow)
    {
        case CF_Police:
        {
            loop = cfloop_police;
            arrayCount = ArrayCount(cfloop_police);
        } break;
    }


    int rowCount = arrayCount / 4;

    MegaBuffer[0] = 0;
    char *bufferPart = MegaBuffer;
    sprintf(MegaBuffer, "0, 1, \"");

    for (int row = 0; row < rowCount; ++row)
    {
        bufferPart = GetZeroPointer(bufferPart);
        int rowOffset = row * 4;
        
        // type
        int type = (int32_t)pgm_read_dword(loop + rowOffset + 1);

        // delay
        float speedRatio = Clamp<float>((float)menu->smoothness, 10, 1000) / 1000.0f;
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
    uint32_t rgb = RGB(red, green, blue);

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


void CommandStopCf(Light_Collection *lightCollection, Network_Clients *networkClients)
{
    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        if (light->features.stopCf)
        {
            char param = 0;
            SendCommand(networkClients, light->ipAddress, Yeelight::StopCf, &param);
        }
    }

    CloseConnections(networkClients);
}