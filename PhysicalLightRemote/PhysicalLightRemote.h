#if DEV_PRINT
#define PrintN(Text) Serial.println((Text))
#define Print(Text) Serial.print((Text))
#define PrintRaw(Text, Len) Serial.write((char *)(Text), (Len))
#else
#define PrintN(Text)
#define Print(Text)
#define PrintRaw(Text, Len)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Button_Read(Value) (!digitalRead(Value))

#define Framerate 20
#define FrameDelay (1000 / Framerate)
#define Seconds(Value) (Framerate * (Value))
#define Minutes(Value) (Seconds((Value)) * Framerate)

#define MEDIUM_BUFFER_SIZE 512
#define BIG_BUFFER_SIZE 1024

// Const Variables
namespace Yeelight
{
    const char SetPower[] = "set_power";
    const char SetBright[] = "set_bright"; 
};


struct Button_Map 
{
    union
    {
        int buttons[5];
        struct
        {
            int buttonA;
            int buttonB;
            int buttonC;
            int buttonD;
            int analogStick;
        };
    };
};


enum Light_Type
{
    Mono,
    Color
};

struct Features
{
    bool setPower;
    bool setBright;
};

struct Light
{
    Light_Type type;
    char ipAddress[16];
    Features features;
    bool isPowered;
};

#define LIGHT_COUNT 32
struct Light_Collection
{
    Light lights[LIGHT_COUNT];
    int currentLightCount;
};

#define NETWORK_CLIENTS_COUNT 16
struct Network_Clients
{
    WiFiClient clients[NETWORK_CLIENTS_COUNT];
    bool isClientOpen[NETWORK_CLIENTS_COUNT];
    int currentUsageCount;
};