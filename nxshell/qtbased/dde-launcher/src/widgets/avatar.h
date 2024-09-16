// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVATAR_H
#define AVATAR_H

#include <QWidget>

#include "accounts_interface.h"
#include "accountsuser_interface.h"

using AccountsInter = org::deepin::dde::Accounts1;
using UserInter = org::deepin::dde::accounts1::User;

class Avatar : public QWidget
{
    Q_OBJECT

public:
    explicit Avatar(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    void setFilePath(const QString &filePath);

private:
    AccountsInter *m_accountsInter;
    UserInter *m_userInter;
    QPixmap m_avatarPixmap;
};

#endif // AVATAR_H
