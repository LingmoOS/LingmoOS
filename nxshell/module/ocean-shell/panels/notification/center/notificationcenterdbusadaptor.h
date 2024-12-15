// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusVariant>
#include <QDBusAbstractAdaptor>

namespace notification {

class NotificationCenterProxy;
class NotificationCenterDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.ocean.Widgets1")

public:
    explicit NotificationCenterDBusAdaptor(QObject *parent = nullptr);

public Q_SLOTS: // methods
    void Toggle();
    void Show();
    void Hide();

private:
    NotificationCenterProxy *impl() const;
};

} // notification
