// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

static const QSize WINDOW_SIZE {699, 449};

MainWindow::MainWindow(DWidget *parent)
    : DMainWindow(parent)
    , m_index(1)
    , m_current(nullptr)
    , m_last(nullptr)
    , m_currentAni(new QPropertyAnimation(this))
    , m_lastAni(new QPropertyAnimation(this))
    , m_settings(new QSettings("deepin", "dde-introduction"))
    , m_displayInter(new WMSwitcherInter("com.deepin.WMSwitcher", "/com/deepin/WMSwitcher",
                                         QDBusConnection::sessionBus(), this))
{
    qInfo() << "Current cpu architecture : " << QSysInfo::currentCpuArchitecture();
    //这个判断版本　QSysInfo
    bool isx86 = QSysInfo::currentCpuArchitecture().startsWith("x86");
    if (isx86) {
        if (DSysInfo::uosType() == DSysInfo::UosType::UosServer ||
            DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterpriseC ||
            DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEuler) {
            m_useVideo = false; //x86架构，服务行业版和服务器欧拉版都是轮播图
        } else {
            m_useVideo = true;
        }
    } else {
        m_useVideo = false;
    }

    setWindowFlags(Qt::CustomizeWindowHint);

    titlebar()->setMenuVisible(false);
    titlebar()->setIcon(QIcon::fromTheme("dde-introduction"));
    //适配1050Dtk不再聚焦TitleBar，现在也无需自绘了
    QWidget *EmptyWidget = new QWidget(this);
    EmptyWidget->setFocusPolicy(Qt::TabFocus);
    EmptyWidget->setGeometry(0,0,0,0);
    setTabOrder(m_pCloseBtn,EmptyWidget);

    setFixedSize(WINDOW_SIZE);
    setTitlebarShadowEnabled(false);

    //UosEnterprise:服务器企业版　　UosEnterpriseC:行业版   UosEuler:服务器欧拉版
    bool bIsServerSystemType;
    if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterprise  ||
        DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterpriseC ||
        DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEuler) {
        bIsServerSystemType = true;
    } else {
        bIsServerSystemType = false;
    }

    //获取deepin版本
    const DSysInfo::DeepinType DeepinType = DSysInfo::deepinType();
    bool bIsDeepinServerType = (DSysInfo::DeepinServer == DeepinType);

    //使用DBUS获取窗口特效是否支持切换
    bool isSuportEffect = QDBusInterface("com.deepin.wm", "/com/deepin/wm", "com.deepin.wm")
                                        .property("compositingAllowSwitch")
                                        .toBool();

    //服务器企业版、行业版、欧拉版、Deepin服务器版不支持窗口特效
    (bIsServerSystemType  || bIsDeepinServerType || !isSuportEffect || Worker::isWaylandType()) ? m_supportWM = false : m_supportWM = true;

    initUI();
    initConnect();

    //设置首次启动的tab顺序
    if (m_isFirst) {
        setTabOrder(m_previousBtn, m_nextBtn);
        //适配1050Dtk不再聚焦TitleBar
        setTabOrder(m_pCloseBtn,EmptyWidget);
        setTabOrder(m_previousBtn, m_doneBtn);
    }
}

MainWindow::~MainWindow()
{
    if(m_current) {
        delete m_current;
        m_current = nullptr;
    }
    if(m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }

    if (m_isFirst) {
        if (!DDE_STARTGUIDE_PATH.isEmpty()) {
            QProcess *pStartAppProcess = new QProcess(this);
            pStartAppProcess->startDetached(DDE_STARTGUIDE_PATH);
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_isFirst) {
        switch (m_index) {
            case 1:
#ifndef DISABLE_VIDEO
            static_cast<VideoWidget *>(m_current)->keyPressEvent(event);
#else
            static_cast<PhotoSlide *>(m_current)->keyPressEvent(event);
#endif
                break;
            case 2:
                static_cast<DesktopModeModule *>(
                    static_cast<BaseModuleWidget *>(m_current)->getModel())
                    ->keyPressEvent(event);
                break;
            case 3:
                static_cast<WMModeModule *>(
                    static_cast<BaseModuleWidget *>(m_current)->getModel())
                    ->keyPressEvent(event);
                break;
            case 4:
                static_cast<IconModule *>(
                    static_cast<BaseModuleWidget *>(m_current)->getModel())
                    ->keyPressEvent(event);
                break;
            default:
                break;
        }
    } else {
        static_cast<NormalModule *>(m_current)->keyPressEvent(event);
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->close();
}

/*******************************************************************************
 1. @函数:    previous
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    上一步按钮响应函数
*******************************************************************************/
void MainWindow::previous()
{
    if (m_currentAni->state() == QPropertyAnimation::Running) {
        return;
    }

    if (!m_supportWM && m_index == 4){
        m_index--;
    }

    //创建上一步按钮选择的界面
    updateModule(--m_index);

    //这段代码为什么变量混着写,可以整改
    m_currentAni->setDuration(300);
    m_lastAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_currentAni->setStartValue(QPoint(m_last->x() - m_last->width(), 0));
    m_currentAni->setEndValue(m_last->rect().topLeft());

    m_lastAni->setDuration(300);
    m_currentAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_lastAni->setStartValue(m_last->rect().topLeft());
    m_lastAni->setEndValue(m_last->rect().topRight());

    m_currentAni->start();
    m_lastAni->start();
}

/*******************************************************************************
 1. @函数:    next
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    下一步按钮响应函数
*******************************************************************************/
void MainWindow::next()
{
    if (m_currentAni->state() == QPropertyAnimation::Running) {
        return;
    }

    //previous函数中添加的窗口特效是否支持判断，因为updateModule函数中第三步判断有问题，只有++
    // create new QWidget, change pointer
    //创建下一步按钮选择的界面
    updateModule(++m_index);

    m_lastAni->setDuration(300);
    m_lastAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_lastAni->setStartValue(m_last->rect().topLeft());
    m_lastAni->setEndValue(QPoint(m_last->x() - m_last->width(), 0));

    m_currentAni->setDuration(300);
    m_currentAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_currentAni->setStartValue(QPoint(m_last->rect().topRight()));
    m_currentAni->setEndValue(QPoint(0, 0));

    m_currentAni->start();
    m_lastAni->start();
}

/*******************************************************************************
 1. @函数:    slotTheme
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    主题变化响应函数
*******************************************************************************/
void MainWindow::slotTheme()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::LightType) {
        if (m_isFirst){
            if(m_nextBtn == nullptr || m_doneBtn == nullptr) return;
            DPalette nextPa = m_nextBtn->palette();
            nextPa.setColor(DPalette::ButtonText, QColor(65, 77, 104, 255));
            m_nextBtn->setPalette(nextPa);
            m_doneBtn->setPalette(nextPa);
        }
        DPalette pl = this->palette();
        pl.setColor(DPalette::Window, Qt::white);
        this->setPalette(pl);
        m_pCloseBtn->setIcon(QIcon(":/resources/close_normal_light.svg"));
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType) {
        if (m_isFirst){
            if(m_nextBtn == nullptr || m_doneBtn == nullptr) return;
            DPalette nextPa = m_nextBtn->palette();
            nextPa.setColor(DPalette::ButtonText, QColor(192, 198, 212, 255));
            m_nextBtn->setPalette(nextPa);
            m_doneBtn->setPalette(nextPa);
        }
        DPalette pl = this->palette();
        pl.setColor(DPalette::Window, QColor("#282828"));
        this->setPalette(pl);
        m_pCloseBtn->setIcon(QIcon(":/resources/close_normal_dark.svg"));
    }
    this->setForegroundRole(DPalette::Window);
    this->setBackgroundRole(DPalette::Window);
}

void MainWindow::initUI()
{
    //设置圆角
    DPlatformWindowHandle handle(this);
    handle.setWindowRadius(10);

    m_fakerWidget = new DWidget(this);
    m_fakerWidget->setFixedSize(700, 450);

    m_pCloseBtn = new DIconButton(this);
    m_pCloseBtn->setFlat(true);
    m_pCloseBtn->setFixedSize(51, 51);
    m_pCloseBtn->setIconSize(QSize(51, 51));

    m_pCloseBtn->move(rect().topRight() - QPoint(m_pCloseBtn->width(), -1));
    connect(m_pCloseBtn, &DIconButton::clicked, this, &MainWindow::close);

    bool isFirst = m_settings->value("IsFirst", true).toBool();
    m_isFirst = isFirst;

    if (isFirst) {
        //首次启动欢迎时关闭键可以使用Tab键聚焦
        m_pCloseBtn->setFocusPolicy(Qt::TabFocus);
        m_nextBtn = new NextButton(tr("Next"), this);
        m_doneBtn = new NextButton(tr("Done"), this);
        m_previousBtn = new previousButton(this);
        m_previousBtn->setIcon(QStyle::StandardPixmap::SP_ArrowBack);
        m_nextBtn->setFixedSize(100, 36);
        m_doneBtn->setFixedSize(100, 36);

        // Addition Button Shadow　//按键阴影
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        shadow_effect->setOffset(0, 2);
        shadow_effect->setColor(QColor(0, 0, 0, int(0.05 * 255)));
        shadow_effect->setBlurRadius(4);
        m_nextBtn->setGraphicsEffect(shadow_effect);
        m_previousBtn->setGraphicsEffect(shadow_effect);

        m_previousBtn->move(10, 404);
        m_nextBtn->move(590, 404);
        m_doneBtn->move(m_nextBtn->pos());
        m_doneBtn->hide();

        //上一步和下一步选择的界面的动画
        m_currentAni->setPropertyName("pos");
        //当前还没变化的界面的动画
        m_lastAni->setPropertyName("pos");

        m_settings->setValue("IsFirst", false);
        //m_closeFrame = new CloseButton(this);//zyf 自绘tab选中的框
        //m_closeFrame->move(657, 9);

        if (m_useVideo) {
#ifndef DISABLE_VIDEO
            m_current = new VideoWidget(false, m_fakerWidget);
            m_nextBtn->setMode(NextButton::Transparent);
#endif
        } else {
            m_current = new PhotoSlide(m_fakerWidget);
            m_nextBtn->setMode(NextButton::Normal);
            static_cast<PhotoSlide *>(m_current)->start(false, false, 1000);
            m_index = 1;//表示欢迎的第1个界面
        }

        slotTheme();
        m_previousBtn->hide();
        m_nextBtn->show();

    } else {
        //日常模式界面
        m_current = new NormalModule(m_fakerWidget);
        DPalette pl = this->palette();
        pl.setColor(DPalette::Window, Qt::white);
        this->setPalette(pl);
    }

    if (m_useVideo) {
        m_current->move(-1, -1);//因为有白边框的问题
    }

    m_current->show();
    slotTheme();
}

void MainWindow::initConnect()
{
    //主题变化响应
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
            &MainWindow::slotTheme);

    if (m_isFirst) {
        connect(m_previousBtn, &DIconButton::clicked, this, &MainWindow::previous);
        connect(m_nextBtn, &NextButton::clicked, this, &MainWindow::next);
        connect(m_doneBtn, &NextButton::clicked, qApp, &QCoreApplication::quit);
        connect(m_currentAni, &QPropertyAnimation::finished, this, &MainWindow::animationHandle);
        //connect(m_closeFrame, &CloseButton::closeMainWindow, this, &MainWindow::close);
    } else {
        connect(static_cast<NormalModule*>(m_current), &NormalModule::closeMainWindow, this, &MainWindow::close);
    }
}

/*******************************************************************************
 1. @函数:    bindAnimation
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    绑定需要动画的界面
*******************************************************************************/
void MainWindow::bindAnimation()
{
    m_currentAni->setTargetObject(m_current);
    m_lastAni->setTargetObject(m_last);
}

/*******************************************************************************
 1. @函数:    updateModule
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    根据index值进行欢迎四个界面的创建,并进行动画绑定
*******************************************************************************/
void MainWindow::updateModule(const int index)
{
    if(m_nextBtn == nullptr || m_previousBtn == nullptr || m_doneBtn == nullptr)
        return;

    m_nextBtn->show();
    m_previousBtn->show();
    m_nextBtn->setMode(NextButton::Normal);
    m_doneBtn->hide();

    m_last = m_current;
    switch (index) {
        case 1:
            if (m_useVideo) {
#ifndef DISABLE_VIDEO
                m_current = new VideoWidget(false, m_fakerWidget);
#endif
                m_fakerWidget->setFixedSize(QSize(700, 450));
                m_fakerWidget->move(-1,-1);//因为有白边框的问题
                m_nextBtn->setMode(NextButton::Transparent);
            } else {
                m_current = new PhotoSlide(m_fakerWidget);
                static_cast<PhotoSlide *>(m_current)->start(false, false, 1000);
                m_nextBtn->setMode(NextButton::Normal);
            }

            m_previousBtn->hide();
            m_fakerWidget->setFocus();
            break;
        case 2:
            m_current = initDesktopModeModule();
            m_fakerWidget->setFocus();
            break;
        case 3: {
            if (m_supportWM) {
                m_current = initWMModeModule();
                m_fakerWidget->setFocus();

                if (!m_nextBtn->isVisible()) {
                    ++m_index;
                }
                break;
            } else {
                ++m_index;
            }
        }
        case 4:
            m_current = initIconModule();
            m_fakerWidget->setFocus();
            m_nextBtn->hide();
            m_doneBtn->show();
            break;
        default:
            break;
    }

    m_current->show();

    bindAnimation();
}

/*******************************************************************************
 1. @函数:    animationHandle
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    m_currentAni动画结束后删除m_last保存的界面
*******************************************************************************/
void MainWindow::animationHandle()
{
    if (m_last) {
        m_last->deleteLater();
        m_last = nullptr;
    }
}

/*******************************************************************************
 1. @函数:    initDesktopModeModule
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    初始化第一次启动桌面模式模​​块
*******************************************************************************/
BaseModuleWidget *MainWindow::initDesktopModeModule()
{
    DesktopModeModule *module = new DesktopModeModule;
    module->updateBigIcon();
    BaseModuleWidget *w = new BaseModuleWidget(module, m_fakerWidget);
    w->setTitle(tr("Choose a desktop mode"));
    w->setDescribe(tr("You can switch modes by right clicking on the dock"));
    w->setFixedSize(WINDOW_SIZE);
    return w;
}

BaseModuleWidget *MainWindow::initWMModeModule()
{
    WMModeModule *module = new WMModeModule;
    module->updateBigIcon();
    BaseModuleWidget *w = new BaseModuleWidget(module, m_fakerWidget);
    w->setTitle(tr("Choose a running mode"));
    w->setDescribe(tr("You can switch it in Control Center > Personalization > Window effect"));
    w->setFixedSize(WINDOW_SIZE);
    return w;
}

BaseModuleWidget *MainWindow::initIconModule()
{
    IconModule *module = new IconModule;
    module->updateBigIcon();
    BaseModuleWidget *w = new BaseModuleWidget(module, m_fakerWidget);
    w->setContentWidgetHeight(FIRST_ICONCONTENT_WINSIZE);
    w->setTitle(tr("Choose an icon theme"));
    w->setDescribe(tr("Change it in Control Center > Personalization > Icon Theme"));
    w->setFixedSize(WINDOW_SIZE);
    return w;
}
