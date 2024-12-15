// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DMainWindow>
#include <DTitlebar>

#include <QMimeData>

#include "mainwindow.h"
#include "../core/constants.h"
#include "../core/dbusworker.h"
#include "../core/utils.h"
#include "../widgets/alertdialog.h"
#include "../widgets/backgroundframe.h"
#include "initialwidget.h"
#include "upgrade_check/upgradecheckwidget.h"
#include "image_preparation/imagepreparationwidget.h"
#include "upgrade_progress/systemupgradewidget.h"
#include "migrate_apps/migratewidget.h"

DTK_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

MainWindow *MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_askForClose(true)
    , m_backgroundFrame(new BackgroundFrame(this, 8))
    , m_centerWidget(new QWidget(this))
    , m_dbusWorker(DBusWorker::getInstance(this))
    , m_stackedlayout(new QStackedLayout(this))
    , m_initialWidget(new InitialWidget(this))
    , m_upgradeCheckWidget(new UpgradeCheckWidget(this))
    , m_imagePreparationWidget(new ImagePreparationWidget(this))
    , m_systemUpgradeWidget(new SystemUpgradeWidget(this))
    , m_migrateWidget(new MigrateWidget(this))
{
    setAcceptDrops(true);
    initUI();
    initConnections();
}

MainWindow* MainWindow::getInstance(QWidget *parent)
{
    return m_instance = (m_instance ? m_instance : new MainWindow(parent));
}

void MainWindow::initUI()
{
    QHBoxLayout *frameLayout = new QHBoxLayout;
    setFixedSize(MAIN_WINDOW_W, MAIN_WINDOW_H);

    m_stackedlayout->addWidget(m_initialWidget);
    m_stackedlayout->addWidget(m_upgradeCheckWidget);
    m_stackedlayout->addWidget(m_imagePreparationWidget);
    m_stackedlayout->addWidget(m_systemUpgradeWidget);
    m_stackedlayout->addWidget(m_migrateWidget);

    qDebug() << "background frame set";
    if (isDarkMode())
    {
        m_backgroundFrame->setBrush(QBrush(QColor("#232323")));
    }
    else
    {
        m_backgroundFrame->setBrush(QBrush(QColor("#ffffff")));
    }
    m_backgroundFrame->setLayout(m_stackedlayout);
    frameLayout->addWidget(m_backgroundFrame);

    m_centerWidget->setLayout(frameLayout);
    setCentralWidget(m_centerWidget);

    // Set icon for window
    titlebar()->setIcon(QIcon(":icons/upgrade-tool.svg"));

}

void MainWindow::initConnections()
{
    connect(m_upgradeCheckWidget, &UpgradeCheckWidget::previousStage, this, &MainWindow::goToInitWidget);
    connect(m_initialWidget, &InitialWidget::upgradeCheckButtonClicked, this, &MainWindow::goToMainContentWidget);
    connect(m_upgradeCheckWidget, &UpgradeCheckWidget::nextStage, this, [this]() {
        m_imagePreparationWidget->checkNetwork();
        this->m_stackedlayout->setCurrentIndex(2);
    });
    connect(m_imagePreparationWidget, &ImagePreparationWidget::StartUpgrade, [this] (QString isoPath) {
        this->m_stackedlayout->setCurrentWidget(m_systemUpgradeWidget);
        emit m_systemUpgradeWidget->start(isoPath);
    });
    // Light/Dark theme switch
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType themeType) {
        if (themeType == DGuiApplicationHelper::DarkType)
        {
            m_backgroundFrame->setBrush(QBrush(QColor("#232323")));
        }
        else
        {
            m_backgroundFrame->setBrush(QBrush(QColor("#ffffff")));
        }
    });
}

// Pop up a dialog when closing main window.
void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_askForClose)
    {
        AlertDialog dlg(this, tr("Do you want to exit the System Upgrade Tool?"), tr("Exit"), DDialog::ButtonType::ButtonWarning);

        if (dlg.exec() == DDialog::Accepted) {
            m_dbusWorker->StopUpgrade();
            event->accept();
        } else {
            event->ignore();
        }
    }
    else
    {
        event->accept();
    }
}

void MainWindow::goToInitWidget()
{
    m_stackedlayout->setCurrentIndex(0);
}

void MainWindow::goToMainContentWidget()
{
    m_stackedlayout->setCurrentIndex(1);
    Q_EMIT m_dbusWorker->StartUpgradeCheck();
}

void MainWindow::showFailedWindow(QString msg)
{
    emit m_dbusWorker->error(tr("Upgrade failed"), msg);
    m_stackedlayout->setCurrentWidget(m_systemUpgradeWidget);
}

void MainWindow::startMigration()
{
    emit m_migrateWidget->start();
    m_stackedlayout->setCurrentWidget(m_migrateWidget);
}
