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

#ifndef LINGMO_TASK_MANAGER_TASKVIEW_H
#define LINGMO_TASK_MANAGER_TASKVIEW_H

#include <QQuickView>
#include <QPointer>
#include <QMenu>

class ThumbnailView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(bool viewVisible READ viewVisible WRITE setViewVisible NOTIFY viewVisibleChanged)
    Q_PROPERTY(QPoint viewPoint READ viewPoint WRITE setViewPoint NOTIFY viewPointChanged)
    Q_PROPERTY(QStringList viewModel READ viewModel WRITE setViewModel NOTIFY viewModelChanged)
    Q_PROPERTY(bool menuVisible READ menuVisible WRITE setMenuVisible NOTIFY menuVisibleChanged)
    Q_PROPERTY(QString winId READ getWinId)
public:
    static ThumbnailView &self();
    ~ThumbnailView() = default;

    bool viewVisible() const;
    void setViewVisible(const bool &viewVisible);

    QStringList viewModel() const;
    void setViewModel(const QStringList &model);

    QPoint viewPoint() const;
    void setViewPoint(const QPoint &point);

    QString getWinId() const;

    bool menuVisible() const;
    void setMenuVisible(const bool &menuVisible);

    Q_INVOKABLE void openMenu(const QString &winId);

protected:
    bool event(QEvent *event);

private:
    explicit ThumbnailView(QWindow *parent = nullptr);

    QStringList m_viewModel;
    QPoint m_viewPoint;
    bool m_viewVisible = false;
    bool m_menuVisible = false;
    QPointer<QMenu> m_contextMenu;
    QMenu *m_menu;

Q_SIGNALS:
    void viewModelChanged(QStringList m_viewModel);
    void viewPointChanged(QPoint m_viewPoint);
    void viewVisibleChanged(bool m_viewVisible);
    void menuVisibleChanged(bool m_menuVisible);
};

#endif //LINGMO_TASK_MANAGER_TASKVIEW_H
