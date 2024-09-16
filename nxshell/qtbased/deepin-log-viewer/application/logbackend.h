// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGBACKEND_H
#define LOGBACKEND_H

#include "structdef.h"
#include "logfileparser.h"

#include <QObject>

class LogFileParser;
class LogExportThread;
class LogSegementExportThread;
class LogBackend : public QObject
{
    Q_OBJECT

public:
    enum SessionType
    {
        Unknown = -1,
        View,
        Export,
        Report
    };

    static LogBackend *instance(QObject *parent = nullptr);
    ~LogBackend();
    explicit LogBackend(QObject *parent = nullptr);

    // 设置命令行当前工作目录
    void setCmdWorkDir(const QString &dirPath);

    // 导出全部日志到指定目录
    int exportAllLogs(const QString &outDir = "");

    // 按类型导出日志
    int exportTypeLogs(const QString &outDir, const QString &type = "");

    // 按条件导出日志
    // condition 可能为级别、事件类型、状态、审计类型等条件
    bool exportTypeLogsByCondition(const QString &outDir, const QString &type, const QString &period, const QString &condition = "", const QString &keyword = "");

    // 按应用导出日志
    int exportAppLogs(const QString &outDir, const QString &appName = "");

    // 按条件导出应用日志
    bool exportAppLogsByCondition(const QString &outDir, const QString& appName, const QString &period, const QString &level = "", const QString &submodule = "", const QString &keyword = "");

    // 获取当前系统支持的日志种类
    QStringList getLogTypes();

    QString getOutDirPath() const;

    static LOG_FLAG type2Flag(const QString &type, QString &error);

    bool reportCoredumpInfo();


    // Gui日志数据展示功能相关接口
    // 设置日志类型
    void setFlag(const LOG_FLAG& flag);

    // 清理日志筛选条件
    void clearAllFilter();

    // 清理日志数据缓存
    void clearAllDatalist();

    // 解析日志通用新接口，便于后续扩展和维护
    void parse(LOG_FILTER_BASE& filter);

    // 解析日志相关接口
    void parseByJournal(const QStringList &arg = QStringList());
    void parseByJournalBoot(const QStringList &arg = QStringList());

    void parseByDpkg(const DKPG_FILTERS &iDpkgFilter);

    void parseByXlog(const XORG_FILTERS &iXorgFilter);
    void parseByBoot();
    void parseByKern(const KERN_FILTERS &iKernFilter);
    void parseByApp(const APP_FILTERS &iAPPFilter);
    void parseByDnf(DNF_FILTERS iDnfFilter);
    void parseByDmesg(DMESG_FILTERS iDmesgFilter);
    void parseByNormal(const NORMAL_FILTERS &iNormalFiler);

    void parseByKwin(const KWIN_FILTERS &iKwinfilter);
    void parseByOOC(const QString &path);

    void parseByAudit(const AUDIT_FILTERS &iAuditFilter);

    void parseByCoredump(const COREDUMP_FILTERS &iCoredumpFilter, bool parseMap = false);

    int loadSegementPage(int nSegementIndex, bool bReset = true);
    // 分段加载，向上/向下滚动，计算分段索引 bNext为true，计算向下滚动后的分段索引
    int getNextSegementIndex(LOG_FLAG type, bool bNext = true);

    void exportLogData(const QString &filePath, const QStringList &strLabels = QStringList());
    void segementExport();
    void stopExportFromUI();

    // 日志过滤相关接口
    static QList<QString> filterLog(const QString &iSearchStr, const QList<QString> &iList);
    static QList<LOG_MSG_BOOT> filterBoot(BOOT_FILTERS ibootFilter, const QList<LOG_MSG_BOOT> &iList);
    static QList<LOG_MSG_NORMAL> filterNomal(NORMAL_FILTERS inormalFilter, const QList<LOG_MSG_NORMAL> &iList);
    static QList<LOG_MSG_DPKG> filterDpkg(const QString &iSearchStr, const QList<LOG_MSG_DPKG> &iList);
    static QList<LOG_MSG_JOURNAL> filterKern(const QString &iSearchStr, const QList<LOG_MSG_JOURNAL> &iList);
    static QList<LOG_MSG_XORG> filterXorg(const QString &iSearchStr, const QList<LOG_MSG_XORG> &iList);
    static QList<LOG_MSG_KWIN> filterKwin(const QString &iSearchStr, const QList<LOG_MSG_KWIN> &iList);
    static QList<LOG_MSG_APPLICATOIN> filterApp(const QString &iSearchStr, const QList<LOG_MSG_APPLICATOIN> &iList);
    static QList<LOG_MSG_APPLICATOIN> filterApp(APP_FILTERS appFilter, const QList<LOG_MSG_APPLICATOIN> &iList);
    static QList<LOG_MSG_DNF> filterDnf(const QString &iSearchStr, const QList<LOG_MSG_DNF> &iList);
    static QList<LOG_MSG_DMESG> filterDmesg(const QString &iSearchStr, const QList<LOG_MSG_DMESG> &iList);
    static QList<LOG_MSG_JOURNAL> filterJournal(const QString &iSearchStr, const QList<LOG_MSG_JOURNAL> &iList);
    static QList<LOG_MSG_JOURNAL> filterJournalBoot(const QString &iSearchStr, const QList<LOG_MSG_JOURNAL> &iList);
    static QList<LOG_FILE_OTHERORCUSTOM> filterOOC(const QString &iSearchStr, const QList<LOG_FILE_OTHERORCUSTOM> &iList);
    static QList<LOG_MSG_AUDIT> filterAudit(AUDIT_FILTERS auditFilter, const QList<LOG_MSG_AUDIT> &iList);
    static QList<LOG_MSG_COREDUMP> filterCoredump(const QString &iSearchStr, const QList<LOG_MSG_COREDUMP> &iList);


private:
    static LogBackend *m_staticbackend;

signals:

    // Json格式的日志数据处理接口
    void parseFinished(LOG_FLAG type, int status = 0);
    void logData(const QList<QString>& list, LOG_FLAG type);
    void stopExport();
    void clearTable();
    // 解析器路由信号
    void dpkgFinished();
    void dpkgData(const QList<LOG_MSG_DPKG>&);
    void xlogFinished();
    void xlogData(const QList<LOG_MSG_XORG>&);
    void bootFinished();
    void bootData(const QList<LOG_MSG_BOOT>&);
    void kernFinished();
    void kernData(const QList<LOG_MSG_JOURNAL>&);
    void dnfFinished(const QList<LOG_MSG_DNF>&);
    void dmesgFinished(const QList<LOG_MSG_DMESG>&);
    void journalFinished();
    void journalBootFinished();
    void journalData(const QList<LOG_MSG_JOURNAL>&);
    void journaBootlData(const QList<LOG_MSG_JOURNAL>&);

    //void normalFinished();  // add by Airy

    void normalFinished();
    void normalData(const QList<LOG_MSG_NORMAL>&);
    void kwinFinished();
    void kwinData(const QList<LOG_MSG_KWIN> &iKwinList);
    /**
     * @brief appFinished 获取数据结束信号
     */
    void appFinished();
    void appData(const QList<LOG_MSG_APPLICATOIN> &iDataList);
    void OOCFinished(int error = 0);
    void OOCData(const QString &data);

    void auditFinished(bool bShowTip = false);
    void auditData(const QList<LOG_MSG_AUDIT>&);

    void coredumpFinished();
    void coredumpData(const QList<LOG_MSG_COREDUMP> &iDataList, bool newData = true);

    /**
     * @brief proccessError 获取日志文件失败错误信息传递信号，传递到主界面显示 DMessage tooltip
     * @param iError 错误字符
     */
    void proccessError(const QString &iError);


    // 导出线程路由信号
    /**
     * @brief sigProgress 导出进度信号
     * @param nCur 当前运行到的进度
     * @param nTotal 要运行的总数
     */
    void sigProgress(int nCur, int nTotal);
    /**
     * @brief sigResult 导出完成信号
     * @param isSuccess 是否成功
     */
    void sigResult(bool isSuccess);
    void sigProcessFull();
private slots:
    void slot_parseFinished(int index, LOG_FLAG type, int status);
    void slot_logData(int index, const QList<QString> &list, LOG_FLAG type);
    void slot_dpkgFinished(int index);
    void slot_dpkgData(int index, QList<LOG_MSG_DPKG> list);
    void slot_XorgFinished(int index);
    void slot_xorgData(int index, QList<LOG_MSG_XORG> list);
    void slot_bootFinished(int index);
    void slot_bootData(int index, QList<LOG_MSG_BOOT> list);
    void slot_kernFinished(int index);
    void slot_kernData(int index, QList<LOG_MSG_JOURNAL> list);
    void slot_kwinFinished(int index);
    void slot_kwinData(int index, QList<LOG_MSG_KWIN> list);
    void slot_dnfFinished(const QList<LOG_MSG_DNF> &list);
    void slot_dmesgFinished(const QList<LOG_MSG_DMESG> &list);
    void slot_journalFinished(int index);
    void slot_journalBootFinished(int index);
    void slot_journalBootData(int index, QList<LOG_MSG_JOURNAL> list);
    void slot_journalData(int index, QList<LOG_MSG_JOURNAL> list);
    void slot_applicationFinished(int index);
    void slot_applicationData(int index, QList<LOG_MSG_APPLICATOIN> list);
    void slot_normalFinished(int index);
    void slot_normalData(int index, QList<LOG_MSG_NORMAL> list);
    void slot_OOCFinished(int index, int error = 0);
    void slot_OOCData(int index, const QString & data);
    void slot_auditFinished(int index, bool bShowTip = false);
    void slot_auditData(int index, QList<LOG_MSG_AUDIT> list);
    void slot_coredumpFinished(int index);
    void slot_coredumpData(int index, QList<LOG_MSG_COREDUMP> list);

    void slot_logLoadFailed(const QString &iError);

    void onExportProgress(int nCur, int nTotal);
    void onExportResult(bool isSuccess);
    void onExportFakeCloseDlg();

private:
    void initConnections();

    // cli导出功能相关接口
    bool parseData(const LOG_FLAG &flag, const QString &period, const QString &condition);

    void executeCLIExport(const QString &originFilePath = "");

    void resetCategoryOutputPath(const QString & path);
    bool getOutDirPath(const QString &path);
    BUTTONID period2Enum(const QString &period);
    int level2Id(const QString &level);
    DNFPRIORITY dnfLevel2Id(const QString &level);
    int normal2eventType(const QString &eventType);
    int audit2eventType(const QString &eventType);
    TIME_RANGE getTimeRange(const BUTTONID& periodId);
    QString getApplogPath(const QString &appName);
    QStringList getLabels(const LOG_FLAG &flag);
    bool hasMatchedData(const LOG_FLAG &flag);
    void parseCoredumpDetailInfo(QList<LOG_MSG_COREDUMP>& list);

private:
    QStringList m_logTypes;
    bool m_newDir {false};
    QString m_outPath {""};
    QString m_cmdWorkDir {""};

    QString m_exportFilePath {""};

public:

    // 日志种类-----解析线程index 键值对
    QMap<LOG_FLAG, int> m_type2ThreadIndex;

    // 日志种类-----原始日志数据键值对 --为了统一日志数据存储方式，日志数据按json字串存储
    QMap<LOG_FLAG, QList<QString>> m_type2LogDataOrigin;

    // 日志种类-----筛选后日志数据键值对  --为了统一日志数据存储方式，日志数据按json字串存储
    QMap<LOG_FLAG, QList<QString>> m_type2LogData;

    // 日志种类-----筛选条件
    QMap<LOG_FLAG, LOG_FILTER_BASE> m_type2Filter;

    // 筛选条件
    /**
     * @brief m_journalFilter 当前系统日志筛选条件
     */
    JOURNAL_FILTERS m_journalFilter;
    /**
     * @brief m_normalFilter 开关机日志当前筛选条件
     */
    NORMAL_FILTERS m_normalFilter;
    //启动日志当前筛选条件
    BOOT_FILTERS m_bootFilter = {"", ""};
    // 应用日志当前筛选条件
    APP_FILTERS m_appFilter;
    /**
     * @brief m_auditFilter 当前审计日志筛选条件
     */
    AUDIT_FILTERS m_auditFilter;

    //当前搜索关键字
    QString m_currentSearchStr {""};



    // 日志缓存数据
    /**
     * @brief jBootList 经过筛选完成的启动日志列表
     */
    /**
     * @brief jBootListOrigin 未经过筛选的启动日志数据 journalctl --boot cmd.
     */
    QList<LOG_MSG_JOURNAL> jBootList, jBootListOrigin;

    // System log data
    QList<LOG_MSG_JOURNAL> jList, jListOrigin;
    // Dmesg log data
    QList<LOG_MSG_DMESG> dmesgList, dmesgListOrigin;

    QList<LOG_MSG_DNF> dnfList, dnfListOrigin; //dnf.log
    /**
     * @brief dList 经过筛选完成的dpkg日志数据
     */
    /**
     * @brief dListOrigin 未经过筛选的dpkg日志数据  dpkg.log
     */
    QList<LOG_MSG_DPKG> dList, dListOrigin;
    /**
     * @brief xList 经过筛选完成的xorg日志数据
     */
    /**
     * @brief xListOrigin 未经过筛选的xorg日志数据   Xorg.0.log
     */
    QList<LOG_MSG_XORG> xList, xListOrigin;
    /**
     * @brief currentBootList 经过筛选完成的启动日志数据
     */
    /**
     * @brief bList 未经过筛选的启动日志数据   boot.log
     */
    QList<LOG_MSG_BOOT> bList, currentBootList;
    /**
     * @brief kList 经过筛选完成的内核日志数据
     */
    /**
     * @brief kListOrigin 未经过筛选的内核日志数据   kern.log
     */
    QList<LOG_MSG_JOURNAL> kList, kListOrigin;

    /**
     * @brief oList未经过筛选的其他日志数据   other
     */
    QList<LOG_FILE_OTHERORCUSTOM> oList, oListOrigin;

    /**
     * @brief cList未经过筛选的自定义日志数据   custom
     */
    QList<LOG_FILE_OTHERORCUSTOM> cList, cListOrigin;

    /**
     * @brief aListOrigin 未经过筛选的审计日志数据   audit/audit.log
     */
    QList<LOG_MSG_AUDIT> aList, aListOrigin;

    /**
     * @brief appListOrigin 未经过筛选的内核日志数据   ~/.cache/deepin/xxx.log(.xxx)
     */
    QList<LOG_MSG_APPLICATOIN> appList, appListOrigin;
    /**
     * @brief norList add 未经过筛选完成的开关机日志数据 by Airy
     */
    QList<LOG_MSG_NORMAL> norList;
    /**
     * @brief nortempList 经过筛选的开关机日志数据 add by Airy
     */
    QList<LOG_MSG_NORMAL> nortempList;
    /**
     * @brief m_currentKwinList add 经过筛选完成的kwin日志数据 by Airy /$HOME/.kwin.log
     */
    QList<LOG_MSG_KWIN> m_currentKwinList;
    /**
     * @brief m_kwinList 未经过筛选的开关机日志数据
     */
    QList<LOG_MSG_KWIN> m_kwinList;


    QList<LOG_MSG_COREDUMP> m_coredumpList;
    QList<LOG_MSG_COREDUMP> m_currentCoredumpList;

private:

    /**
     * @brief m_curAppLog 当前选中的应用项目名称
     */
    QString m_curApp {""};

    SessionType m_sessionType { View };
    SessionType m_lastSessionType { Unknown };
    int m_lastSegementIndex { -1 };
    bool m_bSegementExporting {false};
    int m_segementCount { 0 };
    
    //当前解析的日志类型
    LOG_FLAG m_flag {NONE};

    // 分段导出线程，用来导出doc、xls格式的日志
    LogSegementExportThread *m_pSegementExportThread { nullptr };
    // 日志解析器
    LogFileParser m_logFileParser;

    //当前系统日志获取进程标记量
    int m_journalCurrentIndex {-1};
    //当前klu启动日志获取进程标记量
    int m_journalBootCurrentIndex {-1};
    //当前启动日志获取进程标记量
    int m_bootCurrentIndex {-1};
    int m_dpkgCurrentIndex {-1};
    int m_kernCurrentIndex {-1};
    int m_normalCurrentIndex {-1};
    int m_xorgCurrentIndex {-1};
    int m_kwinCurrentIndex {-1};
    int m_appCurrentIndex {-1};
    int m_OOCCurrentIndex {-1};
    int m_auditCurrentIndex {-1};
    int m_coredumpCurrentIndex {-1};

    bool m_isDataLoadComplete {false};
};

#endif // LOGBACKEND_H
