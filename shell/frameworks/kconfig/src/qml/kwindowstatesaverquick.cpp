// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kwindowstatesaverquick.h"
#include "kconfig_qml_log_settings.h"

#include <QQuickItem>
#include <QQuickWindow>

#include <KWindowStateSaver>

void KWindowStateSaverQuick::classBegin()
{
}

void KWindowStateSaverQuick::componentComplete()
{
    const auto parentItem = qobject_cast<QQuickItem *>(parent());
    if (!parentItem) {
        qCWarning(KCONFIG_QML_LOG) << "WindowStateSaver requires a parent item";
        return;
    }

    const auto window = qobject_cast<QWindow *>(parentItem->window());
    if (!window) {
        qCWarning(KCONFIG_QML_LOG) << "WindowStateSaver requires the parent to be a type that inherits QWindow";
        return;
    }

    new KWindowStateSaver(window, m_configGroupName);
}

void KWindowStateSaverQuick::setConfigGroupName(const QString &name)
{
    if (m_configGroupName != name) {
        m_configGroupName = name;
        Q_EMIT configGroupNameChanged();
    }
}

QString KWindowStateSaverQuick::configGroupName() const
{
    return m_configGroupName;
}

#include "moc_kwindowstatesaverquick.cpp"