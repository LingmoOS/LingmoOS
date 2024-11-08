#include "knotifier.h"
#include <QStringList>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QList>
#include <QVariant>
#include <QDebug>

namespace kdk {

#define SERVICE "org.freedesktop.Notifications"
#define PATH "/org/freedesktop/Notifications"
#define INTERFACE "org.freedesktop.Notifications"

#define NOTIFY "Notify"
#define CLOSE "CloseNotification"


static QList<QVariant> dbusCall(QString serviceName, QString objectPath, QString interfaceName,
                                      QString methodName, QList<QVariant> args = QList<QVariant>())
{
    QList<QVariant> ret;
    ret.clear();

    if (serviceName.isEmpty() || objectPath.isEmpty() || interfaceName.isEmpty() || methodName.isEmpty()) {
        qDebug() << "knotifier : args error!";
        return ret;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(serviceName, objectPath, interfaceName, methodName);
    if (!args.isEmpty()) {
        message.setArguments(args);
    }

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ReplyMessage) {
        ret = reply.arguments();
        return ret;
    } else {
        qDebug() << "knotifier : dbus call method fail " << reply.errorMessage();
    }

    return ret;
}

class KNotifierPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KNotifier)

public:
     KNotifierPrivate(KNotifier *parent = nullptr);

public:
    KNotifier *q_ptr;
    uint m_replaceId;
    uint m_id;
    int m_showTime;
    QString m_bodyText;
    QString m_summary;
    QString m_iconName;
    QString m_appName;
    QStringList m_actionList;

};

KNotifier::KNotifier(QObject *parent) :
    QObject(parent),
    d_ptr(new KNotifierPrivate(this))
{
    Q_D(KNotifier);
}

KNotifier::~KNotifier()
{

}

void KNotifier::setDefaultAction(const QString &appName)
{
    Q_D(KNotifier);
    d->m_actionList.insert(0,"default");
    d->m_actionList.insert(1,appName);

}

uint KNotifier::notify()
{
    Q_D(KNotifier);

    QMap<QString, QVariant> hints;
    QList<QVariant> args;
    args << d->m_appName
         << d->m_replaceId
         << d->m_iconName
         << d->m_summary
         << d->m_bodyText
         << d->m_actionList
         << hints
         << d->m_showTime;
    QList<QVariant> list = dbusCall(SERVICE,PATH,INTERFACE,NOTIFY,args);
    d->m_id = list.at(0).toUInt();
    return d->m_id;
}

void KNotifier::addAction(const QString &appName, const QString &text)
{
    Q_D(KNotifier);

    d->m_actionList.append(appName);
    d->m_actionList.append(text);
}

void KNotifier::setShowTime(int milliseconds)
{
    Q_D(KNotifier);

    d->m_showTime = milliseconds;
}

void KNotifier::setAppName(const QString &appName)
{
    Q_D(KNotifier);

    d->m_appName = appName;
}

void KNotifier::setBodyText(const QString &bodyText)
{
    Q_D(KNotifier);

    d->m_bodyText = bodyText;
}

void KNotifier::setSummary(const QString &summary)
{
    Q_D(KNotifier);

    d->m_summary = summary;
}

void KNotifier::setAppIcon(const QString &iconName)
{
    Q_D(KNotifier);

    d->m_iconName = iconName;
}

void KNotifier::setReplaceId(const uint id)
{
    Q_D(KNotifier);
    d->m_replaceId = id;
}

void KNotifier::closeNotification(uint id)
{

    QList<QVariant> args;
    args << id;
    dbusCall(SERVICE,PATH,INTERFACE,CLOSE,args);
}

KNotifierPrivate::KNotifierPrivate(KNotifier *parent)
    :q_ptr(parent),
      m_replaceId(0),
      m_showTime(KNotifier::Default)
{
    setParent(parent);
}

}
#include "knotifier.moc"
#include "moc_knotifier.cpp"
