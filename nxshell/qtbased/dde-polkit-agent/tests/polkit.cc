// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QString>
#include <polkit-qt5-1/PolkitQt1/Agent/Listener>
#include <polkit-qt5-1/PolkitQt1/Agent/Session>

int main(int argc, char* argv[])
{
    PolkitQt1::Identity            identity;
    const QString                  cookie;
    PolkitQt1::Agent::AsyncResult* result  = nullptr;
    PolkitQt1::Details*            details = nullptr;
    auto session = new PolkitQt1::Agent::Session(identity, cookie, result, details);

    session->authCtrl(AUTH_START, -1);
    session->setResponseEx(0, cookie);

    return 0;
}
