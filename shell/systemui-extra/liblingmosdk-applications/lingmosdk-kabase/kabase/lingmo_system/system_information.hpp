/*
 * liblingmosdk-base's Library
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#ifndef LINGMO_SYSTEM_SYSTEM_INFORMATION_
#define LINGMO_SYSTEM_SYSTEM_INFORMATION_

#include <QString>

namespace kdk
{
namespace kabase
{

class SystemInformation
{
public:
    SystemInformation();
    ~SystemInformation();

    /**
     * @brief 根据 lsb-release 文件的 key 值 获取信息
     *
     * @param key 键值
     *
     * @return 获取到的信息 , 为空时可能确实为空也可能失败
     */
    QString getLsbReleaseInformation(QString key);

    /**
     * @brief 根据 os-release 文件的 key 值 获取信息
     *
     * @param key 键值
     *
     * @return 获取到的信息 , 为空时可能确实为空也可能失败
     */
    QString getOsReleaseInformation(QString key);

    /**
     * @brief 获取 PROJECT_CODENAME 字段的值
     *
     * @param 无
     *
     * @retval 空 失败
     * @retval 非空 获取到的值
     */
    QString getProjectCodeName(void);

    /**
     * @brief 获取 CPU 型号
     *
     * @param 无
     *
     * @retval 空 失败
     * @retval 非空 获取到的值
     */
    QString getCpuModelName(void);

    /**
     * @brief 获取硬件平台信息
     *
     * @param 无
     *
     * @retval 空 失败
     * @retval 非空 获取到的值
     */
    QString getHdPlatform(void);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
