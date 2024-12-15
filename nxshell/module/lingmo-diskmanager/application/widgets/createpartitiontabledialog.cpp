// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "createpartitiontabledialog.h"

#include "partedproxy/dmdbushandler.h"
#include "messagebox.h"

#include <DLabel>
#include <DFrame>
#include <DFontSizeManager>

#include <QDir>
#include <QVBoxLayout>

CreatePartitionTableDialog::CreatePartitionTableDialog(QWidget *parent)
    : DDBase(parent)
{
    initUi();
    initConnection();
}

void CreatePartitionTableDialog::initUi()
{
    m_ComboBox = new DComboBox;
    m_ComboBox->addItem("GPT");
    m_ComboBox->addItem("MSDOS");
    m_ComboBox->setAccessibleName("choosePartitionTable");

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->addWidget(m_ComboBox);

    int index = addButton(tr("Cancel"), true, ButtonNormal);
    getButton(index)->setAccessibleName("cancel");

    DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
    if (info.m_disktype == "unrecognized") {
        m_curType = "create";
        // 当前磁盘无分区表，是否新建分区表?
        setTitle(tr("No partition table in this disk. Create a new one?"));
        m_okCode = addButton(tr("Create"), false, ButtonRecommend);
        getButton(m_okCode)->setAccessibleName("create");
    } else {
        m_curType = "replace";
        // 当前磁盘已有分区表，是否替换此分区表?
        setTitle(tr("The disk has a partition table already. Replace it?"));
        m_okCode = addButton(tr("Replace"), false, ButtonRecommend);
        getButton(m_okCode)->setAccessibleName("replace");
    }

}

void CreatePartitionTableDialog::initConnection()
{
    connect(this, &CreatePartitionTableDialog::buttonClicked, this, &CreatePartitionTableDialog::onButtonClicked);
}

void CreatePartitionTableDialog::onButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text);
    if (index == m_okCode) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        DMDbusHandler::instance()->createPartitionTable(info.m_path, QString("%1").arg(info.m_length), QString("%1").arg(info.m_sectorSize), m_ComboBox->currentText(), m_curType);
        close();
    } else {
        close();
    }
}

