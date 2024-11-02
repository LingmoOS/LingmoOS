/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef OPERATIONMENU_H
#define OPERATIONMENU_H

#include "about-dialog.h"
#include <QMenu>

class MainWindow;
class QToolButton;
class OperationMenuEditWidget;
class QWidgetAction;

class OperationMenu : public QMenu
{
    Q_OBJECT
public:
    explicit OperationMenu(MainWindow *window, QWidget *parent = nullptr);

public Q_SLOTS:
    void updateMenu();

private:
    QAction *m_show_hidden = nullptr;
    QAction *m_forbid_thumbnailing = nullptr;
    QAction *m_resident_in_backend = nullptr;
    QAction *m_showFileExtension = nullptr;
    QAction *m_showCreateTime = nullptr;
    QAction *m_showFoldersInNewWindow = nullptr;
    QAction *m_showRelativeTime = nullptr;

private:
    MainWindow *m_window = nullptr;
    OperationMenuEditWidget *m_edit_widget = nullptr;
    QWidgetAction *m_editWidgetContainer = nullptr;
};

class OperationMenuEditWidget : public QWidget
{
public:
    friend class OperationMenu;
    Q_OBJECT

Q_SIGNALS:
    void operationAccepted();

private:
    explicit OperationMenuEditWidget(MainWindow *window, QWidget *parent = nullptr);

    void updateActions(const QString &currentDirUri, const QStringList &selections);

    QToolButton *m_copy = nullptr;
    QToolButton *m_paste = nullptr;
    QToolButton *m_cut = nullptr;
    QToolButton *m_trash = nullptr;
};

#endif // OPERATIONMENU_H
