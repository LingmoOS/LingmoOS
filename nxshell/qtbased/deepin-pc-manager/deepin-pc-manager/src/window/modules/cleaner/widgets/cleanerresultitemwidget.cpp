// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleanerresultitemwidget.h"

#include "window/modules/common/common.h"
#include "window/modules/common/compixmap.h"

#include <DCheckBox>
#include <DFontSizeManager>
#include <DSpinner>
#include <DTipLabel>

#include <QEvent>
#include <QHBoxLayout>
#include <QThread>

#define ROOT_NODE_WIDTH 734
#define ITEM_INDENTATION 16
#define FATHER_NODE_WIDTH (ROOT_NODE_WIDTH - ITEM_INDENTATION)
#define CHILD_NODE_WIDTH (FATHER_NODE_WIDTH - ITEM_INDENTATION)

CleanerResultItemWidget::CleanerResultItemWidget(QWidget *parent)
    : DFrame(parent)
    , m_isRootItem(false)
    , m_checkBox(nullptr)
    , m_iconLable(nullptr)
    , m_headLabel(nullptr)
    , m_headTipLabel(nullptr)
    , m_tailLabel(nullptr)
    , m_usageLabel(nullptr)
    , m_spinner(nullptr)
    , m_contentFrame(nullptr)
    , m_root(nullptr)
    , m_isWorking(false)
{
    this->setFixedHeight(46);               // 单个项目高度固定
    this->setFixedWidth(FATHER_NODE_WIDTH); // 单个项目宽度固定,避免在不同位置产生变化
    this->setBackgroundRole(DPalette::NoType);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 上下需要留白，模拟窗口内空隙
    setLayout(mainLayout);

    m_contentFrame = new DFrame(this);
    m_contentFrame->setObjectName("contentFrame");
    m_contentFrame->setLineWidth(0);
    m_contentFrame->setFixedHeight(36);
    // 背景，假如是根检查项，将背景重设为NoType
    m_contentFrame->setBackgroundRole(DPalette::ItemBackground);
    mainLayout->addWidget(m_contentFrame);

    QHBoxLayout *firstLayerLayout = new QHBoxLayout;
    firstLayerLayout->setContentsMargins(5, 0, 0, 0);
    m_contentFrame->setLayout(firstLayerLayout);

    m_checkBox = new DCheckBox(this);
    // 避免得到焦点
    m_checkBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_checkBox->setFixedSize(QSize(20, 20));
    m_checkBox->setContentsMargins(0, 5, 0, 5);
    // 所有检查项第一次默认选中
    m_checkBox->setChecked(true);
    firstLayerLayout->addWidget(m_checkBox);

    // 用于清理阶段显示清理结果的提示图标
    m_iconLable = new DTipLabel("", this);
    // 根据UI整改,图标设置为16*16
    m_iconLable->setFixedSize(20, 20);
    // 仅在清理后显示
    m_iconLable->hide();
    m_iconLable->setAlignment(Qt::AlignRight);
    firstLayerLayout->addWidget(m_iconLable);

    // 检查项主标题
    m_headLabel = new DLabel(this);
    m_headLabel->setObjectName("headLable");
    m_headLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headLabel->setContentsMargins(5, 0, 0, 0);
    m_headLabel->adjustSize();
    firstLayerLayout->addWidget(m_headLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

    //    QPalette headPal;
    //    headPal.setColor(QPalette::Text, QColor("#001a2e"));
    //    m_headLabel->setPalette(headPal);
    QFont headFont = m_headLabel->font();
    headFont.setWeight(QFont::Medium);
    headFont.setPixelSize(13);
    m_headLabel->setFont(headFont);

    // 通过stretch调整m_headLabel水平位置
    firstLayerLayout->addStretch(1);

    // 检查项副标题
    m_headTipLabel = new DTipLabel("", this);
    m_headTipLabel->setObjectName("headTipLable");
    m_headTipLabel->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
    m_headTipLabel->setContentsMargins(0, 0, 0, 0);
    m_headTipLabel->adjustSize();
    firstLayerLayout->addWidget(m_headTipLabel);

    QPalette tipPal;
    tipPal.setColor(QPalette::Text, QColor("#526A7F"));
    m_headTipLabel->setPalette(tipPal);
    QFont tipFont = m_headTipLabel->font();
    tipFont.setWeight(QFont::Normal);
    tipFont.setPixelSize(12);
    m_headTipLabel->setFont(tipFont);
    DFontSizeManager::instance()->unbind(m_headTipLabel);

    // 通过stretch调整m_tailLabel水平位置
    firstLayerLayout->addStretch(15);

    // 右侧说明部分，记录状态或文件数量信息
    m_tailLabel = new DTipLabel("", this);
    m_tailLabel->setFont(Utils::getFixFontSize(14));
    DFontSizeManager::instance()->unbind(m_tailLabel);

    m_tailLabel->setObjectName("tailLabel");
    m_tailLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    firstLayerLayout->addWidget(m_tailLabel);
    // 仅在根检查项上显示说明
    m_tailLabel->hide();

    // 最右侧，记录本项检查总计文件空间大小
    m_usageLabel = new DLabel(this);
    m_usageLabel->setFont(Utils::getFixFontSize(14));

    m_usageLabel->setObjectName("usageLabel");
    m_usageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_usageLabel->setContentsMargins(0, 0, 5, 0);
    firstLayerLayout->addWidget(m_usageLabel);

    // 仅在根项目上显示，根项目处于扫描状态时显示并旋转
    m_spinner = new DSpinner(this);
    m_spinner->setObjectName("spinner");
    // spinner大小暂时设置为16*16
    m_spinner->setFixedSize(QSize(16, 16));

    // 对于子项，不显示spinner
    m_spinner->stop();
    m_spinner->setVisible(false);
    firstLayerLayout->addWidget(m_spinner);

    // 设置边框的宽度
    setLineWidth(0);

    // 项目选中时，触发上级和下级检查项状态变更
    connect(m_checkBox, &DCheckBox::clicked, this, &CleanerResultItemWidget::childSelected);
}

CleanerResultItemWidget::~CleanerResultItemWidget() { }

// 设置标题
void CleanerResultItemWidget::setHeadText(const QString &text)
{
    m_headLabel->setText(text);
    m_headStr = text;
    resetElidedTextByFontChange();
}

// 本项目内容说明
void CleanerResultItemWidget::setHeadTipText(const QString &text)
{
    m_headTipLabel->setText(text);
    m_headTipStr = text;
}

// 项目右侧提示信息
// 注意tailtext仅对根项目起作用
// 在子项上需要隐藏
void CleanerResultItemWidget::setTailText(const QString &text)
{
    if (m_isRootItem) {
        // 扫描完成，显示内容
        m_tailLabel->setVisible(true);
        m_tailLabel->setText(text);
        m_tailStr = text;
        resetElidedTextByFontChange();
    }
}

// 由上一级或根结点遍历，在根项被勾选或
void CleanerResultItemWidget::setCheckBoxStatus(bool isChecked)
{
    m_checkBox->setChecked(isChecked);
}

// 清理完成时隐藏勾选框
void CleanerResultItemWidget::setCheckBoxVisable(bool isVisable)
{
    m_checkBox->setVisible(isVisable);
}

// 清理完成时显示完成图标
void CleanerResultItemWidget::setIconVisable(bool isVisable)
{
    m_iconLable->setVisible(isVisable);
}

// 清理成功
void CleanerResultItemWidget::setCleanDone()
{
    // 修改勾选状态，通知根项重新计算大小
    m_checkBox->setVisible(false);
    childSelected();

    setIconVisable(true);
    m_usageLabel->setText("");
    m_usageLabel->setVisible(true);

    // 清理成功显示成功图标
    m_iconLable->setPixmap(QIcon::fromTheme(ANTIAV_OPERATION_SUCCESS).pixmap(16, 16));
    m_iconLable->setVisible(true);

    m_tailLabel->setText(tr("Removed"));
    m_tailLabel->show();
}

// 扫描完成时显示占用空间大小
// 所有检查项在扫描完成时均需要
void CleanerResultItemWidget::setUsedSpace(const QString &text)
{
    m_usageLabel->setVisible(true);
    m_usageLabel->setText(text);
    m_usageStr = text;
    resetElidedTextByFontChange();
}

bool CleanerResultItemWidget::getCheckBoxStatus() const
{
    return m_checkBox->isChecked();
}

// 本项作为检查的根项目,修改页面效果
void CleanerResultItemWidget::setAsRoot()
{
    m_isRootItem = true;
    // 根项不需要显示tip
    m_headTipLabel->setVisible(false);

    // root项固定高度减小,以从视觉上减少与其它项目的距离
    this->setFixedHeight(38);
    m_contentFrame->setFixedHeight(38);
    // 根项的frame不需要背景色
    setBackgroundRole(DPalette::NoType);
    // 根项的内容不需要背景色
    m_contentFrame->setBackgroundRole(DPalette::NoType);

    // 与次级项目宽度保持一致性
    this->setFixedWidth(ROOT_NODE_WIDTH);
}

void CleanerResultItemWidget::setAsChild()
{
    // 缩小长度,与父级对齐右侧
    this->setFixedWidth(CHILD_NODE_WIDTH);
}

// 指向根项目的页面，以同步选中、非选中状态
void CleanerResultItemWidget::setRoot(CleanerResultItemWidget *rootItem)
{
    m_root = rootItem;
    if (rootItem != nullptr) {
        m_isRootItem = false;
    }
}

// 控制根项目扫描显示界面样式
// isStarted 为 true时，表示扫描开始
void CleanerResultItemWidget::setWorkStarted(bool isStarted)
{
    if (m_isRootItem) {
        if (isStarted) {
            // 显示spinner并转动
            m_spinner->start();
            m_spinner->show();
            setTailText(tr("Scanning..."));

            // 为根项重置勾选框
            m_checkBox->show();
        } else {
            m_spinner->stop();
            m_spinner->hide();
        }
    }
    if (isStarted) {
        m_checkBox->setDisabled(false);
    }
    m_isWorking = isStarted;
}

void CleanerResultItemWidget::setUncheckable()
{
    if (m_isRootItem) {
        m_checkBox->hide();
    } else {
        m_checkBox->setDisabled(true);
    }

    m_spinner->hide();
    m_iconLable->hide();
}

// 对于根项目，显示扫描总文件数与选中的文件大小
// 对于子项目，只显示文件总大小
void CleanerResultItemWidget::setScanResult(uint account, const QString &filesize)
{
    setTailText(tr("%1 items, total %2, selected:").arg(account).arg(filesize));
    setUsedSpace(filesize);
}

// 接收到检查项发出的prepare信号后
// 初始化界面显示内容
// 将所有相关的内容清空,以清除上一次扫描的结果
// 将相关控件隐藏
void CleanerResultItemWidget::prepare()
{
    m_usageLabel->setText("");
    m_usageLabel->hide();
    m_tailLabel->setText("");
    m_tailLabel->hide();
    m_spinner->stop();
    m_spinner->hide();
    // 上一次可能被设置为已清理的状态
    m_iconLable->hide();
    m_checkBox->setChecked(true);
    m_checkBox->setDisabled(true);
    m_checkBox->show();
}

// 如果作为子项被选中，同步修改根项
void CleanerResultItemWidget::childSelected()
{
    // 正在展示过程中，不响应“取消勾选”
    // 同时重设勾选框状态
    if (m_isWorking) {
        m_checkBox->setChecked(true);
        return;
    }

    if (!m_isRootItem) {
        // 作为子项，通知上一级和下一次变化
        Q_EMIT childClicked(m_checkBox->checkState());
    } else {
        // 作为根结点，设置全选或全反选
        Q_EMIT rootClicked(m_checkBox->checkState());
    }
}

void CleanerResultItemWidget::paintEvent(QPaintEvent *event)
{
    resetElidedTextByFontChange();
    DFrame::paintEvent(event);
}

// 在label被写入字符串或字体变化时，将超出label的文字部分写作为省略号
void CleanerResultItemWidget::resetElidedTextByFontChange()
{
    if (m_isRootItem) {
        // 根项标题部分内容固定
        // 保留左侧标题和右侧数值
        // 中间部分的右侧超出部分省略
        QFontMetrics fontmet(m_usageLabel->font());
        int usageWidth = fontmet.horizontalAdvance(m_usageStr, -1);
        int headerWidth = fontmet.horizontalAdvance(m_headStr, -1);
        int elidedWidth = ROOT_NODE_WIDTH - usageWidth - headerWidth - 40;
        m_tailLabel->setText(fontmet.elidedText(m_tailStr, Qt::ElideRight, elidedWidth));
    } else {
        if (m_headTipLabel->text().isEmpty()) {
            // 主要是针对父项，保持右侧的完全显示
            // 当前字体宽度
            // 标题右侧超出部分省略
            QFontMetrics fontmet(m_usageLabel->font());
            int usageWidth = fontmet.horizontalAdvance(m_usageStr, -1);
            // 扣除20的预留空白
            int elidedWidth = CHILD_NODE_WIDTH - usageWidth - 20;
            m_headLabel->setText(fontmet.elidedText(m_headStr, Qt::ElideRight, elidedWidth));
        } else {
            QFontMetrics usagefontmet(m_usageLabel->font());
            int usageWidth = usagefontmet.horizontalAdvance(m_usageStr, -1);

            QFontMetrics headfontmet(m_headLabel->font());
            int headWidth = headfontmet.horizontalAdvance(m_headStr, -1);

            // 扣除40的空白
            int elidedWidth = CHILD_NODE_WIDTH - headWidth - usageWidth - 40;
            QFontMetrics fontmet(m_headTipLabel->font());
            m_headTipLabel->setText(fontmet.elidedText(m_headTipStr, Qt::ElideRight, elidedWidth));
        }
    }
}
