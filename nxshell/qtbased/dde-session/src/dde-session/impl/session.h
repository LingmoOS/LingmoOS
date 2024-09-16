// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSION_H
#define SESSION_H

#include <QObject>

class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session(QObject *parent = nullptr);

public Q_SLOTS:
    void Logout();
    uint GetSessionPid();
    QString GetSessionPath();

    void setSessionPid(uint pid);
    void setSessionPath();

private:
    uint m_sessionPid;
    QString m_selfSessionPath;
};

#endif // SESSION_H
