/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BACKENDS_MANAGER_H
#define BACKENDS_MANAGER_H

#include "AuthBackend.h"
#include "HelperProxy.h"

namespace KAuth
{
class BackendsManager
{
    static AuthBackend *auth;
    static HelperProxy *helper;

    BackendsManager();

public:
    static AuthBackend *authBackend();
    static HelperProxy *helperProxy();
    static void setProxyForThread(QThread *thread, HelperProxy *proxy);

private:
    static void init();
};

} // namespace Auth

#endif
