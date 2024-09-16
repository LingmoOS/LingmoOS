// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmanager.h"
#include "sessionbasemodel.h"

#include <QDebug>
#include <DMessageBox>
#include <DSysInfo>

DCORE_USE_NAMESPACE

static const QString DEFAULT_SESSION_NAME = "dde-x11";
static const QString WAYLAND_SESSION_NAME = "dde-wayland";
static const QString DISPLAY_X11_NAME     = "X11";
static const QString DISPLAY_WAYLAND_NAME = "Wayland (Technology Preview)";


const QString displaySessionName(const QString &realName)
{
    if (!DSysInfo::isDeepin()) {
        return realName;
    }

    if (realName == DEFAULT_SESSION_NAME) {
        return DISPLAY_X11_NAME;
    } else if (realName == WAYLAND_SESSION_NAME) {
        return DISPLAY_WAYLAND_NAME;
    }

    return realName;
}

const QString sessionIconName(const QString &realName)
{
    const QStringList standardIconList = {"deepin", "fluxbox", "gnome",
                                          "plasma", "ubuntu", "xfce", "wayland"};
    for (const auto &name : standardIconList) {
        if (realName.contains(name, Qt::CaseInsensitive)) {
            if (realName == DEFAULT_SESSION_NAME) {
                return "x11";
            } else if (realName == WAYLAND_SESSION_NAME) {
	        return "wayland";
	    } else {
                return name;
            }
        }
    }

    return QStringLiteral("unknown");
}

const QString displayNameToSessionName(const QString &name)
{
    if (name == DISPLAY_X11_NAME) {
        return DEFAULT_SESSION_NAME;
    } else if (name == DISPLAY_WAYLAND_NAME) {
        return WAYLAND_SESSION_NAME;
    }

    return name;
}

SessionManager &SessionManager::Reference()
{
    static SessionManager sessionManager;
    return sessionManager;
}

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_sessionModel(new QLightDM::SessionsModel(this))
    , m_userModel(new QLightDM::UsersModel(this))
{
}

SessionManager::~SessionManager()
{

}

void SessionManager::setModel(SessionBaseModel * const model)
{
    m_model = model;
}

int SessionManager::sessionCount() const
{
    return m_sessionModel->rowCount(QModelIndex());
}

QString SessionManager::currentSession() const
{
    return displaySessionName(m_model->sessionKey());
}

QMap<QString, QString> SessionManager::sessionInfo() const
{
    // key:sessionName; value:icon
    QMap<QString, QString> infos;
    int count = sessionCount();
    for (int i = 0; i < count; ++i) {
        const QString &sessionName = m_sessionModel->data(m_sessionModel->index(i), QLightDM::SessionsModel::KeyRole).toString();
        const QString &displayName = displaySessionName(sessionName);
        const QString icon = QString(":/img/sessions_icon/%1_normal.svg").arg(sessionIconName(sessionName));
        infos[displayName] = icon;
    }

    return infos;
}

void SessionManager::updateSession(const QString &userName)
{
    QString defaultSession = defaultConfigSession();
    QString sessionName = lastLoggedInSession(userName);

    m_model->setSessionKey(getSessionKey(sessionName));
    qDebug() << userName << "default session is: " << sessionName;
}

void SessionManager::switchSession(const QString &sessionName)
{
    QString session = displayNameToSessionName(sessionName);
    m_model->setSessionKey(getSessionKey(session));
}

QString SessionManager::getSessionKey(const QString &sessionName) const
{
    const int count = m_sessionModel->rowCount(QModelIndex());
    Q_ASSERT(count);
    QString defaultSessionKey;
    for (int i = 0; i < count; ++i) {
        QString sessionKey = m_sessionModel->data(m_sessionModel->index(i), QLightDM::SessionsModel::KeyRole).toString();
        if (!sessionName.compare(sessionKey, Qt::CaseInsensitive)) {
            return sessionKey;
        }

        if (!DEFAULT_SESSION_NAME.compare(sessionKey, Qt::CaseInsensitive)) {
            defaultSessionKey = sessionKey;
        }
    }

    // NOTE: The current sessionName does not exist
    qWarning() << "The sessionName :%s does not exist, using the default value" << sessionName;
    return defaultSessionKey;
}

QString SessionManager::lastLoggedInSession(const QString &userName) const
{
    for (int i = 0; i < m_userModel->rowCount(QModelIndex()); ++i) {
        if (userName == m_userModel->data(m_userModel->index(i), QLightDM::UsersModel::NameRole).toString()) {
            QString session = m_userModel->data(m_userModel->index(i), QLightDM::UsersModel::SessionRole).toString();
            if (!session.isEmpty())
                return session;
        }
    }

    return defaultConfigSession();
}

QString SessionManager::defaultConfigSession() const
{
    return getDConfigValue(getDefaultConfigFileName(),"defaultSession", DEFAULT_SESSION_NAME).toString();
}
