#include "klinguist.h"
#include <QCoreApplication>
#include <QDebug>
#include <QLocale>

namespace KServer {



KLinguist *KLinguist::m_linguist = nullptr;

KLinguist::KLinguist()
{
    trans = new QTranslator;
    currentLanguage = QString(getenv("LANGUAGE")).section('.',0,0);
    qDebug() << "72b:"<< currentLanguage;
//    currentLanguage = "zh_CN";
    changeLanguage(currentLanguage);
}

KLinguist *KLinguist::getLinguistInstance()
{
    if (nullptr == m_linguist) {
        m_linguist = new KLinguist;
    }
    return m_linguist;
}

void KLinguist::changeLanguage(QString str)
{
    qDebug() << "KLinguist: " << str;
    if (nullptr != trans)
        qApp->removeTranslator(trans);

    if (currentLanguage == "C")
        str = "en_US";

    bool b = trans->load(":/resources/translations/" + str);

    if (!b) {
        str = "zh_CN";
        trans->load(":/resources/translations/" + str);
    }

    setCurrentLanguage(str);
    qDebug() << str << ".qm install bool: " << b;

    qApp->installTranslator(trans);

    emit languageChange();
}

void KLinguist::setCurrentLanguage(QString str)
{
    currentLanguage = str;
}

QString KLinguist::getCurrentLanguage()
{
    qDebug() << "KLinguist languagez: " << currentLanguage;
    return currentLanguage;
}
}
