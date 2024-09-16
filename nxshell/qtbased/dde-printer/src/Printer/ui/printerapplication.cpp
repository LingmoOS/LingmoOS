// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printerapplication.h"
#include "jobmanagerwindow.h"
#include "dprintersshowwindow.h"
#include "zdrivermanager.h"
#include "dprintermanager.h"
#include "zsettings.h"
#include "zjobmanager.h"
#include "config.h"

#include <DApplication>
#include <DLog>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QTranslator>
#include <QDBusError>
#include <QDBusConnection>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

Q_LOGGING_CATEGORY(PRINTERAPP, "org.deepin.dde-printer.app")

enum ApplicationModel {
    APPMODEL_Watch = 0x1,
    APPMODEL_TrayIcon = 0x2,
    APPMODEL_JobsWindow = 0x4,
    APPMODEL_MainWindow = 0x8,
};

PrinterApplication *PrinterApplication::getInstance()
{
    static PrinterApplication *instance = nullptr;
    if (!instance)
        instance = new PrinterApplication();

    return instance;
}

void PrinterApplication::slotNewProcessInstance(qint64 pid, const QStringList &arguments)
{
    Q_UNUSED(pid);
    qCInfo(PRINTERAPP) << "enter slotNewProcessInstance";

    launchWithMode(arguments);
}

int PrinterApplication::launchWithMode(const QStringList &arguments)
{
    int allModel = APPMODEL_Watch | APPMODEL_TrayIcon | APPMODEL_JobsWindow | APPMODEL_MainWindow;
    int appModel = 0;
    if (arguments.count() > 2 && arguments[1] == "-m") {
        appModel = arguments[2].toInt() & allModel;
    }

    appModel = appModel > 0 ? appModel : APPMODEL_MainWindow;

    if (appModel & APPMODEL_JobsWindow) {
        showJobsWindow();
    }

    if ((appModel & APPMODEL_MainWindow)) {
        QString printerName = "";
        if (arguments.count() == 3 && arguments[1] == "-p") {
            printerName = arguments[2];
        }
        showMainWindow(printerName);
    }

    return 0;
}

int PrinterApplication::create()
{
    if (!qApp)
        return -1;


    qApp->loadTranslator();
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("dde-printer");
    qApp->setApplicationVersion(DApplication::buildVersion((QMAKE_VERSION)));
    qApp->setProductIcon(QIcon(":/images/dde-printer.svg"));
    qApp->setProductName(tr("Print Manager"));
    qApp->setApplicationDescription(tr("Print Manager is a printer management tool, which supports adding and removing printers, managing print jobs and so on."));
    qApp->setApplicationLicense("GPLv3.");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    DLogManager::registerJournalAppender();
#else
    QString logRules = g_Settings->getLogRules();
    QLoggingCategory::setFilterRules(logRules);
#endif

    QObject::tr("Direct-attached Device");
    QObject::tr("File");

    DPrinterManager::getInstance()->initLanguageTrans();

    if (!DGuiApplicationHelper::setSingleInstance("dde-printer")) {
        //进程设置单例失败，杀死原始进程，继续设置单例，虽然返回true，但是实际没有生效，所以先杀死原始进程，再设置新进程单例
        qWarning() << "dde-printer is running";
        return -2;
    }

    bool isConnectSuc = connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &PrinterApplication::slotNewProcessInstance);
    if (!isConnectSuc)
        qCWarning(PRINTERAPP) << "newProcessInstance connect: " << isConnectSuc;

    return 0;
}

int PrinterApplication::stop()
{
    g_driverManager->stop();

    delete DPrinterManager::getInstance();
    return 0;
}

int PrinterApplication::showJobsWindow()
{
    if (!m_jobsWindow) {
        m_jobsWindow = new JobManagerWindow();
        connect(m_jobsWindow, &JobManagerWindow::destroyed, this, [&]() {
            m_jobsWindow = nullptr;
        });
        Dtk::Widget::moveToCenter(m_jobsWindow);
    }
    m_jobsWindow->activateWindow();
    m_jobsWindow->showNormal();
    m_jobsWindow->setAccessibleName("jobsWindow");
    return 0;
}

int PrinterApplication::showMainWindow(const QString &printerName)
{
    if (!m_mainWindow) {
        m_mainWindow = new DPrintersShowWindow(printerName);
        Dtk::Widget::moveToCenter(m_mainWindow);

        // 初始化驱动
        g_driverManager->refreshPpds();
    }
    m_mainWindow->activateWindow();
    m_mainWindow->showNormal();
    m_mainWindow->setAccessibleName("mainWindow");

    return 0;
}

void PrinterApplication::slotMainWindowClosed()
{
    if (m_mainWindow) {
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
        g_driverManager->stop();
    }
}


PrinterApplication::PrinterApplication()
    : QObject(nullptr)
    , m_jobsWindow(nullptr)
    , m_mainWindow(nullptr)
{
}

PrinterApplication::~PrinterApplication()
{
    if (m_jobsWindow)
        delete m_jobsWindow;

    if (m_mainWindow)
        delete m_mainWindow;
}


