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
    , m_trayConfig(DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ds.dock.tray", QString(), this))
{
    init();
}

void TraySettings::init()
{
    if (m_trayConfig && m_trayConfig->isValid()) {
        m_hioceannSurfaceIds = m_trayConfig->value(KeyHiddenSurfaceIds).toStringList();
        connect(m_trayConfig.data(), &DConfig::valueChanged, this, [this](const QString& key){
            if (KeyHiddenSurfaceIds == key) {
                auto hioceannSurfaceIds = m_trayConfig->value(KeyHiddenSurfaceIds).toStringList();
                if (hioceannSurfaceIds == m_hioceannSurfaceIds) return;
                m_hioceannSurfaceIds = hioceannSurfaceIds;
                Q_EMIT hioceannSurfaceIdsChanged(m_hioceannSurfaceIds);
            }
            });
    } else {
        qWarning() << "unable to create config for org.lingmo.ocean.dock.tray";
    }

}

bool TraySettings::trayItemIsOnDock(const QString &surfaceIds)
{
    return !m_hioceannSurfaceIds.contains(surfaceIds);
}

void TraySettings::addTrayItemOnDock(const QString &surfaceIds)
{
    if (!m_trayConfig)
        return;
    if (m_hioceannSurfaceIds.contains(surfaceIds)) {
        m_hioceannSurfaceIds.removeOne(surfaceIds);
        m_trayConfig->setValue(KeyHiddenSurfaceIds, m_hioceannSurfaceIds);
    }
}

void TraySettings::removeTrayItemOnDock(const QString &surfaceIds)
{
    if (!m_trayConfig)
        return;
    if (!m_hioceannSurfaceIds.contains(surfaceIds)) {
        m_hioceannSurfaceIds << surfaceIds;
        m_trayConfig->setValue(KeyHiddenSurfaceIds, m_hioceannSurfaceIds);
    }
}

}
