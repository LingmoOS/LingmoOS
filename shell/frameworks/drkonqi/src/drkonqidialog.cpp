/*******************************************************************
 * drkonqidialog.cpp
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include "drkonqidialog.h"

#include <KLocalizedString>
#include <KWindowConfig>

#include "drkonqi_debug.h"
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLocale>
#include <QMenu>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>

#include "backtracegenerator.h"
#include "backtraceparser.h"
#include "backtracewidget.h"
#include "bugzillaintegration/bugzillalib.h"
#include "config-drkonqi.h"
#include "crashedapplication.h"
#include "debuggerlaunchers.h"
#include "debuggermanager.h"
#include "drkonqi.h"
#include "drkonqi_globals.h"
#include "qmlextensions/credentialstore.h"
#include "qmlextensions/doctore.h"
#include "qmlextensions/platformmodel.h"
#include "qmlextensions/reproducibilitymodel.h"
#include "settings.h"

static const QString ABOUT_BUG_REPORTING_URL = QStringLiteral("https://community.kde.org/Get_Involved/Issue_Reporting");
static const QString DRKONQI_REPORT_BUG_URL = KDE_BUGZILLA_URL + QStringLiteral("enter_bug.cgi?product=drkonqi&format=guided");

void DrKonqiDialog::show(DrKonqiDialog::GoTo to)
{
    if (DrKonqi::isSafer() || DrKonqi::minimalMode()) {
        QDialog::show();
        return;
    }

    auto engine = new QQmlApplicationEngine(this);

    static auto l10nContext = new KLocalizedContext(engine);
    l10nContext->setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));
    engine->rootContext()->setContextObject(l10nContext);

    qmlRegisterType<BugzillaManager>("org.kde.drkonqi", 1, 0, "Bugzilla");
    qmlRegisterType<PlatformModel>("org.kde.drkonqi", 1, 0, "PlatformModel");
    qmlRegisterType<ReproducibilityModel>("org.kde.drkonqi", 1, 0, "ReproducibilityModel");
    qmlRegisterType<CredentialStore>("org.kde.drkonqi", 1, 0, "CredentialStore");
    qmlRegisterType<DebugPackageInstaller>("org.kde.drkonqi", 1, 0, "DebugPackageInstaller");

    qmlRegisterSingletonInstance("org.kde.drkonqi", 1, 0, "ReportInterface", ReportInterface::self());
    qmlRegisterSingletonInstance("org.kde.drkonqi", 1, 0, "CrashedApplication", DrKonqi::crashedApplication());
    qmlRegisterSingletonInstance("org.kde.drkonqi", 1, 0, "BacktraceGenerator", DrKonqi::debuggerManager()->backtraceGenerator());

    static Doctore doctore;
    qmlRegisterSingletonInstance("org.kde.drkonqi", 1, 0, "DrKonqi", &doctore);

    auto settings = Settings::self();
    qmlRegisterSingletonInstance("org.kde.drkonqi", 1, 0, "Settings", settings);

    // TODO do we need this second BG?
    qmlRegisterUncreatableType<BacktraceGenerator>("org.kde.drkonqi", 1, 0, "BacktraceGenerator1", QStringLiteral("Cannot create WarningLevel in QML"));
    qmlRegisterUncreatableType<BacktraceParser>("org.kde.drkonqi", 1, 0, "BacktraceParser", QStringLiteral("Cannot create WarningLevel in QML"));

    const QUrl mainUrl(QStringLiteral("qrc:/ui/main.qml"));
    QObject::connect(
        engine,
        &QQmlApplicationEngine::objectCreated,
        this,
        [mainUrl, this, to](QObject *obj, const QUrl &objUrl) {
            if (!obj && mainUrl == objUrl) {
                qWarning() << "Failed to load QML dialog, falling back to QWidget.";
                QDialog::show();
                return;
            }

            switch (to) {
            case GoTo::Main:
                break;
            case GoTo::Sentry:
                QMetaObject::invokeMethod(obj, "goToSentry", Qt::QueuedConnection);
                break;
            }
        },
        Qt::QueuedConnection);
    engine->load(mainUrl);
}

DrKonqiDialog::DrKonqiDialog(QWidget *parent)
    : QDialog(parent)
{
    // NOTE: quitting the application is managed by main.cpp, do not ever call quit directly!
    setAttribute(Qt::WA_DeleteOnClose, true);

    // Setting dialog title and icon
    setWindowTitle(DrKonqi::crashedApplication()->name());
    setWindowIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug")));

    auto *l = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);
    l->addWidget(m_tabWidget);
    m_buttonBox = new QDialogButtonBox(this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
    l->addWidget(m_buttonBox);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &DrKonqiDialog::tabIndexChanged);

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("General"));

    if (!config.readEntry(QStringLiteral("ShowOnlyBacktrace"), false)) {
        buildIntroWidget();
        m_tabWidget->addTab(m_introWidget, i18nc("@title:tab general information", "&General"));
    }

    m_backtraceWidget = new BacktraceWidget(DrKonqi::debuggerManager()->backtraceGenerator(), this);
    m_tabWidget->addTab(m_backtraceWidget, i18nc("@title:tab", "&Developer Information"));

    m_tabWidget->tabBar()->setVisible(m_tabWidget->count() > 1);

    buildDialogButtons();

    KWindowConfig::restoreWindowSize(windowHandle(), config);

    // Force a 16:9 ratio for nice appearance by default.
    QSize aspect(16, 9);
    aspect.scale(sizeHint(), Qt::KeepAspectRatioByExpanding);
    resize(aspect);
}

DrKonqiDialog::~DrKonqiDialog()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("General"));
    KWindowConfig::saveWindowSize(windowHandle(), config);
}

void DrKonqiDialog::tabIndexChanged(int index)
{
    if (index == m_tabWidget->indexOf(m_backtraceWidget)) {
        m_backtraceWidget->generateBacktrace();
    }
}

void DrKonqiDialog::buildIntroWidget()
{
    const CrashedApplication *crashedApp = DrKonqi::crashedApplication();

    m_introWidget = new QWidget(this);
    ui.setupUi(m_introWidget);

    ui.titleLabel->setText(xi18nc("@info",
                                  "<para>We are sorry, <application>%1</application> "
                                  "closed unexpectedly.</para>",
                                  crashedApp->name()));

    QString reportMessage;
    if (!crashedApp->bugReportAddress().isEmpty()) {
        if (crashedApp->fakeExecutableBaseName() == QLatin1String("drkonqi")) { // Handle own crashes
            reportMessage = xi18nc("@info",
                                   "<para>As the Crash Handler itself has failed, the "
                                   "automatic reporting process is disabled to reduce the "
                                   "risks of failing again.<nl /><nl />"
                                   "Please, <link url='%1'>manually report</link> this error "
                                   "to the KDE bug tracking system. Do not forget to include "
                                   "the backtrace from the <interface>Developer Information</interface> "
                                   "tab.</para>",
                                   DRKONQI_REPORT_BUG_URL);
        } else if (DrKonqi::isSafer()) {
            reportMessage = xi18nc("@info",
                                   "<para>The reporting assistant is disabled because "
                                   "the crash handler dialog was started in safe mode."
                                   "<nl />You can manually report this bug to %1 "
                                   "(including the backtrace from the "
                                   "<interface>Developer Information</interface> "
                                   "tab.)</para>",
                                   crashedApp->bugReportAddress());
        } else if (crashedApp->hasDeletedFiles()) {
            reportMessage = xi18nc("@info",
                                   "<para>The reporting assistant is disabled because "
                                   "the crashed application appears to have been updated or "
                                   "uninstalled since it had been started. This prevents accurate "
                                   "crash reporting and can also be the cause of this crash.</para>"
                                   "<para>After updating it is always a good idea to log out and back "
                                   "in to make sure the update is fully applied and will not cause "
                                   "any side effects.</para>");
        } else {
            reportMessage = xi18nc("@info",
                                   "<para>You can help us improve KDE Software by reporting "
                                   "this error.<nl /><link url='%1'>Learn "
                                   "more about bug reporting.</link></para>",
                                   ABOUT_BUG_REPORTING_URL);
        }
    } else {
        reportMessage = xi18nc("@info",
                               "<para>You cannot report this error, because "
                               "<application>%1</application> does not provide a bug reporting "
                               "address.</para>",
                               crashedApp->name());
    }
    ui.infoLabel->setText(reportMessage);
    connect(ui.infoLabel, &QLabel::linkActivated, this, &DrKonqiDialog::linkActivated);

    ui.detailsTitleLabel->setText(QStringLiteral("<strong>%1</strong>").arg(i18nc("@label", "Details:")));

    QLocale locale;
    ui.detailsLabel->setText(xi18nc("@info Note the time information is divided into date and time parts",
                                    "<para>Executable: <application>%1"
                                    "</application> PID: %2 Signal: %3 (%4) "
                                    "Time: %5 %6</para>",
                                    crashedApp->fakeExecutableBaseName(),
                                    // prevent number localization by ki18n
                                    QString::number(crashedApp->pid()),
                                    crashedApp->signalName(),
                                    // prevent number localization by ki18n
                                    QString::number(crashedApp->signalNumber()),
                                    locale.toString(crashedApp->datetime().date(), QLocale::ShortFormat),
                                    locale.toString(crashedApp->datetime().time())));
}

void DrKonqiDialog::buildDialogButtons()
{
    const CrashedApplication *crashedApp = DrKonqi::crashedApplication();

    // Set dialog buttons
    m_buttonBox->setStandardButtons(QDialogButtonBox::Close);

    // Default debugger button and menu (only for developer mode): User2
    DebuggerManager *debuggerManager = DrKonqi::debuggerManager();
    m_debugButton = new QPushButton(this);
    KGuiItem2 debugItem(i18nc("@action:button this is the debug menu button label which contains the debugging applications", "&Debug"),
                        QIcon::fromTheme(QStringLiteral("applications-development")),
                        i18nc("@info:tooltip", "Starts a program to debug the crashed application."));
    KGuiItem::assign(m_debugButton, debugItem);
    m_debugButton->setVisible(false);

    m_debugMenu = new QMenu(this);
    m_debugButton->setMenu(m_debugMenu);

    // Only add the debugger if requested by the config or if a KDevelop session is running.
    const bool showExternal = debuggerManager->showExternalDebuggers();
    QList<AbstractDebuggerLauncher *> debuggers = debuggerManager->availableExternalDebuggers();
    for (AbstractDebuggerLauncher *launcher : std::as_const(debuggers)) {
        if (showExternal || qobject_cast<DBusInterfaceLauncher *>(launcher)) {
            addDebugger(launcher);
        }
    }

    connect(debuggerManager, &DebuggerManager::externalDebuggerAdded, this, &DrKonqiDialog::addDebugger);
    connect(debuggerManager, &DebuggerManager::externalDebuggerRemoved, this, &DrKonqiDialog::removeDebugger);
    connect(debuggerManager, &DebuggerManager::debuggerRunning, this, &DrKonqiDialog::enableDebugMenu);

    // Restart application button
    m_restartButton = new QPushButton(m_buttonBox);
    KGuiItem::assign(m_restartButton, DrStandardGuiItem::appRestart());
    m_restartButton->setEnabled(!crashedApp->hasBeenRestarted() && crashedApp->fakeExecutableBaseName() != QLatin1String("drkonqi"));
    m_buttonBox->addButton(m_restartButton, QDialogButtonBox::ActionRole);
    connect(m_restartButton, &QAbstractButton::clicked, crashedApp, &CrashedApplication::restart);
    connect(crashedApp, &CrashedApplication::restarted, this, &DrKonqiDialog::applicationRestarted);

    // Close button
    QString tooltipText = i18nc("@info:tooltip", "Close this dialog (you will lose the crash information.)");
    m_buttonBox->button(QDialogButtonBox::Close)->setToolTip(tooltipText);
    m_buttonBox->button(QDialogButtonBox::Close)->setWhatsThis(tooltipText);
    m_buttonBox->button(QDialogButtonBox::Close)->setFocus();
}

void DrKonqiDialog::addDebugger(AbstractDebuggerLauncher *launcher)
{
    auto *action = new QAction(QIcon::fromTheme(QStringLiteral("applications-development")),
                               i18nc("@action:inmenu 1 is the debugger name", "Debug in %1", launcher->name()),
                               m_debugMenu);
    m_debugMenu->addAction(action);
    connect(action, &QAction::triggered, launcher, &AbstractDebuggerLauncher::start);
    m_debugMenuActions.insert(launcher, action);

    // Make sure that the debug button is the first button with action role to be
    // inserted, then add the other buttons. See removeDebugger below for more information.
    if (!m_debugButtonInBox) {
        auto buttons = m_buttonBox->buttons();
        m_buttonBox->addButton(m_debugButton, QDialogButtonBox::ActionRole);
        m_debugButton->setVisible(true);
        for (QAbstractButton *button : buttons) {
            if (m_buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
                m_buttonBox->addButton(button, QDialogButtonBox::ActionRole);
            }
        }
        m_debugButtonInBox = true;
    }
}

void DrKonqiDialog::removeDebugger(AbstractDebuggerLauncher *launcher)
{
    QAction *action = m_debugMenuActions.take(launcher);
    if (action) {
        m_debugMenu->removeAction(action);
        action->deleteLater();
        // Remove the button from the box, otherwise the box will force
        // it visible as it calls show() explicitly. (QTBUG-3651)
        if (m_debugMenu->isEmpty()) {
            m_buttonBox->removeButton(m_debugButton);
            m_debugButton->setVisible(false);
            m_debugButton->setParent(this);
            m_debugButtonInBox = false;
        }
    } else {
        qCWarning(DRKONQI_LOG) << "Invalid launcher";
    }
}

void DrKonqiDialog::enableDebugMenu(bool debuggerRunning)
{
    m_debugButton->setEnabled(!debuggerRunning);
}

void DrKonqiDialog::linkActivated(const QString &link)
{
    if (link == DRKONQI_REPORT_BUG_URL || link == ABOUT_BUG_REPORTING_URL) {
        QDesktopServices::openUrl(QUrl(link));
    } else if (link.startsWith(QLatin1String("http"))) {
        qCWarning(DRKONQI_LOG) << "unexpected link";
        QDesktopServices::openUrl(QUrl(link));
    }
}

void DrKonqiDialog::applicationRestarted(bool success)
{
    m_restartButton->setEnabled(!success);
}

#include "drkonqidialog.moc"

#include "moc_drkonqidialog.cpp"
