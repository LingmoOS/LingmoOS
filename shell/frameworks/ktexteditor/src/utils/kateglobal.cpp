/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2009 Erlend Hamberg <ehamberg@gmail.com>

    For the addScrollablePage original
    SPDX-FileCopyrightText: 2003 Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2004 Michael Brade <brade@kde.org>
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateglobal.h"

#include <ktexteditor_version.h>

#include "katebuffer.h"
#include "katecmd.h"
#include "katecmds.h"
#include "kateconfig.h"
#include "katedialogs.h"
#include "katedocument.h"
#include "katehighlightingcmds.h"
#include "katekeywordcompletion.h"
#include "katemodemanager.h"
#include "katescriptmanager.h"
#include "katesedcmd.h"
#include "katesyntaxmanager.h"
#include "katethemeconfig.h"
#include "katevariableexpansionmanager.h"
#include "kateview.h"
#include "katewordcompletion.h"
#include "spellcheck/spellcheck.h"

#include "katenormalinputmodefactory.h"
#include "kateviinputmodefactory.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KPageDialog>

#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QFrame>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringListModel>
#include <QTextToSpeech>
#include <QTimer>

KTextEditor::EditorPrivate::EditorPrivate(QPointer<KTextEditor::EditorPrivate> &staticInstance)
    : KTextEditor::Editor(this)
    , m_aboutData(QStringLiteral("katepart"),
                  i18n("Kate Part"),
                  QStringLiteral(KTEXTEDITOR_VERSION_STRING),
                  i18n("Embeddable editor component"),
                  KAboutLicense::LGPL_V2,
                  i18n("(c) 2000-2022 The Kate Authors"),
                  QString(),
                  QStringLiteral("https://kate-editor.org"))
    , m_dummyApplication(nullptr)
    , m_application(&m_dummyApplication)
    , m_dummyMainWindow(nullptr)
    , m_searchHistoryModel(nullptr)
    , m_replaceHistoryModel(nullptr)
{
    // remember this
    staticInstance = this;

    // register some datatypes
    qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");
    qRegisterMetaType<KTextEditor::Document *>("KTextEditor::Document*");
    qRegisterMetaType<KTextEditor::View *>("KTextEditor::View*");

    //
    // fill about data
    //
    m_aboutData.addAuthor(i18n("Christoph Cullmann"), i18n("Maintainer"), QStringLiteral("cullmann@kde.org"), QStringLiteral("https://cullmann.io"));
    m_aboutData.addAuthor(i18n("Dominik Haumann"), i18n("Core Developer"), QStringLiteral("dhaumann@kde.org"));
    m_aboutData.addAuthor(i18n("Milian Wolff"), i18n("Core Developer"), QStringLiteral("mail@milianw.de"), QStringLiteral("https://milianw.de/"));
    m_aboutData.addAuthor(i18n("Joseph Wenninger"),
                          i18n("Core Developer"),
                          QStringLiteral("jowenn@kde.org"),
                          QStringLiteral("http://stud3.tuwien.ac.at/~e9925371"));
    m_aboutData.addAuthor(i18n("Erlend Hamberg"), i18n("Vi Input Mode"), QStringLiteral("ehamberg@gmail.com"), QStringLiteral("https://hamberg.no/erlend"));
    m_aboutData.addAuthor(i18n("Bernhard Beschow"),
                          i18n("Developer"),
                          QStringLiteral("bbeschow@cs.tu-berlin.de"),
                          QStringLiteral("https://user.cs.tu-berlin.de/~bbeschow"));
    m_aboutData.addAuthor(i18n("Anders Lund"), i18n("Core Developer"), QStringLiteral("anders@alweb.dk"), QStringLiteral("https://alweb.dk"));
    m_aboutData.addAuthor(i18n("Michel Ludwig"), i18n("On-the-fly spell checking"), QStringLiteral("michel.ludwig@kdemail.net"));
    m_aboutData.addAuthor(i18n("Pascal Létourneau"), i18n("Large scale bug fixing"), QStringLiteral("pascal.letourneau@gmail.com"));
    m_aboutData.addAuthor(i18n("Hamish Rodda"), i18n("Core Developer"), QStringLiteral("rodda@kde.org"));
    m_aboutData.addAuthor(i18n("Waldo Bastian"), i18n("The cool buffersystem"), QStringLiteral("bastian@kde.org"));
    m_aboutData.addAuthor(i18n("Charles Samuels"), i18n("The Editing Commands"), QStringLiteral("charles@kde.org"));
    m_aboutData.addAuthor(i18n("Matt Newell"), i18n("Testing, ..."), QStringLiteral("newellm@proaxis.com"));
    m_aboutData.addAuthor(i18n("Michael Bartl"), i18n("Former Core Developer"), QStringLiteral("michael.bartl1@chello.at"));
    m_aboutData.addAuthor(i18n("Michael McCallum"), i18n("Core Developer"), QStringLiteral("gholam@xtra.co.nz"));
    m_aboutData.addAuthor(i18n("Michael Koch"), i18n("KWrite port to KParts"), QStringLiteral("koch@kde.org"));
    m_aboutData.addAuthor(i18n("Christian Gebauer"), QString(), QStringLiteral("gebauer@kde.org"));
    m_aboutData.addAuthor(i18n("Simon Hausmann"), QString(), QStringLiteral("hausmann@kde.org"));
    m_aboutData.addAuthor(i18n("Glen Parker"), i18n("KWrite Undo History, Kspell integration"), QStringLiteral("glenebob@nwlink.com"));
    m_aboutData.addAuthor(i18n("Scott Manson"), i18n("KWrite XML Syntax highlighting support"), QStringLiteral("sdmanson@alltel.net"));
    m_aboutData.addAuthor(i18n("John Firebaugh"), i18n("Patches and more"), QStringLiteral("jfirebaugh@kde.org"));
    m_aboutData.addAuthor(i18n("Andreas Kling"), i18n("Developer"), QStringLiteral("kling@impul.se"));
    m_aboutData.addAuthor(i18n("Mirko Stocker"), i18n("Various bugfixes"), QStringLiteral("me@misto.ch"), QStringLiteral("https://misto.ch/"));
    m_aboutData.addAuthor(i18n("Matthew Woehlke"), i18n("Selection, KColorScheme integration"), QStringLiteral("mw_triad@users.sourceforge.net"));
    m_aboutData.addAuthor(i18n("Sebastian Pipping"),
                          i18n("Search bar back- and front-end"),
                          QStringLiteral("webmaster@hartwork.org"),
                          QStringLiteral("https://hartwork.org/"));
    m_aboutData.addAuthor(i18n("Jochen Wilhelmy"), i18n("Original KWrite Author"), QStringLiteral("digisnap@cs.tu-berlin.de"));
    m_aboutData.addAuthor(i18n("Gerald Senarclens de Grancy"),
                          i18n("QA and Scripting"),
                          QStringLiteral("oss@senarclens.eu"),
                          QStringLiteral("http://find-santa.eu/"));

    m_aboutData.addCredit(i18n("Matteo Merli"), i18n("Highlighting for RPM Spec-Files, Perl, Diff and more"), QStringLiteral("merlim@libero.it"));
    m_aboutData.addCredit(i18n("Rocky Scaletta"), i18n("Highlighting for VHDL"), QStringLiteral("rocky@purdue.edu"));
    m_aboutData.addCredit(i18n("Yury Lebedev"), i18n("Highlighting for SQL"), QString());
    m_aboutData.addCredit(i18n("Chris Ross"), i18n("Highlighting for Ferite"), QString());
    m_aboutData.addCredit(i18n("Nick Roux"), i18n("Highlighting for ILERPG"), QString());
    m_aboutData.addCredit(i18n("Carsten Niehaus"), i18n("Highlighting for LaTeX"), QString());
    m_aboutData.addCredit(i18n("Per Wigren"), i18n("Highlighting for Makefiles, Python"), QString());
    m_aboutData.addCredit(i18n("Jan Fritz"), i18n("Highlighting for Python"), QString());
    m_aboutData.addCredit(i18n("Daniel Naber"));
    m_aboutData.addCredit(i18n("Roland Pabel"), i18n("Highlighting for Scheme"), QString());
    m_aboutData.addCredit(i18n("Cristi Dumitrescu"), i18n("PHP Keyword/Datatype list"), QString());
    m_aboutData.addCredit(i18n("Carsten Pfeiffer"), i18n("Very nice help"), QString());
    m_aboutData.addCredit(i18n("Bruno Massa"), i18n("Highlighting for Lua"), QStringLiteral("brmassa@gmail.com"));

    m_aboutData.addCredit(i18n("All people who have contributed and I have forgotten to mention"));

    m_aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    // set proper Kate icon for our about dialog
    m_aboutData.setProgramLogo(QIcon(QStringLiteral(":/ktexteditor/kate.svg")));

    //
    // dir watch
    //
    m_dirWatch = new KDirWatch();

    //
    // command manager
    //
    m_cmdManager = new KateCmd();

    //
    // variable expansion manager
    //
    m_variableExpansionManager = new KateVariableExpansionManager(this);

    //
    // hl manager
    //
    m_hlManager = new KateHlManager();

    //
    // mode man
    //
    m_modeManager = new KateModeManager();

    //
    // input mode factories
    //
    Q_ASSERT(m_inputModeFactories.size() == KTextEditor::View::ViInputMode + 1);
    m_inputModeFactories[KTextEditor::View::NormalInputMode].reset(new KateNormalInputModeFactory());
    m_inputModeFactories[KTextEditor::View::ViInputMode].reset(new KateViInputModeFactory());

    //
    // spell check manager
    //
    m_spellCheckManager = new KateSpellCheckManager();

    // config objects
    m_globalConfig = new KateGlobalConfig();
    m_documentConfig = new KateDocumentConfig();
    m_viewConfig = new KateViewConfig();
    m_rendererConfig = new KateRendererConfig();

    // create script manager (search scripts)
    m_scriptManager = KateScriptManager::self();

    //
    // init the cmds
    //
    m_cmds = {
        KateCommands::CoreCommands::self(),
        KateCommands::Character::self(),
        KateCommands::Date::self(),
        KateCommands::SedReplace::self(),
        KateCommands::Highlighting::self(),
    };

    // global word completion model
    m_wordCompletionModel = new KateWordCompletionModel(this);

    // global keyword completion model
    m_keywordCompletionModel = new KateKeywordCompletionModel(this);

    // tap to QApplication object for color palette changes
    qApp->installEventFilter(this);
}

KTextEditor::EditorPrivate::~EditorPrivate()
{
    delete m_globalConfig;
    delete m_documentConfig;
    delete m_viewConfig;
    delete m_rendererConfig;

    delete m_modeManager;

    delete m_dirWatch;

    // cu managers
    delete m_scriptManager;
    delete m_hlManager;

    delete m_spellCheckManager;

    // cu model
    delete m_wordCompletionModel;

    // delete variable expansion manager
    delete m_variableExpansionManager;
    m_variableExpansionManager = nullptr;

    // delete the commands before we delete the cmd manager
    qDeleteAll(m_cmds);
    delete m_cmdManager;
}

KTextEditor::Document *KTextEditor::EditorPrivate::createDocument(QObject *parent)
{
    KTextEditor::DocumentPrivate *doc = new KTextEditor::DocumentPrivate(false, false, nullptr, parent);

    Q_EMIT documentCreated(this, doc);

    return doc;
}

// END KTextEditor::Editor config stuff

// config dialog with improved sizing and that allows scrolling
class KTextEditorConfigDialog : public KPageDialog
{
public:
    std::vector<KTextEditor::ConfigPage *> editorPages;

    KTextEditorConfigDialog(KTextEditor::EditorPrivate *editor, QWidget *parent)
        : KPageDialog(parent)
    {
        setWindowTitle(i18n("Configure"));
        setFaceType(KPageDialog::List);
        setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply | QDialogButtonBox::Help);

        // create pages already in construct to have proper layout for sizeHint
        editorPages.reserve(editor->configPages());
        for (int i = 0; i < editor->configPages(); ++i) {
            KTextEditor::ConfigPage *page = editor->configPage(i, this);
            KPageWidgetItem *item = addScrollablePage(page, page->name());
            item->setHeader(page->fullName());
            item->setIcon(page->icon());

            connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, page, &KTextEditor::ConfigPage::apply);
            editorPages.push_back(page);
        }
    }

    QSize sizeHint() const override
    {
        // start with a bit enlarged default size hint to minimize changes of useless scrollbars
        QSize size = KPageDialog::sizeHint() * 1.3;

        // enlarge it to half of the main window size, if that is larger
        if (parentWidget() && parentWidget()->window()) {
            size = size.expandedTo(parentWidget()->window()->size() * 0.5);
        }

        // return bounded size to available real screen space
        return size.boundedTo(screen()->availableSize() * 0.9);
    }

    KPageWidgetItem *addScrollablePage(QWidget *page, const QString &itemName)
    {
        // inspired by KPageWidgetItem *KConfigDialogPrivate::addPageInternal(QWidget *page, const QString &itemName, const QString &pixmapName, const QString
        // &header)
        QWidget *frame = new QWidget;
        QVBoxLayout *boxLayout = new QVBoxLayout(frame);
        boxLayout->setContentsMargins(0, 0, 0, 0);
        boxLayout->setContentsMargins(0, 0, 0, 0);

        QScrollArea *scroll = new QScrollArea;
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setWidget(page);
        scroll->setWidgetResizable(true);
        scroll->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        if (page->minimumSizeHint().height() > scroll->sizeHint().height() - 2) {
            if (page->sizeHint().width() < scroll->sizeHint().width() + 2) {
                // QScrollArea is planning only a vertical scroll bar,
                // try to avoid the horizontal one by reserving space for the vertical one.
                // Currently KPageViewPrivate::_k_modelChanged() queries the minimumSizeHint().
                // We can only set the minimumSize(), so this approach relies on QStackedWidget size calculation.
                scroll->setMinimumWidth(scroll->sizeHint().width() + qBound(0, scroll->verticalScrollBar()->sizeHint().width(), 200) + 4);
            }
        }

        boxLayout->addWidget(scroll);
        return addPage(frame, itemName);
    }
};

void KTextEditor::EditorPrivate::configDialog(QWidget *parent)
{
    QPointer<KTextEditorConfigDialog> kd = new KTextEditorConfigDialog(this, parent);
    if (kd->exec() && kd) {
        KateGlobalConfig::global()->configStart();
        KateDocumentConfig::global()->configStart();
        KateViewConfig::global()->configStart();
        KateRendererConfig::global()->configStart();

        for (auto *page : kd->editorPages) {
            page->apply();
        }

        KateGlobalConfig::global()->configEnd();
        KateDocumentConfig::global()->configEnd();
        KateViewConfig::global()->configEnd();
        KateRendererConfig::global()->configEnd();
    }
    delete kd;
}

int KTextEditor::EditorPrivate::configPages() const
{
    return 4;
}

KTextEditor::ConfigPage *KTextEditor::EditorPrivate::configPage(int number, QWidget *parent)
{
    switch (number) {
    case 0:
        return new KateViewDefaultsConfig(parent);

    case 1:
        return new KateThemeConfigPage(parent);

    case 2:
        return new KateEditConfigTab(parent);

    case 3:
        return new KateSaveConfigTab(parent);

    default:
        break;
    }

    return nullptr;
}

/**
 * Cleanup the KTextEditor::EditorPrivate during QCoreApplication shutdown
 */
static void cleanupGlobal()
{
    // delete if there
    delete KTextEditor::EditorPrivate::self();
}

KTextEditor::EditorPrivate *KTextEditor::EditorPrivate::self()
{
    // remember the static instance in a QPointer
    static bool inited = false;
    static QPointer<KTextEditor::EditorPrivate> staticInstance;

    // just return it, if already inited
    if (inited) {
        return staticInstance.data();
    }

    // start init process
    inited = true;

    // now create the object and store it
    new KTextEditor::EditorPrivate(staticInstance);

    // register cleanup
    // let use be deleted during QCoreApplication shutdown
    qAddPostRoutine(cleanupGlobal);

    // return instance
    return staticInstance.data();
}

void KTextEditor::EditorPrivate::registerDocument(KTextEditor::DocumentPrivate *doc)
{
    Q_ASSERT(!m_documents.contains(doc));
    m_documents.push_back(doc);
}

void KTextEditor::EditorPrivate::deregisterDocument(KTextEditor::DocumentPrivate *doc)
{
    int i = m_documents.indexOf(doc);
    Q_ASSERT(i != -1);
    m_documents.removeAt(i);
}

void KTextEditor::EditorPrivate::registerView(KTextEditor::ViewPrivate *view)
{
    Q_ASSERT(std::find(m_views.begin(), m_views.end(), view) == m_views.end());
    m_views.push_back(view);
}

void KTextEditor::EditorPrivate::deregisterView(KTextEditor::ViewPrivate *view)
{
    auto it = std::find(m_views.begin(), m_views.end(), view);
    Q_ASSERT(it != m_views.end());
    m_views.erase(it);
}

KTextEditor::Command *KTextEditor::EditorPrivate::queryCommand(const QString &cmd) const
{
    return m_cmdManager->queryCommand(cmd);
}

QList<KTextEditor::Command *> KTextEditor::EditorPrivate::commands() const
{
    return m_cmdManager->commands();
}

QStringList KTextEditor::EditorPrivate::commandList() const
{
    return m_cmdManager->commandList();
}

KateVariableExpansionManager *KTextEditor::EditorPrivate::variableExpansionManager()
{
    return m_variableExpansionManager;
}

void KTextEditor::EditorPrivate::updateColorPalette()
{
    // reload the global schema (triggers reload for every view as well)
    // might trigger selection of better matching theme for new palette
    m_rendererConfig->reloadSchema();

    // force full update of all view caches and colors
    m_rendererConfig->updateConfig();
}

void KTextEditor::EditorPrivate::copyToClipboard(const QString &text, const QString &fileName)
{
    // empty => nop
    if (text.isEmpty()) {
        return;
    }

    // move to clipboard
    QApplication::clipboard()->setText(text, QClipboard::Clipboard);

    // LRU, kill potential duplicated, move new entry to top
    // cut after X entries
    auto entry = ClipboardEntry{.text=text, .fileName=fileName};
    m_clipboardHistory.removeOne(entry);
    m_clipboardHistory.prepend(entry);
    if (m_clipboardHistory.size() > m_viewConfig->clipboardHistoryEntries()) {
        m_clipboardHistory.removeLast();
    }

    // notify about change
    Q_EMIT clipboardHistoryChanged();
}

bool KTextEditor::EditorPrivate::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == qApp && event->type() == QEvent::ApplicationPaletteChange) {
        // only update the color once for the event that belongs to the qApp
        updateColorPalette();
    }

    return false; // always continue processing
}

QStringListModel *KTextEditor::EditorPrivate::searchHistoryModel()
{
    if (!m_searchHistoryModel) {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Search"));
        const QStringList history = cg.readEntry(QStringLiteral("Search History"), QStringList());
        m_searchHistoryModel = new QStringListModel(history, this);
    }
    return m_searchHistoryModel;
}

QStringListModel *KTextEditor::EditorPrivate::replaceHistoryModel()
{
    if (!m_replaceHistoryModel) {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Search"));
        const QStringList history = cg.readEntry(QStringLiteral("Replace History"), QStringList());
        m_replaceHistoryModel = new QStringListModel(history, this);
    }
    return m_replaceHistoryModel;
}

void KTextEditor::EditorPrivate::saveSearchReplaceHistoryModels()
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KTextEditor::Search"));
    if (m_searchHistoryModel) {
        cg.writeEntry(QStringLiteral("Search History"), m_searchHistoryModel->stringList());
    }
    if (m_replaceHistoryModel) {
        cg.writeEntry(QStringLiteral("Replace History"), m_replaceHistoryModel->stringList());
    }
}

KSharedConfigPtr KTextEditor::EditorPrivate::config()
{
    // use dummy config for unit tests!
    if (QStandardPaths::isTestModeEnabled()) {
        return KSharedConfig::openConfig(QStringLiteral("katepartrc-unittest"), KConfig::SimpleConfig, QStandardPaths::TempLocation);
    }

    // else: use application configuration, but try to transfer global settings on first use
    auto applicationConfig = KSharedConfig::openConfig();
    if (!KConfigGroup(applicationConfig, QStringLiteral("KTextEditor Editor")).exists()) {
        auto globalConfig = KSharedConfig::openConfig(QStringLiteral("katepartrc"));
        for (const auto &group : {QStringLiteral("Editor"), QStringLiteral("Document"), QStringLiteral("View"), QStringLiteral("Renderer")}) {
            KConfigGroup origin(globalConfig, group);
            KConfigGroup destination(applicationConfig, QStringLiteral("KTextEditor ") + group);
            origin.copyTo(&destination);
        }
    }
    return applicationConfig;
}

void KTextEditor::EditorPrivate::triggerConfigChanged()
{
    // trigger delayed emission, will collapse multiple events to one signal emission
    m_configWasChanged = true;
    QTimer::singleShot(0, this, &KTextEditor::EditorPrivate::emitConfigChanged);
}

void KTextEditor::EditorPrivate::emitConfigChanged()
{
    // emit only once, if still needed
    if (m_configWasChanged) {
        m_configWasChanged = false;
        Q_EMIT configChanged(this);
    }
}

void KTextEditor::EditorPrivate::copyToMulticursorClipboard(const QStringList &texts)
{
    if (texts.size() == 1) {
        qWarning() << "Unexpected size 1 of multicursorClipboard. It should either be empty or greater than 1";
        m_multicursorClipboard = QStringList();
        Q_ASSERT(false);
        return;
    }
    m_multicursorClipboard = texts;
}

QStringList KTextEditor::EditorPrivate::multicursorClipboard() const
{
    return m_multicursorClipboard;
}

QTextToSpeech *KTextEditor::EditorPrivate::speechEngine(KTextEditor::ViewPrivate *view)
{
    Q_ASSERT(view);
    if (!m_speechEngine) {
        m_speechEngine = new QTextToSpeech(this);

        // error handler for errors happening during speech output
        connect(m_speechEngine, &QTextToSpeech::errorOccurred, this, [this](QTextToSpeech::ErrorReason, const QString &errorString) {
            if (m_speechEngineLastUser) {
                speechError(m_speechEngineLastUser, errorString);
            }
        });

        // handle init errors, that will not emit errorOccurred later
        if (m_speechEngine->errorReason() != QTextToSpeech::ErrorReason::NoError) {
            speechError(view, m_speechEngine->errorString());
        }
    }

    // register current view, handle error output and stopping of the speaking
    if (view != m_speechEngineLastUser) {
        if (m_speechEngineLastUser) {
            disconnect(m_speechEngineLastUser, &QObject::destroyed, this, &KTextEditor::EditorPrivate::speechEngineUserDestoyed);
        }
        m_speechEngineLastUser = view;
        connect(m_speechEngineLastUser, &QObject::destroyed, this, &KTextEditor::EditorPrivate::speechEngineUserDestoyed);
    }

    return m_speechEngine;
}

void KTextEditor::EditorPrivate::speechEngineUserDestoyed()
{
    Q_ASSERT(m_speechEngine);
    m_speechEngine->stop();
}

void KTextEditor::EditorPrivate::speechError(KTextEditor::ViewPrivate *view, const QString &errorString)
{
    Q_ASSERT(view);
    auto message = new KTextEditor::Message(errorString, Message::Error);
    message->setPosition(KTextEditor::Message::TopInView);
    message->setView(view);
    view->document()->postMessage(message);
}

#include "moc_kateglobal.cpp"
