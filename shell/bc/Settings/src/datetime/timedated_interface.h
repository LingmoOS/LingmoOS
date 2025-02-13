#ifndef TIMEDATED_INTERFACE_H
#define TIMEDATED_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.freedesktop.timedate1
 */
class OrgFreedesktopTimedate1Interface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.timedate1"; }

public:
    OrgFreedesktopTimedate1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~OrgFreedesktopTimedate1Interface();

    Q_PROPERTY(bool CanNTP READ canNTP)
    inline bool canNTP() const
    { return qvariant_cast< bool >(property("CanNTP")); }

    Q_PROPERTY(bool LocalRTC READ localRTC)
    inline bool localRTC() const
    { return qvariant_cast< bool >(property("LocalRTC")); }

    Q_PROPERTY(bool NTP READ nTP)
    inline bool nTP() const
    { return qvariant_cast< bool >(property("NTP")); }

    Q_PROPERTY(bool NTPSynchronized READ nTPSynchronized)
    inline bool nTPSynchronized() const
    { return qvariant_cast< bool >(property("NTPSynchronized")); }

    Q_PROPERTY(qulonglong RTCTimeUSec READ rTCTimeUSec)
    inline qulonglong rTCTimeUSec() const
    { return qvariant_cast< qulonglong >(property("RTCTimeUSec")); }

    Q_PROPERTY(qulonglong TimeUSec READ timeUSec)
    inline qulonglong timeUSec() const
    { return qvariant_cast< qulonglong >(property("TimeUSec")); }

    Q_PROPERTY(QString Timezone READ timezone)
    inline QString timezone() const
    { return qvariant_cast< QString >(property("Timezone")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> SetLocalRTC(bool in0, bool in1, bool in2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2);
        return asyncCallWithArgumentList(QStringLiteral("SetLocalRTC"), argumentList);
    }

    inline QDBusPendingReply<> SetNTP(bool in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetNTP"), argumentList);
    }

    inline QDBusPendingReply<> SetTime(qlonglong in0, bool in1, bool in2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2);
        return asyncCallWithArgumentList(QStringLiteral("SetTime"), argumentList);
    }

    inline QDBusPendingReply<> SetTimezone(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetTimezone"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace org {
  namespace freedesktop {
    typedef ::OrgFreedesktopTimedate1Interface timedate1;
  }
}
#endif
