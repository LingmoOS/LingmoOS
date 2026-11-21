// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorpage.h"
#include "application.h"
#include "editorsettings.h"
#include "editorview.h"
#include "editorwindow.h"
#include "pagebars.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "ui/dialog_window.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <cmath>

using namespace Lingmo::ui;
using namespace std::chrono_literals;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

// From the editor's edges, as the other margins of the window.
static constexpr double cardInset = 12;

// The text and what floats over it: the find card, Go to Line and the
// loading view.
class EditorPage::EditorArea : public View
{
public:
    explicit EditorArea(EditorPage &page)
        : m_page(page)
    {
        setName("editorArea");
        setClipsChildren(true);
    }

protected:
    void layout() override
    {
        m_page.m_editor->setGeometry(localRect());
        m_page.placeCards();
    }

    void paint(Painter &painter) override
    {
        painter.fillRect(localRect(), m_page.m_editor->backgroundColor());
    }

private:
    EditorPage &m_page;
};

EditorPage::EditorPage(EditorApplication &application, EditorWindow *window, const Options &options)
    : m_application(application)
    , m_window(window)
    , m_document(std::make_unique<Document>())
{
    setName("editorPage");
    m_infoBar = addChild<InfoBar>(*this);
    m_area = addChild<EditorArea>(*this);
    m_editor = m_area->addChild<EditorView>();
    m_statusBar = addChild<StatusBar>(*this);

    m_editor->setDocument(m_document.get());
    m_editor->setSidePadding(14);
    m_editor->onContextMenuRequested = [this](PointF position) { popupContextMenu(m_editor->mapToWindow(position)); };
    m_editor->onZoomRequested = [this](int step) {
        if (m_window)
            m_window->zoomBy(step);
    };
    m_editor->onFilesDropped = [this](const std::vector<std::string> &paths) {
        if (m_window)
            m_window->openPaths(paths);
    };

    m_loadingDelay.setSingleShot(true);
    m_loadingDelay.setInterval(300ms);
    m_loadingDelay.onTimeout = [this] { updateLoading(); };

    Document &document = *m_document;
    const auto status = [this] { m_statusBar->update(); };
    m_connections.push_back(document.cursorChanged.connect(status));
    m_connections.push_back(document.indentationChanged.connect(status));
    m_connections.push_back(document.languageChanged.connect(status));
    m_connections.push_back(document.lineEndingChanged.connect(status));
    m_connections.push_back(document.encodingChanged.connect(status));
    m_connections.push_back(document.lockedChanged.connect(status));
    m_connections.push_back(document.contentsChanged.connect(status));
    m_connections.push_back(m_editor->overwriteModeChanged.connect(status));
    m_connections.push_back(document.noticeChanged.connect([this] {
        m_infoBar->update();
        requestLayout();
    }));
    m_connections.push_back(document.encodingChanged.connect([this] { m_infoBar->update(); }));
    const auto tab = [this] { tabChanged(); };
    m_connections.push_back(document.titleChanged.connect(tab));
    m_connections.push_back(document.modifiedChanged.connect([this] {
        tabChanged();
        m_application.scheduleSessionSave();
    }));
    m_connections.push_back(document.filePathChanged.connect([this] {
        tabChanged();
        m_application.scheduleSessionSave();
    }));
    const auto find = [this] {
        if (m_findBar)
            m_findBar->refresh();
    };
    m_connections.push_back(document.matchesChanged.connect(find));
    m_connections.push_back(document.searchChanged.connect(find));
    m_connections.push_back(document.cursorChanged.connect(find));
    m_connections.push_back(document.contentsChanged.connect([this] {
        if (m_goToLine)
            m_goToLine->refresh();
    }));
    m_connections.push_back(document.loadingChanged.connect([this] {
        if (m_document->isLoading())
            m_loadingDelay.start();
        else
            m_loadingDelay.stop();
        updateLoading();
    }));
    m_connections.push_back(document.loadProgressChanged.connect([this] {
        if (m_loading)
            m_loading->update();
    }));
    m_connections.push_back(document.loaded.connect([this] { onLoaded(); }));

    applySettings();
    const EditorSettings &settings = m_application.settings();
    document.setIndentationDefaults(settings.insertSpaces, settings.tabWidth);
    if (options.restoreState) {
        const TabState &state = *options.restoreState;
        if (!state.draftId.empty())
            document.setDraftId(state.draftId);
        chosenEncoding = state.encoding;
        chosenLanguage = state.language;
        if (!chosenLanguage.empty())
            document.setLanguage(chosenLanguage);
        m_pendingCursor = PendingCursor{state.line, state.column, state.scroll};
        if (!state.path.empty()) {
            document.load(state.path, chosenEncoding, !options.fromSession);
            if (state.unsaved)
                document.restoreDraft();
        } else if (state.unsaved) {
            document.restoreDraft();
            m_pendingCursor.reset();
            document.setCursor({state.line, state.column});
            document.revealCursor();
            const double scroll = state.scroll;
            Lingmo::base::EventLoop::current()->post([this, scroll, alive = std::weak_ptr<bool>(m_alive)] {
                if (!alive.expired())
                    m_editor->scrollToLine(scroll);
            });
        }
    } else if (!options.initialPath.empty()) {
        m_pendingCursor = PendingCursor{options.initialLine, 0, -1};
        document.load(options.initialPath);
    } else if (options.initialText) {
        document.setText(*options.initialText);
        document.goToLine(1, 1);
    }
    focusEditor();
    m_statusBar->update();
    m_infoBar->update();
}

EditorPage::~EditorPage()
{
    m_connections.clear();
    m_editor->setDocument(nullptr);
}

void EditorPage::setEditorWindow(EditorWindow *window)
{
    m_window = window;
    applySettings();
}

void EditorPage::applySettings()
{
    const EditorSettings &settings = m_application.settings();
    if (m_window)
        m_editor->setFont(m_window->editorFont());
    m_editor->setWordWrap(settings.wordWrap);
    m_editor->setLineNumbers(settings.lineNumbers);
    m_editor->setHighlightCurrentLine(settings.highlightCurrentLine);
    m_editor->setShowWhitespace(settings.showWhitespace);
    m_editor->setMatchBrackets(settings.matchBrackets);
    m_editor->setAutoIndent(settings.autoIndent);
    applyTheme();
    m_statusBar->update();
    if (m_findBar)
        m_findBar->refresh();
    requestLayout();
    requestPaint();
}

void EditorPage::retranslate()
{
    if (m_contextMenu)
        m_contextMenu->close();
    m_contextMenu.reset();
    takeChild(m_statusBar).reset();
    m_statusBar = addChild<StatusBar>(*this);
    m_infoBar->update();
    if (m_findBar)
        m_findBar->retranslate();
    if (m_goToLine)
        m_goToLine->refresh();
    if (m_loading)
        m_loading->retranslate();
    if (m_reloadConfirm && m_reloadConfirm->window())
        confirmReload(m_reloadEncoding);
    requestLayout();
}

void EditorPage::focusEditor()
{
    m_editor->setFocus();
}

void EditorPage::open(const std::string &path, int line)
{
    const int index = line > 0 ? line - 1 : 0;
    m_pendingCursor = PendingCursor{index, 0, -1};
    m_document->load(path);
}

TabState EditorPage::sessionState() const
{
    const Document &document = *m_document;
    TabState state;
    state.path = document.filePath();
    state.draftId = document.draftId();
    state.unsaved = document.isModified() || (document.isUntitled() && !document.isEmpty());
    state.line = document.cursorLine();
    state.column = document.cursorColumn();
    state.scroll = m_editor->topLine();
    state.encoding = chosenEncoding;
    state.language = chosenLanguage;
    return state;
}

void EditorPage::showFind(bool replace)
{
    if (m_goToLine)
        m_goToLine->close();
    if (!m_findBar)
        m_findBar = m_area->addChild<FindBar>(*this);
    const Document &document = *m_document;
    std::string text;
    if (document.hasSelection() && document.selectionLength() < 200) {
        std::string selected = document.selectedText();
        if (selected.find('\n') == std::string::npos)
            text = std::move(selected);
    }
    m_findBar->open(text, replace);
    placeCards();
}

void EditorPage::findNext(bool backwards)
{
    if ((!m_findBar || !m_findBar->isVisible()) && m_document->searchText().empty()) {
        showFind(false);
        return;
    }
    if (!m_findBar)
        m_findBar = m_area->addChild<FindBar>(*this);
    m_findBar->find(backwards);
}

void EditorPage::showGoToLine()
{
    if (m_findBar)
        m_findBar->close(false);
    if (!m_goToLine)
        m_goToLine = m_area->addChild<GoToLineBar>(*this);
    m_goToLine->open();
    placeCards();
}

View *EditorPage::card() const
{
    if (m_findBar && m_findBar->isVisible())
        return m_findBar;
    if (m_goToLine && m_goToLine->isVisible())
        return m_goToLine;
    return nullptr;
}

void EditorPage::placeCards()
{
    const RectF bounds = m_area->localRect();
    if (m_findBar) {
        const double width = std::min(460.0, bounds.width - cardInset * 2);
        m_findBar->setGeometry({bounds.width - cardInset - width, cardInset, width, m_findBar->implicitHeight()});
    }
    if (m_goToLine) {
        const double width = std::min(GoToLineBar::implicitWidth, bounds.width - cardInset * 2);
        m_goToLine->setGeometry({bounds.width - cardInset - width, cardInset, width, m_goToLine->implicitHeight()});
    }
    if (m_loading)
        m_loading->setGeometry(bounds);
    cardChanged();
}

void EditorPage::cardChanged()
{
    const View *open = card();
    if (open)
        m_editor->setOcclusion(open->geometry().x - 8, open->geometry().y + open->geometry().height + 6);
    else
        m_editor->setOcclusion(0, 0);
    if (!open)
        settleScroll();
}

// Scrolled above the first line for a card that is gone: back to it.
void EditorPage::settleScroll()
{
    if (m_editor->contentY() < 0)
        m_editor->setContentPosition(m_editor->contentX(), 0);
}

void EditorPage::setEncodingForReload(const std::string &encoding)
{
    chosenEncoding = encoding;
    if (m_document->isModified() && !m_document->isUntitled()) {
        confirmReload(encoding);
        return;
    }
    m_document->reloadWithEncoding(encoding);
}

void EditorPage::showEncodingMenu()
{
    const PointF position = m_statusBar->mapToWindow({m_statusBar->geometry().width - 40, 14});
    m_statusBar->openEncodingMenu(position);
}

void EditorPage::revert()
{
    if (!m_document->isModified()) {
        m_document->reload();
        return;
    }
    confirmReload({});
}

void EditorPage::confirmReload(const std::string &encoding)
{
    if (!m_window)
        return;
    m_reloadEncoding = encoding;
    m_reloadConfirm = std::make_unique<ConfirmDialog>(&m_window->window(),
                                                      encoding.empty() ? format(tr("Revert \xE2\x80\x9C{name}\xE2\x80\x9D?"),
                                                                                {{"name", title()}})
                                                                       : format(tr("Reopen with {encoding}?"),
                                                                                {{"encoding", EditorApplication::encodingLabel(encoding)}}),
                                                      tr("Your unsaved changes will be lost."));
    m_reloadConfirm->setConfirmText(encoding.empty() ? tr("Revert") : tr("Reopen"));
    m_reloadConfirm->setDestructive(true);
    m_reloadConfirm->onAccepted = [this] {
        if (!m_reloadEncoding.empty())
            m_document->reloadWithEncoding(m_reloadEncoding);
        else
            m_document->reload();
    };
    m_reloadConfirm->show();
}

void EditorPage::popupContextMenu(PointF position)
{
    Document &document = *m_document;
    m_contextMenu = std::make_unique<Menu>();
    Menu &menu = *m_contextMenu;
    menu.setItemEnabled(menu.addItem(tr("Undo"), [&document] { document.undo(); }, "Ctrl+Z"), document.canUndo());
    menu.setItemEnabled(menu.addItem(tr("Redo"), [&document] { document.redo(); }, "Ctrl+Shift+Z"), document.canRedo());
    menu.addSeparator();
    menu.setItemEnabled(menu.addItem(tr("Cut"), [this] { m_editor->cut(); }, "Ctrl+X"),
                        document.hasSelection() && !document.isLocked());
    menu.setItemEnabled(menu.addItem(tr("Copy"), [this] { m_editor->copy(); }, "Ctrl+C"), document.hasSelection());
    menu.setItemEnabled(menu.addItem(tr("Paste"), [this] { m_editor->paste(); }, "Ctrl+V"), !document.isLocked());
    menu.setItemEnabled(menu.addItem(tr("Delete"), [&document] { document.deleteSelection(); }, "Del"),
                        document.hasSelection() && !document.isLocked());
    menu.addSeparator();
    menu.addItem(tr("Select All"), [&document] { document.selectAll(); }, "Ctrl+A");
    if (Window *window = this->window())
        menu.popup(window, position);
}

void EditorPage::onLoaded()
{
    if (!m_pendingCursor)
        return;
    const PendingCursor state = *m_pendingCursor;
    m_pendingCursor.reset();
    // Columns from the session are bytes; a line from the command line starts its line.
    m_document->setCursor({state.line, state.column});
    m_document->revealCursor();
    Lingmo::base::EventLoop::current()->post([this, state, alive = std::weak_ptr<bool>(m_alive)] {
        if (alive.expired())
            return;
        if (state.scroll >= 0)
            m_editor->scrollToLine(state.scroll);
        else
            m_editor->ensureCursorVisible(true);
    });
}

void EditorPage::updateLoading()
{
    const bool loading = m_document->isLoading();
    const bool showing = loading && !m_loadingDelay.isActive();
    // A file that takes a moment shows its progress; a quick one shows nothing.
    m_editor->setVisible(!loading || m_loadingDelay.isActive());
    if (showing && !m_loading)
        m_loading = m_area->addChild<LoadingView>(*this);
    if (m_loading) {
        m_loading->setVisible(showing);
        m_loading->update();
    }
    m_area->requestLayout();
    m_area->requestPaint();
}

void EditorPage::applyTheme()
{
    const Theme &theme = Theme::current();
    m_editor->setDarkMode(theme.darkMode);
    m_editor->setAccentColor(theme.highlight);
    const Color background = m_editor->backgroundColor();
    m_editor->setScrollBarDarkMode(0.2126 * background.r + 0.7152 * background.g + 0.0722 * background.b < 0.5 * 255);
}

void EditorPage::layout()
{
    applyTheme();
    const double width = geometry().width;
    const double height = geometry().height;
    double top = 0;
    if (m_infoBar->isVisible()) {
        const double infoHeight = m_infoBar->heightForWidth(width);
        m_infoBar->setGeometry({0, 0, width, infoHeight});
        top = infoHeight;
    }
    const double statusHeight = 28;
    m_statusBar->setGeometry({0, height - statusHeight, width, statusHeight});
    m_area->setGeometry({0, top, width, std::max(0.0, height - statusHeight - top)});
}

void EditorPage::paint(Painter &)
{
}
