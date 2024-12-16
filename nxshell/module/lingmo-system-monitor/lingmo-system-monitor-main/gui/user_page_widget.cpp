/*
* Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd
*
* Author:      lishiqi <lishiqi@uniontech.com>
* Maintainer:  lishiqi <lishiqi@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ddlog.h"
#include "user_page_widget.h"
#include "process_table_view.h"
#include "model/process_table_model.h"
#include <QLayout>
#include <DApplicationHelper>
#include <DApplication>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE
using namespace common::format;
using namespace DDLog;
UserPageWidget::UserPageWidget(DWidget *parent)
    : DFrame(parent)
{
    struct passwd *pws;
    pws = getpwuid(getuid());
    m_currentUser = QString(pws->pw_name);
    qCInfo(app) << "UserPageWidget Construction:"
                << "currentuser:" << m_currentUser;
    initUI();
    initConnections();
}

// destructor
UserPageWidget::~UserPageWidget() {}

// initialize ui components
void UserPageWidget::initUI()
{
    m_dAppHelper = DApplicationHelper::instance();
    // process table view instance
    m_procTable = new ProcessTableView(this, m_currentUser);
    m_procTable->switchDisplayMode(kNoFilter);

    // content margin
    int margin = 10;

    //left user info instance
    m_accountListWidget = new AccountsWidget(this);
    m_accountListWidget->setAutoFillBackground(false);
    m_accountListWidget->setContentsMargins(0, 0, 0, 0);
    m_accountListWidget->setFixedWidth(250);

    /***********************************************usage Labels**************************************************/

    QWidget *usageSummaryWidget = new QWidget(this);
    QHBoxLayout *usageLayout = new QHBoxLayout(usageSummaryWidget);

    usageLayout->addWidget(m_CPUUsageSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->setContentsMargins(0, 0, 0, 0);

    // top user resource usage summary label instance

    m_CPULabel = new DLabel(usageSummaryWidget);
    m_MemeryLabel = new DLabel(usageSummaryWidget);
    m_SMemLabel = new DLabel(usageSummaryWidget);
    m_VMemLabel = new DLabel(usageSummaryWidget);
    m_UploadLabel = new DLabel(usageSummaryWidget);
    m_DownLoadLabel = new DLabel(usageSummaryWidget);
    m_DiskReadLabel = new DLabel(usageSummaryWidget);
    m_DiskWriteLabel = new DLabel(usageSummaryWidget);

    m_CPULabel->setText(QApplication::translate("Process.Table.Header", kProcessCPU));
    m_MemeryLabel->setText(QApplication::translate("Process.Table.Header", kProcessMemory));
    m_SMemLabel->setText(QApplication::translate("Process.Table.Header", kProcessShareMemory));
    m_VMemLabel->setText(QApplication::translate("Process.Table.Header", kProcessVtrMemory));
    m_UploadLabel->setText(QApplication::translate("Process.Table.Header", kProcessUpload));
    m_DownLoadLabel->setText(QApplication::translate("Process.Table.Header", kProcessDownload));
    m_DiskReadLabel->setText(QApplication::translate("Process.Table.Header", kProcessDiskRead));
    m_DiskWriteLabel->setText(QApplication::translate("Process.Table.Header", kProcessDiskWrite));

    m_CPULabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessCPU));
    m_MemeryLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessMemory));
    m_SMemLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessShareMemory));
    m_VMemLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessVtrMemory));
    m_UploadLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessUpload));
    m_DownLoadLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessDownload));
    m_DiskReadLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessDiskRead));
    m_DiskWriteLabel->setToolTip(QApplication::translate("Process.Table.Header", kProcessDiskWrite));

    m_CPUUsageSummary = new DLabel(usageSummaryWidget);
    m_MemeryUsageSummary = new DLabel(usageSummaryWidget);
    m_SMemUsageSummary = new DLabel(usageSummaryWidget);
    m_VMemUsageSummary = new DLabel(usageSummaryWidget);
    m_UploadSummary = new DLabel(usageSummaryWidget);
    m_DownLoadSummary = new DLabel(usageSummaryWidget);
    m_DiskReadSummary = new DLabel(usageSummaryWidget);
    m_DiskWriteSummary = new DLabel(usageSummaryWidget);

    setLabelFormat(m_CPULabel, DPalette::TextTitle);
    setLabelFormat(m_MemeryLabel, DPalette::TextTitle);
    setLabelFormat(m_SMemLabel, DPalette::TextTitle);
    setLabelFormat(m_VMemLabel, DPalette::TextTitle);
    setLabelFormat(m_UploadLabel, DPalette::TextTitle);
    setLabelFormat(m_DownLoadLabel, DPalette::TextTitle);
    setLabelFormat(m_DiskReadLabel, DPalette::TextTitle);
    setLabelFormat(m_DiskWriteLabel, DPalette::TextTitle);
    setLabelFormat(m_CPUUsageSummary, DPalette::TextTips);
    setLabelFormat(m_MemeryUsageSummary, DPalette::TextTips);
    setLabelFormat(m_SMemUsageSummary, DPalette::TextTips);
    setLabelFormat(m_VMemUsageSummary, DPalette::TextTips);
    setLabelFormat(m_UploadSummary, DPalette::TextTips);
    setLabelFormat(m_DownLoadSummary, DPalette::TextTips);
    setLabelFormat(m_DiskReadSummary, DPalette::TextTips);
    setLabelFormat(m_DiskWriteSummary, DPalette::TextTips);

    usageLayout->addWidget(m_CPULabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_CPUUsageSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_MemeryLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_MemeryUsageSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_SMemLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_SMemUsageSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_VMemLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_VMemUsageSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_UploadLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_UploadSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_DownLoadLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_DownLoadSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_DiskReadLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_DiskReadSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addSpacing(5);
    usageLayout->addWidget(m_DiskWriteLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addWidget(m_DiskWriteSummary, 0, Qt::AlignLeft | Qt::AlignVCenter);
    usageLayout->addStretch();
    usageSummaryWidget->setLayout(usageLayout);
    /***********************************************usage Labels**************************************************/

    QWidget *userInfoWideget = new QWidget(this);

    auto *contentlayout = new QVBoxLayout(userInfoWideget);
    contentlayout->setSpacing(margin);
    contentlayout->addWidget(usageSummaryWidget);
    contentlayout->addWidget(m_procTable, 1);
    userInfoWideget->setLayout(contentlayout);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(10);
    layout->setSpacing(0);
    layout->addWidget(m_accountListWidget);
    layout->addWidget(userInfoWideget);
    setLayout(layout);
    setAutoFillBackground(false);
    //初始化汇总数据显示状态
    onHeaderChanged();
}
void UserPageWidget::initConnections()
{
    connect(m_accountListWidget, &AccountsWidget::signalCurrentChanged, this, &UserPageWidget::onUserChanged);
    connect(m_procTable, &ProcessTableView::signalModelUpdated, this, &UserPageWidget::onTextContentChanged);
    connect(m_dAppHelper, &DApplicationHelper::themeTypeChanged, this, &UserPageWidget::onThemeChanged);
    connect(m_procTable, &ProcessTableView::signalHeadchanged, this, &UserPageWidget::onHeaderChanged);
}

// paint event handler
void UserPageWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QPainterPath path;
    path.addRect(QRectF(rect()));
    painter.setOpacity(1);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QColor bgColor = palette.color(DPalette::Background);
}
void UserPageWidget::onUserChanged()
{
    QString userName = m_accountListWidget->getCurrentItemUserName();
    qCInfo(app) << userName << "user changed";
    m_procTable->setUserModeName(userName);
    onTextContentChanged();
    update();
}

void UserPageWidget::onTextContentChanged()
{

    m_CPUUsageSummary->setText(QString("%1%").arg(m_procTable->getUserCPUUsage(), 0, 'f', 1));

    m_MemeryUsageSummary->setText(formatUnit_memory_disk(m_procTable->getUserMemUsage(), KB));

    m_SMemUsageSummary->setText(formatUnit_memory_disk(m_procTable->getUserSMemUsage(), KB));

    m_VMemUsageSummary->setText(formatUnit_memory_disk(m_procTable->getUserVMemUsage(), KB));

    m_UploadSummary->setText(formatUnit_net(8 * m_procTable->getUserUpload(), B, 1, true));

    m_DownLoadSummary->setText(formatUnit_net(8 * m_procTable->getUserDownload(), B, 1, true));

    m_DiskReadSummary->setText(formatUnit_memory_disk(m_procTable->getUserDiskRead(), B, 1, true));

    m_DiskWriteSummary->setText(formatUnit_memory_disk(m_procTable->getUserDiskWrite(), B, 1, true));

    m_CPUUsageSummary->setToolTip(QString("%1%").arg(m_procTable->getUserCPUUsage(), 0, 'f', 1));

    m_MemeryUsageSummary->setToolTip(formatUnit_memory_disk(m_procTable->getUserMemUsage(), KB));

    m_SMemUsageSummary->setToolTip(formatUnit_memory_disk(m_procTable->getUserSMemUsage(), KB));

    m_VMemUsageSummary->setToolTip(formatUnit_memory_disk(m_procTable->getUserVMemUsage(), KB));

    m_UploadSummary->setToolTip(formatUnit_net(8 * m_procTable->getUserUpload(), B, 1, true));

    m_DownLoadSummary->setToolTip(formatUnit_net(8 * m_procTable->getUserDownload(), B, 1, true));

    m_DiskReadSummary->setToolTip(formatUnit_memory_disk(m_procTable->getUserDiskRead(), B, 1, true));

    m_DiskWriteSummary->setToolTip(formatUnit_memory_disk(m_procTable->getUserDiskWrite(), B, 1, true));

    update();
}

void UserPageWidget::onHeaderChanged()
{
    m_CPULabel->setVisible(!m_procTable->isColumnHidden(1));
    m_CPUUsageSummary->setVisible(!m_procTable->isColumnHidden(1));

    m_MemeryLabel->setVisible(!m_procTable->isColumnHidden(3));
    m_MemeryUsageSummary->setVisible(!m_procTable->isColumnHidden(3));

    m_SMemLabel->setVisible(!m_procTable->isColumnHidden(4));
    m_SMemUsageSummary->setVisible(!m_procTable->isColumnHidden(4));

    m_VMemLabel->setVisible(!m_procTable->isColumnHidden(5));
    m_VMemUsageSummary->setVisible(!m_procTable->isColumnHidden(5));

    m_UploadLabel->setVisible(!m_procTable->isColumnHidden(6));
    m_UploadSummary->setVisible(!m_procTable->isColumnHidden(6));

    m_DownLoadLabel->setVisible(!m_procTable->isColumnHidden(7));
    m_DownLoadSummary->setVisible(!m_procTable->isColumnHidden(7));

    m_DiskReadLabel->setVisible(!m_procTable->isColumnHidden(8));
    m_DiskReadSummary->setVisible(!m_procTable->isColumnHidden(8));

    m_DiskWriteLabel->setVisible(!m_procTable->isColumnHidden(9));
    m_DiskWriteSummary->setVisible(!m_procTable->isColumnHidden(9));

    update();
}

void UserPageWidget::setLabelFormat(DLabel *label, DPalette::ColorType corlorType)
{

    auto palette = m_dAppHelper->applicationPalette();

    DFontSizeManager::instance()->bind(label, DFontSizeManager::T7, QFont::Medium);
    // text aligment & elide mode
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setElideMode(Qt::ElideRight);
    // change text color to text tips style
    auto pa = DApplicationHelper::instance()->palette(label);
    palette.setColor(DPalette::Text, palette.color(corlorType));
    label->setPalette(palette);
    label->adjustSize();
}

void UserPageWidget::onThemeChanged()
{
    if (m_CPULabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_CPULabel->setPalette(palette);
    }
    if (m_MemeryLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_MemeryLabel->setPalette(palette);
    }
    if (m_SMemLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_SMemLabel->setPalette(palette);
    }
    if (m_VMemLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_VMemLabel->setPalette(palette);
    }
    if (m_UploadLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_UploadLabel->setPalette(palette);
    }
    if (m_DownLoadLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_DownLoadLabel->setPalette(palette);
    }
    if (m_DiskReadLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_DiskReadLabel->setPalette(palette);
    }
    if (m_DiskWriteLabel) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTitle));
        m_DiskWriteLabel->setPalette(palette);
    }

    if (m_CPUUsageSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_CPUUsageSummary->setPalette(palette);
    }
    if (m_MemeryUsageSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_MemeryUsageSummary->setPalette(palette);
    }
    if (m_SMemUsageSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_SMemUsageSummary->setPalette(palette);
    }
    if (m_VMemUsageSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_VMemUsageSummary->setPalette(palette);
    }
    if (m_UploadSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_UploadSummary->setPalette(palette);
    }
    if (m_DownLoadSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_DownLoadSummary->setPalette(palette);
    }
    if (m_DiskReadSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_DiskReadSummary->setPalette(palette);
    }
    if (m_DiskWriteSummary) {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        palette.setColor(DPalette::Text, palette.color(DPalette::TextTips));
        m_DiskWriteSummary->setPalette(palette);
    }
}
