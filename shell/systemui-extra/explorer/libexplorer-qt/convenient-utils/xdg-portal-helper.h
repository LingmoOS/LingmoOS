/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef XDGPORTALHELPER_H
#define XDGPORTALHELPER_H

#include "explorer-core_global.h"
#include <QObject>

namespace Peony {

class PEONYCORESHARED_EXPORT XdgPortalHelper : public QObject
{
    Q_OBJECT
public:
    static XdgPortalHelper *getInstance();
    bool tryUnusePortal();
    bool tryResetPortal();

private:
    explicit XdgPortalHelper(QObject *parent = nullptr);
};

}

#endif // XDGPORTALHELPER_H
