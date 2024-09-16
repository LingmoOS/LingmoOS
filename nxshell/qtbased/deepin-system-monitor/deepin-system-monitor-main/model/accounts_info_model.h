#ifndef ACCOUNTS_INFO_MODEL_H
#define ACCOUNTS_INFO_MODEL_H
#include "user.h"
//#include <com_deepin_daemon_accounts.h>
//#include <com_deepin_daemon_accounts_user.h>
//#include <org_freedesktop_login1.h>
//#include <com_deepin_dde_controlcenter.h>
#include <QMetaType>
#include <QtDBus>
#include <QDBusInterface>


//using Accounts = com::deepin::daemon::Accounts;
//using AccountsUser = com::deepin::daemon::accounts::User;
//using LoginManager = org::freedesktop::login1::Manager;
//using ControlCenter = com::deepin::dde::ControlCenter;

typedef struct _SessionInfo {
    QString sessionId;
    uint userId;
    QString userName;
    QString seatId;
    QDBusObjectPath sessionPath;
} SessionInfo;
typedef QList<SessionInfo> SessionInfoList;
Q_DECLARE_METATYPE(SessionInfoList);
Q_DECLARE_METATYPE(SessionInfo);


inline QDBusArgument &operator<<(QDBusArgument &argument, const SessionInfo &sessionInfo)
{
    argument.beginStructure();
    argument << sessionInfo.sessionId;
    argument << sessionInfo.userId;
    argument << sessionInfo.userName;
    argument << sessionInfo.seatId;
    argument << sessionInfo.sessionPath;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SessionInfo &sessionInfo)
{
    argument.beginStructure();
    argument >> sessionInfo.sessionId;
    argument >> sessionInfo.userId;
    argument >> sessionInfo.userName;
    argument >> sessionInfo.seatId;
    argument >> sessionInfo.sessionPath;
    argument.endStructure();

    return argument;
}




class AccountsInfoModel : public QObject
{
    Q_OBJECT

public:
    explicit AccountsInfoModel(QObject *parent = nullptr);
    virtual ~AccountsInfoModel();
    void updateUserList(const QStringList &userPathList);
    QList<User *> userList() const;
    User::UserType getCurrentUserType() const;
    bool lockSessionByUserName(const QString &userName);
    bool activateSessionByUserName(const QString &userName);
    bool LogoutByUserName(const QString &userName);
    void EditAccount();
Q_SIGNALS:
    void signalUserOnlineStatusUpdated();


public Q_SLOTS:
    void onUserListChanged(const QStringList &userPathList);
    void updateUserOnlineStatus();

    void onSessionNew(const QString &in0, const QDBusObjectPath &in1);
    void onSessionRemoved(const QString &in0, const QDBusObjectPath &in1);


private:
    QDBusInterface *m_accountsInter;
    QMap<QString, User * > m_userMap ;
    QStringList m_onlineUsers;
    QDBusInterface *m_LoginInter;
    QDBusInterface *m_controlInter;
    QString m_currentUserName;
    int m_currentUserType;
    SessionInfoList m_sessionList {};
};


#endif // ACCOUNTS_INFO_MODEL_H
