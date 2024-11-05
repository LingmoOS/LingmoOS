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

#include "navigation-tool-bar.h"
#include "file-utils.h"

#include <QMenu>
#include <QToolButton>

using namespace Peony;

#include "directory-view-container.h"

NavigationToolBar::NavigationToolBar(QWidget *parent) : QToolBar(parent)
{
    m_back_action = addAction(QIcon::fromTheme("go-previous"), tr("Go Back"), [=]() {
        this->onGoBack();
    });

    m_forward_action = addAction(QIcon::fromTheme("go-next"), tr("Go Forward"), [=]() {
        this->onGoForward();
    });

    m_history_action = addAction(QIcon::fromTheme("lingmo-down-symbolic", QIcon(":/icons/lingmo-down-symbolic")), tr("History"));

    auto historyButtonWidget = widgetForAction(m_history_action);
    auto historyButton = qobject_cast<QToolButton*>(historyButtonWidget);
    //historyButton->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    //historyButton->setArrowType(Qt::NoArrow);
    //historyButton->setPopupMode(QToolButton::DelayedPopup);

    connect(m_history_action, &QAction::triggered, [=]() {
        QMenu historyMenu(this);
        //historyButton->setMenu(&historyMenu);
        auto back_list = m_current_container->getBackList();
        auto current_uri = m_current_container->getCurrentUri();
        auto forward_list = m_current_container->getForwardList();
        QStringList historyList;
        historyList<<back_list;
        int currentIndex = historyList.count();
        historyList<<current_uri<<forward_list;
        QList<QAction*> actions;
        int count = 0;
        for (auto uri : historyList) {
            count++;
            auto action = historyMenu.addAction(QString::number(count) + ". " + uri);
            if (historyMenu.actions().indexOf(action) == currentIndex) {
                action->setCheckable(true);
                action->setChecked(true);
            }
            actions<<action;
        }
        historyMenu.addSeparator();
        historyMenu.addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Clear History"));
        //historyButton->showMenu();
        auto result = historyMenu.exec(historyButtonWidget->mapToGlobal(historyButton->rect().bottomLeft()));
        int clicked_index = historyMenu.actions().indexOf(result);
        if (clicked_index == historyMenu.actions().count() - 1) {
            //FIXME: clear history.
            m_back_action->setDisabled(true);
            m_forward_action->setDisabled(true);
            m_current_container->clearHistory();
        }
        qDebug()<<clicked_index;
        m_current_container->tryJump(clicked_index);
        //historyButton->setMenu(nullptr);
    });

    m_cd_up_action = addAction(QIcon::fromTheme("go-up"), tr("Cd Up"), [=]() {
        if (m_current_container) {
            m_current_container->cdUp();
        }
    });

    m_refresh_action = addAction(QIcon::fromTheme("gtk-refresh"), tr("Refresh"), [=]() {
        Q_EMIT refreshRequest();
    });

    updateActions();
}

void NavigationToolBar::updateActions()
{
    m_back_action->setEnabled(canGoBack());
    m_forward_action->setEnabled(canGoForward());
    m_cd_up_action->setEnabled(canCdUp());
}

bool NavigationToolBar::canCdUp()
{
    if (!m_current_container)
        return false;
    return m_current_container->canCdUp();
}

bool NavigationToolBar::canGoBack()
{
    if (!m_current_container)
        return false;
    return m_current_container->canGoBack();
}

bool NavigationToolBar::canGoForward()
{
    if (!m_current_container)
        return false;
    return m_current_container->canGoForward();
}

void NavigationToolBar::onGoBack()
{
    if (canGoBack()) {
        m_current_container->goBack();
    }
}

void NavigationToolBar::onGoForward()
{
    if (canGoForward()) {
        m_current_container->goForward();
    }
}

void NavigationToolBar::onGoToUri(const QString &uri, bool addHistory, bool forceUpdate)
{
    if (m_current_container) {
        m_current_container->goToUri(uri, addHistory);
        updateWindowLocationRequest(m_current_container->getCurrentUri(), false, forceUpdate);
    }
}

void NavigationToolBar::clearHistory()
{

}

void NavigationToolBar::setCurrentContainer(DirectoryViewContainer *container)
{
    if (m_current_container == container)
        return;
    m_current_container = container;
    Q_EMIT updateWindowLocationRequest(container->getCurrentUri(), false);
}
