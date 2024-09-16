// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displaycontent.h"
#include "logapplicationhelper.h"
#include "logexportthread.h"
#include "logfileparser.h"
#include "exportprogressdlg.h"
#include "logbackend.h"
#include "utils.h"
#include "DebugTimeManager.h"
#include "parsethread/parsethreadbase.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFileDialog>
#include <DFontSizeManager>
#include <DHorizontalLine>
#include <DSplitter>
#include <DScrollBar>
#include <DStandardItem>
#include <DStandardPaths>
#include <DMessageManager>
#include <DDesktopServices>

#include <QAbstractItemView>
#include <QDebug>
#include <QHeaderView>
#include <QtConcurrent>
#include <QIcon>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <QProgressDialog>
#include <QThread>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QDateTime>
#include <QFileIconProvider>
#include <QMenu>
#include <QLoggingCategory>

#include <sys/utsname.h>
#include "malloc.h"
DWIDGET_USE_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logDisplaycontent, "org.deepin.log.viewer.display.content")
#else
Q_LOGGING_CATEGORY(logDisplaycontent, "org.deepin.log.viewer.display.content", QtInfoMsg)
#endif

#define SINGLE_LOAD 300

#define NAME_WIDTH 470
#define LEVEL_WIDTH 80
#define STATUS_WIDTH 90
#define DATETIME_WIDTH 175
#define DEAMON_WIDTH 100
#define TREE_NORMAL_COUNT 50   //大致填满表格区域的行数
/**
 * @brief DisplayContent::DisplayContent 初始化界面\等级数据和实际显示文字转换的数据结构\信号槽连接
 * @param parent
 */
DisplayContent::DisplayContent(QWidget *parent)
    : DWidget(parent)

{
    m_pLogBackend = LogBackend::instance(this);

    initUI();
    initMap();
    initConnections();
}

/**
 * @brief DisplayContent::~DisplayContent 析构成员指针,释放空间
 */
DisplayContent::~DisplayContent()
{
    malloc_trim(0);
}
/**
 * @brief DisplayContent::mainLogTableView  返回主表控件指针给外部调用
 * @return 主表控件成员指针
 */
LogTreeView *DisplayContent::mainLogTableView()
{
    return m_treeView;
}

/**
 * @brief DisplayContent::initUI 初始化布局及界面
 */
void DisplayContent::initUI()
{
    // set table for display log data
    initTableView();
    m_treeView->setMinimumHeight(100);
    m_treeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    //noResultLabel
    noResultLabel = new DLabel(this);
    DPalette pa = DApplicationHelper::instance()->palette(noResultLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(noResultLabel, pa);
    noResultLabel->setText(DApplication::translate("SearchBar", "No search results"));
    DFontSizeManager::instance()->bind(noResultLabel, DFontSizeManager::T4);
    noResultLabel->setAlignment(Qt::AlignCenter);

    //notAuditLabel
    notAuditLabel = new DLabel(this);
    DApplicationHelper::instance()->setPalette(notAuditLabel, pa);
    notAuditLabel->setText(DApplication::translate("Warning", "Security level for the current system: high\n audit only administrators can view the audit log"));
    DFontSizeManager::instance()->bind(notAuditLabel, DFontSizeManager::T4);
    notAuditLabel->setAlignment(Qt::AlignCenter);

    noCoredumpctlLabel = new DLabel(this);
    DApplicationHelper::instance()->setPalette(noCoredumpctlLabel, pa);
    noCoredumpctlLabel->setText(DApplication::translate("Waring", "Unable to obtain crash information, please install systemd-coredump."));
    DFontSizeManager::instance()->bind(noCoredumpctlLabel, DFontSizeManager::T4);
    noCoredumpctlLabel->setAlignment(Qt::AlignCenter);

    noPermissionLabel = new DLabel(this);
    DApplicationHelper::instance()->setPalette(noPermissionLabel, pa);
    noPermissionLabel->setText(DApplication::translate("Warning", "You do not have permission to view it"));
    DFontSizeManager::instance()->bind(noPermissionLabel, DFontSizeManager::T4);
    noPermissionLabel->setAlignment(Qt::AlignCenter);

    //m_spinnerWgt,m_spinnerWgt_K
    m_spinnerWgt = new LogSpinnerWidget(this);
    m_spinnerWgt->setAccessibleName("spinnerWidget");
    m_spinnerWgt->setMinimumHeight(300);
    m_spinnerWgt_K = new LogSpinnerWidget(this);
    m_spinnerWgt_K->setAccessibleName("spinnerWidget_K");
    m_spinnerWgt_K->setMinimumHeight(300);

    //m_detailWgt
    m_detailWgt = new logDetailInfoWidget(this);
    m_detailWgt->setAccessibleName("detailInfoWidget");
    m_detailWgt->setMinimumHeight(70);
    m_detailWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //m_splitter
    m_splitter = new Dtk::Widget::DSplitter( this);
    m_splitter->setAccessibleName("splitterFrame");
    m_splitter->setOrientation(Qt::Vertical);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_spinnerWgt_K);
    m_splitter->addWidget(m_spinnerWgt);
    m_splitter->addWidget(m_detailWgt);
    m_splitter->setStretchFactor(0, 5);
    m_splitter->setStretchFactor(1, 5);
    m_splitter->setStretchFactor(2, 5);
    m_splitter->setStretchFactor(3, 3);
    m_splitter->setHandleWidth(3);
    //this->setStyleSheet("background-color:rgb(255,0,0)");

    // layout for widgets
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(m_splitter);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(3);
    this->setLayout(vLayout);

    //m_exportDlg
    m_exportDlg = new ExportProgressDlg(this);
    m_exportDlg->setAccessibleName("ExportProgressDlg");
    m_exportDlg->hide();


    m_menu = new QMenu(m_treeView);
    m_menu->setAccessibleName("table_menu");
    m_act_openForder = m_menu->addAction(/*tr("在文件管理器中显示")*/ DApplication::translate("Action", "Display in file manager"));
    m_act_refresh = m_menu->addAction(/*tr("刷新")*/ DApplication::translate("Action", "Refresh"));

    //setLoadState
    setLoadState(DATA_COMPLETE);
}

/**
 * @brief DisplayContent::initMap 初始化等级数据和实际显示文字转换的数据结构
 */
void DisplayContent::initMap()
{
    m_transDict.clear();
    m_transDict.insert("Warning", DApplication::translate("Level", "Warning")); //add by Airy for bug 19167 and 19161
    m_transDict.insert("Debug", DApplication::translate("Level", "Debug")); //add by Airy for bug 19167 and 19161
    m_transDict.insert("Info", DApplication::translate("Level", "Info"));
    m_transDict.insert("Error", DApplication::translate("Level", "Error"));

    // icon <==> level
    m_icon_name_map.clear();
    m_icon_name_map.insert(DApplication::translate("Level", "Emergency"), "warning2.svg");
    m_icon_name_map.insert(DApplication::translate("Level", "Alert"), "warning3.svg");
    m_icon_name_map.insert(DApplication::translate("Level", "Critical"), "warning2.svg");
    m_icon_name_map.insert(DApplication::translate("Level", "Error"), "wrong.svg");
    m_icon_name_map.insert(DApplication::translate("Level", "Warning"), "warning.svg");
    m_icon_name_map.insert(DApplication::translate("Level", "Notice"), "warning.svg");
    m_icon_name_map.insert(DApplication::translate("Level", "Info"), "");
    m_icon_name_map.insert(DApplication::translate("Level", "Debug"), "");
    m_icon_name_map.insert("Warning", "warning.svg");
    m_icon_name_map.insert("Debug", "");
    m_icon_name_map.insert("Error", "wrong.svg");

    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Trace"), "");
    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Debug"), "");
    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Info"), "");
    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Warning"), "warning.svg");
    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Error"), "wrong.svg");
    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Critical"), "warning2.svg");
    m_dnfIconNameMap.insert(Dtk::Widget::DApplication::translate("Level", "Super critical"), "warning3.svg");
}

/**
 * @brief DisplayContent::initTableView 初始化主表控件,设置tablemodel
 */
void DisplayContent::initTableView()
{
    m_treeView = new LogTreeView(this);
    m_treeView->setObjectName("mainLogTable");
    m_treeView->setAccessibleName("mainLogTable");
    m_pModel = new QStandardItemModel(this);
    m_treeView->setModel(m_pModel);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
}

/**
 * @brief DisplayContent::initConnections 初始化槽函数信号连接
 */
void DisplayContent::initConnections()
{
    connect(m_treeView, SIGNAL(pressed(const QModelIndex &)), this,
            SLOT(slot_tableItemClicked(const QModelIndex &)));

    connect(this, &DisplayContent::sigDetailInfo, m_detailWgt, &logDetailInfoWidget::slot_DetailInfo);
    connect(m_pLogBackend, &LogBackend::parseFinished, this, &DisplayContent::slot_parseFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::logData, this, &DisplayContent::slot_logData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::dpkgFinished, this, &DisplayContent::slot_dpkgFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::dpkgData, this, &DisplayContent::slot_dpkgData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::xlogFinished, this, &DisplayContent::slot_XorgFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::xlogData, this, &DisplayContent::slot_xorgData,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::bootFinished, this, &DisplayContent::slot_bootFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::bootData, this, &DisplayContent::slot_bootData,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::kernFinished, this, &DisplayContent::slot_kernFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::kernData, this, &DisplayContent::slot_kernData,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::journalFinished, this, &DisplayContent::slot_journalFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::journalData, this, &DisplayContent::slot_journalData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::journaBootlData, this, &DisplayContent::slot_journalBootData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::appFinished, this,
            &DisplayContent::slot_applicationFinished);
    connect(m_pLogBackend, &LogBackend::appData, this,
            &DisplayContent::slot_applicationData);

    connect(m_pLogBackend, &LogBackend::kwinFinished, this, &DisplayContent::slot_kwinFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::kwinData, this, &DisplayContent::slot_kwinData,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::normalData, this, &DisplayContent::slot_normalData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::normalFinished, this, &DisplayContent::slot_normalFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::journalBootFinished, this, &DisplayContent::slot_journalBootFinished);

    connect(m_treeView->verticalScrollBar(), &QScrollBar::valueChanged, this,
            &DisplayContent::slot_vScrollValueChanged);
    connect(m_pLogBackend, &LogBackend::proccessError, this, &DisplayContent::slot_logLoadFailed,
            Qt::QueuedConnection);
    connect(m_pLogBackend, SIGNAL(dnfFinished(QList<LOG_MSG_DNF>)), this, SLOT(slot_dnfFinished(QList<LOG_MSG_DNF>)));
    connect(m_pLogBackend, &LogBackend::dmesgFinished, this, &DisplayContent::slot_dmesgFinished,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::OOCData, this, &DisplayContent::slot_OOCData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::OOCFinished, this, &DisplayContent::slot_OOCFinished,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::auditData, this, &DisplayContent::slot_auditData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::auditFinished, this, &DisplayContent::slot_auditFinished,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::coredumpData, this, &DisplayContent::slot_coredumpData,
            Qt::QueuedConnection);
    connect(m_pLogBackend, &LogBackend::coredumpFinished, this, &DisplayContent::slot_coredumpFinished,
            Qt::QueuedConnection);

    connect(m_pLogBackend, &LogBackend::clearTable, this, &DisplayContent::slot_clearTable);

    connect(m_pLogBackend, &LogBackend::sigResult, this, &DisplayContent::onExportResult);
    connect(m_pLogBackend, &LogBackend::sigProgress, this, &DisplayContent::onExportProgress);
    connect(m_pLogBackend, &LogBackend::sigProcessFull, this, &DisplayContent::onExportFakeCloseDlg);

    // 取消导出
    connect(m_exportDlg, &ExportProgressDlg::sigCloseBtnClicked, m_pLogBackend, &LogBackend::stopExportFromUI);
    connect(m_exportDlg, &ExportProgressDlg::buttonClicked, m_pLogBackend, &LogBackend::stopExportFromUI);

    connect(m_treeView, &LogTreeView::customContextMenuRequested, this, &DisplayContent::slot_requestShowRightMenu);
    connect(LogApplicationHelper::instance(), &LogApplicationHelper::sigValueChanged, this, &DisplayContent::slot_valueChanged_dConfig_or_gSetting);
}

void DisplayContent::createLogTable(const QList<QString> &list, LOG_FLAG type)
{
    m_limitTag = 0;

    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertLogTable(list, 0, end, type);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::insertLogTable(const QList<QString> &list, int start, int end, LOG_FLAG type)
{
    QList<QString> midList = list;
    if (end > start) {
        midList = midList.mid(start, end - start);
    }

    parseListToModel(midList, m_pModel, type);
}

void DisplayContent::parseListToModel(const QList<QString> &list, QStandardItemModel *oPModel, LOG_FLAG type)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << QString("log parse model is empty, type:%1").arg(type);
        return;
    }

    if (list.isEmpty()) {
        qCWarning(logDisplaycontent) << QString("log parse model data is empty, type:%1").arg(type);
        return;
    }

    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    int listCount = list.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        if (type == KERN) {
            LOG_MSG_BASE data;
            data.fromJson(list[i]);
            item = new DStandardItem(data.dateTime);
            item->setData(KERN_TABLE_DATA);
            items << item;
            item = new DStandardItem(data.hostName);
            item->setData(KERN_TABLE_DATA);
            items << item;
            item = new DStandardItem(data.daemonName);
            item->setData(KERN_TABLE_DATA);
            items << item;
            item = new DStandardItem(data.msg);
            item->setData(KERN_TABLE_DATA);
            items << item;
        } else if (type == Kwin) {
            LOG_MSG_BASE data;
            data.fromJson(list[i]);
            item = new DStandardItem(data.msg);
            item->setData(KWIN_TABLE_DATA);
            item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
            items << item;
        }
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

int DisplayContent::loadSegementPage(bool bNext/* = true*/, bool bReset/* = true*/)
{
    int nSegementIndex = m_pLogBackend->getNextSegementIndex(m_flag, bNext);
    qDebug() << "loadSegementPage index:" << nSegementIndex;
    if(nSegementIndex == -1)
        return -1;

    if (bReset)
        clearAllDatas();

    setLoadState(DATA_LOADING, !bReset);

    // 1.正常分段加载翻页，重置表格
    // 2.搜索结果超过分段单位大小后，需要重置表格，显示下一页内容
    if (bReset || (!bReset &&m_pLogBackend->m_type2LogData[m_flag].size() > SEGEMENT_SIZE)) {
        if (m_flag == KERN)
            createKernTableForm();
        else if (m_flag == Kwin)
            createKwinTableForm();
    }

    m_pLogBackend->loadSegementPage(nSegementIndex, bReset);

    return nSegementIndex;
}

/**
 * @brief DisplayContent::generateJournalFile 获取系统日志
 * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 * @param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
 * 0表示紧急
 * 1  ALERT（警告）：必须马上采取措施解决的问题
 * 2  CRIT（严重）：比较严重的情况 *
 * 3  ERR（错误）：运行出现错误
 * 4  WARNING（提醒）：可能会影响系统功能的事件
 * 5  NOTICE（注意）：不会影响系统但值得注意
 * 6  INFO（信息）：一般信息
 * 7  DEBUG（调试）：程序或系统调试信息等
 * @param iSearchStr 搜索关键字,现阶段不用,保留参数
 */
void DisplayContent::generateJournalFile(int id, int lId, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)
    //系统日志上次获取的时间,和筛选条件一起判断,防止获取过于频繁
    if (m_lastJournalGetTime.msecsTo(QDateTime::currentDateTime()) < 500 && m_journalFilter.timeFilter == id && m_journalFilter.eventTypeFilter == lId) {
        qCWarning(logDisplaycontent) << "load journal log: repeat refrsh journal too fast!";
        QItemSelectionModel *p = m_treeView->selectionModel();
        if (p)
            p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
        slot_tableItemClicked(m_pModel->index(0, 0));
        return;
    }
    m_lastJournalGetTime = QDateTime::currentDateTime();
    m_journalFilter.timeFilter = id;
    m_journalFilter.eventTypeFilter = lId;
    m_firstLoadPageData = true;
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    m_isDataLoadComplete = false;
    createJournalTableForm();
    setLoadState(DATA_LOADING);
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime());
    QStringList arg;
    if (lId != LVALL) {
        QString prio = QString("PRIORITY=%1").arg(lId);
        arg.append(prio);
    } else {
        arg.append("all");
    }
    switch (id) {
    case ALL:
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        arg << QString::number(dtStart.toMSecsSinceEpoch() * 1000) << QString::number(dtEnd.toMSecsSinceEpoch() * 1000);
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        arg << QString::number(dtStart.addDays(-2).toMSecsSinceEpoch() * 1000) << QString::number(dtEnd.toMSecsSinceEpoch() * 1000);
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        arg << QString::number(dtStart.addDays(-6).toMSecsSinceEpoch() * 1000) << QString::number(dtEnd.toMSecsSinceEpoch() * 1000);
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        arg << QString::number(dtStart.addMonths(-1).toMSecsSinceEpoch() * 1000) << QString::number(dtEnd.toMSecsSinceEpoch() * 1000);
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        arg << QString::number(dtStart.addMonths(-3).toMSecsSinceEpoch() * 1000) << QString::number(dtEnd.toMSecsSinceEpoch() * 1000);
    }
    break;
    default:
        break;
    }

    if (id >= ALL && id <= THREE_MONTHS)
        m_pLogBackend->parseByJournal(arg);

    m_treeView->setColumnWidth(JOURNAL_SPACE::journalLevelColumn, LEVEL_WIDTH);
    m_treeView->setColumnWidth(JOURNAL_SPACE::journalDaemonNameColumn, DEAMON_WIDTH);
    m_treeView->setColumnWidth(JOURNAL_SPACE::journalDateTimeColumn, DATETIME_WIDTH);
}
/**
 * @brief DisplayContent::createJournalTableStart 获取系统日志完成时第一次加载数据的第一页到treeview中
 * @param list 获得的系统日志数据list
 */
void DisplayContent::createJournalTableStart(const QList<LOG_MSG_JOURNAL> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertJournalTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

/**
 * @brief DisplayContent::createJournalTableForm 系统日志表头项目创建和重置
 */
void DisplayContent::createJournalTableForm()
{
    m_pModel->clear();

    m_pModel->setHorizontalHeaderLabels(
        QStringList() << DApplication::translate("Table", "Level")
        << DApplication::translate("Table", "Process") // modified by Airy
        << DApplication::translate("Table", "Date and Time")
        << DApplication::translate("Table", "Info")
        << DApplication::translate("Table", "User")
        << DApplication::translate("Table", "PID"));
    m_treeView->setColumnWidth(0, LEVEL_WIDTH);
    m_treeView->setColumnWidth(1, DEAMON_WIDTH);
    m_treeView->setColumnWidth(2, DATETIME_WIDTH);
}

/**
 * @brief DisplayContent::generateDpkgFile 触发获取dpkg日志数据线程
 * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 * @param iSearchStr 搜索关键字,保留字段,暂时不用
 */
void DisplayContent::generateDpkgFile(int id, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    setLoadState(DATA_LOADING);
    createDpkgTableForm();
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    DKPG_FILTERS dpkgFilter;

    switch (id) {
    case ALL:
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        dpkgFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        dpkgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        dpkgFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        dpkgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        dpkgFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        dpkgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        dpkgFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        dpkgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        dpkgFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        dpkgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    if (id >= ALL && id <= THREE_MONTHS) {
        m_pLogBackend->parseByDpkg(dpkgFilter);
    }
}

/**
 * @brief DisplayContent::createDpkgTable 获取系统日志完成时第一次加载数据的第一页到treeview中
 * @param list 获得的DPKG日志数据list
 */
void DisplayContent::createDpkgTableStart(const QList<LOG_MSG_DPKG> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertDpkgTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::createDpkgTableForm()
{
    m_pModel->clear();
    m_pModel->setColumnCount(3);
    m_treeView->setColumnWidth(0, DATETIME_WIDTH);
    m_treeView->hideColumn(2);
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "Info")
                                        << DApplication::translate("Table", "Action"));
}

/**
 * @brief DisplayContent::generateKernFile 触发获取内核日志数据线程
 * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 * @param iSearchStr 搜索关键字,保留字段,暂时不用
 */
void DisplayContent::generateKernFile(int id, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)
    m_pLogBackend->clearAllFilter();

    // 填充筛选条件
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    LOG_FILTER_BASE kernFilter;
    kernFilter.type = KERN;
    kernFilter.filePath = "kern";
    kernFilter.segementIndex = -1;

    switch (id) {
    case ALL:
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        kernFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        kernFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        kernFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        kernFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        kernFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        kernFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        kernFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        kernFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        kernFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        kernFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    if (id >= ALL && id <= THREE_MONTHS) {
        m_pLogBackend->m_type2Filter[KERN] = kernFilter;
        loadSegementPage();
    }
}

/**
 * @brief DisplayContent::createJournalTableForm 内核日志表头项目创建和重置
 */
void DisplayContent::createKernTableForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "User")
                                        << DApplication::translate("Table", "Process")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(0, DATETIME_WIDTH - 30);
    m_treeView->setColumnWidth(1, DEAMON_WIDTH);
    m_treeView->setColumnWidth(2, DEAMON_WIDTH);
}

/**
 * @brief DisplayContent::createJournalTableStart 获取内核日志完成时第一次加载数据的第一页到treeview中
 * @param list 获得的内核日志数据list
 */
// modified by Airy for bug  12263
void DisplayContent::createKernTable(const QList<LOG_MSG_JOURNAL> &list)
{
    setLoadState(DATA_COMPLETE);

    m_limitTag = 0;
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertKernTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

/**
 * @brief DisplayContent::insertKernTable 按分页转换插入内核日志到treeview的model中
 * @param list 当前筛选状态下所有符合条件的内核日志数据结构
 * @param start 分页开始的数组下标
 * @param end 分页结束的数组下标
 */
void DisplayContent::insertKernTable(const QList<LOG_MSG_JOURNAL> &list, int start, int end)
{
    QList<LOG_MSG_JOURNAL> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

/**
 * @brief DisplayContent::insertDpkgTable 按分页转换插入dpkg日志到treeview的model中
 * @param list 当前筛选状态下所有符合条件的内核日志数据结构
 * @param start 分页开始的数组下标
 * @param end 分页结束的数组下标
 */
void DisplayContent::insertDpkgTable(const QList<LOG_MSG_DPKG> &list, int start, int end)
{
    QList<LOG_MSG_DPKG> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertXorgTable(const QList<LOG_MSG_XORG> &list, int start, int end)
{
    QList<LOG_MSG_XORG> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertBootTable(const QList<LOG_MSG_BOOT> &list, int start, int end)
{
    QList<LOG_MSG_BOOT> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertKwinTable(const QList<LOG_MSG_KWIN> &list, int start, int end)
{
    QList<LOG_MSG_KWIN> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertNormalTable(const QList<LOG_MSG_NORMAL> &list, int start, int end)
{
    QList<LOG_MSG_NORMAL> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertOOCTable(const QList<LOG_FILE_OTHERORCUSTOM> &list, int start, int end)
{
    QList<LOG_FILE_OTHERORCUSTOM> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertAuditTable(const QList<LOG_MSG_AUDIT> &list, int start, int end)
{
    QList<LOG_MSG_AUDIT> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertCoredumpTable(const QList<LOG_MSG_COREDUMP> &list, int start, int end)
{
    QList<LOG_MSG_COREDUMP> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

/**
 * @brief DisplayContent::generateAppFile 触发获取应用日志数据线程
 * @param app 要获取的某一个应用的项目名称
 * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 * @param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
 * @param iSearchStr 搜索关键字,现阶段不用,保留参数
 */
void DisplayContent::generateAppFile(const QString &app, int id, int lId, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    setLoadState(DATA_LOADING);
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    createAppTableForm();

    APP_FILTERS appFilter;
    appFilter.app = app;
    appFilter.lvlFilter = lId;
    appFilter.submodule = m_pLogBackend->m_appFilter.submodule;
    switch (id) {
    case ALL:
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        appFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        appFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    m_pLogBackend->m_appFilter = appFilter;
    if (id >= ALL && id <= THREE_MONTHS)
        m_pLogBackend->parseByApp(appFilter);
}

/**
 * @brief DisplayContent::createAppTableForm 系统日志表头项目创建和重置 设置列宽
 */
void DisplayContent::createAppTableForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Level")
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "Source")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(0, LEVEL_WIDTH);
    m_treeView->setColumnWidth(1, DATETIME_WIDTH);
    m_treeView->setColumnWidth(2, DEAMON_WIDTH + 28);
}

/**
 * @brief DisplayContent::createAppTable 获取应用日志完成时第一次加载数据的第一页到treeview中
 * @param list 获得的应用日志数据list
 */
void DisplayContent::createAppTable(const QList<LOG_MSG_APPLICATOIN> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertApplicationTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

/**
 * @brief DisplayContent::createBootTable 获取启动日志完成时加载所有数据到treeview中
 * @param list 获得的启动日志数据list
 */
void DisplayContent::createBootTable(const QList<LOG_MSG_BOOT> &list)
{
    m_limitTag = 0;

    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertBootTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::generateBootFile()
{
    setLoadState(DATA_LOADING);
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    createBootTableForm();
    m_pLogBackend->parseByBoot();
}

void DisplayContent::createXorgTableForm()
{
    m_pModel->clear();
    m_pModel->setColumnCount(2);
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Offset")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(0, DATETIME_WIDTH + 20);
}

/**
 * @brief DisplayContent::createXorgTable 获取Xorg日志完成时加载所有数据到treeview中
 * @param list 获得的Xorg日志数据list
 */
void DisplayContent::createXorgTable(const QList<LOG_MSG_XORG> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertXorgTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

/**
 * @brief DisplayContent::generateXorgFile 触发获取Xorg日志数据线程
 * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 */
void DisplayContent::generateXorgFile(int id)
{
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    setLoadState(DATA_LOADING);
    QDateTime dt = QDateTime::currentDateTime();
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    dt.setTime(QTime()); // get zero time
    createXorgTableForm();
    XORG_FILTERS xorgFilter;
    switch (id) {
    case ALL:
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        xorgFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        xorgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        xorgFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        xorgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        xorgFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        xorgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        xorgFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        xorgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        xorgFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        xorgFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    if (id >= ALL && id <= THREE_MONTHS)
        m_pLogBackend->parseByXlog(xorgFilter);
}

void DisplayContent::createKwinTableForm()
{
    m_pModel->clear();
    m_pModel->setColumnCount(1);
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Info"));
}

/**
 * @brief DisplayContent::creatKwinTable 获取kwin日志完成时加载所有数据到treeview中
 * @param list 获得的kwin日志数据list
 */
void DisplayContent::creatKwinTable(const QList<LOG_MSG_KWIN> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertKwinTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

/**
 * @brief DisplayContent::generateKwinFile 触发获取Kwin日志数据线程
 */
void DisplayContent::generateKwinFile(const KWIN_FILTERS &iFilters)
{
    Q_UNUSED(iFilters)
    m_pLogBackend->clearAllFilter();

    LOG_FILTER_BASE filter;
    filter.type = Kwin;
    filter.segementIndex = -1;
    m_pLogBackend->m_type2Filter[Kwin] = filter;
    loadSegementPage();
}

void DisplayContent::createNormalTableForm()
{
    m_pModel->clear();
    m_pModel->setColumnCount(4);
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Event Type")
                                        << DApplication::translate("Table", "Username")
                                        << DApplication::translate("Tbble", "Date and Time")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(0, DATETIME_WIDTH - 20);
    m_treeView->setColumnWidth(1, DATETIME_WIDTH);
    m_treeView->setColumnWidth(2, DATETIME_WIDTH);
    m_treeView->setColumnWidth(3, DATETIME_WIDTH);
}

/**
 * @brief DisplayContent::createNormalTable 开关机日志表头项目创建和重置
 * @param list
 */
void DisplayContent::createNormalTable(const QList<LOG_MSG_NORMAL> &list)
{
    setLoadState(DATA_COMPLETE);

    m_limitTag = 0;
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertNormalTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

/**
 * @brief DisplayContent::generateNormalFile 触发获取开关机日志数据线程
 * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 */
// add by Airy
void DisplayContent::generateNormalFile(int id)
{
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    setLoadState(DATA_LOADING);
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    createNormalTableForm();
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    QDateTime dtStart = dt;
    QDateTime dtEnd = dt;
    dtEnd.setTime(QTime(23, 59, 59, 999));

    NORMAL_FILTERS normalFilter;
    normalFilter.eventTypeFilter = m_curNormalEventType;
    switch (id) {
    case ALL:
        normalFilter.timeFilterEnd = -1;
        normalFilter.timeFilterBegin = -1;
        break;
    case ONE_DAY: {
        normalFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        normalFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        normalFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        normalFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        normalFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        normalFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        normalFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        normalFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        normalFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        normalFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    m_pLogBackend->m_normalFilter = normalFilter;
    if (id >= ALL && id <= THREE_MONTHS)
        m_pLogBackend->parseByNormal(normalFilter);

    m_pLogBackend->nortempList = m_pLogBackend->norList;
}

/**
 * @brief DisplayContent::insertJournalTable 按分页转换插入系统日志到treeview的model中
 * @param logList 当前筛选状态下所有符合条件的系统日志数据结构
 * @param start 分页开始的数组下标
 * @param end 分页结束的数组下标
 */
void DisplayContent::insertJournalTable(QList<LOG_MSG_JOURNAL> logList, int start, int end)
{
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    for (int i = start; i < end; i++) {
        items.clear();
        item = new DStandardItem();
        QString iconPath = m_iconPrefix + getIconByname(logList[i].level);

        if (getIconByname(logList[i].level).isEmpty())
            item->setText(logList[i].level);
        item->setIcon(QIcon(iconPath));
        item->setData(JOUR_TABLE_DATA);
        item->setData(logList[i].level, Log_Item_SPACE::levelRole);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(logList[i].daemonName);
        item->setData(JOUR_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;
        item = new DStandardItem(logList[i].dateTime);
        item->setData(JOUR_TABLE_DATA);
//        item->setData(getTimeFromString(logList[i].dateTime), Qt::UserRole + 2);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        items << item;
        item = new DStandardItem(logList[i].msg);
        item->setData(JOUR_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(3));
        items << item;
        item = new DStandardItem(logList[i].hostName);
        item->setData(JOUR_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(4));
        items << item;
        item = new DStandardItem(logList[i].daemonId);
        item->setData(JOUR_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(5));
        items << item;
        m_pModel->insertRow(m_pModel->rowCount(), items);
    }
    m_treeView->hideColumn(JOURNAL_SPACE::journalHostNameColumn);
    m_treeView->hideColumn(JOURNAL_SPACE::journalDaemonIdColumn);
}

/**
 * @brief DisplayContent::getAppName 获取当前选择的应用的日志路径对应的日志名称
 * @param filePath  当前选择的应用的日志路径
 * @return 对应的日志名称
 */
QString DisplayContent::getAppName(const QString &filePath)
{

    QString ret = Utils::appName(filePath);
    if (ret.isEmpty())
        return ret;

    return LogApplicationHelper::instance()->transName(ret);
}

/**
 * @brief DisplayContent::isAuthProcessAlive 日志获取进程是否存在
 * @return true:存在 false: 不存在
 */
bool DisplayContent::isAuthProcessAlive()
{
    bool ret = false;
    QProcess proc;
    int rslt = proc.execute("ps -aux | grep 'logViewerAuth'");
    return !(ret = (rslt == 0));
}

/**
 * @brief DisplayContent::generateJournalBootFile 触发获取klu下启动日志的线程
 * @param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
 * 0表示紧急
 * 1  ALERT（警告）：必须马上采取措施解决的问题
 * 2  CRIT（严重）：比较严重的情况 *
 * 3  ERR（错误）：运行出现错误
 * 4  WARNING（提醒）：可能会影响系统功能的事件
 * 5  NOTICE（注意）：不会影响系统但值得注意
 * 6  INFO（信息）：一般信息
 * 7  DEBUG（调试）：程序或系统调试信息等
 * @param iSearchStr iSearchStr 搜索关键字,现阶段不用,保留参数
 */
void DisplayContent::generateJournalBootFile(int lId, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)
    m_firstLoadPageData = true;
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    m_isDataLoadComplete = false;
    createJournalBootTableForm();
    setLoadState(DATA_LOADING);
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime());
    QStringList arg;
    if (lId != LVALL) {
        QString prio = QString("PRIORITY=%1").arg(lId);
        arg.append(prio);
    } else {
        arg.append("all");
    }
    m_pLogBackend->parseByJournalBoot(arg);
    // default first row select
    m_treeView->setColumnWidth(JOURNAL_SPACE::journalLevelColumn, LEVEL_WIDTH);
    m_treeView->setColumnWidth(JOURNAL_SPACE::journalDaemonNameColumn, DEAMON_WIDTH);
    m_treeView->setColumnWidth(JOURNAL_SPACE::journalDateTimeColumn, DATETIME_WIDTH);
}

/**
 * @brief DisplayContent::createJournalBootTableStart 获取klu下启动日志完成时第一次加载数据的第一页到treeview中
 * @param list klu下启动日志数据list
 */
void DisplayContent::createJournalBootTableStart(const QList<LOG_MSG_JOURNAL> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertJournalBootTable(list, 0, end);
}

/**
 * @brief DisplayContent::createJournalBootTableForm klu下启动日志表头项目创建和重置
 */
void DisplayContent::createJournalBootTableForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(
        QStringList() << DApplication::translate("Table", "Level")
        << DApplication::translate("Table", "Process") // modified by Airy
        << DApplication::translate("Table", "Date and Time")
        << DApplication::translate("Table", "Info")
        << DApplication::translate("Table", "User")
        << DApplication::translate("Table", "PID"));
    m_treeView->setColumnWidth(0, LEVEL_WIDTH);
    m_treeView->setColumnWidth(1, DEAMON_WIDTH);
    m_treeView->setColumnWidth(2, DATETIME_WIDTH);
}

/**
 * @brief DisplayContent::insertJournalBootTable 按分页转换插入klu下启动日志到treeview的model中
 * @param logList 当前筛选状态下所有符合条件的klu下启动日志数据结构
 * @param start 分页开始的数组下标
 * @param end 分页结束的数组下标
 */
void DisplayContent::insertJournalBootTable(QList<LOG_MSG_JOURNAL> logList, int start, int end)
{
    DStandardItem *item = new DStandardItem();
    QList<QStandardItem *> items;
    for (int i = start; i < end; i++) {
        items.clear();
        QString iconPath = m_iconPrefix + getIconByname(logList[i].level);

        if (getIconByname(logList[i].level).isEmpty())
            item->setText(logList[i].level);
        item->setIcon(QIcon(iconPath));
        item->setData(BOOT_KLU_TABLE_DATA);
        item->setData(logList[i].level, Log_Item_SPACE::levelRole);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(logList[i].daemonName);
        item->setData(BOOT_KLU_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;
        item = new DStandardItem(logList[i].dateTime);
        item->setData(BOOT_KLU_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        items << item;
        item = new DStandardItem(logList[i].msg);
        item->setData(BOOT_KLU_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(3));
        items << item;
        item = new DStandardItem(logList[i].hostName);
        item->setData(BOOT_KLU_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(4));
        items << item;
        item = new DStandardItem(logList[i].daemonId);
        item->setData(BOOT_KLU_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(5));
        items << item;
        m_pModel->insertRow(m_pModel->rowCount(), items);
    }
    m_treeView->hideColumn(JOURNAL_SPACE::journalHostNameColumn);
    m_treeView->hideColumn(JOURNAL_SPACE::journalDaemonIdColumn);

    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
    delete item;
}

void DisplayContent::generateDnfFile(BUTTONID iDate, DNFPRIORITY iLevel)
{
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    setLoadState(DATA_LOADING);
    createDnfForm();
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    DNF_FILTERS dnffilter;
    dnffilter.levelfilter = iLevel;
    switch (iDate) {
    case ALL:
        dnffilter.timeFilter = 0;
        break;
    case ONE_DAY: {
        dnffilter.timeFilter = dt.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        dnffilter.timeFilter = dt.addDays(-2).toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        dnffilter.timeFilter = dt.addDays(-6).toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        dnffilter.timeFilter = dt.addDays(-29).toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        dnffilter.timeFilter = dt.addDays(-89).toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    if (iDate >= ALL && iDate <= THREE_MONTHS)
        m_pLogBackend->parseByDnf(dnffilter);
}

void DisplayContent::createDnfTable(const QList<LOG_MSG_DNF> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertDnfTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::generateDmesgFile(BUTTONID iDate, PRIORITY iLevel)
{
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    setLoadState(DATA_LOADING);
    createDmesgForm();
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    DMESG_FILTERS dmesgfilter;
    dmesgfilter.levelFilter = iLevel;
    switch (iDate) {
    case ALL:
        dmesgfilter.timeFilter = 0;
        break;
    case ONE_DAY: {
        dmesgfilter.timeFilter = dt.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        dmesgfilter.timeFilter = dt.addDays(-2).toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        dmesgfilter.timeFilter = dt.addDays(-6).toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        dmesgfilter.timeFilter = dt.addDays(-29).toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        dmesgfilter.timeFilter = dt.addDays(-89).toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    if (iDate >= ALL && iDate <= THREE_MONTHS)
        m_pLogBackend->parseByDmesg(dmesgfilter);
}

void DisplayContent::createDmesgTable(const QList<LOG_MSG_DMESG> &list)
{
    m_limitTag = 0;
    setLoadState(DATA_COMPLETE);
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertDmesgTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::createDnfForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Level")
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(DNF_SPACE::dnfLvlColumn, LEVEL_WIDTH);
    m_treeView->setColumnWidth(DNF_SPACE::dnfDateTimeColumn, DATETIME_WIDTH);
    m_treeView->hideColumn(3);
}

void DisplayContent::createDmesgForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Level")
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(DMESG_SPACE::dmesgLevelColumn, LEVEL_WIDTH);
    m_treeView->setColumnWidth(DMESG_SPACE::dmesgDateTimeColumn, DATETIME_WIDTH);
    m_treeView->hideColumn(3);
}

void DisplayContent::insertDmesgTable(const QList<LOG_MSG_DMESG> &list, int start, int end)
{
    QList<LOG_MSG_DMESG> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

void DisplayContent::insertDnfTable(const QList<LOG_MSG_DNF> &list, int start, int end)
{
    QList<LOG_MSG_DNF> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

/**
 * @brief DisplayContent::slot_tableItemClicked treeview主表点击槽函数,用来发出信号在详情页显示当前选中项日志详细信息
 * @param index 选中的modelindex
 */
void DisplayContent::slot_tableItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (m_curTreeIndex == index) {
        return;
    }

    m_curTreeIndex = index;

    if (m_flag == OtherLog || m_flag == CustomLog) {
        QString path = m_pModel->item(index.row(), 0)->data(Qt::UserRole + 2).toString();
        m_pLogBackend->setFlag(m_flag);
        generateOOCFile(path);
    } else {
        emit sigDetailInfo(index, m_pModel, getAppName(m_curApp));
    }
}

/**
 * @brief DisplayContent::slot_BtnSelected 连接外部筛选控件筛选条件处理触发获取对应数据的槽函数
 * @param btnId 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
 * @param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
 * @param idx 日志类型选择的listview的当前选中的QModelIndex
 */
void DisplayContent::slot_BtnSelected(int btnId, int lId, QModelIndex idx)
{
    m_curLevel = lId; // m_curLevel equal combobox index-1;
    m_curBtnId = btnId;

    QString treeData = idx.data(ITEM_DATE_ROLE).toString();
    if (treeData.isEmpty())
        return;

    if (treeData.contains(OTHER_TREE_DATA, Qt::CaseInsensitive) || treeData.contains(CUSTOM_TREE_DATA, Qt::CaseInsensitive)) {
        return;
    }

    m_detailWgt->cleanText();
    if (treeData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive)) {
        generateJournalFile(btnId, m_curLevel);
    } else if (treeData.contains(BOOT_KLU_TREE_DATA, Qt::CaseInsensitive)) {
        generateJournalBootFile(m_curLevel);
    } else if (treeData.contains(DPKG_TREE_DATA, Qt::CaseInsensitive)) {
        generateDpkgFile(btnId);
    } else if (treeData.contains(KERN_TREE_DATA, Qt::CaseInsensitive)) {
        generateKernFile(btnId);
    } else if (treeData.contains(".cache")) {
        //        generateAppFile(treeData, btnId, lId);
    } else if (treeData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
        if (!m_curApp.isEmpty()) {
            generateAppFile(m_curApp, btnId, m_curLevel);
        }
    } else if (treeData.contains(XORG_TREE_DATA, Qt::CaseInsensitive)) { // add by Airy
        generateXorgFile(btnId);
    } else if (treeData.contains(LAST_TREE_DATA, Qt::CaseInsensitive)) { // add by Airy
        generateNormalFile(btnId);
    } else if (treeData.contains(DNF_TREE_DATA, Qt::CaseInsensitive)) {
        generateDnfFile(BUTTONID(m_curBtnId), m_curDnfLevel);
    } else if (treeData.contains(DMESG_TREE_DATA, Qt::CaseInsensitive)) {
        generateDmesgFile(BUTTONID(m_curBtnId), PRIORITY(m_curLevel));
    } else if (treeData.contains(KWIN_TREE_DATA, Qt::CaseInsensitive)) {
        KWIN_FILTERS filter;
        generateKwinFile(filter);
    } else if (treeData.contains(AUDIT_TREE_DATA, Qt::CaseInsensitive)) {
        generateAuditFile(BUTTONID(m_curBtnId), AUDITTYPE(m_curAuditType));
    } else if(treeData.contains(COREDUMP_TREE_DATA, Qt::CaseInsensitive)) {
        generateCoredumpFile(btnId);
    }
}

/**
 * @brief DisplayContent::slot_appLogs 根据应用日志应用类型变化触发应用日志获取线程
 * @param btnId 周期Id
 * @param app 应用项目名称
 */
void DisplayContent::slot_appLogs(int btnId, const QString &app)
{
    m_curApp = app;
    m_curBtnId = btnId;
    m_pLogBackend->m_appFilter.clear();
    m_pLogBackend->m_appFilter.submodule = "";
    generateAppFile(app, m_curBtnId, m_curLevel);
}

/**
 * @brief DisplayContent::slot_logCatelogueClicked 日志类型选择列表选项目变化根据日志类型获取数据
 * @param index loglistView当前选择的项目
 */
void DisplayContent::slot_logCatelogueClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (m_curListIdx == index && (m_flag != KERN && m_flag != BOOT)) {
        return;
    }

    m_curListIdx = index;

    clearAllDatas();

    QString itemData = index.data(ITEM_DATE_ROLE).toString();
    if (itemData.isEmpty())
        return;

    //界面参数变化
    int height = this->height();
    int handleW = m_splitter->handleWidth();
    if (itemData.contains(CUSTOM_TREE_DATA, Qt::CaseInsensitive) || itemData.contains(OTHER_TREE_DATA, Qt::CaseInsensitive)) {
        m_splitter->handle(3)->setDisabled(false);
        m_detailWgt->setFixedHeight(QWIDGETSIZE_MAX);
        m_detailWgt->setMinimumHeight(70);

        const int heightLogTree = 164;
        QMargins margins = this->parentWidget()->layout()->layout()->contentsMargins();
        height = this->parentWidget()->height() - margins.top() - margins.bottom();
        int heightDetailWgt = height - heightLogTree - handleW;
        m_splitter->setSizes(QList<int>() << heightLogTree << heightDetailWgt << heightDetailWgt << heightDetailWgt);
    } else {
        height -= handleW;
        m_splitter->handle(3)->setDisabled(true);
        m_detailWgt->setFixedHeight(230);
        m_splitter->setSizes(QList<int>() << height * 5 / 8 << 0 << 0 << height * 3 / 8);
    }

    if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive)) {
        // default level is info so PRIORITY=6
        m_flag = JOURNAL;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(DPKG_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = DPKG;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(XORG_TREE_DATA, Qt::CaseInsensitive)) {
        m_pLogBackend->xList.clear();
        m_flag = XORG;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(BOOT_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = BOOT;
        m_pLogBackend->setFlag(m_flag);
        generateBootFile();
    } else if (itemData.contains(KERN_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = KERN;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(".cache")) {
    } else if (itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
        m_pModel->clear(); // clicked parent node application, clear table contents
        m_flag = APP;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(LAST_TREE_DATA, Qt::CaseInsensitive)) {
        m_pLogBackend->norList.clear();
        m_flag = Normal;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(KWIN_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Kwin;
        m_pLogBackend->setFlag(m_flag);
        KWIN_FILTERS filter;
        filter.msg = "";
    } else if (itemData.contains(BOOT_KLU_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = BOOT_KLU;
        m_pLogBackend->setFlag(m_flag);
        generateJournalBootFile(m_curLevel);
    } else if (itemData.contains(DNF_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Dnf;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(DMESG_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Dmesg;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(OTHER_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = OtherLog;
        m_pLogBackend->setFlag(m_flag);
        generateOOCLogs(OOC_OTHER);
    } else if (itemData.contains(CUSTOM_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = CustomLog;
        m_pLogBackend->setFlag(m_flag);
        generateOOCLogs(OOC_CUSTOM);
    } else if (itemData.contains(AUDIT_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Audit;
        m_pLogBackend->setFlag(m_flag);
    } else if (itemData.contains(COREDUMP_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = COREDUMP;
        m_pLogBackend->setFlag(m_flag);
    }
}

/**
 * @brief DisplayContent::slot_exportClicked 导出按钮触发槽函数,根据当前筛选出来的数据和数据类型,弹出文件目录选择框选择导出目录,执行导出逻辑
 */
void DisplayContent::slot_exportClicked()
{
    QString logName;
    if (m_curListIdx.isValid())
        logName = QString("/%1").arg(m_curListIdx.data().toString());
    else {
        logName = QString("/%1").arg(("New File"));
    }

    QString path, fileName;
    if (m_flag != COREDUMP) {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + logName + ".txt";
        fileName = DFileDialog::getSaveFileName(
                    this, DApplication::translate("File", "Export File"),
                    path,
                    tr("TEXT (*.txt);; Doc (*.doc);; Xls (*.xls);; Html (*.html)"), &selectFilter);
    } else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + logName + ".zip";
        fileName = DFileDialog::getSaveFileName(
                    this, DApplication::translate("File", "Export File"),
                    path,
                    tr("zip(*.zip)"), &selectFilter);
    }

    if (fileName.isEmpty()) {
        DApplication::setActiveWindow(this);
        return;
    }

    //限制当导出文件为空和导出doc和xls时用户改动后缀名导致导出问题，提示导出失败
    QFileInfo exportFile(fileName);

    QString exportSuffix = exportFile.suffix();
    QString selectSuffix = selectFilter.mid(selectFilter.lastIndexOf(".") + 1, selectFilter.size() - selectFilter.lastIndexOf(".") - 2);
    if (fileName.isEmpty()) {
        onExportResult(false);
        return;
    }

    //用户修改后缀名后添加默认的后缀
    if (selectSuffix != exportSuffix) {
        fileName.append(".").append(selectSuffix);
    }

    m_exportDlg->show();

    // 获取表头内容
    QStringList labels;
    for (int col = 0; col < m_pModel->columnCount(); ++col) {
        labels.append(m_pModel->horizontalHeaderItem(col)->text());
    }

    // 后端导出当前页日志数据
    m_pLogBackend->exportLogData(fileName, labels);

    // 若有分段数据，开启分段导出
    m_pLogBackend->segementExport();
}

/**
 * @brief DisplayContent::slot_statusChagned 启动日志的状态combox选项改变槽函数,筛选当前启动日志内容
 * @param status 筛选的状态,有all ok failed
 */
void DisplayContent::slot_statusChagned(const QString &status)
{
    m_pLogBackend->m_bootFilter.statusFilter = status;
    m_pLogBackend->currentBootList = LogBackend::filterBoot(m_pLogBackend->m_bootFilter, m_pLogBackend->bList);
    createBootTableForm();
    createBootTable(m_pLogBackend->currentBootList);
}

void DisplayContent::slot_parseFinished(LOG_FLAG type, int status)
{
    if (m_flag != type)
        return;

    qCDebug(logDisplaycontent) << QString("parse finished m_type2LogData[%1] dataCount: %2 segement index: %3").arg(type).arg(m_pLogBackend->m_type2LogData[type].count()).arg(m_pLogBackend->m_type2Filter[type].segementIndex);

    int nSegementIndex = -1;
    // 取消鉴权时，若导出进度条存在，则隐藏
    if (status == ParseThreadBase::CancelAuth) {
        if (m_exportDlg && !m_exportDlg->isHidden()) {
            m_exportDlg->hide();
            DApplication::setActiveWindow(this);
        }
    } else {
        // 分段加载逻辑处理
        if (m_treeView->verticalScrollBar()->maximum() == 0 || m_pModel->rowCount() < TREE_NORMAL_COUNT) {
            // 数据未填满表格显示区域，分段加载下一段数据
            nSegementIndex = loadSegementPage(true, false);
        }
    }

    // 已加载到文件末尾，依然没有数据，则创建空表显示，若有关键词搜索，则显示无搜索结果
    if (nSegementIndex == -1) {
        if (m_pLogBackend->m_type2LogData[type].isEmpty()) {
            if (!m_pLogBackend->m_currentSearchStr.isEmpty()) {
                setLoadState(DATA_NO_SEARCH_RESULT);
            } else {
                setLoadState(DATA_COMPLETE);
                createLogTable(m_pLogBackend->m_type2LogData[type], type);
            }
            m_detailWgt->cleanText();
            m_detailWgt->hideLine(true);
        } else {
            setLoadState(DATA_COMPLETE);
            m_detailWgt->hideLine(false);
        }

        qCDebug(logDisplaycontent) << QString("parse/search end... type:[%1]").arg(type);
    }
}

void DisplayContent::slot_logData(const QList<QString> &list, LOG_FLAG type)
{
    if (m_flag != type)
        return;

    if (!list.isEmpty()) {
        int rowCount = m_pModel->rowCount();
        if (rowCount == 0)
            createLogTable(list, type);
        else if (rowCount < SINGLE_READ_CNT) {
            int loadCount = SINGLE_READ_CNT - rowCount;
            insertLogTable(list, 0, loadCount, type);
        }
    }
}

void DisplayContent::slot_clearTable()
{
    m_pModel->clear();
    if (m_flag == KERN)
        createKernTableForm();
    else if (m_flag == Kwin)
        createKwinTableForm();
}

/**
 * @brief DisplayContent::slot_dpkgFinished 获取dpkg日志数据线程结果的槽函数,把获取到的数据加入treeview的model中以显示
 */
void DisplayContent::slot_dpkgFinished()
{
    if (m_flag != DPKG)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->dList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createDpkgTableStart(m_pLogBackend->dList);
    }
}

void DisplayContent::slot_dpkgData(const QList<LOG_MSG_DPKG> &list)
{
    if (m_flag != DPKG)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createDpkgTableStart(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=dpkg");
    }
}

/**
 * @brief DisplayContent::slot_XorgFinished 获取xorg日志数据线程获取结束
 */
void DisplayContent::slot_XorgFinished()
{
    if (m_flag != XORG)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->xList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createXorgTable(m_pLogBackend->xList);
    }
}

void DisplayContent::slot_xorgData(const QList<LOG_MSG_XORG> &list)
{
    if (m_flag != XORG)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createXorgTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=xorg");
    }
}

/**
 * @brief DisplayContent::slot_bootFinished 获取启动日志数据线程结果的槽函数,把获取到的数据加入treeview的model中以显示
 */
void DisplayContent::slot_bootFinished()
{
    if (m_flag != BOOT)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->currentBootList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createBootTable(m_pLogBackend->currentBootList);
    }
}

void DisplayContent::slot_bootData(const QList<LOG_MSG_BOOT> &list)
{
    if (m_flag != BOOT)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createBootTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=boot");
    }
}

/**
 * @brief DisplayContent::slot_kernFinished 获取内核日志数据线程结果的槽函数,把获取到的数据加入treeview的model中以显示
 * @param list 启动日志数据线程list
 */
void DisplayContent::slot_kernFinished()
{
    if (m_flag != KERN)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->kList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createKernTable(m_pLogBackend->kList);
    }
}

void DisplayContent::slot_kernData(const QList<LOG_MSG_JOURNAL> &list)
{
    if (m_flag != KERN)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createKernTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=kern");
    }
}

/**
 * @brief DisplayContent::slot_kwinFinished 获取kwin日志数据线程结果的槽函数,把获取到的数据加入treeview的model中以显示
 * @param list kwin日志数据线程list
 */
void DisplayContent::slot_kwinFinished()
{
    if (m_flag != Kwin)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->m_currentKwinList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        creatKwinTable(m_pLogBackend->m_currentKwinList);
    }
}

void DisplayContent::slot_kwinData(const QList<LOG_MSG_KWIN> &list)
{
    if (m_flag != Kwin)
        return;

    if (m_firstLoadPageData && !list.isEmpty()) {
        creatKwinTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=kwin");
    }
}

void DisplayContent::slot_journalFinished()
{
    if (m_flag != JOURNAL)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->jList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createJournalTableStart(m_pLogBackend->jList);
    }
}

void DisplayContent::slot_dnfFinished(const QList<LOG_MSG_DNF> &list)
{
    if (m_flag != Dnf)
        return;

    createDnfTable(list);
    PERF_PRINT_END("POINT-03", "type=dnf");
}

void DisplayContent::slot_dmesgFinished(const QList<LOG_MSG_DMESG> &list)
{
    if (m_flag != Dmesg)
        return;

    createDmesgTable(list);
    PERF_PRINT_END("POINT-03", "type=dmesg");
}

/**
 * @brief DisplayContent::slot_journalData 系统日志数据获取线程槽函数,系统日志为获取500条就会执行此信号,不是一次把所有数据传进来,所以执行槽函数应该为每次获取向现在的model中添加而不是重置
 * @param index 槽函数发出线程的标记量序号
 * @param list 本次获取的500个或以下的数据
 */
void DisplayContent::slot_journalData(const QList<LOG_MSG_JOURNAL> &list)
{
    //判断最近一次获取数据线程的标记量,和信号曹发来的sender的标记量作对比,如果相同才可以刷新,因为会出现上次的获取线程就算停下信号也发出来了
    if (m_flag != JOURNAL)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createJournalTableStart(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-01", "");
        PERF_PRINT_END("POINT-03", "type=system");
    }
}

void DisplayContent::slot_journalBootFinished()
{
    if (m_flag != BOOT_KLU)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->jBootList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createJournalBootTableStart(m_pLogBackend->jBootList);
    }
}

/**
 * @brief DisplayContent::slot_journalBootData klu下启动日志日志数据获取线程槽函数,系统日志为获取500条就会执行此信号,不是一次把所有数据传进来,所以执行槽函数应该为每次获取向现在的model中添加而不是重置
 * @param index 槽函数发出线程的标记量序号
 * @param list 本次获取的500个或以下的数据
 */
void DisplayContent::slot_journalBootData(const QList<LOG_MSG_JOURNAL> &list)
{
    if (m_flag != BOOT_KLU)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createJournalBootTableStart(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=boot_klu");
    }
}

/**
 * @brief DisplayContent::slot_applicationFinished 获取应用日志数据线程结果的槽函数,把获取到的数据加入treeview的model中以显示
 */
void DisplayContent::slot_applicationFinished()
{
    if (m_flag != APP)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->appList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createAppTable(m_pLogBackend->appList);
    }
}

void DisplayContent::slot_applicationData(const QList<LOG_MSG_APPLICATOIN> &list)
{
    if (m_flag != APP)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createAppTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=application");
    }
}

void DisplayContent::slot_normalFinished()
{
    if (m_flag != Normal)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->nortempList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        createNormalTable(m_pLogBackend->nortempList);
    }
}

void DisplayContent::slot_normalData(const QList<LOG_MSG_NORMAL> &list)
{
    if (m_flag != Normal)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createNormalTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=on_off");
    }
}

void DisplayContent::slot_OOCData(const QString &data)
{
    if ((m_flag != OtherLog && m_flag != CustomLog))
        return;

    if (!m_treeView->selectionModel()->selectedRows().isEmpty())
        emit sigDetailInfo(m_treeView->selectionModel()->selectedRows().first(), m_pModel, data);
}

void DisplayContent::slot_auditFinished(bool bShowTip/* = false*/)
{
    if (m_flag != Audit)
        return;
    m_isDataLoadComplete = true;
    if (m_pLogBackend->aList.isEmpty()) {
        if (bShowTip) {
            createAuditTable(m_pLogBackend->aList);
            QTimer::singleShot(50, this, [=]{
                setLoadState(DATA_NOT_AUDIT_ADMIN);
            });
            m_detailWgt->cleanText();
            m_detailWgt->hideLine(true);
        } else {
            setLoadState(DATA_COMPLETE);
            createAuditTable(m_pLogBackend->aList);
        }
    }
}

void DisplayContent::slot_auditData(const QList<LOG_MSG_AUDIT> &list)
{
    if (m_flag != Audit)
        return;

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createAuditTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=audit");
    }
}

void DisplayContent::slot_coredumpFinished()
{
    if (m_flag != COREDUMP)
        return;

    m_isDataLoadComplete = true;
    if (m_pLogBackend->m_currentCoredumpList.isEmpty()) {
        setLoadState(DATA_COMPLETE);
        emit setExportEnable(false);
        createCoredumpTable(m_pLogBackend->m_currentCoredumpList);
    }
}
void DisplayContent::slot_coredumpData(const QList<LOG_MSG_COREDUMP> &list, bool newData)
{
    if (m_flag != COREDUMP)
        return;

    //分页已到底部，来新数据时需要刷新到下一页
    int value = m_treeView->verticalScrollBar()->value();
    int maximum = m_treeView->verticalScrollBar()->maximum();
    if (value == maximum && value > 0 && newData) {
        int rateValue = (m_treeViewLastScrollValue + 25) / SINGLE_LOAD;
        int leftCnt = list.count() - SINGLE_LOAD * rateValue;
        int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
        m_limitTag = rateValue;
        insertCoredumpTable(list, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
    }

    //因为此槽会在同一次加载数据完成前触发数次,所以第一次收到数据需要更新界面状态,后面的话往model里塞数据就行
    if (m_firstLoadPageData && !list.isEmpty()) {
        createCoredumpTable(list);
        m_firstLoadPageData = false;
        PERF_PRINT_END("POINT-03", "type=coredump");
    }
}

void DisplayContent::slot_OOCFinished(int error)
{
    if ((m_flag != OtherLog && m_flag != CustomLog))
        return;
    m_isDataLoadComplete = true;
    setLoadState(DATA_COMPLETE);

    //未通过鉴权在日志区域显示文案：无权限查看
    if (error == 1) {
        clearAllDatas();
        setLoadState(DATA_NO_PERMISSION);
    }
}

/**
 * @brief DisplayContent::slot_logLoadFailed 数据获取失败槽函数，显示错误提示框
 * @param iError 错误信息
 */
void DisplayContent::slot_logLoadFailed(const QString &iError)
{
    QString titleIcon = ICONPREFIX;
    DMessageManager::instance()->sendMessage(this->window(), QIcon(titleIcon + "warning_info.svg"), iError);
}

/**
 * @brief DisplayContent::slot_vScrollValueChanged 滚动条滚动实现分页加载数据槽函数
 * @param valuePixel 当前滑动的像素
 */
void DisplayContent::slot_vScrollValueChanged(int valuePixel)
{
    if (!m_treeView) {
        return;
    }
    if (m_treeView->singleRowHeight() < 0) {
        return;
    }
    //根据当前表格单行行高计算现在滑动了多少项
    int value = valuePixel / m_treeView->singleRowHeight(); // m_treeView->singleRowHeight();
    if (m_treeViewLastScrollValue == value) {
        return;
    }
    m_treeViewLastScrollValue = value;
    //算出现在滚动了多少页
    int rateValue = (value + 25) / SINGLE_LOAD;

    // 滚动到顶部，启动向上分段加载
    if (m_treeView->verticalScrollBar()->minimum() == m_treeView->verticalScrollBar()->value() && 0 == rateValue) {
        loadSegementPage(false);
        return;
    }

    // 滚动到底部，启动向下分段加载
    if (m_treeView->verticalScrollBar()->maximum() == m_treeView->verticalScrollBar()->value()) {
        loadSegementPage();
        return;
    }

    switch (m_flag) {
    case JOURNAL: {
        //如果快滚到页底了就加载下一页数据到表格中
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;

            int leftCnt = m_pLogBackend->jList.count() - SINGLE_LOAD * rateValue;
            //如果在页尾部则只加载最后一页的数量,否则加载单页全部数量
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            //把数据加入model中
            insertJournalTable(m_pLogBackend->jList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);

            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
        update();
    }
    break;
    case BOOT_KLU: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->jBootList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertJournalBootTable(m_pLogBackend->jBootList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case APP: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->appList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            insertApplicationTable(m_pLogBackend->appList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case KERN: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->m_type2LogData[m_flag].count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertLogTable(m_pLogBackend->m_type2LogData[m_flag], SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end, m_flag);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case DPKG: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->dList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertDpkgTable(m_pLogBackend->dList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);

            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case XORG: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->xList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            insertXorgTable(m_pLogBackend->xList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case BOOT: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->currentBootList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            insertBootTable(m_pLogBackend->currentBootList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case Kwin: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->m_type2LogData[m_flag].count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            insertLogTable(m_pLogBackend->m_type2LogData[m_flag], SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end, m_flag);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case Normal: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->nortempList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            insertNormalTable(m_pLogBackend->nortempList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case Dnf: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->dnfList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;
            insertDnfTable(m_pLogBackend->dnfList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(value);
        }
    }
    break;
    case Dmesg: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;
            int leftCnt = m_pLogBackend->dmesgList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertDmesgTable(m_pLogBackend->dmesgList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(value);
        }
    }
    break;
    case OtherLog: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;

            int leftCnt = m_pLogBackend->oList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertOOCTable(m_pLogBackend->oList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case CustomLog: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;

            int leftCnt = m_pLogBackend->cList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertOOCTable(m_pLogBackend->cList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case Audit: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;

            int leftCnt = m_pLogBackend->aList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertAuditTable(m_pLogBackend->aList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    case COREDUMP: {
        if (value < SINGLE_LOAD * rateValue - 20 || value < SINGLE_LOAD * rateValue) {
            if (m_limitTag >= rateValue)
                return;

            int leftCnt = m_pLogBackend->m_currentCoredumpList.count() - SINGLE_LOAD * rateValue;
            int end = leftCnt > SINGLE_LOAD ? SINGLE_LOAD : leftCnt;

            insertCoredumpTable(m_pLogBackend->m_currentCoredumpList, SINGLE_LOAD * rateValue, SINGLE_LOAD * rateValue + end);
            m_limitTag = rateValue;
            m_treeView->verticalScrollBar()->setValue(valuePixel);
        }
    }
    break;
    default:
        break;
    }
}

/**
 * @brief DisplayContent::slot_searchResult 搜索框执行搜索槽函数
 * @param str 要搜索的关键字
 */
void DisplayContent::slot_searchResult(const QString &str)
{
    m_pLogBackend->m_currentSearchStr = str;
    if (m_flag == NONE)
        return;

    bool bHasNext = false;
    switch (m_flag) {
    case JOURNAL: {
        m_pLogBackend->jList = m_pLogBackend->jListOrigin;
        m_pLogBackend->jList.clear();
        m_pLogBackend->jList = LogBackend::filterJournal(m_pLogBackend->m_currentSearchStr, m_pLogBackend->jListOrigin);
        //清空model和分页重新加载
        createJournalTableForm();
        createJournalTableStart(m_pLogBackend->jList);
    }
    break;
    case BOOT_KLU: {
        m_pLogBackend->jBootList.clear();
        m_pLogBackend->jBootList = LogBackend::filterJournalBoot(m_pLogBackend->m_currentSearchStr, m_pLogBackend->jBootListOrigin);
        createJournalBootTableForm();
        createJournalBootTableStart(m_pLogBackend->jBootList);
    }
    break;
    case Kwin:
    case KERN: {
        qCDebug(logDisplaycontent) << QString("search start... keyword:%1").arg(str);
        if (m_pLogBackend->m_type2Filter[m_flag].segementIndex == 0) {
            // 刚好在分段首页，直接搜索，同老逻辑一样
            m_pLogBackend->m_type2LogData[m_flag] = LogBackend::filterLog(m_pLogBackend->m_currentSearchStr, m_pLogBackend->m_type2LogDataOrigin[m_flag]);
            if (m_flag == KERN)
                createKernTableForm();
            else if (m_flag == Kwin)
                createKwinTableForm();
            createLogTable(m_pLogBackend->m_type2LogData[m_flag], m_flag);
        } else if (m_pLogBackend->m_type2Filter[m_flag].segementIndex > 0) {
            // 未在分段首页，重置索引，从头开始搜
            m_pLogBackend->m_type2Filter[m_flag].segementIndex = -1;
        }

        // 显示转圈标记，加载中...
        setLoadState(DATA_LOADING, true);

        // 开启分段加载和搜索数据
        // 若从头开始搜索，需要重置数据，相当于重新加载
        int segementIndex = loadSegementPage(true, m_pLogBackend->m_type2Filter[m_flag].segementIndex == -1);
        bHasNext = segementIndex != -1;
    }
    break;
    case BOOT: {
        m_pLogBackend->m_bootFilter.searchstr = m_pLogBackend->m_currentSearchStr;
        m_pLogBackend->currentBootList = LogBackend::filterBoot(m_pLogBackend->m_bootFilter, m_pLogBackend->bList);
        createBootTableForm();
        createBootTable(m_pLogBackend->currentBootList);
    }
    break;
    case XORG: {
        m_pLogBackend->xList.clear();
        m_pLogBackend->xList = LogBackend::filterXorg(m_pLogBackend->m_currentSearchStr, m_pLogBackend->xListOrigin);
        createXorgTableForm();
        createXorgTable(m_pLogBackend->xList);
    }
    break;
    case DPKG: {
        m_pLogBackend->dList.clear();
        m_pLogBackend->dList = LogBackend::filterDpkg(m_pLogBackend->m_currentSearchStr, m_pLogBackend->dListOrigin);
        createDpkgTableForm();
        createDpkgTableStart(m_pLogBackend->dList);
    }
    break;
    case APP: {
        m_pLogBackend->appList.clear();
        m_pLogBackend->m_appFilter.searchstr = m_pLogBackend->m_currentSearchStr;
        m_pLogBackend->appList = LogBackend::filterApp(m_pLogBackend->m_appFilter, m_pLogBackend->appListOrigin);
        createAppTableForm();
        createAppTable(m_pLogBackend->appList);
    }
    break;
    case Normal: {
        m_pLogBackend->m_normalFilter.searchstr = m_pLogBackend->m_currentSearchStr;
        m_pLogBackend->nortempList = LogBackend::filterNomal(m_pLogBackend->m_normalFilter, m_pLogBackend->norList);
        createNormalTableForm();
        createNormalTable(m_pLogBackend->nortempList);
    }
    break; // add by Airy
    case Dnf: {
        m_pLogBackend->dnfList.clear();
        m_pLogBackend->dnfList = LogBackend::filterDnf(m_pLogBackend->m_currentSearchStr, m_pLogBackend->dnfListOrigin);
        createDnfForm();
        createDnfTable(m_pLogBackend->dnfList);
    }
    break;
    case Dmesg: {
        m_pLogBackend->dmesgList.clear();
        m_pLogBackend->dmesgList = LogBackend::filterDmesg(m_pLogBackend->m_currentSearchStr, m_pLogBackend->dmesgListOrigin);
        createDmesgForm();
        createDmesgTable(m_pLogBackend->dmesgList);
    }
    break;
    case OtherLog: {
        m_pLogBackend->oList.clear();
        m_pLogBackend->oList = LogBackend::filterOOC(m_pLogBackend->m_currentSearchStr, m_pLogBackend->oListOrigin);
        createOOCTableForm();
        createOOCTable(m_pLogBackend->oList);
    }
    break;
    case CustomLog: {
        m_pLogBackend->cList.clear();
        m_pLogBackend->cList = LogBackend::filterOOC(m_pLogBackend->m_currentSearchStr, m_pLogBackend->cListOrigin);
        createOOCTableForm();
        createOOCTable(m_pLogBackend->cList);
    }
    break;
    case Audit: {
        m_pLogBackend->aList.clear();
        m_pLogBackend->m_auditFilter.searchstr = m_pLogBackend->m_currentSearchStr;
        m_pLogBackend->aList = LogBackend::filterAudit(m_pLogBackend->m_auditFilter, m_pLogBackend->aListOrigin);
        createAuditTableForm();
        createAuditTable(m_pLogBackend->aList);
    }
    break;
    case COREDUMP: {
        m_pLogBackend->m_currentCoredumpList.clear();
        m_pLogBackend->m_currentCoredumpList = LogBackend::filterCoredump(m_pLogBackend->m_currentSearchStr, m_pLogBackend->m_coredumpList);
        createCoredumpTableForm();
        createCoredumpTable(m_pLogBackend->m_currentCoredumpList);
    }
    break;
    default:
        break;
    }
    //如果搜索结果为空要显示无搜索结果提示
    if (0 == m_pModel->rowCount()) {
        if (m_pLogBackend->m_currentSearchStr.isEmpty()) {
            if (m_flag != KERN && m_flag != Kwin)
                setLoadState(DATA_COMPLETE);
        } else {
            if ((m_flag != KERN && m_flag != Kwin) || !bHasNext)
                setLoadState(DATA_NO_SEARCH_RESULT);
        }
        m_detailWgt->cleanText();
        m_detailWgt->hideLine(true);
    } else {
        if ((m_flag != KERN && m_flag != Kwin) || !bHasNext) {
            setLoadState(DATA_COMPLETE);
            m_detailWgt->hideLine(false);
        }
    }
}

/**
 * @brief DisplayContent::slot_getSubmodule 应用日志筛选子模块型的选择槽函数,根据所选子模块显示对应应用日志内容
 * @param tcbx 子模块的索引 0全部, > 0 显示指定子模块内容
 */
void DisplayContent::slot_getSubmodule(int tcbx)
{
    if (tcbx == 0)
        m_pLogBackend->m_appFilter.submodule = "";
    else {
        AppLogConfig logConfig = LogApplicationHelper::instance()->appLogConfig(m_pLogBackend->m_appFilter.app);
        int nSubModuleIndex = tcbx - 1;
        if (logConfig.subModules.size() > 0 && nSubModuleIndex < logConfig.subModules.size())
            m_pLogBackend->m_appFilter.submodule = logConfig.subModules[nSubModuleIndex].name;
    }
    m_pLogBackend->appList = LogBackend::filterApp(m_pLogBackend->m_appFilter, m_pLogBackend->appListOrigin);
    createAppTableForm();
    createAppTable(m_pLogBackend->appList);
}

/**
 * @brief DisplayContent::slot_getLogtype 开关机日志筛选开关机日志类型的选择槽函数,根据选择类型筛选当前获取到的所有开关机日志以显示
 * @param tcbx 开关机日志的类型 0全部, 1登陆 2开机 3关机
 */
void DisplayContent::slot_getLogtype(int tcbx)
{
    m_curNormalEventType = tcbx;
    m_pLogBackend->m_normalFilter.eventTypeFilter = tcbx;
    m_pLogBackend->nortempList = LogBackend::filterNomal(m_pLogBackend->m_normalFilter, m_pLogBackend->norList);
    createNormalTableForm();
    createNormalTable(m_pLogBackend->nortempList);
}

void DisplayContent::slot_getAuditType(int tcbx)
{
    m_curAuditType = tcbx;
    m_pLogBackend->m_auditFilter.auditTypeFilter = tcbx;
    m_pLogBackend->aList = LogBackend::filterAudit(m_pLogBackend->m_auditFilter, m_pLogBackend->aListOrigin);
    createAuditTableForm();
    createAuditTable(m_pLogBackend->aList);
}

/**
 * @brief DisplayContent::parseListToModel 把dpkglist加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(const QList<LOG_MSG_DPKG> &iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "dpkg parse model is empty";
        return;
    }
    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "dpkg parse model data is empty";
        return;
    }
    QList<LOG_MSG_DPKG> list = iList;
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    //每次插一行，减少刷新次数，重写QStandardItemModel有问题
    for (int i = 0; i < list.size(); i++) {
        items.clear();
        item = new DStandardItem(list[i].dateTime);
        item->setData(DPKG_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(list[i].msg);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        item->setData(DPKG_TABLE_DATA);
        items << item;
        item = new DStandardItem(list[i].action);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        item->setData(DPKG_TABLE_DATA);
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::parseListToModel 把启动日志数据list加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(const QList<LOG_MSG_BOOT> &iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "boot parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "boot parse model data is empty";
        return;
    }
    QList<LOG_MSG_BOOT> list = iList;
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    for (int i = 0; i < list.size(); i++) {
        items.clear();
        item = new DStandardItem(list[i].status);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        item->setData(BOOT_TABLE_DATA);
        items << item;
        item = new DStandardItem(list[i].msg);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        item->setData(BOOT_TABLE_DATA);
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::parseListToModel 把应用日志数据list加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(QList<LOG_MSG_APPLICATOIN> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "app log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "app log parse model data is empty";
        return;
    }
    QList<QStandardItem *> items;
    DStandardItem *item = nullptr;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        QString CH_str = m_transDict.value(iList[i].level);
        QString lvStr = CH_str.isEmpty() ? iList[i].level : CH_str;
        item = new DStandardItem();
        QString iconPath = m_iconPrefix + getIconByname(iList[i].level);
        if (getIconByname(iList[i].level).isEmpty())
            item->setText(lvStr);
        item->setIcon(QIcon(iconPath));
        item->setData(APP_TABLE_DATA);
        item->setData(lvStr, Log_Item_SPACE::levelRole);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(iList[i].dateTime);
        item->setData(APP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;
        //item = new DStandardItem(getAppName(m_curApp));
        item = new DStandardItem(iList[i].subModule);
        // 若取不到子模块名称，则显示为应用名称
        if (iList[i].subModule.isEmpty())
            item->setText(getAppName(m_curApp));
        item->setData(APP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(APP_TABLE_DATA);
        item->setData(iList[i].detailInfo, Qt::UserRole + 99);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(3));
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::parseListToModel 把xorg日志数据list加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(QList<LOG_MSG_XORG> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "xorg log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "xorg log parse model data is empty";
        return;
    }
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem(iList[i].offset);
        item->setData(XORG_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(XORG_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::parseListToModel 把开关机日志数据list加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(QList<LOG_MSG_NORMAL> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "boot-shutdown-event log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "boot-shutdown-event log parse model data is empty";
        return;
    }
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem(iList[i].eventType);
        item->setData(LAST_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(iList[i].userName);
        item->setData(LAST_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;
        item = new DStandardItem(iList[i].dateTime);
        item->setData(LAST_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(LAST_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(3));
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::parseListToModel 把kwin日志数据list加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(QList<LOG_MSG_KWIN> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "kwin log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "kwin log parse model data is empty";
        return;
    }
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem(iList[i].msg);
        item->setData(KWIN_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

void DisplayContent::parseListToModel(QList<LOG_MSG_DNF> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "dnf log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "dnf log parse model data is empty";
        return;
    }
    QList<QStandardItem *> items;
    DStandardItem *item = nullptr;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        QString CH_str = m_transDict.value(iList[i].level);
        QString lvStr = CH_str.isEmpty() ? iList[i].level : CH_str;
        item = new DStandardItem();
        QString iconPath = m_iconPrefix + m_dnfIconNameMap.value(iList[i].level);
        if (m_dnfIconNameMap.value(iList[i].level).isEmpty())
            item->setText(iList[i].level);
        item->setIcon(QIcon(iconPath));
        item->setData(DNF_TABLE_DATA);
        item->setData(lvStr, Log_Item_SPACE::levelRole);
        items << item;
        item = new DStandardItem(iList[i].dateTime);
        item->setData(DNF_TABLE_DATA);
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(DNF_TABLE_DATA);
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

void DisplayContent::parseListToModel(QList<LOG_MSG_DMESG> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "dmesg log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "dmesg log parse model data is empty";
        return;
    }
    QList<QStandardItem *> items;
    DStandardItem *item = nullptr;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem();
        QString iconPath = m_iconPrefix + getIconByname(iList[i].level);

        if (getIconByname(iList[i].level).isEmpty())
            item->setText(iList[i].level);
        item->setIcon(QIcon(iconPath));
        item->setData(DMESG_TABLE_DATA);
        item->setData(iList[i].level, Log_Item_SPACE::levelRole);
        items << item;
        item = new DStandardItem(iList[i].dateTime);
        item->setData(DMESG_TABLE_DATA);
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(DMESG_TABLE_DATA);
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

void DisplayContent::parseListToModel(QList<LOG_FILE_OTHERORCUSTOM> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "other log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "other log parse model data is empty";
        return;
    }
    QList<QStandardItem *> items;
    DStandardItem *item = nullptr;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem(QFileIconProvider().icon(QFileInfo(iList[i].path)), iList[i].name);
        item->setData(iList[i].path, Qt::UserRole + 2);
        item->setData(OOC_TABLE_DATA);
        items << item;
        item = new DStandardItem(iList[i].dateTimeModify);
        item->setData(OOC_TABLE_DATA);
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

void DisplayContent::parseListToModel(QList<LOG_MSG_AUDIT> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "audit log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "audit log parse model data is empty";
        return;
    }
    QList<QStandardItem *> items;
    DStandardItem *item = nullptr;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem(iList[i].eventType);
        item->setData(AUDIT_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;
        item = new DStandardItem(iList[i].dateTime);
        item->setData(AUDIT_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;
        item = new DStandardItem(iList[i].processName);
        item->setData(AUDIT_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        items << item;
        item = new DStandardItem(iList[i].status);
        item->setData(AUDIT_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(3));
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(AUDIT_TABLE_DATA);
        item->setData(iList[i].origin, AUDIT_ORIGIN_DATAROLE);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(4));
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

void DisplayContent::parseListToModel(QList<LOG_MSG_COREDUMP> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "coredump log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "coredump log parse model data is empty";
        return;
    }
    QList<QStandardItem *> items;
    DStandardItem *item = nullptr;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();

        item = new DStandardItem(iList[i].sig);
        item->setData(COREDUMP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(0));
        items << item;

        item = new DStandardItem(iList[i].dateTime);
        item->setData(COREDUMP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(1));
        items << item;

        item = new DStandardItem(iList[i].coreFile);
        item->setData(COREDUMP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(2));
        items << item;

        item = new DStandardItem(iList[i].userName);
        item->setData(COREDUMP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(3));
        items << item;

        item = new DStandardItem(iList[i].exe);
        item->setData(iList[i].storagePath, Qt::UserRole + 2);
        item->setData(COREDUMP_TABLE_DATA);
        item->setAccessibleText(QString("treeview_context_%1_%2").arg(i).arg(4));
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::setLoadState 设置当前的显示状态
 * @param iState 显示状态
 */
void DisplayContent::setLoadState(DisplayContent::LOAD_STATE iState, bool bSearching/* = false*/)
{
    if (!m_spinnerWgt->isHidden()) {
        m_spinnerWgt->spinnerStop();
        m_spinnerWgt->hide();
    }
    if (!m_spinnerWgt_K->isHidden()) {
        m_spinnerWgt_K->spinnerStop();
        m_spinnerWgt_K->hide();
    }
    if (!noResultLabel->isHidden()) {
        noResultLabel->hide();
    }
    if (!notAuditLabel->isHidden()) {
        notAuditLabel->hide();
    }
    if (!noCoredumpctlLabel->isHidden()) {
        noCoredumpctlLabel->hide();
    }
    if (!noPermissionLabel->isHidden()) {
        noPermissionLabel->hide();
    }

//    if (!m_treeView->isHidden()) {
//        m_treeView->hide();
//    }
    switch (iState) {
    case DATA_LOADING: {
        //如果为正在加载,则不显示主表\搜索为空的提示lable,只显示加载的转圈动画控件,并且禁止导出,导出按钮置灰
        emit setExportEnable(false);
        m_spinnerWgt->show();
        m_spinnerWgt->spinnerStart();
        if (m_flag == OtherLog || m_flag == CustomLog) {
            m_detailWgt->hide();
            m_treeView->show();
        } else {
            m_detailWgt->show();
            if (!bSearching || m_pModel->rowCount() == 0)
                m_treeView->hide();
        }
        break;
    }
    case DATA_COMPLETE: {
        //如果为加载完成,则只显示主表,导出按钮置可用
        m_treeView->show();
        m_detailWgt->show();
        emit setExportEnable(true);

        break;
    }
    case DATA_LOADING_K: {
        //如果为内核正在加载,则不显示主表\搜索为空的提示lable,只显示加载的转圈动画控件,并且禁止导出,导出按钮置灰
        emit setExportEnable(false);
        m_spinnerWgt_K->show();
        m_spinnerWgt_K->spinnerStart();
        break;
    }
    case DATA_NO_SEARCH_RESULT: {
        //如果为无搜索结果状态,则只显示无搜索结果的提示label
        m_treeView->show();
        QTimer::singleShot(50, this, [=]{
            noResultLabel->resize(m_treeView->viewport()->width(), m_treeView->viewport()->height());
            noResultLabel->show();
            noResultLabel->raise();
        });
        //搜索结果为空，导出按钮置灰
        emit setExportEnable(false);
        break;
    }
    case DATA_NOT_AUDIT_ADMIN: {
        // 开启等保四时，若当前用户不是审计管理员，给出提示
        m_treeView->show();
        notAuditLabel->resize(m_treeView->viewport()->width(), m_treeView->viewport()->height());
        notAuditLabel->show();
        notAuditLabel->raise();
        emit setExportEnable(false);
        break;
    }
    case COREDUMPCTL_NOT_INSTALLED: {
        m_treeView->show();
        noCoredumpctlLabel->resize(m_treeView->viewport()->width(), m_treeView->viewport()->height());
        noCoredumpctlLabel->show();
        noCoredumpctlLabel->raise();
        emit setExportEnable(false);
        break;
    }
    case DATA_NO_PERMISSION: {
        m_treeView->show();
        noPermissionLabel->resize(m_treeView->viewport()->width(), m_treeView->viewport()->height());
        noPermissionLabel->show();
        noPermissionLabel->raise();
        emit setExportEnable(false);
        break;
    }
    }
    this->update();
}

/**
 * @brief DisplayContent::onExportResult 导出数据结束槽函数,成功则显示提示toast
 * @param isSuccess 导出是否成功
 */
void DisplayContent::onExportResult(bool isSuccess)
{
    QString titleIcon = ICONPREFIX;
    if (m_exportDlg && !m_exportDlg->isHidden()) {
        m_exportDlg->updateProgressBarValue(0);
        m_exportDlg->hide();
    }

    if (isSuccess) {
        DMessageManager::instance()->sendMessage(this->window(), QIcon(titleIcon + "ok.svg"), DApplication::translate("ExportMessage", "Export successful"));

    } else {
        DMessageManager::instance()->sendMessage(this->window(), QIcon(titleIcon + "warning_info.svg"), DApplication::translate("ExportMessage", "Export failed"));
    }
    PERF_PRINT_END("POINT-04", "");
    DApplication::setActiveWindow(this);
}

/**
 * @brief DisplayContent::onExportFakeCloseDlg
 * doc和xls格式导出最后save之前无进度变化先关闭窗口,后续再在导出逻辑里加进度信号
 */
void DisplayContent::onExportFakeCloseDlg()
{
    if (m_exportDlg && !m_exportDlg->isHidden()) {
        m_exportDlg->hide();
    }
}

/**
 * @brief DisplayContent::clearAllDatalist 清空所有获取的数据list
 */
void DisplayContent::clearAllDatas()
{
    m_detailWgt->cleanText();
    m_pModel->clear();

    m_pLogBackend->clearAllDatalist();
}

/**
 * @brief DisplayContent::onExportProgress 导出时进度显示槽函数,连接导出数据进程
 * @param nCur 当前进行到的count
 * @param nTotal 总数量
 */
void DisplayContent::onExportProgress(int nCur, int nTotal)
{
    if (!m_exportDlg) {
        return;
    }

    //弹窗
    if (m_exportDlg->isHidden()) {
        m_exportDlg->show();
    }

    m_exportDlg->setProgressBarRange(0, nTotal);
    m_exportDlg->updateProgressBarValue(nCur);
}

/**
 * @brief DisplayContent::parseListToModel 把系统日志数据list加入model中以供treeview显示
 * @param iList 要加入model中的原始数据
 * @param oPModel 要增加数据的model指针
 */
void DisplayContent::parseListToModel(QList<LOG_MSG_JOURNAL> iList, QStandardItemModel *oPModel)
{
    if (!oPModel) {
        qCWarning(logDisplaycontent) << "system log parse model is empty";
        return;
    }

    if (iList.isEmpty()) {
        qCWarning(logDisplaycontent) << "system log parse model data is empty";
        return;
    }
    DStandardItem *item = nullptr;
    QList<QStandardItem *> items;
    int listCount = iList.size();
    for (int i = 0; i < listCount; i++) {
        items.clear();
        item = new DStandardItem(iList[i].dateTime);
        item->setData(KERN_TABLE_DATA);
        items << item;
        item = new DStandardItem(iList[i].hostName);
        item->setData(KERN_TABLE_DATA);
        items << item;
        item = new DStandardItem(iList[i].daemonName);
        item->setData(KERN_TABLE_DATA);
        items << item;
        item = new DStandardItem(iList[i].msg);
        item->setData(KERN_TABLE_DATA);
        items << item;
        oPModel->insertRow(oPModel->rowCount(), items);
    }
}

/**
 * @brief DisplayContent::resizeEvent 重载resizeEvent以让无搜索结果提示的lable大小和treeview使文字居中
 * @param event
 */
void DisplayContent::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    noResultLabel->resize(m_treeView->viewport()->width(), m_treeView->viewport()->height());
    notAuditLabel->resize(m_treeView->viewport()->width(), m_treeView->viewport()->height());
}

/**
 * @brief DisplayContent::getIconByname 获取日志等级信息对应的图标文件名
 * @param str 日志等级字符串
 * @return 日志等级图标文件名
 */
QString DisplayContent::getIconByname(const QString &str)
{
    return m_icon_name_map.value(str);
}

void DisplayContent::createBootTableForm()
{
    m_pModel->clear();
    m_pModel->setColumnCount(2);
    m_pModel->setHorizontalHeaderLabels(QStringList() << DApplication::translate("Table", "Status")
                                        << DApplication::translate("Table", "Info"));
    m_treeView->setColumnWidth(0, STATUS_WIDTH);
}

/**
 * @brief DisplayContent::insertApplicationTable 按分页转换插入应用日志到treeview的model中
 * @param logList 当前筛选状态下所有符合条件的应用日志数据结构
 * @param start 分页开始的数组下标
 * @param end 分页结束的数组下标
 */
void DisplayContent::insertApplicationTable(const QList<LOG_MSG_APPLICATOIN> &list, int start, int end)
{
    QList<LOG_MSG_APPLICATOIN> midList = list;
    if (end >= start) {
        midList = midList.mid(start, end - start);
    }
    parseListToModel(midList, m_pModel);
}

/**
 * @author Airy
 * @brief DisplayContent::slot_refreshClicked for refresh日志类型listview右键刷新数据槽函数
 * @param index 当前选中的日志类型的index
 */
void DisplayContent::slot_refreshClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    m_curListIdx = index;

    QString itemData = index.data(ITEM_DATE_ROLE).toString();
    if (itemData.isEmpty())
        return;

    if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive)) {
        // default level is info so PRIORITY=6
        m_flag = JOURNAL;
        m_pLogBackend->setFlag(m_flag);
        generateJournalFile(m_curBtnId, m_curLevel);
    } else if (itemData.contains(DPKG_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = DPKG;
        m_pLogBackend->setFlag(m_flag);
        generateDpkgFile(m_curBtnId);
    } else if (itemData.contains(XORG_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = XORG;
        m_pLogBackend->setFlag(m_flag);
        generateXorgFile(m_curBtnId);
    } else if (itemData.contains(BOOT_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = BOOT;
        m_pLogBackend->setFlag(m_flag);
        generateBootFile();
    } else if (itemData.contains(KERN_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = KERN;
        m_pLogBackend->setFlag(m_flag);
        generateKernFile(m_curBtnId);
    } else if (itemData.contains(".cache")) {
        clearAllDatas();
    } else if (itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
        clearAllDatas();
    } else if (itemData.contains(LAST_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Normal;
        m_pLogBackend->setFlag(m_flag);
        generateNormalFile(m_curBtnId);
    } else if (itemData.contains(KWIN_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Kwin;
        m_pLogBackend->setFlag(m_flag);
        KWIN_FILTERS filter;
        filter.msg = "";
        generateKwinFile(filter);
    } else if (itemData.contains(BOOT_KLU_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = BOOT_KLU;
        m_pLogBackend->setFlag(m_flag);
        generateJournalBootFile(m_curLevel);
    } else if (itemData.contains(DNF_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Dnf;
        m_pLogBackend->setFlag(m_flag);
        generateDnfFile(BUTTONID(m_curBtnId), m_curDnfLevel);
    } else if (itemData.contains(DMESG_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Dmesg;
        m_pLogBackend->setFlag(m_flag);
        generateDmesgFile(BUTTONID(m_curBtnId), PRIORITY(m_curLevel));
    } else if (itemData.contains(OTHER_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = OtherLog;
        m_pLogBackend->setFlag(m_flag);
        generateOOCLogs(OOC_OTHER);
    } else if (itemData.contains(CUSTOM_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = CustomLog;
        m_pLogBackend->setFlag(m_flag);
        generateOOCLogs(OOC_CUSTOM);
    } else if (itemData.contains(AUDIT_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = Audit;
        m_pLogBackend->setFlag(m_flag);
        generateAuditFile(m_curBtnId, m_curAuditType);
    } else if (itemData.contains(COREDUMP_TREE_DATA, Qt::CaseInsensitive)) {
        m_flag = COREDUMP;
        m_pLogBackend->setFlag(m_flag);
        generateCoredumpFile(m_curBtnId);
    }

    if (!itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive) || !itemData.contains(KERN_TREE_DATA, Qt::CaseInsensitive)) { // modified by Airy
    }
}

void DisplayContent::slot_dnfLevel(DNFPRIORITY iLevel)
{
    m_curDnfLevel = iLevel;
    generateDnfFile(BUTTONID(m_curBtnId), m_curDnfLevel);
}

void DisplayContent::generateOOCFile(const QString &path)
{
    setLoadState(DATA_LOADING);
    m_detailWgt->cleanText();
    m_isDataLoadComplete = false;
    m_pLogBackend->parseByOOC(path);
}

void DisplayContent::generateOOCLogs(const OOC_TYPE &type, const QString &iSearchStr/* = ""*/)
{
    m_pLogBackend->clearAllFilter();
    clearAllDatas();

    QList<QStringList> files;
    QList<LOG_FILE_OTHERORCUSTOM>* pListOrigin = nullptr;
    QList<LOG_FILE_OTHERORCUSTOM>* pList = nullptr;

    if (type == OOC_OTHER) {
        files = LogApplicationHelper::instance()->getOtherLogList();
        pListOrigin = &(m_pLogBackend->oListOrigin);
        pList = &(m_pLogBackend->oList);
    }
    else {
        files = LogApplicationHelper::instance()->getCustomLogList();
        pListOrigin = &(m_pLogBackend->cListOrigin);
        pList = &(m_pLogBackend->cList);
    }

    for (QStringList iter : files) {
        LOG_FILE_OTHERORCUSTOM logFileInfo;
        logFileInfo.name = iter.value(0);
        logFileInfo.path = iter.value(1);
        logFileInfo.dateTimeModify = QFileInfo(iter.value(1)).lastModified().toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
        pListOrigin->append(logFileInfo);
    }

    *pList = LogBackend::filterOOC(iSearchStr, *pListOrigin);

    createOOCTableForm();
    createOOCTable(*pList);
}

void DisplayContent::createOOCTableForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "File Name")
                                        << DApplication::translate("Table", "Time Modified"));
    m_treeView->setColumnWidth(0, NAME_WIDTH);
    m_treeView->setColumnWidth(1, DATETIME_WIDTH + 120);
}

void DisplayContent::createOOCTable(const QList<LOG_FILE_OTHERORCUSTOM> &list)
{
    setLoadState(DATA_COMPLETE);

    m_limitTag = 0;
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertOOCTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);

    m_curTreeIndex = QModelIndex();//重置一下
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::generateAuditFile(int id, int lId, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr);
    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    setLoadState(DATA_LOADING);
    createAuditTableForm();
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    AUDIT_FILTERS auditFilter;
    auditFilter.auditTypeFilter = lId;

    switch (id) {
    case ALL: {
    }
    break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        auditFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        auditFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        auditFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        auditFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        auditFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        auditFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        auditFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        auditFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        auditFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        auditFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    m_pLogBackend->m_auditFilter = auditFilter;

    if (id >= ALL && id <= THREE_MONTHS)
        m_pLogBackend->parseByAudit(auditFilter);
}

void DisplayContent::createAuditTableForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "Event Type")
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "Process")
                                        << DApplication::translate("Table", "Status")
                                        << DApplication::translate("Table", "Info"));
#ifndef SHOW_DETAIL
    m_treeView->setColumnWidth(AUDIT_SPACE::auditEventTypeColumn, 125);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditDateTimeColumn, 140);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditProcessNameColumn, 100);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditStatusColumn, 55);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditMsgColumn, DATETIME_WIDTH);
#else
    m_treeView->setColumnWidth(AUDIT_SPACE::auditEventTypeColumn, DATETIME_WIDTH + 20);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditDateTimeColumn, DATETIME_WIDTH);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditProcessNameColumn, DATETIME_WIDTH);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditStatusColumn, STATUS_WIDTH);
    m_treeView->setColumnWidth(AUDIT_SPACE::auditMsgColumn, DATETIME_WIDTH);
    m_treeView->hideColumn(AUDIT_SPACE::auditMsgColumn);
#endif
}

void DisplayContent::createAuditTable(const QList<LOG_MSG_AUDIT> &list)
{
    setLoadState(DATA_COMPLETE);

    m_limitTag = 0;
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertAuditTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}

void DisplayContent::generateCoredumpFile(int id, const QString &iSearchStr)
{
    Q_UNUSED(iSearchStr)

    if (!Utils::isCoredumpctlExist()) {
        setLoadState(COREDUMPCTL_NOT_INSTALLED);
        return;
    }

    m_pLogBackend->clearAllFilter();
    clearAllDatas();
    m_firstLoadPageData = true;
    m_isDataLoadComplete = false;
    setLoadState(DATA_LOADING);
    createCoredumpTableForm();
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime(QTime()); // get zero time
    COREDUMP_FILTERS coreFilter;

    switch (id) {
    case ALL:
        break;
    case ONE_DAY: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        coreFilter.timeFilterBegin = dtStart.toMSecsSinceEpoch();
        coreFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        coreFilter.timeFilterBegin = dtStart.addDays(-2).toMSecsSinceEpoch();
        coreFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        coreFilter.timeFilterBegin = dtStart.addDays(-6).toMSecsSinceEpoch();
        coreFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        coreFilter.timeFilterBegin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        coreFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    case THREE_MONTHS: {
        QDateTime dtStart = dt;
        QDateTime dtEnd = dt;
        dtEnd.setTime(QTime(23, 59, 59, 999));
        coreFilter.timeFilterBegin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        coreFilter.timeFilterEnd = dtEnd.toMSecsSinceEpoch();
    }
    break;
    default:
        break;
    }

    if (id >= ALL && id <= THREE_MONTHS)
        m_pLogBackend->parseByCoredump(coreFilter);
}
void DisplayContent::createCoredumpTableForm()
{
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << DApplication::translate("Table", "SIG")
                                        << DApplication::translate("Table", "Date and Time")
                                        << DApplication::translate("Table", "Core File")
                                        << DApplication::translate("Table", "User Name ")
                                        << DApplication::translate("Table", "EXE"));

    m_treeView->setColumnWidth(COREDUMP_SPACE::COREDUMP_SIG_COLUMN, 110);
    m_treeView->setColumnWidth(COREDUMP_SPACE::COREDUMP_TIME_COLUMN, 150);
    m_treeView->setColumnWidth(COREDUMP_SPACE::COREDUMP_COREFILE_COLUMN, 100);
    m_treeView->setColumnWidth(COREDUMP_SPACE::COREDUMP_UNAME_COLUMN, 100);
    m_treeView->setColumnWidth(COREDUMP_SPACE::COREDUMP_EXE_COLUMN, 135);
}
void DisplayContent::createCoredumpTable(const QList<LOG_MSG_COREDUMP> &list)
{
    setLoadState(DATA_COMPLETE);

    m_limitTag = 0;
    int end = list.count() > SINGLE_LOAD ? SINGLE_LOAD : list.count();
    insertCoredumpTable(list, 0, end);
    QItemSelectionModel *p = m_treeView->selectionModel();
    if (p)
        p->select(m_pModel->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    slot_tableItemClicked(m_pModel->index(0, 0));
}
void DisplayContent::slot_requestShowRightMenu(const QPoint &pos)
{
    if (m_flag != OtherLog && m_flag != CustomLog && m_flag != COREDUMP) {
        return;
    }

    if (m_treeView->indexAt(pos).isValid()) {
        QModelIndex index = m_treeView->currentIndex();
        if (!m_treeView->selectionModel()->selectedIndexes().empty()) {

            QString path;
            if (m_flag == COREDUMP) {
                if (index.siblingAtColumn(2).data().toString() == "missing") {
                    // 文件状态为missing，不能在文管中打开
                    m_act_openForder->setEnabled(false);
                } else {
                    m_act_openForder->setEnabled(true);
                }
                //coredump文件不需要刷新
                m_act_refresh->setEnabled(false);

                path = m_pModel->item(index.row(), 4)->data(Qt::UserRole + 2).toString();
            } else {
                path = m_pModel->item(index.row(), 0)->data(Qt::UserRole + 2).toString();
            }

            //显示当前日志目录
            m_act_openForder->disconnect();
            connect(m_act_openForder, &QAction::triggered, this, [ = ] {
                DDesktopServices::showFileItem(path);
            });

            //刷新逻辑
            m_act_refresh->disconnect();
            connect(m_act_refresh, &QAction::triggered, this, [ = ]() {
                generateOOCFile(path);
            });

            m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
            m_menu->exec(QCursor::pos());
        }
    }
}

void DisplayContent::slot_valueChanged_dConfig_or_gSetting(const QString &key)
{
    if ((key == "customLogFiles" || key == "customlogfiles") && m_flag == CustomLog) {
        generateOOCLogs(OOC_CUSTOM);
    }
}
