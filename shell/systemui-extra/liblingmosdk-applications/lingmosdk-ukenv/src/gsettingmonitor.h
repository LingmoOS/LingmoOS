/*
 * liblingmosdk-ukenv's Library
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

#ifndef LINGMOSDK_UKENV_GSETTINGMONITOR_H_
#define LINGMOSDK_UKENV_GSETTINGMONITOR_H_

#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QObject>

namespace kdk
{

class GsettingMonitor : public QObject
{
    Q_OBJECT

public:
    ~GsettingMonitor();

    /**
     * @brief 获取单例指针
     *
     * @param 无
     *
     * @return 获取到的单例指针
     */
    static GsettingMonitor *getInstance(void);

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

Q_SIGNALS:
    void systemFontSizeChange();
    void systemTransparencyChange();
    void systemThemeChange();

private Q_SLOTS:
    void themeChange(QString key);
    void controlCenterPersonaliseChange(QString key);

private:
    GsettingMonitor();
    void conn(void);
    static bool registerGsetting(QString flag, QByteArray schemasId);

    static GsettingMonitor m_gsettingMonitor;
};

} // namespace kdk

#endif
