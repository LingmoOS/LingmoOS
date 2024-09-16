// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountsuser.h"

#include "daccountsuser_p.h"
#include "dglobalconfig.h"
#include "dutils.h"

#include <QDebug>
#include <QMimeDatabase>
#include <QRegularExpression>

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

DACCOUNTS_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DAccountsUserPrivate::DAccountsUserPrivate(const quint64 uid, DAccountsUser *parent)
    : q_ptr(parent)
{
#if defined(USE_FAKE_INTERFACE)
    const auto &freeDesktopPath = "/com/deepin/daemon/FakeAccounts/User" + QString::number(uid);
    const auto &daemonPath = "/com/deepin/daemon/FakeAccounts/User" + QString::number(uid);
#else
    const auto &freeDesktopPath = "/org/freedesktop/Accounts/User" + QString::number(uid);
    const auto &daemonPath = "/org/deepin/dde/Accounts1/User" + QString::number(uid);
#endif
    m_dSystemUserInter = new DSystemUserInterface(daemonPath, this);
    m_dUserInter = new DUserInterface(freeDesktopPath, this);
}

DAccountsUser::DAccountsUser(const quint64 uid, QObject *parent)
    : QObject(parent)
    , d_ptr(new DAccountsUserPrivate(uid, this))
{
    Q_D(const DAccountsUser);
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::AutomaticLoginChanged,
            this,
            [this](const bool enabled) {
                emit this->automaticLoginChanged(enabled);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::GroupsChanged,
            this,
            [this](const QStringList &list) {
                emit this->groupsChanged(list);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::LayoutChanged,
            this,
            [this](const QString &layout) {
                emit this->layoutChanged(layout.toUtf8());
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::HistoryLayoutChanged,
            this,
            [this](const QStringList &list) {
                QList<QByteArray> tmp;
                for (const auto &v : list)
                    tmp.append(v.toUtf8());
                emit this->layoutListChanged(tmp);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::IconListChanged,
            this,
            [this](const QStringList &list) {
                QList<QByteArray> tmp;
                for (const auto &v : list)
                    tmp.append(v.toUtf8());
                emit this->iconFileListChanged(tmp);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::IconFileChanged,
            this,
            [this](const QString &url) {
                QUrl tmp(url);
                emit this->iconFileChanged(url);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::LocaleChanged,
            this,
            [this](const QString &locale) {
                emit this->localeChanged(locale.toUtf8());
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::LockedChanged,
            this,
            [this](const bool locked) {
                emit this->lockedChanged(locked);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::MaxPasswordAgeChanged,
            this,
            [this](const qint32 nDays) {
                emit this->maxPasswordAgeChanged(nDays);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::NoPasswdLoginChanged,
            this,
            [this](const bool enabled) {
                emit this->noPasswdLoginChanged(enabled);
            });
    connect(d->m_dSystemUserInter,
            &DSystemUserInterface::PasswordHintChanged,
            this,
            [this](const QString &hint) {
                emit this->passwordHintChanged(hint);
            });

    connect(d->m_dUserInter, &DUserInterface::DataChanged, this, &DAccountsUser::userDataChanged);
}

DAccountsUser::~DAccountsUser() { }

AccountTypes DAccountsUser::accountType() const
{
    auto typenum = d_ptr->m_dSystemUserInter->accountType();
    switch (typenum) {
    case 0:
        return AccountTypes::Default;
    case 1:
        return AccountTypes::Admin;
    case 2:
        return AccountTypes::Udcp;
    default:
        return AccountTypes::Unknown; // function should not be executed here
    }
}

bool DAccountsUser::automaticLogin() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->automaticLogin();
}

QString DAccountsUser::fullName() const
{
    Q_D(const DAccountsUser);
    return d->m_dUserInter->realName();
}

quint64 DAccountsUser::GID() const
{
    auto userinfo = getpwnam(userName());
    endpwent();
    if (userinfo == nullptr) {
        qWarning() << strerror(errno);
        return 0;
    }
    return userinfo->pw_gid;
}

quint64 DAccountsUser::UID() const
{
    Q_D(const DAccountsUser);
    return d->m_dUserInter->UID();
}

QStringList DAccountsUser::groups() const
{
    QStringList groupList;
    gid_t *groups{ nullptr };
    int ngroups{ 0 };
    auto username = userName();
    auto gid = GID();
    getgrouplist(username, gid, groups, &ngroups);
    groups = reinterpret_cast<gid_t *>(malloc(sizeof(gid_t) * ngroups));
    if (getgrouplist(username, gid, groups, &ngroups) == -1) {
        qWarning() << strerror(errno);
        free(groups);
        return groupList;
    }
    for (int i = 0; i < ngroups; ++i)
        groupList.append(getgrgid(groups[i])->gr_name);
    free(groups);
    return groupList;
}

QList<QByteArray> DAccountsUser::layoutList() const
{
    Q_D(const DAccountsUser);
    QList<QByteArray> layouts;
    const auto &reply = d->m_dSystemUserInter->historyLayout();
    for (const auto &v : reply)
        layouts.push_back(v.toUtf8());
    return layouts;
}

QString DAccountsUser::homeDir() const
{
    Q_D(const DAccountsUser);
    return d->m_dUserInter->homeDirectory();
}

QList<QByteArray> DAccountsUser::iconFileList() const
{
    Q_D(const DAccountsUser);
    QList<QByteArray> icons;
    QDir icondir(UserIconsDir);
    QFileInfo cusdirinfo(UserCustomIconsDir);
    if (!icondir.exists()) {
        return icons;
    }
    icondir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    icondir.setSorting(QDir::NoSort);
    icons.append(d->getImageFromDir(icondir));
    if (cusdirinfo.exists() and cusdirinfo.isDir()) {
        auto cusdir = QDir(cusdirinfo.absoluteFilePath());
        cusdir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        cusdir.setSorting(QDir::NoSort);
        icons.append(d->getImageFromDir(cusdir));
    }
    return icons;
}

QByteArray DAccountsUser::iconFile() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->iconFile().toUtf8();
}

QByteArray DAccountsUser::layout() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->layout().toUtf8();
}

QByteArray DAccountsUser::locale() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->locale().toUtf8();
}

bool DAccountsUser::locked() const
{
    Q_D(const DAccountsUser);
    return d->m_dUserInter->locked();
}

qint32 DAccountsUser::maxPasswordAge() const
{
    Q_D(const DAccountsUser);
    auto age = d->m_dSystemUserInter->maxPasswordAge();
    return age;
}

QString DAccountsUser::passwordHint() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->passwordHint();
}

QDateTime DAccountsUser::passwordLastChange() const
{
    Q_D(const DAccountsUser);
    auto days = d->m_dSystemUserInter->passwordLastChange();
    return QDateTime::fromSecsSinceEpoch(0).addDays(days);
}

PasswdStatus DAccountsUser::passwordStatus() const
{
    Q_D(const DAccountsUser);
    if (d->m_dUserInter->locked())
        return PasswdStatus::Locked;
    auto mode = d->m_dUserInter->passwordMode();
    if (mode == 0)
        return PasswdStatus::Password;
    else if (mode == 2)
        return PasswdStatus::NoPassword;
    // function should not be executed here
    return PasswdStatus::Unknown;
}

QString DAccountsUser::shell() const
{
    Q_D(const DAccountsUser);
    return d->m_dUserInter->shell();
}

QByteArray DAccountsUser::userName() const
{
    Q_D(const DAccountsUser);
    return d->m_dUserInter->userName().toUtf8();
}

QByteArray DAccountsUser::UUID() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->UUID().toUtf8();
}

bool DAccountsUser::noPasswdLogin() const
{
    Q_D(const DAccountsUser);
    return d->m_dSystemUserInter->noPasswdLogin();
}

QDateTime DAccountsUser::loginTime() const
{
    Q_D(const DAccountsUser);
    auto time = d->m_dUserInter->loginTime();
    return QDateTime::fromSecsSinceEpoch(time);
}

QDateTime DAccountsUser::createdTime() const
{
    Q_D(const DAccountsUser);
    return QDateTime::fromSecsSinceEpoch(d->m_dSystemUserInter->createdTime());
}

DExpected<void> DAccountsUser::setNopasswdLogin(const bool enabled)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->enableNoPasswdLogin(enabled);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setAutomaticLogin(bool enabled)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setAutomaticLogin(enabled);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setFullName(const QString &fullname)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dUserInter->setRealName(fullname);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setGroups(const QStringList &newgroups)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setGroups(newgroups);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setLayoutList(const QList<QByteArray> &newlayouts)
{
    Q_D(const DAccountsUser);
    QStringList tmp;
    for (const auto &layout : newlayouts) {
        tmp.push_back(layout);
    }
    auto reply = d->m_dSystemUserInter->setHistoryLayout(tmp);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setHomeDir(const QString &newhomedir)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dUserInter->setHomeDirectory(newhomedir);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setIconFile(const QUrl &newiconURL)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setIconFile(newiconURL.toString());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setLayout(const QByteArray &newlayout)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setLayout(newlayout);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setLocale(const QByteArray &newlocale)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setLocale(newlocale);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setLocked(const bool locked)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setLocked(locked);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setMaxPasswordAge(const int newndays)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setMaxPasswordAge(newndays);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setPassword(const QByteArray &newpassword)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setPassword(Dutils::encryptPassword(newpassword));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setPasswordHint(const QString &newpasswordhint)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->setPasswordHint(newpasswordhint);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::setShell(const QString &newshellpath)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dUserInter->setShell(newshellpath);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::addGroup(const QString &group)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->addGroup(group);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::deleteGroup(const QString &group)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->deleteGroup(group);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<void> DAccountsUser::deleteIconFile(const QUrl &iconURL)
{
    Q_D(const DAccountsUser);
    auto reply = d->m_dSystemUserInter->deleteIconFile(iconURL.toLocalFile());
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    return {};
}

DExpected<bool> DAccountsUser::isPasswordExpired() const
{
    auto age = maxPasswordAge();
    if (age > 0)
        return true;
    return false;
}

DExpected<ReminderInfo> DAccountsUser::getReminderInfo() const
{
    Q_D(const DAccountsUser);
    ReminderInfo info;
    auto reply = d->m_dSystemUserInter->getReminderInfo();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    const auto &info_p = reply.value();

    info.userName = info_p.userName.toUtf8();

    info.failCountSinceLastLogin = info_p.failCountSinceLastLogin;

    info.spent.lastChange = info_p.spent.lastChange;
    info.spent.expired = info_p.spent.expired;
    info.spent.inactive = info_p.spent.inactive;
    info.spent.max = info_p.spent.max;
    info.spent.min = info_p.spent.min;
    info.spent.warn = info_p.spent.warn;

    info.currentLogin.address = info_p.currentLogin.address.toUtf8();
    info.currentLogin.host = info_p.currentLogin.host.toUtf8();
    info.currentLogin.inittabID = info_p.currentLogin.inittabID.toUtf8();
    info.currentLogin.line = info_p.currentLogin.line.toUtf8();
    info.currentLogin.time = info_p.currentLogin.time.toUtf8();

    info.lastLogin.address = info_p.lastLogin.address.toUtf8();
    info.lastLogin.host = info_p.lastLogin.host.toUtf8();
    info.lastLogin.inittabID = info_p.lastLogin.inittabID.toUtf8();
    info.lastLogin.line = info_p.lastLogin.line.toUtf8();
    info.lastLogin.time = info_p.lastLogin.time.toUtf8();

    return info;
}

DExpected<PasswdExpirInfo> DAccountsUser::passwordExpirationInfo(qint64 &dayLeft) const
{
    Q_D(const DAccountsUser);
    PasswdExpirInfo info;
    auto reply = d->m_dSystemUserInter->passwordExpiredInfo();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }

    const auto &expireStatus = reply.argumentAt(0);
    if (!expireStatus.isValid()) {
        qWarning() << "can't get passwordExpirationInfo: expiredStatus is invalid";
        info = PasswdExpirInfo::Unknown;
    } else {
        switch (expireStatus.toInt()) {
        case 0:
            info = PasswdExpirInfo::Normal;
            break;
        case 1:
            info = PasswdExpirInfo::Closed;
            break;
        case 2:
            info = PasswdExpirInfo::Expired;
            break;
        default:
            info = PasswdExpirInfo::Unknown;
            break;
        }
    }
    const auto &day = reply.argumentAt(1);
    if (!day.isValid()) {
        qWarning() << "can't get passwordExpirationInfo: dayLeft is invalid";
        return info;
    } else {
        dayLeft = day.toLongLong();
    }
    return info;
}

QList<QByteArray> DAccountsUserPrivate::getImageFromDir(const QDir &dir) const
{
    QList<QByteArray> icons;
    auto list = dir.entryInfoList();
    if (list.empty()) {
        return icons;
    }
    QMimeDatabase db;
    auto fileter = QRegularExpression("[\u4e00-\u9fa5]");
    for (const auto &v : list) {
        QMimeType type = db.mimeTypeForFile(v);
        if (!type.name().startsWith("image"))
            continue;
        auto filename = v.fileName();
        if (filename.contains(fileter))
            continue;
        icons.push_back(("file://" + v.absoluteFilePath()).toUtf8());
    }
    return icons;
}

DACCOUNTS_END_NAMESPACE
