#if DEV_PRINT
#define PrintN(Text) Serial.println((Text))

#define Print(Text) Serial.print((Text))

#define PrintRaw(Text, Len) Serial.write((char *)(Text), (Len))

#define PrintColor(Color) Print("R: "); Print(Color.r);\
Print(", G: ");Print(Color.g);\
Print(", B: ");PrintN(Color.b)

#else
#define PrintN(Text)
#define Print(Text)
#define PrintRaw(Text, Len)
#define PrintColor(Color)
#endif


#define RGB(Red, Green, Blue) (((Red) << 16) | ((Green) << 8) | (Blue))
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define ButtonSpecialDigitalRead(Value) (!digitalRead(Value))

#define LongColorStepMS 10

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
    union
    {
        float array[3];
        struct
        {
            float r;
            float g;
            float b;
        };
    };
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


enum Active_Effect
{
    EffectNone,
    EffectRandomColors
};

struct Random_Color_Struct
{
    Color color;
    float *victimColor;
    float *thiefColor;

    float stealTargetAmount;
    float stolenAmount;
};

struct Long_Effect
{
    uint32_t lastCycleTimestamp;
    Active_Effect active;
    uint32_t timeSinceLastSingal;
    bool isRunning;

    // effect-specific
    Random_Color_Struct randomColor;
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


int Wrap(int value, int wrapValue)
{
    value %= wrapValue;
    value += wrapValue;
    value %= wrapValue;
    
    return value;
}

template <class T>
T Min(T a, T b)
{
    if (a > b)
    {
        return b;
    }
    else
    {
        return a;
    }
}

template <class T>
T Max(T a, T b)
{
    if (a < b)
    {
        return b;
    }
    else
    {
        return a;
    }
}

uint32_t ColorToRGB(Color color)
{
    uint32_t red = (uint32_t)(color.r * 255.0f);
    uint32_t green = (uint32_t)(color.g * 255.0f);
    uint32_t blue = (uint32_t)(color.b * 255.0f);

    return RGB(red, green, blue);
}