// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "titlebar.h"
#include "utils.h"

#include <QtGui>
#include <DThemeManager>
#include <DWindowCloseButton>
#include <QPainterPath>
#include "../accessibility/ac-deepin-movie-define.h"

DWIDGET_USE_NAMESPACE

namespace dmr {
class TitlebarPrivate
{
public:
    explicit TitlebarPrivate(Titlebar *parent) : q_ptr(parent) {}

    QColor          playColor                   = QColor(255, 255, 255, 204);
    QColor          lightEffectColor            = QColor(200, 200, 200, 45);
    QColor          darkEffectColor             = QColor(30, 30, 30, 50);
    qreal           offsetX                     = 0;
    qreal           offsetY                     = 15;
    qreal           blurRadius                  = 52;
    QGraphicsDropShadowEffect *m_shadowEffect   = nullptr;
    DTitlebar       *m_titlebar                 = nullptr;
    DLabel          *m_titletxt                 = nullptr;
    bool            m_play                      = false;

    Titlebar *q_ptr;
    Q_DECLARE_PUBLIC(Titlebar)
};
/**
 * @brief Titlebar 构造函数
 * @param parent 父窗口
 */
Titlebar::Titlebar(QWidget *parent) : DBlurEffectWidget(parent), d_ptr(new TitlebarPrivate(this))
{
    Q_D(Titlebar);

    setAttribute(Qt::WA_TranslucentBackground, false);
    setFocusPolicy(Qt::NoFocus);
    setObjectName(TITLEBAR);
    setAccessibleName(TITLEBAR);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    d->m_titlebar = new DTitlebar(this);
    d->m_titlebar->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(d->m_titlebar);
    setLayout(layout);

    d->m_titlebar->setWindowFlags(Qt::WindowMinMaxButtonsHint |
                                  Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    d->m_titlebar->setBackgroundTransparent(false);
    d->m_titlebar->setBlurBackground(true);
    d->m_titlebar->setIcon(QIcon::fromTheme("deepin-movie"));
    d->m_titlebar->setTitle("");
    d->m_titletxt = new DLabel(this);
    d->m_titletxt->setText("");
    d->m_titletxt->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T7));
    d->m_titlebar->addWidget(d->m_titletxt, Qt::AlignCenter);

    d->m_shadowEffect = new QGraphicsDropShadowEffect(this);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &Titlebar::slotThemeTypeChanged);
}

Titlebar::~Titlebar()
{

}

void Titlebar::setIcon(QPixmap& mp)
{
    Q_D(const Titlebar);
    d->m_titlebar->setIcon(mp);
}

void Titlebar::slotThemeTypeChanged()
{
    Q_D(const Titlebar);
    QPalette pa1, pa2;
    if (d->m_play) {
        pa1.setColor(QPalette::ButtonText, d->playColor);
        pa2.setColor(QPalette::WindowText, d->playColor);
        d->m_titlebar->setPalette(pa1);
        d->m_titletxt->setPalette(pa2);
        d->m_shadowEffect->setOffset(d->offsetX, d->offsetY);
        d->m_shadowEffect->setBlurRadius(d->offsetY);
        d->m_shadowEffect->setColor(d->darkEffectColor);
    } else {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            pa1.setColor(QPalette::ButtonText, d->playColor);
            pa2.setColor(QPalette::WindowText, d->playColor);
            d->m_titlebar->setPalette(pa1);
            d->m_titletxt->setPalette(pa2);
            d->m_shadowEffect->setOffset(d->offsetX, d->offsetY);
            d->m_shadowEffect->setBlurRadius(d->offsetY);
            d->m_shadowEffect->setColor(d->darkEffectColor);
        } else {
            pa1.setColor(QPalette::ButtonText, QColor(98, 110, 136, 225));
            pa2.setColor(QPalette::WindowText, QColor(98, 110, 136, 225));
            d->m_titlebar->setPalette(pa1);
            d->m_titletxt->setPalette(pa2);
            d->m_shadowEffect->setOffset(d->offsetX, d->offsetY);
            d->m_shadowEffect->setBlurRadius(d->blurRadius);
            d->m_shadowEffect->setColor(d->lightEffectColor);
        }
    }
    this->setGraphicsEffect(d->m_shadowEffect);
}
/**
 * @brief titlebar 获取标题栏对象指针
 * @return titlebar指针
 */
DTitlebar *Titlebar::titlebar()
{
    Q_D(const Titlebar);
    return d->m_titlebar;
}
/**
 * @brief setTitletxt 设置标题栏文本
 * @param title 标题栏文本
 */
void Titlebar::setTitletxt(const QString &title)
{
    Q_D(const Titlebar);
    d->m_titletxt->setText(title);
}
/**
 * @brief setTitleBarBackground 设置标题栏背景是否为播放状态样式
 * @param flag 传入是否为播放状态
 */
void Titlebar::setTitleBarBackground(bool flag)
{
    Q_D(Titlebar);

    QPalette pa1, pa2;

    d->m_play = flag;

    if (d->m_play) {
        d->m_titlebar->setBackgroundTransparent(d->m_play);
        pa1.setColor(QPalette::ButtonText, d->playColor);
        pa2.setColor(QPalette::WindowText, d->playColor);
        d->m_titlebar->setPalette(pa1);
        d->m_titletxt->setPalette(pa2);
        d->m_shadowEffect->setOffset(d->offsetX, d->offsetX);
        d->m_shadowEffect->setBlurRadius(d->offsetX);
        d->m_shadowEffect->setColor(Qt::transparent);
    } else {
        QPalette palette;
        palette.setColor(QPalette::Background, QColor(200, 200, 200, 50));
        this->setPalette(palette);
        d->m_titlebar->setBackgroundTransparent(d->m_play);
        d->m_titlebar->setBlurBackground(d->m_play);
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            pa1.setColor(QPalette::ButtonText, d->playColor);
            pa2.setColor(QPalette::WindowText, d->playColor);
            d->m_titlebar->setPalette(pa1);
            d->m_titletxt->setPalette(pa2);
            d->m_shadowEffect->setOffset(d->offsetX, d->offsetY);
            d->m_shadowEffect->setBlurRadius(d->blurRadius);
            d->m_shadowEffect->setColor(d->darkEffectColor);
        } else {
            pa1.setColor(QPalette::ButtonText, QColor(98, 110, 136, 225));
            pa2.setColor(QPalette::WindowText, QColor(98, 110, 136, 225));
            d->m_titlebar->setPalette(pa1);
            d->m_titletxt->setPalette(pa2);
            d->m_shadowEffect->setOffset(d->offsetX, d->offsetY);
            d->m_shadowEffect->setBlurRadius(d->blurRadius);
            d->m_shadowEffect->setColor(d->lightEffectColor);
        }
    }
    this->setGraphicsEffect(d->m_shadowEffect);

    update();
}
/**
 * @brief paintEvent 绘制事件函数
 * @param pPaintEvent 绘制事件
 */
void Titlebar::paintEvent(QPaintEvent *pe)
{
    Q_D(const Titlebar);

    QPainter painter(this);
    QBrush bgColor;
    if (d->m_play) {
        QPalette palette;
        QPixmap pixmap = QPixmap(":resources/icons/titlebar.png");
        //palette.setBrush(QPalette::Background, QBrush(pixmap.scaled(window()->width(), 50)));
        bgColor = QBrush(pixmap);
        palette.setColor(QPalette::Background, QColor(0, 0, 0, 0));
        this->setPalette(palette);
    } else {
        bgColor = Qt::transparent;
    }

    QPainterPath pp;
    pp.setFillRule(Qt::WindingFill);
    pp.addRect(rect());
    painter.fillPath(pp, bgColor);
}

FullScreenTitlebar::FullScreenTitlebar(QWidget *parent)
    :QFrame(parent)
{
    setFixedHeight(50);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(4, 0, 30, 0);
    setLayout(mainLayout);

    m_iconLabel = new DLabel;
    m_iconLabel->setFixedSize(50, 50);
    m_iconLabel->setContentsMargins(0, 0, 0, 0);
    QIcon icon = QIcon::fromTheme("deepin-movie");
    QPixmap logo = icon.pixmap(QSize(32, 32));
    m_iconLabel->setPixmap(logo);

    m_textLabel = new DLabel;
    m_textLabel->setFixedHeight(height());
    m_textLabel->setAlignment(Qt::AlignCenter);

    m_timeLabel = new DLabel;
    m_timeLabel->setFixedHeight(height());
    m_timeLabel->setAlignment(Qt::AlignCenter);

    QPalette pa = m_textLabel->palette();
    pa.setColor(QPalette::WindowText,Qt::white);
    m_textLabel->setPalette(pa);
    m_timeLabel->setPalette(pa);

    mainLayout->addWidget(m_iconLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(m_textLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(m_timeLabel);
}

void FullScreenTitlebar::setTitletxt(const QString &sTitle)
{
    m_textLabel->setText(sTitle);
}

void FullScreenTitlebar::setTime(const QString &sTime)
{
    m_timeLabel->setText(sTime);
}

void FullScreenTitlebar::paintEvent(QPaintEvent *pPaintEvent)
{
    QString sTimeText = QTime::currentTime().toString("hh:mm");
    setTime(sTimeText);

    QPainter p(this);
    QPainterPath path;
    path.addRect(rect());
    QLinearGradient linear(QPointF(rect().width() / 2, 0), QPointF(rect().width() / 2, rect().height()));
    linear.setColorAt(0, QColor(0, 0, 0, 0.3 * 255));
    linear.setColorAt(1, Qt::transparent);
    p.fillPath(path, linear);
}

}

