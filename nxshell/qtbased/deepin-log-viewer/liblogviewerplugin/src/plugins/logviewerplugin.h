// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGVIEWERPLUGIN_H
#define LOGVIEWERPLUGIN_H

#include "../application/structdef.h"
#include "../application/logfileparser.h"
#include "../application/exportprogressdlg.h"
#include "../plugininterfaces/logviewerplugininterface.h"

#include <QObject>

class LogViewerPlugin : public QObject, public LogViewerPluginInterface
{
    Q_OBJECT

    //Q_PLUGIN_METADATA(IID "com.deepin.logviewer.LogViewerPlugin" FILE "LogViewerPlugin.json")
    Q_PLUGIN_METADATA(IID "com.deepin.logviewer.LogViewerPlugin")
    Q_INTERFACES(LogViewerPluginInterface)
    /**
     * @brief The LOAD_STATE enum 主表部分的显示状态
     */
    enum LOAD_STATE {
        DATA_LOADING = 0, //正在加载
        DATA_COMPLETE, //加载完成
        DATA_LOADING_K, //内核日志正在加载
        DATA_NO_SEARCH_RESULT, //搜索无记录
    };
public:
    explicit LogViewerPlugin();
    ~LogViewerPlugin() override;


    //generateAppFile 获取系统应用日志的数据
    //param path 要获取的某一个应用的日志的日志文件路径
    //param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
    //param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
    //param iSearchStr 搜索关键字,现阶段不用,保留参数
    void generateAppFile(const QString &path, BUTTONID id, PRIORITY lId, const QString &iSearchStr = "") override;

    // exportAppLogFile 导出应用日志
    // param: path 导出路径
    // param: period 周期，枚举类型，具体值对应周期下拉框选项
    // param: Level 级别，级别，具体值对应级别下拉框选项
    // param: appid 自研应用id，用于导出指定应用的日志，若为空，什么都不做，返回false（例：导出相册的日志信息，传入deepin-album即可）
    // return：true 导出成功， false 导出失败
    bool exportAppLogFile(const QString &path, BUTTONID period, PRIORITY level, const QString &appid) override;
signals:
    //应用日志数据读取结束信号
    void sigAppFinished(int index) override;
    //应用日志数据信号
    void sigAppData(int index, QList<LOG_MSG_APPLICATOIN> iDataList) override;
    //应用日志导出完成信号
    void sigExportResult(bool isSuccess);

public slots:
    //应用日志数据读取结束槽
    void slot_appFinished(int index);
    //应用日志数据槽
    void slot_appData(int index, QList<LOG_MSG_APPLICATOIN> list);
    //应用日志导出完成槽
    void slot_exportResult(bool isSuccess);

protected:
    void clearAllFilter();
    void clearAllDatalist();
    void initConnections();
    //筛选应用日志
    QList<LOG_MSG_APPLICATOIN> filterApp(const QString &iSearchStr, const QList<LOG_MSG_APPLICATOIN> &iList);
    /**
     * @brief getAppName 获取当前选择的应用的日志路径对应的日志名称
     * @param filePath  当前选择的应用的日志路径
     * @return 对应的日志名称
     */
    QString getAppName(const QString &filePath);
    //导出日志
    //path:导出路径
    void exportLogFile(const QString &path);

private:
    /**
     * @brief m_treeView 主数据表控件件
     */
    //LogTreeView *m_treeView;
    /**
     * @brief m_pModel 主数据表的model
     */
    //QStandardItemModel *m_pModel;

    //分割布局
    //Dtk::Widget::DSplitter *m_splitter;

    //详情页控件
    //logDetailInfoWidget *m_detailWgt {nullptr};
    //搜索无结果时显示无搜索结果提示的label
    //Dtk::Widget::DLabel *noResultLabel {nullptr};
    //当前选中的日志类型的index
    //QModelIndex m_curListIdx;
    //日志等级的显示文本和代码内文本的转换map
    QMap<QString, QString> m_transDict;
    //分页加载时,当前加载到的页数
    int m_limitTag {0};
    /**
     * @brief m_spinnerWgt 加载数据时转轮控件
     */
    //LogSpinnerWidget *m_spinnerWgt;
    /**
     * @brief m_spinnerWgt_K 加载内核日志数据时转轮控件
     */
    //LogSpinnerWidget *m_spinnerWgt_K; // add by Airy
    /**
     * @brief m_curAppLog 当前选中的应用的日志文件路径
     */
    QString m_curAppLog;

    QString m_currentStatus;

    //当前选中的时间筛选选项
    int m_curBtnId {ALL};
    //当前选中的等级筛选选项
    int m_curLevel {INF};

    //当前加载的日志类型
    LOG_FLAG m_flag {NONE};
    /**
     * @brief m_logFileParse 获取日志工具类对象
     */
    LogFileParser m_logFileParse;

    /**
     * @brief jBootList 经过筛选完成的启动日志列表
     */
    /**
     * @brief jBootListOrigin 未经过筛选的启动日志数据 journalctl --boot cmd.
     */
    QList<LOG_MSG_JOURNAL> jBootList, jBootListOrigin;

    /**
     * @brief jList 经过筛选完成的系统日志数据
     */
    /**
     * @brief jListOrigin 未经过筛选的系统日志数据 journalctl cmd.
     */
    QList<LOG_MSG_JOURNAL> jList, jListOrigin;
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
     * @brief appList 经过筛选完成的内核日志数据
     */
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
    /**
     * @brief m_iconPrefix 图标资源文件路径前缀
     */
    QString m_iconPrefix = ICONPREFIX;
    /**
     * @brief m_icon_name_map 日志等级对应图标资源文件名的map
     */
    QMap<QString, QString> m_icon_name_map;

    //当前搜索关键字
    QString m_currentSearchStr {""};
    /**
     * @brief m_currentKwinFilter kwin日志当前筛选条件
     */
    KWIN_FILTERS m_currentKwinFilter;
    //导出进度条弹框
    ExportProgressDlg *m_exportDlg {nullptr};
    //是否为第一次加载完成后收到数据,journalctl获取方式专用
    bool m_firstLoadPageData = false;
    //启动日志当前筛选条件
    BOOT_FILTERS m_bootFilter = {"", ""};
    /**
     * @brief m_normalFilter 开关机日志当前筛选条件
     */
    NORMAL_FILTERS m_normalFilter;
    //上次treeview滚筒条的值
    int m_treeViewLastScrollValue = -1;
    //当前的显示加载状态
    //DisplayContent::LOAD_STATE m_state;
    //系统日志上次获取的时间
    QDateTime m_lastJournalGetTime {QDateTime::fromTime_t(0)};
    /**
     * @brief m_journalFilter 当前系统日志筛选条件
     */
    JOURNAL_FILTERS m_journalFilter;
    QList<LOG_MSG_DNF> dnfList, dnfListOrigin; //dnf.log
    QList<LOG_MSG_DMESG> dmesgList, dmesgListOrigin; //dmesg cmd
    QMap<QString, QString> m_dnfIconNameMap;
    DNFPRIORITY m_curDnfLevel {INFO};
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
    bool m_isDataLoadComplete {false};
    //筛选条件
    QString selectFilter;
};

#endif // LOGVIEWERPLUGIN_H
