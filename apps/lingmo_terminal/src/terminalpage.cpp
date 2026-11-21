// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "terminalpage.h"

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "base/strings.h"
#include "base/url.h"
#include "ui/clipboard.h"
#include "ui/drag.h"
#include "ui/keys.h"
#include "ui/menu.h"
#include "ui/scroll_bar.h"
#include "ui/theme.h"
#include "ui/window.h"

#include "TerminalSession.h"
#include "TerminalView.h"
#include "Vt102Emulation.h"

#include "findbar.h"
#include "processhelper.h"
#include "terminalsettings.h"
#include "terminalwindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace strings = Lingmo::base::strings;

// The terminal's margins inside the page; the top is the header's line.
static constexpr double kTerminalMargin = 6;

TerminalPage::TerminalPage(TerminalWindow* window, const std::string& path, const std::vector<std::string>& command)
    : m_window(window)
    , m_command(command)
    , m_session(std::make_unique<Konsole::TerminalSession>())
    , m_terminal(addChild<Konsole::TerminalView>())
    , m_scrollBar(addChild<ScrollBar>(ScrollBar::Orientation::Vertical))
{
    setName("terminalPage");
    m_session->setInitialWorkingDirectory(path);
    if (!command.empty()) {
        m_session->setShellProgram(command.front());
        m_session->setShellProgramArguments(std::vector<std::string>(command.begin() + 1, command.end()));
    }

    m_terminal->setFullCursorHeight(true);
    m_terminal->setBackgroundOpacity(0);
    m_terminal->setSession(m_session.get());

    m_terminal->onShortcut = [this](const Lingmo::ui::KeyEvent& event) { return handleShortcut(event); };
    m_terminal->onContextMenu = [this](double x, double y) { openMenu(x, y); };
    m_terminal->onLinkActivated = [this](const std::string& url) { m_window->openLink(url); };

    m_connections.push_back(m_session->finished.connect([this] { terminalClosed(); }));
    m_connections.push_back(m_session->titleChanged.connect([this] { titleChanged(); }));
    m_connections.push_back(m_terminal->notifyBell.connect([this] { m_window->flashBell(); }));
    m_connections.push_back(m_terminal->multilinePasteRequested.connect([this](const std::string& text) {
        // The shell can exit and close the tab while the dialog is open.
        m_window->confirmPaste(text, [this, alive = std::weak_ptr<bool>(m_alive)] {
            if (!alive.expired())
                m_terminal->confirmPaste();
        });
    }));
    m_connections.push_back(m_terminal->scrollbarParamsChanged.connect([this] { updateScrollBar(); }));
    m_connections.push_back(m_terminal->sizeChanged.connect([this] { updateScrollBar(); }));
    m_connections.push_back(m_terminal->scrollbarValueChanged.connect([this] {
        m_scrollActivity.start();
        m_scrollBar->setMoving(true);
    }));

    // Shown while scrolling or under the pointer.
    m_scrollActivity.setSingleShot(true);
    m_scrollActivity.setInterval(std::chrono::milliseconds(800));
    m_scrollActivity.onTimeout = [this] { m_scrollBar->setMoving(false); };
    m_scrollBar->onPositionRequested = [this](double position) {
        m_terminal->setScrollbarValue(int(std::lround(position)) + m_terminal->scrollbarMinimum());
    };

    applySettings();
}

TerminalPage::~TerminalPage()
{
    m_connections.clear();
    // The view goes before the session it shows.
    takeChild(m_terminal).reset();
}

void TerminalPage::setTerminalWindow(TerminalWindow* window)
{
    m_window = window;
    applySettings();
}

void TerminalPage::setCustomTitle(const std::string& title)
{
    m_customTitle = title;
    titleChanged();
}

std::string TerminalPage::title() const
{
    return !m_customTitle.empty() ? m_customTitle : m_session->title();
}

void TerminalPage::applySettings()
{
    const TerminalSettings& settings = m_window->settings();
    m_terminal->setColorScheme(m_window->colorScheme());
    m_terminal->setTerminalFont(settings.fontName(), m_window->fontPointSize());
    m_terminal->setBlinkingCursor(settings.blinkingCursor());
    m_terminal->setTerminalCursorShape(settings.cursorShape());
    // NotifyBell hands the bell to the window; NoBell ignores it.
    m_terminal->setBellMode(settings.visualBell() ? Konsole::TerminalView::NotifyBell : Konsole::TerminalView::NoBell);
    m_terminal->setConfirmMultilinePaste(settings.confirmMultilinePaste());
    m_terminal->setLinkColor(m_window->chromeForeground());
    m_scrollBar->setDarkMode(m_window->chromeDark());
    m_session->setHistorySize(settings.scrollbackLines());
    if (m_findBar)
        m_findBar->setColors(m_window->terminalBackground(), m_window->chromeForeground(), m_window->chromeDark(),
                             m_window->schemeColor(1, accent::red), m_window->schemeColor(4, Theme::current().highlight));
}

void TerminalPage::retranslate()
{
    if (m_menu)
        m_menu->close();
    m_menu.reset();
    if (m_findBar)
        m_findBar->retranslate();
}

void TerminalPage::startShell()
{
    m_session->startShellProgram();
    m_terminal->setFocus();
}

void TerminalPage::focusTerminal()
{
    m_terminal->setFocus();
}

void TerminalPage::layout()
{
    const RectF bounds = localRect();
    m_terminal->setGeometry({kTerminalMargin, 0, std::max(0.0, bounds.width - 2 * kTerminalMargin),
                             std::max(0.0, bounds.height - kTerminalMargin)});
    // Overlays the right margin, the terminal's height.
    m_scrollBar->setGeometry({bounds.width - ScrollBar::thickness, 0, ScrollBar::thickness,
                              std::max(0.0, bounds.height - kTerminalMargin)});
    if (m_findBar) {
        const Theme& theme = Theme::current();
        const SizeF size = m_findBar->sizeHint();
        m_findBar->setGeometry({bounds.width - theme.largeSpacing - size.width, theme.smallSpacing, size.width,
                                size.height});
    }
    updateScrollBar();
}

void TerminalPage::updateScrollBar()
{
    const int lines = m_terminal->lines();
    const int totalLines = lines + m_terminal->scrollbarMaximum() - m_terminal->scrollbarMinimum();
    m_scrollBar->setRange(lines, totalLines, m_terminal->scrollbarValue() - m_terminal->scrollbarMinimum());
}

// Dropped files and links arrive as one shell word each.
static std::string shellQuote(const std::string& text)
{
    const bool plain = !text.empty() && std::all_of(text.begin(), text.end(), [](char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
                || std::string_view("_@%+=:,./-").find(c) != std::string_view::npos;
    });
    if (plain)
        return text;
    return "'" + strings::replaceAll(text, "'", "'\\''") + "'";
}

void TerminalPage::dropUrls(const std::vector<std::string>& urls)
{
    std::vector<std::string> words;
    for (const std::string& url : urls) {
        // A local file as its path, "file://localhost/" too; other links as they are.
        words.push_back(shellQuote(Lingmo::base::url::toLocalFile(url).value_or(url)));
    }
    // As a paste: bracketed for a shell that asks, so control characters in
    // a name are inserted rather than run as editing keys.
    m_terminal->pasteText(strings::join(words, " ") + " ");
}

static bool hasUrls(const DropOffer& offer)
{
    return offer.hasMimeType("text/uri-list");
}

static std::string textType(const DropOffer& offer)
{
    for (const char* type : {"text/plain;charset=utf-8", "text/plain"}) {
        if (offer.hasMimeType(type))
            return type;
    }
    return std::string();
}

// A tab dragged over the terminal is not something to paste.
bool TerminalPage::canDrop(const DropOffer& offer) const
{
    return hasUrls(offer) || !textType(offer).empty();
}

uint32_t TerminalPage::dragUpdate(const DragEvent&)
{
    return CopyAction;
}

uint32_t TerminalPage::drop(const DragEvent& event)
{
    std::weak_ptr<bool> alive = m_alive;
    if (hasUrls(*event.offer)) {
        event.offer->read("text/uri-list", [this, alive](std::string data) {
            if (!alive.lock())
                return;
            // RFC 2483: one URI a line, CRLF ends, # starts a comment.
            std::vector<std::string> urls;
            for (std::string_view line : strings::split(data, '\n')) {
                line = strings::trimmed(line);
                if (!line.empty() && line.front() != '#')
                    urls.emplace_back(line);
            }
            if (!urls.empty())
                dropUrls(urls);
        });
        return CopyAction;
    }

    event.offer->read(textType(*event.offer), [this, alive](std::string data) {
        if (alive.lock())
            m_terminal->pasteText(data);
    });
    return CopyAction;
}

// Starts from the selection when it is a single line, as a search term would be.
void TerminalPage::openFind()
{
    const std::string selection = m_terminal->hasSelection()
            ? std::string(strings::trimmed(m_terminal->selectedText()))
            : std::string();
    if (!m_findBar) {
        m_findBar = addChild<FindBar>(m_terminal);
        m_findBar->onClosed = [this] { m_terminal->setFocus(); };
        applySettings();
        requestLayout();
    }
    m_findBar->open(selection.find('\n') == std::string::npos ? selection : std::string());
}

bool TerminalPage::handleShortcut(const Lingmo::ui::KeyEvent& event)
{
    const uint32_t mods = event.modifiers;
    const bool ctrl = mods & Lingmo::ui::ControlModifier;
    const bool shift = mods & Lingmo::ui::ShiftModifier;
    const bool alt = mods & Lingmo::ui::AltModifier;
    const uint32_t key = event.key;
    bool accepted = false;

    if (key == key::letter('a') && ctrl && shift) {
        m_terminal->selectAll();
        accepted = true;
    }
    if (key == key::letter('c') && ctrl && shift) {
        m_terminal->copyClipboard();
        accepted = true;
    }
    if (key == key::letter('v') && ctrl && shift) {
        m_terminal->pasteClipboard();
        accepted = true;
    }
    if (key == key::letter('q') && ctrl && shift) {
        m_window->close();
        return true;
    }
    if (key == key::letter('n') && ctrl && shift) {
        m_window->newWindow();
        accepted = true;
    }
    if (key == key::letter('t') && ctrl && shift) {
        m_window->openNewTab();
        accepted = true;
    }
    if (key == key::letter('w') && ctrl && shift) {
        m_window->closeCurrentTab();
        return true;
    }

    // Tabs: Ctrl+Tab and Ctrl+PgDn go right, Ctrl+Shift+Tab and Ctrl+PgUp left;
    // with Shift, Ctrl+PgUp and Ctrl+PgDn move the tab instead.
    if (ctrl) {
        if (key == key::Tab && !shift) {
            m_window->cycleTab(1);
            accepted = true;
        } else if (key == key::Tab && shift) {
            m_window->cycleTab(-1);
            accepted = true;
        } else if (key == key::PageDown) {
            shift ? m_window->moveCurrentTab(1) : m_window->cycleTab(1);
            accepted = true;
        } else if (key == key::PageUp) {
            shift ? m_window->moveCurrentTab(-1) : m_window->cycleTab(-1);
            accepted = true;
        }
    }

    // Alt+1 to Alt+8 pick a tab, Alt+9 the last one.
    if (alt && !ctrl && key >= '1' && key <= '9' && m_window->tabCount() > 1) {
        m_window->selectTab(key == '9' ? m_window->tabCount() - 1 : int(key - '1'));
        accepted = true;
    }

    if (key == key::letter('f') && ctrl && shift) {
        openFind();
        accepted = true;
    }

    if (key == key::letter('k') && ctrl && shift)
        accepted = m_session->clearScreenAndScrollback();

    // Zoom: Ctrl+= (or Ctrl++), Ctrl+-, Ctrl+0.
    if (ctrl && !alt) {
        if (key == '=' || key == '+') {
            m_window->zoom(1);
            accepted = true;
        } else if (key == '-' && !shift) {
            m_window->zoom(-1);
            accepted = true;
        } else if (key == '0' && !shift) {
            m_window->zoom(0);
            accepted = true;
        }
    }

    if (key == key::F1 + 10) {
        m_window->toggleFullScreen();
        accepted = true;
    }

    return accepted;
}

static std::string elided(const std::string& text, size_t length)
{
    const std::u16string utf16 = strings::utf8ToUtf16(text);
    if (utf16.size() <= length)
        return text;
    return strings::utf16ToUtf8(utf16.substr(0, length - 1)) + "\xe2\x80\xa6";
}

// /^[a-z][a-z0-9+.-]*:\/\/\S+$/i
static bool isLink(const std::string& text)
{
    const size_t scheme = text.find("://");
    if (scheme == std::string::npos || scheme == 0 || scheme + 3 >= text.size())
        return false;
    if (!std::isalpha(static_cast<unsigned char>(text[0])))
        return false;
    for (size_t i = 1; i < scheme; ++i) {
        const char c = text[i];
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' && c != '.' && c != '-')
            return false;
    }
    for (size_t i = scheme + 3; i < text.size(); ++i) {
        if (std::isspace(static_cast<unsigned char>(text[i])))
            return false;
    }
    return true;
}

void TerminalPage::openMenu(double x, double y)
{
    // A right click on a word outside the selection selects that word first,
    // so Copy and the file and link items act on what was clicked.
    if (!m_terminal->isSelectedAt(x, y))
        m_terminal->selectWordAt(x, y);

    const std::string selection = m_terminal->hasSelection()
            ? std::string(strings::trimmed(m_terminal->selectedText()))
            : std::string();
    const bool selectionIsLink = isLink(selection);

    std::string link = m_terminal->linkAt(x, y);
    if (link.empty() && selectionIsLink)
        link = selection;
    m_menuLink = link;
    m_menuPath = link.empty() ? ProcessHelper::existingPath(selection, m_session->currentDir()) : std::string();

    const PointF position = m_terminal->mapToWindow({x, y});
    std::weak_ptr<bool> alive = m_alive;
    Clipboard::requestText([this, alive, position](std::string text) {
        if (!alive.lock())
            return;
        m_menuCanPaste = !text.empty();

        // The menu is a window of its own, made on the first right click.
        if (!m_menu)
            m_menu = std::make_unique<Menu>();
        m_menu->clear();

        if (!m_menuLink.empty()) {
            m_menu->addItem(tr("Open Link"), [this] { m_window->openLink(m_menuLink); });
            m_menu->addItem(tr("Copy Link"), [this] { Clipboard::setText(m_menuLink); });
            m_menu->addSeparator();
        }
        if (!m_menuPath.empty()) {
            const std::string name = m_menuPath.substr(m_menuPath.rfind('/') + 1);
            m_menu->addItem(format(tr("Open “{name}”"), {{"name", elided(name.empty() ? m_menuPath : name, 30)}}),
                            [this] { ProcessHelper::openUrl(m_menuPath); });
            m_menu->addItem(tr("Show in File Manager"), [this] { ProcessHelper::showInFileManager(m_menuPath); });
            m_menu->addSeparator();
        }
        if (m_terminal->hasSelection())
            m_menu->addItem(tr("Copy"), [this] { m_terminal->copyClipboard(); });
        if (m_menuCanPaste)
            m_menu->addItem(tr("Paste"), [this] { m_terminal->pasteClipboard(); });
        m_menu->addItem(tr("Select All"), [this] { m_terminal->selectAll(); });
        m_menu->addItem(tr("Find…"), [this] { openFind(); });
        if (m_terminal->scrollbarMaximum() > 0)
            m_menu->addItem(tr("Clear Scrollback"), [this] { m_session->clearScrollback(); });
        m_menu->addItem(tr("Reset Terminal"), [this] { m_session->resetTerminal(); });
        m_menu->addSeparator();
        m_menu->addItem(tr("New Tab"), [this] { m_window->openNewTab(); });
        m_menu->addItem(tr("New Window"), [this] { m_window->newWindow(); });
        m_menu->addItem(tr("Open File Manager"), [this] { ProcessHelper::openFileManager(m_session->currentDir()); });
        m_menu->addSeparator();
        m_menu->addItem(m_window->isFullScreen() ? tr("Exit full screen") : tr("Full screen"),
                        [this] { m_window->toggleFullScreen(); });
        m_menu->addItem(tr("Settings"), [this] { m_window->showSettings(); });

        if (Window* window = this->window())
            m_menu->popup(window, position);
    });
}
