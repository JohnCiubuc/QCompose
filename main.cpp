#include <QCoreApplication>
#include <QDebug>

#include "xkeyboardhook.h"
#include <qcomposeparser.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    XKeyboardHook * hk = new XKeyboardHook;

 QComposeParser();
    return a.exec();
}
