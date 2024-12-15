// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>

namespace accountnetwork {
namespace sessionservice {

class Account : public QObject
{
    Q_OBJECT

public:
    Account(QObject *parent);
    ~Account();
    QString name() const;
    QString activeAccount() const;
    bool isIam();

private:
    void initAccount();
    void initActiveAccount();
    void loadActiveAccount();
    QString parseAccount(const QString &accountInfo);

private slots:
    void onAccountChanged(const QString &username);

private:
    QString m_accountName;
    bool m_isIam;
    QString m_activeAccountName;
};

}
}
#endif
