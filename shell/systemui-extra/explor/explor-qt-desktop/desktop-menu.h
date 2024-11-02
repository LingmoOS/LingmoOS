/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef DESKTOPMENU_H
#define DESKTOPMENU_H

#include <QMenu>
#include "desktop-icon-view.h"

namespace Peony {

class DirectoryViewIface;

class DesktopMenu : public QMenu
{
    Q_OBJECT
public:
    explicit DesktopMenu(DirectoryViewIface *view, QWidget *parent = nullptr);
    const QStringList urisToEdit() {
        return m_uris_to_edit;
    }

    void showProperties(const QString &uri);
    void showProperties(const QStringList &uris);

Q_SIGNALS:
    void setDefaultZoomLevel(Peony::DesktopIconView::ZoomLevel level);
    void setSortType(int sortType);

protected:
    void fillActions();
    const QList<QAction *> constructOpenOpActions();
    const QList<QAction *> constructCreateTemplateActions();
    const QList<QAction *> constructViewOpActions();
    const QList<QAction *> constructFileOpActions();
    const QList<QAction *> constructMenuPluginActions(); //directory view menu extension.
    const QList<QAction *> constructFilePropertiesActions();

    void openWindow(const QString &uri);
    void openWindow(const QStringList &uris);
    void gotoAboutComputer();

private:
    DirectoryViewIface *m_view;
    QString m_directory;
    QStringList m_selections;

    QStringList m_uris_to_edit;

    const int ELIDE_TEXT_LENGTH = 16;
};

}

#endif // DESKTOPMENU_H
