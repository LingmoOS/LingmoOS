// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGAPPLICATIONHELPER_H
#define LOGAPPLICATIONHELPER_H

#include "structdef.h"
#include "dtkcore_config.h"
#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#endif

#include <QMap>
#include <QObject>
#include <QGSettings/QGSettings>

#include <mutex>

/**
 * @brief The LogApplicationHelper class 获取应用日志文件路径信息工具类
 */
class LogApplicationHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief instance 全局单列模式实现
     * @return 此类的唯一对象
     */
    static LogApplicationHelper *instance()
    {
        LogApplicationHelper *sin = m_instance.load();
        if (!sin) {
            //加锁 线程安全
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = m_instance.load();
            if (!sin) {
                sin = new LogApplicationHelper();
                m_instance.store(sin);
            }
        }
        return sin;
    }

    QMap<QString, QString> getMap();
    // 刷新并返回最新的应用配置信息
    AppLogConfigList getAppLogConfigs();

    //根据包名获得显示名称
    QString transName(const QString &str);

    //根据包名获得路径
    QString getPathByAppId(const QString &str);

    //获取所有其他日志文件列表(名称-路径)
    QList<QStringList> getOtherLogList();

    //获取所有自定义日志文件列表(名称-路径)
    QList<QStringList> getCustomLogList();

    AppLogConfig  appLogConfig(const QString& app);
    bool isAppLogConfigExist(const QString& app);

    // 验证是否为有效的应用名
    bool isValidAppName(const QString& appName);

    QDateTime getLastReportTime();
    void saveLastRerportTime(const QDateTime& date);
    // 获取崩溃上报最大条数，默认为50
    int getMaxReportCoredump();

private:
    explicit LogApplicationHelper(QObject *parent = nullptr);

    void init();
    void initAppLog();
    void initOtherLog();
    void initCustomLog();

    void createDesktopFiles();
    void createLogFiles();
    void createTransLogFiles();

    void generateTransName(const QString &path, const QString &name, bool isDeepin, bool isGeneric, bool isName);

    QString getLogFile(const QString &path);

    void loadAppLogConfigsByJson();

    AppLogConfig jsonAppLogConfig(const QString& app);
    bool isJsonAppLogConfigExist(const QString& app);

    void validityJsonLogPath(SubModuleConfig& submodule);

signals:
    void sigValueChanged(const QString &key);

public slots:

private:
    /**
     * @brief m_en_log_map 应用包名-日志路径键值对
     */
    QMap<QString, QString> m_en_log_map;
    /**
     * @brief m_en_trans_map 应用包名-应用显示文本键值对
     */
    QMap<QString, QString> m_en_trans_map;
    /**
     * @brief m_trans_log_map 应用显示文本-日志路径键值对
     */
    QMap<QString, QString> m_trans_log_map;
    /**
     * @brief m_other_log_list 所有其他日志列表，每项包含名称、路径
     */
    QList<QStringList> m_other_log_list;
    /**
     * @brief m_custom_log_list 所有自定义日志列表，每项包含名称、路径
     */
    QList<QStringList> m_custom_log_list;
    /**
     * @brief m_desktop_files 所有符合条件的应用的desktop文件路径
     */
    QStringList m_desktop_files;
    /**
     * @brief m_log_files 所有符合条件的应用的日志文件路径
     */
    QStringList m_log_files;
    /**
     * @brief m_current_system_language 系统语言
     */
    QString m_current_system_language;

    /**
     * @brief m_appLogConfigs 应用日志json配置信息
     */
    AppLogConfigList m_JsonAppLogConfigs;
    /**
     * @brief m_appLogConfigs 应用日志配置信息（包含json配置信息）
     */
    AppLogConfigList m_appLogConfigs;
    /**
     * @brief m_instance 单例用的本类指针的原子性封装
     */
    static std::atomic<LogApplicationHelper *> m_instance;
    /**
     * @brief m_mutex 单例用的锁
     */
    static std::mutex m_mutex;

#ifdef DTKCORE_CLASS_DConfigFile
    //dconfig,自定义日志配置
    Dtk::Core::DConfig *m_pDConfig = nullptr;
#endif
    //gsettings,自定义日志配置
    QGSettings *m_pGSettings = nullptr;
};

#endif  // LOGAPPLICATIONHELPER_H
