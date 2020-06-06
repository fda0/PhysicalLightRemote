// looping color flows

enum ColorFlow
{
    CF_Police
};


#define COLOR_NUM 1
#define COLOR_TEMP_NUM 2
#define SLEEP_NUM 7

#define RGB(Red, Green, Blue) (((Red) << 16) | ((Green) << 8) | (Blue))

#define COLOR(Delay, Color) (Delay), COLOR_NUM, (Color), -1
#define COLOR_TEMP(Delay, ColorTemp) (Delay), COLOR_TEMP_NUM, (ColorTemp), -1
#define SLEEP(Delay) (Delay), SLEEP_NUM, 0, 0

const PROGMEM int32_t cfloop_police[] = 
{
    COLOR(1000, RGB(0, 0, 255)),
    SLEEP(3000),
    COLOR(1000, RGB(255, 0, 0)),
    SLEEP(3000)
};


