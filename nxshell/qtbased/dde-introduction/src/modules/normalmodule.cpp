// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "normalmodule.h"

//Dtk
#include <DApplication>

NormalModule::NormalModule(DWidget *parent)
    : DWidget(parent)
    , m_leftNavigationLayout(new QVBoxLayout)
    , m_rightContentLayout(new QVBoxLayout)
    , m_buttonGrp(new QButtonGroup)
    , m_currentWidget(nullptr)
    , m_wmSwitcher(new WMSwitcher("com.deepin.WMSwitcher", "/com/deepin/WMSwitcher",
                                  QDBusConnection::sessionBus(), this))
    , m_titleLabel(new DLabel(this))
    , m_describe(new DLabel(this))
    , m_index(-1)//这个值初始化
{
    // initTheme(0);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 13, 20, 0);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
            &NormalModule::updateLabel);

    // m_leftNavigationLayout->setMargin(0);
    m_leftNavigationLayout->setSpacing(10);
    m_leftNavigationLayout->setContentsMargins(0, 50, 0, 0);

    m_rightContentLayout->setSpacing(0);
    m_rightContentLayout->setMargin(0);
    m_rightContentLayout->setContentsMargins(0, 0, 0, 0);

    /*DLabel *logo = new DLabel(this);
    //    QIcon::setThemeName("hicolor");
    //    QPixmap pixmap = std::move(QIcon::fromTheme("dde-introduction",
    QIcon(":/resources/dde-introduction.svg")).pixmap(QSize(24, 24) * devicePixelRatioF())); QPixmap
    pixmap = QIcon::fromTheme("dde-introduction").pixmap(QSize(24, 24) * devicePixelRatioF());
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    logo->setPixmap(pixmap);
    logo->move(rect().topLeft() + QPoint(12, 8));
    logo->show();*/

    m_content = new DWidget;
    m_content->setLayout(m_rightContentLayout);

    //左侧导航按钮界面，使用临时变量
    DWidget *leftWidget = new DWidget;
    leftWidget->setObjectName("LeftWidget");
    leftWidget->setLayout(m_leftNavigationLayout);

    layout->addWidget(leftWidget);
    layout->addWidget(m_content);

    // bottom navigation
    // BottomNavigation *bottomNavigation = new BottomNavigation;

    //设置标题的字体属性
    QFont font;
    font.setStyleName("Bold");
    m_titleLabel->setFont(font);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T5);

    //设置描述信息的字体属性
    QFont deFont;
    deFont.setStyleName("Normal");
    m_describe->setFont(deFont);
    DFontSizeManager::instance()->bind(m_describe, DFontSizeManager::T8);
    m_describe->setElideMode(Qt::TextElideMode::ElideRight);

    updateLabel();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    mainLayout->addLayout(layout);
    mainLayout->addWidget(m_describe, 0, Qt::AlignCenter);
    // mainLayout->addWidget(bottomNavigation);
    mainLayout->setContentsMargins(0, 0, 0, 20);

    setLayout(mainLayout);

    setFixedSize(700, 450);
    m_content->setFixedSize(549, 343);

    m_closeFrame = new CloseButton(this);//zyf 关闭按钮的tab选中框
    m_closeFrame->move(rect().topRight() - QPoint(m_closeFrame->width(), -1));
    m_closeFrame->setFocusPolicy(Qt::NoFocus);
    m_closeFrame->beFocused = false;

    tab_be_press = false;//需求，是一开始运行不按tab导航按钮是没有边框的，按了tab之后导航按钮会有边框

    setFocus();

    int moduleCount = 0;
    //窗口特效是否允许切换，从DBUS获取
    bool allow_switch_wm = m_wmSwitcher->AllowSwitch();

    //UosEnterprise:服务器企业版　　UosEnterpriseC:行业版   UosEuler:服务器欧拉版
    bool bIsServerSystemType;
    if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterprise  ||
        DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterpriseC ||
        DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEuler) {
        bIsServerSystemType = true;
    } else {
        bIsServerSystemType = false;
    }

    //deepin类型，桌面，个人等
    const DSysInfo::DeepinType DeepinType = DSysInfo::deepinType();
    bool bIsDeepinServerType = (DeepinType == DSysInfo::DeepinServer);

    //使用DBUS获取窗口特效是否支持切换 这个和上面的m_wmSwitcher->AllowSwitch();
    bool isSuportEffect = QDBusInterface("com.deepin.wm", "/com/deepin/wm", "com.deepin.wm")
                                        .property("compositingAllowSwitch")
                                        .toBool();

    (bIsServerSystemType  || bIsDeepinServerType || !isSuportEffect || Worker::isWaylandType()) ? m_supportWM = false : m_supportWM = true;

#ifndef DISABLE_VIDEO
        NavigationButton *videoBtn = new NavigationButton(tr("Introduction"));
        videoBtn->setToolTip(tr("Introduction"));
        m_buttonMap[videoBtn] = ++moduleCount;
        m_titleMap[videoBtn] = tr("Welcome");
        m_buttonGrp->addButton(videoBtn);
        VideoWidget *videoModule = new VideoWidget(false, this);
        m_modules[moduleCount] = videoModule;
        connect(videoModule, &VideoWidget::cancelCloseFrame, this, &NormalModule::cancelCloseFrame);
        m_button = videoBtn;
        m_button->setChecked(true);
        m_titleLabel->setText(m_titleMap[videoBtn]);
        //根据index值时时刷新导航按钮对应的界面，初始化函数中只是更新动画
        updateCurrentWidget(m_buttonMap[videoBtn]);
#else
    NavigationButton *slideBtn = new NavigationButton(tr("Introduction"), this);
    slideBtn->setToolTip(tr("Introduction"));
    m_buttonMap[slideBtn] = ++moduleCount;
    m_titleMap[slideBtn] = tr("Welcome");
    m_buttonGrp->addButton(slideBtn);
    PhotoSlide *slideModule = new PhotoSlide;
    slideModule->hide();
    slideModule->start(false, false, 2000);
    m_modules[moduleCount] = slideModule;
    m_button = slideBtn;
    slideBtn->setChecked(true);
    m_titleLabel->setText(m_titleMap[slideBtn]);
    //根据index值时时刷新导航按钮对应的界面，初始化函数中只是更新轮播图
    updateCurrentWidget(m_buttonMap[slideBtn]);
#endif 

    //桌面样式导航按钮和内容时尚高效模式创建
    // desktop button
    NavigationButton *desktopBtn = new NavigationButton(tr("Desktop Mode"));
    desktopBtn->setToolTip(tr("Desktop Mode"));
    m_buttonMap[desktopBtn] = ++moduleCount;
    m_titleMap[desktopBtn] = tr("Choose a desktop mode");
    m_describeMap[desktopBtn] = tr("You can switch modes by right clicking on the dock");
    m_buttonGrp->addButton(desktopBtn);
    DesktopModeModule *desktopModeModule = new DesktopModeModule(this);
    desktopModeModule->setFirst(false);
    desktopModeModule->hide();
    m_modules[moduleCount] = desktopModeModule;

    //运行模式导航按钮和内容特效模式界面创建
    // wm button
    if (m_supportWM) {
        NavigationButton *wmBtn = nullptr;
        if (allow_switch_wm) {
            wmBtn = new NavigationButton(tr("Running Mode"));
            wmBtn->setToolTip(tr("Running Mode"));
            m_buttonMap[wmBtn] = ++moduleCount;
            connect(wmBtn, &NavigationButton::widthChanged, this, &NormalModule::updateInterface);
            m_titleMap[wmBtn] = tr("Choose a running mode");
            m_describeMap[wmBtn] =
                tr("Please choose normal mode if you has a low configuration computer");
            m_buttonGrp->addButton(wmBtn);
            WMModeModule *wmModeModule = new WMModeModule(this);
            wmModeModule->setFirst(false);
            wmModeModule->hide();
            m_modules[moduleCount] = wmModeModule;
        }
    }

    //图标主题导航按钮和内容图标选择界面创建
    // icon button
    NavigationButton *iconBtn = new NavigationButton(tr("Icon Theme"));
    iconBtn->setFixedSize(200, 200);
    iconBtn->setToolTip(tr("Icon Theme"));
    m_buttonMap[iconBtn] = ++moduleCount;
    // iconBtn->setText(tr("Icon theme"));
    m_titleMap[iconBtn] = tr("Choose an icon theme");
    m_describeMap[iconBtn] = tr("Change it in Control Center > Personalization > Icon Theme");
    m_buttonGrp->addButton(iconBtn);
    IconModule *iconModule = new IconModule(this);
    iconModule->hide();
    m_modules[moduleCount] = iconModule;
    connect(iconModule, &IconModule::cancelCloseFrame, this, &NormalModule::cancelCloseFrame);

    // support us
    /*NavigationButton *supportBtn = new NavigationButton(tr("Support us"));
    m_buttonMap[supportBtn]   = ++moduleCount;
    //supportBtn->setText(tr("Support us"));
    m_titleMap[supportBtn] = tr("Support us");
    m_buttonGrp->addButton(supportBtn);
    Support *support = new Support(this);
    support->hide();
    m_modules[moduleCount] = support;*/

    // about button
    /*NavigationButton *aboutBtn = new NavigationButton(tr("About us"));
    m_buttonMap[aboutBtn] = ++moduleCount;
    aboutBtn->setText(tr("About us"));
    m_titleMap[aboutBtn] = tr("About us");
    m_buttonGrp->addButton(aboutBtn);
    About *about = new About(this);
    about->hide();
    m_modules[moduleCount] = about;*/

    m_buttonGrp->setExclusive(true);

    // m_leftNavigationLayout->addStretch();

    //单独写个for将m_buttonGrp放到左侧导航布局中
    for (QWidget *w : m_buttonGrp->buttons()) {
        // w->setFixedSize(110, 30);
        w->setMinimumSize(110, 10);
        m_leftNavigationLayout->addWidget(w, 0, Qt::AlignHCenter | Qt::AlignVCenter);
        w->installEventFilter(this);
    }
    m_leftNavigationLayout->addStretch();

    setFocus();

    // m_leftNavigationLayout->addStretch();

    //根据点击m_buttonGrp中的按钮，刷新缓存的按钮，和对应的内容界面信息
    connect(m_buttonGrp,
            static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
            this, &NormalModule::ClickShow);
}

//鼠标点击事件
void NormalModule::mousePressEvent(QMouseEvent *event) {
    cancelCloseFrame();
    DWidget::mousePressEvent(event);
}

//鼠标点击窗口任何地方，都取消选中框，需求
void NormalModule::cancelCloseFrame() {
    m_closeFrame->beFocused = false;
    QAbstractButton *btn = m_buttonGrp->button(-m_index - 1);//m_buttonGrp中的id是负数值，所以用-m_index - 1，m_index是1,2,3,4
    static_cast<NavigationButton*>(btn)->needFrame = false;
    update();
}

void NormalModule::ClickShow(QAbstractButton *btn) {
    QAbstractButton *button = m_buttonGrp->button(-m_index - 1);//m_buttonGrp中的id是负数值，所以用-m_index - 1，m_index是1,2,3,4
    static_cast<NavigationButton*>(button)->needFrame = false;
    updataButton(btn);
    updateCurrentWidget(m_buttonMap[btn]);
    m_titleLabel->setText(m_titleMap[btn]);
    //m_describe->setText(m_describeMap[btn]);
    m_describe->setText(m_describe->fontMetrics().elidedText(m_describeMap[btn], Qt::ElideRight, this->width()));
    m_closeFrame->beFocused = false;
    m_closeFrame->update();
}

//键盘按键事件
void NormalModule::keyPressEvent(QKeyEvent *event)
{
    QWidget *w = m_modules[m_index];

    qint64 tempTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    //什么作用??? 50ms内点击没有作用，50ms之后再点击，然后重复绘制???
    if ((abs(tempTime - m_keypressTime) < 50)) {
        m_keypressTime = tempTime;
        return;
    }
    m_keypressTime = tempTime;

    if(event->key() == Qt::Key_Up && !m_closeFrame->beFocused) {
        int index = m_index;
        if (index == 1) return;

        index = -index;//m_buttonGrp中的id是负数值，所以用-m_index - 1，m_index是1,2,3,4

        QAbstractButton *btn = m_buttonGrp->button(index - 1);
        static_cast<NavigationButton*>(btn)->needFrame = false;

        btn = m_buttonGrp->button(index);
        btn->setChecked(true);

        if (tab_be_press)
            static_cast<NavigationButton*>(btn)->needFrame = true;

        updataButton(btn);
        updateCurrentWidget(m_buttonMap[btn]);
        m_titleLabel->setText(m_titleMap[btn]);
        m_describe->setText(m_describeMap[btn]);

        return;
    }
    else if(event->key() == Qt::Key_Down && !m_closeFrame->beFocused) {
        int index = m_index;

        //如果有特效窗口，往下按到第四个窗口时，后面就没有窗口了，如果没有特效窗口，那么第三个窗口就是最后一个
        if (m_supportWM) {
            if (index == 4) return;
        }
        else {
            if (index == 3) return;
        }

        index = -index - 2;//m_buttonGrp中的id是负数值，所以用-m_index - 1，m_index是1,2,3,4

        QAbstractButton *btn = m_buttonGrp->button(index + 1);
        static_cast<NavigationButton*>(btn)->needFrame = false;

        btn = m_buttonGrp->button(index);
        btn->setChecked(true);

        if (tab_be_press)
            static_cast<NavigationButton*>(btn)->needFrame = true;

        updataButton(btn);
        updateCurrentWidget(m_buttonMap[btn]);

        m_titleLabel->setText(m_titleMap[btn]);
        m_describe->setText(m_describeMap[btn]);

        return;
    }
    else if(event->key() == Qt::Key_Tab) {//tab功能只是切换关闭按钮和导航按钮之间的选中框
        tab_be_press = true;
        m_closeFrame->beFocused = !m_closeFrame->beFocused;//绘制不绘制关闭按钮的选中框
        if (m_closeFrame->beFocused) {
            QAbstractButton *btn = m_buttonGrp->button(-m_index - 1);
            static_cast<NavigationButton*>(btn)->needFrame = false;
        }
        else {
            QAbstractButton *btn = m_buttonGrp->button(-m_index - 1);
            static_cast<NavigationButton*>(btn)->needFrame = true;
        }
        repaint();

        return;
    }
    else if(event->key() == Qt::Key_Return) {//只处理关闭按钮功能
        if (m_closeFrame->beFocused)//代码不规范
            emit closeMainWindow();
    }
    if (!m_closeFrame->beFocused) {
        switch (m_index) {
            case 1:
#ifndef DISABLE_VIDEO
                static_cast<VideoWidget *>(w)->keyPressEvent(event);
#else
                static_cast<PhotoSlide *>(w)->keyPressEvent(event);
#endif
                break;
            case 2:
                static_cast<DesktopModeModule *>(w)->keyPressEvent(event);
                break;
            case 3:
                static_cast<WMModeModule *>(w)->keyPressEvent(event);
                break;
            case 4:
                static_cast<IconModule *>(w)->keyPressEvent(event);
                break;
            default:
                break;
        }
    }
}

//这个有什么作用吗??? 其它类还调用eventFilter，返回值是做什么用的??? zyf也不知道什么作用
bool NormalModule::eventFilter(QObject *watched, QEvent *event)
{
    if (qobject_cast<NavigationButton *>(watched) && event->type() == QEvent::KeyPress) {
        return true;
    }
    return false;
}


/*******************************************************************************
 1. @函数:    updateCurrentWidget
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    根据index值时时刷新导航按钮对应的界面（备注：感觉可以优化，使用QStackedWidget）
*******************************************************************************/
void NormalModule::updateCurrentWidget(const int index)
{
    if (index == m_index)
        return;

    m_index = index;

    if (m_currentWidget) {
        m_rightContentLayout->removeWidget(m_currentWidget);
        m_currentWidget->hide();
    }
    QTimer::singleShot(50, this, SLOT(RefeshQwidget()));
}

void NormalModule::RefeshQwidget() {
    QWidget *w = m_modules[m_index];
    if(w == nullptr)
        return;
    ModuleInterface *module = qobject_cast<ModuleInterface *>(w);
    if (module) {
        //初始化日常启动界面的图片或这动画，直接调用子类的updateSmallIcon函数 //m_modules中存储的是各个类型的界面
        module->updateSmallIcon();
        module->updateSelectBtnPos();
    }
    if (m_index != 1) {
#ifndef DISABLE_VIDEO
        QWidget *pWidget = m_modules[1];
        VideoWidget *video = qobject_cast<VideoWidget *>(pWidget);
        if (video) {
            video->stop();
        }
        video->showVideoControlButton();
#endif
    }

    m_currentWidget = w;
    //该函数==调用paintEvent重绘界面
    repaint();

    m_rightContentLayout->addWidget(m_currentWidget, 0, Qt::AlignCenter);
    if (m_index == 4) {
        m_currentWidget->setFixedSize(549, 278);
    } else {
        m_currentWidget->setFixedSize(549, 309);
    }
    m_currentWidget->show();
}
/*******************************************************************************
 1. @函数:    updataButton
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    更新m_button缓存的button
*******************************************************************************/
void NormalModule::updataButton(QAbstractButton *btn)
{
    m_button->initButton();
    m_button = dynamic_cast<NavigationButton *>(btn);
}

//没用到，可以刪除
void NormalModule::initTheme(int type)
{
    if (type == 0) {
        type = DGuiApplicationHelper::instance()->themeType();
    }
    if (type == 2) {
        DPalette pa = this->palette();
        pa.setColor(DPalette::Window, QColor(40, 40, 40));
        this->setPalette(pa);
    }
}

/*******************************************************************************
 1. @函数:    updateLabel
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    更新描述信息的字体颜色
*******************************************************************************/
void NormalModule::updateLabel()
{
    DPalette dePa = m_describe->palette();
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == DGuiApplicationHelper::LightType)
        dePa.setColor(DPalette::WindowText, QColor("#FF8AA1B4"));
    else
        dePa.setColor(DPalette::WindowText, QColor("#FFC0C6D4"));
    m_describe->setPalette(dePa);
}

//改变所有窗口的大小
void NormalModule::updateInterface(int width)
{
    //如果要用这个函数后面可以整理一下代码
    if (width < 110) {
        for (QWidget *w : m_buttonGrp->buttons()) {
            w->setMinimumSize(110, 30);
            m_content->setFixedSize(549, 343);
        }
        QWidget *w = m_modules[m_index];
        switch (m_index) {
            case 1:
#ifndef DISABLE_VIDEO
                static_cast<VideoWidget *>(w)->updateInterface(QSize(549, 343));
#endif
                break;
            case 2:
                static_cast<DesktopModeModule *>(w)->updateInterface(1.0);
                break;
            //不知道为什么将特效注释掉
            /*case 3:
                static_cast<WMModeModule *>(w)->keyPressEvent(e);
                break;
            case 4:
                static_cast<IconModule *>(w)->keyPressEvent(e);
                break;*/
            default:
                break;
        };
        return;
    }
    QSize size(549, 343);
    int widgetWidth = size.width() - (width - 110);
    int widgetHeigh = size.height() * (widgetWidth / size.width());
    m_content->setFixedSize(widgetWidth, widgetHeigh);

    QWidget *w = m_modules[m_index];
    float f = float(110.0 / width);
    switch (m_index) {
        case 1:
#ifndef DISABLE_VIDEO
            static_cast<VideoWidget *>(w)->updateInterface(QSize(widgetWidth, widgetHeigh));
#endif
            break;
        case 2:
            static_cast<DesktopModeModule *>(w)->updateInterface(f);
            break;
        /*case 3:
            static_cast<WMModeModule *>(w)->keyPressEvent(e);
            break;
        case 4:
            static_cast<IconModule *>(w)->keyPressEvent(e);
            break;*/
        default:
            break;
    };

    for (QWidget *pQAbstractButton : m_buttonGrp->buttons()) {
        pQAbstractButton->setMinimumSize(width + 15, 30);
    }
}

NormalModule::~NormalModule()
{
    delete m_buttonGrp;
}
