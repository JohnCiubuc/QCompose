#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include "xkeyboardhook.h"
#include <qcomposeparser.h>
#include "xkeys.h"
#include <X11/Xlib.h>

QString getUnicodeEscape(QString str)
{
    QString escaped;
    escaped.reserve(6 * str.size());
    for (QString::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        QChar ch = *it;
        ushort code = ch.unicode();
//        if (code < 0x80)
//        {
//            escaped += ch;
//        }
//        else
        {
            escaped += "U";
            escaped += QString::number(code, 16).rightJustified(4, '0');
        }
    }
    return escaped;
}

KeySym loadUnicodeSym(QString uCode)
{
    Display * xDisplay ;
    xDisplay = XOpenDisplay(0);

    if(xDisplay == NULL)
        qWarning() << "Unable to open XxDisplay";
    KeySym s = XStringToKeysym(uCode.toLocal8Bit());

    KeySym keysym_list[] = {s};
    int scratch_keycode = 0x20ad;
    XChangeKeyboardMapping(xDisplay, scratch_keycode, 1, keysym_list, 1);
//    KeyCode code = scratch_keycode;


    return scratch_keycode;

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    XKeyboardHook * hk = new XKeyboardHook;

    QComposeParser test;
    xkeys * xk = new xkeys;
    QString chary = test.lastChar;
    QString aa = "¯\\_(ツ)_/¯";
//    aa = "_";
//    aa = "™";
    KeySym s = ulong(aa.toUtf8().toUInt());
    qDebug("1");
    qDebug() << aa;
    QString str = aa.toUtf8();

//    QTextStream stream(stdout);
//    qDebug() << escaped.toLocal8Bit() << '\n';
//    qDebug() << b;
//    s = XStringToKeysym(escaped.toLocal8Bit());

    Display * xDisplay ;
    Window xWinRoot;
    Window xWinFocus;
    int    xRevert;
    xDisplay = XOpenDisplay(0);

    if(xDisplay == NULL)
        qWarning() << "Unable to open XxDisplay";

    // Get the root window for the current xDisplay.
    xWinRoot = XDefaultRootWindow(xDisplay);
    // Find the window which has the current keyboard focus.
    XGetInputFocus(xDisplay, &xWinFocus, &xRevert);
    qDebug("-----");
    qDebug() << QString("b").toUtf8();
    qDebug() << s;
    qDebug() << (XKeysymToKeycode(xDisplay, s));



    QThread::sleep(5);








    KeySym *keysyms = NULL;
    int keysyms_per_keycode = 0;
    int scratch_keycode = 0; // Scratch space for temporary keycode bindings
    int keycode_low, keycode_high;
    //get the range of keycodes usually from 8 - 255
    XDisplayKeycodes(xDisplay, &keycode_low, &keycode_high);
    //get all the mapped keysyms available
    keysyms = XGetKeyboardMapping(
                  xDisplay,
                  keycode_low,
                  keycode_high - keycode_low,
                  &keysyms_per_keycode);

    //find unused keycode for unmapped keysyms so we can
    //hook up our own keycode and map every keysym on it
    //so we just need to 'click' our once unmapped keycode
    int i;
    for (i = keycode_low; i <= keycode_high; i++)
    {
        int j = 0;
        int key_is_empty = 1;
        for (j = 0; j < keysyms_per_keycode; j++)
        {
            int symindex = (i - keycode_low) * keysyms_per_keycode + j;
            // test for debugging to looking at those value
            // KeySym sym_at_index = keysyms[symindex];
            // char *symname;
            // symname = XKeysymToString(keysyms[symindex]);

            if(keysyms[symindex] != 0)
            {
                key_is_empty = 0;
            }
            else
            {
                break;
            }
        }
        if(key_is_empty)
        {
            scratch_keycode = i;
            break;
        }
    }
    XFree(keysyms);
    XFlush(xDisplay);





//    KeySym sym = XStringToKeysym("U005A"); //uppercase Z

    for(int i = 0; i < aa.length(); i++)
    {
        xDisplay = XOpenDisplay(0);

        if(xDisplay == NULL)
            qWarning() << "Unable to open XxDisplay";

        // Get the root window for the current xDisplay.
        xWinRoot = XDefaultRootWindow(xDisplay);
        // Find the window which has the current keyboard focus.
        XGetInputFocus(xDisplay, &xWinFocus, &xRevert);
        QString c = QString(aa[i]);
        qDebug() << getUnicodeEscape(QString(c));
        auto d = getUnicodeEscape(QString(c));

        KeySym s = XStringToKeysym(d.toLocal8Bit());
        qDebug() << s;
        if (d == c)
        {
            qDebug() << "NOT";
            // not unicode
            XKeyEvent event = xk->createKeyEvent(true, s);
            XTestFakeKeyEvent(xDisplay, event.keycode, True, 0);
            XFlush(xDisplay);
            QThread::msleep(500);

            event = xk->createKeyEvent(true, s);
            XTestFakeKeyEvent(xDisplay, event.keycode, True, 0);
            XFlush(xDisplay);
        }
        else
        {
            KeySym keysym_list[] = {s, s};
            XChangeKeyboardMapping(xDisplay, scratch_keycode, 1, keysym_list, 1);


            XTestFakeKeyEvent(xDisplay, scratch_keycode, True, 0);
            XFlush(xDisplay);
            QThread::msleep(10);
            XTestFakeKeyEvent(xDisplay, scratch_keycode, False, 0);
            XFlush(xDisplay);
            QThread::msleep(10);
            {
                KeySym keysym_list[] = { 0, 0 };
                XChangeKeyboardMapping(xDisplay, scratch_keycode, 1, keysym_list, 1);
            }
        }
//        KeySym s = loadUnicodeSym();



    }




    qDebug("sleepy");
    return a.exec();
}
