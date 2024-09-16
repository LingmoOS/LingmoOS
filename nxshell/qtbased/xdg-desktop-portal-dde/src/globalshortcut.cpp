// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "globalshortcut.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopDDEGlobalShortCut, "xdg-dde-global-shortcut")

GlobalShortcutPortal::GlobalShortcutPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(XdgDesktopDDEGlobalShortCut) << "init global shortcut";
}

uint GlobalShortcutPortal::CreateSession(const QDBusObjectPath &handle,
                                        const QDBusObjectPath &session_handle,
                                        const QString &app_id,
                                        const QVariantMap &options,
                                        QVariantMap &results)
{
    qCDebug(XdgDesktopDDEGlobalShortCut) << "create session";
    return 1;
}

QVariantMap GlobalShortcutPortal::BindShortCuts(const QDBusObjectPath &handle,
                                               const QDBusObjectPath &session_handle,
                                               const QVariantMap &shortcuts,
                                               const QString &parent_window,
                                               const QVariantMap &options)
{
    qCDebug(XdgDesktopDDEGlobalShortCut) << "BindShortCuts";
    return QVariantMap();
}

QVariantMap GlobalShortcutPortal::ListShortCuts(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle)
{
    qCDebug(XdgDesktopDDEGlobalShortCut) << "get ShortCuts";
    return QVariantMap();
}
