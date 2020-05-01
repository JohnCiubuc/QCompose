#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QThread>

#include "xkeyboardhook.h"
#include <qcomposeparser.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QComposeParser * test = new QComposeParser;

    XKeyboardHook * xk = new XKeyboardHook;
    QObject::connect(xk, &XKeyboardHook::detectedKey, test, &QComposeParser::recieveKey);

    return a.exec();
}
