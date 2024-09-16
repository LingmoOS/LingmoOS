// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "demo.h"

#include <QDebug>

AccountsDemo::AccountsDemo()
{
    auto ret = manager.findUserByName("test");
    if (ret) {
        user = ret.value();
    } else {
        qDebug() << ret.error();
    }

    connect(&manager, &DAccountsManager::UserAdded, this, [](const quint64 uid) {
        qDebug() << "new user add:" << uid;
    });
    connect(&manager, &DAccountsManager::UserDeleted, this, [](const quint64 uid) {
        qDebug() << "delete user:" << uid;
    });
    connect(user.data(), &DAccountsUser::automaticLoginChanged, this, [](const bool) {
        qDebug() << "automaticLoginChanged";
    });
    connect(user.data(), &DAccountsUser::noPasswdLoginChanged, this, [](const bool) {
        qDebug() << "noPasswdLoginChanged";
    });
    connect(user.data(), &DAccountsUser::groupsChanged, this, [](const QStringList &) {
        qDebug() << "groupsChanged";
    });
    connect(user.data(), &DAccountsUser::iconFileListChanged, this, [](const QList<QByteArray> &) {
        qDebug() << "iconFileListChanged";
    });
    connect(user.data(), &DAccountsUser::iconFileChanged, this, [](const QUrl &) {
        qDebug() << "iconFileChanged";
    });
    connect(user.data(), &DAccountsUser::layoutListChanged, this, [](const QList<QByteArray> &) {
        qDebug() << "layoutListChanged";
    });
    connect(user.data(), &DAccountsUser::layoutChanged, this, [](const QByteArray &) {
        qDebug() << "layoutChanged";
    });
    connect(user.data(), &DAccountsUser::maxPasswordAgeChanged, this, [](const qint32) {
        qDebug() << "maxPasswordAgeChanged";
    });
    connect(user.data(), &DAccountsUser::passwordHintChanged, this, [](const QString &) {
        qDebug() << "passwordHintChanged";
    });
    connect(user.data(), &DAccountsUser::localeChanged, this, [](const QByteArray &) {
        qDebug() << "localeChanged";
    });
    connect(user.data(), &DAccountsUser::lockedChanged, this, [](const bool) {
        qDebug() << "lockedChanged";
    });
}

void AccountsDemo::run()
{
    manager.isUsernameValid("qwer");

    auto ret = manager.createUser("qwer", "testqwer", AccountTypes::Default);
    if (ret) {
        auto newuser = ret.value();
    } else {
        qDebug() << ret.error();
    }

    manager.deleteUser("qwer", true);
    auto groups = manager.groups();
    if (groups)
        qDebug() << groups.value();
    else
        qDebug() << groups.error();

    qDebug() << user->UID();
    qDebug() << user->UUID();
    user->setIconFile(QUrl("file:///var/lib/AccountsService/icons/13.png"));
    user->addGroup("libvirt");
    user->deleteGroup("libvirt");
    user->deleteIconFile(QUrl("file:///var/lib/AccountsService/icons/local/qwer-dfgdsd31dfs"));
    user->setLayout("cn;");
    user->setLayoutList({ "cn;", "jp;", "en;" });
}
