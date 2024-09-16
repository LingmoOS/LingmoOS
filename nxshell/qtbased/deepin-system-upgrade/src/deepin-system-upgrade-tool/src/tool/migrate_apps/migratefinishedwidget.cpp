// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QIcon>
#include <QFont>
#include <QSpacerItem>

#include "migratefinishedwidget.h"
#include "../mainwindow.h"

MigrateFinishedWidget::MigrateFinishedWidget(QWidget *parent)
    :   QWidget (parent)
    ,   m_iconLabel(new QLabel(this))
    ,   m_titleLabel(new QLabel(this))
    ,   m_confirmButton(new DSuggestButton(this))
    ,   m_mainLayout(new QVBoxLayout(this))
{
    m_iconLabel->setPixmap(QIcon(":/icons/icon_success.svg").pixmap(128, 128));
    m_titleLabel->setForegroundRole(QPalette::Text);
    m_titleLabel->setFont(QFont("SourceHanSansSC", 15, QFont::Bold));
    m_titleLabel->setText(tr("App migration successful"));
    m_confirmButton->setText(tr("OK"));
    m_confirmButton->setFixedSize(250, 36);
    m_mainLayout->addSpacing(48);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacerItem(new QSpacerItem(0, 114514, QSizePolicy::Minimum, QSizePolicy::Maximum));
    m_mainLayout->addWidget(m_confirmButton, 0, Qt::AlignCenter);

    connect(m_confirmButton, &DSuggestButton::clicked, [] {
        MainWindow  *mainWindow = MainWindow::getInstance();
        mainWindow->m_askForClose = false;
        mainWindow->close();
    });
}
