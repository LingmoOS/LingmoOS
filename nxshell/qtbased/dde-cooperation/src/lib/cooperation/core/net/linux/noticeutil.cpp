#include "noticeutil.h"

#include <QDBusInterface>
#include <QDBusReply>

inline constexpr char NotifyServerName[] { "org.freedesktop.Notifications" };
inline constexpr char NotifyServerPath[] { "/org/freedesktop/Notifications" };
inline constexpr char NotifyServerIfce[] { "org.freedesktop.Notifications" };

using namespace cooperation_core;

NoticeUtil::NoticeUtil(QObject *parent)
    : QObject(parent)
{
    initNotifyConnect();
}

NoticeUtil::~NoticeUtil()
{
}

void NoticeUtil::initNotifyConnect()
{
    confirmTimer.setInterval(10 * 1000);
    confirmTimer.setSingleShot(true);

    connect(&confirmTimer, &QTimer::timeout, this, &NoticeUtil::onConfirmTimeout);

    notifyIfc = new QDBusInterface(NotifyServerName,
                                   NotifyServerPath,
                                   NotifyServerIfce,
                                   QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect(NotifyServerName, NotifyServerPath, NotifyServerIfce, "ActionInvoked",
                                          this, SLOT(onActionTriggered(uint, const QString &)));
}

void NoticeUtil::onActionTriggered(uint replacesId, const QString &action)
{
    if (replacesId != recvNotifyId)
        return;

    emit ActionInvoked(action);
}

void NoticeUtil::notifyMessage(const QString &title, const QString &body, const QStringList &actions, QVariantMap hitMap, int expireTimeout)
{
    QDBusReply<uint> reply = notifyIfc->call(QString("Notify"), QString("dde-cooperation"), recvNotifyId,
                                             QString("dde-cooperation"), title, body,
                                             actions, hitMap, expireTimeout);

    recvNotifyId = reply.isValid() ? reply.value() : recvNotifyId;
}

void NoticeUtil::closeNotification()
{
    notifyIfc->call("CloseNotification", recvNotifyId);
}

