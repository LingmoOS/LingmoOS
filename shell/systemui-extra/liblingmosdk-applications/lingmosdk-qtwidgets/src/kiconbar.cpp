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

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include "kiconbar.h"
#include "themeController.h"
#include "parmscontroller.h"
#include <QDebug>

namespace kdk
{
class KIconBarPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KIconBar)

public:
    KIconBarPrivate(KIconBar*parent);
    void elideWidgetName();

protected:
    void changeTheme();
    void changeIconStyle();
    bool eventFilter(QObject *watched, QEvent *event);

private:
    KIconBar* q_ptr;
    QLabel *m_pIconLabel;
    QLabel *m_pNameLabel;
    QHBoxLayout *m_pMainLayout;
    QString m_IconName;
    QString m_widgetName;
    QColor inactiveColor;
};

KIconBar::KIconBar(QWidget*parent)
    :QFrame(parent),
      d_ptr(new KIconBarPrivate(this))
{
    Q_D(KIconBar);
    setObjectName("IconBar");
    setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
    connect(Parmscontroller::self(),&Parmscontroller::modeChanged,this,[=](){
        setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
        d->m_pIconLabel->setPixmap(QIcon::fromTheme(d->m_IconName).pixmap(
                            QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize),
                                  Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize))));
        updateGeometry();
    });
}

KIconBar::KIconBar(const QString &iconName, const QString &text, QWidget *parent)
    :KIconBar(parent)
{
    setIcon(iconName);
    setWidgetName(text);
}

KIconBar::~KIconBar()
{

}

void KIconBar::setIcon(const QString &iconName)
{
    Q_D(KIconBar);

    if(iconName.isEmpty())
        return;
    if(!d->m_pIconLabel)
        return;
    d->m_IconName = iconName;
    d->m_pIconLabel->setPixmap(QIcon::fromTheme(iconName).pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize),Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize))));
    setWindowIcon(QIcon::fromTheme(iconName).pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize),Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize))));
}

void KIconBar::setIcon(const QIcon &icon)
{
    Q_D(KIconBar);

    if(!d->m_pIconLabel)
        return;
    d->m_pIconLabel->setPixmap(icon.pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize),Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize))));
    d->m_IconName = icon.name();
    setWindowIcon(icon.pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize),Parmscontroller::parm(Parmscontroller::Parm::PM_IconBarIconSize))));
}

void KIconBar::setWidgetName(const QString &widgetName)
{
    Q_D(KIconBar);

    if(widgetName.isEmpty())
        return;
    if(!d->m_pNameLabel)
        return;
    d->m_widgetName = widgetName;
    d->elideWidgetName();
}

QSize KIconBar::sizeHint() const
{
    auto size = QFrame::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}
QLabel *KIconBar::nameLabel()
{
    Q_D(KIconBar);
    return d->m_pNameLabel;
}

QLabel *KIconBar::iconLabel()
{
    Q_D(KIconBar);
    return d->m_pIconLabel;
}

void KIconBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(KIconBar);
    if(event->button() == Qt::LeftButton)
        Q_EMIT doubleClick();
}

void KIconBar::resizeEvent(QResizeEvent *event)
{
    Q_D(KIconBar);
    QFrame::resizeEvent(event);
    d->elideWidgetName();
}

KIconBarPrivate::KIconBarPrivate(KIconBar *parent)
    :q_ptr(parent)
{
    Q_Q(KIconBar);
    setParent(parent);
    parent->installEventFilter(this);
    q->setContentsMargins(0,0,0,0);
    m_pMainLayout = new QHBoxLayout(q);
    m_pIconLabel = new QLabel(q);
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(24,24);
    m_pNameLabel = new QLabel(q);

    QHBoxLayout* iconLayout=new QHBoxLayout();
    iconLayout->setContentsMargins(0,8,0,8);
    iconLayout->addWidget(m_pIconLabel);

    m_pMainLayout->setSpacing(0);
    m_pMainLayout->addSpacing(8);
    m_pMainLayout->addLayout(iconLayout);
    m_pMainLayout->addSpacing(4);
    m_pMainLayout->addWidget(m_pNameLabel);
    m_pMainLayout->setContentsMargins(0,0,0,0);
    m_pMainLayout->addStretch();

    changeIconStyle();
    connect(m_gsetting,&QGSettings::changed,this,&KIconBarPrivate::changeIconStyle);

    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,&KIconBarPrivate::changeTheme);

    connect(m_gsetting,&QGSettings::changed,this,[=](const QString &key){
        if(key.contains("systemFontSize"))
            elideWidgetName();
    });
}

void KIconBarPrivate::elideWidgetName()
{
    Q_Q(KIconBar);

    QFontMetrics fm = QApplication::fontMetrics();
    // bug 175664 修正1px
    auto visualWidth = q->width()-m_pNameLabel->geometry().left()-1;
    QString elidedText = fm.elidedText(m_widgetName,Qt::TextElideMode::ElideRight, visualWidth);
    m_pNameLabel->setText(elidedText);
    if(fm.width(m_widgetName) >= visualWidth)
        m_pNameLabel->setToolTip(m_widgetName);
    else
        m_pNameLabel->setToolTip("");
}

void KIconBarPrivate::changeTheme()
{
    Q_Q(KIconBar);
    initThemeStyle();
    if(ThemeController::themeMode() == DarkTheme)
        inactiveColor = QColor(255,255,255);
    else
        inactiveColor = QColor(0,0,0);
    inactiveColor.setAlphaF(0.3);
    QPalette palette = q->palette();
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactiveColor);
    m_pNameLabel->setPalette(palette);
}

void KIconBarPrivate::changeIconStyle()
{
    Q_Q(KIconBar);
    initThemeStyle();
    q->setIcon(m_IconName);
}

bool KIconBarPrivate::eventFilter(QObject *watched, QEvent *event)
{
    Q_Q(KIconBar);
    //窗口停用，变为不活动的窗口
    if(QEvent::WindowDeactivate == event->type())
    {
        inactiveColor.setAlphaF(0.3);
        QPalette palette = q->palette();
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactiveColor);
        m_pNameLabel->setPalette(palette);
    }
    return QObject::eventFilter(watched,event);
}

}
#include "kiconbar.moc"
#include "moc_kiconbar.cpp"

