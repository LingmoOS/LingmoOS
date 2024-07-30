/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich <ettrich@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kconfiggui.h"
#include "kconfig_gui_log_settings.h"

#include <QGuiApplication>

#include <kconfig.h>

static QString configName(const QString &id, const QString &key)
{
    return QLatin1String("session/%1_%2_%3").arg(QGuiApplication::applicationName(), id, key);
}

static KConfig *s_sessionConfig = nullptr;

KConfig *KConfigGui::sessionConfig()
{
#ifdef QT_NO_SESSIONMANAGER
    qCWarning(KCONFIG_GUI_LOG) << "Qt is built without session manager support";
#else
    if (!hasSessionConfig() && qApp->isSessionRestored()) {
        // create the default instance specific config object
        // from applications' -session command line parameter
        s_sessionConfig = new KConfig(configName(qApp->sessionId(), qApp->sessionKey()), KConfig::SimpleConfig);
    }
#endif

    return s_sessionConfig;
}

void KConfigGui::setSessionConfig(const QString &id, const QString &key)
{
    if (hasSessionConfig()) {
        delete s_sessionConfig;
        s_sessionConfig = nullptr;
    }

    // create a new instance specific config object from supplied id & key
    s_sessionConfig = new KConfig(configName(id, key), KConfig::SimpleConfig);
}

bool KConfigGui::hasSessionConfig()
{
    return s_sessionConfig != nullptr;
}
