#include "time.h"
#include "timedated_interface.h"

#include <QDateTime>
#include <QSettings>

Time::Time(QObject *parent)
    : QObject(parent)
    , m_settings("lingmoos", "locale")
{
    m_twentyFour = m_settings.value("twentyFour", false).toBool();

    OrgFreedesktopTimedate1Interface iface(QStringLiteral("org.freedesktop.timedate1"),
                                           QStringLiteral("/org/freedesktop/timedate1"),
                                           QDBusConnection::systemBus());
    m_useNtp = iface.nTP();
}

bool Time::useNtp() const
{
    return m_useNtp;
}

void Time::setUseNtp(bool enabled)
{
    if (m_useNtp != enabled) {
        m_useNtp = enabled;
        save();
        emit useNtpChanged();
    }
}

void Time::save()
{
    OrgFreedesktopTimedate1Interface iface(QStringLiteral("org.freedesktop.timedate1"),
                                           QStringLiteral("/org/freedesktop/timedate1"),
                                           QDBusConnection::systemBus());
    auto reply = iface.SetNTP(m_useNtp, true);

    if (!m_useNtp) {
        QDateTime userTime;
        userTime.setTime(currentTime());
        userTime.setDate(currentDate());
        qint64 timeDiff = userTime.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
        //*1000 for milliseconds -> microseconds
        auto reply = iface.SetTime(timeDiff * 1000, true, true);
        reply.waitForFinished();
    }
}

QTime Time::currentTime() const
{
    return m_currentTime;
}

void Time::setCurrentTime(const QTime &currentTime)
{
    if (m_currentTime != currentTime) {
        m_currentTime = currentTime;
        emit currentDateChanged();
    }
}

QDate Time::currentDate() const
{
    return m_currentDate;
}

void Time::setCurrentDate(const QDate &currentDate)
{
    if (m_currentDate != currentDate) {
        m_currentDate = currentDate;
        emit currentDateChanged();
    }
}

bool Time::twentyFour() const
{
    return m_twentyFour;
}

void Time::setTwentyFour(bool t)
{
    if (m_twentyFour != t) {
        m_twentyFour = t;
        m_settings.setValue("twentyFour", m_twentyFour);

        QDBusInterface("com.lingmo.Statusbar", "/Statusbar").call("setTwentyFourTime", m_twentyFour);

        emit twentyFourChanged();
    }
}
