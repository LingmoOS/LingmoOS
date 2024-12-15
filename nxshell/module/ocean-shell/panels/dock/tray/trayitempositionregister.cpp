// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trayitempositionregister.h"

#include "trayitempositionmanager.h"

namespace docktray {

TrayItemPositionRegisterAttachedType::TrayItemPositionRegisterAttachedType(QObject *parent)
    : QObject(parent)
{
    // register / update visual position
    connect(this, &TrayItemPositionRegisterAttachedType::visualIndexChanged,
            this, [this](){
                registerVisualSize();
                emit visualPositionChanged();
            });
    connect(this, &TrayItemPositionRegisterAttachedType::visualSizeChanged,
            this, [this](){
                registerVisualSize();
                emit visualPositionChanged();
            });
    // recalculate position
    connect(&TrayItemPositionManager::instance(), &TrayItemPositionManager::orientationChanged,
            this, &TrayItemPositionRegisterAttachedType::visualPositionChanged);
    connect(&TrayItemPositionManager::instance(), &TrayItemPositionManager::dockHeightChanged,
            this, &TrayItemPositionRegisterAttachedType::visualPositionChanged);
    connect(&TrayItemPositionManager::instance(), &TrayItemPositionManager::visualItemCountChanged,
            this, &TrayItemPositionRegisterAttachedType::visualPositionChanged);
    connect(&TrayItemPositionManager::instance(), &TrayItemPositionManager::visualItemSizeChanged,
            this, &TrayItemPositionRegisterAttachedType::visualPositionChanged);
}

QPoint TrayItemPositionRegisterAttachedType::visualPosition() const
{
    TrayItemPositionManager & pm = TrayItemPositionManager::instance();
    if (pm.orientation() == Qt::Horizontal) {
        int width = m_visualIndex == 0 ? 0 : pm.visualSize(m_visualIndex - 1).width();
        return QPoint(width, (pm.dockHeight() - m_visualSize.height()) / 2);
    } else {
        int height = m_visualIndex == 0 ? 0 : pm.visualSize(m_visualIndex - 1).height();
        return QPoint((pm.dockHeight() - m_visualSize.width()) / 2, height);
    }
}

void TrayItemPositionRegisterAttachedType::registerVisualSize()
{
    if (m_visualIndex == -1 || m_visualSize.isEmpty()) return;
    TrayItemPositionManager::instance().registerVisualItemSize(m_visualIndex, m_visualSize);
}

}
