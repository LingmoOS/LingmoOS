/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "lspclientpluginview.h"
#include "gotosymboldialog.h"
#include "lspclientcompletion.h"
#include "lspclienthover.h"
#include "lspclientplugin.h"
#include "lspclientservermanager.h"
#include "lspclientsymbolview.h"
#include "lspclientutils.h"
#include "lsptooltip.h"

#include "lspclient_debug.h"

#include <KAcceleratorManager>
#include <KActionCollection>
#include <KActionMenu>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardAction>
#include <KXMLGUIFactory>

#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KTextEditor/MovingInterface>
#include <KTextEditor/SessionConfigInterface>
#include <KTextEditor/View>
#include <KXMLGUIClient>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/movinginterface.h>
#include <ktexteditor/movingrange.h>
#include <ktexteditor_version.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScopeGuard>
#include <QSet>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QTextCodec>
#include <QTimer>
#include <QTreeView>
#include <unordered_map>
#include <utility>

#include <drawing_utils.h>
#include <ktexteditor_utils.h>

namespace RangeData
{
enum {
    // preserve UserRole for generic use where needed
    FileUrlRole = Qt::UserRole + 1,
    RangeRole,
    KindRole,
};

class KindEnum
{
public:
    enum _kind {
        Text = static_cast<int>(LSPDocumentHighlightKind::Text),
        Read = static_cast<int>(LSPDocumentHighlightKind::Read),
        Write = static_cast<int>(LSPDocumentHighlightKind::Write),
        Error = 10 + static_cast<int>(LSPDiagnosticSeverity::Error),
        Warning = 10 + static_cast<int>(LSPDiagnosticSeverity::Warning),
        Information = 10 + static_cast<int>(LSPDiagnosticSeverity::Information),
        Hint = 10 + static_cast<int>(LSPDiagnosticSeverity::Hint),
        Related
    };

    KindEnum(int v)
    {
        m_value = _kind(v);
    }

    KindEnum(LSPDocumentHighlightKind hl)
        : KindEnum(static_cast<_kind>(hl))
    {
    }

    KindEnum(LSPDiagnosticSeverity sev)
        : KindEnum(_kind(10 + static_cast<int>(sev)))
    {
    }

    operator _kind()
    {
        return m_value;
    }

private:
    _kind m_value;
};

static constexpr KTextEditor::MarkInterface::MarkTypes markType = KTextEditor::MarkInterface::markType31;
static constexpr KTextEditor::MarkInterface::MarkTypes markTypeDiagError = KTextEditor::MarkInterface::Error;
static constexpr KTextEditor::MarkInterface::MarkTypes markTypeDiagWarning = KTextEditor::MarkInterface::Warning;
static constexpr KTextEditor::MarkInterface::MarkTypes markTypeDiagOther = KTextEditor::MarkInterface::markType30;
static constexpr KTextEditor::MarkInterface::MarkTypes markTypeDiagAll =
    KTextEditor::MarkInterface::MarkTypes(markTypeDiagError | markTypeDiagWarning | markTypeDiagOther);

}

static QIcon diagnosticsIcon(LSPDiagnosticSeverity severity)
{
    // clang-format off
#define RETURN_CACHED_ICON(name, fallbackname) \
    { \
        static QIcon icon(QIcon::fromTheme(QStringLiteral(name), \
                                           QIcon::fromTheme(QStringLiteral(fallbackname)))); \
        return icon; \
    }
    // clang-format on
    switch (severity) {
    case LSPDiagnosticSeverity::Error:
        RETURN_CACHED_ICON("data-error", "dialog-error")
    case LSPDiagnosticSeverity::Warning:
        RETURN_CACHED_ICON("data-warning", "dialog-warning")
    case LSPDiagnosticSeverity::Information:
    case LSPDiagnosticSeverity::Hint:
        RETURN_CACHED_ICON("data-information", "dialog-information")
    default:
        break;
    }
    return QIcon();
}

static QIcon codeActionIcon()
{
    static QIcon icon(QIcon::fromTheme(QStringLiteral("insert-text")));
    return icon;
}

KTextEditor::Document *findDocument(KTextEditor::MainWindow *mainWindow, const QUrl &url)
{
    auto views = mainWindow->views();
    for (const auto v : views) {
        auto doc = v->document();
        if (doc && doc->url() == url) {
            return doc;
        }
    }
    return nullptr;
}

// helper to read lines from unopened documents
// lightweight and does not require additional symbols
class FileLineReader
{
    QFile file;
    int lastLineNo = -1;
    QString lastLine;

public:
    FileLineReader(const QUrl &url)
        : file(url.toLocalFile())
    {
        file.open(QIODevice::ReadOnly);
    }

    // called with non-descending lineno
    QString line(int lineno)
    {
        if (lineno == lastLineNo) {
            return lastLine;
        }
        while (file.isOpen() && !file.atEnd()) {
            auto line = file.readLine();
            if (++lastLineNo == lineno) {
                QTextCodec::ConverterState state;
                QTextCodec *codec = QTextCodec::codecForName("UTF-8");
                QString text = codec->toUnicode(line.constData(), line.size(), &state);
                if (state.invalidChars > 0) {
                    text = QString::fromLatin1(line);
                }

                text = text.trimmed();
                lastLine = text;
                return text;
            }
        }
        return QString();
    }
};

class LocationTreeDelegate : public QStyledItemDelegate
{
public:
    LocationTreeDelegate(QObject *parent, const QFont &font)
        : QStyledItemDelegate(parent)
        , m_monoFont(font)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto options = option;
        initStyleOption(&options, index);

        painter->save();

        QString text = index.data().toString();

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        QVector<QTextLayout::FormatRange> formats;
        if (!index.parent().isValid()) {
            int lastSlash = text.lastIndexOf(QLatin1Char('/'));
            if (lastSlash != -1) {
                QTextCharFormat fmt;
                fmt.setFontWeight(QFont::Bold);
                formats.append({lastSlash + 1, int(text.length() - (lastSlash + 1)), fmt});
            }
        } else {
            // mind translation; let's hope/assume the colon survived
            int nextColon = text.indexOf(QLatin1Char(':'), 0);
            if (nextColon != 1 && nextColon < text.size()) {
                nextColon = text.indexOf(QLatin1Char(':'), nextColon + 1);
            }
            if (nextColon != -1) {
                QTextCharFormat fmt;
                fmt.setFont(m_monoFont);
                int codeStart = nextColon + 1;
                formats.append({codeStart, int(text.length() - codeStart), fmt});
            }
        }

        /** There might be an icon? Make sure to not draw over it **/
        auto textRectX = options.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &options, options.widget).x();
        auto width = textRectX - options.rect.x();
        painter->translate(width, 0);

        Utils::paintItemViewText(painter, text, options, formats);

        painter->restore();
    }

private:
    QFont m_monoFont;
};

/**
 * @brief This is just a helper class that provides "underline" on Ctrl + click
 */
class CtrlHoverFeedback : public QObject
{
    Q_OBJECT
public:
    void highlight(KTextEditor::View *activeView)
    {
        // sanity checks
        if (!activeView) {
            return;
        }

        auto doc = activeView->document();
        if (!doc) {
            return;
        }

        if (!w) {
            return;
        }

        w->setCursor(Qt::PointingHandCursor);

        // underline the hovered word
        auto &mr = docs[doc];
        if (mr) {
            mr->setRange(range);
        } else {
            auto miface = qobject_cast<KTextEditor::MovingInterface *>(doc);
            if (!miface) {
                return;
            }
            mr.reset(miface->newMovingRange(range));
            // clang-format off
            connect(doc,
                    SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)),
                    this,
                    SLOT(clear(KTextEditor::Document*)),
                    Qt::UniqueConnection);
            connect(doc,
                    SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)),
                    this,
                    SLOT(clear(KTextEditor::Document*)),
                    Qt::UniqueConnection);
            // clang-format on
        }

        static KTextEditor::Attribute::Ptr attr;
        if (!attr) {
            attr = new KTextEditor::Attribute;
            attr->setUnderlineStyle(QTextCharFormat::SingleUnderline);
        }
        mr->setAttribute(attr);
    }

    void clear(KTextEditor::View *activeView)
    {
        if (activeView) {
            auto doc = activeView->document();
            auto it = docs.find(doc);
            if (it != docs.end()) {
                auto &mr = it->second;
                if (mr) {
                    mr->setRange(KTextEditor::Range::invalid());
                }
            }
        }
        if (w && w->cursor() != Qt::IBeamCursor) {
            w->setCursor(Qt::IBeamCursor);
        }
        w.clear();
    }

    void setRangeAndWidget(const KTextEditor::Range &r, QWidget *wid)
    {
        range = r;
        w = wid;
    }

    bool isValid() const
    {
        return !w.isNull();
    }

private:
    Q_SLOT void clear(KTextEditor::Document *doc)
    {
        if (doc) {
            auto it = docs.find(doc);
            if (it != docs.end()) {
                docs.erase(it);
            }
        }
    }

private:
    QPointer<QWidget> w;
    std::unordered_map<KTextEditor::Document *, std::unique_ptr<KTextEditor::MovingRange>> docs;
    KTextEditor::Range range;
};

class SessionDiagnosticSuppressions
{
    // file -> suppression
    // (empty file matches any file)
    QHash<QString, QSet<QString>> m_suppressions;
    const QString ENTRY_PREFIX{QStringLiteral("File_")};

public:
    void readSessionConfig(const KConfigGroup &cg)
    {
        qCInfo(LSPCLIENT) << "reading session config";
        const auto groups = cg.keyList();
        for (const auto &fkey : groups) {
            if (fkey.startsWith(ENTRY_PREFIX)) {
                QString fname = fkey.mid(ENTRY_PREFIX.size());
                QStringList entries = cg.readEntry(fkey, QStringList());
                if (entries.size()) {
                    m_suppressions[fname] = {entries.begin(), entries.end()};
                }
            }
        }
    }

    void writeSessionConfig(KConfigGroup &cg)
    {
        qCInfo(LSPCLIENT) << "writing session config";
        // clear existing entries
        cg.deleteGroup();
        for (auto it = m_suppressions.begin(); it != m_suppressions.end(); ++it) {
            QStringList entries = it.value().values();
            if (entries.size()) {
                cg.writeEntry(ENTRY_PREFIX + it.key(), entries);
            }
        }
    }

    void add(const QString &file, const QString &diagnostic)
    {
        m_suppressions[file].insert(diagnostic);
    }

    void remove(const QString &file, const QString &diagnostic)
    {
        auto it = m_suppressions.find(file);
        if (it != m_suppressions.end()) {
            it->remove(diagnostic);
        }
    }

    bool hasSuppression(const QString &file, const QString &diagnostic)
    {
        auto it = m_suppressions.find(file);
        if (it != m_suppressions.end()) {
            return it->contains(diagnostic);
        } else {
            return false;
        }
    }

    QVector<QString> getSuppressions(const QString &file)
    {
        QVector<QString> result;

        for (const auto &entry : {QString(), file}) {
            auto it = m_suppressions.find(entry);
            if (it != m_suppressions.end()) {
                const auto ds = it.value();
                for (const auto &d : ds) {
                    result.push_back(d);
                }
            }
        }
        return result;
    }
};

class LSPClientPluginViewImpl : public QObject, public KXMLGUIClient, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

    typedef LSPClientPluginViewImpl self_type;

    LSPClientPlugin *m_plugin;
    KTextEditor::MainWindow *m_mainWindow;
    QSharedPointer<LSPClientServerManager> m_serverManager;
    QScopedPointer<LSPClientViewTracker> m_viewTracker;
    QScopedPointer<LSPClientCompletion> m_completion;
    QScopedPointer<LSPClientHover> m_hover;
    QScopedPointer<KTextEditor::TextHintProvider> m_forwardHover;
    QScopedPointer<LSPClientSymbolView> m_symbolView;

    QPointer<QAction> m_findDef;
    QPointer<QAction> m_findDecl;
    QPointer<QAction> m_findTypeDef;
    QPointer<QAction> m_findRef;
    QPointer<QAction> m_findImpl;
    QPointer<QAction> m_triggerHighlight;
    QPointer<QAction> m_triggerSymbolInfo;
    QPointer<QAction> m_triggerGotoSymbol;
    QPointer<QAction> m_triggerFormat;
    QPointer<QAction> m_triggerRename;
    QPointer<QAction> m_expandSelection;
    QPointer<QAction> m_shrinkSelection;
    QPointer<QAction> m_complDocOn;
    QPointer<QAction> m_signatureHelp;
    QPointer<QAction> m_refDeclaration;
    QPointer<QAction> m_complParens;
    QPointer<QAction> m_autoHover;
    QPointer<QAction> m_onTypeFormatting;
    QPointer<QAction> m_incrementalSync;
    QPointer<QAction> m_highlightGoto;
    QPointer<QAction> m_diagnostics;
    QPointer<QAction> m_diagnosticsHighlight;
    QPointer<QAction> m_diagnosticsMark;
    QPointer<QAction> m_diagnosticsHover;
    QPointer<QAction> m_diagnosticsSwitch;
    QPointer<QAction> m_messages;
    QPointer<QAction> m_closeDynamic;
    QPointer<QAction> m_restartServer;
    QPointer<QAction> m_restartAll;
    QPointer<QAction> m_switchSourceHeader;
    QPointer<QAction> m_expandMacro;
    QPointer<QAction> m_quickFix;
    QPointer<QAction> m_memoryUsage;
    QPointer<KActionMenu> m_requestCodeAction;

    QList<QAction *> m_contextMenuActions;

    // toolview
    QScopedPointer<QWidget> m_toolView;
    QPointer<QTabWidget> m_tabWidget;
    // applied search ranges
    typedef QMultiHash<KTextEditor::Document *, KTextEditor::MovingRange *> RangeCollection;
    RangeCollection m_ranges;
    // applied marks
    typedef QSet<KTextEditor::Document *> DocumentCollection;
    DocumentCollection m_marks;
    // modelis either owned by tree added to tabwidget or owned here
    QScopedPointer<QStandardItemModel> m_ownedModel;
    // in either case, the model that directs applying marks/ranges
    QPointer<QStandardItemModel> m_markModel;
    // goto definition and declaration jump list is more a menu than a
    // search result, so let's not keep adding new tabs for those
    // previous tree for definition result
    QPointer<QTreeView> m_defTree;
    // ... and for declaration
    QPointer<QTreeView> m_declTree;
    // ... and for type definition
    QPointer<QTreeView> m_typeDefTree;

    // diagnostics tab
    QPointer<QTreeView> m_diagnosticsTree;
    // tree widget is either owned here or by tab
    QScopedPointer<QTreeView> m_diagnosticsTreeOwn;
    QScopedPointer<QStandardItemModel> m_diagnosticsModel;
    // diagnostics ranges
    RangeCollection m_diagnosticsRanges;
    // and marks
    DocumentCollection m_diagnosticsMarks;
    // suppression tracked by session config
    SessionDiagnosticSuppressions m_sessionDiagnosticSuppressions;

    // views on which completions have been registered
    QSet<KTextEditor::View *> m_completionViews;

    // views on which hovers have been registered
    QSet<KTextEditor::View *> m_hoverViews;

    // outstanding request
    LSPClientServer::RequestHandle m_handle;
    // timeout on request
    bool m_req_timeout = false;

    // accept incoming applyEdit
    bool m_accept_edit = false;
    // characters to trigger format request
    QVector<QChar> m_onTypeFormattingTriggers;

    // ongoing workDoneProgress
    // list of (enhanced server token, progress begin)
    QVector<std::pair<QString, LSPWorkDoneProgressParams>> m_workDoneProgress;

    CtrlHoverFeedback m_ctrlHoverFeedback = {};

    // inner class that forwards directly to method for convenience
    class ForwardingTextHintProvider : public KTextEditor::TextHintProvider
    {
        typedef LSPClientPluginViewImpl parent_type;
        parent_type *m_parent;

    public:
        ForwardingTextHintProvider(parent_type *parent)
            : m_parent(parent)
        {
            Q_ASSERT(m_parent);
        }

        virtual QString textHint(KTextEditor::View *view, const KTextEditor::Cursor &position) override
        {
            return m_parent->onTextHint(view, position);
        }
    };

    SemanticHighlighter m_semHighlightingManager;

Q_SIGNALS:

    /**
     * Signal for location changed.
     * @param document url
     * @param c pos in document
     */
    void addPositionToHistory(const QUrl &url, KTextEditor::Cursor c);

public:
    LSPClientPluginViewImpl(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin, QSharedPointer<LSPClientServerManager> serverManager)
        : QObject(mainWin)
        , m_plugin(plugin)
        , m_mainWindow(mainWin)
        , m_serverManager(std::move(serverManager))
        , m_completion(LSPClientCompletion::new_(m_serverManager))
        , m_hover(LSPClientHover::new_(m_serverManager))
        , m_forwardHover(new ForwardingTextHintProvider(this))
        , m_symbolView(LSPClientSymbolView::new_(plugin, mainWin, m_serverManager))
        , m_semHighlightingManager(m_serverManager)
    {
        KXMLGUIClient::setComponentName(QStringLiteral("lspclient"), i18n("LSP Client"));
        setXMLFile(QStringLiteral("ui.rc"));

        connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &self_type::updateState);
        connect(m_mainWindow, &KTextEditor::MainWindow::unhandledShortcutOverride, this, &self_type::handleEsc);
        connect(m_serverManager.data(), &LSPClientServerManager::serverChanged, this, &self_type::onServerChanged);
        connect(m_plugin, &LSPClientPlugin::showMessage, this, &self_type::onShowMessage);
        connect(m_serverManager.data(), &LSPClientServerManager::serverShowMessage, this, &self_type::onMessage);
        connect(m_serverManager.data(), &LSPClientServerManager::serverLogMessage, this, [this](LSPClientServer *server, LSPLogMessageParams params) {
            switch (params.type) {
            case LSPMessageType::Error:
                params.message.prepend(QStringLiteral("[Error] "));
                break;
            case LSPMessageType::Warning:
                params.message.prepend(QStringLiteral("[Warn] "));
                break;
            case LSPMessageType::Info:
                params.message.prepend(QStringLiteral("[Info] "));
                break;
            case LSPMessageType::Log:
                break;
            }
            params.type = LSPMessageType::Log;
            onMessage(server, params);
        });
        connect(m_serverManager.data(), &LSPClientServerManager::serverWorkDoneProgress, this, &self_type::onWorkDoneProgress);

        m_findDef = actionCollection()->addAction(QStringLiteral("lspclient_find_definition"), this, &self_type::goToDefinition);
        m_findDef->setText(i18n("Go to Definition"));
        m_findDecl = actionCollection()->addAction(QStringLiteral("lspclient_find_declaration"), this, &self_type::goToDeclaration);
        m_findDecl->setText(i18n("Go to Declaration"));
        m_findTypeDef = actionCollection()->addAction(QStringLiteral("lspclient_find_type_definition"), this, &self_type::goToTypeDefinition);
        m_findTypeDef->setText(i18n("Go to Type Definition"));
        m_findRef = actionCollection()->addAction(QStringLiteral("lspclient_find_references"), this, &self_type::findReferences);
        m_findRef->setText(i18n("Find References"));
        actionCollection()->setDefaultShortcut(m_findRef, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_L));
        m_findImpl = actionCollection()->addAction(QStringLiteral("lspclient_find_implementations"), this, &self_type::findImplementation);
        m_findImpl->setText(i18n("Find Implementations"));
        m_triggerHighlight = actionCollection()->addAction(QStringLiteral("lspclient_highlight"), this, &self_type::highlight);
        m_triggerHighlight->setText(i18n("Highlight"));
        m_triggerSymbolInfo = actionCollection()->addAction(QStringLiteral("lspclient_symbol_info"), this, &self_type::symbolInfo);
        m_triggerSymbolInfo->setText(i18n("Symbol Info"));
        m_triggerGotoSymbol = actionCollection()->addAction(QStringLiteral("lspclient_goto_workspace_symbol"), this, &self_type::gotoWorkSpaceSymbol);
        m_triggerGotoSymbol->setText(i18n("Search and Go to Symbol"));
        actionCollection()->setDefaultShortcut(m_triggerGotoSymbol, QKeySequence(Qt::ALT | Qt::CTRL | Qt::Key_P));
        m_triggerFormat = actionCollection()->addAction(QStringLiteral("lspclient_format"), this, [this]() {
            format();
        });
        m_triggerFormat->setText(i18n("Format"));
        actionCollection()->setDefaultShortcut(m_triggerFormat, QKeySequence(QStringLiteral("Ctrl+T, F"), QKeySequence::PortableText));
        m_triggerRename = actionCollection()->addAction(QStringLiteral("lspclient_rename"), this, &self_type::rename);
        m_triggerRename->setText(i18n("Rename"));
        actionCollection()->setDefaultShortcut(m_triggerRename, QKeySequence(Qt::Key_F2));
#if KTEXTEDITOR_VERSION >= QT_VERSION_CHECK(5, 95, 0)
        m_expandSelection = actionCollection()->addAction(QStringLiteral("lspclient_expand_selection"), this, &self_type::expandSelection);
        m_expandSelection->setText(i18n("Expand Selection"));
        actionCollection()->setDefaultShortcut(m_expandSelection, QKeySequence(Qt::CTRL | Qt::Key_2));
        m_shrinkSelection = actionCollection()->addAction(QStringLiteral("lspclient_shrink_selection"), this, &self_type::shrinkSelection);
        m_shrinkSelection->setText(i18n("Shrink Selection"));
        actionCollection()->setDefaultShortcut(m_shrinkSelection, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_2));
#endif
        m_switchSourceHeader = actionCollection()->addAction(QStringLiteral("lspclient_clangd_switchheader"), this, &self_type::clangdSwitchSourceHeader);
        m_switchSourceHeader->setText(i18n("Switch Source Header"));
        actionCollection()->setDefaultShortcut(m_switchSourceHeader, Qt::Key_F12);
        m_expandMacro = actionCollection()->addAction(QStringLiteral("lspclient_rust_analyzer_expand_macro"), this, &self_type::rustAnalyzerExpandMacro);
        m_expandMacro->setText(i18n("Expand Macro"));
        m_quickFix = actionCollection()->addAction(QStringLiteral("lspclient_quick_fix"), this, &self_type::quickFix);
        m_quickFix->setText(i18n("Quick Fix"));
        actionCollection()->setDefaultShortcut(m_quickFix, QKeySequence((Qt::CTRL | Qt::Key_Period)));
        m_requestCodeAction = actionCollection()->add<KActionMenu>(QStringLiteral("lspclient_code_action"));
        m_requestCodeAction->setText(i18n("Code Action"));
        QList<QKeySequence> scuts;
        scuts << QKeySequence(Qt::ALT | Qt::Key_Return) << QKeySequence(Qt::ALT | Qt::Key_Enter);
        actionCollection()->setDefaultShortcuts(m_requestCodeAction, scuts);
        connect(m_requestCodeAction, &QWidgetAction::triggered, this, [this] {
            auto view = m_mainWindow->activeView();
            if (m_requestCodeAction && view) {
                const QPoint p = view->cursorPositionCoordinates();
                m_requestCodeAction->menu()->exec(view->mapToGlobal(p));
            }
        });
        connect(m_requestCodeAction->menu(), &QMenu::aboutToShow, this, &self_type::requestCodeAction);

        // general options
        m_complDocOn = actionCollection()->addAction(QStringLiteral("lspclient_completion_doc"), this, &self_type::displayOptionChanged);
        m_complDocOn->setText(i18n("Show selected completion documentation"));
        m_complDocOn->setCheckable(true);
        m_signatureHelp = actionCollection()->addAction(QStringLiteral("lspclient_signature_help"), this, &self_type::displayOptionChanged);
        m_signatureHelp->setText(i18n("Enable signature help with auto completion"));
        m_signatureHelp->setCheckable(true);
        m_refDeclaration = actionCollection()->addAction(QStringLiteral("lspclient_references_declaration"), this, &self_type::displayOptionChanged);
        m_refDeclaration->setText(i18n("Include declaration in references"));
        m_refDeclaration->setCheckable(true);
        m_complParens = actionCollection()->addAction(QStringLiteral("lspclient_completion_parens"), this, &self_type::displayOptionChanged);
        m_complParens->setText(i18n("Add parentheses upon function completion"));
        m_complParens->setCheckable(true);
        m_autoHover = actionCollection()->addAction(QStringLiteral("lspclient_auto_hover"), this, &self_type::displayOptionChanged);
        m_autoHover->setText(i18n("Show hover information"));
        m_autoHover->setCheckable(true);
        m_onTypeFormatting = actionCollection()->addAction(QStringLiteral("lspclient_type_formatting"), this, &self_type::displayOptionChanged);
        m_onTypeFormatting->setText(i18n("Format on typing"));
        m_onTypeFormatting->setCheckable(true);
        m_incrementalSync = actionCollection()->addAction(QStringLiteral("lspclient_incremental_sync"), this, &self_type::displayOptionChanged);
        m_incrementalSync->setText(i18n("Incremental document synchronization"));
        m_incrementalSync->setCheckable(true);
        m_highlightGoto = actionCollection()->addAction(QStringLiteral("lspclient_highlight_goto"), this, &self_type::displayOptionChanged);
        m_highlightGoto->setText(i18n("Highlight goto location"));
        m_highlightGoto->setCheckable(true);

        // diagnostics
        m_diagnostics = actionCollection()->addAction(QStringLiteral("lspclient_diagnostics"), this, &self_type::displayOptionChanged);
        m_diagnostics->setText(i18n("Show Diagnostics Notifications"));
        m_diagnostics->setCheckable(true);
        m_diagnosticsHighlight = actionCollection()->addAction(QStringLiteral("lspclient_diagnostics_highlight"), this, &self_type::displayOptionChanged);
        m_diagnosticsHighlight->setText(i18n("Show Diagnostics Highlights"));
        m_diagnosticsHighlight->setCheckable(true);
        m_diagnosticsMark = actionCollection()->addAction(QStringLiteral("lspclient_diagnostics_mark"), this, &self_type::displayOptionChanged);
        m_diagnosticsMark->setText(i18n("Show Diagnostics Marks"));
        m_diagnosticsMark->setCheckable(true);
        m_diagnosticsHover = actionCollection()->addAction(QStringLiteral("lspclient_diagnostics_hover"), this, &self_type::displayOptionChanged);
        m_diagnosticsHover->setText(i18n("Show Diagnostics on Hover"));
        m_diagnosticsHover->setCheckable(true);
        m_diagnosticsSwitch = actionCollection()->addAction(QStringLiteral("lspclient_diagnostic_switch"), this, &self_type::switchToDiagnostics);
        m_diagnosticsSwitch->setText(i18n("Switch to Diagnostics Tab"));

        // messages
        m_messages = actionCollection()->addAction(QStringLiteral("lspclient_messages"), this, &self_type::displayOptionChanged);
        m_messages->setText(i18n("Show Messages"));
        m_messages->setCheckable(true);

        // extra
        m_memoryUsage = actionCollection()->addAction(QStringLiteral("lspclient_clangd_memoryusage"), this, &self_type::clangdMemoryUsage);
        m_memoryUsage->setText(i18n("Server Memory Usage"));

        // server control and misc actions
        m_closeDynamic = actionCollection()->addAction(QStringLiteral("lspclient_close_dynamic"), this, &self_type::closeDynamic);
        m_closeDynamic->setText(i18n("Close All Dynamic Reference Tabs"));
        m_restartServer = actionCollection()->addAction(QStringLiteral("lspclient_restart_server"), this, &self_type::restartCurrent);
        m_restartServer->setText(i18n("Restart LSP Server"));
        m_restartAll = actionCollection()->addAction(QStringLiteral("lspclient_restart_all"), this, &self_type::restartAll);
        m_restartAll->setText(i18n("Restart All LSP Servers"));

        auto addSeparator = [this]() {
            auto *sep1 = new QAction(this);
            sep1->setSeparator(true);
            m_contextMenuActions << sep1;
        };

        m_contextMenuActions << m_quickFix << m_requestCodeAction;
        addSeparator();

        QAction *goToAction = new QAction(i18n("Go To"));
        QMenu *goTo = new QMenu();
        goToAction->setMenu(goTo);
        goTo->addActions({m_findDecl, m_findDef, m_findTypeDef, m_switchSourceHeader});

        m_contextMenuActions << goToAction;
        addSeparator();
        m_contextMenuActions << m_findRef;
        m_contextMenuActions << m_triggerRename;
        addSeparator();

        QAction *lspOtherAction = new QAction(i18n("LSP Client"));
        QMenu *lspOther = new QMenu();
        lspOtherAction->setMenu(lspOther);
        lspOther->addAction(m_findImpl);
        lspOther->addAction(m_triggerHighlight);
        lspOther->addAction(m_triggerGotoSymbol);
        lspOther->addAction(m_expandMacro);
        lspOther->addAction(m_triggerFormat);
        lspOther->addAction(m_triggerSymbolInfo);
        lspOther->addSeparator();
        lspOther->addAction(m_diagnosticsSwitch);
        lspOther->addAction(m_closeDynamic);
        lspOther->addSeparator();
        lspOther->addAction(m_restartServer);
        lspOther->addAction(m_restartAll);

        // more options
        auto moreOptions = new KActionMenu(i18n("More options"), this);
        lspOther->addSeparator();
        lspOther->addAction(moreOptions);
        moreOptions->addAction(m_complDocOn);
        moreOptions->addAction(m_signatureHelp);
        moreOptions->addAction(m_refDeclaration);
        moreOptions->addAction(m_complParens);
        moreOptions->addAction(m_autoHover);
        moreOptions->addAction(m_onTypeFormatting);
        moreOptions->addAction(m_incrementalSync);
        moreOptions->addAction(m_highlightGoto);
        moreOptions->addSeparator();
        moreOptions->addAction(m_diagnostics);
        moreOptions->addAction(m_diagnosticsHighlight);
        moreOptions->addAction(m_diagnosticsMark);
        moreOptions->addAction(m_diagnosticsHover);
        moreOptions->addSeparator();
        moreOptions->addAction(m_messages);
        moreOptions->addSeparator();
        moreOptions->addAction(m_memoryUsage);

        m_contextMenuActions << lspOtherAction;
        addSeparator();

        // sync with plugin settings if updated
        connect(m_plugin, &LSPClientPlugin::update, this, &self_type::configUpdated);

        // toolview
        m_toolView.reset(mainWin->createToolView(plugin,
                                                 QStringLiteral("kate_lspclient"),
                                                 KTextEditor::MainWindow::Bottom,
                                                 QIcon::fromTheme(QStringLiteral("format-text-code")),
                                                 i18n("LSP")));
        m_tabWidget = new QTabWidget(m_toolView.data());
        m_toolView->layout()->addWidget(m_tabWidget);
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        m_tabWidget->setTabsClosable(true);
        KAcceleratorManager::setNoAccel(m_tabWidget);
        connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &self_type::tabCloseRequested);
        connect(m_tabWidget, &QTabWidget::currentChanged, this, &self_type::tabChanged);

        // diagnostics tab
        m_diagnosticsTree = new QTreeView();
        m_diagnosticsTree->setAlternatingRowColors(true);
        m_diagnosticsTreeOwn.reset(m_diagnosticsTree);
        m_diagnosticsModel.reset(new QStandardItemModel());
        m_diagnosticsModel->setColumnCount(1);
        m_diagnosticsTree->setUniformRowHeights(true);
        m_diagnosticsTree->setModel(m_diagnosticsModel.data());
        configureTreeView(m_diagnosticsTree);
        connect(m_diagnosticsTree, &QTreeView::clicked, this, &self_type::goToItemLocation);
        connect(m_diagnosticsTree, &QTreeView::doubleClicked, this, &self_type::triggerCodeAction);

        // track position in view to sync diagnostics list
        m_viewTracker.reset(LSPClientViewTracker::new_(plugin, mainWin, 0, 500));
        connect(m_viewTracker.data(), &LSPClientViewTracker::newState, this, &self_type::onViewState);

        connect(m_mainWindow, &KTextEditor::MainWindow::viewCreated, this, &self_type::onViewCreated);

        connect(this, &self_type::ctrlClickDefRecieved, this, &self_type::onCtrlMouseMove);

        configUpdated();
        updateState();

        m_mainWindow->guiFactory()->addClient(this);
    }

    SessionDiagnosticSuppressions &sessionDiagnosticSuppressions()
    {
        return m_sessionDiagnosticSuppressions;
    }

    void onViewCreated(KTextEditor::View *view)
    {
        if (view && view->focusProxy()) {
            view->focusProxy()->installEventFilter(this);
        }
    }

    KTextEditor::View *viewFromWidget(QWidget *widget)
    {
        if (widget) {
            return qobject_cast<KTextEditor::View *>(widget->parentWidget());
        }
        return nullptr;
    }

    /**
     * @brief normal word range queried from doc->wordRangeAt() will not include
     * full header from a #include line. For example in line "#include <abc/some>"
     * we get either abc or some. But for Ctrl + click we need to highlight it as
     * one thing, so this function expands the range from wordRangeAt() to do that.
     */
    static void expandToFullHeaderRange(KTextEditor::Range &range, QStringView lineText)
    {
        auto expandRangeTo = [lineText, &range](QChar c, int startPos) {
            int end = lineText.indexOf(c, startPos);
            if (end > -1) {
                auto startC = range.start();
                startC.setColumn(startPos);
                auto endC = range.end();
                endC.setColumn(end);
                range.setStart(startC);
                range.setEnd(endC);
            }
        };

        int angleBracketPos = lineText.indexOf(QLatin1Char('<'), 7);
        if (angleBracketPos > -1) {
            expandRangeTo(QLatin1Char('>'), angleBracketPos + 1);
        } else {
            int startPos = lineText.indexOf(QLatin1Char('"'), 7);
            if (startPos > -1) {
                expandRangeTo(QLatin1Char('"'), startPos + 1);
            }
        }
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (!obj->isWidgetType()) {
            return QObject::eventFilter(obj, event);
        }

        // common stuff that we need for both events
        auto viewInternal = static_cast<QWidget *>(obj);
        KTextEditor::View *v = viewFromWidget(viewInternal);
        if (!v) {
            return false;
        }

        if (event->type() == QEvent::Leave) {
            if (m_ctrlHoverFeedback.isValid()) {
                m_ctrlHoverFeedback.clear(v);
            }
            return QObject::eventFilter(obj, event);
        }

        auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
        // we are only concerned with mouse events for now :)
        if (!mouseEvent) {
            return false;
        }

        const auto coords = viewInternal->mapTo(v, mouseEvent->pos());
        const auto cur = v->coordinatesToCursor(coords);
        // there isn't much we can do now, just bail out
        if (!cur.isValid()) {
            return false;
        }

        auto doc = v->document();
        if (!doc) {
            return false;
        }

        // The user pressed Ctrl + Click
        if (event->type() == QEvent::MouseButtonPress) {
            if (mouseEvent->button() == Qt::LeftButton && mouseEvent->modifiers() == Qt::ControlModifier) {
                // must set cursor else we will be jumping somewhere else!!
                v->setCursorPosition(cur);
                m_ctrlHoverFeedback.clear(v);
                goToDefinition();
            }
        }
        // The user is hovering with Ctrl pressed
        else if (event->type() == QEvent::MouseMove) {
            if (mouseEvent->modifiers() == Qt::ControlModifier) {
                auto range = doc->wordRangeAt(cur);
                if (!range.isEmpty()) {
                    // check if we are in #include
                    // and expand the word range
                    auto lineText = doc->line(range.start().line());
                    if (lineText.startsWith(QLatin1String("#include")) && range.start().column() > 7) {
                        expandToFullHeaderRange(range, lineText);
                    }

                    m_ctrlHoverFeedback.setRangeAndWidget(range, viewInternal);
                    // this will not go anywhere actually, but just signal whether we have a definition
                    // Also, please rethink very hard if you are going to reuse this method. It's made
                    // only for Ctrl+Hover
                    processCtrlMouseHover(cur);
                } else {
                    // if there is no word, unset the cursor and remove the highlight
                    m_ctrlHoverFeedback.clear(v);
                }
            } else {
                // simple mouse move, make sure to unset the cursor
                // and remove the highlight
                m_ctrlHoverFeedback.clear(v);
            }
        }

        return false;
    }

    ~LSPClientPluginViewImpl() override
    {
        m_mainWindow->guiFactory()->removeClient(this);

        // unregister all code-completion providers, else we might crash
        for (auto view : qAsConst(m_completionViews)) {
            qobject_cast<KTextEditor::CodeCompletionInterface *>(view)->unregisterCompletionModel(m_completion.data());
        }

        // unregister all text-hint providers, else we might crash
        for (auto view : qAsConst(m_hoverViews)) {
            qobject_cast<KTextEditor::TextHintInterface *>(view)->unregisterTextHintProvider(m_forwardHover.data());
        }

        clearAllLocationMarks();
        clearAllDiagnosticsMarks();
    }

    void configureTreeView(QTreeView *treeView)
    {
        treeView->setHeaderHidden(true);
        treeView->setFocusPolicy(Qt::NoFocus);
        treeView->setLayoutDirection(Qt::LeftToRight);
        treeView->setSortingEnabled(false);
        treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        // styling
        treeView->setItemDelegate(new LocationTreeDelegate(treeView, Utils::editorFont()));

        // context menu
        treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        auto menu = new QMenu(treeView);
        menu->addAction(i18n("Expand All"), treeView, &QTreeView::expandAll);
        menu->addAction(i18n("Collapse All"), treeView, &QTreeView::collapseAll);
        auto h = [treeView, menu](const QPoint &p) {
            menu->popup(treeView->viewport()->mapToGlobal(p));
        };
        if (m_diagnosticsTree == treeView) {
            connect(treeView, &QTreeView::customContextMenuRequested, this, &self_type::onDiagnosticsMenu);
        } else {
            connect(treeView, &QTreeView::customContextMenuRequested, h);
        }
    }

    void displayOptionChanged()
    {
        m_diagnosticsHighlight->setEnabled(m_diagnostics->isChecked());
        m_diagnosticsMark->setEnabled(m_diagnostics->isChecked());
        m_diagnosticsHover->setEnabled(m_diagnostics->isChecked());

        // diagnstics tab next
        int diagnosticsIndex = m_tabWidget->indexOf(m_diagnosticsTree);
        // setTabEnabled may still show it ... so let's be more forceful
        if (m_diagnostics->isChecked() && m_diagnosticsTreeOwn) {
            m_diagnosticsTreeOwn.take();
            m_tabWidget->insertTab(0, m_diagnosticsTree, i18nc("@title:tab", "Diagnostics"));
            // Hide close button
            m_tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
            m_tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
        } else if (!m_diagnostics->isChecked() && !m_diagnosticsTreeOwn) {
            m_diagnosticsTreeOwn.reset(m_diagnosticsTree);
            m_tabWidget->removeTab(diagnosticsIndex);
        }
        m_diagnosticsSwitch->setEnabled(m_diagnostics->isChecked());
        m_serverManager->setIncrementalSync(m_incrementalSync->isChecked());
        updateState();
    }

    void configUpdated()
    {
        if (m_complDocOn) {
            m_complDocOn->setChecked(m_plugin->m_complDoc);
        }
        if (m_signatureHelp) {
            m_signatureHelp->setChecked(m_plugin->m_signatureHelp);
        }
        if (m_refDeclaration) {
            m_refDeclaration->setChecked(m_plugin->m_refDeclaration);
        }
        if (m_complParens) {
            m_complParens->setChecked(m_plugin->m_complParens);
        }
        if (m_autoHover) {
            m_autoHover->setChecked(m_plugin->m_autoHover);
        }
        if (m_onTypeFormatting) {
            m_onTypeFormatting->setChecked(m_plugin->m_onTypeFormatting);
        }
        if (m_incrementalSync) {
            m_incrementalSync->setChecked(m_plugin->m_incrementalSync);
        }
        if (m_highlightGoto) {
            m_highlightGoto->setChecked(m_plugin->m_highlightGoto);
        }
        if (m_diagnostics) {
            m_diagnostics->setChecked(m_plugin->m_diagnostics);
        }
        if (m_diagnosticsHighlight) {
            m_diagnosticsHighlight->setChecked(m_plugin->m_diagnosticsHighlight);
        }
        if (m_diagnosticsMark) {
            m_diagnosticsMark->setChecked(m_plugin->m_diagnosticsMark);
        }
        if (m_diagnosticsHover) {
            m_diagnosticsHover->setChecked(m_plugin->m_diagnosticsHover);
        }
        if (m_messages) {
            m_messages->setChecked(m_plugin->m_messages);
        }
        if (m_completion) {
            m_completion->setAutoImport(m_plugin->m_autoImport);
        }
        displayOptionChanged();
    }

    void restartCurrent()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        auto server = m_serverManager->findServer(activeView);
        if (server) {
            m_serverManager->restart(server.data());
        }
    }

    void restartAll()
    {
        m_serverManager->restart(nullptr);
    }

    static void clearMarks(KTextEditor::Document *doc, RangeCollection &ranges, DocumentCollection &docs, uint markType)
    {
        KTextEditor::MarkInterface *iface = docs.contains(doc) ? qobject_cast<KTextEditor::MarkInterface *>(doc) : nullptr;
        if (iface) {
            const QHash<int, KTextEditor::Mark *> marks = iface->marks();
            QHashIterator<int, KTextEditor::Mark *> i(marks);
            while (i.hasNext()) {
                i.next();
                if (i.value()->type & markType) {
                    iface->removeMark(i.value()->line, markType);
                }
            }
            docs.remove(doc);
        }

        for (auto it = ranges.find(doc); it != ranges.end() && it.key() == doc;) {
            delete it.value();
            it = ranges.erase(it);
        }
    }

    static void clearMarks(RangeCollection &ranges, DocumentCollection &docs, uint markType)
    {
        while (!ranges.empty()) {
            clearMarks(ranges.begin().key(), ranges, docs, markType);
        }
    }

    Q_SLOT void clearAllMarks(KTextEditor::Document *doc)
    {
        clearMarks(doc, m_ranges, m_marks, RangeData::markType);
        clearMarks(doc, m_diagnosticsRanges, m_diagnosticsMarks, RangeData::markTypeDiagAll);
    }

    void clearAllLocationMarks()
    {
        clearMarks(m_ranges, m_marks, RangeData::markType);
        // no longer add any again
        m_ownedModel.reset();
        m_markModel.clear();
    }

    void clearAllDiagnosticsMarks()
    {
        clearMarks(m_diagnosticsRanges, m_diagnosticsMarks, RangeData::markTypeDiagAll);
    }

    void addMarks(KTextEditor::Document *doc, QStandardItem *item, RangeCollection *ranges, DocumentCollection *docs)
    {
        Q_ASSERT(item);
        using Style = KSyntaxHighlighting::Theme::TextStyle;

        // only consider enabled items
        if (!(item->flags() & Qt::ItemIsEnabled)) {
            return;
        }

        auto url = item->data(RangeData::FileUrlRole).toUrl();
        // document url could end up empty while in intermediate reload state
        // (and then it might match a parent item with no RangeData at all)
        if (url != doc->url() || url.isEmpty()) {
            return;
        }

        KTextEditor::Range range = item->data(RangeData::RangeRole).value<LSPRange>();
        if (!range.isValid() || range.isEmpty()) {
            return;
        }
        auto line = range.start().line();
        RangeData::KindEnum kind = RangeData::KindEnum(item->data(RangeData::KindRole).toInt());

        KTextEditor::Attribute::Ptr attr;

        bool enabled = m_diagnostics && m_diagnostics->isChecked() && m_diagnosticsHighlight && m_diagnosticsHighlight->isChecked();
        KTextEditor::MarkInterface::MarkTypes markType = RangeData::markType;
        switch (kind) {
        case RangeData::KindEnum::Text: {
            // well, it's a bit like searching for something, so re-use that color
            static KTextEditor::Attribute::Ptr searchAttr;
            if (!searchAttr) {
                searchAttr = new KTextEditor::Attribute();
                const auto theme = KTextEditor::Editor::instance()->theme();
                QColor rangeColor = theme.editorColor(KSyntaxHighlighting::Theme::SearchHighlight);
                searchAttr->setBackground(rangeColor);
                searchAttr->setForeground(QBrush(theme.textColor(KSyntaxHighlighting::Theme::Normal)));
            }
            attr = searchAttr;
            enabled = true;
            break;
        }
        // FIXME are there any symbolic/configurable ways to pick these colors?
        case RangeData::KindEnum::Read: {
            static KTextEditor::Attribute::Ptr greenAttr;
            if (!greenAttr) {
                const auto theme = KTextEditor::Editor::instance()->theme();
                greenAttr = new KTextEditor::Attribute();
                greenAttr->setBackground(Qt::green);
                greenAttr->setForeground(QBrush(theme.textColor(KSyntaxHighlighting::Theme::Normal)));
            }
            attr = greenAttr;
            enabled = true;
            break;
        }
        case RangeData::KindEnum::Write: {
            static KTextEditor::Attribute::Ptr redAttr;
            if (!redAttr) {
                const auto theme = KTextEditor::Editor::instance()->theme();
                redAttr = new KTextEditor::Attribute();
                redAttr->setBackground(Qt::red);
                redAttr->setForeground(QBrush(theme.textColor(KSyntaxHighlighting::Theme::Normal)));
            }
            attr = redAttr;
            enabled = true;
            break;
        }
        // use underlining for diagnostics to avoid lots of fancy flickering
        case RangeData::KindEnum::Error: {
            static KTextEditor::Attribute::Ptr errorAttr;
            if (!errorAttr) {
                const auto theme = KTextEditor::Editor::instance()->theme();
                errorAttr = new KTextEditor::Attribute();
                errorAttr->setUnderlineColor(theme.textColor(Style::Error));
                errorAttr->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            }
            attr = errorAttr;
            markType = RangeData::markTypeDiagError;
            break;
        }
        case RangeData::KindEnum::Warning: {
            static KTextEditor::Attribute::Ptr warnAttr;
            if (!warnAttr) {
                const auto theme = KTextEditor::Editor::instance()->theme();
                warnAttr = new KTextEditor::Attribute();
                warnAttr->setUnderlineColor(theme.textColor(Style::Warning));
                warnAttr->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            }
            attr = warnAttr;
            markType = RangeData::markTypeDiagWarning;
            break;
        }
        case RangeData::KindEnum::Information:
        case RangeData::KindEnum::Hint:
        case RangeData::KindEnum::Related: {
            static KTextEditor::Attribute::Ptr infoAttr;
            if (!infoAttr) {
                const auto theme = KTextEditor::Editor::instance()->theme();
                infoAttr = new KTextEditor::Attribute();
                infoAttr->setUnderlineColor(theme.textColor(Style::Information));
                infoAttr->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            }
            attr = infoAttr;
            markType = RangeData::markTypeDiagOther;
            break;
        }
        }

        if (!attr) {
            qWarning() << "Unexpected null attr";
        }

        // highlight the range
        if (enabled && ranges && attr) {
            KTextEditor::MovingInterface *miface = qobject_cast<KTextEditor::MovingInterface *>(doc);
            Q_ASSERT(miface);
            KTextEditor::MovingRange *mr = miface->newMovingRange(range);
            mr->setZDepth(-90000.0); // Set the z-depth to slightly worse than the selection
            mr->setAttribute(attr);
            mr->setAttributeOnlyForViews(true);
            ranges->insert(doc, mr);
        }

        KTextEditor::MarkInterfaceV2 *iface = qobject_cast<KTextEditor::MarkInterfaceV2 *>(doc);
        Q_ASSERT(iface);
        // add match mark for range
        bool handleClick = true;
        enabled = m_diagnostics && m_diagnostics->isChecked() && m_diagnosticsMark && m_diagnosticsMark->isChecked();
        switch (markType) {
        case RangeData::markType:
            iface->setMarkDescription(markType, i18n("RangeHighLight"));
            iface->setMarkIcon(markType, QIcon());
            handleClick = false;
            enabled = true;
            break;
        case RangeData::markTypeDiagError:
            iface->setMarkDescription(markType, i18n("Error"));
            iface->setMarkIcon(markType, diagnosticsIcon(LSPDiagnosticSeverity::Error));
            break;
        case RangeData::markTypeDiagWarning:
            iface->setMarkDescription(markType, i18n("Warning"));
            iface->setMarkIcon(markType, diagnosticsIcon(LSPDiagnosticSeverity::Warning));
            break;
        case RangeData::markTypeDiagOther:
            iface->setMarkDescription(markType, i18n("Information"));
            iface->setMarkIcon(markType, diagnosticsIcon(LSPDiagnosticSeverity::Information));
            break;
        default:
            Q_ASSERT(false);
            break;
        }
        if (enabled && docs) {
            iface->addMark(line, markType);
            docs->insert(doc);
        }

        // ensure runtime match
        // clang-format off
        connect(doc,
                SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)),
                this,
                SLOT(clearAllMarks(KTextEditor::Document*)),
                Qt::UniqueConnection);
        connect(doc,
                SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)),
                this,
                SLOT(clearAllMarks(KTextEditor::Document*)),
                Qt::UniqueConnection);
        // reload might save/restore marks before/after above signals, so let's clear before that
        connect(doc, &KTextEditor::Document::aboutToReload, this, &self_type::clearAllMarks, Qt::UniqueConnection);

        if (handleClick) {
            connect(doc,
                    SIGNAL(markClicked(KTextEditor::Document*,KTextEditor::Mark,bool&)),
                    this,
                    SLOT(onMarkClicked(KTextEditor::Document*,KTextEditor::Mark,bool&)),
                    Qt::UniqueConnection);
        }
        // clang-format on
    }

    void addMarksRec(KTextEditor::Document *doc, QStandardItem *item, RangeCollection *ranges, DocumentCollection *docs)
    {
        // We only care about @p doc items
        auto docItem = dynamic_cast<DocumentDiagnosticItem *>(item);
        if (docItem && QUrl::fromLocalFile(docItem->data(Qt::UserRole).toString()) != doc->url()) {
            return;
        }

        Q_ASSERT(item);
        addMarks(doc, item, ranges, docs);
        for (int i = 0; i < item->rowCount(); ++i) {
            addMarksRec(doc, item->child(i), ranges, docs);
        }
    }

    void addMarks(KTextEditor::Document *doc, QStandardItemModel *treeModel, RangeCollection &ranges, DocumentCollection &docs)
    {
        // check if already added
        auto oranges = ranges.contains(doc) ? nullptr : &ranges;
        auto odocs = docs.contains(doc) ? nullptr : &docs;

        if (!oranges && !odocs) {
            return;
        }

        Q_ASSERT(treeModel);
        addMarksRec(doc, treeModel->invisibleRootItem(), oranges, odocs);
    }

    void goToDocumentLocation(const QUrl &uri, const KTextEditor::Range &location)
    {
        int line = location.start().line();
        int column = location.start().column();
        KTextEditor::View *activeView = m_mainWindow->activeView();
        if (!activeView || uri.isEmpty() || line < 0 || column < 0) {
            return;
        }

        KTextEditor::Document *document = activeView->document();
        KTextEditor::Cursor cdef(line, column);

        KTextEditor::View *targetView = nullptr;
        if (document && uri == document->url()) {
            targetView = activeView;
        } else {
            targetView = m_mainWindow->openUrl(uri);
        }
        if (targetView) {
            // save current position for location history
            Q_EMIT addPositionToHistory(activeView->document()->url(), activeView->cursorPosition());
            // save the position to which we are jumping in location history
            Q_EMIT addPositionToHistory(targetView->document()->url(), cdef);
            targetView->setCursorPosition(cdef);
            highlightLandingLocation(targetView, location);
        }
    }

    /**
     * @brief give a short 1sec temporary highlight where you land
     */
    void highlightLandingLocation(KTextEditor::View *view, const KTextEditor::Range &location)
    {
        if (!m_highlightGoto || !m_highlightGoto->isChecked()) {
            return;
        }
        auto doc = view->document();
        if (!doc) {
            return;
        }
        auto miface = qobject_cast<KTextEditor::MovingInterface *>(doc);
        if (!miface) {
            return;
        }
        auto mr = miface->newMovingRange(location);
        KTextEditor::Attribute::Ptr attr(new KTextEditor::Attribute);
        attr->setUnderlineStyle(QTextCharFormat::SingleUnderline);
        mr->setView(view);
        mr->setAttribute(attr);
        QTimer::singleShot(1000, doc, [mr] {
            mr->setRange(KTextEditor::Range::invalid());
            delete mr;
        });
    }

    static QModelIndex getPrimaryModelIndex(QModelIndex index)
    {
        // in case of a multiline diagnostics item, a split secondary line has no data set
        // so we need to go up to the primary parent item
        if (!index.data(RangeData::RangeRole).isValid() && index.parent().data(RangeData::RangeRole).isValid()) {
            return index.parent();
        }
        return index;
    }

    void goToItemLocation(const QModelIndex &_index)
    {
        auto index = getPrimaryModelIndex(_index);
        auto url = index.data(RangeData::FileUrlRole).toUrl();
        auto start = index.data(RangeData::RangeRole).value<LSPRange>();
        goToDocumentLocation(url, start);
    }

    // custom item subclass that captures additional attributes;
    // a bit more convenient than the variant/role way
    struct DiagnosticItem : public QStandardItem {
        LSPDiagnostic m_diagnostic;
        LSPCodeAction m_codeAction;
        QSharedPointer<LSPClientRevisionSnapshot> m_snapshot;

        DiagnosticItem(const LSPDiagnostic &d)
            : m_diagnostic(d)
        {
        }

        DiagnosticItem(const LSPCodeAction &c, QSharedPointer<LSPClientRevisionSnapshot> s)
            : m_codeAction(c)
            , m_snapshot(std::move(s))
        {
            m_diagnostic.range = LSPRange::invalid();
        }

        bool isCodeAction() const
        {
            return !m_diagnostic.range.isValid() && m_codeAction.title.size();
        }
    };

    // helper data that holds diagnostics suppressions
    class DiagnosticSuppression
    {
        struct Suppression {
            QRegularExpression diag, code;
        };
        QVector<Suppression> m_suppressions;
        QPointer<KTextEditor::Document> m_document;

    public:
        // construct from configuration
        DiagnosticSuppression(self_type *self, KTextEditor::Document *doc, const QJsonObject &serverConfig)
            : m_document(doc)
        {
            // check regexp and report
            auto checkRegExp = [self](const QRegularExpression &regExp) {
                auto valid = regExp.isValid();
                if (!valid) {
                    auto error = regExp.errorString();
                    auto offset = regExp.patternErrorOffset();
                    auto msg = i18nc("@info", "Error in regular expression: %1\noffset %2: %3", regExp.pattern(), offset, error);
                    self->onShowMessage(KTextEditor::Message::Error, msg);
                }
                return valid;
            };

            Q_ASSERT(doc);
            const auto localPath = doc->url().toLocalFile();
            const auto supps = serverConfig.value(QStringLiteral("suppressions")).toObject();
            for (const auto &entry : supps) {
                // should be (array) tuple (last element optional)
                // [url regexp, message regexp, code regexp]
                const auto patterns = entry.toArray();
                if (patterns.size() >= 2) {
                    const auto urlRegExp = QRegularExpression(patterns.at(0).toString());
                    if (urlRegExp.isValid() && urlRegExp.match(localPath).hasMatch()) {
                        QRegularExpression diagRegExp, codeRegExp;
                        diagRegExp = QRegularExpression(patterns.at(1).toString());
                        if (patterns.size() >= 3) {
                            codeRegExp = QRegularExpression(patterns.at(2).toString());
                        }
                        if (checkRegExp(diagRegExp) && checkRegExp(codeRegExp)) {
                            m_suppressions.push_back({diagRegExp, codeRegExp});
                        }
                    }
                }
            }
            // also consider session suppressions
            const auto suppressions = self->m_sessionDiagnosticSuppressions.getSuppressions(localPath);
            for (const auto &entry : suppressions) {
                auto pattern = QRegularExpression::escape(entry);
                m_suppressions.push_back({QRegularExpression(pattern), {}});
            }
        }

        bool match(const QStandardItem &item) const
        {
            for (const auto &s : m_suppressions) {
                if (s.diag.match(item.text()).hasMatch()) {
                    // retrieve and check code text if we need to match the content as well
                    if (m_document && !s.code.pattern().isEmpty()) {
                        auto range = item.data(RangeData::RangeRole).value<LSPRange>();
                        auto code = m_document->text(range);
                        if (!s.code.match(code).hasMatch()) {
                            continue;
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        KTextEditor::Document *document()
        {
            return m_document;
        }
    };

    // likewise; a custom item for document level model item
    struct DocumentDiagnosticItem : public QStandardItem {
        QScopedPointer<DiagnosticSuppression> m_diagnosticSuppression;
        bool m_enabled = true;
    };

    // double click on:
    // diagnostic item -> request and add actions (below item)
    // code action -> perform action (literal edit and/or execute command)
    // (execution of command may lead to an applyEdit request from server)
    void triggerCodeAction(const QModelIndex &index)
    {
        triggerCodeActionItem(index, false);
    }

    void triggerCodeActionItem(const QModelIndex &index, bool autoApply)
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        QPointer<KTextEditor::Document> document = activeView->document();
        auto server = m_serverManager->findServer(activeView);
        auto it = dynamic_cast<DiagnosticItem *>(m_diagnosticsModel->itemFromIndex(index));
        if (!server || !document || !it) {
            return;
        }

        auto executeCodeAction = [this, server](DiagnosticItem *it) {
            Q_ASSERT(it->isCodeAction());
            auto &action = it->m_codeAction;
            // apply edit before command
            applyWorkspaceEdit(action.edit, it->m_snapshot.data());
            executeServerCommand(server, action.command);
            // diagnostics are likely updated soon, but might be clicked again in meantime
            // so clear once executed, so not executed again
            action.edit.changes.clear();
            action.command.command.clear();
            return;
        };
        // click on an action ?
        if (it->isCodeAction()) {
            executeCodeAction(it);
        }

        // only engage action if
        // * active document matches diagnostic document
        // * if really clicked a diagnostic item
        //   (which is the case as it != nullptr and not a code action)
        // * if no code action invoked and added already
        //   (note; related items are also children)
        auto url = it->data(RangeData::FileUrlRole).toUrl();
        if (url != document->url() || it->data(Qt::UserRole).toBool()) {
            return;
        }

        // store some things to find item safely later on
        QPersistentModelIndex pindex(index);
        QSharedPointer<LSPClientRevisionSnapshot> snapshot(m_serverManager->snapshot(server.data()));
        auto h = [this, url, snapshot, pindex, autoApply, executeCodeAction](const QList<LSPCodeAction> &actions) {
            if (!pindex.isValid()) {
                return;
            }
            auto child = m_diagnosticsModel->itemFromIndex(pindex);
            if (!child) {
                return;
            }
            // add actions below diagnostic item
            for (const auto &action : actions) {
                auto item = new DiagnosticItem(action, snapshot);
                child->appendRow(item);
                auto text = action.kind.size() ? QStringLiteral("[%1] %2").arg(action.kind).arg(action.title) : action.title;
                item->setData(text, Qt::DisplayRole);
                item->setData(codeActionIcon(), Qt::DecorationRole);
                if (autoApply) {
                    executeCodeAction(item);
                }
            }
            m_diagnosticsTree->setExpanded(child->index(), true);
            // mark actions added
            child->setData(true, Qt::UserRole);
        };

        auto range = activeView->selectionRange();
        if (!range.isValid()) {
            range = document->documentRange();
        }
        server->documentCodeAction(url, range, {}, {it->m_diagnostic}, this, h);
    }

    void quickFix()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        KTextEditor::Document *document = activeView->document();

        if (!document) {
            return;
        }

        QStandardItem *topItem = getItem(*m_diagnosticsModel, document->url());
        // try to find current diagnostic based on cursor position
        auto pos = activeView->cursorPosition();
        QStandardItem *targetItem = getItem(topItem, pos, false);
        if (!targetItem) {
            // match based on line position only
            targetItem = getItem(topItem, pos, true);
        }

        if (targetItem) {
            triggerCodeActionItem(targetItem->index(), true);
        }
    }

    bool tabCloseRequested(int index)
    {
        auto widget = m_tabWidget->widget(index);
        if (widget != m_diagnosticsTree) {
            if (m_markModel && widget == m_markModel->parent()) {
                clearAllLocationMarks();
            }
            delete widget;
            return true;
        }
        return false;
    }

    void tabChanged(int index)
    {
        // reset to regular foreground
        m_tabWidget->tabBar()->setTabTextColor(index, QColor());
    }

    void switchToDiagnostics()
    {
        m_tabWidget->setCurrentWidget(m_diagnosticsTree);
        m_mainWindow->showToolView(m_toolView.data());
    }

    void closeDynamic()
    {
        for (int i = 0; i < m_tabWidget->count();) {
            // if so deemed suitable, tab will be spared and not closed
            if (!tabCloseRequested(i)) {
                ++i;
            }
        }
    }

    // local helper to overcome some differences in LSP types
    struct RangeItem {
        QUrl uri;
        LSPRange range;
        LSPDocumentHighlightKind kind;

        bool isValid() const
        {
            return uri.isValid() && range.isValid();
        }
    };

    static bool compareRangeItem(const RangeItem &a, const RangeItem &b)
    {
        return (a.uri < b.uri) || ((a.uri == b.uri) && a.range < b.range);
    }

    // provide Qt::DisplayRole (text) line lazily;
    // only find line's text content when so requested
    // This may then involve opening reading some file, at which time
    // all items for that file will be resolved in one go.
    struct LineItem : public QStandardItem {
        KTextEditor::MainWindow *m_mainWindow;

        LineItem(KTextEditor::MainWindow *mainWindow)
            : m_mainWindow(mainWindow)
        {
        }

        QVariant data(int role = Qt::UserRole + 1) const override
        {
            auto rootItem = this->parent();
            if (role != Qt::DisplayRole || !rootItem) {
                return QStandardItem::data(role);
            }

            auto line = data(Qt::UserRole);
            // either of these mean we tried to obtain line already
            if (line.isValid() || rootItem->data(RangeData::KindRole).toBool()) {
                return QStandardItem::data(role).toString().append(line.toString());
            }

            KTextEditor::Document *doc = nullptr;
            QScopedPointer<FileLineReader> fr;
            for (int i = 0; i < rootItem->rowCount(); i++) {
                auto child = rootItem->child(i);
                if (i == 0) {
                    auto url = child->data(RangeData::FileUrlRole).toUrl();
                    doc = findDocument(m_mainWindow, url);
                    if (!doc) {
                        fr.reset(new FileLineReader(url));
                    }
                }
                auto lineno = child->data(RangeData::RangeRole).value<LSPRange>().start().line();
                auto line = doc ? doc->line(lineno) : fr->line(lineno);
                child->setData(line, Qt::UserRole);
            }

            // mark as processed
            rootItem->setData(RangeData::KindRole, true);

            // should work ok
            return data(role);
        }
    };

    static void
    fillItemRoles(QStandardItem *item, const QUrl &url, const LSPRange _range, RangeData::KindEnum kind, const LSPClientRevisionSnapshot *snapshot = nullptr)
    {
        auto range = snapshot ? transformRange(url, *snapshot, _range) : _range;
        item->setData(QVariant(url), RangeData::FileUrlRole);
        QVariant vrange;
        vrange.setValue(range);
        item->setData(vrange, RangeData::RangeRole);
        item->setData(static_cast<int>(kind), RangeData::KindRole);
    }

    QString getProjectBaseDir()
    {
        QObject *project = m_mainWindow->pluginView(QStringLiteral("kateprojectplugin"));
        if (project) {
            auto baseDir = project->property("projectBaseDir").toString();
            if (!baseDir.endsWith(QLatin1Char('/'))) {
                return baseDir + QLatin1Char('/');
            }

            return baseDir;
        }

        return {};
    }

    QString shortenPath(QString projectBaseDir, QString url)
    {
        if (!projectBaseDir.isEmpty() && url.startsWith(projectBaseDir)) {
            return url.mid(projectBaseDir.length());
        }

        return url;
    }

    void makeTree(const QVector<RangeItem> &locations, const LSPClientRevisionSnapshot *snapshot)
    {
        // group by url, assuming input is suitably sorted that way
        auto treeModel = new QStandardItemModel();
        treeModel->setColumnCount(1);

        QString baseDir = getProjectBaseDir();
        QUrl lastUrl;
        QStandardItem *parent = nullptr;
        for (const auto &loc : locations) {
            // ensure we create a parent, if not already there (bug 427270) or we have a different url
            if (!parent || loc.uri != lastUrl) {
                if (parent) {
                    parent->setText(QStringLiteral("%1: %2").arg(shortenPath(baseDir, lastUrl.toLocalFile())).arg(parent->rowCount()));
                }
                lastUrl = loc.uri;
                parent = new QStandardItem();
                treeModel->appendRow(parent);
            }
            auto item = new LineItem(m_mainWindow);
            parent->appendRow(item);
            // add partial display data; line will be added by item later on
            item->setText(i18n("Line: %1: ", loc.range.start().line() + 1));
            fillItemRoles(item, loc.uri, loc.range, loc.kind, snapshot);
        }
        if (parent) {
            parent->setText(QStringLiteral("%1: %2").arg(shortenPath(baseDir, lastUrl.toLocalFile())).arg(parent->rowCount()));
        }

        // plain heuristic; mark for auto-expand all when safe and/or useful to do so
        if (treeModel->rowCount() <= 2 || locations.size() <= 20) {
            treeModel->invisibleRootItem()->setData(true, RangeData::KindRole);
        }

        m_ownedModel.reset(treeModel);
        m_markModel = treeModel;
    }

    void showTree(const QString &title, QPointer<QTreeView> *targetTree)
    {
        // clean up previous target if any
        if (targetTree && *targetTree) {
            int index = m_tabWidget->indexOf(*targetTree);
            if (index >= 0) {
                tabCloseRequested(index);
            }
        }

        // setup view
        auto treeView = new QTreeView();
        configureTreeView(treeView);

        // transfer model from owned to tree and that in turn to tabwidget
        auto treeModel = m_ownedModel.take();
        treeView->setModel(treeModel);
        treeModel->setParent(treeView);
        int index = m_tabWidget->addTab(treeView, title);
        connect(treeView, &QTreeView::clicked, this, &self_type::goToItemLocation);

        if (treeModel->invisibleRootItem()->data(RangeData::KindRole).toBool()) {
            treeView->expandAll();
        }

        // track for later cleanup
        if (targetTree) {
            *targetTree = treeView;
        }

        // activate the resulting tab
        m_tabWidget->setCurrentIndex(index);
        m_mainWindow->showToolView(m_toolView.data());
    }

    void showMessage(const QString &text, KTextEditor::Message::MessageType level)
    {
        KTextEditor::View *view = m_mainWindow->activeView();
        if (!view || !view->document()) {
            return;
        }

        auto kmsg = new KTextEditor::Message(text, level);
        kmsg->setPosition(KTextEditor::Message::BottomInView);
        kmsg->setAutoHide(500);
        kmsg->setView(view);
        view->document()->postMessage(kmsg);
    }

    void handleEsc(QEvent *e)
    {
        if (!m_mainWindow) {
            return;
        }

        QKeyEvent *k = static_cast<QKeyEvent *>(e);
        if (k->key() == Qt::Key_Escape && k->modifiers() == Qt::NoModifier) {
            if (!m_ranges.empty()) {
                clearAllLocationMarks();
            } else if (m_toolView->isVisible()) {
                m_mainWindow->hideToolView(m_toolView.data());
            }
        }
    }

    template<typename Handler>
    using LocationRequest = std::function<
        LSPClientServer::RequestHandle(LSPClientServer &, const QUrl &document, const LSPPosition &pos, const QObject *context, const Handler &h)>;

    template<typename Handler>
    void positionRequest(const LocationRequest<Handler> &req,
                         const Handler &h,
                         QScopedPointer<LSPClientRevisionSnapshot> *snapshot = nullptr,
                         KTextEditor::Cursor cur = KTextEditor::Cursor::invalid())
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        auto server = m_serverManager->findServer(activeView);
        if (!server) {
            return;
        }

        // track revision if requested
        if (snapshot) {
            snapshot->reset(m_serverManager->snapshot(server.data()));
        }

        KTextEditor::Cursor cursor = cur.isValid() ? cur : activeView->cursorPosition();

        clearAllLocationMarks();
        m_req_timeout = false;
        QTimer::singleShot(1000, this, [this] {
            m_req_timeout = true;
        });
        m_handle.cancel() = req(*server, activeView->document()->url(), {cursor.line(), cursor.column()}, this, h);
    }

    QString currentWord()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        if (activeView) {
            KTextEditor::Cursor cursor = activeView->cursorPosition();
            return activeView->document()->wordAt(cursor);
        } else {
            return QString();
        }
    }

    Q_SIGNAL void ctrlClickDefRecieved(const RangeItem &range);

    Q_SLOT void onCtrlMouseMove(const RangeItem &range)
    {
        if (range.isValid()) {
            if (m_ctrlHoverFeedback.isValid()) {
                m_ctrlHoverFeedback.highlight(m_mainWindow->activeView());
            }
        }
    }

    // some template and function type trickery here, but at least that buck stops here then ...
    template<typename ReplyEntryType, bool doshow = true, typename HandlerType = ReplyHandler<QList<ReplyEntryType>>>
    void processLocations(const QString &title,
                          const typename utils::identity<LocationRequest<HandlerType>>::type &req,
                          bool onlyshow,
                          const std::function<RangeItem(const ReplyEntryType &)> &itemConverter,
                          QPointer<QTreeView> *targetTree = nullptr)
    {
        // no capture for move only using initializers available (yet), so shared outer type
        // the additional level of indirection is so it can be 'filled-in' after lambda creation
        QSharedPointer<QScopedPointer<LSPClientRevisionSnapshot>> s(new QScopedPointer<LSPClientRevisionSnapshot>);
        auto h = [this, title, onlyshow, itemConverter, targetTree, s](const QList<ReplyEntryType> &defs) {
            if (defs.count() == 0) {
                showMessage(i18n("No results"), KTextEditor::Message::Information);
            } else {
                // convert to helper type
                QVector<RangeItem> ranges;
                ranges.reserve(defs.size());
                for (const auto &def : defs) {
                    ranges.push_back(itemConverter(def));
                }
                // ... so we can sort it also
                std::stable_sort(ranges.begin(), ranges.end(), compareRangeItem);
                makeTree(ranges, s.data()->data());

                // assuming that reply ranges refer to revision when submitted
                // (not specified anyway in protocol/reply)
                if (defs.count() > 1 || onlyshow) {
                    showTree(title, targetTree);
                }
                // it's not nice to jump to some location if we are too late
                if (!m_req_timeout && !onlyshow) {
                    // assuming here that the first location is the best one
                    const auto &item = itemConverter(defs.at(0));
                    goToDocumentLocation(item.uri, item.range);
                    // forego mark and such if only a single destination
                    if (defs.count() == 1) {
                        clearAllLocationMarks();
                    }
                }
                // update marks
                updateMarks();
            }
        };

        positionRequest<HandlerType>(req, h, s.data());
    }

    void processCtrlMouseHover(const KTextEditor::Cursor &cursor)
    {
        auto h = [this](const QList<LSPLocation> &defs) {
            if (defs.isEmpty()) {
                return;
            } else {
                const auto item = locationToRangeItem(defs.at(0));
                Q_EMIT this->ctrlClickDefRecieved(item);
                return;
            }
        };

        using Handler = std::function<void(const QList<LSPLocation> &)>;
        auto request = &LSPClientServer::documentDefinition;
        positionRequest<Handler>(request, h, nullptr, cursor);
    }

    static RangeItem locationToRangeItem(const LSPLocation &loc)
    {
        return {loc.uri, loc.range, LSPDocumentHighlightKind::Text};
    }

    void goToDefinition()
    {
        auto title = i18nc("@title:tab", "Definition: %1", currentWord());
        processLocations<LSPLocation>(title, &LSPClientServer::documentDefinition, false, &self_type::locationToRangeItem, &m_defTree);
    }

    void goToDeclaration()
    {
        auto title = i18nc("@title:tab", "Declaration: %1", currentWord());
        processLocations<LSPLocation>(title, &LSPClientServer::documentDeclaration, false, &self_type::locationToRangeItem, &m_declTree);
    }

    void goToTypeDefinition()
    {
        auto title = i18nc("@title:tab", "Type Definition: %1", currentWord());
        processLocations<LSPLocation>(title, &LSPClientServer::documentTypeDefinition, false, &self_type::locationToRangeItem, &m_typeDefTree);
    }

    void findReferences()
    {
        auto title = i18nc("@title:tab", "References: %1", currentWord());
        bool decl = m_refDeclaration->isChecked();
        // clang-format off
        auto req = [decl](LSPClientServer &server, const QUrl &document, const LSPPosition &pos, const QObject *context, const DocumentDefinitionReplyHandler &h)
        { return server.documentReferences(document, pos, decl, context, h); };
        // clang-format on

        processLocations<LSPLocation>(title, req, true, &self_type::locationToRangeItem);
    }

    void findImplementation()
    {
        auto title = i18nc("@title:tab", "Implementation: %1", currentWord());
        processLocations<LSPLocation>(title, &LSPClientServer::documentImplementation, true, &self_type::locationToRangeItem);
    }

    void highlight()
    {
        // determine current url to capture and use later on
        QUrl url;
        const KTextEditor::View *viewForRequest(m_mainWindow->activeView());
        if (viewForRequest && viewForRequest->document()) {
            url = viewForRequest->document()->url();
        }

        auto title = i18nc("@title:tab", "Highlight: %1", currentWord());
        auto converter = [url](const LSPDocumentHighlight &hl) {
            return RangeItem{url, hl.range, hl.kind};
        };

        processLocations<LSPDocumentHighlight, false>(title, &LSPClientServer::documentHighlight, true, converter);
    }

    void symbolInfo()
    {
        // trigger manually the normally automagic hover
        if (auto activeView = m_mainWindow->activeView()) {
            m_hover->showTextHint(activeView, activeView->cursorPosition(), true);
        }
    }

    void requestCodeAction()
    {
        if (!m_requestCodeAction)
            return;
        m_requestCodeAction->menu()->clear();

        KTextEditor::View *activeView = m_mainWindow->activeView();
        if (!activeView) {
            m_requestCodeAction->menu()->addAction(i18n("No Actions"))->setEnabled(false);
            return;
        }

        KTextEditor::Document *document = activeView->document();
        auto server = m_serverManager->findServer(activeView);
        auto range = activeView->selectionRange();
        if (!range.isValid()) {
            auto currentLine = activeView->cursorPosition().line();
            range = KTextEditor::Range(currentLine, 0, currentLine + 1, 0);
        }
        if (!server || !document || !range.isValid()) {
            m_requestCodeAction->menu()->addAction(i18n("No Actions"))->setEnabled(false);
            return;
        }

        QPointer<QAction> loadingAction = m_requestCodeAction->menu()->addAction(i18n("Loading..."));
        loadingAction->setEnabled(false);

        // store some things to find item safely later on
        QSharedPointer<LSPClientRevisionSnapshot> snapshot(m_serverManager->snapshot(server.data()));
        auto h = [this, snapshot, server, loadingAction](const QList<LSPCodeAction> &actions) {
            auto menu = m_requestCodeAction->menu();
            // clearing menu also hides it, and so added actions end up not shown
            if (actions.isEmpty()) {
                menu->addAction(i18n("No Actions"))->setEnabled(false);
            }
            for (const auto &action : actions) {
                auto text = action.kind.size() ? QStringLiteral("[%1] %2").arg(action.kind).arg(action.title) : action.title;
                menu->addAction(text, this, [this, action, snapshot, server]() {
                    applyWorkspaceEdit(action.edit, snapshot.data());
                    executeServerCommand(server, action.command);
                });
            }
            if (loadingAction) {
                menu->removeAction(loadingAction);
            }
        };
        server->documentCodeAction(document->url(), range, {}, {}, this, h);
    }

    void executeServerCommand(QSharedPointer<LSPClientServer> server, const LSPCommand &command)
    {
        if (!command.command.isEmpty()) {
            // accept edit requests that may be sent to execute command
            m_accept_edit = true;
            // but only for a short time
            QTimer::singleShot(2000, this, [this] {
                m_accept_edit = false;
            });
            server->executeCommand(command.command, command.arguments);
        }
    }

    static void applyEdits(KTextEditor::Document *doc, const LSPClientRevisionSnapshot *snapshot, const QList<LSPTextEdit> &edits)
    {
        ::applyEdits(doc, snapshot, edits);
    }

    void applyEdits(const QUrl &url, const LSPClientRevisionSnapshot *snapshot, const QList<LSPTextEdit> &edits)
    {
        auto document = findDocument(m_mainWindow, url);
        if (!document) {
            KTextEditor::View *view = m_mainWindow->openUrl(url);
            if (view) {
                document = view->document();
            }
        }
        applyEdits(document, snapshot, edits);
    }

    void applyWorkspaceEdit(const LSPWorkspaceEdit &edit, const LSPClientRevisionSnapshot *snapshot)
    {
        auto currentView = m_mainWindow->activeView();
        // edits may be in changes or documentChanges
        // the latter is handled in a sneaky way, but not announced in capabilities
        for (auto it = edit.changes.begin(); it != edit.changes.end(); ++it) {
            applyEdits(it.key(), snapshot, it.value());
        }
        // ... as/though the document version is not (yet) taken into account
        for (auto &change : edit.documentChanges) {
            applyEdits(change.textDocument.uri, snapshot, change.edits);
        }
        if (currentView) {
            m_mainWindow->activateView(currentView->document());
        }
    }

    void onApplyEdit(const LSPApplyWorkspaceEditParams &edit, const ApplyEditReplyHandler &h, bool &handled)
    {
        if (handled) {
            return;
        }
        handled = true;

        if (m_accept_edit) {
            qCInfo(LSPCLIENT) << "applying edit" << edit.label;
            applyWorkspaceEdit(edit.edit, nullptr);
        } else {
            qCInfo(LSPCLIENT) << "ignoring edit";
        }
        h({m_accept_edit, QString()});
    }

    template<typename Collection>
    void checkEditResult(const Collection &c)
    {
        if (c.empty()) {
            showMessage(i18n("No edits"), KTextEditor::Message::Information);
        }
    }

    void delayCancelRequest(LSPClientServer::RequestHandle &&h, int timeout_ms = 4000)
    {
        QTimer::singleShot(timeout_ms, this, [h]() mutable {
            h.cancel();
        });
    }

    void format(QChar lastChar = QChar())
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        QPointer<KTextEditor::Document> document = activeView ? activeView->document() : nullptr;
        auto server = m_serverManager->findServer(activeView);
        if (!server || !document) {
            return;
        }

        int tabSize = 4;
        bool insertSpaces = true;
        auto cfgiface = qobject_cast<KTextEditor::ConfigInterface *>(document);
        Q_ASSERT(cfgiface);
        tabSize = cfgiface->configValue(QStringLiteral("tab-width")).toInt();
        insertSpaces = cfgiface->configValue(QStringLiteral("replace-tabs")).toBool();

        // sigh, no move initialization capture ...
        // (again) assuming reply ranges wrt revisions submitted at this time
        QSharedPointer<LSPClientRevisionSnapshot> snapshot(m_serverManager->snapshot(server.data()));
        auto h = [this, document, snapshot, lastChar](const QList<LSPTextEdit> &edits) {
            if (lastChar.isNull()) {
                checkEditResult(edits);
            }
            if (document) {
                // Must clear formatting triggers here otherwise on applying edits we
                // might end up triggering formatting again ending up in an infinite loop
                auto savedTriggers = m_onTypeFormattingTriggers;
                m_onTypeFormattingTriggers.clear();
                applyEdits(document, snapshot.data(), edits);
                m_onTypeFormattingTriggers = savedTriggers;
            }
        };

        auto options = LSPFormattingOptions{tabSize, insertSpaces, QJsonObject()};
        auto handle = !lastChar.isNull()
            ? server->documentOnTypeFormatting(document->url(), activeView->cursorPosition(), lastChar, options, this, h)
            : (activeView->selection() ? server->documentRangeFormatting(document->url(), activeView->selectionRange(), options, this, h)
                                       : server->documentFormatting(document->url(), options, this, h));
        delayCancelRequest(std::move(handle));
    }

    void rename()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        QPointer<KTextEditor::Document> document = activeView ? activeView->document() : nullptr;
        auto server = m_serverManager->findServer(activeView);
        if (!server || !document) {
            return;
        }

        QString wordUnderCursor = document->wordAt(activeView->cursorPosition());
        if (wordUnderCursor.isEmpty()) {
            return;
        }

        bool ok = false;
        // results are typically (too) limited
        // due to server implementation or limited view/scope
        // so let's add a disclaimer that it's not our fault
        QString newName = QInputDialog::getText(activeView,
                                                i18nc("@title:window", "Rename"),
                                                i18nc("@label:textbox", "New name (caution: not all references may be replaced)"),
                                                QLineEdit::Normal,
                                                wordUnderCursor,
                                                &ok);
        if (!ok) {
            return;
        }

        QSharedPointer<LSPClientRevisionSnapshot> snapshot(m_serverManager->snapshot(server.data()));
        auto h = [this, snapshot](const LSPWorkspaceEdit &edit) {
            if (edit.documentChanges.empty()) {
                checkEditResult(edit.changes);
            }
            applyWorkspaceEdit(edit, snapshot.data());
        };
        auto handle = server->documentRename(document->url(), activeView->cursorPosition(), newName, this, h);
        delayCancelRequest(std::move(handle));
    }

#if KTEXTEDITOR_VERSION >= QT_VERSION_CHECK(5, 95, 0)
    void expandSelection()
    {
        changeSelection(true);
    }

    void shrinkSelection()
    {
        changeSelection(false);
    }

    void changeSelection(bool expand)
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        QPointer<KTextEditor::Document> document = activeView ? activeView->document() : nullptr;
        auto server = m_serverManager->findServer(activeView);
        if (!server || !document) {
            return;
        }

        auto h = [this, activeView, expand](const QList<std::shared_ptr<LSPSelectionRange>> &reply) {
            if (reply.isEmpty()) {
                showMessage(i18n("No results"), KTextEditor::Message::Information);
            }

            auto cursors = activeView->cursorPositions();

            if (cursors.size() != reply.size()) {
                showMessage(i18n("Not enough results"), KTextEditor::Message::Information);
            }

            auto selections = activeView->selectionRanges();
            QVector<KTextEditor::Range> ret;

            for (int i = 0; i < cursors.size(); i++) {
                const auto &lspSelectionRange = reply.at(i);

                if (lspSelectionRange) {
                    LSPRange currentRange = selections.isEmpty() || !selections.at(i).isValid() ? LSPRange(cursors.at(i), cursors.at(i)) : selections.at(i);

                    auto resultRange = findNextSelection(lspSelectionRange, currentRange, expand);
                    ret.append(resultRange);
                } else {
                    ret.append(KTextEditor::Range::invalid());
                }
            }

            activeView->setSelections(ret);
        };

        auto handle = server->selectionRange(document->url(), activeView->cursorPositions(), this, h);
        delayCancelRequest(std::move(handle));
    }

    static LSPRange findNextSelection(std::shared_ptr<LSPSelectionRange> selectionRange, const LSPRange &current, bool expand)
    {
        if (expand) {
            while (selectionRange && !selectionRange->range.contains(current)) {
                selectionRange = selectionRange->parent;
            }

            if (selectionRange) {
                if (selectionRange->range != current) {
                    return selectionRange->range;
                } else if (selectionRange->parent) {
                    return selectionRange->parent->range;
                }
            }
        } else {
            std::shared_ptr<LSPSelectionRange> previous = nullptr;

            while (selectionRange && current.contains(selectionRange->range) && current != selectionRange->range) {
                previous = selectionRange;
                selectionRange = selectionRange->parent;
            }

            if (previous) {
                return previous->range;
            }
        }

        return LSPRange::invalid();
    }
#endif

    void clangdSwitchSourceHeader()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        KTextEditor::Document *document = activeView->document();
        auto server = m_serverManager->findServer(activeView);
        if (!server || !document) {
            return;
        }

        auto h = [this](const QString &reply) {
            if (!reply.isEmpty()) {
                m_mainWindow->openUrl(QUrl(reply));
            } else {
                showMessage(i18n("Corresponding Header/Source not found"), KTextEditor::Message::Information);
            }
        };
        server->clangdSwitchSourceHeader(document->url(), this, h);
    }

    void clangdMemoryUsage()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        auto server = m_serverManager->findServer(activeView);
        if (!server)
            return;

        auto h = [this](const QJsonValue &reply) {
            auto view = m_mainWindow->openUrl(QUrl());
            if (view) {
                QJsonDocument json(reply.toObject());
                auto doc = view->document();
                doc->setText(QString::fromUtf8(json.toJson()));
                // position at top
                view->setCursorPosition({0, 0});
                // adjust mode
                const QString mode = QStringLiteral("JSON");
                doc->setHighlightingMode(mode);
                doc->setMode(mode);
                // no save file dialog when closing
                doc->setModified(false);
            }
        };
        server->clangdMemoryUsage(this, h);
    }

    void rustAnalyzerExpandMacro()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        auto server = m_serverManager->findServer(activeView);
        if (!server)
            return;

        auto position = activeView->cursorPosition();
        QPointer<KTextEditor::View> v(activeView);
        auto h = [this, v, position](const LSPExpandedMacro &reply) {
            if (v && !reply.expansion.isEmpty()) {
                LspTooltip::show(reply.expansion, LSPMarkupKind::PlainText, v->mapToGlobal(v->cursorToCoordinate(position)), v, true);
            } else {
                showMessage(i18n("No results"), KTextEditor::Message::Information);
            }
        };
        server->rustAnalyzerExpandMacro(this, activeView->document()->url(), position, h);
    }

    void gotoWorkSpaceSymbol()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        auto server = m_serverManager->findServer(activeView);
        if (!server) {
            return;
        }
        GotoSymbolHUDDialog dialog(m_mainWindow, server);
        dialog.openDialog();
    }

    static QStandardItem *getItem(const QStandardItemModel &model, const QUrl &url)
    {
        // local file in custom role, Qt::DisplayRole might have additional elements
        auto l = model.match(model.index(0, 0, QModelIndex()), Qt::UserRole, url.toLocalFile(), 1, Qt::MatchExactly);
        if (l.length()) {
            return model.itemFromIndex(l.at(0));
        }
        return nullptr;
    }

    static QStandardItem *getItem(const QStandardItem *topItem, KTextEditor::Cursor pos, bool onlyLine)
    {
        QStandardItem *targetItem = nullptr;
        if (topItem) {
            int count = topItem->rowCount();
            int first = 0, last = count;
            // let's not run wild on a linear search in a flood of diagnostics
            if (count > 50) {
                // instead, let's *assume* sorted and use binary search to get closer
                // it probably is sorted, so it should work out
                // if not, at least we tried (without spending/wasting more on sorting)
                auto getLine = [topItem, count](int index) {
                    Q_ASSERT(index >= 0);
                    Q_ASSERT(index < count);
                    auto range = topItem->child(index)->data(RangeData::RangeRole).value<LSPRange>();
                    return range.start().line();
                };
                int first = 0, last = count - 1;
                int target = pos.line();
                while (first + 1 < last) {
                    int middle = first + (last - first) / 2;
                    Q_ASSERT(first != middle);
                    Q_ASSERT(middle != last);
                    if (getLine(middle) < target) {
                        first = middle;
                    } else {
                        last = middle;
                    }
                }
            }
            for (int i = first; i < last; ++i) {
                auto item = topItem->child(i);
                if (!(item->flags() & Qt::ItemIsEnabled)) {
                    continue;
                }
                auto range = item->data(RangeData::RangeRole).value<LSPRange>();
                if ((onlyLine && pos.line() == range.start().line()) || (range.contains(pos))) {
                    targetItem = item;
                    break;
                }
            }
        }
        return targetItem;
    }

    // select/scroll to diagnostics item for document and (optionally) line
    bool syncDiagnostics(KTextEditor::Document *document, int line, bool allowTop, bool doShow)
    {
        if (!m_diagnosticsTree) {
            return false;
        }

        auto hint = QAbstractItemView::PositionAtTop;
        QStandardItem *topItem = getItem(*m_diagnosticsModel, document->url());
        updateDiagnosticsSuppression(topItem, document);
        QStandardItem *targetItem = getItem(topItem, {line, 0}, true);
        if (targetItem) {
            hint = QAbstractItemView::PositionAtCenter;
        }
        if (!targetItem && allowTop) {
            targetItem = topItem;
        }
        if (targetItem) {
            m_diagnosticsTree->blockSignals(true);
            m_diagnosticsTree->scrollTo(targetItem->index(), hint);
            m_diagnosticsTree->setCurrentIndex(targetItem->index());
            m_diagnosticsTree->blockSignals(false);
            if (doShow) {
                m_tabWidget->setCurrentWidget(m_diagnosticsTree);
                m_mainWindow->showToolView(m_toolView.data());
            }
        }
        return targetItem != nullptr;
    }

    void onViewState(KTextEditor::View *view, LSPClientViewTracker::State newState)
    {
        if (!view || !view->document()) {
            return;
        }

        // select top item on view change,
        // but otherwise leave selection unchanged if no match
        switch (newState) {
        case LSPClientViewTracker::ViewChanged:
            syncDiagnostics(view->document(), view->cursorPosition().line(), true, false);
            break;
        case LSPClientViewTracker::LineChanged:
            syncDiagnostics(view->document(), view->cursorPosition().line(), false, false);
            break;
        default:
            // should not happen
            break;
        }
    }

    Q_SLOT void onDiagnosticsMenu(const QPoint &pos)
    {
        Q_UNUSED(pos);

        auto treeView = m_diagnosticsTree.data();
        auto menu = new QMenu(m_diagnosticsTreeOwn.data());
        menu->addAction(i18n("Expand All"), treeView, &QTreeView::expandAll);
        menu->addAction(i18n("Collapse All"), treeView, &QTreeView::collapseAll);
        menu->addSeparator();

        QModelIndex index = treeView->currentIndex();
        auto item = m_diagnosticsModel->itemFromIndex(index);
        auto diagItem = dynamic_cast<DiagnosticItem *>(item);
        auto docDiagItem = dynamic_cast<DocumentDiagnosticItem *>(item);
        if (diagItem) {
            auto diagText = index.data().toString();
            menu->addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy to Clipboard"), [diagText]() {
                QClipboard *clipboard = QGuiApplication::clipboard();
                clipboard->setText(diagText);
            });
            menu->addSeparator();
            auto parent = index.parent();
            docDiagItem = dynamic_cast<DocumentDiagnosticItem *>(m_diagnosticsModel->itemFromIndex(parent));
            // track validity of raw pointer
            QPersistentModelIndex pindex(parent);
            auto h = [this, pindex, diagText, docDiagItem](bool add, const QString &file, const QString &diagnostic) {
                if (!pindex.isValid()) {
                    return;
                }
                if (add) {
                    m_sessionDiagnosticSuppressions.add(file, diagnostic);
                } else {
                    m_sessionDiagnosticSuppressions.remove(file, diagnostic);
                }
                updateDiagnosticsSuppression(docDiagItem, docDiagItem->m_diagnosticSuppression->document(), true);
            };
            using namespace std::placeholders;
            const auto empty = QString();
            if (m_sessionDiagnosticSuppressions.hasSuppression(empty, diagText)) {
                menu->addAction(i18n("Remove Global Suppression"), this, std::bind(h, false, empty, diagText));
            } else {
                menu->addAction(i18n("Add Global Suppression"), this, std::bind(h, true, empty, diagText));
            }
            auto file = parent.data(Qt::UserRole).toString();
            if (m_sessionDiagnosticSuppressions.hasSuppression(file, diagText)) {
                menu->addAction(i18n("Remove Local Suppression"), this, std::bind(h, false, file, diagText));
            } else {
                menu->addAction(i18n("Add Local Suppression"), this, std::bind(h, true, file, diagText));
            }
        } else if (docDiagItem) {
            // track validity of raw pointer
            QPersistentModelIndex pindex(index);
            auto h = [this, docDiagItem, pindex](bool enabled) {
                if (pindex.isValid()) {
                    docDiagItem->m_enabled = enabled;
                }
                updateDiagnosticsState(docDiagItem);
            };
            if (docDiagItem->m_enabled) {
                menu->addAction(i18n("Disable Suppression"), this, std::bind(h, false));
            } else {
                menu->addAction(i18n("Enable Suppression"), this, std::bind(h, true));
            }
        }
        menu->popup(treeView->viewport()->mapToGlobal(pos));
    }

    Q_SLOT void onMarkClicked(KTextEditor::Document *document, KTextEditor::Mark mark, bool &handled)
    {
        // no action if no mark was sprinkled here
        if (m_diagnosticsMarks.contains(document) && syncDiagnostics(document, mark.line, false, true)) {
            handled = true;
        }
    }

    void onDiagnostics(const LSPPublishDiagnosticsParams &diagnostics)
    {
        if (!m_diagnosticsTree) {
            return;
        }

        QStandardItemModel *model = m_diagnosticsModel.data();
        QStandardItem *topItem = getItem(*m_diagnosticsModel, diagnostics.uri);

        // current diagnostics row, if one of incoming diagnostics' document
        int row = -1;
        if (!topItem) {
            // no need to create an empty one
            if (diagnostics.diagnostics.empty()) {
                return;
            }
            topItem = new DocumentDiagnosticItem();
            model->appendRow(topItem);
            topItem->setText(diagnostics.uri.toLocalFile());
            topItem->setData(diagnostics.uri.toLocalFile(), Qt::UserRole);
        } else {
            // try to retain current position
            auto currentIndex = m_diagnosticsTree->currentIndex();
            if (currentIndex.parent() == topItem->index()) {
                row = currentIndex.row();
            }
            topItem->setRowCount(0);
        }

        for (const auto &diag : diagnostics.diagnostics) {
            auto item = new DiagnosticItem(diag);
            topItem->appendRow(item);
            QString source;
            if (diag.source.length()) {
                source = QStringLiteral("[%1] ").arg(diag.source);
            }
            if (diag.code.length()) {
                source += QStringLiteral("(%1) ").arg(diag.code);
            }
            item->setData(diagnosticsIcon(diag.severity), Qt::DecorationRole);
            // rendering of lines with embedded newlines does not work so well
            // so ... split message by lines
            auto lines = diag.message.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            item->setText(source + (lines.size() > 0 ? lines[0] : QString()));
            fillItemRoles(item, diagnostics.uri, diag.range, diag.severity);
            // add subsequent lines to subitems
            // no metadata is added to these,
            // as it can be taken from the parent (for marks and ranges)
            for (int l = 1; l < lines.size(); ++l) {
                auto subitem = new QStandardItem();
                subitem->setText(lines[l]);
                item->appendRow(subitem);
            }
            const auto &relatedInfo = diag.relatedInformation;
            for (const auto &related : relatedInfo) {
                if (related.location.uri.isEmpty()) {
                    continue;
                }
                auto relatedItemMessage = new QStandardItem();
                fillItemRoles(relatedItemMessage, related.location.uri, related.location.range, RangeData::KindEnum::Related);
                auto basename = QFileInfo(related.location.uri.toLocalFile()).fileName();
                auto location = QStringLiteral("%1:%2").arg(basename).arg(related.location.range.start().line());
                relatedItemMessage->setText(QStringLiteral("[%1] %2").arg(location).arg(related.message));
                relatedItemMessage->setData(diagnosticsIcon(LSPDiagnosticSeverity::Information), Qt::DecorationRole);
                item->appendRow(relatedItemMessage);
            }
            m_diagnosticsTree->setExpanded(item->index(), true);
        }

        // TODO perhaps add some custom delegate that only shows 1 line
        // and only the whole text when item selected ??
        m_diagnosticsTree->setExpanded(topItem->index(), true);

        updateDiagnosticsState(topItem);
        // also sync updated diagnostic to current position
        auto currentView = m_mainWindow->activeView();
        if (currentView && currentView->document()) {
            if (!syncDiagnostics(currentView->document(), currentView->cursorPosition().line(), false, false)) {
                // avoid jitter; only restore previous if applicable
                if (row >= 0 && row < topItem->rowCount()) {
                    m_diagnosticsTree->scrollTo(topItem->child(row)->index());
                }
            }
        }
    }

    void updateDiagnosticsSuppression(QStandardItem *topItem, KTextEditor::Document *doc, bool force = false)
    {
        if (!topItem || !doc) {
            return;
        }

        auto diagTopItem = static_cast<DocumentDiagnosticItem *>(topItem);
        auto &suppressions = diagTopItem->m_diagnosticSuppression;
        if (!suppressions || force) {
            auto config = m_serverManager->findServerConfig(doc);
            if (config.isObject()) {
                auto supp = new DiagnosticSuppression(this, doc, config.toObject());
                suppressions.reset(supp);
                updateDiagnosticsState(topItem);
            }
        }
    }

    void updateDiagnosticsState(QStandardItem *topItem)
    {
        if (!topItem) {
            return;
        }

        auto diagTopItem = static_cast<DocumentDiagnosticItem *>(topItem);
        auto enabled = diagTopItem->m_enabled;
        auto suppressions = enabled ? diagTopItem->m_diagnosticSuppression.data() : nullptr;

        int totalCount = topItem->rowCount();
        int count = 0;
        for (int i = 0; i < totalCount; ++i) {
            auto item = topItem->child(i);
            auto hide = suppressions && item && suppressions->match(*item);
            // mark accordingly as flag and (un)hide
            auto flags = item->flags();
            const auto ENABLED = Qt::ItemFlag::ItemIsEnabled;
            if ((flags & ENABLED) != !hide) {
                flags = hide ? (flags & ~ENABLED) : (flags | ENABLED);
                item->setFlags(flags);
                m_diagnosticsTree->setRowHidden(item->row(), topItem->index(), hide);
            }
            count += hide ? 0 : 1;
        }
        // adjust file item level text
        auto suppressed = totalCount - count;
        auto text = topItem->data(Qt::UserRole).toString();
        topItem->setText(suppressed ? i18nc("@info", "%1 [suppressed: %2]", text, suppressed) : text);
        // only hide if really nothing below
        m_diagnosticsTree->setRowHidden(topItem->row(), QModelIndex(), totalCount == 0);

        updateMarks();
    }

    void onServerChanged()
    {
        // need to clear suppressions
        // will be filled at suitable time
        auto &model = m_diagnosticsModel;
        for (int i = 0; i < model->rowCount(); ++i) {
            auto diagItem = static_cast<DocumentDiagnosticItem *>(model->item(i));
            diagItem->m_diagnosticSuppression.reset();
        }
        updateState();
    }

    QString onTextHint(KTextEditor::View *view, const KTextEditor::Cursor &position)
    {
        QString result;
        auto document = view->document();

        if (!m_diagnosticsTree || !m_diagnosticsModel || !document) {
            return result;
        }

        bool autoHover = m_autoHover && m_autoHover->isChecked();
        bool diagHover = m_diagnostics && m_diagnostics->isChecked() && m_diagnosticsHover && m_diagnosticsHover->isChecked();

        QStandardItem *topItem = diagHover ? getItem(*m_diagnosticsModel, document->url()) : nullptr;
        QStandardItem *targetItem = getItem(topItem, position, false);
        if (targetItem) {
            result = targetItem->text();
            // also include related info
            int count = targetItem->rowCount();
            for (int i = 0; i < count; ++i) {
                auto item = targetItem->child(i);
                result += QStringLiteral("\n<br>");
                result += item->text();
            }
            // but let's not get carried away too far
            const int maxsize = m_plugin->m_diagnosticsSize;
            if (result.size() > maxsize) {
                result.resize(maxsize);
                result.append(QStringLiteral("..."));
            }
        } else if (autoHover) {
            // only trigger generic hover if no diagnostic to show;
            // avoids interference by generic hover info
            // (which is likely not so useful in this case/position anyway)
            result = m_hover->textHint(view, position);
        }

        return result;
    }

    KTextEditor::View *viewForUrl(const QUrl &url) const
    {
        const auto views = m_mainWindow->views();
        for (auto *view : views) {
            if (view->document()->url() == url) {
                return view;
            }
        }
        return nullptr;
    }

    void addMessage(LSPMessageType level, const QString &category, const QString &msg, const QString &token = {})
    {
        // skip messaging if not enabled
        if (!m_messages->isChecked()) {
            return;
        }

        // use generic output view
        QVariantMap genericMessage;
        genericMessage.insert(QStringLiteral("category"), category);
        genericMessage.insert(QStringLiteral("text"), msg);

        // translate level to the type keys
        QString type;
        switch (level) {
        case LSPMessageType::Error:
            type = QStringLiteral("Error");
            break;
        case LSPMessageType::Warning:
            type = QStringLiteral("Warning");
            break;
        case LSPMessageType::Info:
            type = QStringLiteral("Info");
            break;
        case LSPMessageType::Log:
            type = QStringLiteral("Log");
            break;
        }
        genericMessage.insert(QStringLiteral("type"), type);

        if (!token.isEmpty()) {
            genericMessage.insert(QStringLiteral("token"), token);
        }

        // host application will handle these message for us, including auto-show settings
        Utils::showMessage(genericMessage, m_mainWindow);
    }

    // params type is same for show or log and is treated the same way
    void onMessage(LSPClientServer *server, const LSPLogMessageParams &params)
    {
        // determine server description
        auto message = params.message;
        if (server) {
            message = QStringLiteral("%1\n%2").arg(LSPClientServerManager::serverDescription(server), message);
        }
        addMessage(params.type, i18nc("@info", "LSP Server"), message);
    }

    void onWorkDoneProgress(LSPClientServer *server, const LSPWorkDoneProgressParams &params)
    {
        // provided token is/should be unique (from server perspective)
        // but we are dealing with multiple servers, so let's make a combined token
        const auto token = QStringLiteral("%1:%2").arg((quintptr)server).arg(params.token.toString());
        // find title in matching begin entry (if any)
        QString title;
        // plain search
        // could have used a map, but now we can discard the oldest one if needed
        int index = -1;
        for (int i = 0; i < m_workDoneProgress.size(); ++i) {
            auto &e = m_workDoneProgress.at(i);
            if (e.first == token) {
                index = i;
                title = e.second.value.title;
                break;
            }
        }
        if (index < 0) {
            if (m_workDoneProgress.size() > 10) {
                m_workDoneProgress.pop_front();
            }
            m_workDoneProgress.push_back({token, params});
        } else if (params.value.kind == LSPWorkDoneProgressKind::End) {
            m_workDoneProgress.remove(index);
        }
        // title (likely) only in initial message
        if (title.isEmpty()) {
            title = params.value.title;
        }
        // let's force percentage to 100 to indicate completion
        // (which it might otherwise not be so it seems)
        const auto percentage = params.value.kind == LSPWorkDoneProgressKind::End ? 100 : params.value.percentage;
        const auto msg = QStringLiteral("%1 [%3%] %2").arg(title).arg(params.value.message).arg(percentage, 3);
        addMessage(LSPMessageType::Info, i18nc("@info", "LSP Server"), msg, token);
    }

    void onShowMessage(KTextEditor::Message::MessageType level, const QString &msg)
    {
        // translate level
        LSPMessageType lvl = LSPMessageType::Log;
        using KMessage = KTextEditor::Message;
        switch (level) {
        case KMessage::Error:
            lvl = LSPMessageType::Error;
            break;
        case KMessage::Warning:
            lvl = LSPMessageType::Warning;
            break;
        case KMessage::Information:
            lvl = LSPMessageType::Info;
            break;
        case KMessage::Positive:
            lvl = LSPMessageType::Log;
            break;
        }

        addMessage(lvl, i18nc("@info", "LSP Client"), msg);
    }

    void onDocumentUrlChanged(KTextEditor::Document *doc)
    {
        // url already changed by this time and new url not useful
        (void)doc;
        // note; url also changes when closed
        // spec says;
        // if a language has a project system, diagnostics are not cleared by *server*
        // but in either case (url change or close); remove lingering diagnostics
        // collect active urls
        QSet<QString> fpaths;
        const auto views = m_mainWindow->views();
        for (const auto view : views) {
            if (auto doc = view->document()) {
                fpaths.insert(doc->url().toLocalFile());
            }
        }
        // check and clear defunct entries
        const auto &model = *m_diagnosticsModel;

        // Remove rows in groups
        int start = -1, count = 0;
        for (int i = 0; i < model.rowCount(); ++i) {
            auto item = model.item(i);
            if (item && !fpaths.contains(item->text())) {
                if (start == -1) {
                    start = i;
                }
                count += 1;
            } else {
                if (start > -1 && count != 0) {
                    m_diagnosticsModel->removeRows(start, count);
                    i = start - 1; // reset i
                }
                start = -1;
                count = 0;
            }
        }

        if (start != -1 && count != 0) {
            m_diagnosticsModel->removeRows(start, count);
        }
    }

    void onTextChanged(KTextEditor::Document *doc)
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();
        if (!activeView || activeView->document() != doc) {
            return;
        }

        if (m_plugin->m_semanticHighlighting) {
            m_semHighlightingManager.doSemanticHighlighting(activeView, true);
        }

        if (m_onTypeFormattingTriggers.empty()) {
            return;
        }

        // NOTE the intendation mode should probably be set to None,
        // so as not to experience unpleasant interference
        auto cursor = activeView->cursorPosition();
        QChar lastChar = cursor.column() == 0 ? QChar::fromLatin1('\n') : doc->characterAt({cursor.line(), cursor.column() - 1});
        if (m_onTypeFormattingTriggers.contains(lastChar)) {
            format(lastChar);
        }
    }

    void updateState()
    {
        KTextEditor::View *activeView = m_mainWindow->activeView();

        auto doc = activeView ? activeView->document() : nullptr;
        auto server = m_serverManager->findServer(activeView);
        bool defEnabled = false, declEnabled = false, typeDefEnabled = false, refEnabled = false, implEnabled = false;
        bool hoverEnabled = false, highlightEnabled = false, codeActionEnabled = false;
        bool formatEnabled = false;
        bool renameEnabled = false;
        bool selectionRangeEnabled = false;
        bool isClangd = false;
        bool isRustAnalyzer = false;

        if (server) {
            const auto &caps = server->capabilities();
            defEnabled = caps.definitionProvider;
            declEnabled = caps.declarationProvider;
            typeDefEnabled = caps.typeDefinitionProvider;
            refEnabled = caps.referencesProvider;
            implEnabled = caps.implementationProvider;
            hoverEnabled = caps.hoverProvider;
            highlightEnabled = caps.documentHighlightProvider;
            formatEnabled = caps.documentFormattingProvider || caps.documentRangeFormattingProvider;
            renameEnabled = caps.renameProvider;
            codeActionEnabled = caps.codeActionProvider;
            selectionRangeEnabled = caps.selectionRangeProvider;

            connect(server.data(), &LSPClientServer::publishDiagnostics, this, &self_type::onDiagnostics, Qt::UniqueConnection);
            connect(server.data(), &LSPClientServer::applyEdit, this, &self_type::onApplyEdit, Qt::UniqueConnection);

            // update format trigger characters
            const auto &fmt = caps.documentOnTypeFormattingProvider;
            if (fmt.provider && m_onTypeFormatting->isChecked()) {
                m_onTypeFormattingTriggers = fmt.triggerCharacters;
            } else {
                m_onTypeFormattingTriggers.clear();
            }
            // and monitor for such
            if (doc) {
                connect(doc, &KTextEditor::Document::textChanged, this, &self_type::onTextChanged, Qt::UniqueConnection);
                connect(doc, &KTextEditor::Document::documentUrlChanged, this, &self_type::onDocumentUrlChanged, Qt::UniqueConnection);
                connect(doc, &KTextEditor::Document::reloaded, this, &self_type::updateState, Qt::UniqueConnection);
            }
            // only consider basename (full path may have been custom specified)
            auto lspServer = QFileInfo(server->cmdline().front()).fileName();
            isClangd = lspServer == QStringLiteral("clangd");
            isRustAnalyzer = lspServer == QStringLiteral("rust-analyzer");

            const bool semHighlightingEnabled = m_plugin->m_semanticHighlighting;
            if (semHighlightingEnabled) {
                m_semHighlightingManager.doSemanticHighlighting(activeView, false);
            }

            connect(activeView, &KTextEditor::View::contextMenuAboutToShow, this, &self_type::prepareContextMenu, Qt::UniqueConnection);
        }

        if (m_findDef) {
            m_findDef->setEnabled(defEnabled);
        }
        if (m_findDecl) {
            m_findDecl->setEnabled(declEnabled);
        }
        if (m_findTypeDef) {
            m_findTypeDef->setEnabled(typeDefEnabled);
        }
        if (m_findRef) {
            m_findRef->setEnabled(refEnabled);
        }
        if (m_findImpl) {
            m_findImpl->setEnabled(implEnabled);
        }
        if (m_triggerHighlight) {
            m_triggerHighlight->setEnabled(highlightEnabled);
        }
        if (m_triggerSymbolInfo) {
            m_triggerSymbolInfo->setEnabled(hoverEnabled);
        }
        if (m_triggerFormat) {
            m_triggerFormat->setEnabled(formatEnabled);
        }
        if (m_triggerRename) {
            m_triggerRename->setEnabled(renameEnabled);
        }
        if (m_complDocOn) {
            m_complDocOn->setEnabled(!server.isNull());
        }
        if (m_restartServer) {
            m_restartServer->setEnabled(!server.isNull());
        }
        if (m_requestCodeAction) {
            m_requestCodeAction->setEnabled(codeActionEnabled);
        }
        if (m_expandSelection) {
            m_expandSelection->setEnabled(selectionRangeEnabled);
        }
        if (m_shrinkSelection) {
            m_shrinkSelection->setEnabled(selectionRangeEnabled);
        }
        m_switchSourceHeader->setEnabled(isClangd);
        m_switchSourceHeader->setVisible(isClangd);
        m_memoryUsage->setEnabled(isClangd);
        m_memoryUsage->setVisible(isClangd);
        m_expandMacro->setEnabled(isRustAnalyzer);
        m_expandMacro->setVisible(isRustAnalyzer);

        // update completion with relevant server
        m_completion->setServer(server);
        if (m_complDocOn) {
            m_completion->setSelectedDocumentation(m_complDocOn->isChecked());
        }
        if (m_signatureHelp) {
            m_completion->setSignatureHelp(m_signatureHelp->isChecked());
        }
        if (m_complParens) {
            m_completion->setCompleteParens(m_complParens->isChecked());
        }
        updateCompletion(activeView, server.data());

        // update hover with relevant server
        m_hover->setServer(server && server->capabilities().hoverProvider ? server : nullptr);
        // need hover either for generic documentHover or for diagnostics
        // so register anyway if server available and will sort out what to do/show later
        updateHover(activeView, server.data());

        updateMarks(doc);

        // connect for cleanup stuff
        if (activeView) {
            connect(activeView, &KTextEditor::View::destroyed, this, &self_type::viewDestroyed, Qt::UniqueConnection);
        }
    }

    void prepareContextMenu(KTextEditor::View *view, QMenu *menu)
    {
        Q_UNUSED(view);

        // make sure the parent is set
        for (auto *act : m_contextMenuActions) {
            act->setParent(menu);
        }

        QAction *insertBefore;
        // the name is used in KXMLGUI as object name
        // we want to insert before the cut action to ensure the KTE spelling menu is still on top
        auto cutName = QString::fromLatin1(KStandardAction::name(KStandardAction::StandardAction::Cut));

        for (auto *act : menu->actions()) {
            if (act->objectName() == cutName) {
                insertBefore = act;
                break;
            }
        }

        if (!insertBefore) {
            Q_ASSERT(!menu->actions().isEmpty());
            insertBefore = menu->actions().first();
        }

        // insert lsp actions at the beginning of the menu
        menu->insertActions(insertBefore, m_contextMenuActions);

        connect(menu, &QMenu::aboutToHide, this, &self_type::cleanUpContextMenu, Qt::UniqueConnection);
    }

    void cleanUpContextMenu()
    {
        // We need to remove our list or they will accumulated on next show event
        for (auto *act : m_contextMenuActions) {
            act->parentWidget()->removeAction(act);
        }
    }

    void updateMarks(KTextEditor::Document *doc = nullptr)
    {
        if (!doc) {
            KTextEditor::View *activeView = m_mainWindow->activeView();
            doc = activeView ? activeView->document() : nullptr;
        }

        // update marks if applicable
        if (m_markModel && doc) {
            addMarks(doc, m_markModel, m_ranges, m_marks);
        }
        if (m_diagnosticsModel && doc) {
            clearMarks(doc, m_diagnosticsRanges, m_diagnosticsMarks, RangeData::markTypeDiagAll);
            addMarks(doc, m_diagnosticsModel.data(), m_diagnosticsRanges, m_diagnosticsMarks);
        }
    }

    void viewDestroyed(QObject *view)
    {
        m_completionViews.remove(static_cast<KTextEditor::View *>(view));
        m_hoverViews.remove(static_cast<KTextEditor::View *>(view));
    }

    void updateCompletion(KTextEditor::View *view, LSPClientServer *server)
    {
        if (!view) {
            return;
        }

        bool registered = m_completionViews.contains(view);

        KTextEditor::CodeCompletionInterface *cci = qobject_cast<KTextEditor::CodeCompletionInterface *>(view);
        Q_ASSERT(cci);

        if (!registered && server && server->capabilities().completionProvider.provider) {
            qCInfo(LSPCLIENT) << "registering cci";
            cci->registerCompletionModel(m_completion.data());
            m_completionViews.insert(view);
        }

        if (registered && !server) {
            qCInfo(LSPCLIENT) << "unregistering cci";
            cci->unregisterCompletionModel(m_completion.data());
            m_completionViews.remove(view);
        }
    }

    void updateHover(KTextEditor::View *view, LSPClientServer *server)
    {
        if (!view) {
            return;
        }

        bool registered = m_hoverViews.contains(view);

        KTextEditor::TextHintInterface *cci = qobject_cast<KTextEditor::TextHintInterface *>(view);
        Q_ASSERT(cci);

        if (!registered && server) {
            qCInfo(LSPCLIENT) << "registering thi";
            cci->registerTextHintProvider(m_forwardHover.data());
            m_hoverViews.insert(view);
        }

        if (registered && !server) {
            qCInfo(LSPCLIENT) << "unregistering thi";
            cci->unregisterTextHintProvider(m_forwardHover.data());
            m_hoverViews.remove(view);
        }
    }

    Q_INVOKABLE QAbstractItemModel *documentSymbolsModel()
    {
        return m_symbolView->documentSymbolsModel();
    }

    void readSessionConfig(const KConfigGroup &config) override
    {
        sessionDiagnosticSuppressions().readSessionConfig(config);
    }

    void writeSessionConfig(KConfigGroup &config) override
    {
        sessionDiagnosticSuppressions().writeSessionConfig(config);
    }
};

QObject *LSPClientPluginView::new_(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin, QSharedPointer<LSPClientServerManager> manager)
{
    return new LSPClientPluginViewImpl(plugin, mainWin, manager);
}

#include "lspclientpluginview.moc"
