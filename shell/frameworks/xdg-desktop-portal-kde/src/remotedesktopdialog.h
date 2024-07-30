/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_DIALOG_H

#include "outputsmodel.h"
#include "quickdialog.h"
#include "remotedesktop.h"
#include "screencast.h"

namespace Ui
{
class RemoteDesktopDialog;
}

class RemoteDesktopDialog : public QuickDialog
{
    Q_OBJECT
public:
    RemoteDesktopDialog(const QString &appName, RemoteDesktopPortal::DeviceTypes deviceTypes, bool screenSharingEnabled, ScreenCastPortal::PersistMode persistMode, QObject *parent = nullptr);

    QList<Output> selectedOutputs() const;
    bool allowRestore() const;

    static QString buildDescription(const QString &appName, RemoteDesktopPortal::DeviceTypes deviceTypes, bool screenSharingEnabled);
};

#endif // XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_DIALOG_H
