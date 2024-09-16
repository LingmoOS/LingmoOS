// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STRUCTDEF_H
#define STRUCTDEF_H
#include <QString>
#include <QDir>
#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>

#include "utils.h"
#define DPKG_TABLE_DATA "dpkgItemData"
#define XORG_TABLE_DATA "XorgItemData"
#define BOOT_TABLE_DATA "bootItemData"
#define KERN_TABLE_DATA "kernItemData"
#define JOUR_TABLE_DATA "journalItemData"
#define BOOT_KLU_TABLE_DATA "bootKluItemData"
#define APP_TABLE_DATA "applicationItemData"
#define LAST_TABLE_DATA "lastItemData"  // add by Airy
#define KWIN_TABLE_DATA "kwinItemData"
#define DMESG_TABLE_DATA "dmesgItemData"
#define DNF_TABLE_DATA "dnfItemData"
#define OOC_TABLE_DATA "OOCItemData"
#define AUDIT_TABLE_DATA "auditItemData"
#define COREDUMP_TABLE_DATA "coredumpItemData"

#define JOUR_TREE_DATA "journalctl"
#define BOOT_KLU_TREE_DATA "bootklu"
#define DPKG_TREE_DATA "/var/log/dpkg.log"
#define XORG_TREE_DATA "/var/log/Xorg.0.log"
#define KWIN_TREE_DATA Utils::homePath + "/.kwin.log"
#define BOOT_TREE_DATA "/var/log/boot.log"
#define KERN_TREE_DATA "/var/log/kern.log"
#define APP_TREE_DATA "application"
#define LAST_TREE_DATA "last"  // add by Airy
#define DNF_TREE_DATA "/var/log/dnf.log"
#define DMESG_TREE_DATA "dmesg"
#define OTHER_TREE_DATA "other log"
#define CUSTOM_TREE_DATA "custom log"
#define AUDIT_TREE_DATA "/var/log/audit/audit.log"
#define COREDUMP_TREE_DATA "coredump log"

#define ITEM_DATE_ROLE (Qt::UserRole + 66)
#define ICONPREFIX "://images/"
#define ICONLIGHTPREFIX "://images/light/"
#define ICONDARKPREFIX "://images/dark/"
#define DOCTEMPLATE "://doc_template/template.doc"

#define TYPE_SYSTEM "system"
#define TYPE_KERNEL "kernel"
#define TYPE_BOOT "boot"
#define TYPE_DPKG "dpkg"
#define TYPE_DNF "dnf"
#define TYPE_KWIN "kwin"
#define TYPE_XORG "xorg"
#define TYPE_APP "app"
#define TYPE_COREDUMP "coredump"
#define TYPE_BSE "boot-shutdown-event"
#define TYPE_OTHER "other"
#define TYPE_CUSTOM "custom"
#define TYPE_AUDIT "audit"

#define AUDIT_ORIGIN_DATAROLE Qt::UserRole + 3

enum PRIORITY { LVALL = -1,
                EMER,
                ALERT,
                CRI,
                ERR,
                WARN,
                NOTICE,
                INF,
                DEB };
enum DNFPRIORITY { DNFINVALID = -2,
                   DNFLVALL,
                   TRACE,
                   SUBDEBUG,
                   DDEBUG,
                   DEBUG,
                   INFO,
                   WARNING,
                   ERROR,
                   CRITICAL,
                   SUPERCRITICAL };
Q_DECLARE_METATYPE(DNFPRIORITY)

enum AUDITTYPE { AUDITLVALL= -1,
                 IDENTAUTH,
                 DAC,
                 MAC,
                 REMOTE,
                 DOCAUDIT,
                 OTHER};
Q_DECLARE_METATYPE(AUDITTYPE)

struct LOG_MSG_JOURNAL {
    // include dateTime level type detailInfo...
    QString dateTime;
    QString hostName;
    QString daemonName;
    QString daemonId;
    QString level;
    QString msg;
};

struct LOG_MSG_DPKG {
    QString dateTime;
    QString action;
    QString msg;
};

struct LOG_MSG_DNF {
    QString dateTime;
    QString level;
    QString msg;
};

struct LOG_MSG_DMESG {
    QString level;
    QString dateTime;
    QString msg;
};
struct LOG_MSG_BOOT {
    QString status;
    QString msg;
};

struct LOG_MSG_APPLICATOIN {
    QString dateTime;
    QString subModule;
    QString level;
    QString src;
    QString msg;
    QString detailInfo;
};

struct LOG_MSG_XORG {
    QString offset;
    QString msg;
};

// add by Airy
struct LOG_MSG_NORMAL {
    QString eventType;
    QString userName;
    QString dateTime;
    QString msg;
};
struct LOG_MSG_KWIN {
    QString msg;
};
struct LOG_FILE_OTHERORCUSTOM {
    QString name;
    QString path;
    QString dateTimeModify;  
};

#define Audit_IdentAuth     "IdentAuth"
#define Audit_DAC           "DAC"
#define Audit_MAC           "MAC"
#define Audit_Remote        "Remote"
#define Audit_DocAudit      "DocAudit"
#define Audit_Other         "Other"

struct LOG_MSG_AUDIT {
    QString auditType;
    QString eventType;
    QString dateTime;
    QString processName;
    QString processId;
    QString status;
    QString msg;
    QString origin;

    bool contains(const QString& searchstr) {
        if (auditType.contains(searchstr, Qt::CaseInsensitive)
                || eventType.contains(searchstr, Qt::CaseInsensitive)
                || dateTime.contains(searchstr, Qt::CaseInsensitive)
                || processName.contains(searchstr, Qt::CaseInsensitive)
                || status.contains(searchstr, Qt::CaseInsensitive)
                || msg.contains(searchstr, Qt::CaseInsensitive))
            return true;

        return false;
    }

    QString auditType2Str(int nAuditType) {
        QString str = "";
        switch (nAuditType) {
        case IDENTAUTH:
            str = Audit_IdentAuth;
            break;
        case DAC:
            str = Audit_DAC;
            break;
        case MAC:
            str = Audit_MAC;
            break;
        case REMOTE:
            str = Audit_Remote;
            break;
        case DOCAUDIT:
            str = Audit_DocAudit;
            break;
        case OTHER:
            str = Audit_Other;
            break;
        default:
            str = "";
            break;
        }

        return str;
    }

    bool filterAuditType(int nAuditType) {
        QString str = auditType2Str(nAuditType);
        if (str.compare(auditType) == 0)
            return true;

        return false;
    }
};

struct LOG_MSG_COREDUMP {
    QString sig;
    QString dateTime;
    QString coreFile;
    QString uid;
    QString userName;
    QString exe;
    QString pid;
    QString storagePath;
    QString stackInfo;
    QString maps;
    QString packgeVersion;
    QString binaryInfo;
    QString appLog;// 存放应用发生崩溃之前产生的应用日志

    QJsonObject toJson() {
        QJsonObject obj;
        obj.insert("sig", sig);
        obj.insert("dateTime", dateTime);
        obj.insert("coreFile", coreFile);
        obj.insert("exe", exe);
        obj.insert("pid", pid);
        obj.insert("storagePath", storagePath);
        obj.insert("stackInfo", stackInfo);
        if (!maps.isEmpty()) {
            obj.insert("maps", maps);
        }
        obj.insert("packageVersion", packgeVersion);
        obj.insert("binaryInfo", binaryInfo);
        if (!appLog.isEmpty())
            obj.insert("appLog", appLog);

        return obj;
    }
};

struct TIME_RANGE {
    qint64 begin = -1;
    qint64 end = -1;
};

//kwin筛选条件，kwin日志只有信息，没有任何可筛选的，但是先放在这，以后统一化
struct KWIN_FILTERS {
    QString msg;
};
struct XORG_FILTERS {
    qint64 timeFilterBegin = -1 ;
    qint64 timeFilterEnd = -1;
};
struct DKPG_FILTERS {
    qint64 timeFilterBegin = -1 ;
    qint64 timeFilterEnd = -1;
};
struct APP_FILTERS {
    qint64 timeFilterBegin = -1 ;
    qint64 timeFilterEnd = -1;
    int lvlFilter;
    QString app;
    QString submodule;
    QString logType = "file";
    QString path;
    QString execPath;
    QString filter;
    QString searchstr = ""; //搜索关键字

    void clear() {
        timeFilterBegin = -1;
        timeFilterEnd = -1;
        lvlFilter = -1;
        app = "";
        logType = "file";
        path = "";
        execPath = "";
        filter = "";
        searchstr = "";
    }
};
typedef QList<APP_FILTERS> APP_FILTERSList;
struct JOURNAL_FILTERS {
    int eventTypeFilter = -99;
    int timeFilter = -99;

};
struct DMESG_FILTERS {
    qint64 timeFilter;
    PRIORITY levelFilter;
};
struct DNF_FILTERS {
    qint64 timeFilter;
    DNFPRIORITY levelfilter;
};

/**
 * @brief The NORMAL_FILTERS struct 开关机日志筛选条件
 */
struct NORMAL_FILTERS {
    qint64 timeFilterBegin = -1 ; //筛选开始时间
    qint64 timeFilterEnd = -1; //筛选结束时间
    int eventTypeFilter = 0; //筛选类型, 有 0全部 1登陆 2开机 3关机
    QString searchstr = ""; //搜索关键字

    void clear() {
        timeFilterBegin = -1;
        timeFilterEnd = -1;
        eventTypeFilter = 0;
        searchstr = "";
    }
};

struct KERN_FILTERS {
    qint64 timeFilterBegin = -1 ;
    qint64 timeFilterEnd = -1;
};

struct COREDUMP_FILTERS {
    qint64 timeFilterBegin = -1 ;
    qint64 timeFilterEnd = -1;
};

/**
 * @brief The BOOT_FILTERS struct 启动日志筛选条件
 */
struct BOOT_FILTERS {
    QString searchstr;//搜索关键字
    QString statusFilter; //状态筛选,有 ALL OK failed

};

/**
 * @brief The AUDIT_FILTERS struct 审计日志筛选条件
 */
struct AUDIT_FILTERS {
    qint64 timeFilterBegin = -1 ; //筛选开始时间
    qint64 timeFilterEnd = -1; //筛选结束时间
    int auditTypeFilter = 0; //筛选类型, 有 0全部 1身份鉴别 2自主访问控制 3强制访问控制 4远程连接 5文件审计
    QString searchstr = ""; //搜索关键字

    void clear() {
        timeFilterBegin = -1;
        timeFilterEnd = -1;
        auditTypeFilter = 0;
        searchstr = "";
    }
};

/**
 * @brief The FILTER_CONFIG struct 筛选控件中的筛选情况记录结构体
 */
struct FILTER_CONFIG {
    int levelCbx = INF + 1; //等级筛选的筛选值
    QString appListCbx = ""; //应用项目名称
    int statusCbx = 0; //启动日志状态筛选下拉框的值
    int dateBtn = 0; //时间筛选按钮当前选择筛选按钮对应BUTTONID
    int typeCbx = 0;
    int auditCbx = 0;
    int dnfCbx = 5;
    QMap<QString, QString> app2Submodule; //应用项目名称-子模块名称

};
enum BUTTONID {
    ALL = 0,
    ONE_DAY,
    THREE_DAYS,
    ONE_WEEK,
    ONE_MONTH,
    THREE_MONTHS,
    RESET,
    EXPORT,
    INVALID = 9999
};

enum LOG_FLAG {
    JOURNAL = 0,
    KERN,
    BOOT,
    XORG,
    DPKG,
    APP,
    Normal,
    Kwin,
    BOOT_KLU,
    Dnf,
    Dmesg,
    OtherLog,
    CustomLog,
    Audit,
    COREDUMP,
    NONE = 9999
}; // modified by
Q_DECLARE_METATYPE(LOG_FLAG)

// 定义日志数据基础结构体，若有特殊字段，则从该基础结构体派生
struct LOG_MSG_BASE {
    LOG_FLAG type = NONE; // 所属日志类型
    QString dateTime;
    QString msg;
    QString hostName;
    QString daemonName;
    QString daemonId;
    QString level;

    QJsonObject toJson() {
        QJsonObject obj;
        obj.insert("type", type);
        obj.insert("dateTime", dateTime);
        obj.insert("msg", msg);
        obj.insert("hostName", hostName);
        obj.insert("daemonName", daemonName);
        obj.insert("daemonId", daemonId);
        obj.insert("level", level);

        return obj;
    }

    void fromJson(const QString& data) {
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(data.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            if (document.isObject()) {
                QJsonObject object = document.object();
                if (object.contains("type"))
                    this->type = static_cast<LOG_FLAG>(object.value("type").toInt());
                if (object.contains("dateTime"))
                    this->dateTime = static_cast<QString>(object.value("dateTime").toString());
                if (object.contains("msg"))
                    this->msg = static_cast<QString>(object.value("msg").toString());
                if (object.contains("hostName"))
                    this->hostName = static_cast<QString>(object.value("hostName").toString());
                if (object.contains("daemonName"))
                    this->daemonName = static_cast<QString>(object.value("daemonName").toString());
                if (object.contains("daemonId"))
                    this->daemonId = static_cast<QString>(object.value("daemonId").toString());
                if (object.contains("level"))
                    this->level = static_cast<QString>(object.value("level").toString());
            }
        }
    }
};
Q_DECLARE_METATYPE(LOG_MSG_BASE)

#define SEGEMENT_SIZE 60000

struct LOG_FILTER_BASE {
    LOG_FLAG type = NONE;
    qint64 timeFilterBegin = -1 ;
    qint64 timeFilterEnd = -1;
    QString filePath;
    int segementIndex;
};

Q_DECLARE_METATYPE(LOG_FILTER_BASE)

struct LOG_FILTER_KERN : public LOG_FILTER_BASE {

};

Q_DECLARE_METATYPE(LOG_FILTER_KERN)

namespace Log_Item_SPACE {
enum LogItemDataRole {
    levelRole = Qt::UserRole + 6
};
}
namespace JOURNAL_SPACE {
enum JOURNAL_DISPLAY_COLUMN {
    journalLevelColumn = 0,
    journalDaemonNameColumn,
    journalDateTimeColumn,
    journalMsgColumn,
    journalHostNameColumn,
    journalDaemonIdColumn
};
}
namespace KERN_SPACE {
enum KERN_DISPLAY_COLUMN {
    kernDateTimeColumn = 0,
    kernHostNameColumn,
    kernDaemonNameColumn,
    kernMsgColumn
};
}
namespace DKPG_SPACE {
enum DKPG_DISPLAY_COLUMN {
    dkpgDateTimeColumn = 0,
    dkpgMsgColumn
};
}
namespace XORG_SPACE {
enum XORG_DISPLAY_COLUMN {
    xorgDateTimeColumn = 0,
    xorgMsgColumn
};
}
namespace APP_SPACE {
enum APP_DISPLAY_COLUMN {
    appLevelColumn = 0,
    appDateTimeColumn,
    appSrcColumn,
    appMsgColumn
};
}
namespace NORMAL_SPACE {
enum NORMAL_DISPLAY_COLUMN {
    normalEventTypeColumn = 0,
    normalUserNameColumn,
    normalDateTimeColumn,
    normalMsgColumn
};
}

namespace DNF_SPACE {
enum DNF_DISPLAY_COLUMN {
    dnfLvlColumn = 0,
    dnfDateTimeColumn,
    dnfMsgColumn
};
}
namespace DMESG_SPACE {
enum DMESG_DISPLAY_COLUMN {
    dmesgLevelColumn = 0,
    dmesgDateTimeColumn,
    dmesgMsgColumn
};
}

namespace AUDIT_SPACE {
enum AUDIT_DISPLAY_COLUMN {
    auditEventTypeColumn = 0,
    auditDateTimeColumn,
    auditProcessNameColumn,
    auditStatusColumn,
    auditMsgColumn
};
}

namespace COREDUMP_SPACE {
enum COREDUMP_DISPLAY_COLUMN {
    COREDUMP_SIG_COLUMN = 0,
    COREDUMP_TIME_COLUMN,
    COREDUMP_COREFILE_COLUMN,
    COREDUMP_UNAME_COLUMN,
    COREDUMP_EXE_COLUMN
};
}
struct EXPORTALL_DATA {
    QStringList files; // 日志文件原始路径
    QMap<QString, QStringList> dir2Files; //包含父目录的日志文件
    QMap<QString, QStringList> dir2Cmds; //包含父目录的日志文件
    QStringList commands; // 需要用命令获得的日志
    QString logCategory; // 日志种类

    EXPORTALL_DATA()
        : files(QStringList())
        , commands(QStringList())
        , logCategory("")
    {
        dir2Files.clear();
        dir2Cmds.clear();
    }

    // 统计存入二级目录文件的总数
    int dir2FilesCount() {
        int nCount = 0;
        QMapIterator<QString, QStringList> i(dir2Files);
        while (i.hasNext()) {
            i.next();
            nCount += i.value().count();
        }

        return nCount;
    }

    // 统计存入二级目录文件的命令查询结果总数
    int dir2CmdsCount() {
        int nCount = 0;
        QMapIterator<QString, QStringList> i(dir2Cmds);
        while (i.hasNext()) {
            i.next();
            nCount += i.value().count();
        }

        return nCount;
    }
};

struct SubModuleConfig {
    QString name;
    QString filter;
    QString execPath;
    QString logType; // 日志类型分为file或journal
    QString logPath;

    SubModuleConfig()
        : name("")
        , filter("")
        , execPath("")
        , logType("journal")
        , logPath("")
    {

    }

    QJsonObject toJson() {
        QJsonObject obj;
        obj.insert("name", name);
        obj.insert("filter", filter);
        obj.insert("execPath", execPath);
        obj.insert("logType", logType);
        obj.insert("logPath", logPath);

        return obj;
    }

    bool isValid() {
        return !name.isEmpty();
    }
};
typedef QList<SubModuleConfig> SubModuleConfigList;

// 自研应用日志配置信息
struct AppLogConfig {

    QString name;
    QString transName;
    QString group;
    QString version; // json配置版本号

    bool    visible;

    SubModuleConfigList subModules;

    AppLogConfig()
        : name("")
        , transName("")
        , group("")
        , version("V1.0")
        , visible(true)
    {

    }

    bool contains(const QString& app) {
        return name == app;
    }

    bool isValid() {
        return !name.isEmpty();
    }
};

#define AppLogConfigList QList<AppLogConfig>

#endif  // STRUCTDEF_H
