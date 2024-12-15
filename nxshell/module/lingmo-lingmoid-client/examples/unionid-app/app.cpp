// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "app.h"

#include <QDebug>
#include <QDBusInterface>

App::App(QObject *parent) : QObject(parent)
{

}

void App::TestStartAuthorize()
{
    QList<QVariant> argumentList;

    static QDBusInterface interface("com.lingmo.lingmoid.Client",
                                        "/com/lingmo/lingmoid/Client",
                                        "com.lingmo.lingmoid.Client");
    // TODO: replace with you own id
    argumentList << "client_id";
    argumentList << "com.lingmo.lingmoid.App";
    argumentList << "/com/lingmo/lingmoid/App";
    argumentList << "com.lingmo.lingmoid.App";
    interface.callWithArgumentList(QDBus::NoBlock, "Register", argumentList);

    argumentList = {};
     // TODO: replace with you own id
    argumentList << "client_id";
    argumentList << QStringList{};
     // TODO: replace with you own redirect uri
    argumentList << "http://test.com/oauth2";
    // state must gen by server and verify on receive code on server
    argumentList << "random_state";
    interface.callWithArgumentList(QDBus::NoBlock, "Authorize", argumentList);
}

void App::OnAuthorized(const QString &code,
                       const QString &state)
{
    // use code and state to finish oauth2 authorize flow
    qDebug() << "OnAuthorized" << code << state;
}

void App::OnCancel()
{
    // authorize canceled
    qDebug() << "OnCancel";
}
