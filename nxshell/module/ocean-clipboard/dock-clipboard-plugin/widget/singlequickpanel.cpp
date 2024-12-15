// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "singlequickpanel.h"

#include <QVBoxLayout>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DToolTip>

SignalQuickPanel::SignalQuickPanel(QWidget *parent)
    :QWidget(parent)
    , m_icon(new CommonIconButton(this))
    , m_description(new DLabel(this))
{
    initUI();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &SignalQuickPanel::refreshBg);
}

SignalQuickPanel::~SignalQuickPanel()
{

}

void SignalQuickPanel::initUI()
{
    m_icon->setFixedSize(QSize(24, 24));

    m_description->setElideMode(Qt::ElideRight);
    DToolTip::setToolTipShowMode(m_description, DToolTip::ShowWhenElided);
    DFontSizeManager::instance()->bind(m_description, DFontSizeManager::T10);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_icon, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_description, 0, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);
}

void SignalQuickPanel::setIcon(const QIcon &icon)
{
    m_icon->setIcon(icon, Qt::black, Qt::white);
}

void SignalQuickPanel::setDescription(const QString &description)
{
    m_description->setText(description);
}

void SignalQuickPanel::setWidgetState(WidgetState state)
{
    if (m_icon)
        m_icon->setActiveState(WS_ACTIVE == state);

    refreshBg();
}

void SignalQuickPanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (underMouse()) {
        Q_EMIT clicked();
    }
    return QWidget::mouseReleaseEvent(event);
}

void SignalQuickPanel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    if (m_icon->activeState() && DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        painter.setBrush(QColor(255, 255, 255, 255 * 0.8));
    } else {
        painter.setBrush(Qt::transparent);
    }
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 8, 8);

    QWidget::paintEvent(event);
}

void SignalQuickPanel::refreshBg()
{
    m_description->setForegroundRole(m_icon->activeState() && DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType ? QPalette::Highlight : QPalette::NoRole);
    update();
}
