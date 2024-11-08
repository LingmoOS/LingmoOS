/*
 * Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmo-logconfigurator.h"
#include "log4qt/logger.h"
#include "log4qt/basicconfigurator.h"
#include "log4qt/level.h"
#include "log4qt/log4qt.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"
#include "log4qt/helpers/initialisationhelper.h"

#include <QDebug>
#include <QStandardPaths>
#include <unistd.h>


LOG4QT_GLOBAL_STATIC(QMutex, single_config)
LingmoUILog4qtConfig *LingmoUILog4qtConfig::mInstance = nullptr;
pid_t g_MainProcPid = -1;

LingmoUILog4qtConfig::LingmoUILog4qtConfig(QObject *parent) : 
    QObject(parent),
    m_threadCheckFile(nullptr),
    m_isInited(false),
    m_gsLog4qtGlobal(nullptr),
    m_gsLog4qtSpecial(nullptr)
{
    m_log4qtProperties.clear();
    m_uMaxFileCount = 0;
    m_uMaxFileSize = 0;
    m_timeCheckFile = 60;
    g_MainProcPid = getpid();
}

LingmoUILog4qtConfig::~LingmoUILog4qtConfig()
{
    if (m_gsLog4qtGlobal) {
        delete m_gsLog4qtGlobal;
        m_gsLog4qtGlobal = nullptr;
    }
    if (m_gsLog4qtSpecial) {
        delete m_gsLog4qtSpecial;
        m_gsLog4qtSpecial = nullptr;
    }
}

LingmoUILog4qtConfig *LingmoUILog4qtConfig::instance()
{
    if (!mInstance)
    {
        QMutexLocker locker(single_config()); 
        if (!mInstance)
        {
            mInstance = new LingmoUILog4qtConfig;
        }
    }
    return mInstance;
}

int LingmoUILog4qtConfig::init(QString strAppName)
{
    QMutexLocker locker(single_config());
    if (m_isInited) {
        return true;
    }
    //全局设置 
    m_log4qtProperties.clear();
    //是否重置所有配置，恢复全局设置默认值
    m_log4qtProperties.setProperty(LINGMOLOG4QT_RESET, "true");
    //设置日志sdk内部记录器级别
    m_log4qtProperties.setProperty(LINGMOLOG4QT_DEBUG, "INFO");
    //日志记录器仓库的阈值
    const QLatin1String key_threshold("log4j.threshold");
    m_log4qtProperties.setProperty(key_threshold, "NULL");
    //设置是否监听QDebug输出的字符串
    m_log4qtProperties.setProperty(LINGMOLOG4QT_HANDLEQT, "true");

    //设置根Logger的输出log等级和输出目的地
    m_log4qtProperties.setProperty(LINGMOLOG4QT_ROOTLOGGER, "DEBUG,daily");

    //设置终端打印记录器
    //记录器类别 --控制台输出
    const QLatin1String key_appender_console("log4j.appender.console");
    m_log4qtProperties.setProperty(key_appender_console, "org.apache.log4j.ConsoleAppender");
    //记录器输出目标
    const QLatin1String key_appender_console_target("log4j.appender.console.target");
    m_log4qtProperties.setProperty(key_appender_console_target, "STDOUT_TARGET");
    //记录器输出布局
    const QLatin1String key_appender_console_layout("log4j.appender.console.layout");
    m_log4qtProperties.setProperty(key_appender_console_layout, "org.apache.log4j.TTCCLayout");
    //记录器布局日期格式
    const QLatin1String key_appender_console_layout_dateformat("log4j.appender.console.layout.dateFormat");
    m_log4qtProperties.setProperty(key_appender_console_layout_dateformat, "yyy-MM-dd hh:mm:ss.zzz");
    //记录器布局包含上下文
    const QLatin1String key_appender_console_layout_contextprinting("log4j.appender.console.layout.contextPrinting");
    m_log4qtProperties.setProperty(key_appender_console_layout_contextprinting, "false");
    //记录器布局包含当前类目
    const QLatin1String key_appender_console_layout_categoryprefixing("log4j.appender.console.layout.categoryPrefixing");
    m_log4qtProperties.setProperty(key_appender_console_layout_categoryprefixing, "false");
    

    //设置定期文件滚动打印记录器
    //记录器类别
    const QLatin1String key_appender_daily("log4j.appender.daily");
    m_log4qtProperties.setProperty(key_appender_daily, "org.apache.log4j.DailyRollingFileAppender");
    //记录器输出文件路径
    const QLatin1String key_appender_daily_file("log4j.appender.daily.file");
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homePath.size() <= 0) {
        return -1;
    }
    QString strLogFilePath = homePath[0]+LINGMOLOG4QT_ROOTPATH+strAppName+".log";
    m_log4qtProperties.setProperty(key_appender_daily_file, strLogFilePath);
    //记录器追加文件内容
    const QLatin1String key_appender_daily_appendfile("log4j.appender.daily.appendFile");
    m_log4qtProperties.setProperty(key_appender_daily_appendfile, "true");
    //记录器是否直接写入文件
    const QLatin1String key_appender_daily_immediateflush("log4j.appender.daily.immediateFlush");
    m_log4qtProperties.setProperty(key_appender_daily_immediateflush, "true");
    //记录器文件名日期后缀 按天
    m_log4qtProperties.setProperty(LINGMOLOG4QT_DAILY_DATEPATTERN, ".yyyy-MM-dd");
    //记录器布局分隔符
    const QLatin1String key_appender_daily_layout("log4j.appender.daily.layout");
    m_log4qtProperties.setProperty(key_appender_daily_layout, "org.apache.log4j.PatternLayout");
    //记录器日志输出格式
    m_log4qtProperties.setProperty(LINGMOLOG4QT_DAILY_CONVERSIONPATTERN, "%d{yyyy-MM-dd HH:mm:ss,zzz}(%-4r)[%t]|%-5p| - %m%n");

    // 从gsettings获取属性值
    initSettings(strAppName, m_log4qtProperties);

    // 根据配置属性初始化rootlogger
    if (!Log4Qt::PropertyConfigurator::configure(m_log4qtProperties)) {
        return -2;
    }
    // 注册程序销毁回调函数
    atexit(shutdown);
    // 启动检查日志文件数量和大小线程
    m_threadCheckFile = new LingmoUILog4qtRolling(strLogFilePath, m_uMaxFileCount, m_uMaxFileSize, m_timeCheckFile);
    m_threadCheckFile->start();
    // 更新已初始化状态
    m_isInited = true;
    return 0;
}

void LingmoUILog4qtConfig::shutdown()
{
    QMutexLocker locker(single_config()); 
    if (mInstance) {
        if (mInstance->m_threadCheckFile) {
            mInstance->m_threadCheckFile->stop();
            mInstance->m_threadCheckFile->wait();
            delete mInstance->m_threadCheckFile;
            mInstance->m_threadCheckFile = nullptr;
        }
        delete mInstance;
        mInstance = nullptr;
    }
}

void LingmoUILog4qtConfig::initSettings(QString strAppName, Log4Qt::Properties &properties)
{
    //读取全局gsettings配置
    const QByteArray gid(LINGMOLOG4QT_SETTINGS);
    if(QGSettings::isSchemaInstalled(gid))
    {
        m_gsLog4qtGlobal = new QGSettings(gid);
        connect(m_gsLog4qtGlobal,&QGSettings::changed,[=](QString key)
        {
            QMutexLocker locker(single_config());
            QVariant gValue;
            bool bLogConfigChange = false;
            bool bLogRollingChange = false;
            if (LINGMOLOG4QT_SETTINGS_RESET == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_RESET);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_RESET, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_DEBUG == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_DEBUG);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_DEBUG, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_HANDLEQT == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_HANDLEQT);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_HANDLEQT, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_ROOTLOGGER == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROOTLOGGER);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_ROOTLOGGER, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_DAILY_DATEPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            }
            if (bLogConfigChange) {
                Log4Qt::PropertyConfigurator::configure(m_log4qtProperties);
            }
            if (LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
                if (gValue.isValid()) {
                    m_timeCheckFile = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
                if (gValue.isValid()) {
                    m_uMaxFileCount = gValue.toUInt();
                    bLogRollingChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE == key) {
                gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
                if (gValue.isValid()) {
                    m_uMaxFileSize = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            }
            if (bLogRollingChange && m_threadCheckFile) {
                m_threadCheckFile->setFileCheckLimit(m_uMaxFileCount, m_uMaxFileSize, m_timeCheckFile);
            }
        });
    }
    if (m_gsLog4qtGlobal) {
        QVariant gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_RESET);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_RESET, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_DEBUG);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_DEBUG, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_HANDLEQT);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_HANDLEQT, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROOTLOGGER);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_ROOTLOGGER, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_DAILY_DATEPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
        if (gValue.isValid()) {
            m_timeCheckFile = gValue.toULongLong();
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
        if (gValue.isValid()) {
            m_uMaxFileCount = gValue.toUInt();
        }
        gValue = m_gsLog4qtGlobal->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
        if (gValue.isValid()) {
            m_uMaxFileSize = gValue.toULongLong();
        }
    }
    //读取特例gsettings配置
    QString strSpecialSetting = QString("%1-%2").arg(LINGMOLOG4QT_SETTINGS).arg(strAppName);
    const QByteArray sid = strSpecialSetting.toUtf8();
    if(QGSettings::isSchemaInstalled(sid))
    {
        m_gsLog4qtSpecial = new QGSettings(sid);
        connect(m_gsLog4qtSpecial,&QGSettings::changed,[=](QString key)
        {
            QMutexLocker locker(single_config());
            QVariant gValue;
            bool bLogConfigChange = false;
            bool bLogRollingChange = false;
            if (LINGMOLOG4QT_SETTINGS_HANDLEQT == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_HANDLEQT);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_HANDLEQT, gValue.toString());
                    bLogConfigChange = true;
                }
            }else if (LINGMOLOG4QT_SETTINGS_ROOTLOGGER == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROOTLOGGER);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_ROOTLOGGER, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_DAILY_DATEPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(LINGMOLOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            }
            if (bLogConfigChange) {
                Log4Qt::PropertyConfigurator::configure(m_log4qtProperties);
            }
            if (LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
                if (gValue.isValid()) {
                    m_timeCheckFile = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
                if (gValue.isValid()) {
                    m_uMaxFileCount = gValue.toUInt();
                    bLogRollingChange = true;
                }
            } else if (LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE == key) {
                gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
                if (gValue.isValid()) {
                    m_uMaxFileSize = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            }
            if (bLogRollingChange && m_threadCheckFile) {
                m_threadCheckFile->setFileCheckLimit(m_uMaxFileCount, m_uMaxFileSize, m_timeCheckFile);
            }
        });
    }
    if (m_gsLog4qtSpecial) {
        QVariant gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROOTLOGGER);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_ROOTLOGGER, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_HANDLEQT);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_HANDLEQT, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_DAILY_DATEPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(LINGMOLOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
        if (gValue.isValid()) {
            m_timeCheckFile = gValue.toULongLong();
        }
        gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
        if (gValue.isValid()) {
            m_uMaxFileCount = gValue.toUInt();
        }
        gValue = m_gsLog4qtSpecial->get(LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
        if (gValue.isValid()) {
            m_uMaxFileSize = gValue.toULongLong();
        }
    }
}