#include "ktimezone.h"
#include "timezoneframe.h"

using namespace KInstaller;

KTimeZone::KTimeZone()
{
    this->setObjectName("KTimeZone");
}

KTimeZone::~KTimeZone()
{
}

QWidget* KTimeZone::CreateObject(QWidget *parent)
{
    m_frameTimeZone = new TimeZoneFrame(parent);
    return m_frameTimeZone;
}

QWidget* KTimeZone::getWidget()
{
      return m_frameTimeZone;
}

QString KTimeZone::getWidgetName()
{
    QString s = QString("");
    return (QString)s;
}

void KTimeZone::next()
{

}

void KTimeZone::back()
{

}

void KTimeZone::onActive()
{

}

void KTimeZone::onLeave()
{

}
