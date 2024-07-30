/*
 * SPDX-FileCopyrightText: 2016-2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016-2018 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_H
#define XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class QDBusMessage;
class AppChooserDialog;

class AppChooserPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.AppChooser")
public:
    explicit AppChooserPortal(QObject *parent);

public Q_SLOTS:
    uint ChooseApplication(const QDBusObjectPath &handle,
                           const QString &app_id,
                           const QString &parent_window,
                           const QStringList &choices,
                           const QVariantMap &options,
                           QVariantMap &results);

    uint
    ChooseApplicationPrivate(const QString &parent_window, const QStringList &urls, const QVariantMap &options, const QDBusMessage &msg, QVariantMap &results);
    void UpdateChoices(const QDBusObjectPath &handle, const QStringList &choices);

private:
    QMap<QString, AppChooserDialog *> m_appChooserDialogs;
};

#endif // XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_H
