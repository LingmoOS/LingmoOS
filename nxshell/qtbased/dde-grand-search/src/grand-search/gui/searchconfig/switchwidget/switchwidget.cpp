// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "switchwidget.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>

#define ICONLABELSIZE   36

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

SwitchWidget::SwitchWidget(const QString &title, QWidget *parent)
    : SwitchWidget(parent, new QLabel(title, parent))
{

}

SwitchWidget::SwitchWidget(QWidget *parent, QWidget *leftWidget)
    : RoundedBackground (parent)
    , m_leftWidget(leftWidget)
{

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(ICONLABELSIZE, ICONLABELSIZE);

    if (!m_leftWidget)
        m_leftWidget = new QLabel(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_switchBtn = new DSwitchButton(this);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_leftWidget);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->addWidget(m_iconLabel);
    m_mainLayout->addLayout(labelLayout, 0);
    m_mainLayout->addWidget(m_switchBtn, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    connect(m_switchBtn, &DSwitchButton::toggled, this, &SwitchWidget::checkedChanged);

    setTopRound(true);
    setBottomRound(true);
}

void SwitchWidget::setChecked(const bool checked)
{
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(checked);
    m_switchBtn->blockSignals(false);
}

bool SwitchWidget::checked() const
{
    return m_switchBtn->isChecked();
}

void SwitchWidget::setTitle(const QString &title)
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        label->setText(title);
        label->setWordWrap(true);
        label->adjustSize();
    }
}

QString SwitchWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

    return QString();
}

void SwitchWidget::setIconEnable(bool e)
{
    m_iconLabel->setVisible(e);
}

void SwitchWidget::setIcon(const QIcon &icon, const QSize &size)
{
    m_iconLabel->setPixmap(icon.pixmap(size));

    update();
}

void SwitchWidget::setEnableBackground(const bool enable)
{
    if (enable == m_hasBack)
        return;
    m_hasBack = enable;
    update();
}

bool SwitchWidget::enableBackground() const
{
    return m_hasBack;
}

void SwitchWidget::paintEvent(QPaintEvent *event)
{
    if (m_hasBack) {
        return RoundedBackground::paintEvent(event);
//        const DPalette &dp = DApplicationHelper::instance()->palette(this);
//        QPainter p(this);
//        p.setPen(Qt::NoPen);
//        p.setBrush(dp.brush(DPalette::ItemBackground));
//        p.drawRoundedRect(rect(), 8, 8);
    }
    return QWidget::paintEvent(event);
}
