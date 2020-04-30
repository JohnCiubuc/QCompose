#include "qcomposeparser.h"

#define POINTER 0x20ad


int surrogate_to_utf32(int high, int low)
{
    return (high << 10) + low - 0x35fdc00;
}

QString getUnicodeEscape(QString str)
{
    QString escaped;
    escaped.reserve(6 * str.size());
    for (QString::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        QChar ch = *it;
        ushort code = ch.unicode();
//        escaped += "U";
        escaped += QString::number(code, 16).rightJustified(4, '0');
    }
    return escaped;
}

void sendString(QString string)
{
    Display * xDisplay ;
    xDisplay = XOpenDisplay(0);
    KeySym blank[] = {0,0};

    if(xDisplay == NULL)
        qWarning() << "Unable to open XxDisplay";

    qDebug() << "String sequence - " << string;
    bool bFlash = false;
    for(int i = 0; i < string.length(); i++)
    {

        QString c = QString(string[i]);
        auto d = getUnicodeEscape(c);

        // SURROGATE PAIR
        if(d[0] == 'd')
        {
            i++;
            QString c = QString(string[i]);
            auto b = getUnicodeEscape(c);
            qDebug() << "~~~~~";
            qDebug() << d;
            qDebug() << b;
            d = QString::number(surrogate_to_utf32(d.toUInt(nullptr, 16), b.toUInt(nullptr, 16)), 16);
        }
//        d = "U1F496";
        d.prepend('U');
        qDebug() << d;
        KeySym s = XStringToKeysym(d.toLocal8Bit());
        KeySym keysym_list[] = {s, s};
        XChangeKeyboardMapping(xDisplay, POINTER+bFlash, 1, keysym_list, 1);


        QThread::msleep(20);
        XTestFakeKeyEvent(xDisplay, POINTER+bFlash, True, 0);
//        XFlush(xDisplay);
        QThread::msleep(20);
        XTestFakeKeyEvent(xDisplay, POINTER+bFlash, False, 0);
        XTestFakeKeyEvent(xDisplay, POINTER+bFlash, False, 0);
        XFlush(xDisplay);
        XTestFakeKeyEvent(xDisplay, POINTER+bFlash, False, 0);
        XFlush(xDisplay);
        QThread::msleep(20);

        XChangeKeyboardMapping(xDisplay, POINTER+bFlash, 1, blank, 1);
        QThread::msleep(20);

        bFlash = !bFlash;
    }
}

void sendUCode(QString uc)
{
    Display * xDisplay ;
    xDisplay = XOpenDisplay(0);
    KeySym blank[] = {0,0};

    if(xDisplay == NULL)
        qWarning() << "Unable to open XxDisplay";

    KeySym s = XStringToKeysym(uc.toLocal8Bit());
    int keycode = XKeysymToKeycode(xDisplay, XK_BackSpace);

    XTestFakeKeyEvent(xDisplay, keycode, True, 0);
//        XFlush(xDisplay);
//    QThread::msleep(10);
    XTestFakeKeyEvent(xDisplay, keycode, False, 0);
    XFlush(xDisplay);
    QThread::msleep(10);
}
QComposeParser::QComposeParser(QObject* parent) : QObject(parent)
{
    setlocale(LC_ALL, "");
    HitNodes = new  QList<linked_node*>;
    Nodes = new  QList<linked_node*>;
    locale = setlocale(LC_ALL, NULL);
//    QFile mainCompose("/usr/share/X11/locale/" + locale + "/Compose");
    QFile mainCompose;
    for (int loadFileL = 0; loadFileL < 2; loadFileL++)
    {
        if(loadFileL == 0)
            mainCompose.setFileName("/home/inathero/.XCompose");
        else
        {
//            return;
            mainCompose.setFileName("/usr/share/X11/locale/" + locale + "/Compose");
        }
        if(mainCompose.open(QFile::ReadOnly))
        {
            QTextStream in(&mainCompose);
            int i = 0;
            while (!in.atEnd())
            {
                i++;
                if (i > 70)
                    break;
                QString line = in.readLine().simplified();

                if (line.size() == 0) continue;
                if (line[0] == '#') continue;

                QStringList parse = line.split(" ");

                if(parse.size() >= 3)
                {
//                QString first = parse.takeFirst();
                    QString first = parse.at(0);
                    if(first.contains("multi_key", Qt::CaseInsensitive))
                    {
                        parse[0] = "<Alt_R>";
                        bool bFirstCharacter = true;
                        linked_node * parentNode = NULL;
                        foreach (QString seq, parse)
                        {
                            char type;
                            try
                            {
                                type = seq[0].toLatin1();
                            }
                            catch (...)
                            {
                                qWarning() << "Invalid sequence";
                                break;
                            }
                            seq.chop(1);
                            seq.remove(0, 1);
                            //current
                            if(type == '<')
                            {
//                            seq = seq.toLower();
                                if (bFirstCharacter)
                                {
                                    bFirstCharacter = false;
                                    // Checking existing nodes
//                                foreach (auto  Node, Nodes)
                                    for(int x = 0; x < Nodes->size(); ++x)
                                    {
                                        linked_node * Node = Nodes->at(x);
                                        // Existing node found
                                        if(Node->character == XStringToKeysym(seq.toLocal8Bit()))
                                        {
                                            // Set current node to next node
                                            parentNode = Node;
                                            continue;
                                        }
                                    }
                                    // Make new node
                                    parentNode = new linked_node;
                                    parentNode->character = XStringToKeysym(seq.toLocal8Bit());
                                    parentNode->debugCharacter = seq;
                                    Nodes->append(parentNode);
                                }
                                else
                                {
                                    // Check for next node
                                    foreach(auto Node, parentNode->nextNodes)
                                    {
                                        // Existing node found
                                        if(Node->character == XStringToKeysym(seq.toLocal8Bit()))
                                        {
                                            parentNode = Node;
                                            continue;
                                        }
                                    }
                                    // No child nodes:

                                    linked_node * newNode = new linked_node;
                                    newNode->character = XStringToKeysym(seq.toLocal8Bit());
                                    newNode->debugCharacter = seq;
                                    newNode->parent = parentNode;
                                    parentNode->nextNodes << newNode;
                                    parentNode = newNode;
                                }

//                            qDebug() << seq << "-" << XStringToKeysym(seq.toLocal8Bit());
                            }
                            // final
                            else if (type == '"')
                            {
                                parentNode->sequence = seq;
//                            qDebug() << "final - " << seq << "-" << XStringToKeysym(seq.toLocal8Bit());
                                // DEBUG
//                                QString debugString = "Full Sequence: ";
//                                QString sequence = seq;
//                                lastChar = seq;
//                                QStringList keys;
//                                keys << parentNode->debugCharacter;
//                                while(parentNode->parent != NULL)
//                                {
//                                    parentNode = parentNode->parent;
////                                qDebug() << parentNode->debugCharacter;
//                                    keys << parentNode->debugCharacter;
//                                }
//                                for(int i = keys.size()-1; i >= 0; --i)
//                                    debugString += " <" + keys.at(i) + ">";
//                                debugString += " : \"" + sequence + "\"";
                            }
                        }
                    }
                }
                else
                    qWarning() << "Invalid sequence detected";

            }
            mainCompose.close();
        }
    }
}

void QComposeParser::recieveKey(XKeyEvent e)
{
    if(bIgnoreNext)
    {
        bIgnoreNext = false;
        return;
    }
    Display * display;

    //Try to attach to the default X11 display.
    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        printf("Error: Could not open display!\n");
        return ;
    }
    auto ks = XkbKeycodeToKeysym( display, e.keycode,
                                  0, e.state & ShiftMask ? 1 : 0);
    qDebug() << XKeysymToString (ks);
//    qDebug() << XStringToKeysym("Alt_R");

    // First key
    // Should be compose key
    if(HitNodes->isEmpty())
    {
        qDebug("A");
        bool bFalse = false;
        for(int x = 0; x < Nodes->size(); ++x)
        {
            linked_node * Node = Nodes->at(x);

            qDebug() << Node->character;
            qDebug() << ks;
            qDebug() << XKeysymToString (Node->character);
            qDebug() << XKeysymToString (ks);
//            qDebug() << XKeysymToString (Node->character);
            if(Node->character == ks)
            {
                foreach(auto nextNode, Node->nextNodes)
                {
                    bFalse = true;
                    linked_node * linkNode = new linked_node;
                    linkNode->sequence = nextNode->sequence;
                    linkNode->parent = nextNode->parent;
                    linkNode->character =nextNode->character;
                    linkNode->nextNodes =nextNode->nextNodes;
                    HitNodes->append(linkNode);
                    qDebug("add");
                }

            }
        }
        for(int i = 0; i < HitNodes->size(); ++i)
        {
            linked_node * pNode = HitNodes->at(i);
            linked_node * Node = pNode;
            qDebug("-1-------------");
//            qDebug() << pNode;
//            qDebug() << *pNode;
//            qDebug() << (*pNode)->character;
            qDebug() << Node->character;
            qDebug() << ks;
            qDebug() << XKeysymToString (Node->character);
            qDebug() << XKeysymToString (ks);
        }
//        if(bFalse)
//            keyHits.append(ks);
    }
    else
    {
//        for(int i = 0; i < HitNodes->size(); ++i)
//        {
//            linked_node ** pNode = HitNodes->at(i);
//            linked_node * Node = *pNode;
//            qDebug("-2-------------");
////            qDebug() << pNode;
////            qDebug() << Node;
////            qDebug() << *pNode;
////            qDebug() << (*pNode)->character;
//            qDebug() << Node->character;
//            qDebug() << ks;
//            qDebug() << XKeysymToString (Node->character);
//            qDebug() << XKeysymToString (ks);
//        }
//        return;;
        qDebug("B");
        bool bFalse = false;
        QList<linked_node*> * tempHitNodes = new  QList<linked_node*>  ;
        for(int i = 0; i < HitNodes->size(); ++i)
        {
            qDebug("B1");
            linked_node * pNode = HitNodes->at(i);
            linked_node * Node = pNode;
            qDebug() << Node;
            qDebug() << XKeysymToString (Node->character);
            qDebug() << Node->sequence;
            qDebug() << Node->nextNodes.size();
            if(Node->character == ks)
            {
                if(!bFalse)
                {
                    bFalse = true;
                    bIgnoreNext = true;
                    sendUCode("U007f");
                }
                if(Node->sequence.isEmpty())
                {
                    foreach(auto nextNode, Node->nextNodes)
                    {
                        linked_node * linkNode = new linked_node;
                        linkNode->sequence = nextNode->sequence;
                        linkNode->parent = nextNode->parent;
                        linkNode->character =nextNode->character;
                        linkNode->nextNodes =nextNode->nextNodes;
                        tempHitNodes->append(linkNode);
                        qDebug("add");
                    }
                }
                else
                {
                    qDebug("GO");
                    sendString(Node->sequence);
                    HitNodes->clear();
                    return;
                }
            }

        }

        if(bFalse)
        {
            qDebug() << "copy";
            HitNodes = tempHitNodes;

        }
        else
            HitNodes->clear();
    }
    //Close the connection to the selected X11 display.
    XCloseDisplay(display);
}
