#ifndef XKEYBOARDHOOK_H
#define XKEYBOARDHOOK_H

#include <QObject>
#include <stdio.h>
#include <ctype.h>
#include <QTimer>
#include <QDebug>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class XKeyboardHook : public QObject
{
    Q_OBJECT
signals:
    void detectedKey(XKeyEvent);
public:
    XKeyboardHook();
private slots:
    void watcher();
private:
    QTimer * watchTimer;
    Display* d;
    Window root;
    Window curFocus;
    char buf[17];
    KeySym ks;
    XComposeStatus comp;
    int len;
    int revert;
};

#endif // XKEYBOARDHOOK_H
