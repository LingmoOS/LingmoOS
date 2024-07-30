/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_USERINFO_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_USERINFO_DIALOG_H

#include "quickdialog.h"

namespace Ui
{
class UserInfoDialog;
}

class OrgFreedesktopAccountsUserInterface;

class UserInfoDialog : public QuickDialog
{
    Q_OBJECT
public:
    explicit UserInfoDialog(const QString &reason, QObject *parent = nullptr);
    ~UserInfoDialog() override;

    QString id() const;
    QString name() const;
    QString image() const;

private:
    OrgFreedesktopAccountsUserInterface *m_userInterface;
};

#endif // XDG_DESKTOP_PORTAL_KDE_USERINFO_DIALOG_H
