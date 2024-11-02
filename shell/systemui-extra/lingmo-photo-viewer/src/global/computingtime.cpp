#include "computingtime.h"

ComputingTime *ComputingTime::m_computingTime(nullptr);

ComputingTime *ComputingTime::getInstance()
{
    if (m_computingTime == nullptr) {
        m_computingTime = new ComputingTime();
    }
    return m_computingTime;
}

void ComputingTime::start()
{
    m_timer->start(10);
}

QString ComputingTime::getTime(const QString &tag)
{
    QString str = QString::number(m_time, 'g', 2);
    QString out = tag + ":" + str + "\n";
    //    printf(out.toLocal8Bit().data());
    return str;
}

ComputingTime::ComputingTime()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ComputingTime::computingTime);
}

void ComputingTime::computingTime()
{
    m_time += 0.01;
}
