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

#include <QLabel>
#include <QBoxLayout>
#include <QColor>
#include <QPalette>
#include <QIcon>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QGraphicsDropShadowEffect>
#include "kballontip.h"
#include <QTimer>
#include <QHBoxLayout>
#include "themeController.h"
#include <kwindoweffects.h>
#include <QPainterPath>
#include <QDebug>
namespace kdk
{
class KBallonTipPrivate:public QObject,public ThemeController
{
    Q_DECLARE_PUBLIC(KBallonTip)
    Q_OBJECT

public:
    KBallonTipPrivate(KBallonTip*parent);
    void adjustStyle();

protected:
    void changeTheme();

private:
    KBallonTip* q_ptr;
    QLabel* m_pContentLabel;
    QLabel* m_pIconLabel;
    TipType m_type;
    QIcon m_icon;
    QColor m_color;
    QHBoxLayout* m_pLayout;
    QVBoxLayout* m_pVLayout;
    int m_time;
    QGraphicsDropShadowEffect *m_pShadow_effect;
};

KBallonTip::KBallonTip(QWidget *parent)
    : QWidget(parent),
      d_ptr(new KBallonTipPrivate(this))
{
    Q_D(KBallonTip);
    setWindowFlag(Qt::ToolTip);
    d->m_pVLayout = new QVBoxLayout(this);
    d->m_pLayout = new QHBoxLayout(this);
    d->m_pLayout->setSpacing(12);
    this->setContentsMargins(24,18,24,18);
    d->m_pContentLabel = new QLabel(this);
    d->m_pContentLabel->setWordWrap(true);
    //设置具体阴影
    d->m_pShadow_effect = new QGraphicsDropShadowEffect(this);
    d->changeTheme();
    d->m_pIconLabel = new QLabel(this);
    QVBoxLayout*vLayout = new QVBoxLayout;
    vLayout->addWidget(d->m_pIconLabel);
    vLayout->addStretch();
    d->m_pLayout->addLayout(vLayout);
    d->m_pLayout->addWidget(d->m_pContentLabel);
    d->m_pLayout->setAlignment(Qt::AlignVCenter);
    d->m_type = TipType::Nothing;
    d->m_pVLayout->addStretch();
    d->m_pVLayout->addLayout(d->m_pLayout);
    d->m_pVLayout->addStretch();
    d->adjustStyle();
    d->m_pVLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
    if(ThemeController::systemFontSize() > 11)
        vLayout->setContentsMargins(0,(ThemeController::systemFontSize()-11),0,0);
    else
        vLayout->setContentsMargins(0,0,0,0);
    connect(d->m_gsetting,&QGSettings::changed,this,[=](const QString &key){
        if(key.contains("systemFontSize"))
            if(ThemeController::systemFontSize() > 11)
                vLayout->setContentsMargins(0,(ThemeController::systemFontSize()-11),0,0);
            else
                vLayout->setContentsMargins(0,0,0,0);
    });
}

KBallonTip::KBallonTip(const QString &content, const TipType &type, QWidget *parent)
    :KBallonTip(parent)
{
    Q_D(KBallonTip);
    d->m_pContentLabel->setText(content);
    d->m_type = type;
    d->adjustStyle();
}

void KBallonTip::showInfo()
{
    Q_D(KBallonTip);

    show();
    QTimer* timer = new QTimer(this);
    timer->start(d->m_time);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeupDestroy()));

}

void KBallonTip::setTipType(const TipType& type)
{
    Q_D(KBallonTip);
    d->m_type = type;
    d->adjustStyle();
}

TipType KBallonTip::tipType()
{
    Q_D(KBallonTip);
    return d->m_type;
}

void KBallonTip::setText(const QString &text)
{
    Q_D(KBallonTip);
    d->m_pContentLabel->setText(text);
}

QString KBallonTip::text()
{
    Q_D(KBallonTip);
    return d->m_pContentLabel->text();
}

void KBallonTip::setContentsMargins(int left, int top, int right, int bottom)
{
    Q_D(KBallonTip);
    d->m_pLayout->setContentsMargins(left,top,right,bottom);
    repaint();
}

void KBallonTip::setContentsMargins(const QMargins &margins)
{
    Q_D(KBallonTip);
    d->m_pLayout->setContentsMargins(margins);
    repaint();
}

void KBallonTip::setTipTime(int my_time)
{
    Q_D(KBallonTip);
    d->m_time=my_time;
}

void KBallonTip::onTimeupDestroy()
{
    this->close();
}

void KBallonTip::paintEvent(QPaintEvent *event)
{
    Q_D(KBallonTip);
    // bug 173714 173165
    adjustSize();

    d->m_pShadow_effect->setColor(this->palette().color(QPalette::Disabled,QPalette::Text));
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    auto color = ThemeController::mixColor(this->palette().color(QPalette::Mid),QColor(100,100,100),0.5);
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(0.2);
    painter.setPen(pen);
    painter.setBrush(this->palette().color(QPalette::Window));
    QRect rect = this->rect();
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        QPainterPath path;
        path.addRect(rect.adjusted(8,8,-8,-8));
        QRegion region(path.toFillPolygon().toPolygon());
        KWindowEffects::enableBlurBehind(this->winId(),true,region);
        this->setMask(region);
        switch (tipType())
        {
        case TipType::Nothing:
            painter.setBrush(this->palette().color(QPalette::Window));
            break;
        case TipType::Normal:
            painter.setBrush(ThemeController::lanhuHSLToQtHsl(98, 65, 78));
            painter.setOpacity(0.15);
            break;
        case TipType::Info:
            painter.setBrush(ThemeController::lanhuHSLToQtHsl(207, 100, 85));
            painter.setOpacity(0.15);
            break;
        case TipType::Warning:
            painter.setBrush(ThemeController::lanhuHSLToQtHsl(35, 100, 82));
            painter.setOpacity(0.15);
            break;
        case TipType::Error:
            painter.setBrush(ThemeController::lanhuHSLToQtHsl(3, 100, 81));
            painter.setOpacity(0.15);
            break;
        default:
            break;
        }
        if(painter.pen().width() == 1)
            painter.translate(0.5,0.5);
        painter.drawRect(rect.adjusted(8,8,-8,-8));
    }
    else
    {
        if(!mask().isNull())
            clearMask();
        painter.setBrush(this->palette().color(QPalette::Window));
        painter.drawRoundedRect(rect.adjusted(8,8,-8,-8), 6, 6);
    }
}

KBallonTipPrivate::KBallonTipPrivate(KBallonTip *parent)
    :q_ptr(parent)
{
    Q_Q(KBallonTip);
    m_time = 1000;
    setParent(parent);
    connect(m_gsetting,&QGSettings::changed,this,[=](){changeTheme();});
}

void KBallonTipPrivate::adjustStyle()
{
    Q_Q(KBallonTip);
    switch (m_type)
    {
    case TipType::Nothing:
        m_icon = QIcon();
        m_pIconLabel->hide();
        break;
    case TipType::Normal:
//        m_icon = QIcon::fromTheme("lingmo-dialog-success");
        m_icon = QIcon::fromTheme("lingmo-dialog-success", QIcon::fromTheme("emblem-default"));
        m_pIconLabel->show();
        break;
    case TipType::Info:;
        m_icon = QIcon::fromTheme("dialog-info");
        m_pIconLabel->show();
        break;
    case TipType::Warning:
        m_icon = QIcon::fromTheme("dialog-warning");
        m_pIconLabel->show();
        break;
    case TipType::Error:
        m_icon = QIcon::fromTheme("dialog-error");
        m_pIconLabel->show();
        break;
    default:
        break;
    }
    m_pIconLabel->setPixmap(m_icon.pixmap(22,22));
    m_pIconLabel->setFixedSize(22,22);
    m_pContentLabel->setAlignment(Qt::AlignLeft);
}

void KBallonTipPrivate::changeTheme()
{
    Q_Q(KBallonTip);
    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        m_pShadow_effect->setOffset(0, 0);
        //阴影半径
        m_pShadow_effect->setColor(Qt::red);
        m_pShadow_effect->setBlurRadius(0);
        q->setGraphicsEffect(m_pShadow_effect);
    }
    else
    {
        m_pShadow_effect->setOffset(0, 0);
        //阴影颜色
        m_pShadow_effect->setColor(q->palette().color(QPalette::Disabled,QPalette::Text));
        //阴影半径
        if(ThemeController::themeMode() == LightTheme)
        {
            m_pShadow_effect->setBlurRadius(15);
        }
        else
            m_pShadow_effect->setBlurRadius(0);
        q->setGraphicsEffect(m_pShadow_effect);
    }
}

}

#include "kballontip.moc"
#include "moc_kballontip.cpp"
