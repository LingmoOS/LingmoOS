// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syscheckresultwidget.h"

#include "systemcheckdefinition.h"

#include <QGridLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

#define SET_ACC_NAME(control, name) SET_ACCESS_NAME_T(control, sysCheckResultWidget, name)

SysCheckResultWidget::SysCheckResultWidget(QWidget *parent)
    : DFrame(parent)
    , m_header(nullptr)
    , m_checkedButton(nullptr)
    , m_fixedButton(nullptr)
    , m_recheckButton(nullptr)

{
    this->setFixedSize(770, 590);
    this->setFocusPolicy(Qt::NoFocus);
    this->setLineWidth(0);
    this->setContentsMargins(0, 10, 0, 10);

    initUI();
    initConnection();
    onCheckFinished();

    // 唯一窗口实例
    this->setAccessibleName("sysCheckResultWidget");
}

void SysCheckResultWidget::onCheckFinished()
{
    hideAll();
    m_checkedButton->show();
    m_recheckButton->show();
    m_checkedButton->setDisabled(false);
    m_recheckButton->setDisabled(false);
}

void SysCheckResultWidget::onFixing()
{
    hideAll();
    m_checkedButton->show();
    m_recheckButton->show();
    m_checkedButton->setDisabled(true);
    m_recheckButton->setDisabled(true);
}

void SysCheckResultWidget::onFixed()
{
    hideAll();
    m_fixedButton->show();
    m_recheckButton->show();
    m_fixedButton->setDisabled(false);
    m_recheckButton->setDisabled(false);
}

void SysCheckResultWidget::setResultModel(const QStandardItemModel &result)
{
    // 对各项设置结果
    for (auto wid : m_itemWidgets) {
        wid->setResult(result);
    }
    resetItemsBackGround();
}

void SysCheckResultWidget::setHeaderContent(int point, int issueCount)
{
    m_header->onPointChanged(point, issueCount);
}

void SysCheckResultWidget::setFastFixValid(bool isValid)
{
    if (isValid) {
        m_checkedButton->show();
        m_fixedButton->hide();
    } else {
        m_fixedButton->show();
        m_checkedButton->hide();
    }
}

void SysCheckResultWidget::resetItemsBackGround()
{
    bool isNeedColored = false;
    for (int index = 0; index < m_itemWidgets.size(); index++) {
        auto wid = m_itemWidgets[index];
        if (!wid->isHidden()) {
            DPalette::ColorType role = isNeedColored ? DPalette::ItemBackground : DPalette::NoType;
            wid->setBackgroundRole(role);
            isNeedColored = !isNeedColored;
        }
    }
}

void SysCheckResultWidget::setAutoStartAppCount(int count)
{
    Q_EMIT autoStartAppCountChanged(count);
}

void SysCheckResultWidget::onFixStarted(SysCheckItemID id)
{
    if (id == SysCheckItemID::Trash) {
        m_currentFixingItems.push_back(id);
        m_recheckButton->setDisabled(true);
        m_checkedButton->setDisabled(true);
    }

    Q_EMIT onFixItemStarted(id);
}

void SysCheckResultWidget::onFixFinished(SysCheckItemID id)
{
    if (id == SysCheckItemID::Trash) {
        if (m_currentFixingItems.contains(id)) {
            m_currentFixingItems.removeAll(id);
        }
        if (!m_currentFixingItems.size()) {
            m_recheckButton->setDisabled(false);
            m_checkedButton->setDisabled(false);
        }
    }
    Q_EMIT onFixItemFinished(id);
}

void SysCheckResultWidget::initUI()
{
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_header = new SysCheckResultHeaderWidget(this);
    m_checkedButton = new DSuggestButton(this);
    SET_ACC_NAME(m_checkedButton, checkedButton);
    m_checkedButton->setFocusPolicy(Qt::NoFocus);
    m_checkedButton->setFixedWidth(150);
    FIXED_FONT_PIXEL_SIZE(m_checkedButton, 14);
    m_checkedButton->setText(kResultWidRightButtonChecked);

    m_fixedButton = new DSuggestButton(this);
    SET_ACC_NAME(m_fixedButton, fixedButton);
    m_fixedButton->setFocusPolicy(Qt::NoFocus);
    m_fixedButton->setFixedWidth(150);
    FIXED_FONT_PIXEL_SIZE(m_fixedButton, 14);
    m_fixedButton->setText(kResultWidRightButtonFixed);

    m_recheckButton = new DPushButton(this);
    SET_ACC_NAME(m_recheckButton, recheckButton);
    m_recheckButton->setFocusPolicy(Qt::NoFocus);
    m_recheckButton->setFixedWidth(150);
    FIXED_FONT_PIXEL_SIZE(m_recheckButton, 14);
    m_recheckButton->setText(kResultWidLeftButtonRecheck);

    createItemWidgets();
    QVBoxLayout *boxLayout = new QVBoxLayout;
    boxLayout->setSpacing(4);

    // 调整检查项的显示顺序
    boxLayout->addWidget(m_itemWidgets[SysCheckItemID::SSH], 0, Qt::AlignTop | Qt::AlignLeft);
    boxLayout->addWidget(m_itemWidgets[SysCheckItemID::AutoStartApp],
                         0,
                         Qt::AlignTop | Qt::AlignLeft);
    boxLayout->addWidget(m_itemWidgets[SysCheckItemID::SystemUpdate],
                         0,
                         Qt::AlignTop | Qt::AlignLeft);
    boxLayout->addWidget(m_itemWidgets[SysCheckItemID::Trash], 0, Qt::AlignTop | Qt::AlignLeft);
    boxLayout->addWidget(m_itemWidgets[SysCheckItemID::Disk], 0, Qt::AlignTop | Qt::AlignLeft);
    boxLayout->addWidget(m_itemWidgets[SysCheckItemID::DevMode], 0, Qt::AlignTop | Qt::AlignLeft);

    boxLayout->addStretch(10);

    FIXED_ROW(0, 146);
    FIXED_ROW(1, 348);
    FIXED_ROW(2, 56);

    FIXED_COL(0, 385);
    FIXED_COL(1, 385);

    mainLayout->addWidget(m_header, 0, 0, 1, -1, Qt::AlignCenter);
    mainLayout->addLayout(boxLayout, 1, 0, 1, -1, Qt::AlignCenter);
    mainLayout->addWidget(m_recheckButton, 2, 0, Qt::AlignRight | Qt::AlignVCenter);
    mainLayout->addWidget(m_checkedButton, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(m_fixedButton, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);

    this->setLayout(mainLayout);
}

void SysCheckResultWidget::initConnection()
{
    connect(m_recheckButton, &QPushButton::clicked, this, &SysCheckResultWidget::requestCheckAgain);
    connect(m_fixedButton, &QPushButton::clicked, this, &SysCheckResultWidget::requestQuit);
    connect(m_checkedButton, &QPushButton::clicked, this, &SysCheckResultWidget::requestFixAll);
}

void SysCheckResultWidget::createItemWidgets()
{
    QStringList accNames = { "sshCheckItem",       "diskCheckItem",    "trashCheckItem",
                             "sysUpdateCheckItem", "devModeCheckItem", "autoStartCheckItem" };
    m_itemWidgets.clear();
    for (int id = SysCheckItemID::SSH; id <= SysCheckItemID::AutoStartApp; id++) {
        SysCheckResultItemWidget *item = new SysCheckResultItemWidget(SysCheckItemID(id), this);

        item->setAccessibleName(accNames[id]);

        connect(item,
                &SysCheckResultItemWidget::requestFixItem,
                this,
                &SysCheckResultWidget::requestFixItem);
        connect(item,
                &SysCheckResultItemWidget::requestSetIgnore,
                this,
                &SysCheckResultWidget::requestSetIgnore);
        connect(item,
                &SysCheckResultItemWidget::widgetHidden,
                this,
                &SysCheckResultWidget::resetItemsBackGround);

        connect(this,
                &SysCheckResultWidget::onFixItemStarted,
                item,
                &SysCheckResultItemWidget::onNotifyFixStarted);
        connect(this,
                &SysCheckResultWidget::onFixItemFinished,
                item,
                &SysCheckResultItemWidget::onNotifyFixFinished);
        connect(this,
                &SysCheckResultWidget::autoStartAppCountChanged,
                item,
                &SysCheckResultItemWidget::onAutoStartAppCountChanged);

        m_itemWidgets.push_back(item);
    }

    resetItemsBackGround();
}

void SysCheckResultWidget::hideAll()
{
    m_fixedButton->hide();
    m_checkedButton->hide();
}
