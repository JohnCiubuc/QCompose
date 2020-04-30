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
private:
    QString locale;
    QList<linked_node*> Nodes;
};

#endif // QCOMPOSEPARSER_H
