// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "comboboxwidget.h"
#include "global/searchhelper.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define COMBOBOXW 250
#define COMBOBOXH 36

ComboboxWidget::ComboboxWidget(const QString &title, QWidget *parent)
    : ComboboxWidget(parent, new QLabel(title, parent))
{

}

ComboboxWidget::ComboboxWidget(QWidget *parent, QWidget *leftWidget)
    : QWidget (parent)
    , m_leftWidget(leftWidget)
{
    if (!m_leftWidget)
        m_leftWidget = new QLabel(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_comboBox = new DComboBox(this);
    m_comboBox->setFixedSize(COMBOBOXW, COMBOBOXH);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_leftWidget);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 6, 10, 6);
    m_mainLayout->addLayout(labelLayout, 0);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_comboBox, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    connect(m_comboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &ComboboxWidget::checkedChanged);
}

void ComboboxWidget::setChecked(const QString &text)
{
    m_comboBox->setCurrentText(text);
}

QString ComboboxWidget::checked() const
{
    return m_comboBox->currentText();
}

void ComboboxWidget::setTitle(const QString &title)
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        label->setText(title);
        label->adjustSize();
    }
}

QString ComboboxWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

    return QString();
}

void ComboboxWidget::setEnableBackground(const bool enable)
{
    if (enable == m_hasBack)
        return;
    m_hasBack = enable;
    update();
}

bool ComboboxWidget::enableBackground() const
{
    return m_hasBack;
}

DComboBox *ComboboxWidget::getComboBox()
{
    return m_comboBox;
}

void ComboboxWidget::paintEvent(QPaintEvent *event)
{
    if (m_hasBack) {
        const DPalette &dp = DApplicationHelper::instance()->palette(this);
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(dp.brush(DPalette::ItemBackground));
        p.drawRoundedRect(rect(), 8, 8);
    }
    return QWidget::paintEvent(event);
}
