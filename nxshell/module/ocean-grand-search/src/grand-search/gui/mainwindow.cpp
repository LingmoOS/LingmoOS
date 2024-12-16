// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow_p.h"
#include "mainwindow.h"
#include "utils/utils.h"
#include "entrance/entrancewidget.h"
#include "exhibition/exhibitionwidget.h"
#include "gui/datadefine.h"
#include "gui/handlevisibility/handlevisibility.h"
#include "business/query/querycontroller.h"
#include "business/matchresult/matchcontroller.h"

#include <DApplication>
#include <DLabel>
#include <DRegionMonitor>
#include <DPlatformHandle>

#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <QScreen>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static const uint MainWindowWidth           = 760;      // 主界面宽度
static const uint MainWindowHeight          = 68;       // 主界面高度
static const uint MainWindowExpandHeight    = 520;      // 主界面高度

MainWindowPrivate::MainWindowPrivate(MainWindow *parent)
    : q_p(parent)
{
    m_handleVisibility = new HandleVisibility(q_p, q_p);
}

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , d_p(new MainWindowPrivate(this))
{
    initUI();
    initConnect();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    activeMainWindow();
    return DBlurEffectWidget::mousePressEvent(event);
}

MainWindow::~MainWindow()
{

}

void MainWindow::connectToController()
{
    Q_ASSERT(d_p->m_entranceWidget);
    Q_ASSERT(d_p->m_exhibitionWidget);
    Q_ASSERT(!d_p->m_queryController);
    Q_ASSERT(!d_p->m_matchController);

    d_p->m_queryController = new QueryController(this);
    d_p->m_matchController = new MatchController(this);

    // 通知查询控制模块发起新的搜索
    connect(d_p->m_entranceWidget, &EntranceWidget::searchTextChanged, d_p->m_queryController, &QueryController::onSearchTextChanged);

    // 通知查询控制器停止搜索
    connect(this, &MainWindow::terminateSearch, d_p->m_queryController, &QueryController::onTerminateSearch);

    // 同步查询搜索任务
    connect(d_p->m_queryController, &QueryController::missionChanged, d_p->m_matchController, &MatchController::onMissionChanged);
    connect(d_p->m_queryController, &QueryController::missionChanged, this, &MainWindow::onMissionChanged);

    // 匹配结果解析显示
    connect(d_p->m_matchController, &MatchController::matchedResult, d_p->m_exhibitionWidget, &ExhibitionWidget::appendMatchedData);
    connect(d_p->m_matchController, &MatchController::searchCompleted, d_p->m_exhibitionWidget, &ExhibitionWidget::onSearchCompleted);
}

void MainWindow::showExhitionWidget(bool bShow)
{
    Q_ASSERT(d_p->m_exhibitionWidget);

    //已经显示/隐藏，就不再重复显示/隐藏
    if (d_p->m_exhibitionWidget && !d_p->m_exhibitionWidget->isHidden() == bShow)
        return;

    d_p->m_exhibitionWidget->setVisible(bShow);
}

void MainWindow::showSerachNoContent(bool bShow)
{
    Q_ASSERT(d_p->m_entranceWidget);
    Q_ASSERT(d_p->m_exhibitionWidget);

    if (Q_LIKELY(d_p->m_searchNoContentWidget))
        d_p->m_searchNoContentWidget->setVisible(bShow);

    if (bShow) {
        if (Q_LIKELY(d_p->m_exhibitionWidget))
            d_p->m_exhibitionWidget->setVisible(false);
        // 显示无搜索结果时，必定在搜索入口界面中不显示应用图标
        showEntranceAppIcon(false);
    }
}

void MainWindow::showEntranceAppIcon(bool bShow)
{
    Q_ASSERT(d_p->m_entranceWidget);

    d_p->m_entranceWidget->showLabelAppIcon(bShow);
}

void MainWindow::onFocusObjectChanged(QObject *obj)
{
    // 焦点回到主窗口时，将焦点设置到输入框中(焦点改变到了预览相关的窗口中则不做处理)
    if (obj && obj == this && d_p->m_entranceWidget) {
        d_p->m_entranceWidget->setFocus();
    }
}

void MainWindow::onPrimaryScreenChanged(const QScreen *screen)
{
    // 主窗口显示在主屏
    disconnect(this, SLOT(onGeometryChanged(const QRect &)));
    onGeometryChanged(screen->geometry());
    connect(screen, &QScreen::geometryChanged, this, &MainWindow::onGeometryChanged);
}

void MainWindow::onGeometryChanged(const QRect &geometry)
{
    int sWidth = geometry.width();
    int sHeight = geometry.height();

    // 移动窗口到屏幕的居中偏上位置
    sWidth = geometry.x() + sWidth/2 - int(MainWindowWidth/2);
    sHeight = geometry.y() + sHeight/4;
    move(sWidth, sHeight);
}

void MainWindow::onHideExhitionWidget()
{
    // 搜索文本为空，隐藏展示界面
    showSerachNoContent(false);
    showExhitionWidget(false);
    showEntranceAppIcon(false);

    updateMainWindowHeight();

    Q_ASSERT(d_p->m_exhibitionWidget);
    d_p->m_exhibitionWidget->clearData();
}

void MainWindow::onResetExhitionWidget(const QString &missionId)
{
    Q_UNUSED(missionId);

    // 搜索任务id改变，重置展示界面
    showSerachNoContent(false);
    showExhitionWidget(true);

    updateMainWindowHeight();

    Q_ASSERT(d_p->m_exhibitionWidget);
    d_p->m_exhibitionWidget->clearData();
}

void MainWindow::onMissionChanged(const QString &missionId, const QString &missionContent)
{
    onResetExhitionWidget(missionId);

    if (missionContent.isEmpty()) {
        onHideExhitionWidget();
    }
}

void MainWindow::initUI()
{
    // 禁用窗口管理器并置顶
    if (Utils::isWayland()) {
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_QuitOnClose);
    } else {
        // 在Qt5.11版本、x11环境下，同时设置BypassWindowManagerHint与Tool属性之后，键盘切换大小写将会导致程序的激活状态发生改变。
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    }
    qDebug() << "current platform name:" << QApplication::platformName() << "   flags:" << this->windowFlags();

    // 控制界面大小和位置
    setFixedSize(MainWindowWidth, MainWindowHeight);
    QScreen *screen = qApp->primaryScreen();
    onPrimaryScreenChanged(screen);

    // 模糊模式
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    // 搜索入口界面
    d_p->m_entranceWidget = new EntranceWidget(this);
    d_p->m_entranceWidget->setFixedHeight(MainWindowHeight);

    // 结果展示界面
    d_p->m_exhibitionWidget = new ExhibitionWidget(this);
    d_p->m_exhibitionWidget->hide();

    // 未搜到内容界面
    d_p->m_searchNoContentWidget = new DLabel(tr("No search results"),this);
    d_p->m_searchNoContentWidget->setAlignment(Qt::AlignCenter);
    d_p->m_searchNoContentWidget->hide();

    d_p->m_mainLayout = new QVBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_entranceWidget);
    d_p->m_mainLayout->addWidget(d_p->m_exhibitionWidget);
    d_p->m_mainLayout->addWidget(d_p->m_searchNoContentWidget);

    // 根据设计图调整主界面布局，限制边距和内容间距为0
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setMargin(0);

    this->setLayout(d_p->m_mainLayout);

    this->winId();
    DPlatformHandle handle(this->windowHandle());
    handle.setWindowRadius(18);
}

void MainWindow::initConnect()
{
    Q_ASSERT(d_p->m_entranceWidget);
    Q_ASSERT(d_p->m_exhibitionWidget);

    // 监控主屏改变信号，及时更新窗口位置
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &MainWindow::onPrimaryScreenChanged);

    connect(d_p->m_entranceWidget, &EntranceWidget::sigSelectPreviousItem, d_p->m_exhibitionWidget, &ExhibitionWidget::onSelectPreviousItem);
    connect(d_p->m_entranceWidget, &EntranceWidget::sigSelectNextItem, d_p->m_exhibitionWidget, &ExhibitionWidget::onSelectNextItem);
    connect(d_p->m_entranceWidget, &EntranceWidget::sigHandleItem, d_p->m_exhibitionWidget, &ExhibitionWidget::onHandleItem);
    connect(d_p->m_entranceWidget, &EntranceWidget::sigCloseWindow, d_p->m_handleVisibility, &HandleVisibility::onCloseWindow);

    connect(d_p->m_exhibitionWidget, &ExhibitionWidget::sigCurrentItemChanged, d_p->m_entranceWidget, &EntranceWidget::onAppIconChanged);
    connect(d_p->m_exhibitionWidget, &ExhibitionWidget::sigShowNoContent, this, &MainWindow::showSerachNoContent);
    connect(d_p->m_exhibitionWidget, &ExhibitionWidget::sigCloseWindow, d_p->m_handleVisibility, &HandleVisibility::onCloseWindow);

    connect(qApp, &QGuiApplication::focusObjectChanged, this, &MainWindow::onFocusObjectChanged);
}

void MainWindow::activeMainWindow()
{
    activateWindow();
    raise();
}

void MainWindow::updateMainWindowHeight()
{
    Q_ASSERT(d_p->m_exhibitionWidget);
    Q_ASSERT(d_p->m_searchNoContentWidget);

    bool bExhibitionWidgetShow = false;
    bool bSearchNoContentWidgetShow = false;

    if (Q_LIKELY(d_p->m_exhibitionWidget))
        bExhibitionWidgetShow = !d_p->m_exhibitionWidget->isHidden();
    if (Q_LIKELY(d_p->m_searchNoContentWidget))
        bSearchNoContentWidgetShow = !d_p->m_searchNoContentWidget->isHidden();

    if (bExhibitionWidgetShow || bSearchNoContentWidgetShow)
        this->setFixedSize(MainWindowWidth,MainWindowExpandHeight);
    else
        this->setFixedSize(MainWindowWidth,MainWindowHeight);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMetaObject::invokeMethod(this, "visibleChanged", Qt::QueuedConnection, Q_ARG(bool, true));

    // 已禁用窗口管理器，在窗口被显示后，需要激活该窗口
    QTimer::singleShot(1, this, &MainWindow::activeMainWindow);

    d_p->m_handleVisibility->registerRegion(true);

    return DBlurEffectWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    emit visibleChanged(false);

    d_p->m_handleVisibility->registerRegion(false);

    return DBlurEffectWidget::hideEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 通知查询控制器停止搜索
    emit terminateSearch();
    // FIXME: DBlurEffectWidget close abort on treeland
    qApp->exit(0);
    _Exit(0);
    // return DBlurEffectWidget::closeEvent(event);
}
