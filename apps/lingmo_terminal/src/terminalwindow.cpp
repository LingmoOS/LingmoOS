// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "terminalwindow.h"

#include <algorithm>
#include <cmath>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/strings.h"
#include "ui/animation.h"
#include "ui/application.h"
#include "ui/dialog_window.h"
#include "ui/drag.h"
#include "ui/header_button.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/view.h"
#include "ui/window.h"
#include "ui/window_header.h"
#include "ui/window_tab_bar.h"

#include "TerminalSession.h"
#include "TerminalView.h"

#include "processhelper.h"
#include "settingsdialog.h"
#include "terminalpage.h"
#include "terminalsettings.h"
#include "windows.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
namespace strings = Lingmo::base::strings;

static constexpr double kHeaderHeight = 40;
// What a tab dragged between windows of this process carries.
static constexpr const char* kTabMimeType = "application/x-lingmo-terminal-tab";
// The "+" button, between the tabs and the window buttons.
static constexpr double kNewTabButtonSize = 32;

// The tabs' pages, the current one shown under the header.
class TerminalWindow::Content : public View
{
public:
    explicit Content(TerminalWindow* owner)
        : m_owner(owner)
    {
        setName("content");
        header = addChild<WindowHeader>(kHeaderHeight);
        header->setButtonMargin(4);
        header->setButtonsCentered(true);
        pages = addChild<View>();
        pages->setName("pages");
    }

    WindowHeader* header;
    View* pages;

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        header->setGeometry({0, 0, width, kHeaderHeight});
        pages->setGeometry({0, kHeaderHeight, width, std::max(0.0, height - kHeaderHeight)});
        for (const auto& page : pages->children())
            page->setGeometry(pages->localRect());
        m_owner->updateBackground();
    }

private:
    TerminalWindow* m_owner;
};

// The title with one tab, the tab bar with more, and the "+" button.
class TerminalWindow::HeaderContent : public View
{
public:
    explicit HeaderContent(TerminalWindow* owner)
        : m_owner(owner)
        , title(addChild<Label>())
        , tabBar(addChild<WindowTabBar>())
        , newTab(addChild<HeaderButton>("add", kNewTabButtonSize))
    {
        setName("headerContent");
        title->setName("title");
        title->setFontSize(13);
        title->setWeight(500);
        title->setElide(Label::Elide::Middle);
        tabBar->setName("tabBar");
        newTab->setName("newTab");
    }

    Label* title;
    WindowTabBar* tabBar;
    HeaderButton* newTab;

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        const Window* window = this->window();
        const double windowWidth = window ? window->size().width : width;

        // Right next to the window buttons, with the window button spacing between.
        const double newTabX = width - kNewTabButtonSize;
        newTab->setGeometry({newTabX, rounding::centeredStart(height, kNewTabButtonSize), kNewTabButtonSize,
                             kNewTabButtonSize});

        // Width taken by the window buttons, mirrored on the left so the
        // title is centred on the window rather than on this item.
        const double reservedWidth = windowWidth - width + kNewTabButtonSize;
        const SizeF titleSize = title->sizeHint();
        const double titleWidth = std::max(0.0, std::min(titleSize.width, windowWidth - reservedWidth * 2));
        title->setGeometry({(windowWidth - titleWidth) / 2, rounding::centeredStart(height, titleSize.height),
                            titleWidth, titleSize.height});

        const double tabLeft = Theme::current().smallSpacing;
        const double tabRight = newTabX - 4;
        const double tabHeight = tabBar->sizeHint().height;
        tabBar->setGeometry({tabLeft, rounding::centeredStart(height, tabHeight), std::max(0.0, tabRight - tabLeft),
                             tabHeight});
    }

    // Takes a tab dragged out of another window of this process; one from
    // another terminal process cannot bring its shell along. While it is over
    // the title bar the tab bar shows it where it would drop.
    bool canDrop(const DropOffer& offer) const override
    {
        return sourceOf(offer) != nullptr;
    }

    void dragEnter(const DragEvent& event) override
    {
        TerminalWindow* source = sourceOf(*event.offer);
        TerminalPage* page = source ? source->currentPage() : nullptr;
        m_owner->m_incoming = true;
        m_owner->updateTitle();
        tabBar->beginIncoming(page ? page->title() : std::string(), event.position.x - tabBar->geometry().x);
    }

    uint32_t dragUpdate(const DragEvent& event) override
    {
        tabBar->setIncomingX(event.position.x - tabBar->geometry().x);
        return MoveAction;
    }

    void dragLeave() override
    {
        tabBar->endIncoming(false);
        m_owner->m_incoming = false;
        m_owner->updateTitle();
    }

    uint32_t drop(const DragEvent& event) override
    {
        TerminalWindow* source = sourceOf(*event.offer);
        const int index = tabBar->incomingIndex();
        tabBar->endIncoming(true);
        m_owner->m_incoming = false;
        if (!source) {
            m_owner->updateTitle();
            return NoAction;
        }
        m_owner->insertTab(source->takePage(), index);
        m_owner->window().activate();
        return MoveAction;
    }

private:
    TerminalWindow* sourceOf(const DropOffer& offer) const
    {
        const DragSource* source = offer.localSource();
        if (!source || !offer.hasMimeType(kTabMimeType) || source->context == m_owner)
            return nullptr;
        return static_cast<TerminalWindow*>(const_cast<void*>(source->context));
    }

    TerminalWindow* m_owner;
};

// Over the content: the zoom indicator and the visual bell.
class TerminalWindow::Overlay : public View
{
public:
    explicit Overlay(TerminalWindow* owner)
        : m_owner(owner)
        , m_zoomLabel(addChild<Label>())
    {
        setName("overlay");
        m_zoomLabel->setFontSize(13);
        m_zoomLabel->setWeight(500);
        m_zoomLabel->setVisible(false);

        m_zoomHide.setSingleShot(true);
        m_zoomHide.setInterval(std::chrono::milliseconds(900));
        m_zoomHide.onTimeout = [this] { startZoomFade(); };
    }

    bool acceptsPointer() const override { return false; }

    // Shows the size briefly while zooming.
    void showZoom(int pointSize)
    {
        m_zoomLabel->setText(format(tr("{size} pt"), {{"size", std::to_string(pointSize)}}));
        if (m_zoomFade)
            m_zoomFade->stop();
        m_zoomOpacity = 1;
        m_zoomLabel->setVisible(true);
        m_zoomLabel->setOpacity(1);
        m_zoomHide.start();
        requestLayout();
        requestPaint();
    }

    // The whole window, title bar included, briefly lightens, or darkens on a light scheme.
    void flash()
    {
        if (!window())
            return;
        if (!m_bellFade) {
            m_bellFade = std::make_unique<Animation>(window());
            m_bellFade->setDuration(std::chrono::milliseconds(180));
            m_bellFade->setEasing(Animation::Easing::Linear);
            m_bellFade->onUpdate = [this](double value) {
                m_bellOpacity = 0.12 * (1 - value);
                requestPaint();
            };
        }
        m_bellFade->stop();
        m_bellOpacity = 0.12;
        m_bellFade->start();
        requestPaint();
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const SizeF labelSize = m_zoomLabel->sizeHint();
        const double width = labelSize.width + theme.largeSpacing * 2;
        const double height = 36;
        // Centred on the window's content, below the header; the bell covers the header too.
        const double contentHeight = std::max(0.0, geometry().height - kHeaderHeight);
        m_zoomRect = {rounding::centeredStart(geometry().width, width),
                      kHeaderHeight + rounding::centeredStart(contentHeight, height), width, height};
        m_zoomLabel->setGeometry({m_zoomRect.x + rounding::centeredStart(width, labelSize.width),
                                  m_zoomRect.y + rounding::centeredStart(height, labelSize.height), labelSize.width,
                                  labelSize.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        if (m_zoomOpacity > 0) {
            painter.save();
            painter.multiplyOpacity(m_zoomOpacity);
            painter.drawSmoothRectangle(m_zoomRect, m_zoomRect.height / 2, theme.secondBackground, 1,
                                       theme.text.withAlpha(0.1));
            painter.restore();
        }
        if (m_bellOpacity > 0)
            painter.fillRect(localRect(), m_owner->chromeForeground().withAlpha(m_bellOpacity));
    }

private:
    void startZoomFade()
    {
        if (!window())
            return;
        if (!m_zoomFade) {
            m_zoomFade = std::make_unique<Animation>(window());
            m_zoomFade->setDuration(std::chrono::milliseconds(200));
            m_zoomFade->setEasing(Animation::Easing::Linear);
            m_zoomFade->onUpdate = [this](double value) {
                m_zoomOpacity = 1 - value;
                m_zoomLabel->setOpacity(m_zoomOpacity);
                m_zoomLabel->setVisible(m_zoomOpacity > 0);
                requestPaint();
            };
        }
        m_zoomFade->start();
    }

    TerminalWindow* m_owner;
    Label* m_zoomLabel;
    RectF m_zoomRect;
    double m_zoomOpacity = 0;
    double m_bellOpacity = 0;
    Lingmo::base::Timer m_zoomHide;
    std::unique_ptr<Animation> m_zoomFade;
    std::unique_ptr<Animation> m_bellFade;
};

TerminalWindow::TerminalWindow(Windows& windows, TerminalSettings& settings, Options options)
    : m_windows(windows)
    , m_settings(settings)
    , m_window(std::make_unique<Window>())
    , m_fontZoom(options.fontZoom)
{
    m_window->setMinimumSize({400, 300});
    m_window->resize(options.size.width > 0 ? options.size : SizeF{760, 450});
    if (options.restoresSize)
        m_window->setRestoresSize("main");

    auto content = std::make_unique<Content>(this);
    m_content = content.get();
    m_header = m_content->header;
    m_headerContent = m_header->setContent<HeaderContent>(this);
    m_window->setContentView(std::move(content));
    m_overlay = m_window->overlay()->addChild<Overlay>(this);

    m_headerContent->newTab->onClicked = [this] { openNewTab(); };
    WindowTabBar* tabBar = m_headerContent->tabBar;
    tabBar->onCurrentChanged = [this](int index) { selectTab(index); };
    tabBar->onCloseRequested = [this](int index) { closeProtection(index); };
    tabBar->onMoveRequested = [this](int from, int to) { moveTab(from, to); };
    tabBar->onContextMenuRequested = [this](int index, PointF position) { openTabMenu(index, position); };
    tabBar->onDetachRequested = [this](int index, PointF position) {
        // Later, not from inside the tab bar's own handler: taking the tab out
        // changes the tabs it is running over.
        std::weak_ptr<bool> alive = m_alive;
        Lingmo::base::EventLoop::current()->post([this, alive, index, position] {
            if (alive.lock())
                detachTab(index, position);
        });
    };
    tabBar->onRenameRequested = [this](int index, const std::string& title) {
        if (index < 0 || index >= tabCount())
            return;
        m_pages[size_t(index)]->setCustomTitle(std::string(strings::trimmed(title)));
        if (TerminalPage* page = currentPage())
            page->focusTerminal();
    };

    m_window->onCloseRequested = [this] { close(); };
    m_window->onActiveChanged = [this] { updateTitle(); };
    m_window->onStatesChanged = [this] { applySettings(); };

    m_connections.push_back(m_settings.changed.connect([this] { applySettings(); }));
    m_connections.push_back(Application::instance()->languageChanged.connect([this] { retranslate(); }));
    if (auto* bus = Application::instance()->sessionBus())
        m_globalMenu = std::make_unique<GlobalMenu>(bus, [this] { return menuEntries(); });
    applySettings();

    if (options.detachedPage) {
        insertTab(std::move(options.detachedPage), 0);
        return;
    }

    if (!options.startDirectory.empty()) {
        openTab(options.startDirectory, options.command);
        return;
    }

    openTab("$PWD", options.command);
}

TerminalWindow::~TerminalWindow()
{
    m_connections.clear();
    m_pageConnections.clear();
    *m_alive = false;
    // The shells end with the window.
    while (!m_pages.empty()) {
        TerminalPage* page = m_pages.back();
        m_pages.pop_back();
        m_content->pages->takeChild(page).reset();
    }
}

Color TerminalWindow::schemeColor(int index, Color fallback) const
{
    if (index < 0 || index >= int(m_schemeColors.size()))
        return fallback;
    return m_schemeColors[size_t(index)];
}

int TerminalWindow::fontPointSize() const
{
    return std::clamp(m_settings.fontPointSize() + m_fontZoom, 5, 40);
}

void TerminalWindow::zoom(int step)
{
    const int size = step == 0 ? m_settings.fontPointSize() : fontPointSize() + step;
    m_fontZoom = std::clamp(size, 5, 40) - m_settings.fontPointSize();
    applySettings();
    m_overlay->showZoom(fontPointSize());
}

void TerminalWindow::applySettings()
{
    m_colorScheme = m_settings.colorScheme();
    const Konsole::ColorSchemeInfo info = Konsole::TerminalView::colorSchemeInfo(m_colorScheme);
    m_terminalBackground = info.valid ? info.background : Color::rgb(0x1A1B26);
    m_chromeForeground = info.valid ? info.foreground : Theme::current().text;
    m_schemeColors.assign(info.colors.begin(), info.colors.end());
    if (!info.valid)
        m_schemeColors.clear();
    // Dark when 0.2126 r + 0.7152 g + 0.0722 b < 0.5, channels from 0 to 1.
    m_chromeDark = (0.2126 * m_terminalBackground.r + 0.7152 * m_terminalBackground.g
                    + 0.0722 * m_terminalBackground.b) / 255.0 < 0.5;

    m_window->setHeaderDarkMode(m_chromeDark);
    updateBackground();

    m_headerContent->title->setFontFamily(m_settings.fontName());
    m_headerContent->tabBar->setDarkMode(m_chromeDark);
    m_headerContent->tabBar->setTextColor(m_chromeForeground);
    for (TerminalPage* page : m_pages)
        page->applySettings();
    updateTitle();
}

// The shells, their output and the tabs stay; menus close, and the dialogs are
// made again, an open one shown again with what it was asking about.
void TerminalWindow::retranslate()
{
    if (m_tabMenu)
        m_tabMenu->close();
    m_tabMenu.reset();
    for (TerminalPage* page : m_pages)
        page->retranslate();
    if (m_settingsDialog)
        m_settingsDialog->retranslate();

    const bool exitShown = m_exitPrompt && m_exitPrompt->window();
    const bool linkShown = m_linkConfirm && m_linkConfirm->window();
    const bool pasteShown = m_pasteConfirm && m_pasteConfirm->window();
    m_exitPrompt.reset();
    m_linkConfirm.reset();
    m_pasteConfirm.reset();
    if (exitShown)
        showExitPrompt();
    if (linkShown)
        showLinkConfirm();
    if (pasteShown)
        showPasteConfirm();
}

void TerminalWindow::updateBackground()
{
    m_window->setBackgroundColor(m_terminalBackground.withAlpha(m_settings.opacity()));
    const SizeF size = m_window->size();
    if (m_settings.blur())
        m_window->setBackgroundBlur({0, 0, size.width, size.height}, Theme::current().windowRadius);
    else
        m_window->setBackgroundBlur({}, 0);
}

void TerminalWindow::updateTitle()
{
    TerminalPage* page = currentPage();
    const std::string title = page ? page->title() : std::string();
    m_window->setTitle(title);
    m_headerContent->title->setText(title);
    m_headerContent->title->setColor(m_window->isActive() ? m_chromeForeground : m_chromeForeground.withAlpha(0.5));

    // A tab being dragged out or in keeps the bar up even with one tab, so
    // it can be dragged back.
    WindowTabBar* tabBar = m_headerContent->tabBar;
    const bool showTabs = tabCount() > 1 || m_incoming || m_detaching;
    tabBar->setVisible(showTabs);
    m_headerContent->title->setVisible(!showTabs);
    for (int i = 0; i < tabCount() && i < tabBar->count(); ++i) {
        const std::string tabTitle = m_pages[size_t(i)]->title();
        if (tabBar->tabTitle(i) != tabTitle)
            tabBar->setTabTitle(i, tabTitle);
    }
    if (m_currentIndex >= 0 && tabBar->currentIndex() != m_currentIndex)
        tabBar->setCurrentIndex(m_currentIndex);
    m_headerContent->requestLayout();
    if (m_globalMenu)
        m_globalMenu->update();
}

std::vector<GlobalMenu::Entry> TerminalWindow::menuEntries() const
{
    using Entry = GlobalMenu::Entry;
    TerminalPage* page = currentPage();
    auto action = [](std::string label, std::function<void()> callback) {
        Entry entry; entry.label = std::move(label); entry.action = std::move(callback); return entry;
    };
    Entry file; file.label = tr("File");
    file.children = {action(tr("New Tab"), [this] { const_cast<TerminalWindow*>(this)->openNewTab(); }),
                     action(tr("New Window"), [this] { const_cast<TerminalWindow*>(this)->newWindow(); }),
                     Entry {.separator = true}, action(tr("Close Window"), [this] { const_cast<TerminalWindow*>(this)->close(); })};
    Entry edit; edit.label = tr("Edit");
    if (page) {
        edit.children = {action(tr("Copy"), [page] { page->terminal()->copyClipboard(); }),
                         action(tr("Paste"), [page] { page->terminal()->pasteClipboard(); }),
                         action(tr("Select All"), [page] { page->terminal()->selectAll(); }),
                         Entry {.separator = true}, action(tr("Find…"), [page] { page->openFind(); }),
                         action(tr("Clear Scrollback"), [page] { page->session().clearScrollback(); })};
    }
    Entry view; view.label = tr("View");
    view.children = {action(isFullScreen() ? tr("Exit Full Screen") : tr("Full Screen"), [this] { const_cast<TerminalWindow*>(this)->toggleFullScreen(); }),
                     action(tr("Settings"), [this] { const_cast<TerminalWindow*>(this)->showSettings(); })};
    return {std::move(file), std::move(edit), std::move(view)};
}

TerminalPage* TerminalWindow::currentPage() const
{
    if (m_currentIndex < 0 || m_currentIndex >= tabCount())
        return nullptr;
    return m_pages[size_t(m_currentIndex)];
}

int TerminalWindow::indexOfPage(const TerminalPage* page) const
{
    const auto found = std::find(m_pages.begin(), m_pages.end(), page);
    return found == m_pages.end() ? -1 : int(found - m_pages.begin());
}

void TerminalWindow::attach(TerminalPage* page)
{
    page->setTerminalWindow(this);
    // Looked up when the shell exits: closing other tabs shifts the index,
    // and a tab dragged into another window closes there.
    std::vector<Lingmo::base::Subscription>& connections = m_pageConnections[page];
    connections.push_back(page->terminalClosed.connect([this, page] {
        const int index = indexOfPage(page);
        if (index >= 0)
            Lingmo::base::EventLoop::current()->post([alive = std::weak_ptr<bool>(m_alive), this, page] {
                if (alive.lock() && indexOfPage(page) >= 0)
                    closeTab(indexOfPage(page));
            });
    }));
    connections.push_back(page->titleChanged.connect([this] { updateTitle(); }));
}

void TerminalWindow::showPage(int index)
{
    m_currentIndex = index;
    for (int i = 0; i < tabCount(); ++i)
        m_pages[size_t(i)]->setVisible(i == index);
    if (TerminalPage* page = currentPage())
        page->focusTerminal();
    updateTitle();
}

void TerminalWindow::openNewTab()
{
    if (TerminalPage* page = currentPage())
        openTab(page->session().currentDir(), {}, m_currentIndex + 1);
    else
        openTab("$HOME");
}

void TerminalWindow::openTab(const std::string& path, const std::vector<std::string>& command, int index)
{
    auto page = std::make_unique<TerminalPage>(this, path, command);
    TerminalPage* created = page.get();
    insertTab(std::move(page), index < 0 ? tabCount() : index);
    created->startShell();
}

void TerminalWindow::insertTab(std::unique_ptr<TerminalPage> page, int index)
{
    TerminalPage* raw = page.get();
    index = std::clamp(index, 0, tabCount());
    m_content->pages->addChild(std::move(page));
    m_pages.insert(m_pages.begin() + index, raw);
    m_headerContent->tabBar->insertTab(index, raw->title());
    attach(raw);
    m_content->requestLayout();
    showPage(index);
}

std::unique_ptr<TerminalPage> TerminalWindow::takeTab(int index)
{
    if (index < 0 || index >= tabCount())
        return nullptr;
    TerminalPage* page = m_pages[size_t(index)];
    m_pages.erase(m_pages.begin() + index);
    m_pageConnections.erase(page);
    m_headerContent->tabBar->removeTab(index);
    std::unique_ptr<View> view = m_content->pages->takeChild(page);
    if (tabCount() > 0)
        showPage(std::min(index, tabCount() - 1));
    else
        m_currentIndex = -1;
    updateTitle();
    return std::unique_ptr<TerminalPage>(static_cast<TerminalPage*>(view.release()));
}

std::unique_ptr<TerminalPage> TerminalWindow::takePage()
{
    std::unique_ptr<TerminalPage> page = takeTab(0);
    m_window->hide();
    return page;
}

// Pulls a tab out into a window of its own, which follows the pointer until
// it is dropped. Escape puts the tab back. `position` is where the pointer is
// in this window.
void TerminalWindow::detachTab(int index, PointF position)
{
    if (index < 0 || index >= tabCount() || tabCount() < 2) {
        m_headerContent->tabBar->cancelDetach();
        return;
    }

    m_detaching = true;
    Options options;
    options.detachedPage = takeTab(index);
    options.size = m_window->size();
    options.fontZoom = m_fontZoom;
    options.restoresSize = false;
    TerminalWindow* detached = m_windows.create(std::move(options));

    // The pointer stays over the new window's title bar, as far across as it
    // is over this one.
    DragSource source;
    source.mimeTypes = {kTabMimeType};
    source.data = [](const std::string&) { return std::string(); };
    source.actions = MoveAction;
    source.attachedWindow = &detached->window();
    source.attachedOffset = {std::min(position.x, detached->window().size().width - 1), kHeaderHeight / 2};
    source.context = detached;

    std::weak_ptr<bool> alive = m_alive;
    std::weak_ptr<bool> detachedAlive = detached->m_alive;
    // Escape puts the tab back; let go anywhere, it stays in its window.
    source.onEnded = [this, alive, detached, detachedAlive, index](DropAction, bool dropped) {
        if (alive.lock()) {
            m_detaching = false;
            updateTitle();
        }
        if (!detachedAlive.lock())
            return;
        if (!dropped && detached->tabCount() == 1 && alive.lock())
            insertTab(detached->takePage(), index);
        if (detached->tabCount() == 0)
            m_windows.release(detached);
        else
            detached->window().activate();
    };

    if (!m_headerContent->tabBar->startDrag(std::move(source))) {
        m_detaching = false;
        insertTab(detached->takePage(), index);
        m_windows.release(detached);
    }
    updateTitle();
}

// Rename Tab…, Close Tab, Close Other Tabs, at the pointer.
void TerminalWindow::openTabMenu(int index, PointF position)
{
    if (index < 0 || index >= tabCount())
        return;
    m_tabMenuIndex = index;
    if (!m_tabMenu)
        m_tabMenu = std::make_unique<Menu>();
    m_tabMenu->clear();
    m_tabMenu->addItem(tr("Rename Tab…"), [this] { m_headerContent->tabBar->startRename(m_tabMenuIndex); });
    m_tabMenu->addSeparator();
    m_tabMenu->addItem(tr("Close Tab"), [this] { closeProtection(m_tabMenuIndex); });
    if (tabCount() > 1)
        m_tabMenu->addItem(tr("Close Other Tabs"), [this] { closeOtherTabs(m_tabMenuIndex); });
    m_tabMenu->popup(m_window.get(), position);
}

void TerminalWindow::closeTab(int index)
{
    if (index < 0 || index >= tabCount())
        return;

    takeTab(index).reset();

    if (tabCount() == 0) {
        m_quitting = true;
        close();
    }
}

// Asks before running action when a process is still running in one of pages.
void TerminalWindow::confirmClose(const std::vector<TerminalPage*>& pages, std::function<void()> action)
{
    const bool active = std::any_of(pages.begin(), pages.end(),
                                    [](TerminalPage* page) { return page->session().hasActiveProcess(); });
    if (!active) {
        action();
        return;
    }

    m_pendingAction = std::move(action);
    showExitPrompt();
}

void TerminalWindow::showExitPrompt()
{
    if (!m_exitPrompt) {
        m_exitPrompt = std::make_unique<ConfirmDialog>(m_window.get(), tr("Process is running, are you sure you want to quit?"));
        m_exitPrompt->setCancelText(tr("Cancel"));
        m_exitPrompt->setConfirmText(tr("OK"));
        m_exitPrompt->setDestructive(true);
        m_exitPrompt->onAccepted = [this] {
            if (auto pending = std::exchange(m_pendingAction, nullptr))
                pending();
        };
        m_exitPrompt->onRejected = [this] { m_pendingAction = nullptr; };
    }
    m_exitPrompt->show();
}

void TerminalWindow::closeProtection(int index)
{
    if (index < 0 || index >= tabCount())
        return;
    TerminalPage* page = m_pages[size_t(index)];
    confirmClose({page}, [this, page] { closeTab(indexOfPage(page)); });
}

void TerminalWindow::closeCurrentTab()
{
    closeProtection(m_currentIndex);
}

void TerminalWindow::closeOtherTabs(int index)
{
    if (index < 0 || index >= tabCount())
        return;
    TerminalPage* keep = m_pages[size_t(index)];
    std::vector<TerminalPage*> others;
    for (TerminalPage* page : m_pages) {
        if (page != keep)
            others.push_back(page);
    }
    confirmClose(others, [this, keep, others] {
        // The kept tab's shell may have ended while the dialog was open; the
        // other tabs then stay rather than leave the window empty.
        if (indexOfPage(keep) < 0)
            return;
        for (TerminalPage* page : others)
            closeTab(indexOfPage(page));
        showPage(indexOfPage(keep));
    });
}

void TerminalWindow::selectTab(int index)
{
    if (index < 0 || index >= tabCount())
        return;
    showPage(index);
}

// Next (step 1) or previous (step -1) tab, wrapping around.
void TerminalWindow::cycleTab(int step)
{
    const int count = tabCount();
    if (count > 1)
        selectTab((m_currentIndex + step + count) % count);
}

// Moves the current tab by step, keeping it current.
void TerminalWindow::moveCurrentTab(int step)
{
    moveTab(m_currentIndex, m_currentIndex + step);
}

void TerminalWindow::moveTab(int from, int to)
{
    if (from == to || from < 0 || to < 0 || from >= tabCount() || to >= tabCount())
        return;
    TerminalPage* page = m_pages[size_t(from)];
    m_pages.erase(m_pages.begin() + from);
    m_pages.insert(m_pages.begin() + to, page);
    m_headerContent->tabBar->moveTab(from, to);
    showPage(indexOfPage(page));
}

// In this process, so every window shares one settings object.
void TerminalWindow::newWindow()
{
    TerminalPage* page = currentPage();
    const std::string directory = page ? page->session().currentDir() : std::string();
    Options options;
    options.startDirectory = directory.empty() ? "$HOME" : directory;
    m_windows.create(std::move(options));
}

void TerminalWindow::close()
{
    if (!m_quitting) {
        if (std::any_of(m_pages.begin(), m_pages.end(),
                        [](TerminalPage* page) { return page->session().hasActiveProcess(); })) {
            confirmClose(m_pages, [this] {
                m_quitting = true;
                close();
            });
            return;
        }
    }
    finishClose();
}

void TerminalWindow::finishClose()
{
    // The shells end with the window, and the application with the last window.
    while (!m_pages.empty())
        takeTab(tabCount() - 1).reset();
    if (m_settingsDialog)
        m_settingsDialog->close();
    m_windows.release(this);
}

bool TerminalWindow::isFullScreen() const
{
    return m_window->states().fullScreen;
}

void TerminalWindow::toggleFullScreen()
{
    m_window->setFullScreen(!isFullScreen());
}

void TerminalWindow::flashBell()
{
    m_overlay->flash();
}

void TerminalWindow::showSettings()
{
    if (!m_settingsDialog)
        m_settingsDialog = std::make_unique<SettingsDialog>(this);
    m_settingsDialog->show();
}

// A link in the output can name any scheme, and opening one starts whatever
// program claims it; only web and mail links open without asking.
void TerminalWindow::openLink(const std::string& url)
{
    const std::string lower = strings::toAsciiLower(url);
    if (lower.starts_with("http:") || lower.starts_with("https:") || lower.starts_with("mailto:")) {
        ProcessHelper::openUrl(url);
        return;
    }

    m_pendingLink = url;
    showLinkConfirm();
}

void TerminalWindow::showLinkConfirm()
{
    if (!m_linkConfirm) {
        m_linkConfirm = std::make_unique<ConfirmDialog>(m_window.get(), tr("Open this link?"));
        m_linkConfirm->setConfirmText(tr("Open"));
        m_linkConfirm->onAccepted = [this] { ProcessHelper::openUrl(m_pendingLink); };
    }
    const std::u16string utf16 = strings::utf8ToUtf16(m_pendingLink);
    m_linkConfirm->setText(utf16.size() > 300 ? strings::utf16ToUtf8(utf16.substr(0, 299)) + "\xe2\x80\xa6"
                                              : m_pendingLink);
    m_linkConfirm->show();
}

// Line breaks in a paste run as if Enter were pressed, so a paste with any
// waits here first.
void TerminalWindow::confirmPaste(const std::string& text, std::function<void()> action)
{
    std::string trimmed = text;
    while (!trimmed.empty() && trimmed.back() == '\n')
        trimmed.pop_back();
    const std::vector<std::string> lines = strings::split(trimmed, '\n');
    std::vector<std::string> shown;
    for (size_t i = 0; i < lines.size() && i < 4; ++i) {
        const std::u16string line = strings::utf8ToUtf16(lines[i]);
        shown.push_back(line.size() > 60 ? strings::utf16ToUtf8(line.substr(0, 59)) + "\xe2\x80\xa6" : lines[i]);
    }
    if (lines.size() > 4)
        shown.push_back("\xe2\x80\xa6");

    m_pasteLineCount = lines.size();
    m_pastePreview = strings::join(shown, "\n");
    m_pendingPaste = std::move(action);
    showPasteConfirm();
}

void TerminalWindow::showPasteConfirm()
{
    if (!m_pasteConfirm) {
        m_pasteConfirm = std::make_unique<ConfirmDialog>(m_window.get(), std::string());
        m_pasteConfirm->setConfirmText(tr("Paste"));
        m_pasteConfirm->onAccepted = [this] {
            if (auto pending = std::exchange(m_pendingPaste, nullptr))
                pending();
        };
        m_pasteConfirm->onRejected = [this] { m_pendingPaste = nullptr; };
    }

    m_pasteConfirm->setTitle(m_pasteLineCount > 1 ? trn("Paste {n} line?", "Paste {n} lines?", m_pasteLineCount)
                                                  : std::string(tr("Paste this line and press Enter?")));
    m_pasteConfirm->setText(m_pastePreview);
    m_pasteConfirm->show();
}
