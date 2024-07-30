/*
 * SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_NOTIFICATIONINHIBITION_H
#define XDG_DESKTOP_PORTAL_KDE_NOTIFICATIONINHIBITION_H

#include <QObject>

class NotificationInhibition : public QObject
{
    Q_OBJECT
public:
    explicit NotificationInhibition(const QString &appId, const QString &reason, QObject *parent = nullptr);
    ~NotificationInhibition() override;

private:
    static void uninhibit(uint cookie);
    uint m_cookie = 0;
};

#endif // XDG_DESKTOP_PORTAL_KDE_NOTIFICATIONINHIBITION_H
