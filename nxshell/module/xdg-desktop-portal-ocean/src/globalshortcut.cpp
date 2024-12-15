// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "globalshortcut.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopOCEANGlobalShortCut, "xdg-ocean-global-shortcut")

GlobalShortcutPortal::GlobalShortcutPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDesktopOCEANGlobalShortCut) << "init global shortcut";
}

uint GlobalShortcutPortal::CreateSession(const QDBusObjectPath &handle,
                                        const QDBusObjectPath &session_handle,
                                        const QString &app_id,
                                        const QVariantMap &options,
                                        QVariantMap &results)
{
    qCDebug(XdgDesktopOCEANGlobalShortCut) << "create session";
    return 1;
}

QVariantMap GlobalShortcutPortal::BindShortCuts(const QDBusObjectPath &handle,
                                               const QDBusObjectPath &session_handle,
                                               const QVariantMap &shortcuts,
                                               const QString &parent_window,
                                               const QVariantMap &options)
{
    qCDebug(XdgDesktopOCEANGlobalShortCut) << "BindShortCuts";
    return QVariantMap();
}

QVariantMap GlobalShortcutPortal::ListShortCuts(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle)
{
    qCDebug(XdgDesktopOCEANGlobalShortCut) << "get ShortCuts";
    return QVariantMap();
}
