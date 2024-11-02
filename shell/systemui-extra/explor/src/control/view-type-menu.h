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

#ifndef VIEWTYPEMENU_H
#define VIEWTYPEMENU_H

#include <QMenu>

namespace Peony {
class ViewFactorySortFilterModel2;
}

class ViewTypeMenu : public QMenu
{
    Q_OBJECT
public:
    explicit ViewTypeMenu(QWidget *parent = nullptr);
    const QIcon getCurrentIconFromViewId();

Q_SIGNALS:
    void switchViewRequest(const QString &viewId, const QIcon &icon, bool resetToZoomLevelHint = false);
    void updateZoomLevelHintRequest(int zoomLevelHint);

public Q_SLOTS:
    void setCurrentView(const QString &viewId, bool blockSignal = false);
    void setCurrentDirectory(const QString &uri);

protected:
    bool isViewIdValid(const QString &viewId);
    void updateMenuActions();

private:
    QString m_current_uri;

    QString m_current_view_id;
    Peony::ViewFactorySortFilterModel2 *m_model;

    QActionGroup *m_view_actions;
};

#endif // VIEWTYPEMENU_H
