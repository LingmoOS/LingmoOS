// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "src/window/modules/common/gsettingkey.h"
#include "src/window/modules/common/compixmap.h"
#include "src/window/modules/common/common.h"

// 首页模块
#include "window/modules/homepagecheck/homepagemodule.h"
#include "window/modules/homepagecheck/homepagemodel.h"
// 安全防护模块
#include "window/modules/safetyprotection/safetyprotectionmodule.h"
// 垃圾清理模块
#include "window/modules/cleaner/cleanermodule.h"
// 安全工具模块
#include "window/modules/securitytools/securitytoolsmodule.h"
// 安全日志弹框
#include "window/modules/securitylog/securitylogdialog.h"
#include "window/modules/securitylog/securitylogadaptorimpl.h"

#include "../window/dialogmanage.h"
#include "../widgets/updateaddressitem.h"
#include "../widgets/multiselectlistview.h"

#include <DSettingsDialog>
#include <DBackgroundGroup>
#include <DGuiApplicationHelper>
#include <DListView>
#include <DStandardItem>
#include <DApplicationHelper>
#include <DTitlebar>
#include <DDialog>
#include <DRadioButton>
#include <DCheckBox>
#include <DSysInfo>
#include <DWarningButton>

#include <QDateTime>
#include <QHBoxLayout>
#include <QMetaEnum>
#include <QDebug>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLocale>
#include <QLinearGradient>
#include <QWidget>
#include <QResizeEvent>
#include <qsettingbackend.h>
#include <QGSettings>
#include <QDBusInterface>
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <QObject>

DEF_USING_NAMESPACE
DTK_USE_NAMESPACE
DCORE_USE_NAMESPACE

const int WidgetMinimumWidget = 950;
const int WidgetMinimumHeight = 640;

//此处为带边距的宽度
const int first_widget_min_width = 200;

const QMargins navItemMargin(5, 3, 5, 3);
const QVariant NavItemMargin = QVariant::fromValue(navItemMargin);
const QString dmanInterface = "com.deepin.Manual.Open";

#define DIALOG_WIDTH 380 //dialog  宽度
#define DIALOG_HEIGHT 145 //dialog  高度
#define DIALOG_PB_WIDTH 140 //dialog  按钮宽度
#define DIALOG_PB_HEIGHT 36 //dialog  按钮高度

#define INSTALL_TIME "defender-install-time" // 安装时间

/******************安全中心主界面初始化******************/
MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_contentLayout(nullptr)
    , m_rightContentLayout(nullptr)
    , m_navView(nullptr)
    , m_rightView(nullptr)
    , m_navModel(nullptr)
    , m_homePageModule(nullptr)
    , m_homePageModel(nullptr)
    , m_backwardBtn(nullptr)
    , m_pSystemTray(nullptr)
    , m_trayMenu(nullptr)
    , m_trayDialag(nullptr)
    , m_gsetting(nullptr)
    , m_dsd(nullptr)
    , m_netControlWidget(nullptr)
    , m_securityLogDialog(nullptr)
    , m_dataInterFaceServer(new DataInterFaceServer("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", QDBusConnection::sessionBus(), this))
    , m_daemonservice(new DaemonService("com.deepin.defender.daemonservice", "/com/deepin/defender/daemonservice", QDBusConnection::sessionBus(), this))
    , m_monitorInterFaceServer(new MonitorInterFaceServer("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_guiHelper(nullptr)
{
    this->setAccessibleName("mainWindow");
    // 设置主窗口属性
    setFixedSize(QSize(WidgetMinimumWidget, WidgetMinimumHeight));
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    // 初始化安全中心gsetting配置
    m_gsetting = new QGSettings("com.deepin.dde.deepin-defender", QByteArray(), this);
    // 设置安装时间
    setInstallTime();
    //启动守护进程，确保守护进程每次都能正常工作
    initDaemonService();
    // 主题变换
    m_guiHelper = DGuiApplicationHelper::instance();
    connect(m_guiHelper, SIGNAL(themeTypeChanged(ColorType)), this, SLOT(themeTypeChange(ColorType)));

    //Initialize view and layout structure
    QWidget *content = new QWidget(this);
    content->setAccessibleName("mainWindow_contentWidget");
    setCentralWidget(content);

    m_contentLayout = new QHBoxLayout(content);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setMargin(0);
    m_contentLayout->setSpacing(0);
    //左边list
    m_navView = new MultiSelectListView(this);
    m_navView->setAccessibleName("contentWidget_selectListView");
    m_navView->setItemSize(QSize(180, 48));
    m_navView->setItemSpacing(0);
    m_navView->setSpacing(20);
    m_navView->setIconSize(QSize(32, 32));
    m_navView->setFrameShape(QFrame::Shape::NoFrame);
    m_navView->setEditTriggers(QListView::NoEditTriggers);
    m_navView->setAutoScroll(true); //滚动条
    m_navView->setTabKeyNavigation(false); //tab按键是否有效
    m_navView->setViewportMargins(QMargins(10, 10, 10, 0)); //滚动区域边框大小
    m_navView->setFixedWidth(first_widget_min_width); //固定窗口值
    m_navView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_navView->setSpacing(0);
    m_navView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_navView->setAutoFillBackground(true);
    m_navView->setBackgroundType(DStyledItemDelegate::BackgroundType(DStyledItemDelegate::RoundedBackground | DStyledItemDelegate::NoNormalState));
    m_contentLayout->addWidget(m_navView);

    // 右侧布局
    m_rightContentLayout = new QHBoxLayout;

    m_rightView = new DBackgroundGroup(m_rightContentLayout);
    m_rightView->setAccessibleName("contentWidget_rightWidget");
    m_rightView->setItemSpacing(2);
    m_rightView->setItemMargins(QMargins(0, 0, 0, 0));
    m_rightView->setContentsMargins(0, 0, 0, 0);
    m_rightView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentLayout->addWidget(m_rightView);

    //标题栏
    DTitlebar *titlebar = this->titlebar();
    titlebar->setIcon(QIcon::fromTheme(DIALOG_DEFENDER));

    auto menu = titlebar->menu();
    if (!menu) {
        menu = new QMenu(this);
    }
    titlebar->setMenu(menu);

    // 设置
    auto action = new QAction(tr("Settings"), this);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::showDefaultSettingDialog);

    // 只有桌面专业版才需要用户反馈
    if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosProfessional
        && DSysInfo::UosType::UosDesktop == SystemType) {
        // 用户反馈
        auto userReplyAction = new QAction(tr("Report issues"), this);
        menu->addAction(userReplyAction);
        connect(userReplyAction, &QAction::triggered, this, &MainWindow::showUserReplyDialog);
    }

    // 安全日志
    auto securityLogAction = new QAction(tr("Logs"), this);
    menu->addAction(securityLogAction);
    connect(securityLogAction, &QAction::triggered, this, &MainWindow::showSecurityLogDialog);

    // 服务器版本上不需要安全日志
    if (DSysInfo::UosType::UosServer == SystemType) {
        securityLogAction->setVisible(false);
    }

    m_backwardBtn = new DIconButton(this); //上一步按钮
    m_backwardBtn->setAccessibleName("backwardButton");
    m_backwardBtn->setIcon(QStyle::SP_ArrowBack);
    titlebar->addWidget(m_backwardBtn, Qt::AlignLeft | Qt::AlignVCenter);
    connect(m_backwardBtn, &DIconButton::clicked, this, &MainWindow::onBackWardClick);

    //Initialize top page view and model
    m_navModel = new QStandardItemModel(m_navView);
    m_navView->setModel(m_navModel);
    // 连接按下信号与窗口切换槽函数
    connect(m_navView, &DListView::pressed, this, &MainWindow::onFirstItemClick);

    QTimer::singleShot(0, this, &MainWindow::initAllModule); //初始化所有模块
    QTimer::singleShot(0, this, &MainWindow::modulePreInitialize); //模块初始化前准备

    // 初始化托盘
    initTray();

    // 初始化数据服务接口 信号槽
    connect(m_dataInterFaceServer, &DataInterFaceServer::requestRemDefaultAllowStatus, this, &MainWindow::setRemDefaultAllowStatus);
}

MainWindow::~MainWindow()
{
    if (m_navModel) {
        m_navModel->clear();
        m_navModel->deleteLater();
        m_navModel = nullptr;
    }
}

void MainWindow::popWidget(ModuleInterface *const inter)
{
    Q_UNUSED(inter)
    // 将最后一层窗口出栈
    popWidget();
}

void MainWindow::popAndDelWidget(ModuleInterface *const inter)
{
    Q_UNUSED(inter)
    // 将最后一层窗口出栈并删除
    popAndDelWidget();
}

void MainWindow::pushWidget(ModuleInterface *const inter, QWidget *const w)
{
    if (!inter) {
        qWarning() << Q_FUNC_INFO << " inter is nullptr";
        return;
    }

    if (!w) {
        qWarning() << Q_FUNC_INFO << " widget is nullptr";
        return;
    }

    pushNormalWidget(inter, w);

    // 退回按钮状态
    updateBackwardBtn();
}

void MainWindow::setModuleVisible(ModuleInterface *const inter, const bool visible)
{
    auto find_it = std::find_if(m_modules.cbegin(), m_modules.cend(), [=](const QPair<ModuleInterface *, QString> &pair) {
        return pair.first == inter;
    });

    if (find_it != m_modules.cend()) {
        m_navView->setRowHidden(find_it - m_modules.cbegin(), !visible);
        Q_EMIT moduleVisibleChanged(find_it->first->name(), visible);
    }
}

void MainWindow::setCurrentModule(int iModuleIdx, int iPageIdx)
{
    // 获取QModelIndex
    QModelIndex modelIndex = m_navModel->index(iModuleIdx, 0);
    m_navView->setCurrentIndex(m_navModel->index(iModuleIdx, 0));
    // 将显示栈清空
    popAllWidgets(0);
    // 激活模块
    ModuleInterface *inter = m_modules[modelIndex.row()].first;
    inter->active(iPageIdx);
}

ModuleInterface *MainWindow::getCurrentModule()
{
    return m_contentStack.top().first;
}

int MainWindow::getModuleIndex(const QString &name)
{
    int index = 0;
    for (QPair<ModuleInterface *, QString> p : m_modules) {
        if (p.first->name() == name)
            return index;
        index++;
    }
    return -1;
}

// 显示默认设置弹框
void MainWindow::showDefaultSettingDialog()
{
    showSettingDialog("");
}

// 显示用户反馈弹框
void MainWindow::showUserReplyDialog()
{
    //创建（或new）对象连接服务
    QDBusInterface interFace("com.deepin.dde.ServiceAndSupport",
                             "/com/deepin/dde/ServiceAndSupport",
                             "com.deepin.dde.ServiceAndSupport",
                             QDBusConnection::sessionBus());
    interFace.call("ServiceSession", 1);
}

// 显示安全日志弹框
void MainWindow::showSecurityLogDialog()
{
    if (m_securityLogDialog == nullptr) {
        SecurityLogAdaptorInterface *interface = new SecurityLogAdaptorImpl;
        m_securityLogDialog = new SecurityLogDialog(nullptr, interface);
        connect(m_securityLogDialog, &SecurityLogDialog::onDialogClose, this, [=]() {
            m_securityLogDialog->deleteLater();
            m_securityLogDialog = nullptr;
        });
        m_securityLogDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_securityLogDialog->setOnButtonClickedClose(true);
        m_securityLogDialog->show();
    }
}

// 显示设置弹窗，并跳转到相应组位置
void MainWindow::showSettingDialog(const QString &groupKey)
{
    QTemporaryFile *tmpFile = new QTemporaryFile;
    tmpFile->open();
    auto backend = new Dtk::Core::QSettingBackend(tmpFile->fileName());

    // 创建设置数据
    Dtk::Core::DSettings *settings;
    bool bStatus = m_dataInterFaceServer->GetUosResourceStatus();
    // 区分服务器版
    if (DSysInfo::UosType::UosServer == SystemType) {
        settings = Dtk::Core::DSettings::fromJsonFile(SETTING_JSON_FOR_SRV_EDITION);
    } else {
        if (bStatus) {
            settings = Dtk::Core::DSettings::fromJsonFile(SETTING_JSON);
        } else {
            settings = Dtk::Core::DSettings::fromJsonFile(SETTING_JSON_FOR_RESOURCE);
        }
    }
    settings->setBackend(backend);

    /* 基础设置 */
    // 基础设置-根据关闭窗口属性gsetting配置，更新设置界面选项
    if (0 == this->getAskType()) //优先显示每次询问按钮是否选中
        settings->setOption(SETTING_BASE_CLOSE_WINDOW, getCloseType());
    else if (1 == this->getAskType())
        settings->setOption(SETTING_BASE_CLOSE_WINDOW, 2);

    /* 安全防护设置 */
    // 区分服务器版
    if (DSysInfo::UosType::UosServer != SystemType) {
        settings->setOption(SETTING_SAFETY_USB_STORAGE_DEVICES, getUsbStorageType());
        if (bStatus) {
            settings->setOption(SETTING_SAFETY_REM_CONTROL, getRemType());
        }
    }

    // 连接设置信号
    connect(settings, &Dtk::Core::DSettings::valueChanged, this, [=](const QString &key, const QVariant &value) {
        if (key.contains(SETTING_CLOSE_WINDOW_TYPE)) {
            int type = value.toInt();
            if (type >= Tray && type <= Exit) {
                this->setCloseType(type); // 设置关闭类型 托盘还是推出
                this->setAskType(0); // 设置访问类型
            } else if (type == Ask) {
                this->setAskType(1); // 设置访问类型是每次访问
            }
        }

        // usb设备保护
        if (key.contains(SETTING_USB_STORAGE_DEVICES_TYPE)) {
            this->setUsbStorageType(value.toInt());
            // 添加安全日志
            if (value.toBool()) {
                m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_SAFY, tr("Changed security settings of USB storage devices to \"Auto remove threats\""));
            } else {
                m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_SAFY, tr("Changed security settings of USB storage devices to \"Ask me\""));
            }
        }

        // 远程访问
        if (key.contains(SETTING_REM_CONTROL_TYPE)) {
            this->setRemType(value.toInt());
            // 添加安全日志
            if (NET_STATUS_ITEMASK == value.toInt()) {
                m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_SAFY, tr("Changed default settings of remote connection to \"Ask me always\""));
            } else if (NET_STATUS_ITEMALLOW == value.toInt()) {
                m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_SAFY, tr("Changed default settings of remote connection to \"Allow always\""));
            } else {
                m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_SAFY, tr("Changed default settings of remote connection to \"Disable always\""));
            }
        }
    });

    // 设置弹窗
    m_dsd = new DSettingsDialog(this);
    m_dsd->updateSettings(settings);
    m_dsd->setModal(true);
    this->setWindowState(Qt::WindowState::WindowActive);
    this->show();
    m_dsd->show();
    m_dsd->scrollToGroup(groupKey);
}

void MainWindow::toggle()
{
    raise();
    if (isMinimized() || !isVisible()) {
        // 显示并恢复窗口为正常状态
        showNormal();
    }

    activateWindow();
}

void MainWindow::popAndDelWidget()
{
    if (!m_contentStack.size())
        return;

    QWidget *w = m_contentStack.pop().second; //栈的顶栈的widget
    m_rightContentLayout->removeWidget(w);
    w->hide();
    w->setParent(nullptr);
    w->deleteLater();
}

void MainWindow::popWidget()
{
    if (!m_contentStack.size())
        return;

    QWidget *w = m_contentStack.pop().second; //栈的顶栈的widget
    w->hide();
    m_rightContentLayout->removeWidget(w);
}

//确保“安全中心”启动后，所有的服务都能正常工作
void MainWindow::initDaemonService()
{
    m_daemonservice->StartApp();
}

/******************各功能模块加载******************/
//初始化模块
void MainWindow::initAllModule()
{
    // 首页体检模块
    // 区分服务器版
    if (DSysInfo::UosType::UosServer == SystemType) {
        m_modules = {
            {new CleanerModule(this), tr("Cleanup")},
        };

        // 浅色主题图标
        m_modulesLightIconList.clear();
        m_modulesLightIconList << HOMEPAGE_CLEAR_LIGHT;

        // 深色主题图标
        m_modulesDarakIconList.clear();
        m_modulesDarakIconList << HOMEPAGE_CLEAR_DARK;
    } else {
        m_homePageModule = new HomePageModule(this);
        m_securityToolsModule = new SecurityToolsModule(this);
        m_modules = {
            {m_homePageModule, tr("Home")},
            {new SafetyProtectionModule(this, this), tr("Protection")},
            {new CleanerModule(this, this), tr("Cleanup")},
            {m_securityToolsModule, tr("Tools")},
        };

        // 浅色主题图标
        m_modulesLightIconList.clear();
        m_modulesLightIconList << HOMEPAGE_MODEL_LIGHT << HOMEPAGE_SECURITY_LIGHT << HOMEPAGE_CLEAR_LIGHT << HOMEPAGE_TOOL_LIGHT;

        // 深色主题图标
        m_modulesDarakIconList.clear();
        m_modulesDarakIconList << HOMEPAGE_MODEL_DARK << HOMEPAGE_SECURITY_DARK << HOMEPAGE_CLEAR_DARK << HOMEPAGE_TOOL_DARK;
    }
    // 获得主题类型
    int type = m_guiHelper->themeType();
    for (auto it = m_modules.cbegin(); it != m_modules.cend(); ++it) {
        DStandardItem *item = new DStandardItem;
        item->setAccessibleText(it->first->name());
        item->setText(it->second);
        QString iconPath;
        if (type == 1) {
            iconPath = m_modulesLightIconList.at(it - m_modules.cbegin());
        } else {
            iconPath = m_modulesDarakIconList.at(it - m_modules.cbegin());
        }
        QIcon icon = QIcon::fromTheme(iconPath);
        item->setIcon(icon);

        item->setData(NavItemMargin, Dtk::MarginsRole);
        item->setData(QVariant(it->first->name()), Qt::AccessibleTextRole);

        m_navModel->appendRow(item);
    }
}

void MainWindow::showModulePage(const QString &module, const QString &page)
{
    int moduleIndex = 0;
    int pageIndex = 0;
    ModuleInterface *inter = nullptr;
    for (int i = 0; i < m_modules.count(); i++) {
        inter = m_modules[i].first;
        if (module == inter->name()) {
            moduleIndex = i;
            break;
        }
    }

    // 判断子页面索引
    pageIndex = getPageIndex(page);

    setCurrentModule(moduleIndex, pageIndex);
    // 显示并恢复窗口为正常状态
    showNormal();
    activateWindow();
}

int MainWindow::getModuleCount()
{
    return m_modules.count();
}

/******************安全中心窗口设置操作******************/
// 关闭窗口事件
void MainWindow::closeEvent(QCloseEvent *event)
{
    // 获得访问类型  0 不访问  1是展现访问关闭窗口
    if (!getAskType()) {
        // 不访问 获得关闭类型 0最小化 1退出
        if (getCloseType()) {
            closeWindow();
            event->ignore();
        } else {
            hide();
            event->ignore();
        }
    } else {
        initCloseDialog(event);
    }
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
}

//左侧列表的点击
void MainWindow::onFirstItemClick(const QModelIndex &index)
{
    m_navView->setCurrentIndex(index);
    ModuleInterface *inter = m_modules[index.row()].first;

    //有数据  并且最后一个和现在点击的一致
    if (!m_contentStack.isEmpty() && m_contentStack.last().first == inter) {
        return;
    }
    popAllWidgets(0);

    inter->active(0);
    m_navView->resetStatus(index);
    m_navView->setFocus();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason state)
{
    if (state == QSystemTrayIcon::ActivationReason::Trigger) {
        if (!this->isVisible()) {
            //页面隐藏，点击托盘重新show
            this->show();
        } else {
            //页面能显示的时候，点击托盘会＂正常/最小化＂切换窗口
            if (this->isMinimized()) {
                this->setWindowState(Qt::WindowState::WindowActive);
            } else {
                this->setWindowState(Qt::WindowState::WindowMinimized);
            }
        }
    }
}

void MainWindow::setSystemsafety(const QModelIndex &index, int sonindex)
{
    ModuleInterface *inter = m_modules[index.row()].first;

    popAllWidgets();
    inter->active(sonindex);
}

// 背景主题变化
void MainWindow::themeTypeChange(ColorType themeType)
{
    Q_UNUSED(themeType);
    // 获得所有行
    int count = m_navModel->rowCount();
    for (int i = 0; i < count; i++) {
        // 每行的item
        QStandardItem *item = m_navModel->item(i, 0);
        QString iconPath;
        if (m_guiHelper->themeType() == DGuiApplicationHelper::ColorType::LightType) {
            iconPath = m_modulesLightIconList.at(i);
        } else {
            iconPath = m_modulesDarakIconList.at(i);
        }
        // 每行item的图标设置
        QIcon icon = QIcon::fromTheme(iconPath);
        item->setIcon(icon);
        item->setData(NavItemMargin, Dtk::MarginsRole);
        m_navModel->setItem(i, 0, item);
    }
}

// 初始化托盘
void MainWindow::initTray()
{
    m_pSystemTray = new QSystemTrayIcon(this);
    m_trayMenu = new QMenu(this);
    m_trayMenu->setAccessibleName("systemTrayMenu");

    // 安全中心
    QAction *show = new QAction(this);
    show->setText(tr("Security Center"));
    show->setToolTip("SecurityCenter");
    m_trayMenu->addAction(show);
    // 点击托盘中的安全中心  激活该窗口并且展现出来
    connect(show, &QAction::triggered, this, [this] {
        this->show();
        this->setWindowState(Qt::WindowState::WindowActive);
    });

    // 托盘退出
    QAction *exit = new QAction(this);
    exit->setText(tr("Exit"));
    m_trayMenu->addAction(exit);
    connect(exit, &QAction::triggered, this, [=] {
        closeWindow();
    });

    // 设置系统托盘的上下文菜单
    m_pSystemTray->setContextMenu(m_trayMenu);
    // 设置系统托盘提示信息、托盘图标
    QString name = tr("Security Center");
    m_pSystemTray->setToolTip(name);
    m_pSystemTray->setIcon(QIcon::fromTheme(DIALOG_DEFENDER));
    connect(m_pSystemTray, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);

    m_pSystemTray->show();
}

// 关闭弹窗  只有在设置是访问才有这个
void MainWindow::initCloseDialog(QCloseEvent *event)
{
    m_trayDialag = new DDialog(this);
    m_trayDialag->setAccessibleName("trayDialog");
    m_trayDialag->setModal(true);
    m_trayDialag->setIcon(QIcon::fromTheme(DIALOG_DEFENDER));
    DRadioButton *radioExit = new DRadioButton(tr("Exit"), this);
    radioExit->setAccessibleName("trayDialog_exitButton");
    DRadioButton *radioMin = new DRadioButton(tr("Minimize to system tray"), this);
    radioMin->setAccessibleName("trayDialog_minimizeButton");
    DCheckBox *radioAsk = new DCheckBox(tr("Do not ask again"), this);
    radioAsk->setAccessibleName("trayDialog_askButton");
    closeType nChoose = static_cast<closeType>(getCloseType());

    //根据GSettings设置settings页面的默认值
    switch (nChoose) {
    case Exit:
        radioExit->setChecked(true);
        break;
    case Tray:
    default:
        radioMin->setChecked(true);
        break;
    }
    radioAsk->setCheckState(Qt::CheckState::Unchecked);

    // 选择你的操作
    m_trayDialag->setMessage(tr("Please choose your action"));
    m_trayDialag->addContent(radioMin);
    m_trayDialag->addContent(radioExit);
    m_trayDialag->addContent(radioAsk);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    QPushButton *cancelBtn = new QPushButton(m_trayDialag);
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setAccessibleName("trayDialog_cancelButton");
    cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
    m_trayDialag->insertButton(0, cancelBtn, false);

    QPushButton *confirmBtn = new QPushButton(m_trayDialag);
    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setObjectName("ConfirmBtn");
    confirmBtn->setAccessibleName("trayDialog_confirmButton");
    confirmBtn->setAttribute(Qt::WA_NoMousePropagation);
    m_trayDialag->insertButton(1, confirmBtn, false);

    int a = m_trayDialag->exec();
    if (1 != a) {
        event->ignore();
        return;
    } else {
        // 确认按钮点击
        if (radioMin->isChecked()) { //最小花点击
            this->hide();
            event->ignore();
        } else if (radioExit->isChecked()) { //退出点击
            closeWindow();
            event->ignore();
        }

        // 根据选择的方式  设置关闭方式
        if (radioMin->isChecked()) {
            setCloseType(Tray);
        } else if (radioExit->isChecked()) {
            setCloseType(Exit);
        }

        //只有当选择radioAsk后才会设置不再询问
        if (radioAsk->isChecked()) {
            setAskType(0);
        }

        m_trayDialag->deleteLater();
        m_trayDialag = nullptr;
    }
}

//模块初始化前准备
void MainWindow::modulePreInitialize()
{
    for (auto it = m_modules.cbegin(); it != m_modules.cend(); ++it) {
        QElapsedTimer et;
        et.start();
        it->first->preInitialize();
    }

    QModelIndex index0 = m_navModel->index(0, 0);
    if (index0.isValid()) {
        onFirstItemClick(index0); //默认点击首页
    }
}

//Only used to from third page to top page can use it
void MainWindow::popAllWidgets(int place)
{
    if (!m_contentStack.count())
        return;

    for (int pageCount = m_contentStack.count(); pageCount > place; pageCount--) {
        QString sModuleName = m_contentStack.top().first->name();
        if (MODULE_HOMEPAGE_NAME == sModuleName) {
            popWidget();
        } else if (MODULE_VIRUS_SCAN_NAME == sModuleName) {
            popWidget();
        } else if (MODULE_DISK_CLEANER_NAME == sModuleName) {
            popWidget();
        } else {
            popAndDelWidget();
        }
    }
}

void MainWindow::pushNormalWidget(ModuleInterface *const inter, QWidget *const w)
{
    //Set the newly added page to fill the blank area
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    w->show();

    m_contentStack.push({inter, w});
    m_rightContentLayout->addWidget(w, m_contentStack.size() == 1 ? 3 : 7);
}

// 获得关闭类型 0托盘最小化 1推出
int MainWindow::getCloseType()
{
    return m_gsetting ? m_gsetting->get(CLOSE_TYPE).toInt() : 0;
}

//0 : Not Ask  1 : Ask      0 托盘最小化  1 退出
void MainWindow::setCloseType(int value)
{
    if (m_gsetting) {
        m_gsetting->set(CLOSE_TYPE, value);
    }
}

// 获得访问类型 0不访问 1访问
int MainWindow::getAskType()
{
    return m_gsetting ? m_gsetting->get(ASK_TYPE).toInt() : 0;
}

// 设置访问  0 不访问 1访问
void MainWindow::setAskType(int value)
{
    if (m_gsetting) {
        m_gsetting->set(ASK_TYPE, value);
    }
}

// 联网管控默认状态配置项 获取/设置
int MainWindow::getUsbStorageType()
{
    return m_gsetting ? m_gsetting->get(USB_STORAGE_DEVICES_TYPE).toInt() : 0;
}

void MainWindow::setUsbStorageType(int value)
{
    if (m_gsetting) {
        m_gsetting->set(USB_STORAGE_DEVICES_TYPE, value);
    }
}

// 远程访问默认状态配置项 获取/设置
int MainWindow::getRemType()
{
    return m_gsetting ? m_gsetting->get(REM_CONTROL_DEFAULT_STATUS).toInt() : 0;
}

void MainWindow::setRemType(int value)
{
    if (m_gsetting) {
        m_gsetting->set(REM_CONTROL_DEFAULT_STATUS, value);
    }
}

// 寻找子页面索引
int MainWindow::getPageIndex(QString pageName)
{
    int retIndex = 0;
    // 安全工具
    // 如果是流量详情
    if (DATA_USAGE_NAME == pageName) {
        retIndex = DATA_USAGE_INDEX;
    } else if (STARTUP_CONTROL_NAME == pageName) {
        // 如果是自启动管理
        retIndex = STARTUP_CONTROL_INDEX;
    } else if (NET_CONTROL_NAME == pageName) {
        // 如果是网络权限管控
        retIndex = NET_CONTROL_INDEX;
    } else if (USB_CONN_NAME == pageName) {
        // 如果是usb管控
        retIndex = USB_CONN_INDEX;
    } else if (LOGIN_SAFETY_NAME == pageName) {
        // 如果是登录安全
        retIndex = LOGIN_SAFETY_INDEX;
    }
    return retIndex;
}

// 点击退回按钮槽
void MainWindow::onBackWardClick(bool checked)
{
    Q_UNUSED(checked);

    // 只针对 首页  进行后退判断和处理
    QString sModuleName = m_contentStack.top().first->name();
    if (MODULE_SECURITY_TOOLS_NAME == sModuleName) {
        popWidget();
        m_securityToolsModule->deleteSunWidget();
    }

    QWidget *topWidget = this->m_contentStack.top().second;
    if (topWidget) {
        topWidget->show();
    }

    // 退回按钮状态
    updateBackwardBtn();
}

// 更新后退按钮
void MainWindow::updateBackwardBtn()
{
    m_backwardBtn->setVisible(false);

    // 只对首页 病毒查杀页 和 防火墙页 作后退按钮判断
    QString sModuleName = m_contentStack.top().first->name();
    if (MODULE_SECURITY_TOOLS_NAME == sModuleName) {
        if (1 < m_contentStack.count()) {
            m_backwardBtn->setVisible(true);
            m_backwardBtn->setDisabled(false);
        } else {
            m_backwardBtn->setVisible(false);
        }
    }
}

/******************安全中心联网远程访问接口******************/
// 联网管控弹框
void MainWindow::showFuncConnectNetControl(const QString &sPkgName)
{
    Q_UNUSED(sPkgName);

    // 远程访问设置跳转
    showNormal();
    this->activateWindow();
    // 显示防火墙页
    showModulePage(MODULE_SECURITY_TOOLS_NAME, "");

    // 联网管控界面显示
    m_securityToolsModule->showNetControlWidget();
    m_securityToolsModule->setNetControlIndex(sPkgName);
}

// 远程访问弹框
void MainWindow::showFuncConnectRemControl(const QString &sPkgName)
{
    Q_UNUSED(sPkgName);

    // 远程访问设置跳转
    showNormal();
    this->activateWindow();

    // 前往到设置弹框设置远程访问项
    if (m_dsd != nullptr) {
        m_dsd->scrollToGroup("safety");
    } else {
        showSettingDialog("safety");
    }
}

// 远程访问设置为始终允许
void MainWindow::setRemDefaultAllowStatus()
{
    // 前往到设置弹框设置远程访问项
    if (m_dsd != nullptr) {
    } else {
    }
}

// 流量详情弹框
void MainWindow::showFuncConnectDataUsage()
{
    // 远程访问设置跳转
    showNormal();
    this->activateWindow();
    // 显示防火墙页
    showModulePage(MODULE_SECURITY_TOOLS_NAME, "");

    // 联网管控界面显示
    m_securityToolsModule->showDataUsageWidget();
}

void MainWindow::closeWindow()
{
    qApp->quit();
}

// 设置安装时间
void MainWindow::setInstallTime()
{
    // 获得安装时间  如果是默认值-1 设置当前时间为安装时间
    QString lasttime = m_gsetting->get(INSTALL_TIME).toString();
    QString defaultTime = "-1";
    if (lasttime == defaultTime) {
        //当前时间
        QDateTime date = QDateTime::currentDateTime();
        QString installTime = date.toString("yyyy-MM-dd hh:mm:ss");
        m_gsetting->set(INSTALL_TIME, installTime);
    }
}
