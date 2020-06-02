void CollectAnalogSamples(Analog_Button *stick)
{
    stick->history[stick->historyIndex] = (int16_t)analogRead(stick->key);
    ++stick->historyIndex;

    if (stick->historyIndex >= AnalogHistoryCount)
    {
        stick->historyIndex = 0;   
    }

    if (stick->hotHistoryCount < AnalogHistoryCount)
    {
        ++stick->hotHistoryCount;
    }
}


void CalculateAnalogValue(Analog_Button *stick)
{
    // TODO(mautesz): overkill stupid stuff - get better analog stick and simplify it?
    stick->lastValue = stick->value;

    int sum = 0;
    for (int i = 0; i < stick->hotHistoryCount; ++i)
    {
        sum += stick->history[i];
    }

    int averagePassOne;
    if (stick->hotHistoryCount > 0)
    {
        averagePassOne = sum / stick->hotHistoryCount;
    }
    else
    {
        averagePassOne = 0;
    }

    const int16_t maxDeviation = 100;
    int sampleCount = 0;
    sum = 0;
    for (int i = 0; i < stick->hotHistoryCount; ++i)
    {
        int16_t sample = stick->history[i];
        if (Abs(sample - averagePassOne) < maxDeviation)
        {
            ++sampleCount;
            sum += sample;
        }
    }        

    if (sampleCount > 0)
    {
        int averagePassTwo = sum / sampleCount;
        stick->value = averagePassTwo;
    }
    else
    {
        Print("No samples in average rage! averagePassOne: ");
        Print(averagePassOne);
        Print(", stick->hotHistoryCount: ");
        PrintN(stick->hotHistoryCount);
    }

    // clearing
    stick->historyIndex = 0;
    stick->hotHistoryCount = 0;
}


void ReadButtons(Button_Map *buttons, uint32_t timestamp)
{
    // digital
    for (int buttonIndex = 0;
         buttonIndex < ArrayCount(buttons->digitalButtons);
         ++buttonIndex)
    {
        Button *button = &buttons->digitalButtons[buttonIndex]; 

        button->lastValue = button->value;
        button->value = ButtonSpecialDigitalRead(button->key);

        if (button->value != button->lastValue)
        {
            button->lastChangeTimestamp = button->changeTimestamp;
            button->changeTimestamp = timestamp;
        }
    }
}

bool DigitalButtonComparison(Button *button, uint32_t timestamp)
{
    return (button->value != button->lastValue) 
            && button->value
            && ((timestamp - 200) > button->lastChangeTimestamp);
}

bool AnalogButtonComparison(Analog_Button *button)
{
    int difference = button->value - button->lastValue;
    int margin = 10;
    return Abs(difference) > margin;
}