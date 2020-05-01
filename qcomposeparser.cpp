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
            d = QString::number(surrogate_to_utf32(d.toUInt(nullptr, 16), b.toUInt(nullptr, 16)), 16);
        }
        d.prepend('U');
        KeySym s = XStringToKeysym(d.toLocal8Bit());
        KeySym keysym_list[] = {s, s};
        XChangeKeyboardMapping(xDisplay, POINTER+bFlash, 1, keysym_list, 1);


        // TODO: Fix / find what causes characters to be ignored randomly on long strings
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

void sendBackspace()
{
    Display * xDisplay ;
    xDisplay = XOpenDisplay(0);

    if(xDisplay == NULL)
        qWarning() << "Unable to open XxDisplay";
    int keycode = XKeysymToKeycode(xDisplay, XK_BackSpace);

    XTestFakeKeyEvent(xDisplay, keycode, True, 0);
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
    QFile mainCompose;
    for (int loadFileL = 0; loadFileL < 2; loadFileL++)
    {
        if(loadFileL == 0)
            mainCompose.setFileName("/home/inathero/.XCompose");
        else
            mainCompose.setFileName("/usr/share/X11/locale/" + locale + "/Compose");
        if(mainCompose.open(QFile::ReadOnly))
        {
            QTextStream in(&mainCompose);
            while (!in.atEnd())
            {
                QString line = in.readLine().simplified();

                if (line.size() == 0) continue;
                if (line[0] == '#') continue;

                QStringList parse = line.split(" ");

                if(parse.size() >= 3)
                {
                    QString first = parse.at(0);
                    if(first.contains("multi_key", Qt::CaseInsensitive))
                    {
                        // Temporarily.
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

                                if (bFirstCharacter)
                                {
                                    bFirstCharacter = false;

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

                            }
                            // final
                            else if (type == '"')
                            {
                                parentNode->sequence = seq;

                                // Start DEBUG to view parsed sequence
//                                QString debugString = "Full Sequence: ";
//                                QString sequence = seq;
//                                lastChar = seq;
//                                QStringList keys;
//                                keys << parentNode->debugCharacter;
//                                while(parentNode->parent != NULL)
//                                {
//                                    parentNode = parentNode->parent;
//                                    keys << parentNode->debugCharacter;
//                                }
//                                for(int i = keys.size()-1; i >= 0; --i)
//                                    debugString += " <" + keys.at(i) + ">";
//                                debugString += " : \"" + sequence + "\"";
//                                qDebug() << debugString;
                                // End Debug
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

    // First key
    // Should be compose key
    if(HitNodes->isEmpty())
    {
        for(int x = 0; x < Nodes->size(); ++x)
        {
            linked_node * Node = Nodes->at(x);
            if(Node->character == ks)
            {
                // Copy next sequence of nodes after compose key
                foreach(auto nextNode, Node->nextNodes)
                {
                    linked_node * linkNode = new linked_node;
                    linkNode->sequence  = nextNode->sequence;
                    linkNode->parent    = nextNode->parent;
                    linkNode->character = nextNode->character;
                    linkNode->nextNodes = nextNode->nextNodes;
                    HitNodes->append(linkNode);
                }

            }
        }
    }
    // After compose key, in the middle of a sequence
    else
    {
        bool bValidSequence = false;
        QList<linked_node*> * tempHitNodes = new  QList<linked_node*>;

        for(int i = 0; i < HitNodes->size(); ++i)
        {
            linked_node * Node = HitNodes->at(i);
            if(Node->character == ks)
            {
                if(!bValidSequence)
                {
                    bValidSequence = true;
                    bIgnoreNext = true;
                    sendBackspace();
                }
                if(Node->sequence.isEmpty())
                {
                    foreach(auto nextNode, Node->nextNodes)
                    {
                        linked_node * linkNode = new linked_node;
                        linkNode->sequence  = nextNode->sequence;
                        linkNode->parent    = nextNode->parent;
                        linkNode->character = nextNode->character;
                        linkNode->nextNodes = nextNode->nextNodes;
                        tempHitNodes->append(linkNode);
                    }
                }
                // Key sequence succesful, launch:
                else
                {
                    sendString(Node->sequence);
                    HitNodes->clear();
                    return;
                }
            }

        }

        // Valid sequence, but no sequences were complete
        if(bValidSequence)
            HitNodes = tempHitNodes;
        //
        else
            HitNodes->clear();
    }
    //Close the connection to the selected X11 display.
    XCloseDisplay(display);
}
