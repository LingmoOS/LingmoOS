// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "datetitlewidget.h"
#include <DFontSizeManager>
#include <QLayout>

DateTitleWidget::DateTitleWidget(QWidget *parent)
    : QWidget(parent)
    , m_dayofMonthLabel(new DLabel(this))
    , m_monthLabel(new DLabel(this))
{
    QHBoxLayout *mainlayout = new QHBoxLayout;
    QVBoxLayout *leftlayout = new QVBoxLayout;
    QVBoxLayout *rightlayout = new QVBoxLayout;

    leftlayout->addStretch();
    leftlayout->addWidget(m_dayofMonthLabel);
    m_dayofMonthLabel->setFixedHeight(100);
    rightlayout->addStretch();
    rightlayout->addWidget(m_monthLabel);

    DFontSizeManager::instance()->bind(m_monthLabel, DFontSizeManager::T4, 70);

    auto font = m_dayofMonthLabel->font();
    font.setPointSize(80);
    font.setFamily("Noto Sans CJK SC");
    font.setWeight(QFont::Thin);
    m_dayofMonthLabel->setFont(font);

    mainlayout->addLayout(leftlayout);
    mainlayout->addLayout(rightlayout);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainlayout);
}

void DateTitleWidget::setDateLabelText(const QString &month, const int &day)
{
    m_dayofMonthLabel->setText(QString::number(day));
    m_monthLabel->setText(month);
}
