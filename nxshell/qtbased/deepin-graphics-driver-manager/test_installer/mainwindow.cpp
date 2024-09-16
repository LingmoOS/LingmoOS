// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "utils.h"


#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <DTitlebar>
#include <DSvgRenderer>
#include <QJsonDocument>
#include <DApplicationHelper>
#include <QMenuBar>
#include <DFontSizeManager>

const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";


MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent),
    m_rebootDelay(3)
{
    m_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                   GraphicMangerServiceName,
                   GraphicMangerPath,
                   QDBusConnection::systemBus());

    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    m_installStateIcon = new QLabel;

    m_installState = new QLabel;
    m_installState->setAlignment(Qt::AlignHCenter);
    m_installState->setVisible(false);
    m_installState->setWordWrap(true);
    m_installState->setObjectName("InstallTips");
    DFontSizeManager::instance()->bind(m_installState, DFontSizeManager::T6);

    m_warningTips = new QLabel;
    m_warningTips->setAlignment(Qt::AlignHCenter);
    m_warningTips->setVisible(false);
    m_warningTips->setWordWrap(true);
    m_warningTips->setObjectName("Warning");
    //m_warningTips->setFrameShape(QFrame::Box);
    DFontSizeManager::instance()->bind(m_warningTips, DFontSizeManager::T8);



    m_cancelButton = new QPushButton;
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setFixedHeight(38);
    m_cancelButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_cancelButton, DFontSizeManager::T6);

    m_rebootButton = new DSuggestButton;
    m_rebootButton->setText(tr("Reboot"));
    m_rebootButton->setFixedHeight(38);
    m_rebootButton->setVisible(true);
    DFontSizeManager::instance()->bind(m_rebootButton, DFontSizeManager::T6);


    m_waterProgress = new DWaterProgress;
    m_waterProgress->setTextVisible(true);
    m_waterProgress->setFixedSize(100, 100);
    m_waterProgress->setValue(0);

    auto *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_installStateIcon);
    centralLayout->setAlignment(m_installStateIcon, Qt::AlignHCenter);
    centralLayout->addWidget(m_waterProgress);
    centralLayout->setAlignment(m_waterProgress, Qt::AlignHCenter);
    centralLayout->addSpacing(30);
    centralLayout->addWidget(m_installState);
    centralLayout->setAlignment(m_installState, Qt::AlignHCenter);
    centralLayout->addSpacing(10);
    centralLayout->addWidget(m_warningTips);
    //centralLayout->setAlignment(m_warningTips, Qt::AlignHCenter);
    centralLayout->addStretch();

    auto *hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(m_cancelButton);
    hBoxLayout->addSpacing(3);
    hBoxLayout->addWidget(m_rebootButton);

    centralLayout->addLayout(hBoxLayout);
    centralLayout->setContentsMargins(10, 150, 10, 10);
    m_centerWidget = new QWidget;
    m_centerWidget->setObjectName("centerWidget");

    m_centerWidget->setLayout(centralLayout);

    auto *mainLayout = new QHBoxLayout;
    auto *mainWidget = new QWidget;
    mainWidget->setObjectName("mainWidget");
    mainLayout->addWidget(m_centerWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    setCentralWidget(mainWidget);
    centralWidget()->setLayout(mainLayout);
    setFixedSize(484, 682);
    setWindowFlags(windowFlags()&~Qt::WindowMinMaxButtonsHint|Qt::WindowMinimizeButtonHint);
    move(qApp->primaryScreen()->geometry().center() - rect().center());
    titlebar()->setTitle(" ");
    titlebar()->setIcon(QIcon(":/resources/icons/deepin-graphics-driver-manager-64px.svg"));
    titlebar()->setFixedWidth(width() - 50);

    m_closeButton = new CloseButton(this);
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setVisible(true);
    m_closeButton->setGeometry(titlebar()->width(),0,50,50);

    onThemeChanged(DGuiApplicationHelper::instance()->themeType());

    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelBtnClicked);
    connect(m_rebootButton, &QPushButton::clicked, [=] {
       reboot();
    });
    QTimer::singleShot(0, this, &MainWindow::onInstall);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MainWindow::onThemeChanged);
    connect(m_closeButton, &CloseButton::clicked, this, &MainWindow::onCancelBtnClicked);

}

MainWindow::~MainWindow()
= default;


void MainWindow::onCancelBtnClicked()
{
    qDebug() << "onCancelBtnClicked";
    QDBusPendingReply<void> reply = m_graphicsDriver->CancelInstall();
    reply.waitForFinished();
    reboot();
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_closeButton->isEnabled()) {
        event->ignore();
    } else {
        onCancelBtnClicked();
        qApp->quit();
    }
}

void MainWindow::onInstall()
{
    m_installState->setText(tr("Installing"));
    m_installState->setVisible(true);
    m_warningTips->setText(tr("It will auto reboot after the successful installation, please do not take any actions"));
    m_warningTips->setVisible(true);
    m_rebootButton->setVisible(false);
    m_closeButton->setEnabled(false);
    titlebar()->setQuitMenuDisabled(true);
#ifdef TEST_UI
    m_timer.setInterval(50);
    m_timer.start();
    m_process = 0;
    updateProgress();
    connect(&m_timer, &QTimer::timeout, [=]{
        if (m_process >= 100) {
            m_timer.stop();
        }
        updateProgress();
        m_process++;
    });
#else
    QDBusPendingReply<void> installReply = m_graphicsDriver->TestInstall();
    installReply.waitForFinished();

    if (!installReply.isValid()) {
        qCritical() << "install error:" << installReply.error();
        updateInstallState(false);
        Utils::resetDisablePluginList();
        return;
    }
    m_process = 0;
    updateProgress();
    connect(m_graphicsDriver, &ComDeepinDaemonGraphicsDriverInterface::ReportProgress, [=](QString ratio){
        m_process = ratio.toInt();
        updateProgress();
    });
#endif
}

void  MainWindow::onThemeChanged(DGuiApplicationHelper::ColorType type)
{
    if (type == DGuiApplicationHelper::ColorType::LightType) {
        DGuiApplicationHelper::instance()->setThemeType(type);

        m_centerWidget->setStyleSheet("QWidget#centerWidget{"
                                      "border-radius: 8px;"
                                      "padding:2px 4px;"
                                      "background-color: rgba(255, 255, 255, 1);"
                                      "}");

        m_installState->setStyleSheet("QLabel {"
                                     "font-weight: medium;"
                                     "color: rgba(0, 0, 0, 0.9);"
                                     "}");

        m_warningTips->setStyleSheet("QLabel {"
                                     "font-weight: normal;"
                                     "color: rgba(0, 0, 0, 0.7);"
                                     "}");

    } else if (type == DGuiApplicationHelper::ColorType::DarkType) {
        DGuiApplicationHelper::instance()->setThemeType(type);

        m_centerWidget->setStyleSheet("QWidget#centerWidget{"
                                      "border-radius: 8px;"
                                      "padding:2px 4px;"
                                      "background-color: rgba(255, 255, 255, 0.05);"
                                      "}");

        m_installState->setStyleSheet("QLabel {"
                                      "font-weight: medium;"
                                      "color: #c0c6d4;"
                                      "}");

        m_warningTips->setStyleSheet("QLabel {"
                                     "font-weight: normal;"
                                     "color: rgba(255, 255, 255, 0.7);"
                                     "}");
    }
}

void MainWindow::updateProgress()
{
    qInfo() << QString("complete progress: %1").arg(m_process);
    if (m_process < 100 && m_process >= 0) {
        if (m_process == 0) {
            m_waterProgress->start();
            m_waterProgress->setValue(1);
        } else {
            m_waterProgress->setValue(m_process);
        }
    } else if (m_process < 0) {
        updateInstallState(false);
        Utils::resetDisablePluginList();
    } else {
        m_process = 0;
        m_waterProgress->stop();
        updateInstallState(true);
    }
}

void MainWindow::reboot()
{
#ifndef TEST_UI
    qInfo() << "reboot";
    QProcess::startDetached("dbus-send --system --print-reply --dest=org.freedesktop.login1 /org/freedesktop/login1 org.freedesktop.login1.Manager.Reboot boolean:true");
#endif
}

void MainWindow::updateInstallState(bool success)
{
    m_waterProgress->setVisible(false);
    m_cancelButton->setVisible(true);
    m_closeButton->setEnabled(true);
    titlebar()->setQuitMenuDisabled(false);
    if (success) {
        m_installStateIcon->setVisible(true);
        m_installStateIcon->setPixmap(Utils::hidpiPixmap(":/resources/icons/success.svg", QSize(128, 128)));
        m_installState->setText(tr("Installation successful"));
        m_warningTips->setText(tr("Your computer will reboot soon"));

    } else {
        m_installStateIcon->setVisible(true);
        m_installStateIcon->setPixmap(Utils::hidpiPixmap(":/resources/icons/fail.svg", QSize(128, 128)));
        m_installState->setText(tr("Installation failed"));
        m_warningTips->setText(tr("Your computer will reboot soon and switch to the previous driver"));
    }

    if (m_timer.isActive()) {
        m_timer.stop();
    }

    m_timer.setInterval(0);
    m_timer.start();
    m_rebootButton->setVisible(true);
    connect(&m_timer, &QTimer::timeout, [=] {
        m_timer.setInterval(1000);
        if (m_rebootDelay > 0) {
            m_rebootButton->setText(tr("Reboot Now (%1)").arg(m_rebootDelay));
            m_rebootDelay--;
        } else {
            m_timer.stop();
            reboot();
        }
    });
}