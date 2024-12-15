// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2021-2021 Uniontech Technology Co., Ltd.
 *
 * @file dlmapplication.h
 *
 * @brief 重写DApplication，重载关闭事件
 *
 * @date 2021-01-09 17:54
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef DLMAPPLICATION_H
#define DLMAPPLICATION_H

#include <DApplication>
DWIDGET_USE_NAMESPACE

class DlmApplication : public DApplication
{
    Q_OBJECT
public:
    DlmApplication(int &argc, char **argv);

protected:
    virtual void handleQuitAction();
signals:
    void applicatinQuit(bool force);
};

#endif // DLMAPPLICATION_H
