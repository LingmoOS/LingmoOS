/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */
#include "screen-area-utils.h"

#include<QVariant>
#include <QGSettings>
#include <mutex>

#define LINGMO_PANEL_SETTING          "org.lingmo.panel.settings"
#define LINGMO_PANEL_POSITION_KEY     "panelposition"
#define LINGMO_PANEL_SIZE_KEY         "panelsize"
#define LINGMO_PANEL_AUtOHIDE_KEY     "hidepanel"

namespace LingmoUIQuick {

class ScreenAreaUtilsPrivate
{
public:
    QGSettings *m_setting {nullptr};
    int m_panelPos {4};
    int m_panelSize {48};
    bool m_panelAutoHide {false};
};

static ScreenAreaUtils *g_screenAreaUtils = nullptr;
static std::once_flag onceFlag;

ScreenAreaUtils *ScreenAreaUtils::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_screenAreaUtils = new ScreenAreaUtils();
    });
    return g_screenAreaUtils;
}

ScreenAreaUtils::~ScreenAreaUtils()
{
    if(d) {
        delete d;
        d = nullptr;
    }
    g_screenAreaUtils = nullptr;
}

QRect ScreenAreaUtils::getAvailableGeometry(QScreen *screen)
{
    QRect availableRect = screen->geometry();
    int adjustment = d->m_panelAutoHide ? 0 : d->m_panelSize;
    //上: 1, 下: 0, 左: 2, 右: 3
    switch (d->m_panelPos) {
        default:
        case 0: {
            availableRect.adjust(0, 0, 0, - adjustment);
            break;
        }
        case 1: {
            availableRect.adjust(0, adjustment, 0, 0);
            break;
        }
        case 2: {
            availableRect.adjust(adjustment, 0, 0, 0);
            break;
        }
        case 3: {
            availableRect.adjust(0, 0, - adjustment, 0);
            break;
        }
    }
    return availableRect;
}

ScreenAreaUtils::ScreenAreaUtils(QObject *parent) : QObject(parent), d(new ScreenAreaUtilsPrivate)
{
    initPanelSetting();
}

void ScreenAreaUtils::initPanelSetting()
{
    const QByteArray id(LINGMO_PANEL_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        d->m_setting = new QGSettings(id, QByteArray(), this);

        QStringList keys = d->m_setting->keys();
        if (keys.contains(LINGMO_PANEL_POSITION_KEY)) {
            d->m_panelPos = d->m_setting->get(LINGMO_PANEL_POSITION_KEY).toInt();
        }

        if (keys.contains(LINGMO_PANEL_SIZE_KEY)) {
            d->m_panelSize = d->m_setting->get(LINGMO_PANEL_SIZE_KEY).toInt();
        }

        if (keys.contains(LINGMO_PANEL_AUtOHIDE_KEY)) {
            d->m_panelAutoHide = d->m_setting->get(LINGMO_PANEL_AUtOHIDE_KEY).toBool();
        }

        connect(d->m_setting, &QGSettings::changed, this, [this] (const QString& key) {
            if (key == LINGMO_PANEL_POSITION_KEY) {
                d->m_panelPos = d->m_setting->get(LINGMO_PANEL_POSITION_KEY).toInt();
                Q_EMIT availableGeometryChanged();
            } else if (key == LINGMO_PANEL_SIZE_KEY) {
                d->m_panelSize = d->m_setting->get(key).toInt();
                Q_EMIT availableGeometryChanged();
            } else if (key == LINGMO_PANEL_AUtOHIDE_KEY) {
                d->m_panelAutoHide = d->m_setting->get(key).toInt();
                Q_EMIT availableGeometryChanged();
            }
        });
    }
}
}
