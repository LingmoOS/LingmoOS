// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "dtitlebar.h"
#include "dthememanager.h"
#include "dhidpihelper.h"
#include "utils.h"
#include "../3rdparty/core/settings.h"

#include <DWidgetUtil>
#include <DPalette>

#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <QShortcut>
#include <QProcess>
#include <QDesktopWidget>
#include <QApplication>
#include <QPropertyAnimation>

DGUI_USE_NAMESPACE

const QSize STANDARD_SIZE = QSize(344, 545); //标准模式的固定大小
const QSize SCIENTIFIC_MIN_SIZE = QSize(451, 542); //科学模式的最小size
const QSize PROGRAMM_SIZE = QSize(451, 574); //程序员模式固定大小

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    m_settings = DSettingsAlt::instance();
    m_mainLayout = new QStackedLayout();
    m_tbMenu = new DMenu(this);
    QIcon t_icon = QIcon::fromTheme("deepin-calculator");
    titlebar()->setIcon(t_icon);
    titlebar()->setMenu(m_tbMenu);
    titlebar()->setTitle("");

    m_simpleAction = new QAction(tr("Standard"), this);
    m_scAction = new QAction(tr("Scientific"), this);
    m_programmerAction = new QAction(tr("Programmer"), this);

    m_pActionGroup = new QActionGroup(this); //实现互斥checked
    m_pActionGroup->addAction(m_simpleAction);
    m_pActionGroup->addAction(m_scAction);
    m_pActionGroup->addAction(m_programmerAction);
    m_simpleAction->setCheckable(true);
    m_scAction->setCheckable(true);
    m_programmerAction->setCheckable(true);

#ifdef ENABLE_SCIENTIFIC
    m_modeshowmenu = new DMenu(tr("Mode"), this);
    m_tbMenu->addSeparator(); //添加分隔符
    m_modeshowmenu->addAction(m_simpleAction);
    m_modeshowmenu->addAction(m_scAction);
    m_modeshowmenu->addAction(m_programmerAction);
    m_tbMenu->addMenu(m_modeshowmenu);
#endif

    initModule();
    initTheme();

    setWindowTitle(tr("Calculator"));

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MainWindow::initTheme);
    connect(m_simpleAction, &QAction::triggered, this, &MainWindow::switchToSimpleMode);
    connect(m_scAction, &QAction::triggered, this, &MainWindow::switchToScientificMode);
    connect(m_programmerAction, &QAction::triggered, this, &MainWindow::switchToProgrammerMode);
}

MainWindow::~MainWindow()
{
}

void MainWindow::switchModeBack()
{
    int mode = m_settings->getOption("mode").toInt();
    switch(mode){
    case 0:
        switchToScientificMode();
        switchToSimpleMode();
        break;
    case 1:
        switchToSimpleMode();
        switchToScientificMode();
        break;
    case 2:
        switchToSimpleMode();
        switchToProgrammerMode();
        break;
    default:
        switchToScientificMode();
        switchToSimpleMode();
    }
}
void MainWindow::initTheme()
{
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
        DPalette titlePa = titlebar()->palette();
        titlePa.setColor(DPalette::Light, QColor(240, 240, 240));
        titlePa.setColor(DPalette::Dark, QColor(240, 240, 240));
        titlePa.setColor(DPalette::Base, QColor(240, 240, 240));
        titlebar()->setPalette(titlePa);
    } else {
        DPalette titlePa = titlebar()->palette();
        QColor normalbackground = QColor(0, 0, 0);
        normalbackground.setAlphaF(0.1);
        titlePa.setColor(DPalette::Light, normalbackground);
        titlePa.setColor(DPalette::Dark, normalbackground);
        titlePa.setColor(DPalette::Base, normalbackground);
//        titlePa.setColor(DPalette::Light, QColor(37, 37, 37));
//        titlePa.setColor(DPalette::Dark, QColor(37, 37, 37));
//        titlePa.setColor(DPalette::Base, QColor(37, 37, 37));
        titlebar()->setPalette(titlePa);
    }
}

void MainWindow::initModule()
{
    int mode = m_settings->getOption("mode").toInt();
    QWidget *centralWidget = new QWidget(this);

    centralWidget->setLayout(m_mainLayout);
    setCentralWidget(centralWidget);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_isinit = true;
    Settings::instance()->programmerBase = 10;//默认程序员模式的基础进制是10
    switch (mode) {
    case 0:
        m_basicModule = new BasicModule(this);
        m_mainLayout->addWidget(m_basicModule);
        m_simpleAction->setChecked(true);
        switchToSimpleMode();
        break;
    case 1:
        m_scientificModule = new scientificModule(this);
        m_mainLayout->addWidget(m_scientificModule);
        m_scAction->setChecked(true);
        switchToScientificMode();
        break;
    case 2:
        m_programmerModule = new ProgramModule(this);
        m_mainLayout->addWidget(m_programmerModule);
        connect(m_programmerModule, &ProgramModule::activateWindow, [=](){
            titlebar()->setFocus();
        });
        m_programmerAction->setChecked(true);
        switchToProgrammerMode();
        break;
    default:
        m_basicModule = new BasicModule(this);
        m_mainLayout->addWidget(m_basicModule);
        m_simpleAction->setChecked(true);
        switchToSimpleMode();
        break;
    }
    m_isinit = false;
}

void MainWindow::switchToSimpleMode()
{
    if (Settings::instance()->programmerBase != 0)
        programmerOldBase = Settings::instance()->programmerBase;
    Settings::instance()->programmerBase = 0;
    if (!m_basicModule) {
        m_basicModule = new BasicModule(this);
        m_mainLayout->addWidget(m_basicModule);
        emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    }
    if (m_settings->getOption("mode") != 0 || m_isinit) {
        m_settings->setOption("mode", 0);
        m_mainLayout->setCurrentWidget(m_basicModule);
        setFixedSize(STANDARD_SIZE);
    }
}

void MainWindow::switchToScientificMode()
{
    if (Settings::instance()->programmerBase != 0)
        programmerOldBase = Settings::instance()->programmerBase;
    Settings::instance()->programmerBase = 0;
    if (!m_scientificModule) {
        m_scientificModule = new scientificModule(this);
        m_mainLayout->addWidget(m_scientificModule);
        emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    }
    if (m_settings->getOption("mode") != 1 || m_isinit) {
        m_settings->setOption("mode", 1);
        m_mainLayout->setCurrentWidget(m_scientificModule);
        m_scientificModule->checkLineEmpty();
        setFixedSize(SCIENTIFIC_MIN_SIZE);
    }
}

void MainWindow::switchToProgrammerMode()
{
    Settings::instance()->programmerBase = programmerOldBase;
    if (!m_programmerModule) {
        m_programmerModule = new ProgramModule(this);
        m_mainLayout->addWidget(m_programmerModule);
        connect(m_programmerModule, &ProgramModule::activateWindow, [=](){
            titlebar()->setFocus();
        });
        emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    }
    if (m_settings->getOption("mode") != 2 || m_isinit) {
        m_settings->setOption("mode", 2);
        m_mainLayout->setCurrentWidget(m_programmerModule);
        setFixedSize(PROGRAMM_SIZE);
    }
}

/**
 * @brief 焦点不在basicmodul时也触发keypress
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_mainLayout->currentWidget() == m_basicModule)
        m_basicModule->setKeyPress(event);
    else if (m_mainLayout->currentWidget() == m_scientificModule)
        m_scientificModule->setKeyPress(event);
    else if (m_mainLayout->currentWidget() == m_programmerModule) {
        m_programmerModule->setKeyPress(event);
    }
    return;
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    m_settings->setOption("windowX", event->pos().x());
    m_settings->setOption("windowY", event->pos().y());
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    m_settings->setOption("windowWidth", event->size().width());
    m_settings->setOption("windowHeight", event->size().height());
    DMainWindow::resizeEvent(event);
}

