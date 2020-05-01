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
    while (1)
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
            qDebug("c");
            if (curFocus == PointerRoot)
                curFocus = root;
            qDebug("ad");
            XSelectInput(d, curFocus, KeyPressMask|KeyReleaseMask|FocusChangeMask);
            qDebug("ae");

            break;

        case KeyPress:

            detectedKey(ev.xkey);
            //                len = XLookupString(&ev.xkey, buf, 16, &ks, &comp);
            //                if (len > 0 && isprint(buf[0]))
            //                {
            //                    buf[len]=0;
            //                    printf("String is: %s\n", buf);
            //                }
            //                else
            //                {
            //                    printf ("Key is: %d\n", (int)ks);
            //                }
            //        }

        }
    }
}
