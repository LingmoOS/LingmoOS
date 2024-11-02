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

#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QToolBar>
#include "explor-core_global.h"

class QMenu;
class QActionGroup;

namespace Peony {

class DirectoryViewContainer;

class AdvancedLocationBar;
class NavigationToolBar;

class NavigationBar : public QToolBar
{
    Q_OBJECT
public:
    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar();

    bool isPathEditing();
    const QString getLastPreviewPageId();

Q_SIGNALS:
    void updateWindowLocationRequest(const QString &uri, bool addHistory, bool forceUpdate = false);
    void refreshRequest();

    void switchPreviewPageRequest(const QString &id);

public Q_SLOTS:
    void bindContainer(DirectoryViewContainer *container);
    void updateLocation(const QString &uri);

    void setBlock(bool block = true);
    void startEdit();
    void finishEdit();

    void triggerAction(const QString &id);

protected:
    const QString getCurrentUri();

    void paintEvent(QPaintEvent *e);

private:
    NavigationToolBar *m_left_control;
    AdvancedLocationBar *m_center_control;
    QActionGroup *m_group;

    QAction *m_checked_preview_action;
    QString m_last_preview_page_id_in_window = nullptr;
};

}

#endif // NAVIGATIONBAR_H
