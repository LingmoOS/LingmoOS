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

#ifndef KBREADCRUMB_H
#define KBREADCRUMB_H

#include <QTabBar>

namespace kdk
{
/**
 *  @defgroup BreadCrumb
 */
class KBreadCrumbPrivate;

/**
 * @brief KBreadCrumbType，包括两种类型。
 */
enum KBreadCrumbType
{
    FlatBreadCrumb,
    CubeBreadCrumb
};

/**
 * @brief 面包屑控件
 */
class KBreadCrumb : public QTabBar
{
    Q_OBJECT

public:
    explicit KBreadCrumb(QWidget *parent = nullptr);

    /**
     * @brief 设置图标
     * @param icon
     */
    void setIcon(const QIcon &icon);

    /**
     * @brief 获取图标
     * @return
     */
    QIcon icon() const;

    /**
     * @brief 返回按钮边框是否凸起
     * @return
     */
    bool  isFlat() const;

    /**
     * @brief 设置按钮边框是否凸起
     * @param flat
     */
    void  setFlat(bool flat);

protected:
    QSize tabSizeHint(int index) const;
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KBreadCrumb)
    KBreadCrumbPrivate*const d_ptr;
};
}


#endif // KBREADCRUMB_H
