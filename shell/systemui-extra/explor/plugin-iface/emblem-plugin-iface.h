/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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

#ifndef EMBLEMPLUGINIFACE_H
#define EMBLEMPLUGINIFACE_H

#include "plugin-iface.h"
#include "explor-core_global.h"

#include "emblem-provider.h"

#define EmblemPluginInterface_iid "org.lingmo.explor-qt.plugin-iface.EmblemPluginInterface"

namespace Peony {

class PEONYCORESHARED_EXPORT EmblemPluginInterface : public PluginInterface
{
public:
    virtual ~EmblemPluginInterface() {}

    virtual EmblemProvider *create() = 0;
};

}
Q_DECLARE_INTERFACE (Peony::EmblemPluginInterface, EmblemPluginInterface_iid)

#endif // EMBLEMPLUGINIFACE_H
