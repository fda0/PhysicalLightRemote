


void CommandPower(Light_Collection *lightCollection, Network_Clients *networkClients)
{
    for (int lightIndex = 0;
         lightIndex < lightCollection->currentLightCount;
         ++lightIndex)
    {
        Light *light = &lightCollection->lights[lightIndex];

        char paramBuffer[MediumBufferSize];
        sprintf(paramBuffer, "\"%s\", \"smooth\", 500", (light->isPowered ? "off" : "on"));
        SendCommand(networkClients, light->ipAddress, Yeelight::SetPower, paramBuffer);

        light->isPowered = !(light->isPowered);
    }

    CloseConnections(networkClients);
}
