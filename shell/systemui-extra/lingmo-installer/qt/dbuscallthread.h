#ifndef DBUSCALLTHREAD_H
#define DBUSCALLTHREAD_H

#include <QThread>
#include <QObject>
#include "titlewidget.h"

class DbusCallThread : public QThread
{
    Q_OBJECT
public:
    DbusCallThread(QVariantList json, QString runtime, QString appName, QString debPath);


protected:
    void run();

private:
    QString         m_runtime;
    QString         m_appName;
    QVariantList    m_jsonInfo;
    QString         m_debPath;

signals:
    void installFail();
    void installSuccess();

};

#endif // DBUSCALLTHREAD_H
