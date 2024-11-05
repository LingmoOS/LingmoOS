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

#ifndef TABPAGE_H
#define TABPAGE_H

#include <QTabWidget>
#include "explorer-core_global.h"

#include <QTimer>

namespace Peony {

class DirectoryViewContainer;

class PEONYCORESHARED_EXPORT TabPage : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabPage(QWidget *parent = nullptr);

    DirectoryViewContainer *getActivePage();

Q_SIGNALS:
    void currentActiveViewChanged();
    void currentLocationChanged();
    void currentSelectionChanged();

    void viewTypeChanged();

    void updateWindowLocationRequest(const QString &uri, bool addHistory = true, bool forceUpdate = false);

    void menuRequest(const QPoint &pos);
    void signal_itemAdded(const QString &uri);

public Q_SLOTS:
    void addPage(const QString &uri);
    void refreshCurrentTabText();

    void stopLocationChange();

protected:
    void rebindContainer();

private:
    QTimer m_double_click_limiter;

    const int ELIDE_TEXT_LENGTH = 16;
};

}

#endif // TABPAGE_H
