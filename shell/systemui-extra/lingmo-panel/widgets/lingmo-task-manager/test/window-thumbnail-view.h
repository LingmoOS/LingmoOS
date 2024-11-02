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
 *
 */

#ifndef LINGMO_TASK_MANAGER_TASKVIEW_H
#define LINGMO_TASK_MANAGER_TASKVIEW_H

#include <QQuickView>
#include <QPointer>
#include <QMenu>

class WindowThumbnailView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(bool viewShow READ viewShow WRITE setViewShow NOTIFY viewShowChanged)
    Q_PROPERTY(bool menuShow READ menuShow WRITE setMenuShow NOTIFY menuShowChanged)
    Q_PROPERTY(QPoint viewPoint READ viewPoint WRITE setViewPoint NOTIFY viewPointChanged)
    Q_PROPERTY(QStringList viewModel READ viewModel WRITE setViewModel NOTIFY viewModelChanged)
public:
    explicit WindowThumbnailView(QWindow *parent = nullptr);
    ~WindowThumbnailView() = default;

    bool viewShow() const;
    void setViewShow(const bool &viewShow);

    bool menuShow() const;
    void setMenuShow(const bool &menuShow);

    QStringList viewModel() const;
    void setViewModel(const QStringList &model);

    QPoint viewPoint() const;
    void setViewPoint(const QPoint &point);

    Q_INVOKABLE void openMenu(const QString &winId);

protected:
    bool event(QEvent *event);

private:
    QStringList m_viewModel;
    QPoint m_viewPoint;
    QPointer<QMenu> m_contextMenu;
    bool m_viewShow = false;
    bool m_menuShow = false;

Q_SIGNALS:
    void viewModelChanged(QStringList m_viewModel);
    void viewPointChanged(QPoint m_viewPoint);
    void viewShowChanged(bool m_viewShow);
    void menuShowChanged(bool m_menuShow);
};


#endif //LINGMO_TASK_MANAGER_TASKVIEW_H
