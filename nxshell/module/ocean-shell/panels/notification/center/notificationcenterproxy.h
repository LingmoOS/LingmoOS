// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

namespace notification {
class NotificationCenterPanel;
class NotificationCenterProxy : public QObject
{
    Q_OBJECT
public:
    explicit NotificationCenterProxy(QObject *parent = nullptr);
    ~NotificationCenterProxy();

public slots:
    void Toggle();
    void Show();
    void Hide();

private:
    NotificationCenterPanel *panel() const;
};
}
