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
QComposeParser::QComposeParser(QObject* parent) : QObject(parent)
{
    setlocale(LC_ALL, "");
    locale = setlocale(LC_ALL, NULL);
//    QFile mainCompose("/usr/share/X11/locale/" + locale + "/Compose");
    QFile mainCompose("/home/inathero/.XCompose");
    QFile out("/tmp/out");
    out.open(QFile::WriteOnly);
    QTextStream stream(&out);
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
                QString first = parse.takeFirst();
                if(first.contains("multi_key", Qt::CaseInsensitive))
                {
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
                            seq = seq.toLower();
                            if (bFirstCharacter)
                            {
                                bFirstCharacter = false;
                                // Checking existing nodes
                                foreach (auto Node, Nodes)
                                {
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
                                Nodes << parentNode;
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
                            QString debugString = "Full Sequence: ";
                            QString sequence = seq;
                            lastChar = seq;
                            QStringList keys;
                            keys << parentNode->debugCharacter;
                            while(parentNode->parent != NULL)
                            {
                                parentNode = parentNode->parent;
//                                qDebug() << parentNode->debugCharacter;
                                keys << parentNode->debugCharacter;
                            }
                            for(int i = keys.size()-1; i >= 0; --i)
                                debugString += " <" + keys.at(i) + ">";
                            debugString += " : \"" + sequence + "\"";
                            qDebug() << debugString;
                            stream << debugString << "\n";
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

void QComposeParser::recieveKey(XKeyEvent e)
{
    Display * display;

    //Try to attach to the default X11 display.
    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        printf("Error: Could not open display!\n");
        return ;
    }
    auto ks = XKeycodeToKeysym(display, e.keycode, 0);
    qDebug() << XKeysymToString (ks);
    qDebug() << HitNodes.count();
    if(keyHits.isEmpty())
    {
        qDebug("A");
        bool bFalse = false;
        foreach(auto Node, Nodes)
        {
            if(Node->character == ks)
            {
                bFalse = true;
                if(Node->sequence.isEmpty())
                {
//                    linked_node* n = new linked_node;
//                    n = Node;
//                    HitNodes << n;
                }
                else
                {
                    qDebug("GO");
                    sendString(Node->sequence);
                    keyHits.clear();
                    return;
                }
            }
        }
        if(bFalse)
            keyHits.append(ks);
    }
    else
    {
        qDebug("B");
        bool bFalse = false;
        foreach(auto Node, Nodes)
        {
            qDebug("C1");
            qDebug() << Node->sequence;
            qDebug() << Node->character;
            if(Node->character == keyHits.first())
            {
                foreach(auto Node, Node->nextNodes)
                {
                    if(Node->character == ks)
                    {
                        bFalse = true;
                        if(Node->sequence.isEmpty())
                        {
                            //                    linked_node* n = new linked_node;
                            //                    n = Node;
                            //                    HitNodes << n;
                        }
                        else
                        {
                            qDebug("GO");
                            sendString(Node->sequence);
                            keyHits.clear();
                            return;
                        }
                    }
                }
            }
        }
        if(bFalse)
        {
            keyHits.append(ks);
        }
        else
            keyHits.clear();
    }
    //Close the connection to the selected X11 display.
    XCloseDisplay(display);
}
