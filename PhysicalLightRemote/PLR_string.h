

int FindFirstOf(const char* inputStr, const char* searchStr, int maxSearchRange = 2048)
{
    int input = 0;
    while ((inputStr[input] != 0))
    {
        int inputCopy = input;
        int search = 0;

        while (inputStr[inputCopy] == searchStr[search])
        {
            ++inputCopy;
            ++search;

            if (searchStr[search] == 0)
            {
                return input;
            }
        }

        if (inputCopy > maxSearchRange)
        {
            return -1;
        }
        
        ++input;
    }
    return -1;
}

void CatString(char *output, const char *source, int startPos, int length)
{
    int outputIndex = 0;
    for (int sourceIndex = startPos;
         sourceIndex < (startPos + length);
         ++sourceIndex)
    {
        output[outputIndex] = source[sourceIndex];
        outputIndex += 1;
    }

    if (output[length - 1] != 0)
    {
        output[length] = 0;
    }
}

void CopyString(char *output, const char *source)
{
    while (*source != 0)
    {
        *output = *source;
        ++output;
        ++source;
    }
    *output = 0;
}


bool AreStringIdentical(const char *a, const char *b)
{
    while (*a == *b)
    {
        if (*a == 0)
        {
            return true;
        }
        
        ++a;
        ++b;
    }

    return false;
}

int GetZeroPosition(const char *string)
{
    int pos = 0;
    while (string[pos] != 0)
    {
        pos += 1;
    }

    return pos;
}

char* GetZeroPointer(char *input)
{
    while (*input != 0)
    {
        input += 1;
    }

    return input;
}