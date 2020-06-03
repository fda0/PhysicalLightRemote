

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
    for (int i = startPos; i < (startPos + length); ++i)
    {
        output[i] = source[i];
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

