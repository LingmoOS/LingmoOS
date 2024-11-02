#ifndef KLINGUISH_H
#define KLINGUISH_H

#include <QObject>
#include <QTranslator>
#include "serverdll.h"

namespace KServer {

class SERVERDLL_SHARE KLinguist : public QObject
{
    Q_OBJECT

private:
    KLinguist();

public:
    static KLinguist *getLinguistInstance();
    QString SERVERDLL_SHARE getCurrentLanguage();
    void setCurrentLanguage(QString str);
    QString currentLanguage;

private:
    static KLinguist *m_linguist;
    QTranslator *trans;

signals:
    void languageChange();

public slots:
    void changeLanguage(QString str);
};

}

#endif // KLINGUISH_H
