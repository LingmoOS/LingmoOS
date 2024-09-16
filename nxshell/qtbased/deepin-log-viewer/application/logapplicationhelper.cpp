// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logapplicationhelper.h"
#include "utils.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>
#include <QDateTime>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logAppHelper, "org.deepin.log.viewer.application.helper")
#else
Q_LOGGING_CATEGORY(logAppHelper, "org.deepin.log.viewer.application.helper", QtInfoMsg)
#endif

std::atomic<LogApplicationHelper *> LogApplicationHelper::m_instance;
std::mutex LogApplicationHelper::m_mutex;

// 自研应用日志json配置文件目录
const QString APP_LOG_CONFIG_PATH = "/usr/share/deepin-log-viewer/deepin-log.conf.d";

const QString COREDUMP_REPORT_TIME = "coredumpReportTime";
const QString COREDUMP_REPORT_TIME_GSETTING = "coredumpreporttime";
const QString COREDUMP_REPORT_MAX = "coredumpReportMax";
const QString COREDUMP_REPORT_MAX_GSETTING = "coredumpreportmax";
const QString DCONFIG_APPID = "org.deepin.log.viewer";
const QString GSETTING_APPID = "com.deepin.log.viewer";

// 默认崩溃上报最大条数
#define MAX_COREDUMP_REPORT 50

/**
 * @brief LogApplicationHelper::LogApplicationHelper 构造函数，获取日志文件路径和应用名称
 * @param parent 父对象
 */
LogApplicationHelper::LogApplicationHelper(QObject *parent)
    : QObject(parent)
{
    init();
}

/**
 * @brief LogApplicationHelper::init  初始化数据函数
 */
void LogApplicationHelper::init()
{
    getAppLogConfigs();
    initOtherLog();
    initCustomLog();
}

void LogApplicationHelper::initAppLog()
{
    // get current system language shortname
    m_current_system_language = QLocale::system().name();

    // get desktop & trasnames
    createDesktopFiles();

    // load json config
    loadAppLogConfigsByJson();

    createLogFiles();

    createTransLogFiles();
}

void LogApplicationHelper::initOtherLog()
{
    //配置其他日志文件(可以是目录)
    m_other_log_list.clear();

    QList<QStringList> m_other_log_list_temp;
    m_other_log_list_temp.append(QStringList() << "alternatives.log" << "/var/log/alternatives.log");
    m_other_log_list_temp.append(QStringList() << "history.log" << "/var/log/apt/history.log");
    m_other_log_list_temp.append(QStringList() << "term.log" << "/var/log/apt/term.log");
    m_other_log_list_temp.append(QStringList() << "auth.log" << "/var/log/auth.log");

    m_other_log_list_temp.append(QStringList() << "access_log" << "/var/log/cups/access_log");
    m_other_log_list_temp.append(QStringList() << "error_log" << "/var/log/cups/error_log");
    m_other_log_list_temp.append(QStringList() << "daemon.log" << "/var/log/daemon.log");

    m_other_log_list_temp.append(QStringList() << "debug" << "/var/log/debug");
    m_other_log_list_temp.append(QStringList() << "dde-dconfig-daemon" << "/var/log/deepin/dde-dconfig-daemon");
    m_other_log_list_temp.append(QStringList() << "dde-file-manager-daemon" << "/var/log/deepin/dde-file-manager-daemon");
    m_other_log_list_temp.append(QStringList() << "deepin-boot-maker-service.log" << "/var/log/deepin/deepin-boot-maker-service.log");

    m_other_log_list_temp.append(QStringList() << "deepin-diskmanager-service" << "/var/log/deepin/deepin-diskmanager-service/Log");
    m_other_log_list_temp.append(QStringList() << "deepin-home-appstore-daemon" << "/var/log/deepin/deepin-home-appstore-daemon");
    m_other_log_list_temp.append(QStringList() << "deepin-log-viewer-service" << "/var/log/deepin/deepin-log-viewer-service/Log");
    m_other_log_list_temp.append(QStringList() << "deepin-sync-helper_ERROR.log" << "/var/log/deepin-deepinid-daemon/deepin-sync-helper_ERROR.log");

    m_other_log_list_temp.append(QStringList() << "deepin-sync-helper.log" << "/var/log/deepin-deepinid-daemon/deepin-sync-helper.log");
    m_other_log_list_temp.append(QStringList() << "deepin-defender-system-daemon.log" << "/var/log/deepin-defender-system-daemon.log");
    m_other_log_list_temp.append(QStringList() << "deepin-face.log" << "/var/log/deepin-face.log");
    m_other_log_list_temp.append(QStringList() << "deepin-installer-first-boot.log" << "/var/log/deepin-installer-first-boot.log");

    m_other_log_list_temp.append(QStringList() << "deepin-installer-init.log" << "/var/log/deepin-installer-init.log");
    m_other_log_list_temp.append(QStringList() << "deepin-installer.log" << "/var/log/deepin-installer.log");
    m_other_log_list_temp.append(QStringList() << "deepin-monitornetflow.log" << "/var/log/deepin-monitornetflow.log");
    m_other_log_list_temp.append(QStringList() << "faillog" << "/var/log/faillog");
    m_other_log_list_temp.append(QStringList() << "fontconfig.log" << "/var/log/fontconfig.log");

    m_other_log_list_temp.append(QStringList() << "lastlog" << "/var/log/lastlog");
    m_other_log_list_temp.append(QStringList() << "lightdm.log" << "/var/log/lightdm/lightdm.log");
    m_other_log_list_temp.append(QStringList() << "seat0-greeter.log" << "/var/log/lightdm/seat0-greeter.log");
    m_other_log_list_temp.append(QStringList() << "x-0.log" << "/var/log/lightdm/x-0.log");

    m_other_log_list_temp.append(QStringList() << "x-1.log" << "/var/log/lightdm/x-1.log");
    m_other_log_list_temp.append(QStringList() << "messages" << "/var/log/messages");
    m_other_log_list_temp.append(QStringList() << "udcpg.log" << "/var/log/udcpg/udcpg.log");
    m_other_log_list_temp.append(QStringList() << "uos-applicense-agent.log" << "/var/log/uos/uos-applicense-agent/uos-applicense-agent.log");

    m_other_log_list_temp.append(QStringList() << "uos-deviceh-cmd.log" << "/var/log/uos/uos-license-agent/uos-deviceh-cmd.log");
    m_other_log_list_temp.append(QStringList() << "uos-license-agent" << "/var/log/uos/uos-license-agent");
    m_other_log_list_temp.append(QStringList() << "uos-license-upgrade" << "/var/log/uos/uos-license-upgrade");
    m_other_log_list_temp.append(QStringList() << "user.log" << "/var/log/user.log");
    m_other_log_list_temp.append(QStringList() << "vnetlib" << "/var/log/vnetlib");

    m_other_log_list_temp.append(QStringList() << "wtmp" << "/var/log/wtmp");
    m_other_log_list_temp.append(QStringList() << "dcp_log" << "/var/log/cups/dcp_log");
    m_other_log_list_temp.append(QStringList() << "deepin-graphics-driver-manager-server.log" << "/var/log/deepin-graphics-driver-manager-server.log");
    m_other_log_list_temp.append(QStringList() << "uos-activator-kms.log" << "/var/log/uos/uos-activator-kms/uos-activator-kms.log");

    m_other_log_list_temp.append(QStringList() << "deepin-recovery-gui.log" << "/recovery/deepin-recovery-gui.log");
    m_other_log_list_temp.append(QStringList() << "oem-custom-tool" << "/var/local/oem-custom-tool");
    m_other_log_list_temp.append(QStringList() << "iso-customizer-agent.log" << "/root/.cache/isocustomizer-agent/iso-customizer-agent/iso-customizer-agent.log");
    m_other_log_list_temp.append(QStringList() << "downloader" << "~/.config/uos/downloader/Log");

    m_other_log_list_temp.append(QStringList() << ".kwin.log" << "~/.kwin.log");
    m_other_log_list_temp.append(QStringList() << "audit.log" << "/var/log/audit/audit.log");
    m_other_log_list_temp.append(QStringList() << "udcp-client" << "/var/log/udcp-client");

    //窗管日志和配置文件监视
    m_other_log_list_temp.append(QStringList() << ".kwin_x11.log" << "~/.kwin_x11.log");
    m_other_log_list_temp.append(QStringList() << "kwinrc" << "~/.config/kwinrc");
    m_other_log_list_temp.append(QStringList() << "kglobalshortcutsrc" << "~/.config/kglobalshortcutsrc");
    m_other_log_list_temp.append(QStringList() << "kwinrulesrc" << "~/.config/kwinrulesrc");


    for (QStringList iter : m_other_log_list_temp) {
        QString path = iter.at(1);
        if (path.startsWith("~/"))
            path.replace(0, 1, Utils::homePath);
        m_other_log_list.append(QStringList() << iter.at(0) << path);
    }
}

//初始化自定义日志列表
void LogApplicationHelper::initCustomLog()
{
    m_custom_log_list.clear();

#ifdef DTKCORE_CLASS_DConfigFile
    //初始化DConfig配置
    if (!m_pDConfig) {
        m_pDConfig = Dtk::Core::DConfig::create(DCONFIG_APPID, DCONFIG_APPID, "", this);

        // 判断配置是否有效
        if (!m_pDConfig->isValid()) {
            qCWarning(logAppHelper) << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_pDConfig->name(), m_pDConfig->subpath());
            m_pDConfig->deleteLater();
            m_pDConfig = nullptr;
            return;
        }

        connect(m_pDConfig, &Dtk::Core::DConfig::valueChanged, this, [this](const QString & key) {
            initCustomLog();
            emit this->sigValueChanged(key);
        });
    }

    //读取DConfig配置
    QStringList sList = m_pDConfig->value("customLogFiles").toStringList();
    for (QString iter : sList) {
        QString path = iter;
        if (path.isEmpty())
            continue;

        if (path.startsWith("~/"))
            path.replace(0, 1, Utils::homePath);
        //忽略非文本文件和不存在的文件
        if (!QFile::exists(path) || !Utils::isTextFileType(path)) {
            continue;
        }
        m_custom_log_list.append(QStringList() << QFileInfo(iter).fileName() << path);
    }

    //需要查询是否是特殊机型，例如hw机型
    if(m_pDConfig->keyList().contains("specialComType"))
        Utils::specialComType = m_pDConfig->value("specialComType").toInt();
#endif

    //初始化gsetting配置
    if (!m_pGSettings) {
        if (QGSettings::isSchemaInstalled(GSETTING_APPID.toUtf8())) {
            m_pGSettings = new QGSettings(GSETTING_APPID.toUtf8(), "/com/deepin/log/viewer/");

            //监听key的value是否发生了变化
            connect(m_pGSettings, &QGSettings::changed, this, [ = ](const QString & key) {
                initCustomLog();
                emit this->sigValueChanged(key);
            });

            //读取gsetting配置
            QStringList sList2 = m_pGSettings->get("customlogfiles").toStringList();
            for (QString iter : sList2) {
                QString path = iter;
                if (path.startsWith("~/"))
                    path.replace(0, 1, Utils::homePath);
                //忽略非文本文件和不存在的文件
                if (!QFile::exists(path) || !Utils::isTextFileType(path)) {
                    continue;
                }
                m_custom_log_list.append(QStringList() << QFileInfo(iter).fileName() << path);
            }
        } else {
            qCWarning(logAppHelper) << "cannot find gsettings config file";
        }
    }
}

/**
 * @brief LogApplicationHelper::createDesktopFiles 通过所有符合条件的destop文件获得包名和对应的应用文本
 */
void LogApplicationHelper::createDesktopFiles()
{
    m_desktop_files.clear();
    m_en_trans_map.clear();

    //在该目录下遍历所有desktop文件
    QString path = "/usr/share/applications";
    QDir dir(path);
    if (!dir.exists())
        return;

    QStringList fileInfoList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList tempDesktopFiles;
    QStringList regStr {"deepin-", "dde-", "org.deepin", "com.deepin", "uos-"};
    for (QString desktop : fileInfoList) {
        //需要符合以deepin或者dde开头的应用
        for (auto &it : regStr) {
            if (desktop.contains(it)) {
                tempDesktopFiles.append(desktop);
                break;
            }
        }
    }
    for (QString var : tempDesktopFiles) {
        QString filePath = path + "/" + var;
        QFile fi(filePath);
        if (!fi.open(QIODevice::ReadOnly))
            continue;
        bool isDeepin = false;
        bool isGeneric = false;
        bool isName = false;
        bool canDisplay = true;
        while (!fi.atEnd()) {
            QString lineStr = fi.readLine();
            lineStr.replace("\n", "");

            if (lineStr.startsWith("GenericName", Qt::CaseInsensitive) && !isGeneric) {
                isGeneric = true;
            }
            if (lineStr.startsWith("Name", Qt::CaseInsensitive) && !isName) {
                isName = true;
            }
            if (lineStr.startsWith("Hidden")) {
                QStringList hiddenList = lineStr.split("=", QString::SkipEmptyParts);
                if (hiddenList.value(1, "") == "true") {
                    canDisplay = false;
                }
            }
            QString currentDesktop(qgetenv("XDG_CURRENT_DESKTOP"));
            if (lineStr.startsWith("OnlyShowIn")) {
                QString onlyShowValue = lineStr.split("=", QString::SkipEmptyParts).value(1, "");
                if (onlyShowValue.contains(currentDesktop)) {
                    continue;
                } else {
                    canDisplay = false;
                }
            }

            if (lineStr.startsWith("NotShowIn")) {
                QStringList notShowInList = lineStr.split("=", QString::SkipEmptyParts).value(1, "").split(";", QString::SkipEmptyParts);
                if (std::any_of(notShowInList.begin(), notShowInList.end(), [currentDesktop](const auto & data) {
                return data == currentDesktop;
            })) {
                    canDisplay = false;
                }
            }

            if (!lineStr.contains("X-Deepin-Vendor", Qt::CaseInsensitive)) {
                continue;
            }

            QStringList _x_vendor_list = lineStr.split("=", QString::SkipEmptyParts);
            if (_x_vendor_list.count() != 2) {
                continue;
            }

            QString rval = _x_vendor_list[1];
            if (0 == rval.compare("deepin", Qt::CaseInsensitive)) {
                isDeepin = true;
                break;
            }
        }
        fi.close();
        //转换插入应用包名和应用显示文本到数据结构
        if (canDisplay) {
            m_desktop_files.append(var);
            generateTransName(filePath, var.split(QDir::separator()).last(), isDeepin, isGeneric, isName);
        }
    }
}

/**
 * @brief LogApplicationHelper::createLogFiles 根据找到的符合要求的desktop文件的应用去初始化应用日志文件路径
 */
void LogApplicationHelper::createLogFiles()
{
    m_en_log_map.clear();
    m_log_files.clear();

    QString homePath = Utils::homePath;
    if (homePath.isEmpty()) {
        return;
    }
    QString path = homePath + "/.cache/deepin/";
    QDir appDir(path);
    if (!appDir.exists()) {
        return;
    }

    m_log_files = appDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    for (auto i = 0; i < m_desktop_files.count(); ++i) {
        QString desktopName = m_desktop_files[i].split(QDir::separator()).last();
        QString _name = desktopName.mid(0, desktopName.lastIndexOf("."));
        // 默认在~/.cache/deepin目录下取自研应用日志路径
        for (auto j = 0; j < m_log_files.count(); ++j) {
            //desktop文件名和日志文件名比较，相同则符合条件
            if (_name == m_log_files[j]) {
                QString logPath = path + m_log_files[j];
                m_en_log_map.insert(_name, logPath);
                break;
            }
        }
    }
}

void LogApplicationHelper::createTransLogFiles()
{
    m_trans_log_map.clear();
    QMap<QString, QString>::const_iterator iter = m_en_log_map.constBegin();
    while (iter != m_en_log_map.constEnd()) {
        QString displayName = m_en_trans_map.value(iter.key());
        QString logPath = getLogFile(iter.value());

        // 针对journal日志，使用logPath存储项目名称，便于后续解析流程处理
        if (iter.key() == iter.value())
            logPath = iter.key();

        //排除其他日志
        bool bFind = false;
        for (QStringList iterOther : getOtherLogList()) {
            if (iterOther.at(1) == logPath) {
                bFind = true;
                break;
            }
        }

        if (!bFind) {
            m_trans_log_map.insert(displayName, logPath);
        }
        ++iter;
    }
}

/**
 * @brief LogApplicationHelper::parseField 从desktop文件中提取正确的显示文本
 * @param path desktop文件路径
 * @param name 包名
 * @param isDeepin  是否是deepin应用
 * @param isGeneric 是否有GenericName字段
 * @param isName 是否有Name字段
 */
void LogApplicationHelper::generateTransName(const QString &path, const QString &name, bool isDeepin, bool isGeneric,
                                      bool isName)
{
    Q_UNUSED(isName)
    QFile fi(path);
    if (!fi.open(QIODevice::ReadOnly)) {
        return;
    }
    m_en_trans_map.insert(name.mid(0, name.lastIndexOf(".")), name.mid(0, name.lastIndexOf("."))); // desktop name
    if (name.contains("shutdown")) {
    }
    while (!fi.atEnd()) {
        QString lineStr = fi.readLine();
        lineStr.replace("\n", "");

        if (isDeepin) {
            // GenericName所对应的当前语言的值=》GenericName无任何语言标记的值=》Name对应语言的值
            // =》Name无任何语言标记的值 =》desktop的名称

            if (isGeneric) {
                if (!lineStr.startsWith("GenericName"))
                    continue;
            } else {
                if (!lineStr.startsWith("Name"))
                    continue;
            }

        } else {
            // Name对应语言的值 =》Name无任何语言标记的值 =》desktop的名称
            if (!lineStr.startsWith("Name"))
                continue;
        }

        QStringList gNameList = lineStr.split("=", QString::SkipEmptyParts);
        if (gNameList.count() != 2)
            continue;

        QString leftStr = gNameList[0];
        QString genericName = gNameList[1];
        if (leftStr.split("_").count() == 2) {
            if (leftStr.contains(m_current_system_language)) {
                m_en_trans_map.insert(name.mid(0, name.lastIndexOf(".")), genericName);
                break;
            }
        } else if (leftStr.contains(m_current_system_language.split("_")[0])) {
            m_en_trans_map.insert(name.mid(0, name.lastIndexOf(".")), genericName);
            break;
        } else if (0 == leftStr.compare("GenericName", Qt::CaseInsensitive) || 0 == leftStr.compare("Name", Qt::CaseInsensitive)) {
            m_en_trans_map.insert(name.mid(0, name.lastIndexOf(".")), genericName); // GenericName=xxxx
        }
    }
}

/**
 * @brief LogApplicationHelper::getLogFile 通过日志目录获取日志文件路经
 * @param path 通过日志文件目录
 * @return 日志文件路经
 */
QString LogApplicationHelper::getLogFile(const QString &path)
{
    QString ret;
    QDir subdir(path);
    if (!subdir.exists())
        return ret;

    QStringList logFiles = subdir.entryList(QDir::NoDotAndDotDot | QDir::Files);

    for (int j = 0; j < logFiles.count(); j++) {
        QString fileName = logFiles.at(j);
        if (!fileName.endsWith(".log"))
            continue;
        ret = QString("%1/%2").arg(path).arg(fileName);
        break;
    }
    return ret;
}

void LogApplicationHelper::loadAppLogConfigsByJson()
{
    m_JsonAppLogConfigs.clear();

    QDir dir(APP_LOG_CONFIG_PATH);
    if (!dir.exists()) {
        qCWarning(logAppHelper) << QString("%1 does not exist.").arg(APP_LOG_CONFIG_PATH);
        return;
    }

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.json");
    QFileInfoList fiList = dir.entryInfoList();
    foreach (QFileInfo fi, fiList) {
        QFile file(fi.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly))
            continue;

        QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            if (document.isObject()) {
                QJsonObject object = document.object();

                AppLogConfig logConfig;
                logConfig.name = object.value("name").toString();
                logConfig.transName = m_en_trans_map.value(logConfig.name);
                logConfig.group = object.value("group").toString();
                logConfig.version = object.value("version").toString();
                // 支持解析任意格式的visible字段值
                if (object.contains("visible")) {
                    QJsonValue value = object.value("visible");
                    if (value.isString()) {
                        if (value == "true")
                            logConfig.visible = true;
                        else if (value == "false")
                            logConfig.visible = false;
                        else
                            logConfig.visible = object.value("visible").toString().toInt();
                    }
                    else if (value.isBool())
                        logConfig.visible = object.value("visible").toBool();
                    else
                        logConfig.visible = object.value("visible").toInt();
                }

                // json配置文件兼容性保障
                // 解析初版json，将json中关键字段值放到新版应用配置submodule结构体中
                if (object.contains("logType") || object.contains("logPath") || object.contains("exec")) {
                    SubModuleConfig submodule;
                    submodule.name = logConfig.name;
                    submodule.filter = "";
                    submodule.execPath = object.value("exec").toString();
                    submodule.logType = object.value("logType").toString();
                    submodule.logPath = object.value("logPath").toString();
                    if (submodule.logType == "file") {
                        // 对应用日志的logPath做有效性判断，若无效，则填入默认日志路径~/.cache/deepin
                        validityJsonLogPath(submodule);
                    }
                    logConfig.subModules.append(submodule);
                }

                // 解析新版json中的submodule字段
                if (object.contains("submodules")) {
                    if (object.value("submodules").isArray()) {
                        QJsonArray arr = object.value("submodules").toArray();
                        logConfig.subModules.clear();
                        for (int i = 0; i < arr.size(); ++i) {
                            QJsonObject obj = arr[i].toObject();
                            SubModuleConfig submodule;
                            if (obj.contains("name") && obj.value("name").isString())
                                submodule.name = obj.value("name").toString();
                            if (obj.contains("filter") && obj.value("filter").isString())
                                submodule.filter = obj.value("filter").toString();
                            if (obj.contains("exec") && obj.value("exec").isString())
                                submodule.execPath = obj.value("exec").toString();
                            if (obj.contains("logType") && obj.value("logType").isString())
                                submodule.logType = obj.value("logType").toString();
                            if (obj.contains("logPath") && obj.value("logPath").isString())
                                submodule.logPath = obj.value("logPath").toString();

                            if (submodule.logType == "file") {
                                // 对应用日志的logPath做有效性判断，若无效，则填入默认日志路径~/.cache/deepin
                                validityJsonLogPath(submodule);
                            }
                            logConfig.subModules.append(submodule);
                        }
                    }
                }

                m_JsonAppLogConfigs.push_back(logConfig);
            }
        }
    }
}

AppLogConfig LogApplicationHelper::jsonAppLogConfig(const QString &app)
{
    if (app.isEmpty())
        return AppLogConfig();

    if (m_JsonAppLogConfigs.isEmpty()) {
        loadAppLogConfigsByJson();
    }

    foreach (AppLogConfig config, m_JsonAppLogConfigs) {
        if (config.contains(app)) {
            return config;
        }
    }

    return AppLogConfig();
}

bool LogApplicationHelper::isJsonAppLogConfigExist(const QString &app)
{
    foreach (AppLogConfig config, m_JsonAppLogConfigs) {
        if (config.contains(app)) {
            return true;
        }
    }

    return false;
}

// 对应用日志的logPath做有效性判断
void LogApplicationHelper::validityJsonLogPath(SubModuleConfig &submodule)
{
    if (submodule.logType != "file")
        return;

    QString homePath = Utils::homePath;
    QString path = homePath + "/.cache/deepin/";
    QStringList defaultLogFolders;
    QDir logDir(path);
    if (logDir.exists()) {
        defaultLogFolders = logDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    QString defaultLogPath;
    if (defaultLogFolders.contains(submodule.name)) {
        defaultLogPath = getLogFile(path + submodule.name);
    }

    // 若logPath无效，则填入默认日志路径~/.cache/deepin
    if (!submodule.logPath.isEmpty()) {
        // 转为绝对路径
        if (submodule.logPath.front() == '~')
            submodule.logPath.replace(0, 1, homePath);

        QFileInfo fi(submodule.logPath);
        if (!fi.exists()) {
            if (!defaultLogPath.isEmpty())
                submodule.logPath = defaultLogPath;
        }
    } else if(submodule.logPath.isEmpty()) {
        submodule.logPath = defaultLogPath;
    }
}

//刷新并返回所有显示文本对应的应用日志路径
QMap<QString, QString> LogApplicationHelper::getMap()
{
    initAppLog();

    return m_trans_log_map;
}

// 刷新并返回最新的应用配置信息
AppLogConfigList LogApplicationHelper::getAppLogConfigs()
{
    initAppLog();

    m_appLogConfigs.clear();

    // 根据m_trans_log_map的顺序转换未做json配置的应用信息到appConfig中
    QMap<QString, QString>::const_iterator iter = m_trans_log_map.constBegin();
    while (iter != m_trans_log_map.constEnd()) {
        QString transName = iter.key();
        if (transName.isEmpty()) {
            ++iter;
            continue;
        }

        // 根据翻译名称获取应用项目名称
        QString appName = "";
        QMap<QString, QString>::const_iterator iterApp = m_en_trans_map.constBegin();
        while (iterApp != m_en_trans_map.constEnd()) {
            if (iterApp.value() == transName) {
                appName = iterApp.key();
                break;
            }
            ++iterApp;
        }

        if (isJsonAppLogConfigExist(appName)) {
            // 若该应用已被json配置过，则直接将json配置添加应用配置列表中
            m_appLogConfigs.push_back(jsonAppLogConfig(appName));
        } else {
            // 该应用未被json配置过，则转为json方式存储到应用配置列表中
            AppLogConfig appConfig;
            appConfig.name = appName;
            appConfig.transName = transName;
            appConfig.visible = true;

            SubModuleConfig subModule;
            subModule.name = appName;
            subModule.logType = "file";
            subModule.logPath = getLogFile(m_en_log_map[appName]);

            appConfig.subModules.append(subModule);

            m_appLogConfigs.push_back(appConfig);
        }

        ++iter;
    }

    // 将新增的json配置应用更新到应用配置列表中
    for (auto appConfig : m_JsonAppLogConfigs) {
        if (!isAppLogConfigExist(appConfig.name))
            m_appLogConfigs.push_back(appConfig);
    }

    return m_appLogConfigs;
}

//获取所有其他日志文件列表
QList<QStringList> LogApplicationHelper::getOtherLogList()
{
    return m_other_log_list;
}

//获取所有自定义日志文件列表
QList<QStringList> LogApplicationHelper::getCustomLogList()
{
    return m_custom_log_list;
}

AppLogConfig LogApplicationHelper::appLogConfig(const QString &app)
{
    if (app.isEmpty())
        return AppLogConfig();

    if (m_appLogConfigs.isEmpty()) {
        getAppLogConfigs();
    }

    foreach (AppLogConfig config, m_appLogConfigs) {
        if (config.contains(app)) {
            return config;
        }
    }

    return AppLogConfig();
}

bool LogApplicationHelper::isAppLogConfigExist(const QString &app)
{
    foreach (AppLogConfig config, m_appLogConfigs) {
        if (config.contains(app)) {
            return true;
        }
    }

    return false;
}

bool LogApplicationHelper::isValidAppName(const QString &appName)
{
    if (m_en_log_map.find(appName) != m_en_log_map.end())
        return true;

    return false;
}

QDateTime LogApplicationHelper::getLastReportTime()
{
    QVariant time;

#ifdef DTKCORE_CLASS_DConfigFile
    time = m_pDConfig->value(COREDUMP_REPORT_TIME);
#else
    if (m_pGSettings) {
        time = m_pGSettings->get(COREDUMP_REPORT_TIME_GSETTING);
    }
#endif

    if (time.isValid()) {
        return QDateTime::fromString(time.toString(), "yyyy-MM-dd hh:mm:ss");
    }

    return QDateTime();
}

void LogApplicationHelper::saveLastRerportTime(const QDateTime &date)
{
    QString str = date.toString("yyyy-MM-dd hh:mm:ss");

#ifdef DTKCORE_CLASS_DConfigFile
    m_pDConfig->setValue(COREDUMP_REPORT_TIME, str);
#else
    if (m_pGSettings)
        m_pGSettings->set(COREDUMP_REPORT_TIME_GSETTING, str);
#endif
}

int LogApplicationHelper::getMaxReportCoredump()
{
    QVariant value(MAX_COREDUMP_REPORT);

#ifdef DTKCORE_CLASS_DConfigFile
    value = m_pDConfig->value(COREDUMP_REPORT_MAX);
#else
    if (m_pGSettings) {
        value = m_pGSettings->get(COREDUMP_REPORT_MAX_GSETTING);
    }
#endif

    qCWarning(logAppHelper) << "coredump report max:" << value.toInt();
    return value.toInt();
}

//从应用包名转换为应用显示文本
QString LogApplicationHelper::transName(const QString &str)
{
    return m_en_trans_map.value(str);
}

QString LogApplicationHelper::getPathByAppId(const QString &str)
{
    return m_en_log_map.value(str);
}
