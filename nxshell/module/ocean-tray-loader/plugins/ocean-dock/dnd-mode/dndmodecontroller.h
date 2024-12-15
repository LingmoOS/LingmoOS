// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNDMODECONTROLLER_H
#define DNDMODECONTROLLER_H

#include <QObject>
#include <QVariant>

#include <DSingleton>
#include <DConfig>

#include "notification1interface.h"

#define KEY_DNDMODE         0

using NotifyInter = org::lingmo::ocean::Notification1;

class DndModeController : public QObject, public Dtk::Core::DSingleton<DndModeController>
{
    friend Dtk::Core::DSingleton<DndModeController>;

    Q_OBJECT
public:
    void enable(bool enable)
    {
        m_notifyInter->SetSystemInfo(KEY_DNDMODE, QDBusVariant(enable));
    }

    bool isDndModeEnabled() const { return m_dndModeEnabled; }

    void toggle()
    {
        m_notifyInter->SetSystemInfo(KEY_DNDMODE, QDBusVariant(!isDndModeEnabled()));
    }

Q_SIGNALS:
    void dndModeChanged(bool enabled);

private:
    DndModeController()
        : m_dConfig(Dtk::Core::DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ocean.shell.notification", QString(), this))
        , m_notifyInter(new NotifyInter("org.lingmo.ocean.Notification1", "/org/lingmo/ocean/Notification1", QDBusConnection::sessionBus(), this))
        , m_dndModeEnabled(false)
    {
        if (!m_dConfig) {
            qWarning() << "DndModeController: dConfig is null";
            return;
        }

        // 使用NotifyInter获取水dnd mode的值是异步方式,用gsetting获取和监听比较方便
        m_dndModeEnabled = m_dConfig->value("dndMode", false).toBool();

        connect(m_dConfig, &Dtk::Core::DConfig::valueChanged, this, [this] (const QString &key) {
            if (key == "dndMode") {
                m_dndModeEnabled =  m_dConfig->value("dndMode").toBool();
                Q_EMIT dndModeChanged(m_dndModeEnabled);
            }
        });
    }

    ~DndModeController() {}

private:
    Dtk::Core::DConfig *m_dConfig = nullptr;
    NotifyInter *m_notifyInter;
    bool m_dndModeEnabled;
};

#endif
