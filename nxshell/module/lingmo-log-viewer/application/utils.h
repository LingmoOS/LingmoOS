// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H
#define SINGLE_READ_CNT 500
#define SINGLE_READ_CNT_COREDUMP 600

#include "dtkcore_config.h"
#ifdef DTKCORE_CLASS_DConfig
#include <DConfig>
#endif

#include <QObject>
#include <QHash>

struct LOG_REPEAT_COREDUMP_INFO {
    QString exePath = "";
    int times = 1; // 出现次数
    float repetitionRate = 0.0;
};

/**
 * @brief 公用工具静态函数类
 */
class Utils : public QObject
{
    Q_OBJECT

public:
    explicit Utils(QObject *parent = nullptr);
    ~Utils();
    /**
     * @brief The CommandErrorType enum
     * QProccess命令返回日志的过程中如果发生报错的类型
     */
    enum CommandErrorType {
        NoError = 0x0000, //无错误，预备，一般不用
        PermissionError = 0x0001, //权限问题
        RetryError = 0x0002 //返回请重试的情况
    };
    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;
    static QMap<QString, QStringList> m_mapAuditType2EventType;

    static QString getQssContent(const QString &filePath);
    static QString getConfigPath();
    static QString getAppDataPath();
    static bool isFontMimeType(const QString &filePath);
    static bool isTextFileType(const QString &filePath);
    static QString suffixList();
    static QPixmap renderSVG(const QString &filePath, const QSize &size);
    static QString loadFontFamilyFromFiles(const QString &fontFileName);
    static QString getReplaceColorStr(const char *d);
    static QByteArray replaceEmptyByteArray(const QByteArray &iReplaceStr);
    static CommandErrorType isErroCommand(const QString &str);
    static bool checkAndDeleteDir(const QString &iFilePath);
    static bool deleteDir(const QString &iFilePath);
    static void replaceColorfulFont(QString *iStr);
    static bool isWayland();
    static bool sleep(unsigned int msec);
    //创建多级文件夹
    static QString mkMutiDir(const QString &path);
    //授权框
    static bool checkAuthorization(const QString &actionId, qint64 applicationPid);
    //系统版本号
    static QString osVersion();
    static QString auditType(const QString& eventType);
    static double convertToMB(quint64 cap, const int size = 1024);
    static QString getUserNamebyUID(uint uid);  //根据uid获取用户名
    static QString getUserHomePathByUID(uint uid); //根据uid获取用户家目录
    static QString getCurrentUserName();
    static bool isValidUserName(const QString &userName);
    static bool isCoredumpctlExist();  // is coredumpctl installed
    static QString getHomePath(const QString &userName = "");
    static QString appName(const QString &filePath);
    static void resetToNormalAuth(const QString &path);
    // 统计所有崩溃记录重复次数
    static QList<LOG_REPEAT_COREDUMP_INFO> countRepeatCoredumps(qint64 timeBegin = -1, qint64 timeEnd = -1);
    // 获取高频重复崩溃记录exe路径名单
    static QStringList getRepeatCoredumpExePaths();
    // 更新高频重复崩溃记录exe路径到文件
    static void updateRepeatCoredumpExePaths(const QList<LOG_REPEAT_COREDUMP_INFO> &infos = QList<LOG_REPEAT_COREDUMP_INFO>());
    // 执行cmd命令
    static QByteArray executeCmd(const QString& cmd, const QStringList& args = QStringList(), const QString& workPath = QString());
    /**
     * @brief specialComType 是否是特殊机型，like huawei
     * 取值有3种（-1,0,>0），默认为-1（未知），0（不是特殊机型）,>0（特殊机型）
     */
    static int specialComType;
    static QString homePath;
    static bool runInCmd;
};

#ifdef DTKCORE_CLASS_DConfigFile
class LoggerRules : public QObject {
  Q_OBJECT
public:
  explicit LoggerRules(QObject *parent = nullptr);
  ~LoggerRules();

  void initLoggerRules();
  inline QString rules() const { return m_rules; }
  void setRules(const QString &rules);

private:
  void appendRules(const QString &rules);

private:
  QString m_rules;
  Dtk::Core::DConfig *m_config;
};
#endif

#endif
