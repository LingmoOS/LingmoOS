/*
 *
 * Copyright:  2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@cs2c.com.cn.com>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#ifndef SCREENVOICE_H
#define SCREENVOICE_H

#include "src/tools/CaptureToolCap.h"
class ScreenVoice : public CaptureToolCap
{
    Q_OBJECT
public:
    explicit ScreenVoice(QObject *parent = nullptr);
    QString name() const override;
    static QString nameID();
    QString description() const override;
    bool isValid() const override;
};

#endif // SCREENVOICE_H
