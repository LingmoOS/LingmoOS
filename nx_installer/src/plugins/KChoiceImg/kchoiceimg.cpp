#include "kchoiceimg.h"
#include "imgframe.h"
using namespace KInstaller;
KChoiceImg::KChoiceImg()
{
    this->setObjectName("KChoiceImg");
}


KChoiceImg::~KChoiceImg()
{
}

QWidget* KChoiceImg::CreateObject(QWidget *parent)
{
    m_frameLicense = new ImgFrame(parent);
    return m_frameLicense;
}
QWidget* KChoiceImg::getWidget()
{
      return m_frameLicense;
}

QString KChoiceImg::getWidgetName()
{
    QString s = QString("");
    return (QString)s;
}

void KChoiceImg::next()
{

}
void KChoiceImg::back()
{

}
void KChoiceImg::onActive()
{

}
void KChoiceImg::onLeave()
{

}
