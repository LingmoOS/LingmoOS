// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tip.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDBusReply>

#include <DUtil>
#include <dthememanager.h>
#include <DLabel>

DWIDGET_USE_NAMESPACE
namespace dmr {
class TipPrivate
{
public:
    explicit TipPrivate(Tip *parent) : q_ptr(parent) {}

    void setBackgroundImage(const QPixmap &srcPixmap);

    QBrush          background;
    int             radius              = 8;
    int             shadowWidth         = 20;
    QMargins        shadowMargins       = QMargins(20, 20, 20, 20);
    QColor          borderColor         = QColor(0, 0, 0, static_cast<int>(0.2 * 255));

    DLabel          *textLable          = nullptr;
    QFrame          *m_interFrame       = nullptr;


    Tip *q_ptr;
    Q_DECLARE_PUBLIC(Tip)
};


Tip::Tip(const QPixmap &icon, const QString &text, QWidget *parent)
    : QFrame(parent), d_ptr(new TipPrivate(this))
{
    DThemeManager::instance()->registerWidget(this);
    Q_D(Tip);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::ToolTip | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("Tip");
    setContentsMargins(0, 0, 0, 0);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(7, 4, 7, 4);
    layout->setSpacing(0);

    d->m_interFrame = new QFrame(this);
    d->m_interFrame->setContentsMargins(0, 0, 0, 0);
    auto interlayout = new QHBoxLayout(d->m_interFrame);
    interlayout->setContentsMargins(0, 0, 0, 0);
    interlayout->setSpacing(5);
    auto iconLabel = new QLabel;
    iconLabel->setObjectName("TipIcon");
    iconLabel->setFixedSize(icon.size());
    if (icon.isNull()) {
        iconLabel->hide();
    } else {
        iconLabel->setPixmap(icon);
    }

    d->textLable = new DLabel(text);
    d->textLable->setObjectName("TipText");
    d->textLable->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->textLable->setWordWrap(false);
    DFontSizeManager::instance()->bind(d->textLable, DFontSizeManager::T8);
    d->textLable->setForegroundRole(DPalette::ToolTipText);

    interlayout->addWidget(iconLabel, 0, Qt::AlignVCenter);
    interlayout->addWidget(d->textLable, 0, Qt::AlignVCenter);
    layout->addWidget(d->m_interFrame, 0, Qt::AlignVCenter);

    adjustSize();

    auto *bodyShadow = new QGraphicsDropShadowEffect(this);
    bodyShadow->setBlurRadius(10.0);
    bodyShadow->setColor(QColor(0, 0, 0, static_cast<int>(0.1 * 255)));
    bodyShadow->setOffset(0, 2.0);
    hide();

    m_pWMDBus = new QDBusInterface("com.deepin.WMSwitcher","/com/deepin/WMSwitcher","com.deepin.WMSwitcher",QDBusConnection::sessionBus());
    QDBusReply<QString> reply = m_pWMDBus->call("CurrentWM");
    bIsWM = reply.value().contains("deepin wm");
    connect(m_pWMDBus, SIGNAL(WMChanged(QString)), this, SLOT(slotWMChanged(QString)));
}

Tip::~Tip()
{

}

void Tip::enterEvent(QEvent *e)
{
    hide();

    QFrame::enterEvent(e);
}

QBrush Tip::background() const
{
    Q_D(const Tip);
    return d->background;
}

void Tip::setText(const QString text)
{
    Q_D(const Tip);
    d->textLable->setText(text);
    m_strText = text;
    update();
}

int Tip::radius() const
{
    Q_D(const Tip);
    return d->radius;
}

QColor Tip::borderColor() const
{
    Q_D(const Tip);
    return d->borderColor;
}

void Tip::setBackground(QBrush background)
{
    Q_D(Tip);
    d->background = background;
}

void Tip::setRadius(int radius)
{
    Q_D(Tip);
    d->radius = radius;
}

void Tip::setBorderColor(QColor borderColor)
{
    Q_D(Tip);
    d->borderColor = borderColor;
}

void Tip::slotWMChanged(QString msg)
{
    if (msg.contains("deepin metacity")) {
        bIsWM = false;
    } else {
        bIsWM = true;
    }
}

void Tip::pop(QPoint center)
{
    Q_D(Tip);
    this->show();
    center = center - QPoint(width() / 2, height() / 2);
    this->move(center);
}

#ifdef _OLD
void Tip::paintEvent(QPaintEvent *)
{
    Q_D(Tip);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    auto radius = d->radius;
    auto penWidthf = 1.0;
    const QPalette pal = QGuiApplication::palette();//this->palette();
    QColor background = pal.color(QPalette::ToolTipBase);
    DPalette pa_name = DApplicationHelper::instance()->palette(d->textLable);
    pa_name.setBrush(DPalette::Text, pa_name.color(DPalette::ToolTipText));
    pa_name.setBrush(DPalette::ToolTipText, pa_name.color(DPalette::ToolTipText));
    d->textLable->setForegroundRole(DPalette::Text);
    d->textLable->setForegroundRole(DPalette::ToolTipText);
    d->textLable->setPalette(pa_name);
    auto borderColor = d->borderColor;
    auto margin = 2.0;
    auto shadowMargins = QMarginsF(margin, margin, margin, margin);

    auto backgroundRect = QRectF(rect()).marginsRemoved(shadowMargins);
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(backgroundRect, radius, radius);
    painter.fillPath(backgroundPath, background);

    QPainterPath borderPath;
    QRectF borderRect = QRectF(rect());
    auto borderRadius = radius;
    QMarginsF borderMargin(penWidthf / 2, penWidthf / 2, penWidthf / 2, penWidthf / 2);

    borderRadius += penWidthf / 2;
    borderRect = borderRect.marginsAdded(borderMargin).marginsRemoved(shadowMargins);
    borderPath.addRoundedRect(borderRect, borderRadius, borderRadius);
    QPen borderPen(borderColor);
    borderPen.setWidthF(penWidthf);
    painter.strokePath(borderPath, borderPen);
}
#else
void Tip::paintEvent(QPaintEvent *)
{
    Q_D(Tip);
    QPainter pt(this);
    pt.setRenderHint(QPainter::Antialiasing);

    int transparency = 245;
    if (!bIsWM) {
        transparency = 255;
    }
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        pt.setPen(QColor(0, 0, 0, 10));
        pt.setBrush(QBrush(QColor(247, 247, 247, transparency)));
    } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        pt.setPen(QColor(255, 255, 255, 10));
        pt.setBrush(QBrush(QColor(42, 42, 42, transparency)));
    } else {
        pt.setPen(QColor(0, 0, 0, 10));
        pt.setBrush(QBrush(QColor(247, 247, 247, transparency)));
    }

    QRect rect = this->rect();
    QPainterPath painterPath;
    if (bIsWM) {
        rect.setWidth(rect.width() - 1);
        rect.setHeight(rect.height() - 1);
        painterPath.addRoundedRect(rect, d->radius, d->radius);
    } else {
        painterPath.addRect(rect);
    }
    pt.drawPath(painterPath);

}
#endif

void Tip::resizeEvent(QResizeEvent *ev)
{
    return QWidget::resizeEvent(ev);
}

void Tip::resetSize(const int maxWidth)
{
    Q_D(Tip);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    QFontMetrics fm(font);
    auto w = fm.boundingRect(d->textLable->text()).width();

    if (w >= maxWidth - 14) {
        d->textLable->setWordWrap(true);
        this->setFixedWidth(maxWidth);
        d->textLable->setFixedWidth(maxWidth - 14);
    }
}

}
