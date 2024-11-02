/*
 * liblingmosdk-qtwidgets's Library
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

#ifndef KLINEFRAME_H
#define KLINEFRAME_H

#include <QFrame>
#include "gui_g.h"
namespace kdk {

class KHLineFramePrivate;
class KVLineFramePrivate;

class GUI_EXPORT KHLineFrame : public QFrame
{
    Q_OBJECT
public:
    KHLineFrame(QWidget* parent = nullptr);
    ~KHLineFrame();

    /**
     * @brief 设置是否遵循默认色板
     * @param flag
     * @since 2.4
     */
    void setFollowPalette(bool flag);
private:
    Q_DECLARE_PRIVATE(KHLineFrame)
    KHLineFramePrivate* const d_ptr;
};

class GUI_EXPORT KVLineFrame : public QFrame
{
    Q_OBJECT
public:
    KVLineFrame(QWidget* parent = nullptr);
    ~KVLineFrame();
    /**
     * @brief 设置是否遵循默认色板
     * @param flag
     * @since 2.4
     */
    void setFollowPalette(bool flag);
private:
    Q_DECLARE_PRIVATE(KVLineFrame)
    KVLineFramePrivate* const d_ptr;
};

}
#endif // KLINEFRAME_H
