// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>

class SessionBaseModel;
class SessionManager : public QObject
{
    Q_OBJECT

public:
    static SessionManager &Reference();

public:
    void setModel(SessionBaseModel * const model);
    int sessionCount() const;
    QString currentSession() const;
    QMap<QString, QString> sessionInfo() const;
    void updateSession(const QString &userName);
    void switchSession(const QString &session);

private:
    QString getSessionKey(const QString &sessionName) const;
    QString lastLoggedInSession(const QString &userName) const;
    bool isWaylandExisted() const;
    QString defaultConfigSession() const;

private:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;

private:
    SessionBaseModel *m_model;
    QLightDM::SessionsModel *m_sessionModel;
    QLightDM::UsersModel *m_userModel;
};

#endif // SESSIONWIDGET_H
