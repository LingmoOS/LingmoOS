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

#ifndef GSETTINGS_H_
#define GSETTINGS_H_

#include <QObject>
#include <QString>
#include <QVariant>
#include <QHash>
#include <QByteArray>

namespace kdk
{
namespace kabase
{

class Gsettings : public QObject
{
    Q_OBJECT
public:
    Gsettings();
    ~Gsettings();

    /**
     * @brief 获取指针
     *
     * @param 无
     *
     * @return 获取到的指针
     */
    static Gsettings *getPoint(void);

    /* 通用 gsettings 使用 */
    /**
     * @brief 注册 gsettings
     *
     * @param flag 标识
     * @param schemasId gsettings id
     *
     * @retval 0 成功
     * @retval 非0 失败
     */
    static int registerObject(QString flag, QByteArray schemasId);

    /**
     * @brief 取消注册 gsettings
     *
     * @param flag 标识
     *
     * @retval 0 成功
     * @retval 非0 失败
     */
    static int cancelRegister(QString flag);

    /**
     * @brief 获取 key 值
     *
     * @param flag 标识
     * @param key key
     *
     * @return 获取到的 key 值
     */
    static QVariant get(QString flag, QString key);

    /**
     * @brief 设置 key 值
     *
     * @param flag 标识
     * @param key key
     * @param value value
     *
     * @retval 0 成功
     * @retval 非0 失败
     */
    static int set(QString flag, QString key, QVariant value);

    /**
     * @brief 重置 key 值
     *
     * @param flag 标识
     * @param key key
     *
     * @retval 0 成功
     * @retval 非0 失败
     */
    static int reset(QString flag, QString key);

    /**
     * @brief 获取所有 key 值
     *
     * @param flag 标识
     *
     * @retval 获取到的 key 值
     */
    static QStringList keys(QString flag);

    /* 系统 gsettings */
    /**
     * @brief 获取系统字号
     *
     * @param 无
     *
     * @return 获取到的系统字号
     */
    static QVariant getSystemFontSize(void);

    /**
     * @brief 获取系统透明度
     *
     * @param 无
     *
     * @return 获取到的系统透明度
     */
    static QVariant getSystemTransparency(void);

    /**
     * @brief 获取系统主题
     *
     * @param 无
     *
     * @return 获取到的系统主题
     */
    static QVariant getSystemTheme(void);

private:
    void conn(void);

Q_SIGNALS:
    void systemFontSizeChange();
    void systemTransparencyChange();
    void systemThemeChange();

private Q_SLOTS:
    void slotThemeChange(QString key);
    void slotControlCenterPersonaliseChange(QString key);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
