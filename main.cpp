#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include "xkeyboardhook.h"
#include <qcomposeparser.h>
#include "xkeys.h"
#include <X11/Xlib.h>

#define POINTER 0x20ad

QString getUnicodeEscape(QString str)
{
    QString escaped;
    escaped.reserve(6 * str.size());
    for (QString::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        QChar ch = *it;
        ushort code = ch.unicode();
        escaped += "U";
        escaped += QString::number(code, 16).rightJustified(4, '0');
    }
    return escaped;
}

void sendString(QString string)
{
    Display * xDisplay ;
    xDisplay = XOpenDisplay(0);

    if(xDisplay == NULL)
        qWarning() << "Unable to open XxDisplay";

    for(int i = 0; i < string.length(); i++)
    {

        QString c = QString(string[i]);
        auto d = getUnicodeEscape(c);

        KeySym s = XStringToKeysym(d.toLocal8Bit());
        KeySym keysym_list[] = {s, s};
        XChangeKeyboardMapping(xDisplay, POINTER, 1, keysym_list, 1);


        XTestFakeKeyEvent(xDisplay, POINTER, True, 0);
        XFlush(xDisplay);
        QThread::msleep(10);
        XTestFakeKeyEvent(xDisplay, POINTER, False, 0);
        XFlush(xDisplay);
        QThread::msleep(50);

    }
    KeySym keysym_list[] = { 0, 0 };
    XChangeKeyboardMapping(xDisplay, POINTER, 1, keysym_list, 1);
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QComposeParser test;
    QString chary = test.lastChar;



    QThread::sleep(2);
    sendString(chary);

    return a.exec();
}
