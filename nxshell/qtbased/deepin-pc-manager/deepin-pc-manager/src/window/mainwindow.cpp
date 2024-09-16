// Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "modules/common/invokers/invokerfactory.h"
#include "src/window/modules/common/common.h"
#include "src/window/modules/common/compixmap.h"
#include "src/window/modules/common/gsettingkey.h"

// 首页模块
#include "modules/homepagecheck/homepagemodel.h"
#include "modules/homepagecheck/homepagemodule.h"
// 垃圾清理模块
#include "modules/cleaner/cleanermodule.h"
// 安全工具模块
#include "modules/securitytools/securitytoolsmodule.h"

// 重启安全中心弹框
#include "../widgets/multiselectlistview.h"

#include <qsettingbackend.h>

#include <DApplicationHelper>
#include <DBackgroundGroup>
#include <DCheckBox>
#include <DDialog>
#include <DFloatingMessage>
#include <DGuiApplicationHelper>
#include <DListView>
#include <DMessageManager>
#include <DRadioButton>
#include <DSettingsDialog>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DStandardItem>
#include <DSysInfo>
#include <DTitlebar>
#include <DWarningButton>

#include <QDBusInterface>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QLocale>
#include <QMetaEnum>
#include <QObject>
#include <QPushButton>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QWidget>

DEF_USING_NAMESPACE
DTK_USE_NAMESPACE
DCORE_USE_NAMESPACE

const int WidgetMinimumWidget = 950;
const int WidgetMinimumHeight = 640;

// 此处为带边距的宽度
const int first_widget_min_width = 179;

const QMargins navItemMargin(5, 3, 5, 3);
const QVariant NavItemMargin = QVariant::fromValue(navItemMargin);
const QString dmanInterface = "com.deepin.Manual.Open";

#define DIALOG_WIDTH 380 // dialog  宽度
#define DIALOG_HEIGHT 145 // dialog  高度
#define DIALOG_PB_WIDTH 140 // dialog  按钮宽度
#define DIALOG_PB_HEIGHT 36 // dialog  按钮高度

#define INSTALL_TIME "defender-install-time" // 安装时间

/******************安全中心主界面初始化******************/
MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_contentLayout(nullptr)
    , m_splitLine(nullptr)
    , m_rightContentLayout(nullptr)
    , m_navView(nullptr)
    , m_rightView(nullptr)
    , m_navModel(nullptr)
    , m_homePageModule(nullptr)
    , m_homePageModel(nullptr)
    , m_securityToolsModule(nullptr)
    , m_backwardBtn(nullptr)
    , m_pSystemTray(nullptr)
    , m_trayMenu(nullptr)
    , m_trayDialag(nullptr)
    , m_gsetting(nullptr)
    , m_dsd(nullptr)
    , m_guiHelper(nullptr)
    , m_pSecurityToolDBusInter(nullptr)
{
    this->setAccessibleName("mainWindow");
    setObjectName(MAIN_WINDOW_OBJ_NAME);
    // 设置主窗口属性
    setFixedSize(QSize(WidgetMinimumWidget, WidgetMinimumHeight));
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    // 初始化UI
    initUI();
    // 初始化数据
    initData();
    // 初始化信号槽
    initSignalSlots();
    // 初始化托盘
    initTray();

    QTimer::singleShot(0, this, &MainWindow::initAllModule); // 初始化所有模块
    QTimer::singleShot(0, this, &MainWindow::modulePreInitialize); // 模块初始化前准备
}

MainWindow::~MainWindow()
{
    if (m_dsd) {
        m_dsd->deleteLater();
        m_dsd = nullptr;
    }

    if (m_navModel) {
        m_navModel->clear();
        m_navModel->deleteLater();
        m_navModel = nullptr;
    }
}

/********************安全中心模块加载**********************/
// 初始化UI
void MainWindow::initUI()
{
    // Initialize view and layout structure
    QWidget *content = new QWidget(this);
    content->setAccessibleName("mainWindow_contentWidget");
    setCentralWidget(content);

    m_contentLayout = new QHBoxLayout(content);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setMargin(0);
    m_contentLayout->setSpacing(0);
    // 左边list
    m_navView = new MultiSelectListView(this);
    m_navView->setLineWidth(0);
    m_navView->setAccessibleName("contentWidget_selectListView");
    m_navView->setUniformItemSizes(true);
    m_navView->setItemSize(QSize(159, 48));
    m_navView->setIconSize(QSize(48, 48));
    m_navView->setFrameShape(QFrame::Shape::NoFrame);
    m_navView->setEditTriggers(QListView::NoEditTriggers);
    m_navView->setAutoScroll(true); // 滚动条
    m_navView->setTabKeyNavigation(false); // tab按键是否有效
    m_navView->setViewportMargins(QMargins(10, 0, 10, 0)); // 滚动区域边框大小
    m_navView->setFixedWidth(first_widget_min_width); // 固定窗口值
    m_navView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_navView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_navView->setAutoFillBackground(true);
    m_navView->setBackgroundType(
        DStyledItemDelegate::BackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground
                                            | DStyledItemDelegate::BackgroundType::NoNormalState));
    m_navView->setItemSpacing(0);
    m_contentLayout->addWidget(m_navView);

    // 分割线
    m_splitLine = new DFrame(this);
    m_splitLine->setAccessibleName("splitLineFrame");
    m_splitLine->setFrameShape(QFrame::VLine);
    m_splitLine->setFrameShadow(QFrame::Plain);
    m_splitLine->setFixedHeight(600);
    m_splitLine->setFixedWidth(1);
    m_contentLayout->addWidget(m_splitLine);

    // 右侧布局
    m_rightContentLayout = new QHBoxLayout;
    m_rightContentLayout->setContentsMargins(0, 0, 0, 0);

    m_rightView = new DBackgroundGroup(m_rightContentLayout);
    m_rightView->setAccessibleName("contentWidget_rightWidget");
    m_rightView->setItemSpacing(2);
    m_rightView->setItemMargins(QMargins(0, 0, 0, 0));
    m_rightView->setContentsMargins(0, 0, 0, 0);
    m_rightView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentLayout->addWidget(m_rightView);
}

// 初始化数据
void MainWindow::initData()
{
    m_pSecurityToolDBusInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.securitytooldialog",
                                                    "/com/deepin/pc/manager/securitytooldialog",
                                                    "com.deepin.pc.manager.securitytooldialog",
                                                    ConnectType::SESSION,
                                                    this);

    // 初始化安全中心gsetting配置
    m_gsetting = InvokerFactory::GetInstance().CreateSettings(DEEPIN_PC_MANAGER_GSETTING_PATH,
                                                              QByteArray(),
                                                              this);

    // 设置安装时间
    setInstallTime();

    // 主题变换
    m_guiHelper = DGuiApplicationHelper::instance();
    connect(m_guiHelper,
            SIGNAL(themeTypeChanged(ColorType)),
            this,
            SLOT(themeTypeChange(ColorType)));

    // 标题栏
    DTitlebar *titlebar = this->titlebar();
    titlebar->setIcon(QIcon::fromTheme(DIALOG_PC_MANAGER));

    auto menu = titlebar->menu();
    menu->setFont(Utils::getFixFontSize(14));
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

    m_backwardBtn = new DIconButton(this); // 上一步按钮
    m_backwardBtn->setAccessibleName("backwardButton");
    m_backwardBtn->setIcon(QStyle::SP_ArrowBack);
    titlebar->addWidget(m_backwardBtn, Qt::AlignLeft | Qt::AlignVCenter);
    connect(m_backwardBtn, &DIconButton::clicked, this, &MainWindow::onBackWardClick);

    // Initialize top page view and model
    m_navModel = new QStandardItemModel(m_navView);
    m_navView->setModel(m_navModel);
    // 连接按下信号与窗口切换槽函数
    connect(m_navView, &DListView::pressed, this, &MainWindow::onFirstItemClick);
}

// 初始化信号槽
void MainWindow::initSignalSlots()
{
}

void MainWindow::popWidget(ModuleInterface *const inter)
{
    // 外部模块调用时，判断是否当前模块界面处于首页
    if (m_contentStack.size()) {
        if (inter == m_contentStack.top().first) {
            // 将最后一层窗口出栈
            popWidget();
        }
    }
}

void MainWindow::popAndDelWidget(ModuleInterface *const inter)
{
    Q_UNUSED(inter)
    // 由外部模块发起的pop需要过滤
    if (m_contentStack.top().first != inter) {
        return;
    }

    // 将最后一层窗口出栈并删除
    popAndDelWidget();
    updateBackwardBtn();
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
    auto find_it = std::find_if(m_modules.cbegin(),
                                m_modules.cend(),
                                [=](const QPair<ModuleInterface *, QString> &pair) {
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
    this->setFocus();

    // 激活模块
    ModuleInterface *inter = m_modules[modelIndex.row()].first;
    inter->active(iPageIdx);
}

ModuleInterface *MainWindow::getCurrentModule()
{
    if (m_contentStack.isEmpty()) {
        return nullptr;
    }

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

void MainWindow::setBackForwardButtonStatus(bool status)
{
    if (m_backwardBtn) {
        m_backwardBtn->setEnabled(status);
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

// 初始化模块
void MainWindow::initAllModule()
{
    // 首页体检模块
    if (!m_homePageModule) {
        m_homePageModule = new HomePageModule(this, this);
    }
    if (!m_securityToolsModule) {
        m_securityToolsModule = new SecurityToolsModule(this, this);
    }
    m_modules = {
        {m_homePageModule, tr("Full Check")},
        {new CleanerModule(this, this), tr("Cleanup")},
        {m_securityToolsModule, tr("Tools")},
    };

    // 浅色主题图标
    m_modulesLightIconList.clear();
    m_modulesLightIconList << HOMEPAGE_MODEL_LIGHT << HOMEPAGE_CLEAR_LIGHT << HOMEPAGE_TOOL_LIGHT;

    // 深色主题图标
    m_modulesDarakIconList.clear();
    m_modulesDarakIconList << HOMEPAGE_MODEL_DARK << HOMEPAGE_CLEAR_DARK << HOMEPAGE_TOOL_DARK;

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

        item->setData(Utils::getFixFontSize(14), Qt::FontRole);
        m_navModel->appendRow(item);
    }
}

// 模块初始化前准备
void MainWindow::modulePreInitialize()
{
    for (auto it = m_modules.cbegin(); it != m_modules.cend(); ++it) {
        QElapsedTimer et;
        et.start();
        it->first->preInitialize();
    }

    QModelIndex index0 = m_navModel->index(0, 0);
    if (index0.isValid()) {
        onFirstItemClick(index0); // 默认点击首页
    }
}

// 左侧列表的点击
void MainWindow::onFirstItemClick(const QModelIndex &index)
{
    m_navView->setCurrentIndex(index);
    ModuleInterface *inter = m_modules[index.row()].first;

    // 有数据  并且最后一个和现在点击的一致
    if (!m_contentStack.isEmpty() && m_contentStack.last().first == inter) {
        return;
    }
    popAllWidgets(0);

    inter->active(0);
    m_navView->resetStatus(index);
    m_navView->setFocus();
}

// 模块入栈
void MainWindow::pushNormalWidget(ModuleInterface *const inter, QWidget *const w)
{
    // Set the newly added page to fill the blank area
    w->setParent(this);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    w->show();

    m_contentStack.push({inter, w});
    m_rightContentLayout->addWidget(w, m_contentStack.size() == 1 ? 3 : 7);
    qInfo() << "[MainWindow] [pushNormalWidget] widget name " << w;
}

// 功能模块切换出栈
void MainWindow::popAllWidgets(int place)
{
    Q_UNUSED(place);
    if (!m_contentStack.count())
        return;

    // 系统体检、病毒扫描、垃圾清理widget无需delete
    // 防火墙、安全防护、安全工具widget需要delete
    // popWidget();
    for (int pageCount = m_contentStack.count(); pageCount > place; pageCount--) {
        popWidget(); // 是否删除界面，由模块在deactive时决定
    }

    // pop delete all, means we need to deactive all and active someone later
    for (auto module : m_modules) {
        module.first->deactive();
    }

    // 清除栈容器内所有对象 --确保外部接口调用菜单显示之前栈容器为空
    m_contentStack.clear();
}

// 模块出栈
void MainWindow::popAndDelWidget()
{
    if (!m_contentStack.size())
        return;

    QWidget *w = m_contentStack.pop().second; // 栈的顶栈的widget
    if (w) {
        m_rightContentLayout->removeWidget(w);
        w->hide();
        w->deleteLater();
    }
}

// 模块出栈
void MainWindow::popWidget()
{
    if (!m_contentStack.size())
        return;

    // 反激活切换前的模块 - widget是否需要delete，由各个模块的module自己决定

    // 掩藏/移除切换前的模块
    QWidget *w = m_contentStack.pop().second; // 栈的顶栈的widget
    if (w) {
        w->hide();
        m_rightContentLayout->removeWidget(w);

        qInfo() << "[MainWindow] [popWidget] widget name " << w;
    }
}

/********************安全中心菜单配置**************************/
// 显示默认设置弹框
void MainWindow::showDefaultSettingDialog()
{
    showSettingDialog("");
}

// 显示用户反馈弹框
void MainWindow::showUserReplyDialog()
{
    // 创建（或new）对象连接服务
    QDBusInterface interFace("com.deepin.dde.ServiceAndSupport",
                             "/com/deepin/dde/ServiceAndSupport",
                             "com.deepin.dde.ServiceAndSupport",
                             QDBusConnection::sessionBus());
    interFace.call("ServiceSession", 1);
}

// 显示设置弹窗，并跳转到相应组位置
void MainWindow::showSettingDialog(const QString &groupKey)
{
    QTemporaryFile *tmpFile = new QTemporaryFile;
    tmpFile->open();
    auto backend = new Dtk::Core::QSettingBackend(tmpFile->fileName(), this);

    // 创建设置数据
    Dtk::Core::DSettings *settings;

    settings = Dtk::Core::DSettings::fromJsonFile(SETTING_JSON);
    settings->setParent(this);
    settings->setBackend(backend);

    // 脚本直接设置的部分
    /* 基础设置 */
    // 基础设置-根据关闭窗口属性gsetting配置，更新设置界面选项
    // 优先显示每次询问按钮是否选中
    if (0 == this->getAskType()) {
        settings->setOption(SETTING_BASE_CLOSE_WINDOW, getCloseType());
    } else if (1 == this->getAskType()) {
        settings->setOption(SETTING_BASE_CLOSE_WINDOW, 2);
    }

    // 连接设置信号
    // 脚本直接设置的部分
    connect(settings,
            &Dtk::Core::DSettings::valueChanged,
            this,
            [=](const QString &key, const QVariant &value) {
                if (key.contains(SETTING_CLOSE_WINDOW_TYPE)) {
                    int type = value.toInt();
                    if (type >= Tray && type <= Exit) {
                        this->setCloseType(type); // 设置关闭类型 托盘还是推出
                        this->setAskType(0); // 设置访问类型
                    } else if (type == Ask) {
                        this->setAskType(1); // 设置访问类型是每次访问
                    }
                }
            });

    // 设置弹窗
    m_dsd = new DSettingsDialog(this);
    m_dsd->setFont(Utils::getFixFontSize(14));
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

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason state)
{
    if (state == QSystemTrayIcon::ActivationReason::Trigger) {
        if (!this->isVisible()) {
            // 页面隐藏，点击托盘重新show
            this->show();
        } else {
            // 页面能显示的时候，点击托盘会＂正常/最小化＂切换窗口
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
    // 设置后台服务弹框主题
    int type = int(themeType);
    DBUS_NOBLOCK_INVOKE(m_pSecurityToolDBusInter, "SetDefenderPaletteType", type);

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
    m_pSystemTray->setIcon(QIcon::fromTheme(DIALOG_PC_MANAGER));
    connect(m_pSystemTray, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);

    m_pSystemTray->show();
}

// 关闭弹窗  只有在设置是访问才有这个
void MainWindow::initCloseDialog(QCloseEvent *event)
{
    m_trayDialag = new DDialog(this);
    m_trayDialag->setAccessibleName("trayDialog");
    m_trayDialag->setModal(true);
    m_trayDialag->setIcon(QIcon::fromTheme(DIALOG_PC_MANAGER));
    DRadioButton *radioExit = new DRadioButton(tr("Exit"), this);
    radioExit->setAccessibleName("trayDialog_exitButton");
    DRadioButton *radioMin = new DRadioButton(tr("Minimize to system tray"), this);
    radioMin->setAccessibleName("trayDialog_minimizeButton");
    DCheckBox *radioAsk = new DCheckBox(tr("Do not ask again"), this);
    radioAsk->setAccessibleName("trayDialog_askButton");
    closeType nChoose = static_cast<closeType>(getCloseType());

    // 根据GSettings设置settings页面的默认值
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
    cancelBtn->setText(tr("Cancel", "button"));
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setAccessibleName("trayDialog_cancelButton");
    cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
    m_trayDialag->insertButton(0, cancelBtn, false);

    QPushButton *confirmBtn = new QPushButton(m_trayDialag);

    confirmBtn->setText(tr("Confirm", "button"));
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
        if (radioMin->isChecked()) { // 最小花点击
            this->hide();
            event->ignore();
        } else if (radioExit->isChecked()) { // 退出点击
            closeWindow();
            event->ignore();
        }

        // 根据选择的方式  设置关闭方式
        if (radioMin->isChecked()) {
            setCloseType(Tray);
        } else if (radioExit->isChecked()) {
            setCloseType(Exit);
        }

        // 只有当选择radioAsk后才会设置不再询问
        if (radioAsk->isChecked()) {
            setAskType(0);
        }

        m_trayDialag->deleteLater();
        m_trayDialag = nullptr;
    }
}

// 获得关闭类型 0托盘最小化 1推出
int MainWindow::getCloseType()
{
    return m_gsetting ? m_gsetting->GetValue(CLOSE_TYPE).toInt() : 0;
}

// 0 : Not Ask  1 : Ask      0 托盘最小化  1 退出
void MainWindow::setCloseType(int value)
{
    if (m_gsetting) {
        m_gsetting->SetValue(CLOSE_TYPE, value);
    }
}

// 获得访问类型 0不访问 1访问
int MainWindow::getAskType()
{
    return m_gsetting ? m_gsetting->GetValue(ASK_TYPE).toInt() : 0;
}

// 设置访问  0 不访问 1访问
void MainWindow::setAskType(int value)
{
    if (m_gsetting) {
        m_gsetting->SetValue(ASK_TYPE, value);
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
    } else if (TRUSTED_PROTECTION_NAME == pageName) {
        // 如果是登录安全
        retIndex = TRUSTED_PROTECTION_INDEX;
    }
    return retIndex;
}

// 点击退回按钮槽
void MainWindow::onBackWardClick(bool checked)
{
    Q_UNUSED(checked);

    if (m_contentStack.isEmpty()) {
        return;
    }

    // 针对病毒查杀页 进行后退判断和处理
    QString sModuleName = m_contentStack.top().first->name();
    if (MODULE_HOMEPAGE_NAME == sModuleName) {
        m_homePageModule->requestStopExaming();
        m_homePageModule->onBackForward();
        popAndDelWidget();
    } else if (MODULE_SECURITY_TOOLS_NAME == sModuleName
               || MODULE_DISK_CLEANER_NAME == sModuleName) {
        // MainWindow统一管理子模块窗口的删除释放
        popAndDelWidget();
    }

    // 这里需要保证m_contentStack.size()>0,避免崩溃
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
    if (m_contentStack.isEmpty()) {
        return;
    }

    m_backwardBtn->setVisible(false);

    // 只对 病毒查杀页、安全工具、防火墙页 作后退按钮判断
    QString sModuleName = m_contentStack.top().first->name();
    if (MODULE_SECURITY_TOOLS_NAME == sModuleName || MODULE_DISK_CLEANER_NAME == sModuleName
        || MODULE_HOMEPAGE_NAME == sModuleName) {
        if (1 < m_contentStack.count()) {
            m_backwardBtn->setVisible(true);
            m_backwardBtn->setDisabled(false);
        } else {
            m_backwardBtn->setVisible(false);
        }
    }
}

void MainWindow::closeWindow()
{
    qApp->quit();
}

// 设置安装时间
void MainWindow::setInstallTime()
{
    // 获得安装时间  如果是默认值-1 设置当前时间为安装时间
    QString lasttime = m_gsetting->GetValue(INSTALL_TIME).toString();
    QString defaultTime = "-1";
    if (lasttime == defaultTime) {
        // 当前时间
        QDateTime date = QDateTime::currentDateTime();
        QString installTime = date.toString("yyyy-MM-dd hh:mm:ss");
        m_gsetting->SetValue(INSTALL_TIME, installTime);
    }
}
