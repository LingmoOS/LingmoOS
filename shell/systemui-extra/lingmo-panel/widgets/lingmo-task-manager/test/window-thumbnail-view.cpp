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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *          qiqi49 <qiqi@kylinos.cn>
 */

#include <QQmlEngine>
#include <QQmlContext>

#include "window-thumbnail-view.h"
#include "lingmo-task-manager.h"
#include "actions.h"

#include <windowmanager/windowmanager.h>
class MousePressHandler : public QObject
{
    Q_OBJECT
public:
    explicit MousePressHandler(WindowThumbnailView *parent = nullptr);
    ~MousePressHandler() = default;

private:
    QRect m_windowGeometry;
    WindowThumbnailView *m_windowThumbnailView;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

MousePressHandler::MousePressHandler(WindowThumbnailView *parent) : QObject(parent)
{
    m_windowThumbnailView = parent;
    m_windowGeometry = m_windowThumbnailView->geometry();
}

bool MousePressHandler::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPoint pos = mouseEvent->globalPos();
        if (!m_windowGeometry.contains(pos)) {
            m_windowThumbnailView->setMenuShow(false);
            m_windowThumbnailView->setViewShow(false);
        }
    }
    return QObject::eventFilter(watched, event);
}

WindowThumbnailView::WindowThumbnailView(QWindow *parent)
{
    setResizeMode(QQuickView::SizeViewToRootObject);
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    setColor("transparent");

//    qmlRegisterType<XWindowThumbnail>("org.lingmo.taskManager.core", 1, 0, "XWindowThumbnail");
    engine()->rootContext()->setContextProperty("thumbnailView", this);
    engine()->rootContext()->setContextProperty("taskModel", &TaskManager::LingmoUITaskManager::self());
    setSource(QUrl("qrc:/ThumbnailView.qml"));
}

QStringList WindowThumbnailView::viewModel() const
{
    return m_viewModel;
}

void WindowThumbnailView::setViewModel(const QStringList &model)
{
    m_viewModel.clear();

    if (model.isEmpty()) {
        return;
    }
    m_viewModel = model;

    Q_EMIT viewModelChanged(m_viewModel);
}

QPoint WindowThumbnailView::viewPoint() const
{
    return m_viewPoint;
}

void WindowThumbnailView::setViewPoint(const QPoint &point)
{
    m_viewPoint = QPoint(point.x(), point.y() - height());
}

bool WindowThumbnailView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Expose: {
        if (isExposed()) {
            kdk::WindowManager::setSkipTaskBar(this, true);
            kdk::WindowManager::setSkipSwitcher(this, true);
            kdk::WindowManager::setGeometry(this, QRect(m_viewPoint, QSize(width(), height())));
        }
        break;
    }
    default:
        break;
    }
    return QQuickWindow::event(event);
}

void WindowThumbnailView::setViewShow(const bool &viewShow)
{
    m_viewShow = viewShow;
    if (!m_menuShow && !m_viewShow) {
        hide();
    }
}

bool WindowThumbnailView::viewShow() const
{
    return m_viewShow;
}

void WindowThumbnailView::openMenu(const QString &winId)
{
    TaskManager::Actions menuActions = TaskManager::LingmoUITaskManager::self().windowActions(winId);

    setMenuShow(true);
    if (m_contextMenu) {
        m_contextMenu->close();
    }

    QMenu *menu = new QMenu;
    MousePressHandler *mousePressHandle = new MousePressHandler(this);
    menu->installEventFilter(mousePressHandle);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    for (const TaskManager::Action* menuAction : menuActions) {
        menu->addAction(menuAction->internalAction());
        menuAction->internalAction()->setParent(menu);
    }

    if (menu->actions().isEmpty()) {    
        delete menu;
        return;
    }

    connect(menu, &QMenu::aboutToHide, this, [this] {
        setMenuShow(false);
    });
    m_contextMenu = menu;
    menu->popup(QCursor::pos());
}

bool WindowThumbnailView::menuShow() const
{
    return m_menuShow;
}

void WindowThumbnailView::setMenuShow(const bool &menuShow)
{
    m_menuShow =  menuShow;
}

#include "window-thumbnail-view.moc"
