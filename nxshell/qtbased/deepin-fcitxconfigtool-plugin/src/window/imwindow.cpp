// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imwindow.h"
#include "imaddwindow.h"
#include "imsettingwindow.h"
#include "immodel/immodel.h"
#include "publisher/publisherdef.h"
#include "fcitxInterface/config.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTranslator>
#include <libintl.h>
#include <QToolButton>
#include <QComboBox>
#include <QAccessible>
#include "widgets/accessiblewidget.h"
#include "widgets/titlelabel.h"
#include <DWidget>

// 接口工厂
QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
#if !defined(CMAKE_SAFETYTEST_ARG_ON)
    if (object && object->isWidgetType()) {
        if (classname == "QLabel")
            interface = new AccessibleLabel(qobject_cast<QLabel *>(object));

        if (classname == "QPushButton")
            interface = new AccessiblePButton(qobject_cast<QPushButton *>(object));

        if (classname == "QToolButton")
            interface = new AccessibleTButton(qobject_cast<QToolButton *>(object));

        if (classname == "QComboBox")
            interface = new AccessibleComboBox(qobject_cast<QComboBox *>(object));

        if (classname == "FcitxKeyLabelWidget")
            interface = new AccessibleKeyLabelWidget(qobject_cast<FcitxKeyLabelWidget *>(object));

        if (classname == "FcitxKeySettingsItem")
            interface = new AccessibleKeySettingsItem(qobject_cast<FcitxKeySettingsItem *>(object));

        if (classname == "FcitxComBoboxSettingsItem")
            interface = new AccessibleComBoboxSettingsItem(qobject_cast<FcitxComBoboxSettingsItem *>(object));

        if (classname == "DFloatingButton")
            interface = new AccessibleDFloatingButton(qobject_cast<DFloatingButton *>(object));

        if (classname == "DSearchEdit")
            interface = new AccessibleDSearchEdit(qobject_cast<DSearchEdit *>(object));

        if (classname == "DCommandLinkButton")
            interface = new AccessibleDCommandLinkButton(qobject_cast<DCommandLinkButton *>(object));

        if (classname == "FcitxTitleLabel")
            interface = new AccessibleTitleLabel(qobject_cast<FcitxTitleLabel *>(object));
    }
#endif
    return interface;
}

IMWindow::IMWindow(QWidget *parent)
    : DWidget(parent)
{
    initFcitxInterface();
    initUI();
    initConnect();
}

IMWindow::~IMWindow()
{
    IMModel::instance()->deleteIMModel();
    DeleteObject_Null(m_stackedWidget);
    DeleteObject_Null(m_pLayout);
#if !defined(CMAKE_SAFETYTEST_ARG_ON)
    QAccessible::removeFactory(accessibleFactory);
#endif
}

void IMWindow::initFcitxInterface()
{
    bindtextdomain("fcitx", LOCALEDIR);
    bind_textdomain_codeset("fcitx", "UTF-8");
    FcitxLogSetLevel(FCITX_NONE);
    FcitxQtInputMethodItem::registerMetaType();
    FcitxQtKeyboardLayout::registerMetaType();
    if (!Fcitx::Global::instance()->inputMethodProxy()) {
        QProcess::startDetached("fcitx -r");
    }
    // 安装工厂
#if !defined(CMAKE_SAFETYTEST_ARG_ON)
    QAccessible::installFactory(accessibleFactory);
#endif
}

void IMWindow::initUI()
{
    m_stackedWidget = new QStackedWidget(this);
    m_settingWindow = new IMSettingWindow(this);
    connect(m_settingWindow, &IMSettingWindow::requestNextPage, this, &IMWindow::requestNextPage);
    m_addWindow = new IMAddWindow(this);
    m_addWindow->setAccessibleName("addWindow");
    m_stackedWidget->addWidget(m_settingWindow);
    m_stackedWidget->addWidget(m_addWindow);
    m_stackedWidget->setCurrentIndex(0);
    //界面布局
    m_pLayout = new QVBoxLayout(this);
    m_pLayout->addWidget(m_stackedWidget);
    m_pLayout->setMargin(0);
    m_pLayout->setSpacing(0);
}

void IMWindow::initConnect()
{
    auto func = [ = ]() {
        m_stackedWidget->setCurrentIndex(PopIMSettingWindow);
        m_settingWindow->updateUI();
    };
    connect(m_addWindow, &IMAddWindow::popSettingsWindow, func);
    connect(m_settingWindow, &IMSettingWindow::availWidgetAdd, m_addWindow, &IMAddWindow::pushItemAvailwidget);
    connect(m_settingWindow, &IMSettingWindow::popIMAddWindow, [ = ]() {
        m_stackedWidget->setCurrentIndex(PopIMAddWindow);
        m_addWindow->updateUI();
    });

    connect(m_settingWindow, &IMSettingWindow::popShortKeyListWindow, [ = ](const QString & curName, const QStringList & list, QString & name) {
        QString tmpString;
        for (const QString &key : list) {
            tmpString = (key != list.last())? key + "+": key;
        }

        m_stackedWidget->setCurrentIndex(PopShortcutKeyWindow);
        m_settingWindow->updateUI();
    });
}
