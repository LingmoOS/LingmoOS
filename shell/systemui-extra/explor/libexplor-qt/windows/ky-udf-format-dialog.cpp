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

#include "ky-udf-format-dialog.h"
#include "format-dlg-create-delegate.h"
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <volumeManager.h>
#include <QMutexLocker>

#include <libkyudfburn/disccontrol.h>

using namespace UdfBurn;

static bool b_finished = false;
static bool b_failed = false;
static bool b_canClose = true;

UdfFormatDialog::UdfFormatDialog(const QString &uri, DiscControl *discControl, QWidget *parent):
        QDialog(parent), m_uri(uri), m_check(false), m_discControl(discControl)
{

    setAutoFillBackground(true);
    setWindowTitle(tr("Format"));
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
    m_mainLayout->addWidget(m_discTypeEdit, 1, 3, 1, 6);

    m_discNameLabel = new QLabel;
    m_discNameLabel->setText(tr("Device Name:"));
    m_discNameEdit = new QLineEdit;
    m_mainLayout->addWidget(m_discNameLabel, 2, 1, 1, 2);
    m_mainLayout->addWidget(m_discNameEdit, 2, 3, 1, 6);

    m_progress = new QProgressBar;
    m_progress->setMinimum(0);
//    m_progress->setMaximum(0);
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

    connect(m_okBtn, &QPushButton::clicked, this, &UdfFormatDialog::slot_udfFormat, Qt::UniqueConnection);
    connect(m_cancelBtn, &QPushButton::clicked, this, &UdfFormatDialog::slot_udfCancel, Qt::UniqueConnection);
    connect(m_discControl, &DiscControl::formatUdfFinished, this,  &UdfFormatDialog::slot_formatFinished, Qt::UniqueConnection);

    //监控光驱设备是否被移除
    auto volumeManager = Experimental_Peony::VolumeManager::getInstance();
    connect(volumeManager,&Experimental_Peony::VolumeManager::volumeRemove,this,&UdfFormatDialog::slot_volumeDeviceRemove);
}

UdfFormatDialog::~UdfFormatDialog()
{
    FormatDlgCreateDelegate::getInstance()->removeFromUdfWrapperMap(this->m_uri);
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

void UdfFormatDialog::slot_udfFormat()
{
    setButtonState(true);
    if (!udfFormatEnsureMsgBox()) {
        setButtonState(false);
        return;
    }

    if (m_discNameEdit->text().isEmpty()) {
        QMessageBox::warning(nullptr, tr("Warning"), tr("The disc name cannot be set to empty, please re-enter it!"), QMessageBox::Ok);
        setButtonState(false);
        return;
    }

    m_progress->setVisible(true);
    m_progress->setMaximum(0);
    b_canClose = false;

    /* udf格式化线程 */
    m_thread = new QThread();
    m_discControl->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_discControl, [=](){
         m_discControl->formatUdfSync(m_discNameEdit->text());
    }, Qt::UniqueConnection);
    //connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater, Qt::UniqueConnection);
    //connect(m_thread, &QThread::finished, m_discControl, &DiscControl::deleteLater, Qt::UniqueConnection);
    connect(m_thread, &QThread::finished, this, &UdfFormatDialog::slot_FreeMemory, Qt::UniqueConnection);
    m_thread->start();
}

void UdfFormatDialog::slot_udfCancel()
{
    this->close();
}

void UdfFormatDialog::slot_formatFinished(bool successful, QString errorInfo)
{
    m_thread->quit();

    b_canClose = true;
    m_progress->setVisible(false);
    setButtonState(false);

    if (successful) {
        QMessageBox::about(this, tr("Format"), tr("Format operation has been finished successfully."));

    } else {
        b_failed = true;
        if(errorInfo.isEmpty())
            errorInfo = tr("Sorry, the format operation is failed!");
        QMessageBox::critical(this, tr("Failed"), errorInfo, QMessageBox::Ok);
    }

    this->close();
}

void UdfFormatDialog::slot_volumeDeviceRemove(const QString dev)
{
    qDebug() << __func__ << __LINE__ << QString("[%1] device has been removed").arg(dev);
    QMutexLocker locker(&m_mutex);
    if (m_check) {
        qDebug() << __LINE__ << "m_check =  " << m_check;
        return ;
    }
    if (dev == m_discControl->discDevice()) {
        qDebug() << __func__ << __LINE__ << QString("[%1] prepare to kill the formatting process").arg(dev);
        m_discControl->setRemoved(true);
        m_discControl->killFormatProcess();
    }
}

void UdfFormatDialog::slot_FreeMemory()
{
    qDebug() << __LINE__ << "UDF format thread Finshed";
    QMutexLocker locker(&m_mutex);
    this->m_check = true;
    this->m_discControl->deleteLater();  // 这个在前
    this->m_thread->deleteLater(); // 这个在后
}


void UdfFormatDialog::closeEvent(QCloseEvent *e)
{
    if (!b_canClose) {
        QMessageBox::warning(nullptr, tr("Formatting. Do not close this window"), tr("Formatting. Do not close this window"), QMessageBox::Ok);
        e->ignore();
        return;
    }
    FormatDlgCreateDelegate::getInstance()->removeFromUdfWrapperMap(this->m_uri);
}

bool UdfFormatDialog::udfFormatEnsureMsgBox()
{
    QMessageBox ensureMsgBox(this);
    ensureMsgBox.setText(tr("Formatting this disc will erase all data on it. Please backup all retained data before formatting. Do you want to continue ?"));
    ensureMsgBox.setWindowTitle(tr("Format"));

    QPushButton* okBtn = ensureMsgBox.addButton(tr("Begin Format"), QMessageBox::YesRole);
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

void UdfFormatDialog::setButtonState(bool state)
{
    m_okBtn->setDisabled(state);
    m_cancelBtn->setDisabled(state);
    m_discNameEdit->setDisabled(state);
}

#endif;
