// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifysetting.h"

#include <DConfig>

namespace notification {

NotifySetting * NotifySetting::instance()
{
    static NotifySetting instance;
    return &instance;
}

int NotifySetting::bubbleCount() const
{
    return m_bubbleCount;
}

int NotifySetting::contentRowCount() const
{
    return m_contentRowCount;
}

NotifySetting::NotifySetting(QObject *parent)
    : QObject(parent)
{
    m_setting = Dtk::Core::DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ocean.shell.notification");
    connect(m_setting, &Dtk::Core::DConfig::valueChanged, this, [this](const QString &key) {
        if (key == "bubbleCount") {
            updateMaxBubbleCount();
        } else if (key == "bubbleContentRowCount") {
            updateBubbleContentRowCount();
        }
    });

    updateMaxBubbleCount();
    updateBubbleContentRowCount();
}

NotifySetting::~NotifySetting()
{
    if (m_setting)
        m_setting->deleteLater();
}

void NotifySetting::updateMaxBubbleCount()
{
    auto count = m_setting->value("bubbleCount").toInt();
    if (count < 1) {
        qWarning() << "Invalid settings of bubbleCount:" << count << ", it should be greater than 0";
        return;
    }

    if (m_bubbleCount != count) {
        m_bubbleCount = count;
        Q_EMIT bubbleCountChanged(count);
    }
}

void NotifySetting::updateBubbleContentRowCount()
{
    auto count = m_setting->value("bubbleContentRowCount").toInt();
    if (count < 1) {
        qWarning() << "Invalid settings of bubbleContentRowCount:" << count << ", it should be greater than 0";
        return;
    }

    if (m_contentRowCount != count) {
        m_contentRowCount = count;
        Q_EMIT contentRowCountChanged(count);
    }
}
}
