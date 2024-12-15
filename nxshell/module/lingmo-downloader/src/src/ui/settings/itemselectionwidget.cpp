// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file itemselectionwidget.cpp
 *
 * @brief 自定义复选框窗口控件，主要实现标题名称居左，复选框居右，且复选框为圆形的控件窗口
 *
 *@date 2020-06-09 10:49
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
#include "itemselectionwidget.h"

#include <QHBoxLayout>
#include <QDebug>
#include <DCommandLinkButton>
#include "httpadvancedsettingwidget.h"

ItemSelectionWidget::ItemSelectionWidget(QWidget *parent, bool isHttp)
    : QWidget(parent)
{
    initUI(isHttp);
    initConnections();
}

// 初始化界面
void ItemSelectionWidget::initUI(bool isHttp)
{
//    setStyleSheet("background:rgba(211, 211, 211, 1)");

    m_label = new DLabel("HTTP下载");
    m_checkBox = new DCheckBox;
//    m_pCheckBox->setStyleSheet("QCheckBox::indicator:unchecked{image:url(:/icons/unchecked);width:24px;height:24px;}"
//                               "QCheckBox::indicator:checked{image:url(:/icons/checked);width:24px;height:24px;}");

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_checkBox);
    mainLayout->addWidget(m_label);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    if(isHttp){
        DCommandLinkButton * combtn = new DCommandLinkButton(tr("Advanced"));   //高级设置
        connect(combtn,&DCommandLinkButton::clicked, [=](){
            HttpAdvancedSettingWidget h(this);
            h.exec();
        });
        mainLayout->addWidget(combtn);
    }

    setLayout(mainLayout);
}

// 初始化链接
void ItemSelectionWidget::initConnections()
{
    connect(m_checkBox, &QCheckBox::stateChanged, this, &ItemSelectionWidget::onCheckBoxStateChanged);
}

void ItemSelectionWidget::onCheckBoxStateChanged(int state)
{
    if (state == Qt::Unchecked) {
        emit checkBoxIsChecked(false);
    } else if(state == Qt::Checked) {
        emit checkBoxIsChecked(true);
    }
}

void ItemSelectionWidget::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void ItemSelectionWidget::setCheckBoxChecked(bool isChecked)
{
    m_checkBox->setChecked(isChecked);
}

void ItemSelectionWidget::setCheckboxState(Qt::CheckState state)
{
    m_checkBox->setCheckState(state);
}

void ItemSelectionWidget::setBlockSignals(bool lock)
{
    m_checkBox->blockSignals(lock);
}

void ItemSelectionWidget::setAccessibleName(QString name)
{
    m_checkBox->setAccessibleName(name);
}

