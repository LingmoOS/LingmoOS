#ifdef Q_OS_UNIX
#include <stdlib.h>
#endif

#include <QDebug>
#include <QThread>
#include <QTimer>

#include "AverageLoadManager.h"

AverageLoadManager::AverageLoadManager(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_min()
    , m_max(0)
{
    m_timer->setSingleShot(false);
    m_timer->setInterval(500);
    connect(m_timer, SIGNAL(timeout()), SLOT(update()));
}

void AverageLoadManager::activate(bool enabled)
{
    if (available()) {
        if (enabled) {
            m_timer->start();
        } else {
            m_timer->stop();
        }
    }
}

bool AverageLoadManager::available() const
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
    return true;
#else
    return false;
#endif
}

QPair<int, int> AverageLoadManager::workersRange() const
{
    return qMakePair(m_min, m_max);
}

void AverageLoadManager::update()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
    double averages[3];
    if (getloadavg(averages, 3) == -1) {
        return;
    }

    const float processors = QThread::idealThreadCount();
    const float relativeLoadPerProcessor = averages[0] / processors; // relative system load
    const float targetedBaseLoad = 0.7f;

    const float x = relativeLoadPerProcessor / targetedBaseLoad;
    auto const linearLoadFunction = [](float x) {
        return -x + 2.0f;
    };
    // auto const reciprocalLoadFunction = [](float x) { return 1.0f / (0.5*x+0.5); };

    m_min = qRound(qMax(1.0f, linearLoadFunction(1000 * processors)));
    m_max = qRound(qMin(2 * processors, processors * linearLoadFunction(0.0)));
    const float y = linearLoadFunction(x);
    const int threads = qBound(m_min, qRound(processors * y), m_max);
    qDebug() << threads << y << x << relativeLoadPerProcessor << averages[0] << processors;
    Q_EMIT recommendedWorkerCount(threads);
#endif
}

#include "moc_AverageLoadManager.cpp"
