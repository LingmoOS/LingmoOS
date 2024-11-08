/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __SCREEN_H__
#define __SCREEN_H__


#include "screen_gamma.h"
#include "screen_power.h"

class Screen: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "lingmo.framework.devices.screen")
public:
    enum eScreenMode {
        primaryScreenOnlyMode = 0,
        mirrorScreenMode,
        extensionMode,
        secondaryScreenOnlyMode,
        showKDS,
        ERROR = 0x99};

    Q_ENUM(eScreenMode)

    enum eBrightnessControl {
        unknown = -1,//获取失败，理论上应该不会获取失败，是system的dbus服务
        gamma,//upm不控制背光则优先为gamma控制
        powerManager,//背光由upm控制。
        none};//第三种情况，upm不控制背光并且不支持gamma调节

    Q_ENUM(eBrightnessControl)

    explicit Screen(QObject *parent = nullptr);
    ~Screen();

public Q_SLOTS:
    /**
     * @brief setScreenBrightness 设置显示器亮度，需要先获取所有显示器的亮度再使用。
     * @param name 显示器名称
     * @param screenBrightness 指定亮度
     * @return
     */
    bool setScreenBrightness(QString name, uint screenBrightness);

    /**
     * @brief getScreenBrightness 获取显示器的亮度
     * @param name
     * @return 指定显示器的亮度
     */
    uint getScreenBrightness(QString name);

    /**
     * @brief setAllScreenBrightness 分别设置显示器的亮度
     * @param screenMap 显示器亮度与其对应的显示器亮度
     * @return 成功标记
     */
    bool setScreensBrightness(OutputGammaInfoList outputsInfo);

    /**
     * @brief setAllScreenSameBrightness 设置所有显示为同一个亮度
     * @param brighenss 全部亮度
     * @return 与参数一致
     */
    bool setAllScreenSameBrightness(uint brightness);

    /**
     * @brief getAllScreenBrightness
     * @return 与参数一致
     */
    OutputGammaInfoList getAllScreenBrightness();

    /**
     * @brief setPrimaryBrightness 设置主屏亮度
     * @param screenBrightness 指定主屏亮度
     * @return 与参数一致
     */
    bool setPrimaryScreenBrightness(uint screenBrightness);

    /**
     * @brief getPrimaryBrightness 获取主屏亮度
     * @return 主屏亮度
     */
    uint getPrimaryScreenBrightness();

    /**
     * @brief increaseBrightness 增加显示器亮度，效果等同于快捷键,目前仅笔记本可用
     * @return
     */
    bool increaseBuiltinScreenBrightness();

    /**
     * @brief decreaseBuiltinBrightness  降低显示器亮度，效果等同于快捷键,目前仅笔记本可用
     * @return
     */
    bool decreaseBuiltinBrightness();
    /**
     * @brief getScreenMode 获取显示器显示模式
     *   primaryScreenOnlyMode = 0, 第一屏幕
     *   mirrorScreenMode =,镜像屏幕
     *   extensionMode,扩展
     *   secondaryScreenOnlyMode,第二屏
     * @return 显示模式
     */
    QString getScreenMode();

    /**
     * @brief setScreenMode 设置显示模式
     * @param mode 指定模式
     *   primaryScreenOnlyMode = 0, 第一屏幕
     *   mirrorScreenMode =,镜像屏幕
     *   extensionMode,扩展
     *   secondaryScreenOnlyMode,第二屏
     * @return 设置标记，0失败，1成功
     */
    bool setScreenMode(const QString &mode);

    /**
     * @brief setScreensParam 设置显示器参数
     * @param screenParam 显示器参数组合
     * @return 设置状态
     */
    bool setScreensParam(const QString &screenParam);

    /**
     * @brief getScreensParam 获取显示器状态
     * @return 显示器信息
     */
    QString getScreensParam();

    /**
     * @brief openDisplaySetting 打开控制面板的显示器页面
     */
    void openDisplaySetting();

    /**
     * @brief setColorTemperature
     * @param appName
     * @param colorTemp
     * @return 设置标记，0失败，1成功
     */
    bool setColorTemperature(QString appName, int colorTemp);

    /**
     * @brief setEyeCare 设置护眼模式，
     * 0,进入护眼模式
     * 1,退出护眼模式
     * @param state
     * @return 设置标记，false 失败，true 成功
     */
    bool setEyeCareEnabled(bool state);

    /**
     * @brief getEyeCare 获取护眼模式
     * @return 护眼模式
     */
    bool isEyeCareEnabled();

    /**
     * @brief setNightMode 设置夜间模式
     * @param state 夜间模式状态
     * 0：关闭夜间模式
     * 1：开启夜间模式
     * @return 设置标记，0失败，1成功
     */
    bool setNightModeEnable(bool state);

    /**
     * @brief getNightMode 获取是否为夜间模式
     * @return 夜间模式 设置标记，0非夜间模式，1时进入模式
     */
    bool isNightModeEnable();

private:
    /**
     * @brief brightControlByUPM 获取电源管理是否可以控制亮度
     * @return
     */
    eBrightnessControl brightControlByUPM();

    /**
     * @brief watchDbusServer
     */
    void watchUpmRegisterDbusServer();

    QDBusMessage callGamma(const QString &method,
                       const QVariant &arg1 = QVariant(),
                       const QVariant &arg2 = QVariant(),
                       const QVariant &arg3 = QVariant(),
                       const QVariant &arg4 = QVariant());

    QDBusMessage callXrandr(const QString &method,
                        const QVariant &arg1 = QVariant(),
                        const QVariant &arg2 = QVariant(),
                        const QVariant &arg3 = QVariant(),
                        const QVariant &arg4 = QVariant());

    QDBusMessage callBrighenss(const QString &method,
                        const QVariant &arg1 = QVariant(),
                        const QVariant &arg2 = QVariant(),
                        const QVariant &arg3 = QVariant(),
                        const QVariant &arg4 = QVariant());

    QDBusMessage callMediaKeys(const QString &method,
                        const QVariant &arg1 = QVariant(),
                        const QVariant &arg2 = QVariant(),
                        const QVariant &arg3 = QVariant(),
                        const QVariant &arg4 = QVariant());

private Q_SLOTS:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    ScreenAbstract *m_pscreen = nullptr;

Q_SIGNALS:
    /**
     * @brief nightModeChanged 夜间模式变化
     */
    void nightModeChanged(bool);

    /**
     * @brief EyeCareChanged 护眼模式变化
     */
    void eyeCareChanged(bool);

    /**
     * @brief primaryScreenBrightnessChanged 主屏信号变化
     */
    void primaryScreenBrightnessChanged(uint);

    /**
     * @brief screenBrightnessChanged 亮度变化信号
     */
    void screenBrightnessChanged(QString name,uint brightness);

    /**
     * @brief modeChanged 模式改变信号
     * @param mode
     */
    void sceenModeChanged(QString mode);

    /**
     * @brief paramChanged 显示器参数
     * @param param
     */
    void paramChanged(QString param);

private:
    QString m_screenMode = "";
    QString m_screenParam = "";
    QString m_primaryName = "";
    QMap<QString, int> m_allScreenBright;

    uint8_t m_primaryBright = 0;
    uint8_t m_screenCount = 0;

    int m_brightControlByUPM = -1;

    QMetaEnum m_outputModeEnum;
    QGSettings *m_pColorGsettings = nullptr;
};
#endif

