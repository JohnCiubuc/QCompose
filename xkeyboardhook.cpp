#include "xkeyboardhook.h"

XKeyboardHook::XKeyboardHook()
{
    Display* d = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(d);
    Window curFocus;
    char buf[17];
    KeySym ks;
    XComposeStatus comp;
    int len;
    int revert;

    XGetInputFocus (d, &curFocus, &revert);
    XSelectInput(d, curFocus, KeyPressMask|KeyReleaseMask|FocusChangeMask);

    while (1)
    {
        XEvent ev;
        XNextEvent(d, &ev);
        switch (ev.type)
        {
            case FocusOut:
                if (curFocus != root)
                    XSelectInput(d, curFocus, 0);
                XGetInputFocus (d, &curFocus, &revert);
                if (curFocus == PointerRoot)
                    curFocus = root;
                XSelectInput(d, curFocus, KeyPressMask|KeyReleaseMask|FocusChangeMask);
                break;

            case KeyPress:
                printf ("Got key!\n");
                len = XLookupString(&ev.xkey, buf, 16, &ks, &comp);
                if (len > 0 && isprint(buf[0]))
                {
                    buf[len]=0;
                    printf("String is: %s\n", buf);
                }
                else
                {
                    printf ("Key is: %d\n", (int)ks);
                }
        }

    }
}
