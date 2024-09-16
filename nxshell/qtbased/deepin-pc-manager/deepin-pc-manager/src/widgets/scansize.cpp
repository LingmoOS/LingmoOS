// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scansize.h"

#include "../window/modules/common/common.h"

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpressionValidator>

ScanSize::ScanSize(QWidget *parent)
    : QWidget(parent)
    , m_context(new QLabel(this))
    , m_unit(new QLabel(this))
    , m_pLineEdit(new DLineEdit(this))
    , m_check(new QCheckBox(this))
{
    this->setContentsMargins(0, 0, 0, 0);
    // 布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    m_context->setText(tr("Do not scan archives larger than"));
    m_unit->setText("MB(20-5120)");
    m_pLineEdit->setText("20");
    m_pLineEdit->lineEdit()->setValidator(
        new QRegularExpressionValidator(QRegularExpression("0|[1-9]\\d{0,3}"), m_pLineEdit));
    // 不可以编辑
    m_pLineEdit->setEnabled(false);
    layout->addWidget(m_check);
    layout->addWidget(m_context);
    layout->addSpacing(5);
    layout->addWidget(m_pLineEdit);
    layout->addWidget(m_unit);
    layout->addStretch(0);
    this->setLayout(layout);
    // 信号绑定
    connect(m_pLineEdit, &DLineEdit::textChanged, this, &ScanSize::lineEditChange);
    connect(m_pLineEdit, &DLineEdit::focusChanged, this, &ScanSize::focusChanged);
    connect(m_check, &QCheckBox::stateChanged, this, &ScanSize::checkBoxChange);
}

// 设置大小
void ScanSize::setSize(int val)
{
    m_pLineEdit->setText(QString("%1").arg(val));
}

// 设置选择框
void ScanSize::setcheckbox(bool status)
{
    m_check->setChecked(status);
    // 勾选了输入框可以编辑  不够选不可以编辑
    m_pLineEdit->setEnabled(status);
}

// 选择框的改变
void ScanSize::checkBoxChange(int status)
{
    m_pLineEdit->setEnabled(m_check->isChecked());
    emit notifyCheckBoxChange(status);
}

// 编辑改变
void ScanSize::lineEditChange(QString str)
{
    if ((str == "") || (m_pLineEdit->text().toInt() < MIN_SCAN)
        || (m_pLineEdit->text().toInt() > MAX_SCAN)) {
        // 该注释下一版需要,暂时不删
        // m_pLineEdit->showAlertMessage(tr("Limited between 20-5120"), this->parentWidget(), -1);
        m_pLineEdit->setAlert(true);
        m_pLineEdit->setAlertMessageAlignment(Qt::AlignLeft);
    } else {
        m_pLineEdit->setAlert(false);
        m_pLineEdit->hideAlertMessage();
    }
}

// 焦点改变
void ScanSize::focusChanged(bool change)
{
    // 失去焦点 代表输入完成
    if (!change) {
        // 判断输入范围  默认都是在范围中
        if ((m_pLineEdit->text().toInt() <= MIN_SCAN) || (m_pLineEdit->text() == "")) {
            setSize(MIN_SCAN);
        } else if (m_pLineEdit->text().toInt() >= MAX_SCAN) {
            setSize(MAX_SCAN);
        }
        emit notifyInputFinised(m_pLineEdit->text().toInt());
    }
}
