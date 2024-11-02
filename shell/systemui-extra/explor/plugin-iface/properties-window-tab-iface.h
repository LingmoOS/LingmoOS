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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#ifndef PROPERTIESWINDOWTABIFACE_H
#define PROPERTIESWINDOWTABIFACE_H

#include <QWidget>
#include "explor-core_global.h"

#define PropertiesWindowTabIface_iid "org.lingmo.explor-qt.plugin-iface.PropertiesWindowTabInterface"

namespace Peony {

class PEONYCORESHARED_EXPORT PropertiesWindowTabIface : public QWidget
{
    Q_OBJECT

public:
    PropertiesWindowTabIface(QWidget *parent = nullptr) : QWidget(parent){}
    virtual ~PropertiesWindowTabIface() {}
    PropertiesWindowTabIface();

    bool m_thisPageChanged = false;
    /*!
     * 作为当前页面是否改变的依据
     * \brief
     */
    virtual void thisPageChanged() {
        this->m_thisPageChanged = true;
    }
    /*!
     * \brief 响应确认按钮保存全部数据
     */
    virtual void saveAllChange() {};

Q_SIGNALS:
    /*!
     * 当前tab页面请求关闭窗口
     * \brief
     */
    void requestCloseMainWindow();

};

}

Q_DECLARE_INTERFACE(Peony::PropertiesWindowTabIface, PropertiesWindowTabIface_iid)

#endif // PROPERTIESWINDOWTABIFACE_H
