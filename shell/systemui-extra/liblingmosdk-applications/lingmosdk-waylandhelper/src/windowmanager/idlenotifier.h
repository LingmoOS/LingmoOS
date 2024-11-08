#ifndef IDLENOTIFIER_H
#define IDLENOTIFIER_H

#include <QObject>
#include "wmregister.h"


namespace kdk
{


class IdleNotifier: public QObject
{
    Q_OBJECT

public:
    IdleNotifier(QObject *parent = nullptr);
    void setIdleInterval(int msec);

Q_SIGNALS:
    void idled();
    void resumed();


private:
    WmRegister* m_wm = nullptr;
};

}
#endif // IDLENOTIFIER_H
