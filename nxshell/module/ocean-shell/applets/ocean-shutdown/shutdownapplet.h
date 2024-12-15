// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

DS_BEGIN_NAMESPACE
namespace shutdown {
class TreeLandLockScreen;
class ShutdownApplet : public DApplet
{
    Q_OBJECT
public:
    explicit ShutdownApplet(QObject *parent = nullptr);
    ~ShutdownApplet();

    virtual bool load() override;

public Q_SLOTS:
    bool requestShutdown();
    bool requestLock();
    bool requestSwitchUser();

private:
    QScopedPointer<TreeLandLockScreen> m_lockscreen;
};

}
DS_END_NAMESPACE
