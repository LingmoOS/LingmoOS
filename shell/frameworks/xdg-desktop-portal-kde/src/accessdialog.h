/*
 * SPDX-FileCopyrightText: 2017 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_ACCESS_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_ACCESS_DIALOG_H

#include "quickdialog.h"
#include <QVariantMap>

class AccessDialog : public QuickDialog
{
    Q_OBJECT
public:
    explicit AccessDialog(QObject *parent = nullptr);

    void setAcceptLabel(const QString &label);
    void setBody(const QString &body);
    void setIcon(const QString &icon);
    void setRejectLabel(const QString &label);
    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);

    void createDialog();

private:
    QVariantMap m_props;
};

#endif // XDG_DESKTOP_PORTAL_KDE_ACCESS_DIALOG_H
