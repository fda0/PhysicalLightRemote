

void FillLightData(Light_Collection *lightCollection, int targetLightIndex, 
                   const char *ipBuffer, Features *featuresBuffer, bool isPowered)
{
    using namespace Yeelight;    
    Light *light = &lightCollection->lights[targetLightIndex];
    
    CopyString(light->ipAddress, ipBuffer);
    light->features = *featuresBuffer;
    light->isPowered = isPowered;

#if DEV_PRINT
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
#endif
}

void ParseUdpRead(Light_Collection *lightCollection, const char *buffer)
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

            for (int lightIndex = 0; 
                 lightIndex < lightCollection->currentLightCount;
                 ++lightIndex)
            {
                if (AreStringIdentical(ipBuffer, lightCollection->lights[lightIndex].ipAddress))
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
                    featuresBuffer.setRgb = (FindFirstOf(buffer, SetRgb, powerOffset) != -1);

                    buffer += powerOffset + sizeof(powerTag) + 1;
                    bool isPowered = (*buffer == 'n');
                    FillLightData(lightCollection, lightCollection->currentLightCount, ipBuffer, &featuresBuffer, isPowered);
                    ++lightCollection->currentLightCount;
                }

            }

        }
    }
}


void UdpReadMultipleMessages(WiFiUDP *udp, Light_Collection *lightCollection)
{
    for (int networkReadIndex = 0; 
     networkReadIndex < 10;
     ++networkReadIndex)
    {
        char buffer[BigBufferSize] = {0};
        if (UdpRead(udp, buffer, sizeof(buffer)))
        {
            ParseUdpRead(lightCollection, buffer);
        }
        else
        {
            // Print("End of Messages, received: ");
            // PrintN(networkReadIndex + 1);
            break;
        }
    }
}

