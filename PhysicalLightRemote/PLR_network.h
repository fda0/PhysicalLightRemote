

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