//
// Description: Kate Plugin for GDB integration
//
//
// SPDX-FileCopyrightText: 2010 Ian Wakeling <ian.wakeling@ntlworld.com>
// SPDX-FileCopyrightText: 2010-2014 Kåre Särs <kare.sars@iki.fi>
//
//  SPDX-License-Identifier: LGPL-2.0-only

#include "plugin_kategdb.h"

#include <QFile>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QScrollBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeWidget>

#include <KActionCollection>
#include <KConfigGroup>
#include <KXMLGUIFactory>
#include <QAction>
#include <QMenu>

#include <KColorScheme>
#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KPluginFactory>

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/view.h>

K_PLUGIN_FACTORY_WITH_JSON(KatePluginGDBFactory, "kategdbplugin.json", registerPlugin<KatePluginGDB>();)

KatePluginGDB::KatePluginGDB(QObject *parent, const VariantList &)
    : KTextEditor::Plugin(parent)
{
    // FIXME KF5 KGlobal::locale()->insertCatalog("kategdbplugin");
}

KatePluginGDB::~KatePluginGDB()
{
}

QObject *KatePluginGDB::createView(KTextEditor::MainWindow *mainWindow)
{
    return new KatePluginGDBView(this, mainWindow);
}

KatePluginGDBView::KatePluginGDBView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWin)
    : QObject(mainWin)
    , m_mainWin(mainWin)
{
    m_lastExecUrl = QUrl();
    m_lastExecLine = -1;
    m_lastExecFrame = 0;
    m_kateApplication = KTextEditor::Editor::instance()->application();
    m_focusOnInput = true;
    m_activeThread = -1;

    KXMLGUIClient::setComponentName(QStringLiteral("kategdb"), i18n("Kate GDB"));
    setXMLFile(QStringLiteral("ui.rc"));

    m_toolView.reset(
        m_mainWin->createToolView(plugin, i18n("Debug View"), KTextEditor::MainWindow::Bottom, QIcon::fromTheme(QStringLiteral("debug-run")), i18n("Debug")));

    m_localsStackToolView.reset(m_mainWin->createToolView(plugin,
                                                          i18n("Locals and Stack"),
                                                          KTextEditor::MainWindow::Right,
                                                          QIcon::fromTheme(QStringLiteral("debug-run")),
                                                          i18n("Locals and Stack")));

    m_tabWidget = new QTabWidget(m_toolView.get());
    // Output
    m_outputArea = new QTextEdit();
    m_outputArea->setAcceptRichText(false);
    m_outputArea->setReadOnly(true);
    m_outputArea->setUndoRedoEnabled(false);
    // fixed wide font, like konsole
    m_outputArea->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    // alternate color scheme, like konsole
    KColorScheme schemeView(QPalette::Active, KColorScheme::View);
    m_outputArea->setTextBackgroundColor(schemeView.foreground().color());
    m_outputArea->setTextColor(schemeView.background().color());
    QPalette p = m_outputArea->palette();
    p.setColor(QPalette::Base, schemeView.foreground().color());
    m_outputArea->setPalette(p);

    // input
    m_inputArea = new KHistoryComboBox(true);
    connect(m_inputArea, static_cast<void (KHistoryComboBox::*)(const QString &)>(&KHistoryComboBox::returnPressed), this, &KatePluginGDBView::slotSendCommand);
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_inputArea, 10);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    m_outputArea->setFocusProxy(m_inputArea); // take the focus from the outputArea

    m_gdbPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_gdbPage);
    layout->addWidget(m_outputArea);
    layout->addLayout(inputLayout);
    layout->setStretch(0, 10);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // stack page
    QWidget *stackContainer = new QWidget();
    QVBoxLayout *stackLayout = new QVBoxLayout(stackContainer);
    m_threadCombo = new QComboBox();
    m_stackTree = new QTreeWidget();
    stackLayout->addWidget(m_threadCombo);
    stackLayout->addWidget(m_stackTree);
    stackLayout->setStretch(0, 10);
    stackLayout->setContentsMargins(0, 0, 0, 0);
    stackLayout->setSpacing(0);
    QStringList headers;
    headers << QStringLiteral("  ") << i18nc("Column label (frame number)", "Nr") << i18nc("Column label", "Frame");
    m_stackTree->setHeaderLabels(headers);
    m_stackTree->setRootIsDecorated(false);
    m_stackTree->resizeColumnToContents(0);
    m_stackTree->resizeColumnToContents(1);
    m_stackTree->setAutoScroll(false);
    connect(m_stackTree, &QTreeWidget::itemActivated, this, &KatePluginGDBView::stackFrameSelected);

    connect(m_threadCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KatePluginGDBView::threadSelected);

    QWidget *variableContainer = new QWidget();
    QVBoxLayout *variableLayout = new QVBoxLayout(variableContainer);
    m_scopeCombo = new QComboBox();
    connect(m_scopeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KatePluginGDBView::scopeSelected);
    m_localsView = new LocalsView();
    variableLayout->addWidget(m_scopeCombo);
    variableLayout->addWidget(m_localsView);
    variableLayout->setStretch(0, 10);
    variableLayout->setContentsMargins(0, 0, 0, 0);
    variableLayout->setSpacing(0);

    QSplitter *locStackSplitter = new QSplitter(m_localsStackToolView.get());
    locStackSplitter->addWidget(variableContainer);
    locStackSplitter->addWidget(stackContainer);
    locStackSplitter->setOrientation(Qt::Vertical);

    // config page
    m_configView = new ConfigView(nullptr, mainWin);

    m_ioView = std::make_unique<IOView>();
    connect(m_configView, &ConfigView::showIO, this, &KatePluginGDBView::showIO);

    m_tabWidget->addTab(m_gdbPage, i18nc("Tab label", "GDB Output"));
    m_tabWidget->addTab(m_configView, i18nc("Tab label", "Settings"));

    m_debugView = new Backend(this);
    connect(m_debugView, &DebugViewInterface::readyForInput, this, &KatePluginGDBView::enableDebugActions);
    connect(m_debugView, &DebugViewInterface::debuggerCapabilitiesChanged, [this] {
        enableDebugActions(true);
    });

    connect(m_debugView, &DebugViewInterface::outputText, this, &KatePluginGDBView::addOutputText);

    connect(m_debugView, &DebugViewInterface::outputError, this, &KatePluginGDBView::addErrorText);

    connect(m_debugView, &DebugViewInterface::debugLocationChanged, this, &KatePluginGDBView::slotGoTo);

    connect(m_debugView, &DebugViewInterface::breakPointSet, this, &KatePluginGDBView::slotBreakpointSet);

    connect(m_debugView, &DebugViewInterface::breakPointCleared, this, &KatePluginGDBView::slotBreakpointCleared);

    connect(m_debugView, &DebugViewInterface::clearBreakpointMarks, this, &KatePluginGDBView::clearMarks);

    connect(m_debugView, &DebugViewInterface::programEnded, this, &KatePluginGDBView::programEnded);

    connect(m_debugView, &DebugViewInterface::gdbEnded, this, &KatePluginGDBView::programEnded);

    connect(m_debugView, &DebugViewInterface::gdbEnded, this, &KatePluginGDBView::gdbEnded);

    connect(m_debugView, &DebugViewInterface::stackFrameInfo, this, &KatePluginGDBView::insertStackFrame);

    connect(m_debugView, &DebugViewInterface::stackFrameChanged, this, &KatePluginGDBView::stackFrameChanged);

    connect(m_debugView, &DebugViewInterface::scopesInfo, this, &KatePluginGDBView::insertScopes);

    connect(m_debugView, &DebugViewInterface::variableScopeOpened, m_localsView, &LocalsView::openVariableScope);
    connect(m_debugView, &DebugViewInterface::variableScopeClosed, m_localsView, &LocalsView::closeVariableScope);
    connect(m_debugView, &DebugViewInterface::variableInfo, m_localsView, &LocalsView::addVariableLevel);

    connect(m_debugView, &DebugViewInterface::threadInfo, this, &KatePluginGDBView::insertThread);

    connect(m_debugView, &DebugViewInterface::debuggeeOutput, this, &KatePluginGDBView::addOutput);

    connect(m_debugView, &DebugViewInterface::sourceFileNotFound, this, [this](const QString &fileName) {
        displayMessage(xi18nc("@info",
                              "<title>Could not open file:</title><nl/>%1<br/>Try adding a search path to Advanced Settings -> Source file search paths",
                              fileName),
                       KTextEditor::Message::Error);
    });

    connect(m_localsView, &LocalsView::localsVisible, m_debugView, &DebugViewInterface::slotQueryLocals);

    connect(m_configView, &ConfigView::configChanged, this, [this]() {
        if (!m_configView->debuggerIsGDB())
            return;

        GDBTargetConf config = m_configView->currentGDBTarget();
        if (m_debugView->targetName() == config.targetName) {
            m_debugView->setFileSearchPaths(config.srcPaths);
        }
    });

    // Actions
    m_configView->registerActions(actionCollection());

    QAction *a = actionCollection()->addAction(QStringLiteral("debug"));
    a->setText(i18n("Start Debugging"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::CTRL | Qt::Key_F7)));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotDebug);

    a = actionCollection()->addAction(QStringLiteral("kill"));
    a->setText(i18n("Kill / Stop Debugging"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::ALT | Qt::Key_F7)));
    connect(a, &QAction::triggered, m_debugView, &DebugViewInterface::slotKill);

    a = actionCollection()->addAction(QStringLiteral("rerun"));
    a->setText(i18n("Restart Debugging"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::CTRL | Qt::SHIFT | Qt::Key_F7)));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotRestart);

    a = actionCollection()->addAction(QStringLiteral("toggle_breakpoint"));
    a->setText(i18n("Toggle Breakpoint / Break"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("breakpoint")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::SHIFT | Qt::Key_F11)));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotToggleBreakpoint);

    a = actionCollection()->addAction(QStringLiteral("step_in"));
    a->setText(i18n("Step In"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-step-into")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::Key_F10)));
    connect(a, &QAction::triggered, m_debugView, &DebugViewInterface::slotStepInto);

    a = actionCollection()->addAction(QStringLiteral("step_over"));
    a->setText(i18n("Step Over"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-step-over")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::Key_F9)));
    connect(a, &QAction::triggered, m_debugView, &DebugViewInterface::slotStepOver);

    a = actionCollection()->addAction(QStringLiteral("step_out"));
    a->setText(i18n("Step Out"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-step-out")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::SHIFT | Qt::Key_F10)));
    connect(a, &QAction::triggered, m_debugView, &DebugViewInterface::slotStepOut);

    a = actionCollection()->addAction(QStringLiteral("move_pc"));
    a->setText(i18nc("Move Program Counter (next execution)", "Move PC"));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotMovePC);
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::ALT | Qt::Key_F9)));

    a = actionCollection()->addAction(QStringLiteral("run_to_cursor"));
    a->setText(i18n("Run To Cursor"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run-cursor")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::SHIFT | Qt::Key_F9)));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotRunToCursor);

    a = actionCollection()->addAction(QStringLiteral("continue"));
    a->setText(i18n("Continue"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    actionCollection()->setDefaultShortcut(a, QKeySequence((Qt::CTRL | Qt::Key_F7)));
    connect(a, &QAction::triggered, m_debugView, &DebugViewInterface::slotContinue);

    a = actionCollection()->addAction(QStringLiteral("print_value"));
    a->setText(i18n("Print Value"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("document-preview")));
    connect(a, &QAction::triggered, this, &KatePluginGDBView::slotValue);

    // popup context m_menu
    m_menu = new KActionMenu(i18n("Debug"), this);
    actionCollection()->addAction(QStringLiteral("popup_gdb"), m_menu);
    connect(m_menu->menu(), &QMenu::aboutToShow, this, &KatePluginGDBView::aboutToShowMenu);

    m_breakpoint = m_menu->menu()->addAction(i18n("popup_breakpoint"), this, &KatePluginGDBView::slotToggleBreakpoint);

    QAction *popupAction = m_menu->menu()->addAction(i18n("popup_run_to_cursor"), this, &KatePluginGDBView::slotRunToCursor);
    popupAction->setText(i18n("Run To Cursor"));
    popupAction = m_menu->menu()->addAction(QStringLiteral("move_pc"), this, &KatePluginGDBView::slotMovePC);
    popupAction->setText(i18nc("Move Program Counter (next execution)", "Move PC"));

    enableDebugActions(false);

    connect(m_mainWin, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &KatePluginGDBView::handleEsc);

    const auto documents = KTextEditor::Editor::instance()->application()->documents();
    for (auto doc : documents) {
        enableBreakpointMarks(doc);
    }

    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentCreated, this, &KatePluginGDBView::enableBreakpointMarks);

    m_toolView->installEventFilter(this);

    m_mainWin->guiFactory()->addClient(this);
}

KatePluginGDBView::~KatePluginGDBView()
{
    m_mainWin->guiFactory()->removeClient(this);
}

void KatePluginGDBView::readSessionConfig(const KConfigGroup &config)
{
    m_configView->readConfig(config);
}

void KatePluginGDBView::writeSessionConfig(KConfigGroup &config)
{
    m_configView->writeConfig(config);
}

void KatePluginGDBView::slotDebug()
{
    disconnect(m_ioView.get(), &IOView::stdOutText, nullptr, nullptr);
    disconnect(m_ioView.get(), &IOView::stdErrText, nullptr, nullptr);
    if (m_configView->showIOTab()) {
        connect(m_ioView.get(), &IOView::stdOutText, m_ioView.get(), &IOView::addStdOutText);
        connect(m_ioView.get(), &IOView::stdErrText, m_ioView.get(), &IOView::addStdErrText);
    } else {
        connect(m_ioView.get(), &IOView::stdOutText, this, &KatePluginGDBView::addOutputText);
        connect(m_ioView.get(), &IOView::stdErrText, this, &KatePluginGDBView::addErrorText);
    }
    QStringList ioFifos;
    ioFifos << m_ioView->stdinFifo();
    ioFifos << m_ioView->stdoutFifo();
    ioFifos << m_ioView->stderrFifo();

    enableDebugActions(true);
    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);
    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
    m_scopeCombo->clear();
    m_localsView->clear();

    if (m_configView->debuggerIsGDB()) {
        m_debugView->runDebugger(m_configView->currentGDBTarget(), ioFifos);
    } else {
        m_debugView->runDebugger(m_configView->currentDAPTarget(true));
    }
}

void KatePluginGDBView::slotRestart()
{
    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);
    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
    m_scopeCombo->clear();
    m_localsView->clear();

    m_debugView->slotReRun();
}

void KatePluginGDBView::aboutToShowMenu()
{
    if (!m_debugView->debuggerRunning() || m_debugView->debuggerBusy()) {
        m_breakpoint->setText(i18n("Insert breakpoint"));
        m_breakpoint->setDisabled(true);
        return;
    }

    m_breakpoint->setDisabled(false);

    KTextEditor::View *editView = m_mainWin->activeView();
    QUrl url = editView->document()->url();
    int line = editView->cursorPosition().line();

    line++; // GDB uses 1 based line numbers, kate uses 0 based...

    if (m_debugView->hasBreakpoint(url, line)) {
        m_breakpoint->setText(i18n("Remove breakpoint"));
    } else {
        m_breakpoint->setText(i18n("Insert breakpoint"));
    }
}

void KatePluginGDBView::slotToggleBreakpoint()
{
    if (!actionCollection()->action(QStringLiteral("continue"))->isEnabled()) {
        m_debugView->slotInterrupt();
    } else {
        KTextEditor::View *editView = m_mainWin->activeView();
        QUrl currURL = editView->document()->url();
        int line = editView->cursorPosition().line();

        m_debugView->toggleBreakpoint(currURL, line + 1);
    }
}

void KatePluginGDBView::slotBreakpointSet(const QUrl &file, int line)
{
    KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(m_kateApplication->findUrl(file));

    if (iface) {
        iface->addMark(line, KTextEditor::MarkInterface::BreakpointActive);
    }
}

void KatePluginGDBView::slotBreakpointCleared(const QUrl &file, int line)
{
    KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface *>(m_kateApplication->findUrl(file));

    if (iface) {
        iface->removeMark(line, KTextEditor::MarkInterface::BreakpointActive);
    }
}

void KatePluginGDBView::slotMovePC()
{
    KTextEditor::View *editView = m_mainWin->activeView();
    QUrl currURL = editView->document()->url();
    KTextEditor::Cursor cursor = editView->cursorPosition();

    m_debugView->movePC(currURL, cursor.line() + 1);
}

void KatePluginGDBView::slotRunToCursor()
{
    KTextEditor::View *editView = m_mainWin->activeView();
    QUrl currURL = editView->document()->url();
    KTextEditor::Cursor cursor = editView->cursorPosition();

    // GDB starts lines from 1, kate returns lines starting from 0 (displaying 1)
    m_debugView->runToCursor(currURL, cursor.line() + 1);
}

void KatePluginGDBView::slotGoTo(const QUrl &url, int lineNum)
{
    // skip not existing files
    if (!QFile::exists(url.toLocalFile())) {
        m_lastExecLine = -1;
        return;
    }

    m_lastExecUrl = url;
    m_lastExecLine = lineNum;

    KTextEditor::View *editView = m_mainWin->openUrl(m_lastExecUrl);
    editView->setCursorPosition(KTextEditor::Cursor(m_lastExecLine, 0));
    m_mainWin->window()->raise();
    m_mainWin->window()->setFocus();
}

void KatePluginGDBView::enableDebugActions(bool enable)
{
    const bool canMove = m_debugView->canMove();
    actionCollection()->action(QStringLiteral("step_in"))->setEnabled(enable && canMove);
    actionCollection()->action(QStringLiteral("step_over"))->setEnabled(enable && canMove);
    actionCollection()->action(QStringLiteral("step_out"))->setEnabled(enable && canMove);
    actionCollection()->action(QStringLiteral("move_pc"))->setEnabled(enable && m_debugView->supportsMovePC());
    actionCollection()->action(QStringLiteral("run_to_cursor"))->setEnabled(enable && m_debugView->supportsRunToCursor());
    actionCollection()->action(QStringLiteral("popup_gdb"))->setEnabled(enable);
    actionCollection()->action(QStringLiteral("continue"))->setEnabled(enable && m_debugView->canContinue());
    actionCollection()->action(QStringLiteral("print_value"))->setEnabled(enable);

    // "toggle breakpoint" doubles as interrupt while the program is running
    actionCollection()->action(QStringLiteral("toggle_breakpoint"))->setEnabled(m_debugView->canSetBreakpoints());
    actionCollection()->action(QStringLiteral("kill"))->setEnabled(m_debugView->debuggerRunning());
    actionCollection()->action(QStringLiteral("rerun"))->setEnabled(m_debugView->debuggerRunning());

    m_inputArea->setEnabled(enable && !m_debugView->debuggerBusy());
    m_threadCombo->setEnabled(enable);
    m_stackTree->setEnabled(enable);
    m_scopeCombo->setEnabled(enable);
    m_localsView->setEnabled(enable);

    if (enable) {
        m_inputArea->setFocusPolicy(Qt::WheelFocus);

        if (m_focusOnInput || m_configView->takeFocusAlways()) {
            m_inputArea->setFocus();
            m_focusOnInput = false;
        } else {
            if (m_mainWin->activeView()) {
                m_mainWin->activeView()->setFocus();
            }
        }
    } else {
        m_inputArea->setFocusPolicy(Qt::NoFocus);
        if (m_mainWin->activeView()) {
            m_mainWin->activeView()->setFocus();
        }
    }

    m_ioView->enableInput(!enable && m_debugView->debuggerRunning());

    if ((m_lastExecLine > -1)) {
        KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(m_kateApplication->findUrl(m_lastExecUrl));

        if (iface) {
            if (enable) {
                iface->setMarkDescription(KTextEditor::MarkInterface::Execution, i18n("Execution point"));
                iface->setMarkIcon(KTextEditor::MarkInterface::Execution, QIcon::fromTheme(QStringLiteral("arrow-right")));
                iface->addMark(m_lastExecLine, KTextEditor::MarkInterface::Execution);
            } else {
                iface->removeMark(m_lastExecLine, KTextEditor::MarkInterface::Execution);
            }
        }
    }
}

void KatePluginGDBView::programEnded()
{
    // don't set the execution mark on exit
    m_lastExecLine = -1;
    m_stackTree->clear();
    m_scopeCombo->clear();
    m_localsView->clear();
    m_threadCombo->clear();

    // Indicate the state change by showing the debug outputArea
    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);
}

void KatePluginGDBView::gdbEnded()
{
    m_outputArea->clear();
    m_localsView->clear();
    m_ioView->clearOutput();
    clearMarks();
}

void KatePluginGDBView::clearMarks()
{
    KTextEditor::MarkInterface *iface;
    const auto documents = m_kateApplication->documents();
    for (KTextEditor::Document *doc : documents) {
        iface = qobject_cast<KTextEditor::MarkInterface *>(doc);
        if (iface) {
            const QHash<int, KTextEditor::Mark *> marks = iface->marks();
            QHashIterator<int, KTextEditor::Mark *> i(marks);
            while (i.hasNext()) {
                i.next();
                if ((i.value()->type == KTextEditor::MarkInterface::Execution) || (i.value()->type == KTextEditor::MarkInterface::BreakpointActive)) {
                    iface->removeMark(i.value()->line, i.value()->type);
                }
            }
        }
    }
}

void KatePluginGDBView::slotSendCommand()
{
    QString cmd = m_inputArea->currentText();

    if (cmd.isEmpty()) {
        cmd = m_lastCommand;
    }

    m_inputArea->addToHistory(cmd);
    m_inputArea->setCurrentItem(QString());
    m_focusOnInput = true;
    m_lastCommand = cmd;
    m_debugView->issueCommand(cmd);

    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void KatePluginGDBView::insertStackFrame(int level, const QString &info)
{
    if (level < 0) {
        m_stackTree->resizeColumnToContents(2);
        return;
    }

    if (level == 0) {
        m_stackTree->clear();
    }
    QStringList columns;
    columns << QStringLiteral("  "); // icon place holder
    columns << QString::number(level);
    int lastSpace = info.lastIndexOf(QLatin1Char(' '));
    QString shortInfo = info.mid(lastSpace);
    columns << shortInfo;

    QTreeWidgetItem *item = new QTreeWidgetItem(columns);
    item->setToolTip(2, QStringLiteral("<qt>%1<qt>").arg(info));
    m_stackTree->insertTopLevelItem(level, item);
}

void KatePluginGDBView::stackFrameSelected()
{
    m_debugView->changeStackFrame(m_stackTree->currentIndex().row());
}

void KatePluginGDBView::stackFrameChanged(int level)
{
    QTreeWidgetItem *current = m_stackTree->topLevelItem(m_lastExecFrame);
    QTreeWidgetItem *next = m_stackTree->topLevelItem(level);

    if (current) {
        current->setIcon(0, QIcon());
    }
    if (next) {
        next->setIcon(0, QIcon::fromTheme(QStringLiteral("arrow-right")));
    }
    m_lastExecFrame = level;
}

void KatePluginGDBView::insertScopes(const QList<dap::Scope> &scopes, std::optional<int> activeId)
{
    m_scopeCombo->clear();

    int selected = -1;
    int index = 0;
    for (const auto &scope : scopes) {
        QString name = scope.expensive.value_or(false) ? QStringLiteral("%1!").arg(scope.name) : scope.name;
        if (activeId && (activeId == scope.variablesReference)) {
            selected = index;
        }
        m_scopeCombo->addItem(QIcon::fromTheme(QStringLiteral("")).pixmap(10, 10), scope.name, scope.variablesReference);
        ++index;
    }
    if (selected >= 0) {
        m_scopeCombo->setCurrentIndex(selected);
    }
}

void KatePluginGDBView::scopeSelected(int scope)
{
    if (scope < 0)
        return;
    m_debugView->changeScope(m_scopeCombo->itemData(scope).toInt());
}

void KatePluginGDBView::insertThread(const dap::Thread &thread, bool active)
{
    if (thread.id < 0) {
        m_threadCombo->clear();
        m_activeThread = -1;
        return;
    }
    QString text = i18n("Thread %1", thread.id);
    if (!thread.name.isEmpty()) {
        text += QStringLiteral(": %1").arg(thread.name);
    }
    if (!active) {
        m_threadCombo->addItem(QIcon::fromTheme(QStringLiteral("")).pixmap(10, 10), text, thread.id);
    } else {
        m_threadCombo->addItem(QIcon::fromTheme(QStringLiteral("arrow-right")).pixmap(10, 10), text, thread.id);
        m_activeThread = m_threadCombo->count() - 1;
    }
    m_threadCombo->setCurrentIndex(m_activeThread);
}

void KatePluginGDBView::threadSelected(int thread)
{
    if (thread < 0)
        return;
    m_debugView->changeThread(m_threadCombo->itemData(thread).toInt());
}

QString KatePluginGDBView::currentWord()
{
    KTextEditor::View *kv = m_mainWin->activeView();
    if (!kv) {
        qDebug() << "no KTextEditor::View";
        return QString();
    }

    if (!kv->cursorPosition().isValid()) {
        qDebug() << "cursor not valid!";
        return QString();
    }

    int line = kv->cursorPosition().line();
    int col = kv->cursorPosition().column();

    QString linestr = kv->document()->line(line);

    int startPos = qMax(qMin(col, linestr.length() - 1), 0);
    int lindex = linestr.length() - 1;
    int endPos = startPos;
    while (startPos >= 0
           && (linestr[startPos].isLetterOrNumber() || linestr[startPos] == QLatin1Char('_') || linestr[startPos] == QLatin1Char('~')
               || ((startPos > 1) && (linestr[startPos] == QLatin1Char('.')) && !linestr[startPos - 1].isSpace())
               || ((startPos > 2) && (linestr[startPos] == QLatin1Char('>')) && (linestr[startPos - 1] == QLatin1Char('-'))
                   && !linestr[startPos - 2].isSpace()))) {
        if (linestr[startPos] == QLatin1Char('>')) {
            startPos--;
        }
        startPos--;
    }
    while (
        endPos < linestr.length()
        && (linestr[endPos].isLetterOrNumber() || linestr[endPos] == QLatin1Char('_')
            || ((endPos < lindex - 1) && (linestr[endPos] == QLatin1Char('.')) && !linestr[endPos + 1].isSpace())
            || ((endPos < lindex - 2) && (linestr[endPos] == QLatin1Char('-')) && (linestr[endPos + 1] == QLatin1Char('>')) && !linestr[endPos + 2].isSpace())
            || ((endPos > 1) && (linestr[endPos - 1] == QLatin1Char('-')) && (linestr[endPos] == QLatin1Char('>'))))) {
        if (linestr[endPos] == QLatin1Char('-')) {
            endPos++;
        }
        endPos++;
    }
    if (startPos == endPos) {
        qDebug() << "no word found!";
        return QString();
    }

    // qDebug() << linestr.mid(startPos+1, endPos-startPos-1);
    return linestr.mid(startPos + 1, endPos - startPos - 1);
}

void KatePluginGDBView::slotValue()
{
    QString variable;
    KTextEditor::View *editView = m_mainWin->activeView();
    if (editView && editView->selection() && editView->selectionRange().onSingleLine()) {
        variable = editView->selectionText();
    }

    if (variable.isEmpty()) {
        variable = currentWord();
    }

    if (variable.isEmpty()) {
        return;
    }

    m_inputArea->addToHistory(m_debugView->slotPrintVariable(variable));
    m_inputArea->setCurrentItem(QString());

    m_mainWin->showToolView(m_toolView.get());
    m_tabWidget->setCurrentWidget(m_gdbPage);

    QScrollBar *sb = m_outputArea->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void KatePluginGDBView::showIO(bool show)
{
    if (show) {
        m_tabWidget->addTab(m_ioView.get(), i18n("IO"));
    } else {
        m_tabWidget->removeTab(m_tabWidget->indexOf(m_ioView.get()));
    }
}

void KatePluginGDBView::addOutput(const dap::Output &output)
{
    if (output.isSpecialOutput()) {
        addErrorText(output.output);
        return;
    }
    if (m_configView->showIOTab()) {
        if (output.category == dap::Output::Category::Stdout) {
            m_ioView->addStdOutText(output.output);
        } else {
            m_ioView->addStdOutText(output.output);
        }
    } else {
        if (output.category == dap::Output::Category::Stdout) {
            addOutputText(output.output);
        } else {
            addErrorText(output.output);
        }
    }
}

void KatePluginGDBView::addOutputText(QString const &text)
{
    QScrollBar *scrollb = m_outputArea->verticalScrollBar();
    if (!scrollb) {
        return;
    }
    bool atEnd = (scrollb->value() == scrollb->maximum());

    QTextCursor cursor = m_outputArea->textCursor();
    if (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::End);
    }
    cursor.insertText(text);

    if (atEnd) {
        scrollb->setValue(scrollb->maximum());
    }
}

void KatePluginGDBView::addErrorText(QString const &text)
{
    m_outputArea->setFontItalic(true);
    addOutputText(text);
    m_outputArea->setFontItalic(false);
}

bool KatePluginGDBView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if ((obj == m_toolView.get()) && (ke->key() == Qt::Key_Escape)) {
            m_mainWin->hideToolView(m_toolView.get());
            event->accept();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void KatePluginGDBView::handleEsc(QEvent *e)
{
    if (!m_mainWin || !m_toolView) {
        return;
    }

    QKeyEvent *k = static_cast<QKeyEvent *>(e);
    if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
        if (m_toolView->isVisible()) {
            m_mainWin->hideToolView(m_toolView.get());
        }
    }
}

void KatePluginGDBView::enableBreakpointMarks(KTextEditor::Document *document)
{
    KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(document);

    if (iface) {
        iface->setEditableMarks(iface->editableMarks() | KTextEditor::MarkInterface::BreakpointActive);
        iface->setMarkDescription(KTextEditor::MarkInterface::BreakpointActive, i18n("Breakpoint"));
        iface->setMarkIcon(KTextEditor::MarkInterface::BreakpointActive, QIcon::fromTheme(QStringLiteral("breakpoint")));
    }
}

void KatePluginGDBView::displayMessage(const QString &msg, KTextEditor::Message::MessageType level)
{
    KTextEditor::View *kv = m_mainWin->activeView();
    if (!kv) {
        return;
    }

    delete m_infoMessage;
    m_infoMessage = new KTextEditor::Message(msg, level);
    m_infoMessage->setWordWrap(true);
    m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
    m_infoMessage->setAutoHide(8000);
    m_infoMessage->setAutoHideMode(KTextEditor::Message::Immediate);
    m_infoMessage->setView(kv);
    kv->document()->postMessage(m_infoMessage);
}

#include "plugin_kategdb.moc"
