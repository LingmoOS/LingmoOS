#include "idlenotifier.h"


using namespace kdk;

IdleNotifier::IdleNotifier(QObject *parent)
    :QObject(parent)
{
    m_wm = new WmRegister(this);
    connect(m_wm->winInterface(), &AbstractInterface::idled,this,&IdleNotifier::idled);
    connect(m_wm->winInterface(), &AbstractInterface::resumed,this,&IdleNotifier::resumed);
}

void IdleNotifier::setIdleInterval(int msec)
{
     return m_wm->winInterface()->setIdleInterval(msec);
}
