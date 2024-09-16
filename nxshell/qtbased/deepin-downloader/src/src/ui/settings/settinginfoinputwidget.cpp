// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file settinginfoinputwidget.cpp
 *
 * @brief 设置信息可编辑输入窗口控件，最大下载限速以及最大上传限速用到此控件
 *
 *@date 2020-06-09 10:45
 *
 * Author: yuandandan  <yuandandan@uniontech.com>
 *
 * Maintainer: yuandandan  <yuandandan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "settinginfoinputwidget.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QHBoxLayout>

SettingInfoInputWidget::SettingInfoInputWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
}

void SettingInfoInputWidget::initUI()
{
    m_lineEdit = new DLineEdit;
    m_lineEdit->setAccessibleName(m_lineEdit->text());
    m_titleLabel = new DLabel;
    m_unitLabel = new DLabel;
    m_rangeLabel = new DLabel;

    m_lineEdit->setMinimumWidth(150);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_titleLabel);
    hLayout->addWidget(m_lineEdit);
    hLayout->setContentsMargins(0, 0, 0, 0);

    m_widget = new QWidget;
    m_widget->setLayout(hLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
//    pMainLayout->addWidget(m_titleLabel);
//    pMainLayout->addWidget(m_lineEdit);
    mainLayout->addWidget(m_widget);
    mainLayout->addWidget(m_unitLabel);
    mainLayout->addWidget(m_rangeLabel);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
}

void SettingInfoInputWidget::initConnections()
{
    connect(m_lineEdit, &DLineEdit::textChanged, this, &SettingInfoInputWidget::onLineEditTextChanged);
    connect(m_lineEdit, &DLineEdit::focusChanged, this, &SettingInfoInputWidget::focusChanged);
}

void SettingInfoInputWidget::onLineEditTextChanged(const QString &text)
{
    emit textChanged(text);
}

void SettingInfoInputWidget::setTitleLabelText(const QString &text)
{
    m_titleLabel->setText(text);
}

void SettingInfoInputWidget::setUnitLabelText(const QString &text)
{
    m_unitLabel->setText(text);
}

void SettingInfoInputWidget::setRangeLabelText(const QString &text)
{
    m_rangeLabel->setText(text);
}

void SettingInfoInputWidget::setLineEditText(const QString &text)
{
    m_lineEdit->setText(text);
    m_lineEdit->setAccessibleName(text);
}

void SettingInfoInputWidget::setLineEditIsDisabled(bool isDisabled)
{
    m_lineEdit->setDisabled(isDisabled);
}

void SettingInfoInputWidget::setValid(const int &minValid, const int &maxValid)
{
    QIntValidator *validator = new QIntValidator(minValid, maxValid, this);
    m_lineEdit->lineEdit()->setValidator(validator);
}

QString SettingInfoInputWidget::getLineEditText()
{
    QString strText = m_lineEdit->text();

    return strText;
}

void SettingInfoInputWidget::setWidgetWidth(const int &width)
{
    m_widget->setFixedWidth(width);
}

void SettingInfoInputWidget::setRangeLabelFont(QFont font)
{
    m_rangeLabel->setFont(font);
}

void SettingInfoInputWidget::setRangeLabelPalette(DPalette palette)
{
    m_rangeLabel->setPalette(palette);
}

void SettingInfoInputWidget::setLineEditAlert(bool isAlert)
{
    m_lineEdit->setAlert(isAlert);
}

DLineEdit *SettingInfoInputWidget::getLineEdit()
{
    return m_lineEdit;
}

bool SettingInfoInputWidget::getIsAlert()
{
    return m_lineEdit->isAlert();
}

