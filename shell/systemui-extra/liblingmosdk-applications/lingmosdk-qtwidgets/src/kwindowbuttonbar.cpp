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

#include "kwindowbuttonbar.h"
#include "themeController.h"
#include <QPixmap>
#include <QMenu>
#include <QHBoxLayout>
#include <QVariant>
#include "parmscontroller.h"

namespace kdk
{
class KWindowButtonBarPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KWindowButtonBar)

public:
    KWindowButtonBarPrivate(KWindowButtonBar*parent);

protected:
    void changeTheme() override;

private:
    KWindowButtonBar* q_ptr;
    KMenuButton *m_pMenuBtn;
    QPushButton *m_pMinimumBtn;
    QPushButton *m_pmaximumBtn;
    QPushButton *m_pCloseBtn;
    MaximumButtonState m_maximumButtonState;
    QWidget*m_pParentWidget;
    QColor m_pixColor;
    bool m_followMode;
};

KWindowButtonBar::KWindowButtonBar(QWidget *parent)
    :QFrame(parent),
    d_ptr(new KWindowButtonBarPrivate(this))
{
    Q_D(KWindowButtonBar);
    d->m_pParentWidget = parent;
    d->m_pMenuBtn->setFocusPolicy(Qt::NoFocus);
    d->m_pMinimumBtn->setFocusPolicy(Qt::NoFocus);
    d->m_pmaximumBtn->setFocusPolicy(Qt::NoFocus);
    d->m_pCloseBtn->setFocusPolicy(Qt::NoFocus);
    setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
//    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    d->m_pCloseBtn->installEventFilter(this);
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
        setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
        d->m_pMenuBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pmaximumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pCloseBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pMinimumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        updateGeometry();
        repaint();
    });
}

KWindowButtonBar::~KWindowButtonBar()
{

}

QPushButton *KWindowButtonBar::minimumButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pMinimumBtn;
}

QPushButton *KWindowButtonBar::maximumButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pmaximumBtn;
}

QPushButton *KWindowButtonBar::closeButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pCloseBtn;
}

KMenuButton *KWindowButtonBar::menuButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pMenuBtn;
}

MaximumButtonState KWindowButtonBar::maximumButtonState()
{
    Q_D(KWindowButtonBar);
    return d->m_maximumButtonState;
}

void KWindowButtonBar::setMaximumButtonState(MaximumButtonState state)
{
    Q_D(KWindowButtonBar);
    d->m_maximumButtonState = state;
    d->changeTheme();
}

void KWindowButtonBar::setFollowMode(bool flag)
{
    Q_D(KWindowButtonBar);
    d->m_followMode=flag;
}

bool KWindowButtonBar::followMode()
{
    Q_D(KWindowButtonBar);
    return d->m_followMode;
}

void KWindowButtonBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(KWindowButtonBar);
    if(event->button() == Qt::LeftButton && d->m_pmaximumBtn->isEnabled())
        Q_EMIT doubleClick();
}

bool KWindowButtonBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KWindowButtonBar);
    if(QEvent::WindowDeactivate == event->type())
    {
        d->m_pixColor = QColor(179,179,179);
        auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
        auto minIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-minimize-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
        d->m_pCloseBtn->setIcon(closeIcon);
        d->m_pMinimumBtn->setIcon(minIcon);

        if(d->m_maximumButtonState == Maximum)
        {
            auto maxIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-maximize-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
            d->m_pmaximumBtn->setIcon(maxIcon);
            d->m_pmaximumBtn->setToolTip(tr("Maximize"));
        }
        else
        {
            auto maxIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-restore-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
            d->m_pmaximumBtn->setIcon(maxIcon);
            d->m_pmaximumBtn->setToolTip(tr("Restore"));
        }
    }
    if(QEvent::WindowActivate == event->type())
    {
        if (ThemeController::themeMode() == LightTheme)
            d->m_pixColor = QColor(31,32,34);
        else
            d->m_pixColor = QColor(255,255,255);
        auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
        d->m_pCloseBtn->setIcon(closeIcon);
        d->m_pMinimumBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

        if(d->m_maximumButtonState == Maximum)
        {
            d->m_pmaximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            d->m_pmaximumBtn->setToolTip(tr("Maximize"));
        }
        else
        {
            d->m_pmaximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
            d->m_pmaximumBtn->setToolTip(tr("Restore"));
        }
    }
    if(watched == d->m_pCloseBtn)
    {
        //根据不同状态重绘icon颜色
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if(isEnabled())
            {
                auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
                if(mouseEvent->button() == Qt::LeftButton)
                    d->m_pixColor = QColor(255,255,255);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::Enter:
            if(isEnabled())
            {
                d->m_pixColor = QColor(255,255,255);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::MouseButtonRelease:
            if(isEnabled())
            {
                auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
                if(mouseEvent->button() == Qt::LeftButton)
                    d->m_pixColor = QColor(255,255,255);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::Leave:
            if(isEnabled())
            {
                if(d->m_pCloseBtn->isActiveWindow())
                {
                if (ThemeController::themeMode() == LightTheme)
                    d->m_pixColor = QColor(31,32,34);
                else
                    d->m_pixColor = QColor(255,255,255);
                }
                else
                {
                    d->m_pixColor = QColor(179,179,179);
                }
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::WindowDeactivate:
            {
                d->m_pixColor = QColor(179,179,179);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        default:
            break;
        }
    }
    return QFrame::eventFilter(watched,event);
}

QSize KWindowButtonBar::sizeHint() const
{
    auto size = QFrame::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}

KWindowButtonBarPrivate::KWindowButtonBarPrivate(KWindowButtonBar *parent)
    :q_ptr(parent),m_followMode(true)
{
    Q_Q(KWindowButtonBar);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(8);
    hLayout->setContentsMargins(0,0,0,0);

    //m_maximumButtonState = Maximum;

    m_pMenuBtn = new KMenuButton(q);
    m_pMenuBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));

    m_pMinimumBtn = new QPushButton(q);
    m_pMinimumBtn->setToolTip(tr("Minimize"));
    m_pMinimumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));

    m_pmaximumBtn = new QPushButton(q);
    m_pmaximumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));

    m_pCloseBtn = new QPushButton(q);
    m_pCloseBtn->setObjectName("CloseButton");
    m_pCloseBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
    m_pCloseBtn->setToolTip(tr("Close"));

    hLayout->setContentsMargins(0,0,4,0);
    hLayout->setSpacing(4);
    hLayout->addStretch();
    hLayout->addWidget(m_pMenuBtn);
//    hLayout->addSpacing(4);
    hLayout->addWidget(m_pMinimumBtn);
//    hLayout->addSpacing(4);
    hLayout->addWidget(m_pmaximumBtn);
//    hLayout->addSpacing(4);
    hLayout->addWidget(m_pCloseBtn);
//    hLayout->addSpacing(4);
    q->setLayout(hLayout);

    //控件自己控制样式、响应主题变化
    m_pMinimumBtn->setProperty("isWindowButton", 0x1);
    m_pMinimumBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pMinimumBtn->setFlat(true);
    m_pMinimumBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

    m_pmaximumBtn->setProperty("isWindowButton", 0x1);
    m_pmaximumBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pmaximumBtn->setFlat(true);

    m_pCloseBtn->setProperty("isWindowButton", 0x02);
    m_pCloseBtn->setProperty("useIconHighlightEffect", 0x08);
    m_pCloseBtn->setFlat(true);
    m_pCloseBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeTheme();});
    connect(m_pmaximumBtn,&QPushButton::clicked,this,[=]()
        {
            if(m_maximumButtonState == Maximum)
                m_maximumButtonState =Restore;
            else
                m_maximumButtonState = Maximum;
            changeTheme();
        });
    setParent(parent);
}

void KWindowButtonBarPrivate::changeTheme()
{
    Q_Q(KWindowButtonBar);
    initThemeStyle();
    if(q->isActiveWindow())
    {
        if(q->isEnabled())
        {
            if (ThemeController::themeMode() == LightTheme)
                m_pixColor = QColor(31,32,34);
            else
                m_pixColor = QColor(255,255,255);
            auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize),Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))),m_pixColor);
            m_pCloseBtn->setIcon(closeIcon);
        }
        if(m_maximumButtonState == Maximum)
        {
                m_pmaximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
                m_pmaximumBtn->setToolTip(tr("Maximize"));
        }
        else
        {
                m_pmaximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
                m_pmaximumBtn->setToolTip(tr("Restore"));
        }
    }
}

}
#include "kwindowbuttonbar.moc"
#include "moc_kwindowbuttonbar.cpp"
