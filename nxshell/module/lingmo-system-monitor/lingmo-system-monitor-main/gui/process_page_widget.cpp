// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_page_widget.h"

#include "application.h"
#include "main_window.h"
#include "kill_process_confirm_dialog.h"
#include "xwin_kill_preview_widget.h"
#include "monitor_compact_view.h"
#include "monitor_expand_view.h"
#include "process_table_view.h"
#include "settings.h"
#include "ui_common.h"
#include "common/common.h"
#include "common/perf.h"
#include "process/process_set.h"
#include "process/process_db.h"
#include "wm/wm_window_list.h"
#include "detail_view_stacked_widget.h"
#include "system/cpu_set.h"
#include "common/eventlogutils.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DLabel>
#include <DStackedWidget>

#include <QDebug>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QPainterPath>
#include <QSizePolicy>
//loading显示时间（ms）
#define NORMAL_PERFORMANCE_CPU_LOADING_TIME 100
#define CPU_FREQUENCY_STANDARD "2.30GHz"

DWIDGET_USE_NAMESPACE
using namespace core::process;
using namespace common::init;
// process context summary text
static const char *kProcSummaryTemplateText =
        QT_TRANSLATE_NOOP("Process.Summary", "(%1 applications and %2 processes are running)");

// application context text
static const char *appText = QT_TRANSLATE_NOOP("Process.Show.Mode", "Applications");
// my process context mode text
static const char *myProcText = QT_TRANSLATE_NOOP("Process.Show.Mode", "My processes");
// all process context mode text
static const char *allProcText = QT_TRANSLATE_NOOP("Process.Show.Mode", "All processes");
// process loading text
static const char *loadingText = QT_TRANSLATE_NOOP("Process.Loading", "Loading");

// constructor
ProcessPageWidget::ProcessPageWidget(DWidget *parent)
    : DFrame(parent)
{
    // initialize global settings
    m_settings = Settings::instance();
    // initialize ui components
    initUI();
    initConnections();
}

// destructor
ProcessPageWidget::~ProcessPageWidget() {}

// initialize ui components
void ProcessPageWidget::initUI()
{
    // global app helper instance
    auto *dAppHelper = DApplicationHelper::instance();
    // global palette
    auto palette = dAppHelper->applicationPalette();

    // content margin
    int margin = 10;

    m_rightStackView = new DetailViewStackedWidget(this);

    // tool area background
    auto *tw = new QWidget(this);
    // content area background
    m_processWidget = new QWidget(this);
    m_rightStackView->addProcessWidget(m_processWidget);

    // left =====> stackview
    m_views = new DStackedWidget(this);
    m_views->setAutoFillBackground(false);
    m_views->setContentsMargins(0, 0, 0, 0);
    m_views->setFixedWidth(common::getStatusBarMaxWidth());

    // right ====> tab button + process table
    auto *contentlayout = new QVBoxLayout(m_processWidget);
    contentlayout->setSpacing(margin);
    contentlayout->setContentsMargins(0, 0, 0, 0);

    // tools area layout
    auto *toolsLayout = new QHBoxLayout(tw);
    toolsLayout->setSpacing(margin);
    toolsLayout->setContentsMargins(0, 0, 0, 0);

    // process context instance
    m_procViewMode = new DLabel(tw);
    m_procViewMode->setText(DApplication::translate("Process.Show.Mode", appText));   // default text
    DFontSizeManager::instance()->bind(m_procViewMode, DFontSizeManager::T7, QFont::Medium);
    // text aligment
    m_procViewMode->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // change text color to text title style
    auto pam = DApplicationHelper::instance()->palette(m_procViewMode);
    palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
    m_procViewMode->setPalette(palette);

    // process context summary instance
    m_procViewModeSummary = new DLabel(tw);
    DFontSizeManager::instance()->bind(m_procViewModeSummary, DFontSizeManager::T7, QFont::Medium);
    // text aligment & elide mode
    m_procViewModeSummary->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_procViewModeSummary->setElideMode(Qt::ElideRight);
    // change text color to text tips style
    auto pa = DApplicationHelper::instance()->palette(m_procViewModeSummary);
    palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
    m_procViewModeSummary->setPalette(palette);

    // change icon type when theme changed
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this,
            &ProcessPageWidget::changeIconTheme);

    auto *modeButtonGroup = new DButtonBox(tw);
    modeButtonGroup->setFixedWidth(30 * 3);
    modeButtonGroup->setFixedHeight(26);

    // show application mode button
    m_appButton = new DButtonBoxButton(QIcon(), {}, modeButtonGroup);
    m_appButton->setIconSize(QSize(26, 24));
    m_appButton->setCheckable(true);
    m_appButton->setFocusPolicy(Qt::TabFocus);
    m_appButton->setToolTip(DApplication::translate("Process.Show.Mode", appText));
    m_appButton->setAccessibleName(m_appButton->toolTip());

    // show my process mode button
    m_myProcButton = new DButtonBoxButton(QIcon(), {}, modeButtonGroup);
    m_myProcButton->setIconSize(QSize(26, 24));
    m_myProcButton->setCheckable(true);
    m_myProcButton->setFocusPolicy(Qt::TabFocus);
    m_myProcButton->setToolTip(DApplication::translate("Process.Show.Mode", myProcText));
    m_myProcButton->setAccessibleName(m_myProcButton->toolTip());

    // show all process mode button
    m_allProcButton = new DButtonBoxButton(QIcon(), {}, modeButtonGroup);
    m_allProcButton->setIconSize(QSize(26, 24));
    m_allProcButton->setCheckable(true);
    m_allProcButton->setFocusPolicy(Qt::TabFocus);
    m_allProcButton->setToolTip(DApplication::translate("Process.Show.Mode", allProcText));
    m_allProcButton->setAccessibleName(m_allProcButton->toolTip());

    // install event filters to handle left/right direction key press
    m_appButton->installEventFilter(this);
    m_myProcButton->installEventFilter(this);
    m_allProcButton->installEventFilter(this);

    // change icon type based on current theme when initialized
    changeIconTheme(dAppHelper->themeType());

    QList<DButtonBoxButton *> list;
    list << m_appButton << m_myProcButton << m_allProcButton;
    modeButtonGroup->setButtonList(list, true);

    // add widgets to tools layout
    toolsLayout->addWidget(m_procViewMode, 0, Qt::AlignLeft | Qt::AlignVCenter);
    toolsLayout->addWidget(m_procViewModeSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    toolsLayout->addStretch();
    toolsLayout->addWidget(modeButtonGroup, 0, Qt::AlignRight);

    tw->setLayout(toolsLayout);

    //Spinner
    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(32, 32);
    m_spinner->setVisible(true);
    m_spinnerWidget = new QWidget(this);
    m_loadingLabel = new DLabel(this);
    m_loadingLabel->setText(DApplication::translate("Process.Loading", loadingText));
    m_loadingLabel->setFixedWidth(120);
    QHBoxLayout *loadingLayout = new QHBoxLayout(this);
    loadingLayout->addStretch();
    loadingLayout->addWidget(m_spinner, Qt::AlignCenter);
    loadingLayout->addWidget(m_loadingLabel, Qt::AlignCenter);
    loadingLayout->addStretch();
    m_spinnerWidget->setLayout(loadingLayout);
    //loading 和 进程列表
    m_loadingAndProcessTB = new DStackedWidget(this);
    m_loadingAndProcessTB->setAutoFillBackground(false);
    m_loadingAndProcessTB->setContentsMargins(0, 0, 0, 0);

    // process table view instance
    m_procTable = new ProcessTableView(m_processWidget);

    m_loadingAndProcessTB->addWidget(m_procTable);
    m_loadingAndProcessTB->addWidget(m_spinnerWidget);

    contentlayout->addWidget(tw);
    contentlayout->addWidget(m_loadingAndProcessTB, 1);
    m_processWidget->setLayout(contentlayout);

    // fill left side bar & main content area
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 9, 9);
    layout->setSpacing(16);
    layout->addWidget(m_views);
    layout->addWidget(m_rightStackView);
    setLayout(layout);

    setAutoFillBackground(false);

    // restore previous saved process view mode if any previous settings found
    const QVariant &vindex = m_settings->getOption(kSettingKeyProcessTabIndex, kFilterApps);
    int index = vindex.toInt();
    switch (index) {
    case kFilterCurrentUser: {
        //        //打开时也显示loading
        //        m_loadingAndProcessTB->setCurrentWidget(m_spinnerWidget);
        // show my process view
        m_myProcButton->setChecked(true);
        m_procTable->switchDisplayMode(kFilterCurrentUser);
        m_procViewMode->setText(
                DApplication::translate("Process.Show.Mode", myProcText));   // default text
        //记录当前选中按钮
        m_procBtnCheckedType = USER_PROCESS;
    } break;
    case kNoFilter: {
        //        //打开时也显示loading
        //        m_loadingAndProcessTB->setCurrentWidget(m_spinnerWidget);
        // show all process view
        m_allProcButton->setChecked(true);
        m_procTable->switchDisplayMode(kNoFilter);
        m_procViewMode->setText(
                DApplication::translate("Process.Show.Mode", allProcText));   // default text
        //记录当前选中按钮
        m_procBtnCheckedType = ALL_PROCESSS;
    } break;
    default: {
        //        //打开时也显示loading
        //        m_loadingAndProcessTB->setCurrentWidget(m_spinnerWidget);
        // show my application view by default
        m_appButton->setChecked(true);
        m_procTable->switchDisplayMode(kFilterApps);
        m_procViewMode->setText(
                DApplication::translate("Process.Show.Mode", appText));   // default text
        //记录当前选中按钮
        m_procBtnCheckedType = MY_APPS;
    }
    }   // ::switch(index)

    connect(m_rightStackView, &DetailViewStackedWidget::currentChanged, this, &ProcessPageWidget::onDetailWidgetChanged);
    QTimer::singleShot(5, this, SLOT(onLoadLeftDataWidgetDelay()));
    // this->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
    //   m_loadingAndProcessTB->setCurrentWidget(m_procTable);
}

void ProcessPageWidget::initConnections()
{
    auto *mainWindow = gApp->mainWindow();
    // kill application signal triggered, show application kill preview widget
    // connect(mainWindow, &MainWindow::killProcessPerformed, this,
    //         &ProcessPageWidget::showWindowKiller);
    // switch display mode signal triggered, switch performance display mode
    connect(mainWindow, &MainWindow::displayModeChanged, this,
            &ProcessPageWidget::switchDisplayMode);

    // show my application when my application button toggled
    connect(m_appButton, &DButtonBoxButton::clicked, this, &ProcessPageWidget::onAppButtonClicked);

    // show my process when my process button toggled
    connect(m_myProcButton, &DButtonBoxButton::clicked, this, &ProcessPageWidget::onUserProcButtonClicked);

    // show all application when all application button toggled
    connect(m_allProcButton, &DButtonBoxButton::clicked, this, &ProcessPageWidget::onAllProcButtonClicked);

    // update process summary text when process summary info updated background
    auto *monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
    // Note: do not update on non-GUI thread.
    connect(monitor, &SystemMonitor::appAndProcCountUpdate, this, &ProcessPageWidget::onAppAndProcCountUpdated);

    auto *dAppHelper = DApplicationHelper::instance();
    // change text color dynamically on theme type change, if not do this way, text color wont synchronize with theme type
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, [=]() {
        if (m_procViewMode) {
            auto palette = DApplicationHelper::instance()->applicationPalette();
            palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
            m_procViewMode->setPalette(palette);
        }
        if (m_procViewModeSummary) {
            auto palette = DApplicationHelper::instance()->applicationPalette();
            palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
            m_procViewModeSummary->setPalette(palette);
        }
    });
}

void ProcessPageWidget::onLoadLeftDataWidgetDelay()
{
    // compact view instance
    m_compactView = new MonitorCompactView(m_views);
    connect(m_compactView, &MonitorCompactView::signalDetailInfoClicked, m_rightStackView, &DetailViewStackedWidget::onDetailInfoClicked);
    connect(m_compactView, &MonitorCompactView::signalDetailInfoByDbus, m_rightStackView, &DetailViewStackedWidget::onDbusSendMsgChangeDetailInfoWidget);
    // expand view instance
    m_expandView = new MonitorExpandView(m_views);
    connect(m_expandView, &MonitorExpandView::signalDetailInfoClicked, m_rightStackView, &DetailViewStackedWidget::onDetailInfoClicked);
    connect(m_expandView, &MonitorExpandView::signalDetailInfoByDbus, m_rightStackView, &DetailViewStackedWidget::onDbusSendMsgChangeDetailInfoWidget);
    m_views->insertWidget(kDisplayModeCompact, m_compactView);
    m_views->insertWidget(kDisplayModeExpand, m_expandView);
    // restore previous backupped display mode if any
    const QVariant &mode = Settings::instance()->getOption(kSettingKeyDisplayMode, kDisplayModeCompact);
    m_views->setCurrentIndex(mode.toInt());
}

void ProcessPageWidget::onDetailWidgetChanged(int index)
{
    QWidget *curDetailWidget = m_rightStackView->widget(index);
    if (m_compactView)
        m_compactView->setDetailButtonVisible(m_processWidget == curDetailWidget);
    if (m_expandView)
        m_expandView->setDetailButtonVisible(m_processWidget == curDetailWidget);
}

// event filter
bool ProcessPageWidget::eventFilter(QObject *obj, QEvent *event)
{
    // filter key press events for process show mode buttons, move focus ether way
    if (event->type() == QEvent::KeyPress) {
        if (obj == m_appButton) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if (kev->key() == Qt::Key_Right) {
                m_myProcButton->setFocus();
                return true;
            }
        } else if (obj == m_myProcButton) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if (kev->key() == Qt::Key_Right) {
                m_allProcButton->setFocus();
                return true;
            } else if (kev->key() == Qt::Key_Left) {
                m_appButton->setFocus();
                return true;
            }
        } else if (obj == m_allProcButton) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if (kev->key() == Qt::Key_Left) {
                m_myProcButton->setFocus();
                return true;
            }
        }
    }

    return DFrame::eventFilter(obj, event);
}

void ProcessPageWidget::switchProcessPage()
{
    m_rightStackView->onSwitchProcessPage();
}

void ProcessPageWidget::switchCurrentNoFilterPage()
{
    m_procTable->switchDisplayMode(kNoFilter);
    m_allProcButton->setChecked(true);
    m_procViewMode->setText(DApplication::translate("Process.Show.Mode", allProcText));
    m_procBtnCheckedType = ALL_PROCESSS;
}

// paint event handler
void ProcessPageWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QPainterPath path;
    path.addRect(QRectF(rect()));
    painter.setOpacity(1);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QColor bgColor = palette.color(DPalette::Background);

    //    // 显示0.1s
    //    QTimer::singleShot(NORMAL_PERFORMANCE_CPU_LOADING_TIME, this, [ = ]() {m_loadingAndProcessTB->setCurrentWidget(m_procTable);});
}

// create application kill preview widget
void ProcessPageWidget::createWindowKiller()
{
    // application kill preview instace
    m_xwkillPreview = new XWinKillPreviewWidget();
    // handle left mouse button click event inside the preview widget, popup process kill confirm dialog
    connect(m_xwkillPreview,
            &XWinKillPreviewWidget::windowClicked,
            this,
            &ProcessPageWidget::popupKillConfirmDialog);
}

void ProcessPageWidget::onAppAndProcCountUpdated(int appCount, int procCount)
{
    const QString &buf = DApplication::translate("Process.Summary", kProcSummaryTemplateText);
    m_procViewModeSummary->setText(buf.arg(appCount).arg(procCount));
}

// change icon theme when theme changed
void ProcessPageWidget::changeIconTheme(DGuiApplicationHelper::ColorType themeType)
{
    QIcon appIcon;
    QIcon myProcIcon;
    QIcon allProcIcon;

    if (themeType == DApplicationHelper::LightType) {
        // light theme icon
        appIcon.addFile(iconPathFromQrc("light/app_normal.svg"), {}, QIcon::Normal, QIcon::Off);
        appIcon.addFile(iconPathFromQrc("light/app_highlight.svg"), {}, QIcon::Normal, QIcon::On);

        myProcIcon.addFile(iconPathFromQrc("light/me_normal.svg"), {}, QIcon::Normal, QIcon::Off);
        myProcIcon.addFile(iconPathFromQrc("light/me_highlight.svg"), {}, QIcon::Normal, QIcon::On);

        allProcIcon.addFile(iconPathFromQrc("light/all_normal.svg"), {}, QIcon::Normal, QIcon::Off);
        allProcIcon.addFile(iconPathFromQrc("light/all_highlight.svg"), {}, QIcon::Normal, QIcon::On);
    } else if (themeType == DApplicationHelper::DarkType) {
        // dark theme icon
        appIcon.addFile(iconPathFromQrc("dark/app_normal_dark.svg"), {}, QIcon::Normal, QIcon::Off);
        appIcon.addFile(iconPathFromQrc("dark/app_highlight.svg"), {}, QIcon::Normal, QIcon::On);

        myProcIcon.addFile(iconPathFromQrc("dark/me_normal_dark.svg"), {}, QIcon::Normal, QIcon::Off);
        myProcIcon.addFile(iconPathFromQrc("dark/me_highlight.svg"), {}, QIcon::Normal, QIcon::On);

        allProcIcon.addFile(iconPathFromQrc("dark/all_normal_dark.svg"), {}, QIcon::Normal, QIcon::Off);
        allProcIcon.addFile(iconPathFromQrc("dark/all_highlight.svg"), {}, QIcon::Normal, QIcon::On);
    }

    m_appButton->setIcon(appIcon);
    m_appButton->setIconSize(QSize(26, 24));

    m_myProcButton->setIcon(myProcIcon);
    m_myProcButton->setIconSize(QSize(26, 24));

    m_allProcButton->setIcon(allProcIcon);
    m_allProcButton->setIconSize(QSize(26, 24));
}

// popup application kill confirm dialog
void ProcessPageWidget::popupKillConfirmDialog(pid_t pid)
{
    QString title = DApplication::translate("Kill.Process.Dialog", "End process");
    QString description = DApplication::translate("Kill.Process.Dialog",
                                                  "Force ending this application may cause data "
                                                  "loss.\nAre you sure you want to continue?");

    KillProcessConfirmDialog dialog;
    // always stay on top
    dialog.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    dialog.setMessage(description);
    dialog.addButton(DApplication::translate("Kill.Process.Dialog", "Cancel", "button"), false);
    dialog.addButton(DApplication::translate("Kill.Process.Dialog", "Force End", "button"), true,
                     DDialog::ButtonWarning);
    dialog.exec();
    if (dialog.result() == QMessageBox::Ok) {
        QString name = m_procTable->getProcessName(pid);
        QJsonObject obj {
            { "tid", EventLogUtils::ProcessKilled },
            { "version", QCoreApplication::applicationVersion() },
            { "process_name", name }
        };
        EventLogUtils::get().writeLogs(obj);

        ProcessDB::instance()->killProcess(pid);
    }
}

// show application kill preview widget
void ProcessPageWidget::showWindowKiller()
{
    gApp->mainWindow()->showMinimized();
    createWindowKiller();
}

void ProcessPageWidget::switchDisplayMode(int mode)
{
    m_views->setCurrentIndex(mode);
}

void ProcessPageWidget::onAllProcButtonClicked()
{
    //若已选中，再次点击不会加载数据
    if (m_procBtnCheckedType != ALL_PROCESSS) {
        PERF_PRINT_BEGIN("POINT-04", QString("switch(%1->%2)").arg(m_procViewMode->text()).arg(DApplication::translate("Process.Show.Mode", allProcText)));
        //        m_loadingAndProcessTB->setCurrentWidget(m_spinnerWidget);
        m_procViewMode->setText(DApplication::translate("Process.Show.Mode", allProcText));
        m_procViewMode->adjustSize();
        m_procTable->switchDisplayMode(kNoFilter);
        if (CPUPerformance == CPUMaxFreq::High) {
            m_settings->setOption(kSettingKeyProcessTabIndex, kNoFilter);
        } else {
            m_settings->setOption(kSettingKeyProcessTabIndex, kFilterApps);
        }
        PERF_PRINT_END("POINT-04");
    }
    //记录当前按钮为已选中
    m_procBtnCheckedType = ALL_PROCESSS;
}

void ProcessPageWidget::onUserProcButtonClicked()
{
    //   qCInfo(app) << CPUPerformance << "CPUPerformance";
    //若已选中，再次点击不会加载数据
    if (m_procBtnCheckedType != USER_PROCESS) {
        PERF_PRINT_BEGIN("POINT-04", QString("switch(%1->%2)").arg(m_procViewMode->text()).arg(DApplication::translate("Process.Show.Mode", myProcText)));
        //        m_loadingAndProcessTB->setCurrentWidget(m_spinnerWidget);
        m_procViewMode->setText(DApplication::translate("Process.Show.Mode", myProcText));
        m_procViewMode->adjustSize();
        m_procTable->switchDisplayMode(kFilterCurrentUser);
        if (CPUPerformance == CPUMaxFreq::High) {
            m_settings->setOption(kSettingKeyProcessTabIndex, kFilterCurrentUser);
        } else {
            m_settings->setOption(kSettingKeyProcessTabIndex, kFilterApps);
        }
        PERF_PRINT_END("POINT-04");
    };
    //记录当前按钮为已选中
    m_procBtnCheckedType = USER_PROCESS;
}

void ProcessPageWidget::onAppButtonClicked()
{
    //若已选中，再次点击不会加载数据
    if (m_procBtnCheckedType != MY_APPS) {
        PERF_PRINT_BEGIN("POINT-04", QString("switch(%1->%2)").arg(m_procViewMode->text()).arg(DApplication::translate("Process.Show.Mode", appText)));
        //        m_loadingAndProcessTB->setCurrentWidget(m_spinnerWidget);
        m_procViewMode->setText(DApplication::translate("Process.Show.Mode", appText));
        m_procViewMode->adjustSize();
        m_procTable->switchDisplayMode(kFilterApps);
        m_settings->setOption(kSettingKeyProcessTabIndex, kFilterApps);
        PERF_PRINT_END("POINT-04");
    }
    //记录当前按钮为已选中
    m_procBtnCheckedType = MY_APPS;
}
