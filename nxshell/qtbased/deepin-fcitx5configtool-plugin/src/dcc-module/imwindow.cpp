// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imwindow.h"

#include "imsettingwindow.h"

#include <dbusprovider.h>
#include <libintl.h>

#include <DWidget>

#include <QAccessible>
#include <QAccessibleInterface>
#include <QComboBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QTranslator>
#include <QVBoxLayout>

IMWindow::IMWindow(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

IMWindow::~IMWindow() = default;

void IMWindow::initUI()
{
    auto *dbus = new fcitx::kcm::DBusProvider(this);
    m_stackedWidget = new QStackedWidget(this);
    m_settingWindow = new IMSettingWindow(dbus, this);
    qInfo() << "load setting window";
    m_stackedWidget->addWidget(m_settingWindow);
    m_stackedWidget->setCurrentIndex(0);
    // 界面布局
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->addWidget(m_stackedWidget);
    m_pLayout->setMargin(0);
    m_pLayout->setSpacing(0);
}
