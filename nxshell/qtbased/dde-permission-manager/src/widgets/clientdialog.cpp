// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clientdialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DPushButton>
#include <DLabel>
#include <DFontSizeManager>
#include <DWarningButton>
#include <DSuggestButton>
#include <QApplication>

ClientDialog::ClientDialog(QWidget *parent)
    : DDialog(parent)
{
    QWidget *mainWidget = new QWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainWidget->setLayout(mainLayout);

    // 弹框文字部分
    m_titleLabel = new DLabel();
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setAlignment(Qt::AlignHCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager =  DFontSizeManager::instance();
    fontManager->bind(m_titleLabel, DFontSizeManager::T5, QFont::DemiBold);
    m_messageLabel = new DLabel();
    m_messageLabel->setAlignment(Qt::AlignHCenter);
    fontManager->bind(m_messageLabel, DFontSizeManager::T8, QFont::Normal);
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setMargin(0);
    textLayout->setContentsMargins(30, 0, 30, 0);
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_messageLabel);
    mainLayout->addLayout(textLayout);
    mainLayout->addSpacing(20);


    QHBoxLayout *frameLayout = new QHBoxLayout();
    frameLayout->setMargin(0);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setMargin(0);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addSpacing(30);
    frameLayout->addLayout(leftLayout);

    // 弹框选项部分
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setMargin(0);
    m_buttonLayout->setContentsMargins(20, 0, 20, 20);
    m_buttonLayout->setSpacing(40);
    frameLayout->addLayout(m_buttonLayout);
    // 选项倒计时关闭
    m_clickedButtonIndex = -1;
    m_closeTimer = new QTimer(this);
    m_closeTimer->setInterval(1000);
    m_closeTimer->stop();
    m_defaultButtonCount = 15;
    m_defaultButton = nullptr;
    connect(m_closeTimer, &QTimer::timeout, [=]() {
        if (m_defaultButton == nullptr) {
            qWarning() << "button not exist";
            m_closeTimer->stop();
            return;
        }
        m_defaultButtonCount--;
        m_defaultButton->setText(m_defaultButtonText + closeTimerText(m_defaultButtonCount));
        if (m_defaultButtonCount == -1) {
            m_closeTimer->stop();
            int closeIndex = m_buttonList.indexOf(m_defaultButton);
            m_clickedButtonIndex = closeIndex;
            this->done(closeIndex);
        }
    });

    // 弹框底部
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setMargin(0);
    rightLayout->setContentsMargins(0, 0, 0, 10);
    rightLayout->addStretch();
    m_CloseLabel = new DLabel();
    m_CloseLabel->setText("");
    m_CloseLabel->setFixedWidth(30);
    rightLayout->addWidget(m_CloseLabel);
    frameLayout->addLayout(rightLayout);

    mainLayout->addLayout(frameLayout);
    addContent(mainWidget);

    // 右上角选项
    this->setCloseButtonVisible(false);

    // 左上角icon
    this->setIcon(QIcon::fromTheme("preferences-system"));

}

ClientDialog::~ClientDialog()
{
}

int ClientDialog::exec()
{
    int code = DAbstractDialog::exec();
    qInfo() << "client dialog exec:" << m_clickedButtonIndex;
    return m_clickedButtonIndex >= 0 ? m_clickedButtonIndex : code;
}

int ClientDialog::addButton(const QString &text, bool isDefault, ButtonType type)
{
    int index = buttonCount();

    QAbstractButton *button;

    switch (type) {
    case ButtonWarning:
        button = new DWarningButton(this);
        break;
    case ButtonRecommend:
        button = new DSuggestButton(this);
        break;
    default:
        button = new QPushButton(this);
        break;
    }
    button->setMinimumSize(135, 45);

    button->setText(text);
    button->setAttribute(Qt::WA_NoMousePropagation);
    connect(button, &QAbstractButton::clicked, this, [=](bool){
        QAbstractButton *senderButton = qobject_cast<QAbstractButton *>(sender());
        if (!senderButton) {
            qWarning() << "button of cliented is invalid";
            return;
        }
        int clickIndex = m_buttonList.indexOf(senderButton);
        if (clickIndex < 0) {
            qWarning() << "button of cliented is invalid";
            return;
        }
        m_clickedButtonIndex = clickIndex;
        if (m_closeTimer->isActive()) {
            m_closeTimer->stop();
        }
        this->done(clickIndex);
    });

    if (isDefault) {
        if (m_closeTimer->isActive()) {
            m_closeTimer->stop();
        }
        if (m_defaultButton != nullptr) {
            // 覆盖场景时文案还原
            m_defaultButton->setText(m_defaultButtonText);
        }
        m_defaultButton = button;
        m_defaultButtonText = text;
        m_defaultButtonCount = 15;
        button->setText(text + closeTimerText(m_defaultButtonCount));
        m_closeTimer->start();
    }

    m_buttonList.append(button);
    m_buttonLayout->addWidget(button);

    return index;
}

void ClientDialog::setTitle(const QString &title)
{
    if (m_titleLabel->text() == title) {
        return;
    }
    m_titleLabel->setText(title);
    m_titleLabel->setHidden(title.isEmpty());
}

void ClientDialog::setMessage(const QString &message)
{
    if (m_messageLabel->text() == message) {
        return;
    }
    m_messageLabel->setText(message);
    m_messageLabel->setHidden(message.isEmpty());
}

void ClientDialog::setButtonText(int index, const QString &text)
{
    QAbstractButton *button = m_buttonList.at(index);
    if (button == nullptr) {
        qWarning() << "button not exist";
        return;
    }
    button->setText(text);
}

int ClientDialog::buttonCount()
{
    return m_buttonList.size();
}

void ClientDialog::startBottomCloseTimer()
{
    if (m_CloseLabel == nullptr) {
        qWarning() << "close label not exist.";
        return;
    }
    QTimer *bottomTimer = new QTimer(this);
    bottomTimer->setInterval(1000);
    int num = 15;
    m_CloseLabel->setText(QString::number(num)+"s");
    connect(bottomTimer, &QTimer::timeout, [=]() mutable {
        num--;
        m_CloseLabel->setText(QString::number(num)+"s");
        if (num == -1) {
            bottomTimer->stop();
            this->done(-1);
        }
    });
    bottomTimer->start();
}

QString ClientDialog::closeTimerText(int num)
{
    return "  ("+QString::number(num)+"s)";
}
