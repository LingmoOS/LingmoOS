// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "updateaddressitem.h"

#include <QCheckBox>
#include <QLabel>

UpdateAddressItem::UpdateAddressItem(QWidget *parent)
    : QWidget(parent)
    , m_context(new QLabel(this))
    , m_address(new DLineEdit(this))
    , m_checkbox(new QCheckBox(this))
{
    this->setContentsMargins(0, 0, 0, 0);
    // 布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_context->setText(tr("Virus database server address"));
    // 不可以编辑
    m_address->setEnabled(false);
    layout->addWidget(m_checkbox);
    layout->addWidget(m_context);
    layout->addWidget(m_address);
    this->setLayout(layout);
    // 信号绑定
    connect(m_address, &DLineEdit::textChanged, this, &UpdateAddressItem::lineEditChange);
    connect(m_address, &DLineEdit::focusChanged, this, &UpdateAddressItem::focusChanged);
    connect(m_checkbox, &QCheckBox::stateChanged, this, [=](int status) {
        m_address->setEnabled(m_checkbox->isChecked());
        Q_EMIT sendCheckBoxChange(status);
    });
}

// 设置升级地址
void UpdateAddressItem::setUpdateAddres(QString address)
{
    m_address->setText(address);
}

// 设置勾选状态
void UpdateAddressItem::setCheckboxStatus(bool status)
{
    m_checkbox->setChecked(status);
    m_address->setEnabled(status);
    // 根据勾选状态显示 占位提示
    if (status && m_address->text().isEmpty()) {
        m_address->setAlert(true);
    } else {
        m_address->setAlert(false);
    }
}

// 编辑改变
void UpdateAddressItem::lineEditChange(QString str)
{
    // 输入框每次改变内容 判定是不是要显示红色提示
    if ((str.isEmpty() && getCheckBoxStatus())) {
        m_address->setAlert(true);
    } else {
        m_address->setAlert(false);
    }
}

// 焦点改变
void UpdateAddressItem::focusChanged(bool change)
{
    // 失去焦点 代表输入完成
    if (!change) {
        Q_EMIT sendInputFinised(m_address->text());
    }
}

bool UpdateAddressItem::getCheckBoxStatus() const
{
    return m_checkbox->isChecked();
}
