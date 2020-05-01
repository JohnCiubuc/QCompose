#include "xkeyboardhook.h"

int catcher( Display *disp, XErrorEvent *xe )
{
    qDebug() <<  "Window Destroyed" ;
    return 0;
}

XKeyboardHook::XKeyboardHook()
{
    d = XOpenDisplay(NULL);
    root = DefaultRootWindow(d);

    XGetInputFocus (d, &curFocus, &revert);
    XSelectInput(d, curFocus, KeyPressMask|KeyReleaseMask|FocusChangeMask);

    watchTimer = new QTimer(this);
    connect(watchTimer, &QTimer::timeout, this, &XKeyboardHook::watcher);
    watchTimer->start(1);

}

void XKeyboardHook::watcher()
{
    XEvent ev;
    XNextEvent(d, &ev);
    switch (ev.type)
    {
    case FocusOut:
        if (curFocus != root)
            XSelectInput(d, curFocus, 0);
        XSetErrorHandler( catcher );
        XGetInputFocus (d, &curFocus, &revert);
        if (curFocus == PointerRoot)
            curFocus = root;
        XSelectInput(d, curFocus, KeyPressMask|KeyReleaseMask|FocusChangeMask);

        break;

    case KeyPress:

        detectedKey(ev.xkey);

    }
}
