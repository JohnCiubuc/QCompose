#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include "xkeyboardhook.h"
#include <qcomposeparser.h>
#include <QObject>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QComposeParser * test = new QComposeParser;

    XKeyboardHook * xk = new XKeyboardHook;
    QObject::connect(xk, &XKeyboardHook::detectedKey, test, &QComposeParser::recieveKey);
//    for (int i = 0; i < chary.length(); i++)
//    {
//        qDebug() << chary[i];
//    }
//    return 0;
//    QThread::sleep(2);
//    qDebug() << chary;
//    sendString(chary);

    return a.exec();
}
