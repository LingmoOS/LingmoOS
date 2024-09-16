// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uninstallconfirmpage.h"
#include "utils/utils.h"

#include <QDebug>
#include <QVBoxLayout>

UninstallConfirmPage::UninstallConfirmPage(QWidget *parent)
    : QWidget(parent)
    , m_icon(new DLabel(this))
    , m_tips(new DLabel(this))
    , m_infoWrapperWidget(new QWidget(this))
    , m_infoControl(new InfoControlButton(tr("Show related packages"), tr("Collapse", "button"), this))
    , m_dependsInfomation(new InstallProcessInfoView(440, 190, this))
    , m_cancelBtn(new DPushButton(this))
    , m_confirmBtn(new DPushButton(this))
{
    this->setAcceptDrops(false);
    const QIcon icon = QIcon::fromTheme("application-x-deb");

    // set Icon size and location
    m_icon->setFixedSize(64, 64);
    m_icon->setPixmap(icon.pixmap(64, 64));

    // 自动化测试
    m_icon->setObjectName("UninstallPageIcon");
    m_icon->setAccessibleName("UninstallPageIcon");

    m_tips->setMinimumHeight(120);
    m_tips->setMinimumWidth(440);
    m_tips->setAlignment(Qt::AlignCenter);

    // cancel button settings
    m_cancelBtn->setText(tr("Cancel", "button"));
    m_cancelBtn->setMinimumSize(120, 36);
    m_confirmBtn->setText(tr("Confirm"));
    m_confirmBtn->setMinimumSize(120, 36);

    // 添加确认和返回按钮的焦点策略
    m_confirmBtn->setFocusPolicy(Qt::TabFocus);
    m_cancelBtn->setFocusPolicy(Qt::TabFocus);

    // 添加确认和返回按钮的enter触发
    m_confirmBtn->setAutoDefault(true);
    m_cancelBtn->setAutoDefault(true);

    // When uninstalling dependent packages, if there are prompts for dependent packages.
    m_dependsInfomation->setVisible(false);
    m_dependsInfomation->setAcceptDrops(false);
    m_dependsInfomation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // layout of buttons
    QHBoxLayout *btnsLayout = new QHBoxLayout();
    btnsLayout->setSpacing(0);
    btnsLayout->setContentsMargins(0, 0, 0, 0);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_cancelBtn);
    btnsLayout->addSpacing(20);
    btnsLayout->addWidget(m_confirmBtn);
    btnsLayout->addStretch();

    // Layout of icons and tips
    QVBoxLayout *contentLayout = new QVBoxLayout(this);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addStretch();
    contentLayout->addWidget(m_icon);
    contentLayout->addStretch(0);
    contentLayout->setAlignment(m_icon, Qt::AlignHCenter);
    contentLayout->addWidget(m_tips);
    contentLayout->addStretch();

    m_infoWrapperWidget->setLayout(contentLayout);

    // The details of the uninstall process and the layout of the dependent information.
    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addWidget(m_infoWrapperWidget);
    centralLayout->addWidget(m_infoControl);
    centralLayout->setAlignment(m_infoControl, Qt::AlignHCenter);
    centralLayout->addSpacing(15);
    centralLayout->addWidget(m_dependsInfomation);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    // 卸载页面上边距增加15px  底边距变为20,适应大字体
    centralLayout->setContentsMargins(20, 15, 20, 20);

    // Set font and size
    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);

    Utils::bindFontBySizeAndWeight(m_tips, normalFontFamily, 14, QFont::Normal);
    Utils::bindFontBySizeAndWeight(m_cancelBtn, normalFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_confirmBtn, normalFontFamily, 14, QFont::Medium);
    m_dependsInfomation->setTextFontSize(14, QFont::Normal);
    Utils::bindFontBySizeAndWeight(m_cancelBtn, normalFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_confirmBtn, normalFontFamily, 14, QFont::Medium);

    setLayout(centralLayout);

    connect(m_cancelBtn, &DPushButton::clicked, this, &UninstallConfirmPage::signalUninstallCanceled);
    connect(m_confirmBtn, &DPushButton::clicked, this, &UninstallConfirmPage::signalUninstallAccepted);
    connect(m_infoControl, &InfoControlButton::expand, this, &UninstallConfirmPage::slotShowDetail);
    connect(m_infoControl, &InfoControlButton::shrink, this, &UninstallConfirmPage::slotHideDetail);
}

void UninstallConfirmPage::setPackage(const QString &name)
{
    // add tips
    QString tips = tr("Are you sure you want to uninstall %1?\nAll dependencies will also be removed");
    if (!m_requiredList.isEmpty()) {
        tips = tr("Are you sure you want to uninstall %1?\nThe system or other applications may not work properly");
    }
    const QSize boundingSize = QSize(m_tips->width(), 340);
    m_tips->setText(
        Utils::holdTextInRect(m_tips->font(), tips.arg(name), boundingSize));  // 2020.0210修改中英文状态下描述输出自动换行
}

void UninstallConfirmPage::setRequiredList(const QStringList &requiredList)
{
    // According to the dependency status, it is determined whether there is a package that depends on the current package, and if
    // so, it is prompted.
    m_requiredList = requiredList;
    if (!requiredList.isEmpty()) {
        m_infoControl->setVisible(true);
    } else {
        m_infoControl->setVisible(false);
    }

    m_dependsInfomation->setTextColor(DPalette::TextTitle);
    m_dependsInfomation->appendText(requiredList.join(", "));
}

void UninstallConfirmPage::showEvent(QShowEvent *e)
{
    // 每次展示时设置默认焦点
    m_confirmBtn->setFocus();
    QWidget::showEvent(e);
}

void UninstallConfirmPage::slotShowDetail()
{
    // Show dependency information
    m_infoWrapperWidget->setVisible(false);
    m_dependsInfomation->setVisible(true);
}

void UninstallConfirmPage::slotHideDetail()
{
    // Hide dependency information
    m_infoWrapperWidget->setVisible(true);
    m_dependsInfomation->setVisible(false);
}
