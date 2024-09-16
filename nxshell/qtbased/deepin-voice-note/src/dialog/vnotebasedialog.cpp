// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialog/vnotebasedialog.h"
#include "globaldef.h"
#include "common/utils.h"

#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QShowEvent>

/**
 * @brief VNoteBaseDialog::VNoteBaseDialog
 * @param parent
 */
VNoteBaseDialog::VNoteBaseDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    initUI();
    InitConnections();
}

/**
 * @brief VNoteBaseDialog::initUI
 */
void VNoteBaseDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(TITLEBAR_H);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme(DEEPIN_VOICE_NOTE).pixmap(QSize(32, 32)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setIconSize(QSize(50, 50));

    m_tileText = new DLabel(this);
    m_tileText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_tileText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_tileText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_tileText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

/**
 * @brief VNoteBaseDialog::InitConnections
 */
void VNoteBaseDialog::InitConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });
}

/**
 * @brief VNoteBaseDialog::setLogoVisable
 * @param visible true logo 可见
 */
void VNoteBaseDialog::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}

/**
 * @brief VNoteBaseDialog::setTitle
 * @param title 标题
 */
void VNoteBaseDialog::setTitle(const QString &title)
{
    if (nullptr != m_tileText) {
        m_tileText->setText(title);
    }
}

/**
 * @brief VNoteBaseDialog::getContentLayout
 * @return 窗口布局
 */
QLayout *VNoteBaseDialog::getContentLayout()
{
    return m_contentLayout;
}

/**
 * @brief VNoteBaseDialog::addContent
 * @param content 添加窗口
 */
void VNoteBaseDialog::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

/**
 * @brief VNoteBaseDialog::setIconPixmap
 * @param iconPixmap
 */
void VNoteBaseDialog::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setPixmap(iconPixmap);
    }
}

/**
 * @brief VNoteBaseDialog::closeEvent
 * @param event
 */
void VNoteBaseDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(QDialog::Rejected);

    Q_EMIT closed();
}

/**
 * @brief VNoteBaseDialog::showEvent
 * @param event
 */
void VNoteBaseDialog::showEvent(QShowEvent *event)
{
    DAbstractDialog::showEvent(event);

    setAttribute(Qt::WA_Resized, false);
    setFocus(Qt::PopupFocusReason);
    if (!testAttribute(Qt::WA_Resized)) {
        QSize size = sizeHint();

        size.setWidth(qMax(size.width(), DEFAULT_WINDOW_W));
        size.setHeight(qMax(size.height(), DEFAULT_WINDOW_H));

        resize(size);
        setAttribute(Qt::WA_Resized, false);
    }
}
