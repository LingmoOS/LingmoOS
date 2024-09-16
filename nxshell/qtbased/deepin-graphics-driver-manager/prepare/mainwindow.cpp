// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "resolutionwidget.h"
#include "singleresolutionwidget.h"
#include "utils.h"
#include "resolution.h"
#include "../test_installer/mainwindow.h"


#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <DTitlebar>
#include <DSvgRenderer>
#include <QJsonObject>
#include <QJsonDocument>
#include <DApplicationHelper>
#include <iostream>
#include <DFontSizeManager>



const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";


MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{

    m_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                   GraphicMangerServiceName,
                   GraphicMangerPath,
                   QDBusConnection::systemBus());

    m_toggleButton = new QPushButton;
    m_toggleButton->setText(tr("Switch"));
    m_toggleButton->setFixedHeight(38);
    m_toggleButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_toggleButton, DFontSizeManager::T6);


    m_warningTips = new QLabel;
    m_warningTips->setAlignment(Qt::AlignHCenter);
    m_warningTips->setVisible(false);
    m_warningTips->setWordWrap(true);
    m_warningTips->setObjectName("WarningTips");
    DFontSizeManager::instance()->bind(m_warningTips, DFontSizeManager::T8);


    m_installTips = new QLabel;
    m_installTips->setVisible(false);
    m_installTips->setWordWrap(true);
    m_installTips->setAlignment(Qt::AlignHCenter);
    m_installTips->setContentsMargins(65, 0, 65, 0);
    m_installTips->setObjectName("installTips");
    m_installTips->setFixedWidth(424);
    DFontSizeManager::instance()->bind(m_installTips, DFontSizeManager::T6);


    m_spinner = new DSpinner();
    m_spinner->setFixedSize(32, 32);
    m_spinner->setVisible(false);

    m_tipsIcon = new QLabel;

    m_updateButton = new QPushButton;
    m_updateButton->setText(tr("Update"));
    m_updateButton->setFixedHeight(38);
    m_updateButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_updateButton, DFontSizeManager::T6);

    m_rebootButton = new DSuggestButton;
    m_rebootButton->setText(tr("Reboot Now"));
    m_rebootButton->setFixedHeight(38);
    m_rebootButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_rebootButton, DFontSizeManager::T6);

    m_cancelButton = new QPushButton;
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setFixedHeight(38);
    m_cancelButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_cancelButton, DFontSizeManager::T6);

    m_tryButton = new DSuggestButton;
    m_tryButton->setText(tr("Try Again"));
    m_tryButton->setFixedHeight(38);
    m_tryButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_tryButton, DFontSizeManager::T6);

    m_okButton = new QPushButton;
    m_okButton->setText(tr("OK"));
    m_okButton->setFixedHeight(38);
    m_okButton->setVisible(false);
    DFontSizeManager::instance()->bind(m_okButton, DFontSizeManager::T6);


    m_vendorIcon = new QLabel;
    m_vendorIcon->setAlignment(Qt::AlignCenter);
    m_vendorIcon->setFixedSize(140, 140);
    //m_vendorIcon->setFrameShape(QFrame::Box);

    m_vendorName = new QLabel;
    m_vendorName->setWordWrap(true);
    m_vendorName->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_vendorName->setFixedHeight(54);
    m_vendorName->setContentsMargins(10, 0, 10, 0);
    //m_vendorName->setFrameShape(QFrame::Box);
    DFontSizeManager::instance()->bind(m_vendorName, DFontSizeManager::T8);



    m_resolutionsLayout = new QVBoxLayout;
    m_resolutionsLayout->setContentsMargins(0, 0, 0, 0);
    m_resolutionsLayout->setSpacing(10);
    m_resolutionsWidget = new QWidget;
    m_resolutionsWidget->setLayout(m_resolutionsLayout);

    auto *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_vendorIcon);
    centralLayout->setAlignment(m_vendorIcon, Qt::AlignHCenter);
    centralLayout->addSpacing(10);
    centralLayout->addWidget(m_vendorName);
    centralLayout->addSpacing(10);
    centralLayout->addWidget(m_resolutionsWidget);
    //centralLayout->addSpacing(50);
    centralLayout->addWidget(m_tipsIcon);
    centralLayout->setAlignment(m_tipsIcon, Qt::AlignHCenter);
    centralLayout->addWidget(m_spinner);
    centralLayout->setAlignment(m_spinner, Qt::AlignHCenter);
    centralLayout->addWidget(m_installTips);
    centralLayout->setAlignment(m_installTips, Qt::AlignHCenter);
    centralLayout->addStretch();
    centralLayout->addWidget(m_warningTips);
    centralLayout->addSpacing(10);
    centralLayout->addWidget(m_toggleButton);
    centralLayout->addWidget(m_updateButton);
    centralLayout->addWidget(m_okButton);

    auto *hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(m_cancelButton);
    hBoxLayout->addSpacing(8);
    hBoxLayout->addWidget(m_rebootButton);
    hBoxLayout->addWidget(m_tryButton);


    centralLayout->addLayout(hBoxLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(10, 30, 10, 10);
    titlebar()->setTitle(" ");
    titlebar()->setIcon(QIcon(":/resources/icons/deepin-graphics-driver-manager-64px.svg"));
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

    onThemeChanged(DGuiApplicationHelper::instance()->themeType());


    connect(m_toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleBtnClicked);
    connect(m_updateButton, &QPushButton::clicked, this, &MainWindow::onToggleBtnClicked);
    connect(m_rebootButton, &QPushButton::clicked, this, &MainWindow::onRebootBtnClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelBtnClicked);
    connect(m_tryButton, &QPushButton::clicked, this, &MainWindow::onTryClicked);
    connect(m_okButton, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MainWindow::onThemeChanged);

    QTimer::singleShot(0, this, &MainWindow::loadResolutions);

}

MainWindow::~MainWindow()
= default;

void MainWindow::keyPressEvent(QKeyEvent *e)
{

}

void MainWindow::noResolutions()
{
    m_tipsIcon->setVisible(true);
    m_tipsIcon->setPixmap(Utils::hidpiPixmap(":/resources/icons/fail.svg", QSize(128, 128)));
    m_warningTips->setText(tr("Your hardware is not supported currently, please wait for future version."));
    m_warningTips->setVisible(true);
    m_vendorName->setVisible(false);
    m_vendorIcon->setVisible(false);
    m_resolutionsWidget->setVisible(false);
    m_toggleButton->setVisible(false);
}

void MainWindow::loadDevice()
{
    QString devices;
#ifdef TEST_UI
    QString path = RESOURCES_DIR"/test/device.json";
    QFile file(path);
    if (file.exists()) {
       file.open(QIODevice::ReadOnly);
       devices = QString(file.readAll());
       qInfo() << devices;
    }
#else
    QDBusPendingReply<QString> getDeviceReply = m_graphicsDriver->GetDevice();
    getDeviceReply.waitForFinished();
    if (!getDeviceReply.isValid()) {
        qCritical() << getDeviceReply.error();
        return;
    }
    devices = getDeviceReply.value();
#endif
    const QJsonObject devObj = Utils::QStringToJson(devices);
    for (const auto d : devObj["devices"].toArray()) {
        Device device(d.toObject());
        if (device.name().isEmpty() || device.info().isEmpty()) {
            return;
        }
        m_devices.push_back(device);
    }
    setVendorIcon();
}

void MainWindow::setVendorIcon()
{
    bool bIntel = false;
    bool bAmd = false;
    bool bNvidia = false;

    QStringList devInfo;
    foreach(const auto dev, m_devices) {
        if (dev.name().contains("intel", Qt::CaseInsensitive)) {
            bIntel = true;
        }

        if (dev.name().contains("amd", Qt::CaseInsensitive)) {
            bAmd = true;
        }

        if (dev.name().contains("nvidia", Qt::CaseInsensitive)) {
            bNvidia = true;
        }
        devInfo << dev.info();
    }

    QString iconPath;
    if (bIntel && !bAmd && !bNvidia) {
       iconPath = ":/resources/icons/Intel.svg";
    }

    if (bAmd && !bIntel && !bNvidia) {
        iconPath = ":/resources/icons/AMD.svg";
    }

    if (bNvidia && !bIntel && !bAmd) {
        iconPath = ":/resources/icons/NVIDIA.svg";
    }

    if (bIntel && bNvidia && !bAmd) {
        iconPath = ":/resources/icons/Intel-NVIDIA.svg";
    }

    m_vendorIcon->setPixmap(Utils::hidpiPixmap(iconPath, QSize(140, 140)));
    m_vendorName->setText(devInfo.join('\n'));
}

void MainWindow::loadResolutions()
{
    loadDevice();
    QString strResolution;
#ifdef TEST_UI
    QString path = RESOURCES_DIR"/test/nvidia.json";
    QFile file(path);
    if (file.exists()) {
       file.open(QIODevice::ReadOnly);
       strResolution = QString(file.readAll());
    }
#else

    QDBusPendingReply<QString> resolutionReply = m_graphicsDriver->GetResolutionTitle();
    resolutionReply.waitForFinished();
    if (!resolutionReply.isValid()) {
        qCritical() << resolutionReply.error();
    }
    strResolution = resolutionReply.value();
#endif

    qInfo() << strResolution;
    QJsonObject resolutionRoot = Utils::QStringToJson(strResolution);
    qInfo() << resolutionRoot;
    QJsonArray resolutions = resolutionRoot["resolutions"].toArray();
    if (resolutions.empty()) {
        noResolutions();
        return;
    }

    if (resolutionRoot["type"].toInt() == INTEL_NVIDIA_USE_INTEL) {
        m_warningTips->setVisible(true);
        m_warningTips->setText(tr("Switching to the discrete graphics interface may cause a black screen."));

    } else if (resolutionRoot["type"].toInt() == INTEL_NVIDIA_USE_NVIDIA) {
        m_warningTips->setVisible(true);
        m_warningTips->setText(tr("Switching to the integrated graphics interface may cause a black screen."));
    }

    if (resolutions.size() == 1) {
        m_usedIndex = 0;
        Resolution solution(resolutions[0].toObject());
        ResolutionWidget *rw = new SingleResolutionWidget(m_graphicsDriver, solution);
        rw->initUI();
        m_resolutionsLayout->addWidget(rw);
        connect(rw, &ResolutionWidget::clicked, this, &MainWindow::onResolutionSelected);
        if (rw->canUpdate()) {
            m_updateButton->setVisible(true);
            m_toggleButton->setVisible(false);
        } else {
            m_okButton->setVisible(true);
        }
    } else {
        int index = 0;
        for (const auto res : resolutions) {
            Resolution solution(res.toObject());
            auto *rw = new ResolutionWidget(m_graphicsDriver, solution);
            rw->initUI();
            m_resolutionsLayout->addWidget(rw);
            connect(rw, &ResolutionWidget::clicked, this, &MainWindow::onResolutionSelected);
            if (solution.enable()) {
                m_usedIndex = index;
                m_selectedIndex = index;
                qInfo() << "m_usedIndex = " << m_usedIndex;
                qInfo() << "m_selectedIndex = " << m_selectedIndex;
                if (rw->canUpdate()) {
                    m_updateButton->setVisible(true);
                    m_toggleButton->setVisible(false);
                } else {
                    m_okButton->setVisible(true);
                }
            }
            qInfo() << "index = " << index;
            index++;
        }

    }
}

void MainWindow::onResolutionSelected()
{
    auto *rw = dynamic_cast<ResolutionWidget *>(sender());
    qInfo() << "Resolution selected: " << rw->resolution().name();

    const int idx = m_resolutionsLayout->indexOf(rw);
    Q_ASSERT(idx != -1);

    m_selectedIndex = idx;

    for (int i = 0; i != m_resolutionsLayout->count(); ++i)
    {
        auto *w = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(i)->widget());
        w->setChecked(i == idx);
    }
    qInfo() << "m_usedIndex = " << m_usedIndex;
    qInfo() << "m_selectedIndex = " << m_selectedIndex;

    m_updateButton->setVisible(rw->canUpdate());
    m_toggleButton->setVisible(!m_updateButton->isVisible() && m_selectedIndex != m_usedIndex);
    m_okButton->setVisible(!(m_updateButton->isVisible() || m_toggleButton->isVisible()));
}

void MainWindow::onUpdateBtnClicked()
{
//    auto *new_driver_widget = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(m_usedIndex)->widget());
//    new_driver_widget->prepareInstall();
//    m_toggleButton->setVisible(false);
//    m_updateButton->setVisible(false);
}

void MainWindow::onToggleBtnClicked()
{
    //Q_ASSERT(m_selectedIndex != m_usedIndex);
    m_updateButton->setEnabled(false);
    m_toggleButton->setEnabled(false);
    auto *new_driver_widget = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(m_selectedIndex)->widget());
    connect(new_driver_widget, &ResolutionWidget::preInstallProgress, this, &MainWindow::onPreInstallProgress);
    new_driver_widget->prepareInstall();
}

void MainWindow::onRebootBtnClicked()
{
    qInfo() << "onRebootBtnClicked";
    QProcess::startDetached("dbus-send --system --print-reply --dest=org.freedesktop.login1 /org/freedesktop/login1 org.freedesktop.login1.Manager.Reboot boolean:true");
}

void MainWindow::onCancelBtnClicked()
{
    qInfo() << "onCancelBtnClicked";
    QDBusPendingReply<void> reply = m_graphicsDriver->CancelInstall();
    reply.waitForFinished();
    qApp->quit();
}

void MainWindow::onTryClicked()
{
    qInfo() << "onTryClicked";
    auto *new_driver_widget = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(m_selectedIndex)->widget());
    connect(new_driver_widget, &ResolutionWidget::preInstallProgress, this, &MainWindow::onPreInstallProgress);
    m_startPreInstall = false;
    m_tipsIcon->setVisible(false);
    m_tryButton->setVisible(false);
    setVendorIcon();
    m_vendorIcon->setVisible(true);
    m_vendorName->setVisible(true);
    m_installTips->setVisible(false);
    new_driver_widget->prepareInstall();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    qInfo() << "closeEvent";
    QDBusPendingReply<void> reply = m_graphicsDriver->CancelInstall();
    reply.waitForFinished();
    qApp->quit();
}

void MainWindow::onPreInstallProgress(int progress)
{
    qInfo() << "preInstallerProgress = " << progress;

    if (progress >= 0 && progress < 100) {
        if (!m_startPreInstall) {
            m_resolutionsWidget->setVisible(false);
            m_toggleButton->setVisible(false);
            m_updateButton->setVisible(false);
            m_warningTips->setVisible(false);
            m_rebootButton->setVisible(true);
            m_rebootButton->setEnabled(false);
            m_cancelButton->setEnabled(false);
            m_cancelButton->setVisible(true);
            if (!m_spinner->isPlaying()) {
                m_spinner->start();
                m_spinner->setVisible(true);
            }
            //m_installTips->setText(tr("Please reboot for switching to another driver, which may cost several minutes, please wait patiently"));
            //m_installTips->setVisible(true);
            m_startPreInstall = true;
        }
    } else if (progress < 0){
        m_resolutionsWidget->setVisible(false);
        m_toggleButton->setVisible(false);
        m_updateButton->setVisible(false);
        m_warningTips->setVisible(false);
        m_rebootButton->setVisible(false);
        m_cancelButton->setVisible(true);
        m_tipsIcon->setVisible(true);
        m_tryButton->setVisible(true);
        m_cancelButton->setEnabled(true);
        m_tryButton->setEnabled(true);
        m_tipsIcon->setPixmap(Utils::hidpiPixmap(":/resources/icons/fail.svg", QSize(128, 128)));
        m_vendorIcon->setVisible(false);
        m_vendorName->setVisible(false);
        m_installTips->setText(tr("Preparation failed for driver switching"));
        m_installTips->setVisible(true);
        Utils::resetDisablePluginList();
        if (m_spinner->isPlaying()) {
            m_spinner->stop();
            m_spinner->setVisible(false);
        }
        qCritical() << "prepareInstall failed";
    } else {
        m_resolutionsWidget->setVisible(false);
        m_toggleButton->setVisible(false);
        m_updateButton->setVisible(false);
        m_warningTips->setVisible(false);
        m_rebootButton->setVisible(true);
        m_cancelButton->setVisible(true);
        m_rebootButton->setEnabled(true);
        m_cancelButton->setEnabled(true);
        if (m_spinner->isPlaying()) {
            m_spinner->stop();
            m_spinner->setVisible(false);
        }
        m_installTips->setText(tr("Please reboot for switching to another driver, which may cost several minutes, please wait patiently"));
        m_installTips->setVisible(true);
        Utils::setDisablePluginList();
    }
}

void MainWindow::onThemeChanged(DGuiApplicationHelper::ColorType type)
{
    if (type == DGuiApplicationHelper::ColorType::LightType) {
        DGuiApplicationHelper::instance()->setThemeType(type);
        m_centerWidget->setStyleSheet("QWidget#centerWidget{"
                                      "border-radius: 8px;"
                                      "padding:2px 4px;"
                                      "background-color: rgba(255, 255, 255, 1);"
                                      "}");

        m_vendorName->setStyleSheet("QLabel {"
                                    "color: #001a2e;"
                                    "}");


        m_warningTips->setStyleSheet("QLabel {"
                                     "color: #526a7f;"
                                     "}");

        m_installTips->setStyleSheet("QLabel {"
                                     "font-weight: 500;"
                                     "color: rgba(0, 0, 0, 0.9);"
                                     "}");


    } else if (type == DGuiApplicationHelper::ColorType::DarkType) {
        DGuiApplicationHelper::instance()->setThemeType(type);
        m_centerWidget->setStyleSheet("QWidget#centerWidget{"
                                      "border-radius: 8px;"
                                      "padding:2px 4px;"
                                      "background-color: rgba(255, 255, 255, 0.05);"
                                      "}");

        m_vendorName->setStyleSheet("QLabel {"
                                    "color: #c0c6d4;"
                                    "}");

        m_warningTips->setStyleSheet("QLabel {"
                                     "color: #6d7c88;"
                                     "}");

        m_installTips->setStyleSheet("QLabel {"
                                     "font-weight: 500;"
                                     "}");
    }
}
