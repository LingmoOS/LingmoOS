// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "unmountwarningdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

UnmountWarningDialog::UnmountWarningDialog(QWidget *parent) : DDBase(parent)
{
    initUi();
    initConnection();
}

void UnmountWarningDialog::initUi()
{
    DPalette palette1;
    QColor color("#000000");
    color.setAlphaF(0.9);
    palette1.setColor(DPalette::ToolTipText, color);

    DPalette palette2;
    palette2.setColor(DPalette::TextTips, QColor("#2C4767"));

    QFont font;
    font.setWeight(QFont::Medium);
    font.setFamily("Source Han Sans");
    font.setPixelSize(14);

    DLabel *titleLabel = new DLabel(tr("Unmounting system disk may result in system crash"), this);
    titleLabel->setFont(font);
    titleLabel->setPalette(palette1);
    titleLabel->setAlignment(Qt::AlignCenter);

    m_checkBox = new DCheckBox(this);
    DLabel *label = new DLabel(tr("I will take the risks that may arise"), this);
    label->setFont(font);
    label->setPalette(palette2);
    label->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(m_checkBox);
    layout->addSpacing(10);
    layout->addWidget(label);
    layout->addStretch();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(layout);

    int index = addButton(tr("Cancel"), true, ButtonNormal);
    m_okCode = addButton(tr("Unmount"), false, ButtonWarning);
    getButton(m_okCode)->setDisabled(true);

    getButton(index)->setAccessibleName("cancelBtn");
    getButton(m_okCode)->setAccessibleName("unmountBtn");
}

void UnmountWarningDialog::initConnection()
{
    connect(m_checkBox, &DCheckBox::stateChanged, this, &UnmountWarningDialog::onCheckBoxStateChange);
}

void UnmountWarningDialog::onCheckBoxStateChange(int state)
{
    switch (state) {
    case Qt::CheckState::Unchecked: {
        getButton(m_okCode)->setDisabled(true);
        break;
    }
    case Qt::CheckState::Checked: {
        getButton(m_okCode)->setDisabled(false);
        break;
    }
    default:
        break;
    }
}



