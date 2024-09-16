// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../constants.h"
#include "traysettings.h"

namespace dock {

TraySettings* TraySettings::instance()
{
    static TraySettings* dockSettings = nullptr;
    if (!dockSettings) {
        dockSettings = new TraySettings();
    }

    return dockSettings;
}

TraySettings::TraySettings(QObject* parent)
    : QObject(parent)
    , m_trayConfig(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.dock.tray", QString(), this))
{
    init();
}

void TraySettings::init()
{
    if (m_trayConfig && m_trayConfig->isValid()) {
        m_hiddenSurfaceIds = m_trayConfig->value(KeyHiddenSurfaceIds).toStringList();
        connect(m_trayConfig.data(), &DConfig::valueChanged, this, [this](const QString& key){
            if (KeyHiddenSurfaceIds == key) {
                auto hiddenSurfaceIds = m_trayConfig->value(KeyHiddenSurfaceIds).toStringList();
                if (hiddenSurfaceIds == m_hiddenSurfaceIds) return;
                m_hiddenSurfaceIds = hiddenSurfaceIds;
                Q_EMIT hiddenSurfaceIdsChanged(m_hiddenSurfaceIds);
            }
            });
    } else {
        qWarning() << "unable to create config for org.deepin.dde.dock.tray";
    }

}

bool TraySettings::trayItemIsOnDock(const QString &surfaceIds)
{
    return !m_hiddenSurfaceIds.contains(surfaceIds);
}

void TraySettings::addTrayItemOnDock(const QString &surfaceIds)
{
    if (!m_trayConfig)
        return;
    if (m_hiddenSurfaceIds.contains(surfaceIds)) {
        m_hiddenSurfaceIds.removeOne(surfaceIds);
        m_trayConfig->setValue(KeyHiddenSurfaceIds, m_hiddenSurfaceIds);
    }
}

void TraySettings::removeTrayItemOnDock(const QString &surfaceIds)
{
    if (!m_trayConfig)
        return;
    if (!m_hiddenSurfaceIds.contains(surfaceIds)) {
        m_hiddenSurfaceIds << surfaceIds;
        m_trayConfig->setValue(KeyHiddenSurfaceIds, m_hiddenSurfaceIds);
    }
}

}
