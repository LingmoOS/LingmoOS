// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepage.h"
#include "common/utils.h"

#include <DApplicationHelper>
#include <DFontSizeManager>

#include <QGridLayout>
#include <QVBoxLayout>

/**
 * @brief HomePage::HomePage
 * @param parent
 */
HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    initUi();
    initConnection();
}

/**
 * @brief HomePage::initUi
 */
void HomePage::initUi()
{
    m_PushButton = new DSuggestButton(
        QString(DApplication::translate("HomePage", "Create Notebook")), this);
    m_PushButton->setFixedSize(QSize(302, 36));
    DFontSizeManager::instance()->bind(m_PushButton, DFontSizeManager::T6);
    m_PushButton->installEventFilter(this);

    m_Image = new VNoteIconButton(this, "home_page_logo.svg");
    m_Image->setIconSize(QSize(128, 128));
    m_Image->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_Image->setFlat(true);

    m_Text = new DLabel(this);
    m_Text->setMinimumSize(QSize(500, 40));
    m_Text->setText(QString(DApplication::translate(
        "HomePage", "Create a notebook to start recording voice and making notes")));
    m_Text->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_Text, DFontSizeManager::T8);
    DPalette pa = DApplicationHelper::instance()->palette(m_Text);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    m_Text->setPalette(pa);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_Image, 1, 1, Qt::AlignCenter);
    layout->addWidget(m_PushButton, 2, 1, Qt::AlignCenter);
    layout->addWidget(m_Text, 3, 1, Qt::AlignCenter);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 0);
    layout->setRowStretch(3, 0);
    layout->setRowStretch(4, 1);
    layout->setContentsMargins(0, 0, 0, 50);

    this->setLayout(layout);
}

/**
 * @brief HomePage::initConnection
 */
void HomePage::initConnection()
{
    connect(m_PushButton, SIGNAL(clicked()), this, SIGNAL(sigAddFolderByInitPage()));
}

/**
 * @brief HomePage::eventFilter
 * @param o
 * @param e
 * @return
 */
bool HomePage::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o)
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Tab) {
            Utils::setTitleBarTabFocus(keyEvent);
            return true;
        } else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            emit sigAddFolderByInitPage();
            return true;
        }
    }
    return false;
}
