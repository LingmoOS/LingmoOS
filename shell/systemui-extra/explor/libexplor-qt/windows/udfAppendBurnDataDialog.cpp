/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifdef LINGMO_UDF_BURN

#include "udfAppendBurnDataDialog.h"
#include <libkyudfburn/disccontrol.h>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QDir>

#include "file-enumerator.h"
#include "file-info-job.h"
#include "file-info.h"

static bool b_canClose = true;

using namespace UdfBurn;

UdfAppendBurnDataDialog::UdfAppendBurnDataDialog(const QString &uri, DiscControl *discControl, QWidget *parent):
        QDialog(parent), m_uri(uri),m_discControl(discControl)
{

    setAutoFillBackground(true);
    setWindowTitle(tr("AppendBurnData"));
    setBackgroundRole(QPalette::Base);
    setContentsMargins(24,24,24,24);
    setFixedSize(m_widgetWidth, m_widgetHeight);
    setWindowIcon(QIcon::fromTheme("system-file-manager"));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    m_mainLayout = new QGridLayout(this);
    m_mainLayout->setMargin(0);
    m_mainLayout->setHorizontalSpacing(10);
    m_mainLayout->setVerticalSpacing(28);

    m_discTypeLabel = new QLabel;
    m_discTypeLabel->setText(tr("Disc Type:"));
    m_discTypeEdit = new QLineEdit;
    m_discTypeEdit->setReadOnly(true);
    m_discTypeEdit->setEnabled(false);
    m_mainLayout->addWidget(m_discTypeLabel, 1, 1, 1, 2);
    m_mainLayout->addWidget(m_discTypeEdit, 1, 3, 1, 8);

    m_discNameLabel = new QLabel;
    m_discNameLabel->setText(tr("Device Name:"));
    m_discNameEdit = new QLineEdit;
    m_mainLayout->addWidget(m_discNameLabel, 2, 1, 1, 2);
    m_mainLayout->addWidget(m_discNameEdit, 2, 3, 1, 8);

    m_progress = new QProgressBar;
    m_progress->setMinimum(0);
    m_progress->setValue(0);
    m_progress->setTextVisible(false);
    m_progress->setVisible(false);
    m_mainLayout->addWidget(m_progress, 3, 1, 1, 8);

    m_okBtn = new QPushButton;
    m_okBtn->setText(tr("OK"));
    m_cancelBtn = new QPushButton;
    m_cancelBtn->setText(tr("Cancel"));
    m_mainLayout->addWidget(m_cancelBtn, 4, 5, 1, 2);
    m_mainLayout->addWidget(m_okBtn, 4, 7, 1, 2);


    QString discType = m_discControl->discMediaType();
    QString discName = m_discControl->discLabel();
    if (discType.isEmpty()) {
        m_discTypeEdit->setText(tr("Unknown"));
    } else {
        m_discTypeEdit->setText(discType);
    }
    m_discNameEdit->setText(discName);
    b_canClose = true;

    connect(m_okBtn, &QPushButton::clicked, this, &UdfAppendBurnDataDialog::slot_udfAppendBurnData, Qt::UniqueConnection);
    connect(m_cancelBtn, &QPushButton::clicked, this, &UdfAppendBurnDataDialog::slot_udfAppendBurnDataCancel, Qt::UniqueConnection);
    connect(m_discControl, &DiscControl::appendBurnDataUdfFinished, this,  &UdfAppendBurnDataDialog::slot_appendBurnDataFinished, Qt::UniqueConnection);
}

UdfAppendBurnDataDialog::~UdfAppendBurnDataDialog()
{
    if(m_discControl){
        m_discControl->deleteLater();
        m_discControl = nullptr;
    }
    if(m_thread){
        m_thread->quit();
        m_thread->deleteLater();
        m_thread = nullptr;
    }
}

void UdfAppendBurnDataDialog::slot_udfAppendBurnDataCancel()
{
    this->close();
}

void UdfAppendBurnDataDialog::slot_udfAppendBurnData()
{
    qDebug() << "begin slot_udappendBurnData";

    /* R类型光盘，讲缓存刻录数据的临时目录绝对路径传入刻录接口进行刻录操作 */
    QString parentDirForBurnFiles = QDir::homePath()+"/.cache/LingmoTransitBurner/";
    m_urisOfBurningCachedData.clear();

    Peony::FileEnumerator e;
    e.setEnumerateDirectory(QString("file://").append(parentDirForBurnFiles));
    e.enumerateSync();
    if(e.getChildren().size() <= 0){
        QMessageBox::warning(nullptr, tr("Warning"), tr("No burn data, please add!"), QMessageBox::Ok);
        setButtonState(false);
        m_urisOfBurningCachedData.clear();
        this->close();
        return;
    }
    for(auto &fileInfo : e.getChildren()){
        m_urisOfBurningCachedData.append(fileInfo.get()->uri());
    }

    setButtonState(true);
    if (!udfAppendBurnDataEnsureMsgBox()) {
        setButtonState(false);
        return;
    }

    if (m_discNameEdit->text().isEmpty()) {
        QMessageBox::warning(nullptr, tr("Warning"), tr("The disc name cannot be set to empty, please re-enter it!"), QMessageBox::Ok);
        setButtonState(false);
        return;
    }

    qDebug() << "卷标名称为： " << m_discNameEdit->text();

    m_progress->setVisible(true);
    m_progress->setMaximum(0);
    b_canClose = false;

    /* udf追加刻录线程 */
    m_thread = new QThread();
    m_discControl->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_discControl, [=](){
        m_discControl->appendBurnDataUdfSync(m_discNameEdit->text(), parentDirForBurnFiles);
    }, Qt::UniqueConnection);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater, Qt::UniqueConnection);
    connect(m_thread, &QThread::finished, m_discControl, &DiscControl::deleteLater, Qt::UniqueConnection);
    m_thread->start();
}

#include "file-operation-utils.h"
void UdfAppendBurnDataDialog::slot_appendBurnDataFinished(bool successful, QString errorInfo)
{
    m_thread->quit();

    b_canClose = true;
    m_progress->setVisible(false);
    setButtonState(false);

    if (successful) {
        QMessageBox::about(this, tr("AppendBurnData"), tr("AppendBurnData operation has been finished successfully."));
        /* 刻录操作完成后需要手动清空刻录缓存数据即'.cache/LingmoTransitBurner/' */
        Peony::FileOperationUtils::remove(m_urisOfBurningCachedData);

    } else {
        if(errorInfo.isEmpty())
            errorInfo = tr("Sorry, the appendBurnData operation is failed!");
        QMessageBox::critical(this, tr("Failed"), errorInfo, QMessageBox::Ok);
    }

    this->close();
}

void UdfAppendBurnDataDialog::closeEvent(QCloseEvent *e)
{
    if (!b_canClose) {
        QMessageBox::warning(nullptr, tr("Burning. Do not close this window"), tr("Burning. Do not close this window"), QMessageBox::Ok);
        e->ignore();
        return;
    }
}

bool UdfAppendBurnDataDialog::udfAppendBurnDataEnsureMsgBox()
{
    QMessageBox ensureMsgBox(this);
    ensureMsgBox.setText(tr("Burning this disc will append datas on it. Do you want to continue ?"));
    ensureMsgBox.setWindowTitle(tr("Burn"));

    QPushButton* okBtn = ensureMsgBox.addButton(tr("Begin Burning"), QMessageBox::YesRole);
    QPushButton* cancelBtn = ensureMsgBox.addButton(tr("Close"), QMessageBox::NoRole);

    ensureMsgBox.exec();

    if (ensureMsgBox.clickedButton() == cancelBtn) {
        setButtonState(false);
        return false;
    } else if (ensureMsgBox.clickedButton() == okBtn) {
        return true;
    }
    return false;
}

void UdfAppendBurnDataDialog::setButtonState(bool state)
{
    m_okBtn->setDisabled(state);
    m_cancelBtn->setDisabled(state);
    m_discNameEdit->setDisabled(state);
}

#endif
