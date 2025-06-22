#ifndef KXMLGUITEST_H
#define KXMLGUITEST_H

#include <QObject>
#include <kxmlguiclient.h>

class Client : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    Client()
    {
    }

    using KXMLGUIClient::setComponentName;
    using KXMLGUIClient::setXMLFile;

public Q_SLOTS:
    void slotSec();
};
#endif
