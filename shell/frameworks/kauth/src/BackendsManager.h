/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KAUTH_BACKENDS_MANAGER_H
#define KAUTH_BACKENDS_MANAGER_H

#include "AuthBackend.h"
#include "HelperProxy.h"
#include "kauthcore_export.h"

namespace KAuth
{
class KAUTHCORE_EXPORT BackendsManager
{
private:
    static AuthBackend *auth;
    static HelperProxy *helper;

    KAUTHCORE_NO_EXPORT BackendsManager();

public:
    static AuthBackend *authBackend();
    static HelperProxy *helperProxy();

private:
    KAUTHCORE_NO_EXPORT static void init();
    KAUTHCORE_NO_EXPORT static QList<QObject *> retrieveInstancesIn(const QString &path);
};

} // namespace Auth

#endif
