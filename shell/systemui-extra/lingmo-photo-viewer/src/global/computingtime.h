#ifndef COMPUTINGTIME_H
#define COMPUTINGTIME_H

#include <QObject>
#include <QTimer>
#include <QDebug>
class ComputingTime : public QObject
{
    Q_OBJECT
public:
    static ComputingTime *getInstance();
    void start();
    QString getTime(const QString &tag = "");

private:
    ComputingTime();
    static ComputingTime *m_computingTime;
    QTimer *m_timer = nullptr;
    double m_time = 0;

private Q_SLOTS:
    void computingTime();
};

#endif // COMPUTINGTIME_H
