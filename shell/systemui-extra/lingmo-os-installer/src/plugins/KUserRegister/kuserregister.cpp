#include "kuserregister.h"

using namespace KInstaller;

KUserRegister::KUserRegister()
{
}

KUserRegister::~KUserRegister()
{
}

QWidget* KUserRegister::CreateObject(QWidget *parent)
{
    this->setObjectName("KUserRegister");
    m_frameUser = new UserFrame(parent);
    return m_frameUser;
}

QWidget* KUserRegister::getWidget()
{
      return m_frameUser;
}

QString KUserRegister::getWidgetName()
{
    QString s = "";
    return (QString)s;
}

void KUserRegister::next()
{

}

void KUserRegister::back()
{

}

void KUserRegister::onActive()
{

}

void KUserRegister::onLeave()
{

}
