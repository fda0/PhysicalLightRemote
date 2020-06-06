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
#define ButtonSpecialDigitalRead(Value) (!digitalRead(Value))

#define SmallBufferSize 256
#define MediumBufferSize 512
#define BigBufferSize 1024
#define MegaBufferSize (BigBufferSize * 4)
char MegaBuffer[MegaBufferSize];

// Const Variables
namespace Yeelight
{
    const char SetPower[] = "set_power";
    const char SetBright[] = "set_bright";
    const char SetRgb[] = "set_rgb";
    const char SetCtAbx[] = "set_ct_abx";
    const char StartCf[] = "start_cf";
    const char StopCf[] = "stop_cf";
};


struct Button
{
    int key;
    uint32_t changeTimestamp;
    uint32_t lastChangeTimestamp;
    bool value;
    bool lastValue;
};

#define AnalogHistoryCount (512)
struct Analog_Button
{
    int key;
    int value;
    int lastValue;
    int historyIndex;
    int hotHistoryCount;
    int16_t history[AnalogHistoryCount];
};

struct Button_Map 
{
    union
    {
        Button digitalButtons[6];
        struct
        {
            Button buttonA;
            Button buttonB;
            Button buttonC;
            Button buttonD;
            Button arrowA;
            Button arrowB;
        };
    };

    Analog_Button stick;
};

struct Features
{
    bool setPower;
    bool setBright;
    bool setRgb;
    bool setCtAbx;
    bool startCf;
    bool stopCf;
};

struct Light
{
    char ipAddress[16];
    Features features;
    bool isPowered;
    int brightness;
};

#define LightCount 32
struct Light_Collection
{
    Light lights[LightCount];
    int currentLightCount;
    bool hasAnyWhiteLight;
};

#define NetworkClientsCount 16
struct Network_Clients
{
    WiFiClient clients[NetworkClientsCount];
    int currentOpenCount;
};

enum Mode
{
    ModeNone,
    ModeA,
    ModeB,
    ModeC,
    ModeD
};

enum ColorChangeMode
{
    ColorNone,
    ColorRGB,
    ColorR,
    ColorG,
    ColorB
};

struct Color
{
    float r;
    float g;
    float b;
};

#define MenuPageCount 2
struct Menu_State
{
    int page;
    int smoothness;
    Mode mode;
    ColorChangeMode colorChangeMode;
    Color color;
};

struct Save_State
{
    int firstTime;
    int smoothness;
};

// helper functions
inline int Abs(int a)
{
    if (a < 0)
        return a * -1;

    return a;
}

template <class T>
T Clamp(T value, T min, T max)
{
    if (value > max)
    {
        value = max;
    }

    if (value < min)
    {
        value = min;
    }

    return value;
}