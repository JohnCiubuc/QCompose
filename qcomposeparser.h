#ifndef QCOMPOSEPARSER_H
#define QCOMPOSEPARSER_H

#include <QObject>
#include <QFile>
#include <locale.h>
#include <QDebug>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include "xkeys.h"


#ifndef BOOLFIX
#define BOOLFIX

typedef int Bool;

#endif

#include <X11/Xlib.h>
#include <stdbool.h>

#include <X11/XKBlib.h>
#include <QThread>

struct linked_node
{
    linked_node * parent = NULL;
    QList<linked_node *> nextNodes;
    int modifier;
    KeySym character;
    QString debugCharacter;
    QString sequence;
    void triggerSequence()
    {
        qDebug() << "Trigger: " + sequence;
    }
};

class QComposeParser : public QObject
{
    Q_OBJECT
public:
    explicit QComposeParser(QObject *parent = nullptr);

    QString lastChar;
signals:
public slots:
    void recieveKey(XKeyEvent e);
private:
    QString locale;
    QList<linked_node*> * Nodes;
    QList<linked_node*> * HitNodes;
    QList<KeySym> keyHits;
    bool bIgnoreNext = false;
};

#endif // QCOMPOSEPARSER_H
