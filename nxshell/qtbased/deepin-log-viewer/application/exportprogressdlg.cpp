// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exportprogressdlg.h"
#include "structdef.h"

#include <DLabel>
#include <DApplication>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QAbstractButton>
/**
 * @brief ExportProgressDlg::ExportProgressDlg 构造函数中主要用来初始化界面
 * @param parent 父对象指针
 */
ExportProgressDlg::ExportProgressDlg(DWidget *parent)
    : DDialog(parent)
{
    setIcon(QIcon::fromTheme("deepin-log-viewer"));

    DWidget *pWidget = new DWidget(this);
    QVBoxLayout *pVLayout = new QVBoxLayout();

    DLabel *txtLabel = new DLabel(DApplication::translate("ExportDlg", "Exporting...")); //提示信息
    txtLabel->setAlignment(Qt::AlignCenter);
    //设置字号
    DFontSizeManager::instance()->bind(txtLabel, DFontSizeManager::T6);
    DPalette pa = DApplicationHelper::instance()->palette(txtLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::BrightText));
    DApplicationHelper::instance()->setPalette(txtLabel, pa);
    QVBoxLayout *pVLayouttxt = new QVBoxLayout();
    pVLayouttxt->setContentsMargins(0, 0, 0, 10);
    //pVLayouttxt->addSpacing(10);
    pVLayouttxt->addWidget(txtLabel, Qt::AlignHCenter);
    pVLayout->addLayout(pVLayouttxt);
    //进度条设置初始化
    m_pExportProgressBar = new DProgressBar(this);
    m_pExportProgressBar->setTextVisible(false);
    m_pExportProgressBar->setMaximumHeight(8);
    m_pExportProgressBar->setRange(0, 100);
    pVLayout->addWidget(m_pExportProgressBar);
    pVLayout->setContentsMargins(0, 0, 0, 5);
    pWidget->setLayout(pVLayout);

    addContent(pWidget);

    addButton(DApplication::translate("ExportDlg", "Cancel"), false, DDialog::ButtonNormal);
    setModal(true);
}

/**
 * @brief ExportProgressDlg::setProgressBarRange 设置进度条的最大值最小值
 * @param minValue 进度条最小值
 * @param maxValue 进度条最大值
 */
void ExportProgressDlg::setProgressBarRange(int minValue, int maxValue)
{
    if (m_pExportProgressBar != nullptr) {
        if (maxValue > minValue)
            m_pExportProgressBar->setRange(minValue, maxValue);
    }
}

/**
 * @brief ExportProgressDlg::updateProgressBarValue  给进度条赋值以显示进度
 * @param curValue 当前进度条的值
 */
void ExportProgressDlg::updateProgressBarValue(int curValue)
{
    if (m_pExportProgressBar != nullptr) {
        m_pExportProgressBar->setValue(curValue);
        update();
    }
}

/**
 * @brief ExportProgressDlg::hideEvent 隐藏时清空进度条进度并发出取消信号
 * @param event 隐藏事件
 */
void ExportProgressDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    m_pExportProgressBar->setValue(m_pExportProgressBar->minimum());
    emit sigCloseBtnClicked();
}
