/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kwidget.h"
#include "gui_g.h"
#include "xatom-helper.h"
#include <QBoxLayout>
#include <QPalette>
#include <QPainter>
#include <QApplication>
#include "parmscontroller.h"
#include "KWindowEffects"
#include <QGuiApplication>
#include "lingmostylehelper/lingmo-decoration-manager.h"
#include "lingmostylehelper/lingmo-shell-manager.h"

namespace kdk
{
#define  FocusIn QEvent::FocusIn
#define  FocusOut QEvent::FocusOut
class KWidgetPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KWidget)

public:
    KWidgetPrivate(KWidget*parent);

private:
    KWidget *q_ptr;
    KIconBar* m_pIconBar;
    KWindowButtonBar* m_pWindowButtonBar;
    QHBoxLayout* m_pTitleLayout;
    QHBoxLayout* m_pWidgetLayout;
    QVBoxLayout* m_pMainLayout;
    QWidget* m_pBaseWidget;
    QWidget* m_pSideWidget;
    QString m_IconName;
    QGSettings* m_pTransparency;
    double tranSparency;
    LayoutType m_layoutType;
    bool m_sidebarFollowMode;

    void adjustBackground();
    void adjustFlagsTitleStatus(Qt::WindowFlags type);
    void adjustFlagTitleStatus(Qt::WindowFlags type);
};

KWidget::KWidget(QWidget *parent)
    :QWidget(parent),
      d_ptr(new KWidgetPrivate(this))
{
    Q_D(KWidget);
    setFocusPolicy(Qt::ClickFocus);
    setObjectName("Kwidget");
//    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
    // bug 163681 弃用主题毛玻璃，改用kwindowsystem毛玻璃接口
    this->setProperty("useSystemStyleBlur", true);
    /* 开启背景模糊效果（毛玻璃） */
//    KWindowEffects::enableBlurBehind(this->winId());

    if(Parmscontroller::isTabletMode())
    {
        d->m_pWindowButtonBar->maximumButton()->hide();
    }
    else
    {
        d->m_pWindowButtonBar->maximumButton()->show();
    }
    connect(d->m_pWindowButtonBar->minimumButton(),&QPushButton::clicked,this,&KWidget::showMinimized);
    connect(d->m_pWindowButtonBar->maximumButton(),&QPushButton::clicked,this,[=]()
        {
            if(this->isMaximized())
                showNormal();
            else
                showMaximized();
        });
    connect(d->m_pWindowButtonBar->closeButton(),&QPushButton::clicked,this,&KWidget::close);
    connect(d->m_pWindowButtonBar,&KWindowButtonBar::doubleClick,this,[=]()
        {
            if(this->isMaximized())
                showNormal();
            else
                showMaximized();
        });
    connect(d->m_pIconBar,&KIconBar::doubleClick,this,[=]()
        {
            if(this->isMaximized())
                showNormal();
            else
                showMaximized();
        });
    changeIconStyle();
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeIconStyle();});

    if (QGSettings::isSchemaInstalled("org.lingmo.control-center.personalise")) {
        d->m_pTransparency = new QGSettings("org.lingmo.control-center.personalise");
        d->tranSparency = d->m_pTransparency->get("transparency").toDouble();
        connect(d->m_pTransparency, &QGSettings::changed, this, [=](QString value) {
            if (value == "transparency" && ThemeController::themeMode() != ClassicTheme ) {
                d->tranSparency = d->m_pTransparency->get("transparency").toDouble();
                d->adjustBackground();
            }
        });
    }
    d->adjustBackground();
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
        if(d->m_sidebarFollowMode)
        {
            d->m_pSideWidget->setFixedWidth(Parmscontroller::parm(Parmscontroller::Parm::PM_Widget_SideWidget_Width));
            d->m_pIconBar->setFixedWidth(Parmscontroller::parm(Parmscontroller::Parm::PM_Widget_SideWidget_Width));
        }
        if(Parmscontroller::isTabletMode())
        {
            if(d->m_pWindowButtonBar->followMode())
                d->m_pWindowButtonBar->maximumButton()->hide();
        }
        else
        {
            if(d->m_pWindowButtonBar->followMode())
                d->m_pWindowButtonBar->maximumButton()->show();
        }
        updateGeometry();
    });
    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeTheme();});

    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("xcb"),Qt::CaseInsensitive))
    {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
    
    installEventFilter(this);
    resize(800,600);
}

KWidget::~KWidget()
{

}

void KWidget::setIcon(const QIcon &icon)
{
    Q_D(KWidget);
    d->m_IconName = icon.name();
    d->m_pIconBar->setIcon(icon);
//    setWindowIcon(icon.pixmap(QSize(36,36)));
    setWindowIcon(icon);
}

void KWidget::setIcon(const QString &iconName)
{
    Q_D(KWidget);
    d->m_IconName = iconName;
    d->m_pIconBar->setIcon(iconName);
//    setWindowIcon(QIcon::fromTheme(iconName).pixmap(QSize(36,36)));
    setWindowIcon(QIcon::fromTheme(iconName));
}

void KWidget::setWidgetName(const QString &widgetName)
{
    Q_D(KWidget);
    d->m_pIconBar->setWidgetName(widgetName);
    QWidget::setWindowTitle(widgetName);
}

QWidget *KWidget::sideBar()
{
    Q_D(KWidget);
    return d->m_pSideWidget;
}

QWidget *KWidget::baseBar()
{
    Q_D(KWidget);
    return d->m_pBaseWidget;
}

KWindowButtonBar *KWidget::windowButtonBar()
{
    Q_D(KWidget);
    return d->m_pWindowButtonBar;
}

KIconBar *KWidget::iconBar()
{
    Q_D(KWidget);
    return d->m_pIconBar;
}

void KWidget::setLayoutType(LayoutType type)
{
    Q_D(KWidget);
    d->m_layoutType = type;
    d->adjustBackground();
}

void KWidget::setWindowFlags(Qt::WindowFlags type)
{
    Q_D(KWidget);
    QWidget::setWindowFlags(type);
    d->adjustFlagsTitleStatus(type);
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("xcb"),Qt::CaseInsensitive))
    {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }

}

void KWidget::setWindowFlag(Qt::WindowType flag, bool on)
{
    Q_D(KWidget);
    QWidget::setWindowFlag(flag,on);
    d->adjustFlagTitleStatus(flag);
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("xcb"),Qt::CaseInsensitive))
    {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
}
void KWidget::setSidebarFollowMode(bool flag)
{
    Q_D(KWidget);
    d->m_sidebarFollowMode=flag;
}

bool KWidget::sidebarFollowMode()
{
    Q_D(KWidget);
    return d->m_sidebarFollowMode;
}
bool KWidget::eventFilter(QObject *target, QEvent *event)
{
    Q_D(KWidget);
    if(target == this && (event->type() == QEvent::WindowStateChange|| event->type() == QEvent::Show))
    {
        if(this->isMaximized())
            d->m_pWindowButtonBar->setMaximumButtonState(MaximumButtonState::Restore);
        else
            d->m_pWindowButtonBar->setMaximumButtonState(MaximumButtonState::Maximum);
    }
    if(target == this && (event->type() == FocusIn|| event->type() == FocusOut) || event->type() == QEvent::ActivationChange)
        d->adjustBackground();
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        if((event->type() == QEvent::PlatformSurface || event->type() == QEvent::Show || event->type() == QEvent::UpdateRequest))
        {
            #ifdef USE_LINGMO_PROTOCOL
            if(LingmoUIShellManager::getInstance()->lingmoshellReady())
                LingmoUIShellManager::getInstance()->removeHeaderBar(this->windowHandle());
            else
                LINGMODecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
            #else
                LINGMODecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
            #endif
        }
    }
    if(target == this && (event->type() == QEvent::Close || event->type() == QEvent::Hide))
       {
           d->m_pWindowButtonBar->closeButton()->setAttribute(Qt::WA_UnderMouse, false);
           QHoverEvent hoverEvent(QEvent::HoverLeave, QPoint(10, 10), QPoint(0, 0));
           QCoreApplication::sendEvent(d->m_pWindowButtonBar->closeButton(), &hoverEvent);
       }
    return QWidget::eventFilter(target, event);
}

void KWidget::changeIconStyle()
{
    Q_D(KWidget);
    initThemeStyle();
    setIcon(d->m_IconName);
}

void KWidget::changeTheme()
{
    Q_D(KWidget);
    if(ThemeController::widgetTheme() != ClassicTheme)
    {
        this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
        KWindowEffects::enableBlurBehind(this->winId());
    }
    initThemeStyle();
    d->adjustBackground();
}

KWidgetPrivate::KWidgetPrivate(KWidget *parent)
    :q_ptr(parent),
    m_sidebarFollowMode(true)
{
    Q_Q(KWidget);
    m_pMainLayout = new QVBoxLayout();
    m_pTitleLayout = new QHBoxLayout();
    m_layoutType = VerticalType;

    //图标和标题名称
    m_pIconBar = new KIconBar(q);
    m_pIconBar->setObjectName("IconBar");
    m_pIconBar->setFixedWidth(Parmscontroller::parm(Parmscontroller::Parm::PM_Widget_SideWidget_Width));
    m_pIconBar->setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
    //窗口三联
    m_pWindowButtonBar = new KWindowButtonBar(q);
    m_pWindowButtonBar->setObjectName("TitleBar");
    m_pWindowButtonBar->setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));

    m_pTitleLayout->addWidget(m_pIconBar);
    m_pTitleLayout->addWidget(m_pWindowButtonBar);
    m_pTitleLayout->setSpacing(0);
    m_pTitleLayout->setContentsMargins(0,0,0,0);

    m_pWidgetLayout = new QHBoxLayout();
    m_pBaseWidget = new QWidget(q);
    m_pBaseWidget->setObjectName("BaseWidget");
    m_pSideWidget = new QWidget(q);
    m_pSideWidget->setObjectName("SideWidget");
    m_pSideWidget->setFixedWidth(Parmscontroller::parm(Parmscontroller::Parm::PM_Widget_SideWidget_Width));
    m_pWidgetLayout->addWidget(m_pSideWidget);
    m_pWidgetLayout->addWidget(m_pBaseWidget);

    m_pMainLayout->addLayout(m_pTitleLayout);
    m_pMainLayout->addLayout(m_pWidgetLayout);

    m_pMainLayout->setSpacing(0);
    m_pMainLayout->setContentsMargins(0,0,0,0);
    q->setLayout(m_pMainLayout);
    m_pBaseWidget->setAutoFillBackground(true);
    m_pBaseWidget->setBackgroundRole(QPalette::Base);
    m_pWindowButtonBar->setAutoFillBackground(true);
    m_pWindowButtonBar->setBackgroundRole(QPalette::Base);
    setParent(parent);
}

void KWidgetPrivate::adjustBackground()
{
    Q_Q(KWidget);

    m_pIconBar->setAutoFillBackground(true);
    m_pSideWidget->setAutoFillBackground(true);
    QColor sideColor = q->palette().color(QPalette::Window);
    sideColor.setAlphaF(tranSparency);
    QPalette sidePalette = q->palette();
    sidePalette.setColor(QPalette::Window,sideColor);
    m_pWindowButtonBar->setPalette(q->palette());
    m_pIconBar->setPalette(q->palette());
    switch (m_layoutType)
    {
    case VerticalType:
        m_pIconBar->setBackgroundRole(QPalette::Base);
        m_pSideWidget->hide();
        break;
    case HorizontalType:
        m_pSideWidget->setPalette(sidePalette);
        m_pIconBar->setBackgroundRole(QPalette::Window);
        m_pIconBar->setPalette(sidePalette);
        m_pSideWidget->show();
        break;
    case MixedType:
        m_pIconBar->setBackgroundRole(QPalette::Base);
        m_pSideWidget->setPalette(sidePalette);
        m_pSideWidget->show();
        break;
    default:
        break;
    }

}

void KWidgetPrivate::adjustFlagsTitleStatus(Qt::WindowFlags type)
{
    Q_Q(KWidget);
    m_pWindowButtonBar->menuButton()->hide();
    switch (type)
    {
    case Qt::Drawer:
        m_pWindowButtonBar->minimumButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Tool:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::ToolTip:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::SplashScreen:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Dialog:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Sheet:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Popup:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Desktop:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
//        m_pWindowButtonBar->menuButton()->hide();
        q->deleteLater();
        break;
    case Qt::ForeignWindow:
//        m_pWindowButtonBar->menuButton()->hide();
//        break;
    case Qt::CoverWindow:
//        m_pWindowButtonBar->menuButton()->hide();
//        break;
    case Qt::Window:
//        m_pWindowButtonBar->menuButton()->hide();
//        break;
    case Qt::Widget:
//        m_pWindowButtonBar->menuButton()->hide();
//        break;
    case Qt::SubWindow:
//        m_pWindowButtonBar->menuButton()->hide();
        break;
    default:
        break;
    }
}

void KWidgetPrivate::adjustFlagTitleStatus(Qt::WindowFlags type)
{
    Q_Q(KWidget);
    switch (type)
    {
    case Qt::Drawer:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Tool:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::ToolTip:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::SplashScreen:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Dialog:
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Sheet:
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Popup:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Desktop:
        m_pWindowButtonBar->minimumButton()->hide();
        m_pWindowButtonBar->maximumButton()->hide();
        m_pWindowButtonBar->closeButton()->hide();
        m_pWindowButtonBar->menuButton()->hide();
        q->deleteLater();
        break;
    case Qt::ForeignWindow:
        q->setWindowFlags(Qt::WindowMinMaxButtonsHint);      //为窗口添加最大化和最小化按钮
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::CoverWindow:
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Window:
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::Widget:
        m_pWindowButtonBar->menuButton()->hide();
        break;
    case Qt::SubWindow:
        m_pWindowButtonBar->menuButton()->hide();
        break;
    default:
        break;
    }
}
}

#include "kwidget.moc"
#include "moc_kwidget.cpp"
