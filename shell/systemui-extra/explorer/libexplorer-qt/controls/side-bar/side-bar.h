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

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "explorer-core_global.h"
#include "FMWindowIface.h"

#include <QDockWidget>

namespace Peony {

class PEONYCORESHARED_EXPORT SideBar : public QDockWidget
{
    Q_OBJECT
public:
    explicit SideBar(QWidget *parent = nullptr);
    ~SideBar();

    virtual QSize sizeHint() const;

    virtual void resizeEvent(QResizeEvent *event);

    FMWindowIface *getWindowIface();

Q_SIGNALS:
    void updateWindowLocationRequest(const QString &uri, bool addHistory = true, bool forceUpdate = false);
    void labelButtonClicked(bool checked);
    void labelIdChanged(int labelId);
    void labelNameChanged(const QString &labelName);
    void newTabRequest(const QString &uri);
    void newWindowRequest(const QString &uri);

    void propertiesWindowRequest(const QString &uri);
    void propertiesWindowRequest(const QStringList &uris);
};

}

#endif // SIDEBAR_H
