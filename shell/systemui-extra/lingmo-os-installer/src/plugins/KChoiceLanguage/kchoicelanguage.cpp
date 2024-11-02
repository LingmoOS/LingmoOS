#include "kchoicelanguage.h"
#include "languageframe.h"

using namespace KInstaller;

KChoiceLanguage::KChoiceLanguage()
{
}

KChoiceLanguage::~KChoiceLanguage()
{
}

QWidget* KChoiceLanguage::CreateObject(QWidget *parent)
{
    m_frameLanguage = new LanguageFrame(parent);
    return m_frameLanguage;
}

QWidget* KChoiceLanguage::getWidget()
{
      return m_frameLanguage;
}

QString KChoiceLanguage::getWidgetName()
{
    QString s = "";
    return (QString)s;
}

void KChoiceLanguage::next()
{

}

void KChoiceLanguage::back()
{

}

void KChoiceLanguage::onActive()
{

}

void KChoiceLanguage::onLeave()
{

}
