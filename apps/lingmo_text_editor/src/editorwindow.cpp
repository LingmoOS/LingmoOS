// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorwindow.h"
#include "application.h"
#include "colors.h"
#include "dialogs.h"
#include "editorsettings.h"
#include "editorview.h"
#include "pagebars.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/strings.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/drag.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/frame_client.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/text_layout.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "ui/window.h"
#include "ui/window_header.h"
#include "ui/window_tab_bar.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
namespace strings = Lingmo::base::strings;

static constexpr double headerHeight = 40;
static constexpr const char *tabMimeType = "application/x-lingmo-text-editor-tab";

// The header above the tabs' pages, the current one shown.
class EditorWindow::Content : public View
{
public:
    Content()
        : header(addChild<WindowHeader>(headerHeight))
        , pages(addChild<View>())
    {
        setName("content");
        header->setButtonMargin(4);
        header->setButtonsCentered(true);
        pages->setName("pages");
    }

    WindowHeader *header;
    View *pages;

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        header->setGeometry({0, 0, width, headerHeight});
        pages->setGeometry({0, headerHeight, width, std::max(0.0, height - headerHeight)});
        for (const auto &page : pages->children())
            page->setGeometry(pages->localRect());
    }
};

// The tab bar, the "+" after the last tab and the main menu button.
class EditorWindow::HeaderContent : public View
{
public:
    explicit HeaderContent(EditorWindow *owner)
        : owner(owner)
        , tabBar(addChild<WindowTabBar>())
        , newTab(addChild<HeaderButton>("add", 32))
        , menu(addChild<HeaderButton>("menu", 32))
    {
        setName("headerContent");
        tabBar->setName("tabBar");
        newTab->setName("newTabButton");
        newTab->setToolTip({});
        menu->setName("menuButton");
    }

    EditorWindow *owner;
    WindowTabBar *tabBar;
    HeaderButton *newTab;
    HeaderButton *menu;

protected:
    // Tabs dragged from another window of this process, and files.
    bool canDrop(const DropOffer &offer) const override
    {
        if (offer.hasMimeType("text/uri-list"))
            return true;
        const DragSource *source = offer.localSource();
        return source && offer.hasMimeType(tabMimeType) && source->context && source->context != owner;
    }

    void dragEnter(const DragEvent &event) override
    {
        const DragSource *source = event.offer ? event.offer->localSource() : nullptr;
        if (!source || !event.offer->hasMimeType(tabMimeType))
            return;
        const auto *from = static_cast<const EditorWindow *>(source->context);
        const EditorPage *page = from->currentPage();
        incoming = true;
        tabBar->beginIncoming(page ? page->title() : std::string(), event.position.x - tabBar->geometry().x);
        followTrack();
    }

    uint32_t dragUpdate(const DragEvent &event) override
    {
        if (event.offer && event.offer->hasMimeType(tabMimeType) && event.offer->localSource()) {
            tabBar->setIncomingX(event.position.x - tabBar->geometry().x);
            followTrack();
            return MoveAction;
        }
        return CopyAction;
    }

    void dragLeave() override
    {
        if (!incoming)
            return;
        incoming = false;
        tabBar->endIncoming(false);
        followTrack();
    }

    uint32_t drop(const DragEvent &event) override
    {
        DropOffer *offer = event.offer;
        const DragSource *source = offer ? offer->localSource() : nullptr;
        if (source && offer->hasMimeType(tabMimeType)) {
            const int index = tabBar->incomingIndex();
            incoming = false;
            tabBar->endIncoming(true);
            followTrack();
            auto *from = static_cast<EditorWindow *>(const_cast<void *>(source->context));
            std::unique_ptr<EditorPage> page = from->takeDetachedPage();
            if (page) {
                owner->insertTab(std::move(page), index < 0 ? owner->tabCount() : index);
                owner->window().activate();
            }
            return MoveAction;
        }
        if (offer && offer->hasMimeType("text/uri-list")) {
            EditorWindow *window = owner;
            // The source answers when it likes, maybe after the window closed.
            offer->read("text/uri-list", [window, alive = std::weak_ptr<bool>(owner->m_alive)](std::string data) {
                if (alive.expired())
                    return;
                std::vector<std::string> paths;
                for (const std::string &line : strings::split(data, '\n')) {
                    const std::string_view url = strings::trimmed(line);
                    if (url.empty() || url[0] == '#')
                        continue;
                    if (const std::optional<std::string> path = Lingmo::base::url::toLocalFile(url))
                        paths.push_back(*path);
                }
                window->openPaths(paths);
            });
            return CopyAction;
        }
        return NoAction;
    }

    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        // At the end, with the window button spacing before the window buttons.
        menu->setGeometry({width - 32, rounding::centeredStart(height, 32), 32, 32});
        // The terminal's inset; past the new tab button, a stretch of title
        // bar stays free to move the window by.
        const double left = Theme::current().smallSpacing;
        const double right = width - 32 - (4 + 32 + 40);
        const double tabHeight = tabBar->sizeHint().height;
        tabBar->setGeometry({left, rounding::centeredStart(height, tabHeight), std::max(0.0, right - left), tabHeight});
        placeNewTab();
    }

public:
    // The bar's tabs made to match the pages: removed, moved and inserted,
    // so the bar animates them.
    void syncTabs(const std::vector<EditorPage *> &pages, int current)
    {
        for (int i = int(barPages.size()) - 1; i >= 0; --i) {
            if (std::find(pages.begin(), pages.end(), barPages[size_t(i)]) == pages.end()) {
                tabBar->removeTab(i);
                barPages.erase(barPages.begin() + i);
            }
        }
        for (size_t i = 0; i < pages.size(); ++i) {
            if (i < barPages.size() && barPages[i] == pages[i])
                continue;
            const auto found = std::find(barPages.begin() + long(i), barPages.end(), pages[i]);
            if (found != barPages.end()) {
                tabBar->moveTab(int(found - barPages.begin()), int(i));
                barPages.erase(found);
            } else {
                tabBar->insertTab(int(i), pages[i]->title());
            }
            barPages.insert(barPages.begin() + long(i), pages[i]);
        }
        updateLabels();
        if (current >= 0 && tabBar->currentIndex() != current)
            tabBar->setCurrentIndex(current);
        followTrack();
    }

    void updateLabels()
    {
        for (size_t i = 0; i < barPages.size(); ++i) {
            const std::string title = barPages[i]->title();
            if (tabBar->tabTitle(int(i)) != title)
                tabBar->setTabTitle(int(i), title);
            tabBar->setTabModified(int(i), barPages[i]->modified());
        }
    }

    // The track eases while tabs slide; the "+" follows it frame by frame,
    // placed as the header lays out, after the bar has moved.
    void followTrack()
    {
        requestLayout();
        Window *window = this->window();
        if (!window)
            return;
        if (!follower || follower->window() != window)
            follower = std::make_unique<TrackFollower>(window, this);
        follower->kick();
    }

private:
    class TrackFollower final : public FrameClient
    {
    public:
        TrackFollower(Window *window, HeaderContent *header)
            : FrameClient(window)
            , m_header(header)
        {
        }

        void kick()
        {
            // Longer than the bar's slide and settle.
            m_framesLeft = 40;
            startTicking();
        }

    protected:
        void tick(uint64_t) override
        {
            const double width = m_header->tabBar->trackWidth();
            m_header->requestLayout();
            // Started again, so the next frame ticks it after the bar.
            stopTicking();
            if (--m_framesLeft > 0 || width != m_lastWidth)
                startTicking();
            m_lastWidth = width;
        }

    private:
        HeaderContent *m_header;
        int m_framesLeft = 0;
        double m_lastWidth = -1;
    };

    // Follows the last tab.
    void placeNewTab()
    {
        // A crowded track scrolls inside the bar: the "+" stays at its end.
        const double track = std::min(tabBar->trackWidth(), tabBar->geometry().width);
        newTab->setGeometry({tabBar->geometry().x + track + 4, rounding::centeredStart(geometry().height, 32), 32, 32});
    }

    std::vector<EditorPage *> barPages;
    std::unique_ptr<TrackFollower> follower;
    bool incoming = false;
};

EditorWindow::EditorWindow(EditorApplication &application, Options options)
    : m_application(application)
    , m_window(std::make_unique<Window>())
    , m_zoom(options.zoom)
{
    m_window->setMinimumSize({480, 320});
    m_window->resize(options.size.width > 0 ? options.size : SizeF{900, 640});
    if (options.restoresSize)
        m_window->setRestoresSize("Main");
    m_window->setBackgroundColor(EditorColor::WindowBackground);

    auto content = std::make_unique<Content>();
    m_content = content.get();
    m_header = m_content->header->setContent<HeaderContent>(this);
    m_window->setContentView(std::move(content));

    WindowTabBar *tabBar = m_header->tabBar;
    tabBar->setLoneTabDraggable(false);
    tabBar->onCurrentChanged = [this](int index) { selectTab(index); };
    tabBar->onCloseRequested = [this](int index) { closeTab(index); };
    tabBar->onMoveRequested = [this](int from, int to) { moveTab(from, to); };
    tabBar->onContextMenuRequested = [this](int index, PointF position) { openTabMenu(index, position); };
    tabBar->onDetachRequested = [this](int index, PointF position) {
        // A lone tab has nowhere to go; the window moves instead.
        if (m_pages.size() < 2) {
            m_header->tabBar->cancelDetach();
            m_window->startMove();
            return;
        }
        // Later, not from inside the tab bar's handler: taking the tab out
        // removes the tab it runs in.
        Lingmo::base::EventLoop::current()->post([this, index, position, alive = std::weak_ptr<bool>(m_alive)] {
            if (!alive.expired())
                detachTab(index, position);
        });
    };
    m_header->newTab->onClicked = [this] { newTab(); };
    m_header->menu->onClicked = [this] { showMainMenu(); };

    m_window->onCloseRequested = [this] { requestClose(); };
    m_window->onActiveChanged = [this] {
        if (!m_window->isActive())
            return;
        m_application.windowActivated(this);
        if (EditorPage *page = currentPage())
            page->document().checkDisk();
    };
    m_connections.push_back(m_application.settings().changed.connect([this] { applySettings(); }));
    m_languageChanged = Application::instance()->languageChanged.connect([this] { retranslate(); });
    addShortcuts();

    if (options.detachedPage) {
        insertTab(std::move(options.detachedPage), 0);
    } else {
        for (const TabState &state : options.initialTabs) {
            EditorPage::Options page;
            page.restoreState = state;
            page.fromSession = true;
            newTab(page);
        }
        if (m_pages.empty())
            newTab();
        selectTab(std::clamp(options.initialIndex, 0, int(m_pages.size()) - 1));
    }
    updateTitle();
}

EditorWindow::~EditorWindow()
{
    m_pageConnections.clear();
    m_connections.clear();
}

void EditorWindow::retranslate()
{
    if (m_mainMenu)
        m_mainMenu->close();
    m_mainMenu.reset();
    if (m_tabMenu)
        m_tabMenu->close();
    m_tabMenu.reset();
    for (EditorPage *page : m_pages)
        page->retranslate();
    m_header->updateLabels();
    updateTitle();

    if (m_statistics && m_statistics->window())
        showStatistics();
    if (m_preferences && m_preferences->window()) {
        m_preferences.reset();
        showPreferences();
    }
    if (m_shortcuts && m_shortcuts->window()) {
        m_shortcuts.reset();
        showShortcuts();
    }
    if (m_about && m_about->window()) {
        m_about.reset();
        showAbout();
    }
    if (m_saveDialog)
        m_saveDialog->retranslate();
}

void EditorWindow::zoomBy(int step)
{
    static constexpr double steps[] = {0.5, 0.67, 0.8, 0.9, 1, 1.1, 1.25, 1.5, 1.75, 2, 2.5, 3};
    static constexpr int count = int(std::size(steps));
    if (step == 0) {
        m_zoom = 1;
    } else {
        int index = count - 1;
        for (int i = 0; i < count; ++i) {
            if (steps[i] >= m_zoom - 0.001) {
                index = i;
                break;
            }
        }
        index = std::clamp(index + step, 0, count - 1);
        m_zoom = steps[index];
    }
    for (EditorPage *page : m_pages)
        page->applySettings();
}

FontSpec EditorWindow::editorFont() const
{
    const EditorSettings &settings = m_application.settings();
    const FontSpec system = systemFixedFont();
    // Without a size of its own the editor draws at 10 pt, whatever the
    // system fixed font's size.
    const double size = settings.fontSize > 0 ? settings.fontSize : 10;
    return FontSpec{settings.fontFamily.empty() ? system.family : settings.fontFamily,
                    std::max(4.0, std::round(size * m_zoom * 2) / 2)};
}

WindowTabBar *EditorWindow::tabBar() const
{
    return m_header->tabBar;
}

View *EditorWindow::headerContent() const
{
    return m_header;
}

EditorPage *EditorWindow::currentPage() const
{
    return m_currentIndex >= 0 && m_currentIndex < int(m_pages.size()) ? m_pages[size_t(m_currentIndex)] : nullptr;
}

int EditorWindow::indexOfPage(const EditorPage *page) const
{
    const auto it = std::find(m_pages.begin(), m_pages.end(), page);
    return it == m_pages.end() ? -1 : int(it - m_pages.begin());
}

void EditorWindow::attach(EditorPage *page)
{
    page->setEditorWindow(this);
}

void EditorWindow::pagesChanged()
{
    m_pageConnections.clear();
    for (EditorPage *page : m_pages) {
        m_pageConnections.push_back(page->tabChanged.connect([this] {
            m_header->updateLabels();
            updateTitle();
        }));
    }
    m_header->syncTabs(m_pages, m_currentIndex);
    updateTitle();
}

EditorPage *EditorWindow::newTab(const EditorPage::Options &options, int index)
{
    auto page = std::make_unique<EditorPage>(m_application, this, options);
    EditorPage *raw = page.get();
    insertTab(std::move(page), index < 0 ? int(m_pages.size()) : index);
    m_application.scheduleSessionSave();
    return raw;
}

void EditorWindow::insertTab(std::unique_ptr<EditorPage> page, int index)
{
    EditorPage *raw = page.get();
    index = std::clamp(index, 0, int(m_pages.size()));
    attach(raw);
    m_content->pages->addChild(std::move(page));
    m_pages.insert(m_pages.begin() + index, raw);
    m_content->requestLayout();
    showPage(index);
    raw->focusEditor();
    m_application.scheduleSessionSave();
}

std::unique_ptr<EditorPage> EditorWindow::takePage(int index)
{
    if (index < 0 || index >= int(m_pages.size()))
        return nullptr;
    EditorPage *page = m_pages[size_t(index)];
    m_pages.erase(m_pages.begin() + index);
    std::unique_ptr<View> view = m_content->pages->takeChild(page);
    view.release();
    std::unique_ptr<EditorPage> owned(page);
    if (m_currentIndex >= int(m_pages.size()) || m_currentIndex > index)
        --m_currentIndex;
    m_currentIndex = std::clamp(m_currentIndex, m_pages.empty() ? -1 : 0, int(m_pages.size()) - 1);
    showPage(m_currentIndex);
    return owned;
}

void EditorWindow::showPage(int index)
{
    m_currentIndex = index;
    for (size_t i = 0; i < m_pages.size(); ++i)
        m_pages[i]->setVisible(int(i) == index);
    pagesChanged();
    m_application.scheduleSessionSave();
}

void EditorWindow::selectTab(int index)
{
    if (index < 0 || index >= int(m_pages.size()))
        return;
    showPage(index);
    m_pages[size_t(index)]->focusEditor();
}

void EditorWindow::cycleTab(int step)
{
    const int count = int(m_pages.size());
    if (count > 1)
        selectTab((m_currentIndex + step + count) % count);
}

void EditorWindow::moveTab(int from, int to)
{
    const int count = int(m_pages.size());
    if (from == to || from < 0 || to < 0 || from >= count || to >= count)
        return;
    EditorPage *page = m_pages[size_t(from)];
    m_pages.erase(m_pages.begin() + from);
    m_pages.insert(m_pages.begin() + to, page);
    showPage(to);
    page->focusEditor();
}

void EditorWindow::updateTitle()
{
    const EditorPage *page = currentPage();
    m_window->setTitle(page ? page->title() : tr("Text Editor"));
}

int EditorWindow::indexOfPath(const std::string &absolute, const std::string &canonical) const
{
    for (size_t i = 0; i < m_pages.size(); ++i) {
        const std::string &path = m_pages[i]->document().filePath();
        if (!path.empty() && (path == absolute || path == canonical))
            return int(i);
    }
    return -1;
}

bool EditorWindow::isPristine(const EditorPage *page)
{
    if (!page)
        return false;
    const Document &document = page->document();
    return document.isUntitled() && document.isEmpty() && !document.isModified() && !document.isLoading();
}

void EditorWindow::openFile(const std::string &path, int line)
{
    const int existing = indexOfPath(path, path);
    if (existing >= 0) {
        selectTab(existing);
        if (line > 0)
            currentPage()->document().goToLine(line, 1);
        return;
    }
    if (isPristine(currentPage())) {
        currentPage()->open(path, line);
    } else {
        EditorPage::Options options;
        options.initialPath = path;
        options.initialLine = line > 0 ? line - 1 : 0;
        newTab(options);
    }
    EditorSettings &settings = m_application.settings();
    const std::string folder = std::filesystem::path(path).parent_path().string();
    if (settings.lastFolder != folder) {
        settings.lastFolder = folder;
        settings.commit();
    }
}

void EditorWindow::openText(const std::string &text)
{
    EditorPage::Options options;
    options.initialText = text;
    newTab(options);
}

void EditorWindow::openPaths(const std::vector<std::string> &paths)
{
    if (!paths.empty())
        m_application.openPaths(paths, 0, this);
}

std::vector<TabState> EditorWindow::sessionTabs() const
{
    std::vector<TabState> tabs;
    for (const EditorPage *page : m_pages)
        tabs.push_back(page->sessionState());
    return tabs;
}

void EditorWindow::confirmDiscard(const std::vector<EditorPage *> &dirty, std::function<void()> action)
{
    if (dirty.empty()) {
        action();
        return;
    }
    if (!m_saveDialog) {
        m_saveDialog = std::make_unique<SaveChangesDialog>(m_window.get());
        m_saveDialog->onSaveRequested = [this](EditorPage *page, std::function<void(bool)> done) {
            savePage(page, [done](bool saved, bool) { done(saved); });
        };
    }
    m_saveDialog->ask(dirty, std::move(action));
}

void EditorWindow::closeTab(int index, bool ask)
{
    if (index < 0 || index >= int(m_pages.size()))
        return;
    EditorPage *page = m_pages[size_t(index)];
    if (ask && page->document().isModified()) {
        confirmDiscard({page}, [this, page, alive = std::weak_ptr<bool>(m_alive)] {
            if (!alive.expired())
                closeTab(indexOfPage(page), false);
        });
        return;
    }
    // Kept for Reopen Closed Tab; an unsaved text stays in its draft.
    if (!isPristine(page)) {
        TabState state = page->sessionState();
        Document &document = page->document();
        if (document.isModified() || (document.isUntitled() && !document.isEmpty())) {
            document.writeDraftNow();
            state.unsaved = true;
        } else {
            document.removeDraft();
        }
        m_application.pushClosedTab(state);
    }
    std::unique_ptr<EditorPage> taken = takePage(index);
    // The page may be the one handling this event; it goes once the event is over.
    std::shared_ptr<EditorPage> doomed(std::move(taken));
    Lingmo::base::EventLoop::current()->post([doomed] {});
    m_application.scheduleSessionSave();
    if (m_pages.empty()) {
        m_closeConfirmed = true;
        requestClose();
        return;
    }
    if (EditorPage *current = currentPage())
        current->focusEditor();
}

void EditorWindow::closeCurrentTab()
{
    closeTab(m_currentIndex);
}

void EditorWindow::closeOtherTabs(int index)
{
    if (index < 0 || index >= int(m_pages.size()))
        return;
    EditorPage *keep = m_pages[size_t(index)];
    std::vector<EditorPage *> others;
    std::vector<EditorPage *> dirty;
    for (EditorPage *page : m_pages) {
        if (page == keep)
            continue;
        others.push_back(page);
        if (page->document().isModified())
            dirty.push_back(page);
    }
    confirmDiscard(dirty, [this, keep, others, alive = std::weak_ptr<bool>(m_alive)] {
        if (alive.expired())
            return;
        for (EditorPage *page : others)
            closeTab(indexOfPage(page), false);
        selectTab(indexOfPage(keep));
    });
}

void EditorWindow::closeTabsToRight(int index)
{
    std::vector<EditorPage *> right(m_pages.begin() + std::min(index + 1, int(m_pages.size())), m_pages.end());
    std::vector<EditorPage *> dirty;
    for (EditorPage *page : right) {
        if (page->document().isModified())
            dirty.push_back(page);
    }
    confirmDiscard(dirty, [this, right, alive = std::weak_ptr<bool>(m_alive)] {
        if (alive.expired())
            return;
        for (EditorPage *page : right)
            closeTab(indexOfPage(page), false);
    });
}

void EditorWindow::reopenClosedTab()
{
    const std::optional<TabState> state = m_application.takeClosedTab();
    if (!state || state->draftId.empty())
        return;
    if (!state->path.empty() && indexOfPath(state->path, state->path) >= 0) {
        selectTab(indexOfPath(state->path, state->path));
        return;
    }
    EditorPage::Options options;
    options.restoreState = state;
    newTab(options);
}

void EditorWindow::quitApplication()
{
    if (m_application.settings().restoreSession) {
        m_application.quit();
        return;
    }
    std::vector<EditorPage *> dirty;
    for (EditorWindow *window : m_application.windows()) {
        for (EditorPage *page : window->pages()) {
            if (page->document().isModified())
                dirty.push_back(page);
        }
    }
    EditorApplication &application = m_application;
    confirmDiscard(dirty, [&application] { application.quit(); });
}

void EditorWindow::requestClose()
{
    if (m_application.isQuitting())
        return;
    if (m_closeConfirmed) {
        finishClose();
        return;
    }
    // The last window going is the application quitting.
    if (m_application.windowCount() <= 1) {
        quitApplication();
        return;
    }
    std::vector<EditorPage *> dirty;
    for (EditorPage *page : m_pages) {
        if (page->document().isModified())
            dirty.push_back(page);
    }
    confirmDiscard(dirty, [this, alive = std::weak_ptr<bool>(m_alive)] {
        if (alive.expired())
            return;
        for (EditorPage *page : m_pages)
            page->document().removeDraft();
        m_closeConfirmed = true;
        finishClose();
    });
}

void EditorWindow::finishClose()
{
    m_window->hide();
    m_application.releaseWindow(this);
}

void EditorWindow::savePage(EditorPage *page, std::function<void(bool, bool)> done)
{
    if (!page)
        return;
    Document &document = page->document();
    std::error_code error;
    if (document.isUntitled() || (!document.isWritable() && !std::filesystem::exists(document.filePath(), error))) {
        saveAs(page, std::move(done));
        return;
    }
    const bool saved = document.save();
    if (!saved && document.notice() == Document::ReadOnly) {
        saveAs(page, std::move(done));
        return;
    }
    if (done)
        done(saved, false);
}

void EditorWindow::saveAs(EditorPage *page, std::function<void(bool, bool)> done)
{
    if (!page)
        return;
    const Document &document = page->document();
    const EditorSettings &settings = m_application.settings();
    const std::string folder = document.isUntitled() ? settings.lastFolder
                                                     : std::filesystem::path(document.filePath()).parent_path().string();
    std::string name;
    if (document.isUntitled()) {
        std::string title = document.title();
        for (char &c : title) {
            if (c == '/' || c == '\\' || c == ':')
                c = ' ';
        }
        name = std::string(strings::trimmed(title));
        if (name.empty())
            name = tr("Untitled");
        if (name.find('.') == std::string::npos)
            name += ".txt";
    } else {
        name = document.fileName();
    }

    m_saveChooser = std::make_unique<FileChooser>(m_window.get());
    m_saveChooser->setMode(FileChooser::Mode::SaveFile);
    m_saveChooser->setTitle(tr("Save As"));
    if (!folder.empty())
        m_saveChooser->setCurrentFolder(folder);
    m_saveChooser->setCurrentFile((folder.empty() ? std::string() : folder) + "/" + name);
    auto finished = std::make_shared<std::function<void(bool, bool)>>(std::move(done));
    m_saveChooser->onAccepted = [this, page, finished](const std::vector<std::string> &paths, const std::string &) {
        if (paths.empty() || indexOfPage(page) < 0)
            return;
        const std::string &path = paths.front();
        const bool saved = page->document().saveAs(path);
        if (saved) {
            EditorSettings &settings = m_application.settings();
            settings.lastFolder = std::filesystem::path(path).parent_path().string();
            settings.commit();
        }
        // `done` may open this dialog again for the next page, so it runs last.
        if (*finished)
            (*finished)(saved, false);
    };
    m_saveChooser->onRejected = [finished] {
        if (*finished)
            (*finished)(false, true);
    };
    m_saveChooser->open();
}

void EditorWindow::saveAll()
{
    // A page may be closed while an earlier one is saved as.
    auto dirty = std::make_shared<std::vector<std::pair<EditorPage *, std::weak_ptr<bool>>>>();
    for (EditorPage *page : m_pages) {
        if (page->document().isModified())
            dirty->emplace_back(page, page->lifetime());
    }
    // A failed save shows its notice on its page; only a cancelled dialog stops the rest.
    // Only the save waiting to finish holds the chain; it holds itself weakly.
    auto next = std::make_shared<std::function<void()>>();
    const std::weak_ptr<std::function<void()>> self = next;
    *next = [this, dirty, self, alive = std::weak_ptr<bool>(m_alive)] {
        if (alive.expired() || dirty->empty())
            return;
        const auto [page, lifetime] = dirty->front();
        dirty->erase(dirty->begin());
        if (lifetime.expired()) {
            if (const auto again = self.lock())
                (*again)();
            return;
        }
        savePage(page, [next = self.lock()](bool, bool cancelled) {
            if (!cancelled)
                (*next)();
        });
    };
    (*next)();
}

void EditorWindow::openDialog()
{
    m_openChooser = std::make_unique<FileChooser>(m_window.get());
    m_openChooser->setMode(FileChooser::Mode::OpenFiles);
    m_openChooser->setTitle(tr("Open"));
    const EditorPage *page = currentPage();
    if (page && !page->document().isUntitled())
        m_openChooser->setCurrentFolder(std::filesystem::path(page->document().filePath()).parent_path().string());
    else if (!m_application.settings().lastFolder.empty())
        m_openChooser->setCurrentFolder(m_application.settings().lastFolder);
    m_openChooser->onAccepted = [this](const std::vector<std::string> &paths, const std::string &) {
        m_application.openPaths(paths, 0, this);
    };
    m_openChooser->open();
}

void EditorWindow::showMainMenu()
{
    EditorPage *page = currentPage();
    Document *document = page ? &page->document() : nullptr;
    m_mainMenu = std::make_unique<Menu>();
    Menu &menu = *m_mainMenu;
    Menu *recent = nullptr;
    Menu *edit = nullptr;
    Menu *view = nullptr;
    menu.addItem(tr("Open\xE2\x80\xA6"), [this] { openDialog(); }, "Ctrl+O");
    recent = menu.addSubmenu(tr("Open Recent"));
    menu.setItemEnabled(1, !m_application.recentFiles().files().empty());
    for (const std::string &path : m_application.recentFiles().files()) {
        recent->addItem(std::filesystem::path(path).filename().string(), [this, path] {
            m_application.openPaths({path}, 0, this);
        });
    }
    recent->addSeparator();
    recent->addItem(tr("Clear Recent Files"), [this] { m_application.recentFiles().clear(); });
    menu.addSeparator();
    menu.addItem(tr("New Tab"), [this] { newTab(); }, "Ctrl+T");
    menu.addItem(tr("New Window"), [this] { m_application.newWindow(); }, "Ctrl+N");
    menu.setItemEnabled(menu.addItem(tr("Reopen Closed Tab"), [this] { reopenClosedTab(); }, "Ctrl+Shift+T"),
                        m_application.canReopenClosedTab());
    menu.addSeparator();
    menu.setItemEnabled(menu.addItem(tr("Save"), [this] { savePage(currentPage()); }, "Ctrl+S"), document != nullptr);
    menu.setItemEnabled(menu.addItem(tr("Save As\xE2\x80\xA6"), [this] { saveAs(currentPage()); }, "Ctrl+Shift+S"),
                        document != nullptr);
    menu.addItem(tr("Save All"), [this] { saveAll(); }, "Ctrl+Alt+S");
    menu.setItemEnabled(menu.addItem(tr("Revert to Saved\xE2\x80\xA6"),
                                     [this] {
                                         if (EditorPage *current = currentPage())
                                             current->revert();
                                     }),
                        document && !document->isUntitled() && document->isModified());
    menu.addItem(tr("Close Tab"), [this] { closeCurrentTab(); }, "Ctrl+W");
    menu.addSeparator();
    menu.addItem(tr("Find\xE2\x80\xA6"), [this] {
        if (EditorPage *current = currentPage())
            current->showFind(false);
    }, "Ctrl+F");
    menu.addItem(tr("Replace\xE2\x80\xA6"), [this] {
        if (EditorPage *current = currentPage())
            current->showFind(true);
    }, "Ctrl+H");
    menu.addItem(tr("Go to Line\xE2\x80\xA6"), [this] {
        if (EditorPage *current = currentPage())
            current->showGoToLine();
    }, "Ctrl+I");
    menu.addSeparator();

    edit = menu.addSubmenu(tr("Edit"));
    const auto onDocument = [this](std::function<void(Document &)> action) {
        return [this, action] {
            if (EditorPage *current = currentPage())
                action(current->document());
        };
    };
    edit->setItemEnabled(edit->addItem(tr("Undo"), onDocument([](Document &d) { d.undo(); }), "Ctrl+Z"),
                         document && document->canUndo());
    edit->setItemEnabled(edit->addItem(tr("Redo"), onDocument([](Document &d) { d.redo(); }), "Ctrl+Shift+Z"),
                         document && document->canRedo());
    edit->addSeparator();
    edit->addItem(tr("Duplicate Line"), onDocument([](Document &d) { d.duplicateLines(); }), "Ctrl+Alt+D");
    edit->addItem(tr("Delete Line"), onDocument([](Document &d) { d.deleteLines(); }), "Ctrl+D");
    edit->addItem(tr("Move Line Up"), onDocument([](Document &d) { d.moveLines(true); }), "Alt+Up");
    edit->addItem(tr("Move Line Down"), onDocument([](Document &d) { d.moveLines(false); }), "Alt+Down");
    edit->addSeparator();
    // TRANSLATORS: changes the selection to upper case; written in the case it applies, as are "lowercase" and "Title Case".
    edit->addItem(tr("UPPERCASE"), onDocument([](Document &d) { d.changeCase(Document::Upper); }), "Ctrl+U");
    edit->addItem(tr("lowercase"), onDocument([](Document &d) { d.changeCase(Document::Lower); }), "Ctrl+Shift+U");
    edit->addItem(tr("Title Case"), onDocument([](Document &d) { d.changeCase(Document::Title); }));
    edit->addSeparator();
    edit->addItem(tr("Indent"), onDocument([](Document &d) { d.indentSelection(false); }), "Tab");
    edit->addItem(tr("Unindent"), onDocument([](Document &d) { d.indentSelection(true); }), "Shift+Tab");

    view = menu.addSubmenu(tr("View"));
    EditorSettings &settings = m_application.settings();
    const auto toggle = [&settings](bool EditorSettings::*member) {
        return [&settings, member] {
            settings.*member = !(settings.*member);
            settings.commit();
        };
    };
    view->setItemChecked(view->addItem(tr("Word Wrap"), toggle(&EditorSettings::wordWrap), "Alt+W"), settings.wordWrap);
    view->setItemChecked(view->addItem(tr("Line Numbers"), toggle(&EditorSettings::lineNumbers)), settings.lineNumbers);
    view->setItemChecked(view->addItem(tr("Highlight Current Line"), toggle(&EditorSettings::highlightCurrentLine)),
                         settings.highlightCurrentLine);
    view->setItemChecked(view->addItem(tr("Show Whitespace"), toggle(&EditorSettings::showWhitespace)),
                         settings.showWhitespace);
    view->setItemChecked(view->addItem(tr("Match Brackets"), toggle(&EditorSettings::matchBrackets)), settings.matchBrackets);
    view->addSeparator();
    view->addItem(tr("Zoom In"), [this] { zoomBy(1); }, "Ctrl+=");
    view->addItem(tr("Zoom Out"), [this] { zoomBy(-1); }, "Ctrl+-");
    view->setItemEnabled(view->addItem(tr("Actual Size"), [this] { zoomBy(0); }, "Ctrl+0"), m_zoom != 1);
    view->setItemChecked(view->addItem(tr("Full Screen"), [this] { toggleFullScreen(); }, "F11"), isFullScreen());

    menu.addSeparator();
    menu.addItem(tr("Document Statistics"), [this] { showStatistics(); });
    menu.addSeparator();
    menu.addItem(tr("Settings"), [this] { showPreferences(); }, "Ctrl+,");
    menu.addItem(tr("Keyboard Shortcuts"), [this] { showShortcuts(); }, "Ctrl+?");
    menu.addItem(tr("About Text Editor"), [this] { showAbout(); });

    menu.onClosed = [this, alive = std::weak_ptr<bool>(m_alive)] {
        if (!alive.expired())
            m_header->menu->setChecked(false);
    };
    m_header->menu->setChecked(true);
    menu.popupFor(m_header->menu, 4);
}

void EditorWindow::showPreferences()
{
    if (!m_preferences)
        m_preferences = std::make_unique<PreferencesDialog>(m_window.get(), m_application);
    m_preferences->show();
}

void EditorWindow::showShortcuts()
{
    if (!m_shortcuts)
        m_shortcuts = std::make_unique<ShortcutsDialog>(m_window.get());
    m_shortcuts->show();
}

// A whole number with "," between groups of three digits.
static std::string localeNumber(int64_t value)
{
    std::string digits = std::to_string(value < 0 ? -value : value);
    for (int i = int(digits.size()) - 3; i > 0; i -= 3)
        digits.insert(size_t(i), ",");
    return (value < 0 ? "-" : "") + digits;
}

void EditorWindow::showStatistics()
{
    EditorPage *page = currentPage();
    if (!page)
        return;
    const DocumentStatistics stats = page->document().statistics();
    std::string text = format(tr("Lines: {count}"), {{"count", localeNumber(stats.lines)}}) + "\n"
            + format(tr("Words: {count}"), {{"count", localeNumber(stats.words)}}) + "\n"
            + format(tr("Characters: {count}"), {{"count", localeNumber(stats.characters)}}) + "\n"
            + format(tr("Characters without spaces: {count}"), {{"count", localeNumber(stats.charactersWithoutSpaces)}});
    if (stats.hasSelection)
        // TRANSLATORS: each value is a count with its noun, such as "3 lines".
        text += "\n\n" + format(tr("Selection: {lines}, {words}, {characters}"),
                                {{"lines", trn("{n} line", "{n} lines", uint64_t(stats.selectionLines))},
                                 {"words", trn("{n} word", "{n} words", uint64_t(stats.selectionWords))},
                                 {"characters", trn("{n} character", "{n} characters", uint64_t(stats.selectionCharacters))}});
    m_statistics = std::make_unique<ConfirmDialog>(m_window.get(), page->title(), text);
    m_statistics->setCancelVisible(false);
    m_statistics->setConfirmText(tr("OK"));
    m_statistics->show();
}

void EditorWindow::showAbout()
{
    if (!m_about)
        m_about = std::make_unique<AboutDialog>(m_window.get());
    m_about->show();
}

void EditorWindow::showNotification(const std::string &text, int milliseconds)
{
    if (!m_toast)
        m_toast = std::make_unique<Toast>(m_window.get());
    m_toast->show(text, std::chrono::milliseconds(milliseconds));
}

void EditorWindow::moveToNewWindow(int index)
{
    if (index < 0 || index >= int(m_pages.size()) || m_pages.size() < 2)
        return;
    std::unique_ptr<EditorPage> page = takePage(index);
    Options options;
    options.detachedPage = std::move(page);
    options.size = m_window->size();
    options.zoom = m_zoom;
    options.restoresSize = false;
    m_application.newWindow(std::move(options));
}

// A tab pulled off the bar: a window of its own under the pointer, carried
// by the drag until it is dropped on a tab bar or let go.
void EditorWindow::openTabMenu(int index, PointF position)
{
    if (index < 0 || index >= int(m_pages.size()))
        return;
    m_tabMenuIndex = index;
    m_tabMenu = std::make_unique<Menu>();
    const std::string path = m_pages[size_t(index)]->document().filePath();
    const size_t move = m_tabMenu->addItem(tr("Move to New Window"), [this] {
        Lingmo::base::EventLoop::current()->post([this, index = m_tabMenuIndex, alive = std::weak_ptr<bool>(m_alive)] {
            if (!alive.expired())
                moveToNewWindow(index);
        });
    });
    m_tabMenu->setItemEnabled(move, m_pages.size() > 1);
    if (!path.empty()) {
        EditorApplication &application = m_application;
        m_tabMenu->addItem(tr("Copy Path"), [&application, path] { application.copyText(path); });
        m_tabMenu->addItem(tr("Show in Folder"), [&application, path] { application.showInFileManager(path); });
    }
    m_tabMenu->addSeparator();
    m_tabMenu->addItem(tr("Close Tab"), [this] { closeTab(m_tabMenuIndex); }, "Ctrl+W");
    if (m_pages.size() > 1)
        m_tabMenu->addItem(tr("Close Other Tabs"), [this] { closeOtherTabs(m_tabMenuIndex); });
    if (index < int(m_pages.size()) - 1)
        m_tabMenu->addItem(tr("Close Tabs to the Right"), [this] { closeTabsToRight(m_tabMenuIndex); });
    m_tabMenu->popup(m_window.get(), position);
}

void EditorWindow::detachTab(int index, PointF position)
{
    if (index < 0 || index >= int(m_pages.size()) || m_pages.size() < 2) {
        m_header->tabBar->cancelDetach();
        return;
    }
    std::unique_ptr<EditorPage> page = takePage(index);
    Options options;
    options.detachedPage = std::move(page);
    options.size = m_window->size();
    options.zoom = m_zoom;
    options.restoresSize = false;
    EditorWindow *window = m_application.newWindow(std::move(options));
    DragSource source;
    source.mimeTypes = {tabMimeType};
    source.data = [](const std::string &) { return std::string(); };
    source.actions = MoveAction;
    source.attachedWindow = &window->window();
    source.attachedOffset = {std::min(position.x, window->window().size().width - 1), headerHeight / 2};
    source.context = window;
    source.onFinished = [this, window, index, alive = std::weak_ptr<bool>(m_alive)](DropAction action) {
        if (alive.expired())
            return;
        const std::vector<EditorWindow *> windows = m_application.windows();
        if (std::find(windows.begin(), windows.end(), window) == windows.end())
            return;
        // Let go on nothing: the tab goes back where it was.
        if (action == NoAction && window->tabCount() == 1)
            insertTab(window->takeDetachedPage(), index);
        if (window->tabCount() == 0) {
            window->m_closeConfirmed = true;
            window->finishClose();
        } else {
            window->window().activate();
        }
    };
    if (!m_header->tabBar->startDrag(std::move(source)))
        window->window().activate();
}

std::unique_ptr<EditorPage> EditorWindow::takeDetachedPage()
{
    std::unique_ptr<EditorPage> page = takePage(0);
    if (m_pages.empty())
        m_window->hide();
    return page;
}

bool EditorWindow::isFullScreen() const
{
    return m_window->states().fullScreen;
}

void EditorWindow::toggleFullScreen()
{
    m_window->setFullScreen(!isFullScreen());
}

void EditorWindow::applySettings()
{
    for (EditorPage *page : m_pages)
        page->applySettings();
}

void EditorWindow::addShortcuts()
{
    Window &window = *m_window;
    const auto page = [this](std::function<void(EditorPage &)> action) {
        return [this, action] {
            if (EditorPage *current = currentPage())
                action(*current);
        };
    };
    const uint32_t ctrl = ControlModifier;
    const uint32_t shift = ShiftModifier;
    const uint32_t alt = AltModifier;
    window.addShortcut(ctrl, key::letter('t'), [this] { newTab(); });
    window.addShortcut(ctrl, key::letter('n'), [this] { m_application.newWindow(); });
    window.addShortcut(ctrl, key::letter('o'), [this] { openDialog(); });
    window.addShortcut(ctrl, key::letter('s'), [this] { savePage(currentPage()); });
    window.addShortcut(ctrl | shift, key::letter('s'), [this] { saveAs(currentPage()); });
    window.addShortcut(ctrl | alt, key::letter('s'), [this] { saveAll(); });
    window.addShortcut(ctrl, key::letter('w'), [this] { closeCurrentTab(); });
    window.addShortcut(ctrl | shift, key::letter('w'), [this] { requestClose(); });
    window.addShortcut(ctrl, key::letter('q'), [this] { quitApplication(); });
    window.addShortcut(ctrl | shift, key::letter('t'), [this] { reopenClosedTab(); });
    window.addShortcut(ctrl, key::PageDown, [this] { cycleTab(1); });
    window.addShortcut(ctrl, key::Tab, [this] { cycleTab(1); });
    window.addShortcut(ctrl, key::PageUp, [this] { cycleTab(-1); });
    window.addShortcut(ctrl | shift, key::Tab, [this] { cycleTab(-1); });
    window.addShortcut(ctrl | shift, key::PageDown, [this] { moveTab(m_currentIndex, m_currentIndex + 1); });
    window.addShortcut(ctrl | shift, key::PageUp, [this] { moveTab(m_currentIndex, m_currentIndex - 1); });
    for (int i = 0; i < 9; ++i) {
        window.addShortcut(alt, uint32_t('1' + i), [this, i] { selectTab(i == 8 ? int(m_pages.size()) - 1 : i); });
    }
    window.addShortcut(ctrl, key::letter('f'), page([](EditorPage &p) { p.showFind(false); }));
    window.addShortcut(ctrl, key::letter('h'), page([](EditorPage &p) { p.showFind(true); }));
    const uint32_t f3 = key::F1 + 2;
    const uint32_t f11 = key::F1 + 10;
    window.addShortcut(NoModifier, f3, page([](EditorPage &p) { p.findNext(false); }));
    window.addShortcut(ctrl, key::letter('g'), page([](EditorPage &p) { p.findNext(false); }));
    window.addShortcut(shift, f3, page([](EditorPage &p) { p.findNext(true); }));
    window.addShortcut(ctrl | shift, key::letter('g'), page([](EditorPage &p) { p.findNext(true); }));
    window.addShortcut(ctrl, key::letter('i'), page([](EditorPage &p) { p.showGoToLine(); }));
    window.addShortcut(ctrl, key::letter('l'), page([](EditorPage &p) { p.showGoToLine(); }));
    // Ctrl+=, Ctrl++ (shifted on most layouts, or the keypad's) and Ctrl+-.
    window.addShortcut(ctrl, '=', [this] { zoomBy(1); });
    window.addShortcut(ctrl, '+', [this] { zoomBy(1); });
    window.addShortcut(ctrl | shift, '+', [this] { zoomBy(1); });
    window.addShortcut(ctrl, 0xffab, [this] { zoomBy(1); });
    window.addShortcut(ctrl, '-', [this] { zoomBy(-1); });
    window.addShortcut(ctrl, 0xffad, [this] { zoomBy(-1); });
    window.addShortcut(ctrl, '0', [this] { zoomBy(0); });
    window.addShortcut(alt, key::letter('w'), [this] {
        EditorSettings &settings = m_application.settings();
        settings.wordWrap = !settings.wordWrap;
        settings.commit();
    });
    window.addShortcut(NoModifier, f11, [this] { toggleFullScreen(); });
    window.addShortcut(ctrl, ',', [this] { showPreferences(); });
    window.addShortcut(ctrl | shift, '?', [this] { showShortcuts(); });
    window.addShortcut(ctrl | shift, '/', [this] { showShortcuts(); });
    window.addShortcut(ctrl, key::letter('d'), page([](EditorPage &p) { p.document().deleteLines(); }));
    window.addShortcut(ctrl | alt, key::letter('d'), page([](EditorPage &p) { p.document().duplicateLines(); }));
    window.addShortcut(ctrl, key::letter('u'), page([](EditorPage &p) { p.document().changeCase(Document::Upper); }));
    window.addShortcut(ctrl | shift, key::letter('u'), page([](EditorPage &p) { p.document().changeCase(Document::Lower); }));
}
