#include "kylicense.h"
#include "licenseframe.h"
using namespace KInstaller;
KyLicense::KyLicense()
{
}


KyLicense::~KyLicense()
{
}

QWidget* KyLicense::CreateObject(QWidget *parent)
{
    m_frameLicense = new LicenseFrame(parent);
    return m_frameLicense;
}
QWidget* KyLicense::getWidget()
{
      return m_frameLicense;
}

QString KyLicense::getWidgetName()
{
    QString s = QString("");
    return (QString)s;
}

void KyLicense::next()
{

}
void KyLicense::back()
{

}
void KyLicense::onActive()
{

}
void KyLicense::onLeave()
{

}
