// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progresspage.h"
#include "popupdialog.h"
#include "customwidget.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>


ProgressPage::ProgressPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

ProgressPage::~ProgressPage()
{

}

void ProgressPage::setProgressType(Progress_Type eType)
{
    m_eType = eType;
    if(PT_Rename == m_eType) {
        m_pPauseContinueButton->setVisible(false);
        m_pCancelBtn->setVisible(false);
    } else{
        m_pPauseContinueButton->setVisible(true);
        m_pCancelBtn->setVisible(true);
    }

    if (PT_Compress == m_eType || PT_CompressAdd == m_eType) { // 压缩
        m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + tr("Calculating..."));
    }  else if (PT_Delete == m_eType) { // 删除
        m_pSpeedLbl->setText(tr("Speed", "delete") + ": " + tr("Calculating..."));
    }  else if (PT_Rename == m_eType) { // 重命名
        m_pSpeedLbl->setText(tr("Speed", "rename") + ": " + tr("Calculating..."));
    }  else if (PT_Convert == m_eType) { // 格式转换
        m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + tr("Calculating..."));
    } else if (PT_Comment == m_eType) { // 压缩后添加注释进度
        m_pSpeedLbl->setText("");
    } else { // 解压
        m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + tr("Calculating..."));
    }

    m_pRemainingTimeLbl->setText(tr("Time left") + ": " + tr("Calculating...")); // 剩余时间计算中
}

void ProgressPage::setTotalSize(qint64 qTotalSize)
{
    m_qTotalSize = qTotalSize;
}

void ProgressPage::setArchiveName(const QString &strArchiveName)
{
    m_strArchiveName = strArchiveName;
    QFileInfo fileinfo(strArchiveName);

    QFontMetrics elideFont(m_pArchiveNameLbl->font());
    m_pArchiveNameLbl->setText(elideFont.elidedText(fileinfo.fileName(), Qt::ElideMiddle, 160));     // 设置压缩包名称
    m_pArchiveNameLbl->setToolTip(fileinfo.absoluteFilePath());

    // 设置类型图片
    QFileIconProvider provider;
    QIcon icon = provider.icon(fileinfo);
    m_pPixmapLbl->setPixmap(icon.pixmap(128, 128));
}

QString ProgressPage::archiveName()
{
    return m_strArchiveName;
}

void ProgressPage::setProgress(double dPercent)
{
    // 添加对异常进度的判断处理
    if (dPercent > 100) {
        return;
    }

    int iPercent = qRound(dPercent);
    if (m_iPerent >= iPercent) {
        return ;
    }

    m_iPerent = iPercent;
    m_pProgressBar->setValue(m_iPerent);     // 进度条刷新值
    m_pProgressBar->update();

    // 刷新界面显示
    double dSpeed = 0.0;
    qint64 qRemainingTime = 0;
    calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    m_timer.restart();      // 重启定时器

    // 显示速度和剩余时间
    displaySpeedAndTime(dSpeed, qRemainingTime);
}

void ProgressPage::setCurrentFileName(const QString &strFileName)
{
    QFontMetrics elideFont(m_pFileNameLbl->font());

    if (PT_Compress == m_eType || PT_CompressAdd == m_eType) {   // 压缩和追加压缩
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Compressing") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (PT_Delete == m_eType) {   // 删除
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Deleting") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (PT_Rename == m_eType) {   // 重命名
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Renaming") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (PT_Convert == m_eType) {     // 转换
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Converting") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (PT_Comment == m_eType) {     // 注释进度
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Updating the comment..."), Qt::ElideMiddle, 520));
    } else {    // 解压
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Extracting") + ": " + strFileName, Qt::ElideMiddle, 520));
    }
}

void ProgressPage::resetProgress()
{
    // 修复取消按键默认有焦点效果
    m_pCancelBtn->clearFocus();
    m_pPauseContinueButton->clearFocus();

    // 修复暂停状态返回列表后再切换到进度界面状态混乱
    m_pPauseContinueButton->setText(tr("Pause", "button"));
    m_pPauseContinueButton->setChecked(false);

    // 重置相关参数
    m_pProgressBar->setValue(0);
    if (m_eType == PT_Comment) {
        m_pFileNameLbl->setText(tr("Updating the comment..."));
    } else {
        m_pFileNameLbl->setText(tr("Calculating..."));
    }

    m_timer.elapsed();
    m_iPerent = 0;
    m_qConsumeTime = 0;
}

void ProgressPage::restartTimer()
{
    m_timer.restart();
}

void ProgressPage::setPushButtonCheckable(bool a, bool b)
{
    m_pCancelBtn->setEnabled(a); // 取消按钮
    m_pPauseContinueButton->setEnabled(b); // 暂停继续按钮
}

bool ProgressPage::isPause()
{
    return m_pPauseContinueButton->text() == tr("Continue", "button");
}

void ProgressPage::initUI()
{
    // 初始化控件
    m_pPixmapLbl = new DLabel(this);
    m_pArchiveNameLbl = new DLabel(this);
    m_pProgressBar = new DProgressBar(this);
    m_pFileNameLbl = new DLabel(this);
    m_pSpeedLbl = new DLabel(this);
    m_pRemainingTimeLbl = new DLabel(this);
    m_pCancelBtn = new CustomPushButton(tr("Cancel", "button"), this);
    m_pPauseContinueButton = new CustomSuggestButton(tr("Pause", "button"), this);

    // 初始化压缩包名称样式
    DFontSizeManager::instance()->bind(m_pArchiveNameLbl, DFontSizeManager::T5, QFont::DemiBold);
    m_pArchiveNameLbl->setForegroundRole(DPalette::ToolTipText);

    // 配置进度条
    m_pProgressBar->setRange(0, 100);
    m_pProgressBar->setFixedSize(240, 8);
    m_pProgressBar->setValue(1);
    m_pProgressBar->setOrientation(Qt::Horizontal);  //水平方向
    m_pProgressBar->setAlignment(Qt::AlignVCenter);
    m_pProgressBar->setTextVisible(false);

    // 设置文件名样式
    m_pFileNameLbl->setMaximumWidth(520);
    m_pFileNameLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pFileNameLbl, DFontSizeManager::T8);

    // 设置速度和剩余时间样式
    m_pSpeedLbl->setForegroundRole(DPalette::TextTips);
    m_pRemainingTimeLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pSpeedLbl, DFontSizeManager::T8);
    DFontSizeManager::instance()->bind(m_pRemainingTimeLbl, DFontSizeManager::T8);

    // 设置取消按钮样式
    m_pCancelBtn->setMinimumSize(200, 36);

    // 设置暂停继续按钮样式
    m_pPauseContinueButton->setMinimumSize(200, 36);
    m_pPauseContinueButton->setCheckable(true);

    // 速度和剩余时间布局
    QHBoxLayout *pSpeedLayout = new QHBoxLayout;
    pSpeedLayout->addStretch();
    pSpeedLayout->addWidget(m_pSpeedLbl);
    pSpeedLayout->addSpacing(15);
    pSpeedLayout->addWidget(m_pRemainingTimeLbl);
    pSpeedLayout->addStretch();

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pCancelBtn, 2);
    pBtnLayout->addSpacing(10);
    pBtnLayout->addWidget(m_pPauseContinueButton, 2);
    pBtnLayout->addStretch(1);

    // 主布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->setSpacing(0);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pArchiveNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(25);
    pMainLayout->addWidget(m_pProgressBar, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(10);
    pMainLayout->addWidget(m_pFileNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(-2);
    pMainLayout->addLayout(pSpeedLayout);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setContentsMargins(20, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void ProgressPage::initConnections()
{
    connect(m_pPauseContinueButton, &DSuggestButton::clicked, this, &ProgressPage::slotPauseClicked);
    connect(m_pCancelBtn, &DPushButton::clicked, this, &ProgressPage::slotCancelClicked);
}

void ProgressPage::calSpeedAndRemainingTime(double &dSpeed, qint64 &qRemainingTime)
{
    if (m_qConsumeTime < 0) {
        m_timer.start();
    }

    m_qConsumeTime += m_timer.elapsed();

    if (m_qConsumeTime < 0) {
        dSpeed = 0.0;
        qRemainingTime = 0;
        return;
    }

    // 计算速度
    if (0 == m_qConsumeTime) {
        dSpeed = 0.0; //处理速度
    } else {
        if (PT_Convert == m_eType) {
            dSpeed = 2 * (m_qTotalSize / 1024.0 * m_iPerent / 100) / m_qConsumeTime * 1000;
        } else {
            dSpeed = (m_qTotalSize / 1024.0 * m_iPerent / 100) / m_qConsumeTime * 1000;
        }
    }

    // 计算剩余时间
    double sizeLeft = 0;
    if (PT_Convert == m_eType) {
        sizeLeft = (m_qTotalSize * 2 / 1024.0) * (100 - m_iPerent) / 100; //剩余大小
    } else {
        sizeLeft = (m_qTotalSize / 1024.0) * (100 - m_iPerent) / 100; //剩余大小
    }

    if (dSpeed != 0.0) {
        qRemainingTime = qint64(sizeLeft / dSpeed); //剩余时间
    }

    if (100 != qRemainingTime && 0 == qRemainingTime) {
        qRemainingTime = 1;
    }
}

void ProgressPage::displaySpeedAndTime(double dSpeed, qint64 qRemainingTime)
{
    // 计算剩余需要的小时。
    qint64 hour = qRemainingTime / 3600;
    // 计算剩余的分钟
    qint64 minute = (qRemainingTime - hour * 3600) / 60;
    // 计算剩余的秒数
    qint64 seconds = qRemainingTime - hour * 3600 - minute * 60;
    // 格式化数据
    QString hh = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString mm = QString("%1").arg(minute, 2, 10, QLatin1Char('0'));
    QString ss = QString("%1").arg(seconds, 2, 10, QLatin1Char('0'));

    //add update speed and time label
    // 设置剩余时间
    m_pRemainingTimeLbl->setText(tr("Time left") + ": " + hh + ":" + mm + ":" + ss);
    if (PT_Compress == m_eType || PT_CompressAdd == m_eType) {
        if (dSpeed < 1024) {
            // 速度小于1024k， 显示速度单位为KB/s
            m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
        } else if (dSpeed > 1024 && dSpeed < 1024 * 300) {
            // 速度大于1M/S，且小于300MB/s， 显示速度单位为MB/s
            m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
        } else {
            // 速度大于300MB/s，显示速度为>300MB/s
            m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + ">300MB/s");
        }
    } else if (PT_Delete == m_eType) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "delete") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "delete") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
        }
    } else if (PT_Rename == m_eType) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "rename") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "rename") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
        }
    } else if (PT_UnCompress == m_eType) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
        } else if (dSpeed > 1024 && dSpeed < 1024 * 300) {
            m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + ">300MB/s");
        }
    } else if (PT_Convert == m_eType) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + QString::number(dSpeed, 'f', 2) + "KB/s");
        } else if (dSpeed > 1024 && dSpeed < 1024 * 300) {
            m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/s");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + ">300MB/s");
        }
    } else if (PT_Comment == m_eType) {
        m_pSpeedLbl->setText("");
        m_pRemainingTimeLbl->setText("");
    }
}

void ProgressPage::slotPauseClicked(bool bChecked)
{
    if (bChecked) {
        // 暂停操作
        m_pPauseContinueButton->setText(tr("Continue", "button"));
        emit signalPause();
    } else {
        // 继续操作
        m_pPauseContinueButton->setText(tr("Pause", "button"));
        emit signalContinue();
    }
}

void ProgressPage::slotCancelClicked()
{
    // 若不是暂停状态，先暂停
    bool CheckedFlag = m_pPauseContinueButton->isChecked();
    if (!CheckedFlag) { // 原来是运行状态
        emit m_pPauseContinueButton->clicked(true);
    }

    // 对话框文字描述
    QString strDesText;
    if (PT_Compress == m_eType) {
        strDesText = tr("Are you sure you want to stop the compression?");      // 是否停止压缩
    } else if (PT_UnCompress == m_eType) {
        strDesText = tr("Are you sure you want to stop the decompression?");      // 是否停止解压
    } else if (PT_Delete == m_eType) {
        strDesText = tr("Are you sure you want to stop the deletion?");      // 是否停止删除
    } else if (PT_CompressAdd == m_eType) {
        strDesText = tr("Are you sure you want to stop the compression?");      // 是否停止追加
    } else if (PT_Convert == m_eType) {
        strDesText = tr("Are you sure you want to stop the conversion?");      // 是否停止转换
    }

    // 弹出取消询问对话框
    SimpleQueryDialog dialog(this);
    int iResult = dialog.showDialog(strDesText, tr("Cancel", "button"), DDialog::ButtonNormal, tr("Confirm", "button"), DDialog::ButtonRecommend);
    if (1 == iResult) {
        // 取消操作
        emit signalCancel();
    } else {
        // 恢复原来状态
        if (!CheckedFlag) { // 原来是运行状态
            emit m_pPauseContinueButton->clicked(false);
        }
    }
}
