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

#ifndef STABLECOMPUTERPROPERTIESPAGE_H
#define STABLECOMPUTERPROPERTIESPAGE_H
#include <QWidget>

#include "properties-window-tab-iface.h"
#include "explor-core_global.h"
#include "volume-manager.h"

class QFormLayout;

namespace Peony {

class PEONYCORESHARED_EXPORT ComputerPropertiesPage : public PropertiesWindowTabIface
{
    Q_OBJECT
public:
    explicit ComputerPropertiesPage(const QString &uri, QWidget *parent = nullptr);

protected:
    void addSeparator();

    /**
     * \brief dbus获取文件系统函数
     * \param uri
     * \return
     */
    QString getFileSystemType(QString uri);

    /*!
     * \brief 通过给定的targetUri从全部卷中获取匹配的卷。
     * \param targetUri 需要匹配的目标Uri (挂载路径)
     * \return 具有给定targetUri的卷，如果不存在则返回 nullptr
     */
    static std::shared_ptr<Volume> EnumerateOneVolumeByTargetUri(QString targetUri);

    /**
     * \brief 统一格式化容量字符串
     * \param capacityNum
     * \return
     */
    QString formatCapacityString(quint64 capacityNum);
private:
    QString m_uri;
    QFormLayout *m_layout;

    // PropertiesWindowTabIface interface
public:
    void saveAllChange();
};

}

#endif // COMPUTERPROPERTIESPAGE_H
