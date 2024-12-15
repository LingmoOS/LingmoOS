// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "partitiondialog.h"
#include "partedproxy/dmdbushandler.h"
#include "partitioninfo.h"


#include <DFrame>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QDebug>

PartitionDialog::PartitionDialog(QWidget *parent)
    : DDBase(parent)
{
    initUi();
}

void PartitionDialog::initUi()
{
    PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();

    setTitle(tr("Partition %1").arg(info.m_path));
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    m_tipLabel = new DLabel(tr("It will increase the number of partitions on the disk"), this);
    m_tipLabel->setWordWrap(true);
    m_tipLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T6);

    mainLayout->addWidget(m_tipLabel);

    int index = addButton(tr("Cancel"), false, ButtonNormal);
    m_okCode = addButton(tr("Confirm"), true, ButtonRecommend);

    getButton(index)->setAccessibleName("cancelBtn");
    getButton(m_okCode)->setAccessibleName("confirmBtn");
}

void PartitionDialog::setTitleText(const QString &title, const QString &subTitle)
{
    setTitle(title);
    m_tipLabel->setText(subTitle);
}
