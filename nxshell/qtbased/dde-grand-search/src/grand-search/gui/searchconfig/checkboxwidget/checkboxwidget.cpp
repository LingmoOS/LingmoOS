// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwidget.h"
#include "checkboxitem.h"
#include "global/searchconfigdefine.h"

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

CheckBoxWidget::CheckBoxWidget(QWidget *parent)
    : DWidget(parent)
{
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(1);
    m_mainLayout->setMargin(0);

    setLayout(m_mainLayout);
}

CheckBoxWidget::~CheckBoxWidget()
{
}

int CheckBoxWidget::addCheckBox(const QString &text, bool isChecked)
{
    int index = m_checkBoxList.size();

    CheckBoxItem *c = new CheckBoxItem(text, this);
    c->setFixedSize(CHECKBOXITEMWIDTH, CHECKBOXITEMHEIGHT);
    addCheckBox(index, c, isChecked);

    return index;
}

void CheckBoxWidget::addCheckBox(int index, CheckBoxItem *item, bool isChecked)
{
    item->setChecked(isChecked);
    m_mainLayout->insertWidget(index, item);
    m_checkBoxList << item;

    for (int i = 0; i < m_checkBoxList.count(); ++i) {
        if (i == 0) {
            m_checkBoxList[i]->setTopRound(true);
        } else if (i == m_checkBoxList.count() - 1) {
            m_checkBoxList[i]->setBottomRound(true);
        } else {
            m_checkBoxList[i]->setTopRound(false);
            m_checkBoxList[i]->setBottomRound(false);
        }
    }

    connect(item, &CheckBoxItem::toggled, this, &CheckBoxWidget::onCheckBoxChecked);
}

bool CheckBoxWidget::setCheckBoxChecked(int index, bool isChecked)
{
    if (index < 0 || index >= m_checkBoxList.size())
        return false;

    m_checkBoxList[index]->setChecked(isChecked);
    return true;
}

bool CheckBoxWidget::setCheckBoxChecked(const QString &text, bool isChecked)
{
    int i = -1;
    for (const auto *c : m_checkBoxList) {
        ++i;
        if (c->text() == text)
            break;
    }

    return setCheckBoxChecked(i, isChecked);
}

void CheckBoxWidget::onCheckBoxChecked(bool checked)
{
    CheckBoxItem *c = qobject_cast<CheckBoxItem *>(sender());
    if (c) {
        int index = m_checkBoxList.indexOf(c);
        emit checkedChanged(index, checked);
    }
}
