

bool UdpRead(WiFiUDP *udp, char *outputBuffer, int bufferSize)
{
    int packetSize = udp->parsePacket();
    if (packetSize) 
    {
        int readLength = udp->read(outputBuffer, bufferSize);
        if (readLength > 0)
        {
            outputBuffer[readLength] = 0;
            return true;
        }
        else
        {
            outputBuffer[0] = 0;
            return false;
        }
    }
}

void SendMulticastMessage(WiFiUDP *udp, IPAddress *multicast)
{
    udp->beginMulticast(WiFi.localIP(), *multicast, 1982);
    udp->beginPacketMulticast(*multicast, 1982, WiFi.localIP());
    udp->print("M-SEARCH * HTTP/1.1\r\n"
               "HOST: 239.255.255.250:1982\r\n"
               "MAN: \"ssdp:discover\"\r\n"
               "ST: wifi_bulb");
    udp->endPacket();
    udp->begin(1982);
}

void CloseConnections(Network_Clients *networkClients)
{
    for (int clientIndex = 0;
         clientIndex < networkClients->currentOpenCount;
         ++clientIndex)
    {
        WiFiClient *client = &networkClients->clients[clientIndex];
        while (client->connected())
        {
            client->stop();
        }
    }

    networkClients->currentOpenCount = 0;
}

void SendCommand(Network_Clients *networkClients, const char *ipAddress, 
                 const char *method, const char *params)
{
    if (networkClients->currentOpenCount == NetworkClientsCount)
    {
        CloseConnections(networkClients);
    }

    WiFiClient *client = &networkClients->clients[networkClients->currentOpenCount++];

    char buffer[MediumBufferSize];
    sprintf(buffer, "{\"id\": 1, \"method\": \"%s\", \"params\":[%s]}", method, params);

    Print("Full command sent: ");
    PrintN(buffer);

    if (client->connect(ipAddress, 55443)) 
    {
        client->println(buffer);
    }
}

