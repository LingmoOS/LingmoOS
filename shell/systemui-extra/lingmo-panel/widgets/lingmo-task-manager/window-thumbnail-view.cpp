/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#include <QQmlEngine>
#include <QQmlContext>

#include "window-thumbnail-view.h"
#include "lingmo-task-manager.h"
#include "actions.h"

#include <windowmanager/windowmanager.h>

ThumbnailView &ThumbnailView::self()
{
    static ThumbnailView s_self;
    return s_self;
}

QStringList ThumbnailView::viewModel() const
{
    return m_viewModel;
}

void ThumbnailView::setViewModel(const QStringList &model)
{
    m_viewModel.clear();

    if (model.isEmpty()) {
        return;
    }
    m_viewModel = model;

    Q_EMIT viewModelChanged(m_viewModel);
}

QPoint ThumbnailView::viewPoint() const
{
    return m_viewPoint;
}

void ThumbnailView::setViewPoint(const QPoint &point)
{
    m_viewPoint = QPoint(point.x(), point.y() - height());
}

bool ThumbnailView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Expose: {
        if (isExposed()) {
            kdk::WindowManager::setSkipTaskBar(this, true);
            kdk::WindowManager::setSkipSwitcher(this, true);
            kdk::WindowManager::setGeometry(this, QRect(m_viewPoint, QSize(width(), height())));
//            kdk::WindowManager::setGeometry(this, QRect(QPoint(100, 100), QSize(200, 200)));
        }
        break;
    }

    case QEvent::MouseButtonPress: {
        if (m_menuVisible) {
            m_menu->hide();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QQuickWindow::event(event);
}

ThumbnailView::ThumbnailView(QWindow *parent)
{
    setResizeMode(QQuickView::SizeViewToRootObject);
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setColor("transparent");

    engine()->rootContext()->setContextProperty("thumbnailView", this);
    engine()->rootContext()->setContextProperty("taskModel", &TaskManager::LingmoUITaskManager::self());
    setSource(QUrl("qrc:/org.lingmo.panel.taskManager/qml/ThumbnailView.qml"));
}

void ThumbnailView::setViewVisible(const bool &viewVisible)
{
    m_viewVisible = viewVisible;
    if (!m_menuVisible && !m_viewVisible) {
        TaskManager::LingmoUITaskManager::self().activateWindowView({});
        hide();
    }
}

bool ThumbnailView::viewVisible() const
{
    return m_viewVisible;
}

QString ThumbnailView::getWinId() const
{
    return QString::number(QWindow::winId());
}

void ThumbnailView::openMenu(const QString &winId)
{
    TaskManager::Actions menuActions = TaskManager::LingmoUITaskManager::self().windowActions(winId);

    setMenuVisible(true);
    if (m_contextMenu) {
        m_contextMenu->close();
    }

    m_menu = new QMenu;
    m_menu->setAttribute(Qt::WA_DeleteOnClose);

    for (const TaskManager::Action* menuAction : menuActions) {
        m_menu->addAction(menuAction->internalAction());
        menuAction->internalAction()->setParent(m_menu);
    }

    if (m_menu->actions().isEmpty()) {
        delete m_menu;
        return;
    }

    connect(m_menu, &QMenu::aboutToHide, this, [this] {
        setMenuVisible(false);
        if (!geometry().contains(QCursor::pos())) {
            setViewVisible(false);
        }
    });
    connect(m_menu, &QMenu::triggered, this, [this] {
        setViewVisible(false);
    });
    m_contextMenu = m_menu;
    m_menu->popup(QCursor::pos());
}

bool ThumbnailView::menuVisible() const
{
    return m_menuVisible;
}

void ThumbnailView::setMenuVisible(const bool &menuVisible)
{
    m_menuVisible =  menuVisible;
}