/*******************************************************************
 * backtracewidget.cpp
 * SPDX-FileCopyrightText: 2009, 2010 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include "backtracewidget.h"

#include <QDebug>
#include <QLabel>
#include <QScrollBar>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>
#include <qdesktopservices.h>

#include "backtracegenerator.h"
#include "backtraceratingwidget.h"
#include "crashedapplication.h"
#include "debuggermanager.h"
#include "drkonqi.h"
#include "drkonqi_globals.h"
#include "parser/backtraceparser.h"

static const char extraDetailsLabelMargin[] = " margin: 5px; ";

BacktraceWidget::BacktraceWidget(BacktraceGenerator *generator, QWidget *parent, bool showToggleBacktrace)
    : QWidget(parent)
    , m_btGenerator(generator)
{
    ui.setupUi(this);

    // Debug package installer
    m_debugPackageInstaller = new DebugPackageInstaller(this);
    connect(m_debugPackageInstaller, &DebugPackageInstaller::error, this, &BacktraceWidget::debugPackageError);
    connect(m_debugPackageInstaller, &DebugPackageInstaller::packagesInstalled, this, &BacktraceWidget::regenerateBacktrace);
    connect(m_debugPackageInstaller, &DebugPackageInstaller::canceled, this, &BacktraceWidget::debugPackageCanceled);

    connect(m_btGenerator, &BacktraceGenerator::starting, this, &BacktraceWidget::setAsLoading);
    connect(m_btGenerator, &BacktraceGenerator::done, this, &BacktraceWidget::loadData);
    connect(m_btGenerator, &BacktraceGenerator::someError, this, &BacktraceWidget::loadData);
    connect(m_btGenerator, &BacktraceGenerator::failedToStart, this, &BacktraceWidget::loadData);
    connect(m_btGenerator, &BacktraceGenerator::newLine, this, &BacktraceWidget::backtraceNewLine);

    connect(ui.m_extraDetailsLabel, &QLabel::linkActivated, this, &BacktraceWidget::extraDetailsLinkActivated);
    ui.m_extraDetailsLabel->setVisible(false);
    ui.m_extraDetailsLabel->setStyleSheet(QLatin1String(extraDetailsLabelMargin));

    // Setup the buttons
    KGuiItem::assign(ui.m_reloadBacktraceButton,
                     KGuiItem2(i18nc("@action:button", "&Reload"),
                               QIcon::fromTheme(QStringLiteral("view-refresh")),
                               i18nc("@info:tooltip",
                                     "Use this button to "
                                     "reload the crash information (backtrace). This is useful when you have "
                                     "installed the proper debug symbol packages and you want to obtain "
                                     "a better backtrace.")));
    connect(ui.m_reloadBacktraceButton, &QPushButton::clicked, this, &BacktraceWidget::regenerateBacktrace);

    KGuiItem::assign(ui.m_installDebugButton,
                     KGuiItem2(i18nc("@action:button", "&Install Debug Symbols"),
                               QIcon::fromTheme(QStringLiteral("system-software-update")),
                               i18nc("@info:tooltip",
                                     "Use this button to "
                                     "install the missing debug symbols packages.")));
    ui.m_installDebugButton->setVisible(false);
    connect(ui.m_installDebugButton, &QPushButton::clicked, this, &BacktraceWidget::installDebugPackages);
    if (DrKonqi::crashedApplication()->hasDeletedFiles()) {
        ui.m_installDebugButton->setEnabled(false);
        ui.m_installDebugButton->setToolTip(i18nc("@info:tooltip",
                                                  "Symbol installation is unavailable because the application "
                                                  "was updated or uninstalled after it had been started."));
    }

    KGuiItem::assign(ui.m_copyButton,
                     KGuiItem2(QString(),
                               QIcon::fromTheme(QStringLiteral("edit-copy")),
                               i18nc("@info:tooltip",
                                     "Use this button to copy the "
                                     "crash information (backtrace) to the clipboard.")));
    connect(ui.m_copyButton, &QPushButton::clicked, this, &BacktraceWidget::copyClicked);
    ui.m_copyButton->setEnabled(false);

    KGuiItem::assign(ui.m_saveButton,
                     KGuiItem2(QString(),
                               QIcon::fromTheme(QStringLiteral("document-save")),
                               i18nc("@info:tooltip",
                                     "Use this button to save the "
                                     "crash information (backtrace) to a file. This is useful "
                                     "if you want to take a look at it or to report the bug "
                                     "later.")));
    connect(ui.m_saveButton, &QPushButton::clicked, this, &BacktraceWidget::saveClicked);
    ui.m_saveButton->setEnabled(false);

    // Create the rating widget
    m_backtraceRatingWidget = new BacktraceRatingWidget(ui.m_statusWidget);
    ui.m_statusWidget->addCustomStatusWidget(m_backtraceRatingWidget);

    ui.m_statusWidget->setIdle(QString());

    // Do we need the "Show backtrace" toggle action ?
    if (!showToggleBacktrace) {
        ui.mainLayout->removeWidget(ui.m_toggleBacktraceCheckBox);
        ui.m_toggleBacktraceCheckBox->setVisible(false);
        toggleBacktrace(true);
    } else {
        // Generate help widget
        ui.m_backtraceHelpLabel->setText(
            i18n("<h2>What is a \"backtrace\" ?</h2><p>A backtrace basically describes what was "
                 "happening inside the application when it crashed, so the developers may track "
                 "down where the mess started. They may look meaningless to you, but they might "
                 "actually contain a wealth of useful information.<br />Backtraces are commonly "
                 "used during interactive and post-mortem debugging.</p>"));
        ui.m_backtraceHelpIcon->setPixmap(QIcon::fromTheme(QStringLiteral("help-hint")).pixmap(48, 48));
        connect(ui.m_toggleBacktraceCheckBox, &QCheckBox::toggled, this, &BacktraceWidget::toggleBacktrace);
        toggleBacktrace(false);
    }

    ui.m_backtraceEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void BacktraceWidget::setAsLoading()
{
    // remove the syntax highlighter
    delete m_highlighter;
    m_highlighter = nullptr;

    // Set the widget as loading and disable all the action buttons
    ui.m_backtraceEdit->setText(i18nc("@info:status", "Loading..."));
    ui.m_backtraceEdit->setEnabled(false);

    ui.m_statusWidget->setBusy(i18nc("@info:status", "Generating backtrace... (this may take some time)"));
    m_backtraceRatingWidget->setUsefulness(BacktraceParser::Useless);
    m_backtraceRatingWidget->setState(BacktraceGenerator::Loading);

    ui.m_extraDetailsLabel->setVisible(false);
    ui.m_extraDetailsLabel->clear();

    ui.m_installDebugButton->setVisible(false);
    ui.m_reloadBacktraceButton->setEnabled(false);

    ui.m_copyButton->setEnabled(false);
    ui.m_saveButton->setEnabled(false);
}

// Force backtrace generation
void BacktraceWidget::regenerateBacktrace()
{
    setAsLoading();

    if (!DrKonqi::debuggerManager()->debuggerIsRunning()) {
        m_btGenerator->start();
    } else {
        anotherDebuggerRunning();
    }

    Q_EMIT stateChanged();
}

void BacktraceWidget::generateBacktrace()
{
    if (m_btGenerator->state() == BacktraceGenerator::NotLoaded) {
        // First backtrace generation
        regenerateBacktrace();
    } else if (m_btGenerator->state() == BacktraceGenerator::Loading) {
        // Set in loading state, the widget will catch the backtrace events anyway
        setAsLoading();
        Q_EMIT stateChanged();
    } else {
        //*Finished* states
        setAsLoading();
        Q_EMIT stateChanged();
        // Load already generated information
        loadData();
    }
}

void BacktraceWidget::anotherDebuggerRunning()
{
    // As another debugger is running, we should disable the actions and notify the user
    ui.m_backtraceEdit->setEnabled(false);
    ui.m_backtraceEdit->setText(i18nc("@info",
                                      "Another debugger is currently debugging the "
                                      "same application. The crash information could not be fetched."));
    m_backtraceRatingWidget->setState(BacktraceGenerator::Failed);
    m_backtraceRatingWidget->setUsefulness(BacktraceParser::Useless);
    ui.m_statusWidget->setIdle(i18nc("@info:status", "The crash information could not be fetched."));
    ui.m_extraDetailsLabel->setVisible(true);
    ui.m_extraDetailsLabel->setText(xi18nc("@info/rich",
                                           "Another debugging process is attached to "
                                           "the crashed application. Therefore, the DrKonqi debugger cannot "
                                           "fetch the backtrace. Please close the other debugger and "
                                           "click <interface>Reload</interface>."));
    ui.m_installDebugButton->setVisible(false);
    ui.m_reloadBacktraceButton->setEnabled(true);
}

void BacktraceWidget::loadData()
{
    // Load the backtrace data from the generator
    m_backtraceRatingWidget->setState(m_btGenerator->state());

    if (m_btGenerator->state() == BacktraceGenerator::Loaded) {
        ui.m_backtraceEdit->setEnabled(true);
        ui.m_backtraceEdit->setPlainText(m_btGenerator->backtrace());

        // scroll to crash
        QTextCursor crashCursor = ui.m_backtraceEdit->document()->find(QStringLiteral("[KCrash Handler]"));
        if (crashCursor.isNull()) {
            crashCursor = ui.m_backtraceEdit->document()->find(QStringLiteral("KCrash::defaultCrashHandler"));
        }
        if (!crashCursor.isNull()) {
            crashCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
            ui.m_backtraceEdit->verticalScrollBar()->setValue(ui.m_backtraceEdit->cursorRect(crashCursor).top());
        }

        // highlight if possible
        if (m_btGenerator->debuggerIsGDB()) {
            KSyntaxHighlighting::Repository repository;
            m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(ui.m_backtraceEdit->document());
            m_highlighter->setTheme((palette().color(QPalette::Base).lightness() < 128) ? repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
                                                                                        : repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

            const auto def = repository.definitionForName(QStringLiteral("GDB Backtrace"));
            m_highlighter->setDefinition(def);
        }

        BacktraceParser *btParser = m_btGenerator->parser();
        m_backtraceRatingWidget->setUsefulness(btParser->backtraceUsefulness());

        // Generate the text to put in the status widget (backtrace usefulness)
        QString usefulnessText;
        switch (btParser->backtraceUsefulness()) {
        case BacktraceParser::ReallyUseful:
            usefulnessText = i18nc("@info", "The generated crash information is useful");
            break;
        case BacktraceParser::MayBeUseful:
            usefulnessText = i18nc("@info", "The generated crash information may be useful");
            break;
        case BacktraceParser::ProbablyUseless:
            usefulnessText = i18nc("@info", "The generated crash information is probably not useful");
            break;
        case BacktraceParser::Useless:
            usefulnessText = i18nc("@info", "The generated crash information is not useful");
            break;
        default:
            // let's hope nobody will ever see this... ;)
            usefulnessText = i18nc("@info",
                                   "The rating of this crash information is invalid. "
                                   "This is a bug in DrKonqi itself.");
            break;
        }
        ui.m_statusWidget->setIdle(usefulnessText);

        if (btParser->backtraceUsefulness() != BacktraceParser::ReallyUseful) {
            // Not a perfect bactrace, ask the user to try to improve it
            ui.m_extraDetailsLabel->setVisible(true);
            if (canInstallDebugPackages()) {
                // The script to install the debug packages is available
                ui.m_extraDetailsLabel->setText(xi18nc("@info/rich",
                                                       "You can click the <interface>"
                                                       "Install Debug Symbols</interface> button in order to automatically "
                                                       "install the missing debugging information packages. If this method "
                                                       "does not work: please read <link url='%1'>How to "
                                                       "create useful crash reports</link> to learn how to get a useful "
                                                       "backtrace; install the needed packages (<link url='%2'>"
                                                       "list of files</link>) and click the "
                                                       "<interface>Reload</interface> button.",
                                                       QLatin1String(TECHBASE_HOWTO_DOC),
                                                       QLatin1String("#missingDebugPackages")));
                ui.m_installDebugButton->setVisible(true);
                // Retrieve the libraries with missing debug info
                const QStringList missingLibraries = btParser->librariesWithMissingDebugSymbols();
                m_debugPackageInstaller->setMissingLibraries(missingLibraries);
            } else {
                // No automated method to install the missing debug info
                // Tell the user to read the howto and reload
                ui.m_extraDetailsLabel->setText(xi18nc("@info/rich",
                                                       "Please read <link url='%1'>How to "
                                                       "create useful crash reports</link> to learn how to get a useful "
                                                       "backtrace; install the needed packages (<link url='%2'>"
                                                       "list of files</link>) and click the "
                                                       "<interface>Reload</interface> button.",
                                                       QLatin1String(TECHBASE_HOWTO_DOC),
                                                       QLatin1String("#missingDebugPackages")));
            }
        }

        ui.m_copyButton->setEnabled(true);
        ui.m_saveButton->setEnabled(true);
    } else if (m_btGenerator->state() == BacktraceGenerator::Failed) {
        // The backtrace could not be generated because the debugger had an error
        m_backtraceRatingWidget->setUsefulness(BacktraceParser::Useless);

        ui.m_statusWidget->setIdle(i18nc("@info:status", "The debugger has quit unexpectedly."));

        ui.m_backtraceEdit->setPlainText(i18nc("@info:status", "The crash information could not be generated."));

        ui.m_extraDetailsLabel->setVisible(true);
        ui.m_extraDetailsLabel->setText(xi18nc("@info/rich",
                                               "You could try to regenerate the "
                                               "backtrace by clicking the <interface>Reload"
                                               "</interface> button."));
    } else if (m_btGenerator->state() == BacktraceGenerator::FailedToStart) {
        // The backtrace could not be generated because the debugger could not start (missing)
        // Tell the user to install it.
        m_backtraceRatingWidget->setUsefulness(BacktraceParser::Useless);

        ui.m_statusWidget->setIdle(i18nc("@info:status",
                                         "<strong>The debugger application is missing or "
                                         "could not be launched.</strong>"));

        ui.m_backtraceEdit->setPlainText(i18nc("@info:status", "The crash information could not be generated."));
        ui.m_extraDetailsLabel->setVisible(true);
        ui.m_extraDetailsLabel->setText(xi18nc("@info/rich",
                                               "<strong>You need to first install the debugger "
                                               "application (%1) then click the <interface>Reload"
                                               "</interface> button.</strong>",
                                               m_btGenerator->debuggerName()));
    }

    ui.m_reloadBacktraceButton->setEnabled(true);

    Q_EMIT stateChanged();
}

void BacktraceWidget::backtraceNewLine(const QString &line)
{
    // While loading the backtrace (unparsed) a new line was sent from the debugger, append it
    ui.m_backtraceEdit->append(line.trimmed());
}

void BacktraceWidget::copyClicked()
{
    ui.m_backtraceEdit->selectAll();
    ui.m_backtraceEdit->copy();
}

void BacktraceWidget::saveClicked()
{
    DrKonqi::saveReport(m_btGenerator->backtrace(), this);
}

void BacktraceWidget::highlightExtraDetailsLabel(bool highlight)
{
    const QString stylesheet = ((!highlight) ? QLatin1String("border-width: 0px;")
                                             : QLatin1String("border-width: 2px; "
                                                             "border-style: solid; "
                                                             "border-color: red;"))
        + QLatin1String(extraDetailsLabelMargin);

    ui.m_extraDetailsLabel->setStyleSheet(stylesheet);
}

void BacktraceWidget::focusImproveBacktraceButton()
{
    ui.m_installDebugButton->setFocus();
}

void BacktraceWidget::installDebugPackages()
{
    ui.m_installDebugButton->setVisible(false);
    m_debugPackageInstaller->installDebugPackages();
}

void BacktraceWidget::debugPackageError(const QString &errorMessage)
{
    ui.m_installDebugButton->setVisible(true);
    KMessageBox::error(this,
                       errorMessage,
                       i18nc("@title:window",
                             "Error during the installation of"
                             " debug symbols"));
}

void BacktraceWidget::debugPackageCanceled()
{
    ui.m_installDebugButton->setVisible(true);
}

bool BacktraceWidget::canInstallDebugPackages() const
{
    return m_debugPackageInstaller->canInstallDebugPackages();
}

void BacktraceWidget::toggleBacktrace(bool show)
{
    ui.m_backtraceStack->setCurrentWidget(show ? ui.backtracePage : ui.backtraceHelpPage);
}

void BacktraceWidget::extraDetailsLinkActivated(QString link)
{
    if (link.startsWith(QLatin1String("http"))) {
        // Open externally
        QDesktopServices::openUrl(QUrl(link));
    } else if (link == QLatin1String("#missingDebugPackages")) {
        BacktraceParser *btParser = m_btGenerator->parser();

        QStringList missingDbgForFiles = btParser->librariesWithMissingDebugSymbols();
        missingDbgForFiles.insert(0, DrKonqi::crashedApplication()->executable().absoluteFilePath());

        // HTML message
        QString message = QStringLiteral("<html>") + i18n("The packages containing debug information for the following application and libraries are missing:")
            + QStringLiteral("<br /><ul>");

        for (const QString &string : std::as_const(missingDbgForFiles)) {
            message += QLatin1String("<li>") + string + QLatin1String("</li>");
        }

        message += QStringLiteral("</ul></html>");

        KMessageBox::information(this, message, i18nc("messagebox title", "Missing debug information packages"));
    }
}

#include "moc_backtracewidget.cpp"
