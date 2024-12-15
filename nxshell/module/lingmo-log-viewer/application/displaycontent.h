// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISPLAYCONTENT_H
#define DISPLAYCONTENT_H
#include "filtercontent.h" //add by Airy
#include "logdetailinfowidget.h"
#include "logfileparser.h"
#include "logiconbutton.h"
#include "logspinnerwidget.h"
#include "logtreeview.h"
#include "structdef.h"

#include <DApplicationHelper>
#include <DIconButton>
#include <DLabel>
#include <DSpinner>
#include <DTableView>
#include <DTextBrowser>

#include <QStandardItemModel>
#include <QWidget>
#include <QDateTime>

class ExportProgressDlg;
class LogBackend;
/**
 * @brief The DisplayContent class 主显示数据区域控件,包括数据表格和详情页
 */
class DisplayContent : public Dtk::Widget::DWidget
{
    Q_OBJECT
    /**
     * @brief The LOAD_STATE enum 主表部分的显示状态
     */
    enum LOAD_STATE {
        DATA_LOADING = 0, //正在加载
        DATA_COMPLETE, //加载完成
        DATA_LOADING_K, //内核日志正在加载
        DATA_NO_SEARCH_RESULT, //搜索无记录
        DATA_NOT_AUDIT_ADMIN, // 提示不是审计管理员
        DATA_NO_AUDIT_LOG,    // 提示没有审计日志
        COREDUMPCTL_NOT_INSTALLED, //未安装coredumpctl工具
        DATA_NO_PERMISSION //没有权限查看
    };

    enum OOC_TYPE {
        OOC_OTHER = 0, // 其他日志
        OOC_CUSTOM     // 自定义日志
    };

public:
    explicit DisplayContent(QWidget *parent = nullptr);
    ~DisplayContent();
    LogTreeView *mainLogTableView();

private:
    void initUI();
    void initMap();
    void initTableView();
    void setTableViewData();
    void initConnections();

    // 基于Json数据的建表接口
    void createLogTable(const QList<QString>& list, LOG_FLAG type);
    void insertLogTable(const QList<QString>& list, int start, int end, LOG_FLAG type);
    void parseListToModel(const QList<QString>& list, QStandardItemModel *oPModel, LOG_FLAG type);

    int loadSegementPage(bool bNext = true, bool bReset = true);

    void generateJournalFile(int id, int lId, const QString &iSearchStr = "");
    void createJournalTableStart(const QList<LOG_MSG_JOURNAL> &list);
    void createJournalTableForm();
    void generateDpkgFile(int id, const QString &iSearchStr = "");
    void createDpkgTableStart(const QList<LOG_MSG_DPKG> &list);
    void createDpkgTableForm();

    void generateKernFile(int id, const QString &iSearchStr = "");
    void createKernTableForm();
    void createKernTable(const QList<LOG_MSG_JOURNAL> &list);

    void generateAppFile(const QString &app, int id, int lId, const QString &iSearchStr = "");
    void createAppTableForm();
    void createAppTable(const QList<LOG_MSG_APPLICATOIN> &list);

    void createBootTableForm();
    void createBootTable(const QList<LOG_MSG_BOOT> &list);
    void generateBootFile();

    void createXorgTableForm();
    void createXorgTable(const QList<LOG_MSG_XORG> &list);
    void generateXorgFile(int id); // add by Airy for peroid

    void createKwinTableForm();
    void creatKwinTable(const QList<LOG_MSG_KWIN> &list);
    void generateKwinFile(const KWIN_FILTERS &iFilters);

    void createNormalTableForm();
    void createNormalTable(const QList<LOG_MSG_NORMAL> &list); // add by Airy
    void generateNormalFile(int id); // add by Airy for peroid

    //其他日志或者自定义日志
    void generateOOCFile(const QString &path);
    void generateOOCLogs(const OOC_TYPE &type, const QString &iSearchStr = "");
    void createOOCTableForm();
    void createOOCTable(const QList<LOG_FILE_OTHERORCUSTOM> &list);

    // 审计日志
    void generateAuditFile(int id, int lId, const QString &iSearchStr = "");
    void createAuditTableForm();
    void createAuditTable(const QList<LOG_MSG_AUDIT> &list);

    //coredump log
    void generateCoredumpFile(int id, const QString &iSearchStr = "");
    void createCoredumpTableForm();
    void createCoredumpTable(const QList<LOG_MSG_COREDUMP> &list);

    void insertJournalTable(QList<LOG_MSG_JOURNAL> logList, int start, int end);
    void insertApplicationTable(const QList<LOG_MSG_APPLICATOIN> &list, int start, int end);
    void insertKernTable(const QList<LOG_MSG_JOURNAL> &list, int start,
                         int end); // add by Airy for bug 12263
    void insertDpkgTable(const QList<LOG_MSG_DPKG> &list, int start, int end);
    void insertXorgTable(const QList<LOG_MSG_XORG> &list, int start, int end);
    void insertBootTable(const QList<LOG_MSG_BOOT> &list, int start, int end);
    void insertKwinTable(const QList<LOG_MSG_KWIN> &list, int start, int end);
    void insertNormalTable(const QList<LOG_MSG_NORMAL> &list, int start, int end);
    void insertOOCTable(const QList<LOG_FILE_OTHERORCUSTOM> &list, int start, int end);
    void insertAuditTable(const QList<LOG_MSG_AUDIT> &list, int start, int end);
    void insertCoredumpTable(const QList<LOG_MSG_COREDUMP> &list, int start, int end);
    QString getAppName(const QString &filePath);

    bool isAuthProcessAlive();

    void generateJournalBootFile(int lId, const QString &iSearchStr = "");
    void createJournalBootTableStart(const QList<LOG_MSG_JOURNAL> &list);
    void createJournalBootTableForm();
    void insertJournalBootTable(QList<LOG_MSG_JOURNAL> logList, int start, int end);

    void generateDnfFile(BUTTONID iDate, DNFPRIORITY iLevel);
    void createDnfTable(const QList<LOG_MSG_DNF> &list);

    void generateDmesgFile(BUTTONID iDate, PRIORITY iLevel);
    void createDmesgTable(const QList<LOG_MSG_DMESG> &list);
    void createDnfForm();
    void createDmesgForm();
    void insertDmesgTable(const QList<LOG_MSG_DMESG> &list, int start, int end);
    void insertDnfTable(const QList<LOG_MSG_DNF> &list, int start, int end);

signals:
    void loadMoreInfo();
    /**
     * @brief sigDetailInfo 选中treeview显示详情页信号
     * @param index 当前选中的index
     * @param pModel 当前的model指针
     * @param name 当前应用日志选择的日志名称
     */
    void sigDetailInfo(QModelIndex index, QStandardItemModel *pModel, QString name, const int error = 0);
    /**
     * @brief setExportEnable 是否允许导出信号
     * @param iEnable 是否允许导出
     */
    void setExportEnable(bool iEnable);

    void sigCoredumpDetailInfo(QList<LOG_MSG_COREDUMP> cList);

public slots:
    void slot_valueChanged_dConfig_or_gSetting(const QString &key);
    void slot_requestShowRightMenu(const QPoint &pos);
    void slot_tableItemClicked(const QModelIndex &index);
    void slot_BtnSelected(int btnId, int lId, QModelIndex idx);
    void slot_appLogs(int btnId, const QString &app);

    void slot_logCatelogueClicked(const QModelIndex &index);
    void slot_exportClicked();

    void slot_statusChagned(const QString &status);

    // Json格式的日志数据处理接口
    void slot_parseFinished(LOG_FLAG type, int status);
    void slot_logData(const QList<QString> &list, LOG_FLAG type);
    void slot_clearTable();
    void slot_dpkgFinished();
    void slot_dpkgData(const QList<LOG_MSG_DPKG> &list);
    void slot_XorgFinished();
    void slot_xorgData(const QList<LOG_MSG_XORG> &list);
    void slot_bootFinished();
    void slot_bootData(const QList<LOG_MSG_BOOT> &list);
    void slot_kernFinished();
    void slot_kernData(const QList<LOG_MSG_JOURNAL> &list);
    void slot_kwinFinished();
    void slot_kwinData(const QList<LOG_MSG_KWIN> &list);
    void slot_dnfFinished(const QList<LOG_MSG_DNF> &list);
    void slot_dmesgFinished(const QList<LOG_MSG_DMESG> &list);
    void slot_journalFinished();
    void slot_journalBootFinished();
    void slot_journalBootData(const QList<LOG_MSG_JOURNAL> &list);
    void slot_journalData(const QList<LOG_MSG_JOURNAL> &list);
    void slot_applicationFinished();
    void slot_applicationData(const QList<LOG_MSG_APPLICATOIN> &list);
    void slot_normalFinished();
    void slot_normalData(const QList<LOG_MSG_NORMAL> &list);
    void slot_OOCFinished(int error = 0);
    void slot_OOCData(const QString &data);
    void slot_auditFinished(bool bShowTip = false);
    void slot_auditData(const QList<LOG_MSG_AUDIT> &list);
    void slot_coredumpFinished();
    void slot_coredumpData(const QList<LOG_MSG_COREDUMP> &list, bool newData);

    void slot_logLoadFailed(const QString &iError);
    void slot_vScrollValueChanged(int valuePixel);

    void slot_searchResult(const QString &str);
    void slot_getSubmodule(int tcbx);
    void slot_getLogtype(int tcbx); // add by Airy
    void slot_getAuditType(int tcbx);
    void slot_refreshClicked(const QModelIndex &index); //add by Airy for adding refresh
    void slot_dnfLevel(DNFPRIORITY iLevel);

    //导出前把当前要导出的当前信息的Qlist转换成QStandardItemModel便于导出
    void parseListToModel(const QList<LOG_MSG_DPKG> &iList, QStandardItemModel *oPModel);
    void parseListToModel(const QList<LOG_MSG_BOOT> &iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_APPLICATOIN> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_XORG> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_JOURNAL> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_NORMAL> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_KWIN> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_DNF> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_DMESG> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_FILE_OTHERORCUSTOM> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_AUDIT> iList, QStandardItemModel *oPModel);
    void parseListToModel(QList<LOG_MSG_COREDUMP> iList, QStandardItemModel *oPModel);
    QString getIconByname(const QString &str);
    void setLoadState(LOAD_STATE iState, bool bSearching = false);
    void onExportProgress(int nCur, int nTotal);
    void onExportResult(bool isSuccess);
    void onExportFakeCloseDlg();
    void clearAllDatas();

private:
    void resizeEvent(QResizeEvent *event);

private:
    /**
     * @brief m_treeView 主数据表控件件
     */
    LogTreeView *m_treeView;
    /**
     * @brief m_pModel 主数据表的model
     */
    QStandardItemModel *m_pModel;

    //分割布局
    Dtk::Widget::DSplitter *m_splitter;

    //详情页控件
    logDetailInfoWidget *m_detailWgt {nullptr};
    //搜索无结果时显示无搜索结果提示的label
    Dtk::Widget::DLabel *noResultLabel {nullptr};
    // 不是审计管理员提示的label
    Dtk::Widget::DLabel *notAuditLabel {nullptr};
    Dtk::Widget::DLabel *noCoredumpctlLabel {nullptr};
    Dtk::Widget::DLabel *noPermissionLabel {nullptr};
    //当前选中的日志类型的index
    QModelIndex m_curListIdx;
    //当前选中的treeview的index
    QModelIndex m_curTreeIndex;
    //日志等级的显示文本和代码内文本的转换map
    QMap<QString, QString> m_transDict;
    //分页加载时,当前加载到的页数
    int m_limitTag {0};
    /**
     * @brief m_spinnerWgt 加载数据时转轮控件
     */
    LogSpinnerWidget *m_spinnerWgt;
    /**
     * @brief m_spinnerWgt_K 加载内核日志数据时转轮控件
     */
    LogSpinnerWidget *m_spinnerWgt_K; // add by Airy

    // 列表右键菜单
    QMenu *m_menu{ nullptr };
    QAction *m_act_openForder{ nullptr };
    QAction *m_act_refresh{ nullptr };

    /**
     * @brief m_curAppLog 当前选中的应用的项目名称
     */
    QString m_curApp;

    // 当前选中的时间筛选选项
    int m_curBtnId {ALL};
    // 当前选中的等级筛选选项
    int m_curLevel {INF};
    // 当前选中的审计类型选项
    int m_curAuditType {0};
    // 当前选中的开关机事件类型选项
    int m_curNormalEventType{0};

    //当前加载的日志类型
    LOG_FLAG m_flag {NONE};

    // 日志后端 解析、接收和转发日志数据
    LogBackend* m_pLogBackend;

    /**
     * @brief m_iconPrefix 图标资源文件路径前缀
     */
    QString m_iconPrefix = ICONPREFIX;
    /**
     * @brief m_icon_name_map 日志等级对应图标资源文件名的map
     */
    QMap<QString, QString> m_icon_name_map;

    //导出进度条弹框
    ExportProgressDlg *m_exportDlg {nullptr};
    //是否为第一次加载完成后收到数据,journalctl获取方式专用
    bool m_firstLoadPageData = false;

    //上次treeview滚筒条的值
    int m_treeViewLastScrollValue = -1;
    //当前的显示加载状态
    DisplayContent::LOAD_STATE m_state;
    //系统日志上次获取的时间
    QDateTime m_lastJournalGetTime {QDateTime::fromTime_t(0)};
    /**
     * @brief m_journalFilter 当前系统日志筛选条件
     */
    JOURNAL_FILTERS m_journalFilter;

    QMap<QString, QString> m_dnfIconNameMap;
    DNFPRIORITY m_curDnfLevel {INFO};
    bool m_isDataLoadComplete {false};
    //筛选条件
    QString selectFilter;
};

#endif // DISPLAYCONTENT_H
