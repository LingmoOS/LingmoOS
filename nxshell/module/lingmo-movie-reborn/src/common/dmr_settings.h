// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_SETTINGS_H
#define _DMR_SETTINGS_H

#include <QObject>
#include <QPointer>

#include <DSettingsOption>
#include <DSettingsGroup>
#include <DSettings>

namespace dmr {
using namespace Dtk::Core;
/**
 * @file 配置文件管理类，封装读取和保存配置文件等操作
 */
class Settings: public QObject
{
    Q_OBJECT
    
signals:
    /**
     * @brief shortcuts配置值变化后发送的信号
     * @param 某一功能
     * @param 快捷键
     */
    void shortcutsChanged(const QString &, const QVariant &);
    /**
     * @brief base配置值变化后发送的信号
     * @param base下的某一配置
     * @param 配置的值
     */
    void baseChanged(const QString &, const QVariant &);
    /**
     * @brief subtitle配置值变化后发送的信号
     * @param subtitle下的某一配置
     * @param 配置的值
     */
    void subtitleChanged(const QString &, const QVariant &);
    /**
     * @brief base.play.playmode配置值变化后发送的信号
     * @param base.play.playmode下的某一配置
     * @param 配置的值
     */
    void defaultplaymodechanged(const QString &, const QVariant &);
    /**
      * @brief base.decode.select 配置值变化后发送的信号
      * @param base.decode.select 下的某一配置
      * @param 配置的值
      */
    void setDecodeModel(const QString &, const QVariant &);
    /**
      * @brief 刷新解码方式
      */
    void refreshDecode();
    /**
     * @brief base.play.mute配置值变化后发送的信号
     * @param base.play.mute下的某一配置
     * @param 配置的值
     */
    void baseMuteChanged(const QString &, const QVariant &);
    /**
     * @brief base.play.hwaccel配置值变化后发送的信号
     * @param base.play.hwaccel下的某一配置
     * @param 配置的值
     */
    void hwaccelModeChanged(const QString &, const QVariant &);
    
public:
    enum Flag {
        ClearWhenQuit,
        ShowThumbnailMode,
        ResumeFromLast,
        AutoSearchSimilar,
        PreviewOnMouseover,
        MultipleInstance,
        PauseOnMinimize,
    };
    /**
     * @brief 获取类单列对象
     */
    static Settings &get();
    /**
     * @brief 获取类单列对象
     * @param 返回配置文件路径
     */
    QString configPath() const
    {
        return m_sConfigPath;
    }
    /**
     * @brief 获取DSetting指针
     * @param DSetting指针
     */
    QPointer<DSettings> settings()
    {
        return m_pSettings;
    }
    /**
     * @brief 返回对应sname的分组配置
     * @param DSettingsGroup类指针
     */
    QPointer<DSettingsGroup> group(const QString &sName)
    {
        return settings()->group(sName);
    }
    /**
     * @brief 返回对应shortcuts的分组配置
     * @param DSettingsGroup类指针
     */
    QPointer<DSettingsGroup> shortcuts()
    {
        return group("shortcuts");
    }
    /**
     * @brief 返回对应base的分组配置
     * @param DSettingsGroup类指针
     */
    QPointer<DSettingsGroup> base()
    {
        return group("base");
    }
    /**
     * @brief 返回对应subtitle的分组配置
     * @param DSettingsGroup类指针
     */
    QPointer<DSettingsGroup> subtitle()
    {
        return group("subtitle");
    }
    /**
     * @brief 设置base.general分组下的配置值
     * @param 配置项名
     * @param 配置值
     */
    void setGeneralOption(const QString &sOpt, const QVariant &var);
    /**
     * @brief 返回base.general的配置值
     * @param 配置项名
     * @return 配置值
     */
    QVariant generalOption(const QString &sOpt);
    /**
     * @brief 设置base.play分组下的配置值
     * @param 配置项名
     * @param 配置值
     */
    void setInternalOption(const QString &sOpt, const QVariant &var);
    /**
     * @brief 返回base.play的配置值
     * @param 配置项名
     * @return 配置值
     */
    QVariant internalOption(const QString &sOpt);

    // user override for mpv opengl interop
    /**
     * @brief 返回forced_interop的配置值
     * @return 配置值
     */
    QString forcedInterop();
    // disable interop at all
    /**
     * @brief 返回disable_interop的配置值
     * @return 配置值
     */
    bool disableInterop();

    // convient helpers
    /**
     * @brief 返回Flag枚举中某一配置的值(bool)
     * @return 配置的值
     */
    bool isSet(Flag f) const;
    /**
     * @brief 返回影院支持的协议
     * @return 播发协议集合
     */
    QStringList commonPlayableProtocols() const;
    /**
     * @brief 判断影院是否支持改协议
     * @param 协议
     * @return 是否支持
     */
    bool iscommonPlayableProtocol(const QString &sScheme) const;
    /**
     * @brief 返回截图路径
     * @return 截图路径
     */
    QString screenshotLocation();
    /**
     * @brief 返回截图路径
     * @return 截图路径
     */
    QString screenshotNameTemplate();
    /**
     * @brief 生成连拍截图文件名
     * @return 截图文件名
     */
    QString screenshotNameSeqTemplate();
    /**
     * @brief 获取键值
     * @return 键值
     */
    static QString flag2key(Settings::Flag f);


public slots:
    /**
     * @brief 崩溃检测
     */
    void crashCheck();
    /**
     * @brief 设置崩溃状态
     */
    void onSetCrash();

private:
    Settings();

    QPointer<DSettings> m_pSettings;   ///DSetting指针
    QString m_sConfigPath;             ///配置文件路径
    static Settings *m_pTheSettings;   ///单例唯一实例
};

}

#endif /* ifndef _DMR_SETTINGS_H */
