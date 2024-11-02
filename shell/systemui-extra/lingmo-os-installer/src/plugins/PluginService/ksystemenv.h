#ifndef KSYSTEMENV_H
#define KSYSTEMENV_H

#include <QString>
#include <QObject>
#include "serverdll.h"

namespace KServer {

class SERVERDLL_SHARE KSystemENV : public QObject
{
    Q_OBJECT
public:
    explicit KSystemENV(QObject* parent = nullptr);
    ~KSystemENV();
    static KSystemENV* init();
    static KSystemENV* getInstance();

    void readENVToFile();
    QString SERVERDLL_SHARE getENVValue(QString name);


public:
    static KSystemENV* m_envInstance;
    QStringList m_envStrs;
};


//QString getStringEVN(QString evnStr);


}

#endif // KSystemENV_H
