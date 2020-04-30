#include "qcomposeparser.h"

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
