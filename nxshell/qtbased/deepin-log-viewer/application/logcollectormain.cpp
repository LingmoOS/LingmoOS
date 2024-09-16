// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logcollectormain.h"
#include "logsettings.h"
#include "DebugTimeManager.h"
#include "utils.h"
#include "logallexportthread.h"
#include "exportprogressdlg.h"

#include "dbusmanager.h"

#include <sys/utsname.h>

#include <DApplication>
#include <DTitlebar>
#include <DWindowOptionButton>
#include <DWindowCloseButton>
#include <DMessageManager>

#include <QDateTime>
#include <QDebug>
#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSizePolicy>
#include <QList>
#include <QKeyEvent>

#include <DAboutDialog>
#include <DFileDialog>

//958+53+50 976
//日志类型选择器宽度
#define LEFT_LIST_WIDTH 200
DWIDGET_USE_NAMESPACE

//刷新间隔
static const QString refreshIntervalKey = "base.RefreshInterval";
/**
 * @brief LogCollectorMain::LogCollectorMain 构造函数
 * @param parent 父对象
 */
LogCollectorMain::LogCollectorMain(QWidget *parent)
    : DMainWindow(parent)
{
    qRegisterMetaType<DNFPRIORITY>("DNFPRIORITY");
    initUI();
    initConnection();

    initShortCut();
    //日志类型选择器选第一个
    m_logCatelogue->setDefaultSelect();
    //设置最小窗口尺寸
    setMinimumSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT);
    //恢复上次关闭时记录的窗口大小
    resize(LogSettings::instance()->getConfigWinSize());
    Utils::m_mapAuditType2EventType = LogSettings::instance()->loadAuditMap();
}

/**
 * @brief LogCollectorMain::~LogCollectorMain 析构函数
 */
LogCollectorMain::~LogCollectorMain()
{
    /** delete when app quit */
    if (m_searchEdt) {
        delete m_searchEdt;
        m_searchEdt = nullptr;
    }

    if (m_backend) {
        delete m_backend;
        m_backend = nullptr;
    }
    //如果窗体状态不是最大最小状态，则记录此时窗口尺寸到配置文件里，方便下次打开时恢复大小
    if (windowState() == Qt::WindowNoState) {
        LogSettings::instance()->saveConfigWinSize(width(), height());
    }
}

/**
 * @brief LogCollectorMain::initUI 初始化界面
 */
void LogCollectorMain::initUI()
{
    /** add searchEdit */
    m_searchEdt = new DSearchEdit();

    m_searchEdt->setPlaceHolder(DApplication::translate("SearchBar", "Search"));
    m_searchEdt->setMaximumWidth(400);
    titlebar()->setCustomWidget(m_searchEdt, true);
    /** add titleBar */
    titlebar()->setIcon(QIcon::fromTheme("deepin-log-viewer"));
    titlebar()->setTitle("");
#ifdef SPLITTER_TYPE

    //m_hLayout = new QHBoxLayout;
    m_hSplitter = new Dtk::Widget::DSplitter(this);
    m_hSplitter->setOrientation(Qt::Horizontal);
    /** left frame */
    m_logCatelogue = new LogListView(this);
    m_logCatelogue->setMaximumWidth(LEFT_LIST_WIDTH);
    m_logCatelogue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_hSplitter->addWidget(m_logCatelogue);
    m_hSplitter->setStretchFactor(0, 1);

    m_vLayout = new QVBoxLayout;
    /** topRight frame */
    m_topRightWgt = new FilterContent();
    // m_topRightWgt->setMinimumWidth(8000);
    // m_topRightWgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_vLayout->addWidget(m_topRightWgt);
    /** midRight frame */
    m_midRightWgt = new DisplayContent(this);
    m_vLayout->addWidget(m_midRightWgt, 1);
    m_vLayout->setContentsMargins(0, 10, 0, 10);
    m_vLayout->setSpacing(6);
    DWidget *rightWidget = new DWidget(this);
    //撑开右侧控件
    rightWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    rightWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    rightWidget->setContentsMargins(10, 0, 0, 0);
    rightWidget->setLayout(m_vLayout);
    // rightWidget->setMinimumWidth(693);
    m_hSplitter->addWidget(rightWidget);
    m_hSplitter->setStretchFactor(1, 10);
    m_hSplitter->setOpaqueResize(true);
    m_hSplitter->setContentsMargins(0, 0, 10, 0);
    m_hSplitter->setChildrenCollapsible(false);
    m_hSplitter->setSizes(QList<int>() << 200);
    //m_hSplitter->setSpacing(10);
    this->setCentralWidget(m_hSplitter);
    // this->centralWidget()->setLayout(m_hSplitter);

#else

    this->setCentralWidget(new DWidget());

    m_hLayout = new QHBoxLayout();

    /** left frame */
    m_logCatelogue = new LogListView();
    m_logCatelogue->setObjectName("logTypeSelectList");
    m_logCatelogue->setAccessibleName("left_side_bar");
    m_hLayout->addWidget(m_logCatelogue, 1);
    m_logCatelogue->setFixedWidth(160);
    m_vLayout = new QVBoxLayout;
    /** topRight frame */
    m_topRightWgt = new FilterContent();
    m_topRightWgt->setAccessibleName("filterWidget");
    m_vLayout->addWidget(m_topRightWgt);
    /** midRight frame */
    m_midRightWgt = new DisplayContent();

    m_vLayout->addWidget(m_midRightWgt, 1);
    m_vLayout->setContentsMargins(0, 10, 0, 10);
    m_vLayout->setSpacing(6);

    m_hLayout->addLayout(m_vLayout, 10);
    m_hLayout->setContentsMargins(0, 0, 10, 0);
    m_hLayout->setSpacing(10);

    this->centralWidget()->setAccessibleName("centralWidget");
    this->centralWidget()->setLayout(m_hLayout);
    m_searchEdt->setObjectName("searchEdt");
    m_searchEdt->lineEdit()->setObjectName("searchChildEdt");
    m_topRightWgt->setObjectName("FilterContent");
    m_midRightWgt->setObjectName("DisplayContent");
    titlebar()->setObjectName("titlebar");
    initTitlebarExtensions();

#endif

    m_originFilterWidth = m_topRightWgt->geometry().width();
}

void LogCollectorMain::initTitlebarExtensions()
{
    DMenu *refreshMenu = new DMenu(this);
    DMenu *menu = new DMenu(DApplication::translate("titlebar", "Refresh interval"), refreshMenu);
    menu->setAccessibleName("refresh_interval_menu");
    m_refreshActions.push_back(menu->addAction(qApp->translate("titlebar", "10 sec")));
    m_refreshActions.push_back(menu->addAction(qApp->translate("titlebar", "1 min")));
    m_refreshActions.push_back(menu->addAction(qApp->translate("titlebar", "5 min")));
    m_refreshActions.push_back(menu->addAction(qApp->translate("titlebar", "No refresh")));

    QActionGroup *group = new QActionGroup(menu);
    for (auto &it : m_refreshActions) {
        it->setCheckable(true);
        group->addAction(it);
    }

    QObject::connect(group, &QActionGroup::triggered,
                     this, &LogCollectorMain::switchRefreshActionTriggered);
    refreshMenu->addMenu(menu);
    titlebar()->setMenu(refreshMenu);
    //获取配置
    initSettings();
    //设置刷新频率
    QVariant var = m_settings->getOption(refreshIntervalKey);
    if (!var.isNull()) {
        int index = var.toInt();
        m_refreshActions[index]->setChecked(true);
        m_refreshActions[index]->triggered(true);
    }
    DWidget *widget = new DWidget;
    QHBoxLayout *layout = new QHBoxLayout(widget);
    m_exportAllBtn = new DIconButton(widget);
    m_exportAllBtn->setFixedSize(QSize(36, 36));
    m_exportAllBtn->setIcon(QIcon::fromTheme("export"));
    m_exportAllBtn->setIconSize(QSize(36, 36));
    m_exportAllBtn->setToolTip(qApp->translate("titlebar", "Export All"));
    m_exportAllBtn->setAccessibleName(qApp->translate("titlebar", "Export All"));
    m_refreshBtn = new DIconButton(widget);
    m_refreshBtn->setIcon(QIcon::fromTheme("refresh"));
    m_refreshBtn->setFixedSize(QSize(36, 36));
    m_refreshBtn->setIconSize(QSize(36, 36));
    m_refreshBtn->setToolTip(qApp->translate("titlebar", "Refresh Now"));
    m_refreshBtn->setAccessibleName(qApp->translate("titlebar", "Refresh Now"));
    layout->addSpacing(115);
    layout->addWidget(m_exportAllBtn);
    layout->addSpacing(2);
    layout->addWidget(m_refreshBtn);
    titlebar()->addWidget(widget, Qt::AlignLeft);
    connect(m_refreshBtn, &QPushButton::clicked, this, [ = ] {
        m_topRightWgt->setLeftButtonState(true);
        m_topRightWgt->setChangedcomboxstate(false);
        emit m_logCatelogue->sigRefresh(m_logCatelogue->currentIndex());
    });
    connect(m_exportAllBtn, &QPushButton::clicked, this, &LogCollectorMain::exportAllLogs);
}

void LogCollectorMain::switchRefreshActionTriggered(QAction *action)
{
    int index = m_refreshActions.indexOf(action);
    int timeInterval = 0;
    switch (index) {
    case 0:
        timeInterval = 10 * 1000; //10秒刷新
        break;
    case 1:
        timeInterval = 60 * 1000; //1分钟刷新
        break;
    case 2:
        timeInterval = 5 * 60 * 1000; //5分钟刷新
        break;
    default:
        break;
    }
    //先停止刷新
    if (m_refreshTimer && m_refreshTimer->isActive()) {
        m_refreshTimer->stop();
    }
    //开启定时器刷新
    if (timeInterval > 0) {
        if (nullptr == m_refreshTimer) {
            m_refreshTimer = new QTimer(this);
            connect(m_refreshTimer, &QTimer::timeout, this, [ = ] {
                m_topRightWgt->setLeftButtonState(true);
                m_topRightWgt->setChangedcomboxstate(false);
                //触发刷新信号
                emit m_logCatelogue->sigRefresh(m_logCatelogue->currentIndex());
                m_logCatelogue->setFocus();
            });
        }
        m_refreshTimer->start(timeInterval);
    }
    //同步刷新配置参数
    if (m_settings && index >= 0) {
        m_settings->setOption(refreshIntervalKey, index);
        m_settings->sync();
    }
}

void LogCollectorMain::initSettings()
{
    m_settings = DSettings::fromJsonFile(":/deepin-log-viewer-setting.json");
    QString configpath = Utils::getConfigPath();
    QDir dir(configpath);
    if (!dir.exists()) {
        Utils::mkMutiDir(configpath);
    };
    m_backend = new QSettingBackend(dir.filePath("config.conf"));
    m_settings->setBackend(m_backend);
}

void LogCollectorMain::exportAllLogs()
{
    static bool authorization = false;
    if (false == authorization) {
        QString policyActionId = "";
        // 开启等保四，若当前用户是审计管理员，使用单用户审计管理员鉴权
        if (DBusManager::isSEOpen() && DBusManager::isAuditAdmin())
            policyActionId = "com.deepin.pkexec.logViewerAuth.exportLogsSelf";
        else
            // 其他情况，默认为多用户鉴权
            policyActionId = "com.deepin.pkexec.logViewerAuth.exportLogs";

        if (!Utils::checkAuthorization(policyActionId, qApp->applicationPid())) {
            return;
        }
        authorization = true;
    }

    // 时间
    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    // 主机名
    utsname _utsname;
    uname(&_utsname);
    QString hostname = QString(_utsname.nodename);

    static QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileFullPath = defaultDir + "/" + QString("%1_%2_all_logs.zip").arg(dateTime).arg(hostname);
    QString newPath = DFileDialog::getSaveFileName(this, "", fileFullPath, "*.zip");
    if (newPath.isEmpty()) {
        return;
    }

    // 导出路径白名单检查
    QFileInfo info(newPath);
    QString outPath = info.path();
    QStringList availablePaths =  DLDBusHandler::instance(this)->whiteListOutPaths();

    bool bAvailable = false;
    for (auto path : availablePaths) {
        if (outPath.startsWith(path)) {
            bAvailable = true;
            break;
        }
    }
    if (!bAvailable) {
        QString titleIcon = ICONPREFIX;
        DMessageManager::instance()->sendMessage(this->window(), QIcon(titleIcon + "warning_info.svg"), DApplication::translate("ExportMessage", "The export directory is not available. Please choose another directory for the export operation."));
        return;
    }

    //添加文件后缀
    if (!newPath.endsWith(".zip")) {
        newPath += ".zip";
    }

    if (m_exportDlg == nullptr) {
        m_exportDlg = new ExportProgressDlg(this);
        DLDBusHandler::instance(this);
    }
    //导出是否完成
    bool exportcomplete = false;
    LogAllExportThread *thread = new LogAllExportThread(m_logCatelogue->getLogTypes(), newPath);
    thread->setAutoDelete(true);
    connect(thread, &LogAllExportThread::updateTolProcess, this, [ = ](int tol) {
        m_exportDlg->setProgressBarRange(0, tol);
    });
    connect(thread, &LogAllExportThread::updatecurrentProcess, this, [ = ](int cur) {
        m_exportDlg->updateProgressBarValue(cur);
    });
    connect(thread, &LogAllExportThread::exportFinsh, this, [&exportcomplete, this](bool ret) {
        exportcomplete = true;
        m_exportDlg->close();
        m_midRightWgt->onExportResult(ret);
    });
    QThreadPool::globalInstance()->start(thread);
    m_exportDlg->exec();
    if (!exportcomplete) {
        thread->slot_cancelExport();
    }
}
/**
 * @brief LogCollectorMain::initConnection 连接信号槽
 */
void LogCollectorMain::initConnection()
{
    //! search
    connect(m_searchEdt, &DSearchEdit::textChanged, m_midRightWgt,
            &DisplayContent::slot_searchResult);

    //! filter widget
    connect(m_topRightWgt, SIGNAL(sigButtonClicked(int, int, QModelIndex)), m_midRightWgt,
            SLOT(slot_BtnSelected(int, int, QModelIndex)));
    connect(m_topRightWgt, &FilterContent::sigDnfLvlChanged, m_midRightWgt,
            &DisplayContent::slot_dnfLevel);
    connect(m_topRightWgt, &FilterContent::sigCbxAppIdxChanged, m_midRightWgt,
            &DisplayContent::slot_appLogs);
    connect(m_topRightWgt, &FilterContent::sigCbxSubModuleChanged, m_midRightWgt,
            &DisplayContent::slot_getSubmodule); // add by Airy

    connect(m_topRightWgt, &FilterContent::sigExportInfo, m_midRightWgt,
            &DisplayContent::slot_exportClicked);

    connect(m_topRightWgt, &FilterContent::sigStatusChanged, m_midRightWgt,
            &DisplayContent::slot_statusChagned);

    connect(m_topRightWgt, &FilterContent::sigLogtypeChanged, m_midRightWgt,
            &DisplayContent::slot_getLogtype); // add by Airy

    connect(m_topRightWgt, &FilterContent::sigAuditTypeChanged, m_midRightWgt,
            &DisplayContent::slot_getAuditType);

    connect(m_topRightWgt, &FilterContent::sigCbxAppIdxChanged, m_logCatelogue,
            &LogListView::slot_getAppPath); // add by Airy for getting app path
    connect(m_midRightWgt, &DisplayContent::setExportEnable, m_topRightWgt,
            &FilterContent::setExportButtonEnable, Qt::DirectConnection);
    //自适应宽度
    connect(m_logCatelogue, SIGNAL(sigRefresh(const QModelIndex &)), m_midRightWgt,
            SLOT(slot_refreshClicked(const QModelIndex &))); // add by Airy for adding refresh
    connect(m_logCatelogue, SIGNAL(sigRefresh(const QModelIndex &)), m_topRightWgt,
            SLOT(slot_logCatelogueRefresh(const QModelIndex &)));

    connect(m_logCatelogue, &LogListView::sigRefresh, this, &LogCollectorMain::slotClearInfoandFocus);
    //! treeView widget

    connect(m_logCatelogue, SIGNAL(itemChanged(const QModelIndex &)), m_midRightWgt,
            SLOT(slot_logCatelogueClicked(const QModelIndex &)));

    //! set tree <==> combobox visible
    connect(m_logCatelogue, SIGNAL(itemChanged(const QModelIndex &)), m_topRightWgt,
            SLOT(slot_logCatelogueClicked(const QModelIndex &)));

    // when item changed clear search text
    connect(m_logCatelogue, &LogListView::itemChanged, this, &LogCollectorMain::slotClearInfoandFocus);
    connect(m_topRightWgt, &FilterContent::sigButtonClicked, this, &LogCollectorMain::slotClearInfoandFocus);
    connect(m_topRightWgt, &FilterContent::sigCbxAppIdxChanged, this, &LogCollectorMain::slotClearInfoandFocus);

    //dnf日志下拉框选项切换清空搜索栏
    connect(m_topRightWgt, &FilterContent::sigDnfLvlChanged, this, &LogCollectorMain::slotClearInfoandFocus);
    //启动日志下拉框选项切换清空搜索栏
    connect(m_topRightWgt, &FilterContent::sigStatusChanged, this, &LogCollectorMain::slotClearInfoandFocus);
    //开关机日志下拉框选项切换清空搜索栏
    connect(m_topRightWgt, &FilterContent::sigLogtypeChanged, this, &LogCollectorMain::slotClearInfoandFocus);
}

void LogCollectorMain::slotClearInfoandFocus()
{
    m_searchEdt->clearEdit();
    if (!m_topRightWgt->getChangedcomboxstate() && m_topRightWgt->getLeftButtonState()) {
        m_topRightWgt->setLeftButtonState(false);
    } else if (m_topRightWgt->getChangedcomboxstate()) {
        m_logCatelogue->setFocus();
    }
}

/**
 * @brief LogCollectorMain::initShortCut 初始化快捷键
 */
void LogCollectorMain::initShortCut()
{
    // Resize Window --> Ctrl+Alt+F
    if (nullptr == m_scWndReize) {
        m_scWndReize = new QShortcut(this);
        m_scWndReize->setKey(Qt::CTRL + Qt::ALT + Qt::Key_F);
        m_scWndReize->setContext(Qt::ApplicationShortcut);
        m_scWndReize->setAutoRepeat(false);

        connect(m_scWndReize, &QShortcut::activated, this, [this] {
            if (this->windowState() & Qt::WindowMaximized)
            {
                this->showNormal();
            } else if (this->windowState() == Qt::WindowNoState)
            {
                this->showMaximized();
            }
        });
    }

    // Find font --> Ctrl+F
    if (nullptr == m_scFindFont) {
        m_scFindFont = new QShortcut(this);
        m_scFindFont->setKey(Qt::CTRL + Qt::Key_F);
        m_scFindFont->setContext(Qt::ApplicationShortcut);
        m_scFindFont->setAutoRepeat(false);

        connect(m_scFindFont, &QShortcut::activated, this,
                [this] { this->m_searchEdt->lineEdit()->setFocus(Qt::MouseFocusReason); });
    }

    // export file --> Ctrl+E
    if (nullptr == m_scExport) {
        m_scExport = new QShortcut(this);
        m_scExport->setKey(Qt::CTRL + Qt::Key_E);
        m_scExport->setContext(Qt::ApplicationShortcut);
        m_scExport->setAutoRepeat(false);

        connect(m_scExport, &QShortcut::activated, this,
                [this] { this->m_topRightWgt->shortCutExport(); });
    }
}

/**
 * @brief LogCollectorMain::handleApplicationTabEventNotify
 * 处理application中notify的tab keyevent ,直接在dapplication中调用
 * 只调整我们需要调整的顺序,其他的默认
 * @param obj 接收事件的对象
 * @param evt 对象接收的键盘事件
 * @return true处理并屏蔽事件 false交给application的notify处理
 */
bool LogCollectorMain::handleApplicationTabEventNotify(QObject *obj, QKeyEvent *evt)
{
    if (evt->key() == Qt::Key_Tab) {
        DWindowCloseButton *closebtn = this->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
        if (obj == this->titlebar()) {
            return false;
        } else if (obj->objectName() == "searchChildEdt") {
            return false;
        } else if (obj == closebtn) {
            m_logCatelogue->setFocus(Qt::TabFocusReason);
            return true;
        } else if (obj->objectName() == "mainLogTable") {
            return false;
        }
    } else if (evt->key() == Qt::Key_Backtab) {
        DWindowOptionButton *optionbtn = this->titlebar()->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
        if (obj->objectName() == "logTypeSelectList") {
            DWindowCloseButton *closeButton = titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
            if (closeButton) {
                closeButton->setFocus();
            }
            return true;
        } else if (obj == optionbtn) {
            return false;
        } else if (obj->objectName() == "searchChildEdt") {
            m_midRightWgt->mainLogTableView()->setFocus(Qt::BacktabFocusReason);
            return true;
        }
    }
    return false;
}

void LogCollectorMain::closeEvent(QCloseEvent *event)
{
    PERF_PRINT_BEGIN("POINT-02", "");
    DMainWindow::closeEvent(event);
}
