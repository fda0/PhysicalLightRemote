void ChangePage(Menu *menu, bool forward)
{
    if (forward)
    {
        menu->page += 1;
    }
    else
    {
        menu-> page -= 1;
    }

    if (menu->page < 0)
    {
        menu->page = 0;
    }

    if (menu->page >= MenuPageCount)
    {
        menu->page = MenuPageCount - 1;
    }
}


void SetMode(Menu *menu, Mode mode)
{
    menu->mode = mode;
}