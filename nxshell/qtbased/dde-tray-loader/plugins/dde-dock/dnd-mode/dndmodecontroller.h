// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNDMODECONTROLLER_H
#define DNDMODECONTROLLER_H

#include <QObject>
#include <QGSettings>
#include <QVariant>

#include <DSingleton>

#include <com_deepin_dde_notification.h>

#define KEY_DNDMODE         0

using NotifyInter = com::deepin::dde::Notification;

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
        : m_gsetting(nullptr)
        , m_notifyInter(new NotifyInter("com.deepin.dde.Notification", "/com/deepin/dde/Notification", QDBusConnection::sessionBus(), this))
        , m_dndModeEnabled(false)
    {
        if (!QGSettings::isSchemaInstalled("com.deepin.dde.notifications")) {
            return;
        }
        m_gsetting = new QGSettings("com.deepin.dde.notifications", QByteArray(), this);

        if (!m_gsetting) {
            return;
        }

        // 使用NotifyInter获取水dnd mode的值是异步方式,用gsetting获取和监听比较方便
        m_dndModeEnabled = m_gsetting->keys().contains("dndmode", Qt::CaseSensitivity::CaseInsensitive) ? m_gsetting->get("dndmode").toBool() : false;

        connect(m_gsetting, &QGSettings::changed, this, [this] (const QString &key) {
            if (key == "dndmode") {
                m_dndModeEnabled =  m_gsetting->get("dndmode").toBool();
                Q_EMIT dndModeChanged(m_dndModeEnabled);
            }
        });
    }

    ~DndModeController() {}

private:
    QGSettings *m_gsetting;
    NotifyInter *m_notifyInter;
    bool m_dndModeEnabled;
};

#endif
