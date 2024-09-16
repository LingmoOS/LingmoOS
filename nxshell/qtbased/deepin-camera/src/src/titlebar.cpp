// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebar.h"
#include "datamanager.h"

class TitlebarPrivate
{
public:
    explicit TitlebarPrivate(Titlebar *parent) : q_ptr(parent) {}

    QColor          lightColor                  = QColor(255, 255, 255, 204);
    QColor          darkColor                   = QColor(98, 110, 136, 225);
    QColor          darkEffectColor             = QColor(30, 30, 30, 50);
    qreal           offsetX                     = 0;
    qreal           offsetY                     = 15;
    qreal           blurRadius                  = 52;
    QGraphicsDropShadowEffect *m_shadowEffect   = nullptr;
    DTitlebar       *m_titlebar                 = nullptr;

    Titlebar *q_ptr;
    Q_DECLARE_PUBLIC(Titlebar)
};

Titlebar::Titlebar(QWidget *parent) : DBlurEffectWidget(parent), d_ptr(new TitlebarPrivate(this))
{
    Q_D(Titlebar);

    setAttribute(Qt::WA_TranslucentBackground, true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    d->m_titlebar = new DTitlebar(this);
    d->m_titlebar->setBackgroundTransparent(true);
    d->m_titlebar->setBlurBackground(false);
    layout->addWidget(d->m_titlebar);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &Titlebar::slotThemeTypeChanged);
}

DTitlebar *Titlebar::titlebar()
{
    Q_D(const Titlebar);
    return d->m_titlebar;
}

void Titlebar::slotThemeTypeChanged()
{
    Q_D(const Titlebar);
    QPalette pa;
    if(DataManager::instance()->getdevStatus() != NOCAM) {
        pa.setColor(QPalette::ButtonText, d->lightColor);
        d->m_titlebar->setPalette(pa);
    } else{
        if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            pa.setColor(QPalette::ButtonText, d->lightColor);
            d->m_titlebar->setPalette(pa);
        } else {
            pa.setColor(QPalette::ButtonText, d->darkColor);
            d->m_titlebar->setPalette(pa);
        }
    }
}

void Titlebar::paintEvent(QPaintEvent *pe)
{
    Q_D(const Titlebar);

    QPainter painter(this);
    QPalette pa;
    QPen pen(QColor(0, 0, 0, 0));
    QLinearGradient linearGradient(width(), 0, width(), height());

    linearGradient.setColorAt(0, QColor(0, 0, 0, 255 * 0.5));   //垂直线性渐变
    linearGradient.setColorAt(1, QColor(0, 0, 0, 0));

    if(DataManager::instance()->getdevStatus() != NOCAM) {
        pa.setColor(QPalette::ButtonText, d->lightColor);
        d->m_titlebar->setPalette(pa);
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QBrush(linearGradient));
    painter.setPen(pen);
    painter.drawRect(rect());
}

Titlebar::~Titlebar()
{

}
