/*
 * liblingmosdk-waylandhelper's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef WMREGISTER_H
#define WMREGISTER_H

#include <QObject>
#include "../lingmosdk-waylandhelper_global.h"
#include "abstractinterface.h"


namespace kdk
{
class LINGMOSDKWAYLANDHELPER_EXPORT WmRegister:public QObject
{
public:
    WmRegister(QObject*parent = nullptr);
    ~WmRegister();

    AbstractInterface* winInterface();

private:
    AbstractInterface *m_winInterface{nullptr};
};
}

#endif // WMREGISTER_H
