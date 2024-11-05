/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "navigation-bar.h"

#include "navigation-tool-bar.h"
#include "advanced-location-bar.h"
#include "directory-view-container.h"

#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"

#include <QPainter>
#include <QColor>

#include <QDebug>

using namespace Peony;

NavigationBar::NavigationBar(QWidget *parent) : QToolBar(parent)
{
    setContentsMargins(0, 0, 5, 0);
    setFixedHeight(38);
    setMovable(false);
    setFloatable(false);
    m_left_control = new NavigationToolBar(this);
    m_left_control->setFixedWidth(m_left_control->sizeHint().width());

    m_left_control->setContentsMargins(0, 0, 0, 0);
    addWidget(m_left_control);
    addSeparator();

    m_center_control = new AdvancedLocationBar(this);
    m_center_control->setContentsMargins(0, 0, 0, 0);
    addWidget(m_center_control);
    addSeparator();

    connect(m_left_control, &NavigationToolBar::updateWindowLocationRequest,
            this, &NavigationBar::updateWindowLocationRequest);
    connect(m_left_control, &NavigationToolBar::refreshRequest,
            this, &NavigationBar::refreshRequest);
    connect(m_center_control, &AdvancedLocationBar::updateWindowLocationRequest,
            this, &NavigationBar::updateWindowLocationRequest);
    connect(m_center_control, &AdvancedLocationBar::refreshRequest,
            this, &NavigationBar::refreshRequest);

    auto manager = PreviewPageFactoryManager::getInstance();
    auto ids = manager->getPluginNames();
    QActionGroup *group = new QActionGroup(this);
    m_group = group;
    group->setExclusive(true);
    for (auto id : ids) {
        auto factory = manager->getPlugin(id);
        auto action = group->addAction(factory->icon(), factory->name());
        action->setCheckable(true);
        connect(action, &QAction::triggered, [=]() {
            if (m_checked_preview_action == action) {
                action->setChecked(false);
                m_checked_preview_action = nullptr;
            } else {
                m_checked_preview_action = action;
                action->setChecked(true);
            }
            m_last_preview_page_id_in_window = id;
            Q_EMIT this->switchPreviewPageRequest(m_checked_preview_action? m_checked_preview_action->text(): nullptr);
        });
    }
    addActions(group->actions());
}

NavigationBar::~NavigationBar()
{

}

void NavigationBar::bindContainer(DirectoryViewContainer *container)
{
    m_left_control->setCurrentContainer(container);
    m_left_control->updateActions();
    updateLocation(container->getCurrentUri());
}

void NavigationBar::updateLocation(const QString &uri)
{
    m_center_control->updateLocation(uri);
    m_left_control->updateActions();
}

void NavigationBar::setBlock(bool block)
{
    qDebug()<<"block"<<block;
    this->blockSignals(block);
    m_left_control->blockSignals(block);
    m_center_control->blockSignals(block);
    m_left_control->setDisabled(block);
    m_center_control->setDisabled(block);
}

void NavigationBar::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    //auto color = m_styled_menu->palette().window().color();
    auto color = this->palette().base().color();
    color.setAlpha(127);
    p.fillRect(this->rect().adjusted(-1, -1, 1, 1), color);
    QToolBar::paintEvent(e);
}

bool NavigationBar::isPathEditing()
{
    return m_center_control->isEditing();
}

const QString NavigationBar::getLastPreviewPageId()
{
    if (m_last_preview_page_id_in_window.isNull()) {
        return PreviewPageFactoryManager::getInstance()->getLastPreviewPageId();
    }
    return m_last_preview_page_id_in_window;
}

void NavigationBar::startEdit()
{
    m_center_control->startEdit();
}

void NavigationBar::finishEdit()
{
    m_center_control->finishEdit();
}

void NavigationBar::triggerAction(const QString &id) {
    for (auto action : m_group->actions()) {
        if (action->text() == id) {
            action->trigger();
            return;
        }
    }
}
