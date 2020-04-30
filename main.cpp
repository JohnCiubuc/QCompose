#include <QCoreApplication>
#include <QDebug>

#include "xkeyboardhook.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    XKeyboardHook * hk = new XKeyboardHook;

    return a.exec();
}
