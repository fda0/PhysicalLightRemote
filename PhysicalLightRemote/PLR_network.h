

bool UdpRead(WiFiUDP *udp, char *outputBuffer, int bufferSize)
{
    int packetSize = udp->parsePacket();
    Print("Packet size: ");
    PrintN(packetSize);
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
            return false;
        }
    }
}

void SendMulticastMessage(WiFiUDP *udp, IPAddress *multicast)
{
    PrintN("Sending multicast discover message");
    udp->beginMulticast(WiFi.localIP(), *multicast, 1982);
    udp->beginPacketMulticast(*multicast, 1982, WiFi.localIP());
    udp->print("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1982\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb");
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

        PrintN("Connection closed");
    }

    networkClients->currentOpenCount = 0;
}

void SendCommand(Network_Clients *networkClients, const char *ipAddress, 
                 const char *method, const char *params)
{
    if (networkClients->currentOpenCount == NETWORK_CLIENTS_COUNT)
    {
        CloseConnections(networkClients);
    }

    WiFiClient *client = &networkClients->clients[networkClients->currentOpenCount++];

    char buffer[MEDIUM_BUFFER_SIZE];
    sprintf(buffer, "{\"id\": 1, \"method\": \"%s\", \"params\":[%s]}", method, params);

    if (client->connect(ipAddress, 55443)) 
    {
        client->println(buffer);
    }
    
    PrintN("Command Sent");
}

