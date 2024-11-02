/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef INTEL_COMPUTERPERSONALITEM_H
#define INTEL_COMPUTERPERSONALITEM_H

#include "intel-abstract-computer-item.h"

namespace Intel {

class ComputerPersonalItem : public AbstractComputerItem
{
    Q_OBJECT
public:
    explicit ComputerPersonalItem(const QString &uri, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent = nullptr);

    Type itemType() override {return Personal;}
    const QString uri() override {return m_uri;}
    const QString displayName() override;
    bool hasChildren() override {return !m_parentNode;}
    void findChildren() override;
    void clearChildren() override;

private:
    QString m_uri;
};

}

#endif // COMPUTERPERSONALITEM_H
