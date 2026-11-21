// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notresponding/not_responding_sheets.h"

#include <algorithm>
#include <cmath>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "core/main_loop.h"
#include "shell/common/task_manager.h"
#include "shell/shell_window.h"
#include "ui/application.h"
#include "ui/dialog_window.h"
#include "ui/embedded_window.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "wm/shell_surface.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace Lingmo::Shell {

// The layout's spacings: icon to text, title to body, text to the buttons
// (the column's 12 and the button row's top margin 8), between the buttons.
static constexpr double kIconSize = 48;
static constexpr double kRowSpacing = 14;
static constexpr double kTextSpacing = 4;
static constexpr double kButtonsGap = 12 + 8;
static constexpr double kButtonSpacing = 8;

NotRespondingView::NotRespondingView(std::string name, std::string icon)
    : m_name(std::move(name))
    , m_icon(addChild<ui::ThemeIcon>(std::move(icon), kIconSize))
    , m_title(addChild<ui::Label>())
    , m_body(addChild<ui::Label>())
    , m_forceQuit(addChild<ui::DialogButton>(std::string(), true))
    , m_wait(addChild<ui::DialogButton>())
{
    retranslate();
    m_title->setWrap(true);
    m_title->setFontSize(14);
    m_title->setWeight(600);
    m_body->setWrap(true);
    m_body->setFontSize(12);
    m_forceQuit->setDestructive(true);
    m_forceQuit->onClicked = [this] {
        if (onForceQuit)
            onForceQuit();
    };
    m_wait->onClicked = [this] {
        if (onWait)
            onWait();
    };
    applyTheme();
}

const std::string &NotRespondingView::title() const
{
    return m_title->text();
}

void NotRespondingView::retranslate()
{
    m_title->setText(format(tr("“{name}” is not responding"), {{"name", m_name}}));
    m_body->setText(tr("You can wait for it to respond, or force quit it. Anything it has not saved will be lost."));
    m_forceQuit->setText(tr("Force Quit"));
    m_wait->setText(tr("Wait"));
    requestLayout();
}

void NotRespondingView::applyTheme()
{
    m_themeGeneration = ui::Theme::generation();
    const ui::Theme &theme = ui::Theme::current();
    m_title->setColor(theme.text);
    m_body->setColor(theme.disabledText);
}

double NotRespondingView::implicitHeight(double width) const
{
    const double textWidth = width - 2 * kPadding - kIconSize - kRowSpacing;
    const double text = m_title->heightForWidth(textWidth) + kTextSpacing + m_body->heightForWidth(textWidth);
    const double buttons = std::max(m_forceQuit->sizeHint().height, m_wait->sizeHint().height);
    return kPadding + std::max(kIconSize, text) + kButtonsGap + buttons + kPadding;
}

void NotRespondingView::layout()
{
    const double width = geometry().width;
    const double textX = kPadding + kIconSize + kRowSpacing;
    const double textWidth = width - kPadding - textX;
    const double titleHeight = m_title->heightForWidth(textWidth);
    const double bodyHeight = m_body->heightForWidth(textWidth);
    const double textHeight = titleHeight + kTextSpacing + bodyHeight;
    const double rowHeight = std::max(kIconSize, textHeight);
    // The row centres the text column; the icon keeps to the top.
    const double textY = kPadding + (rowHeight - textHeight) / 2;
    m_icon->setGeometry({double(kPadding), double(kPadding), kIconSize, kIconSize});
    m_title->setGeometry({textX, textY, textWidth, titleHeight});
    m_body->setGeometry({textX, textY + titleHeight + kTextSpacing, textWidth, bodyHeight});

    const ui::SizeF wait = m_wait->sizeHint();
    const ui::SizeF forceQuit = m_forceQuit->sizeHint();
    const double buttonsHeight = std::max(wait.height, forceQuit.height);
    const double buttonsY = kPadding + rowHeight + kButtonsGap;
    const double waitX = width - kPadding - wait.width;
    m_wait->setGeometry({waitX, buttonsY + (buttonsHeight - wait.height) / 2, wait.width, wait.height});
    m_forceQuit->setGeometry({waitX - kButtonSpacing - forceQuit.width,
                              buttonsY + (buttonsHeight - forceQuit.height) / 2, forceQuit.width,
                              forceQuit.height});
}

void NotRespondingView::paint(ui::Painter &painter)
{
    if (m_themeGeneration != ui::Theme::generation())
        applyTheme();
    // Opaque, as a dialog is; the compositor clips the corners and draws the
    // outline and the shadow.
    painter.fillRect(localRect(), ui::Theme::current().secondBackground);
}

NotRespondingSheets::NotRespondingSheets(Shell *shell)
    : m_shell(shell)
{
    wm::WindowManager *manager = m_shell->windowManager();
    manager->setNotRespondingPrompt([this](wm::Window *window) { return prompt(window); });
    m_subscriptions.push_back(manager->windowAdded.connect([this](wm::Window *window) { observe(window); }));
    m_subscriptions.push_back(
            manager->windowRemoved.connect([this](wm::Window *window) { onWindowRemoved(window); }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
    for (wm::Window *window : manager->windows())
        observe(window);
}

void NotRespondingSheets::retranslate()
{
    for (auto &[application, sheet] : m_sheets) {
        sheet.view->retranslate();
        sheet.window->shellSurface()->setTitle(sheet.view->title());
        const double height = std::ceil(sheet.view->implicitHeight(NotRespondingView::kWidth));
        sheet.window->setSize(base::Size(NotRespondingView::kWidth, int(height)));
    }
}

NotRespondingSheets::~NotRespondingSheets()
{
    *m_alive = false;
    m_shell->windowManager()->setNotRespondingPrompt({});
    m_subscriptions.clear();
    for (wm::Window *window : m_observed)
        window->removeObserver(this);
    m_observed.clear();
    m_sheets.clear();
    m_closed.clear();
}

int64_t NotRespondingSheets::applicationOf(const wm::Window *window)
{
    return window->processId() > 0 ? window->processId() : -int64_t(window->id());
}

void NotRespondingSheets::observe(wm::Window *window)
{
    // Only application windows are pinged.
    if (!window || window->isShellWindow() || window->isPopup() || m_observed.count(window))
        return;
    window->addObserver(this);
    m_observed.insert(window);
}

bool NotRespondingSheets::prompt(wm::Window *window)
{
    const int64_t application = applicationOf(window);
    if (m_waiting.count(application))
        return true;

    const auto it = m_sheets.find(application);
    if (it == m_sheets.end()) {
        show(window, application);
        return true;
    }
    // Another window of the same application: the sheet moves to it.
    if (it->second.parentWindow != window->id())
        hang(it->second, window);
    it->second.window->activate();
    return true;
}

void NotRespondingSheets::show(wm::Window *window, int64_t application)
{
    const base::DesktopEntry *entry = nullptr;
    const DesktopEntryPtr found = m_shell->tasks().entryForWindow(window->id());
    entry = found.get();
    std::string name = entry ? entry->name : std::string();
    if (name.empty())
        name = window->title().empty() ? window->applicationId() : window->title();
    const std::string icon = entry && !entry->icon.empty() ? entry->icon : "application-x-desktop";

    Sheet sheet;
    sheet.window = std::make_unique<ShellWindow>(m_shell, wm::WindowRole::Sheet);
    auto view = std::make_unique<NotRespondingView>(name, icon);
    sheet.view = view.get();
    view->onWait = [this, application] { wait(application); };
    view->onForceQuit = [this, application] { forceQuit(application); };
    sheet.window->setContentView(std::move(view));
    sheet.window->shellSurface()->setTitle(sheet.view->title());

    ui::EmbeddedWindow *toolkitWindow = sheet.window->window();
    toolkitWindow->addShortcut(ui::NoModifier, ui::key::Escape, [this, application] { wait(application); });
    toolkitWindow->addShortcut(ui::NoModifier, ui::key::Return, [this, application] { wait(application); });
    sheet.view->waitButton()->setFocus();

    hang(sheet, window);
    const double height = std::ceil(sheet.view->implicitHeight(NotRespondingView::kWidth));
    sheet.window->setSize(base::Size(NotRespondingView::kWidth, int(height)));
    auto &stored = m_sheets[application] = std::move(sheet);
    // Mapping activates it; the compositor centres it on its parent.
    stored.window->show();
}

void NotRespondingSheets::hang(Sheet &sheet, wm::Window *parent)
{
    sheet.parentWindow = parent->id();
    // The parent's output's scale.
    const base::Rect bounds = parent->bounds();
    if (const Shell::Output *output = m_shell->outputAt(bounds.center()))
        sheet.window->setOutput(output->name);
    sheet.window->setParentWindow(parent->id());
}

void NotRespondingSheets::wait(int64_t application)
{
    m_waiting.insert(application);
    close(application);
}

void NotRespondingSheets::forceQuit(int64_t application)
{
    const auto it = m_sheets.find(application);
    if (it == m_sheets.end())
        return;
    wm::WindowManager *manager = m_shell->windowManager();
    if (wm::Window *parent = manager->windowById(it->second.parentWindow))
        manager->forceQuit(parent);
    close(application);
}

void NotRespondingSheets::close(int64_t application)
{
    const auto it = m_sheets.find(application);
    if (it == m_sheets.end())
        return;
    std::unique_ptr<ShellWindow> window = std::move(it->second.window);
    m_sheets.erase(it);
    window->hide();
    // A button of this very window may be what closes it.
    m_closed.push_back(std::move(window));
    core::mainLoop()->post([this, alive = m_alive] {
        if (*alive)
            m_closed.clear();
    });
}

void NotRespondingSheets::onWindowResponsiveChanged(wm::Window *window)
{
    if (!window->isResponsive())
        return;
    const int64_t application = applicationOf(window);
    m_waiting.erase(application);
    close(application);
}

void NotRespondingSheets::onWindowDestroying(wm::Window *window)
{
    window->removeObserver(this);
    m_observed.erase(window);
}

void NotRespondingSheets::onWindowRemoved(wm::Window *window)
{
    for (const auto &[application, sheet] : m_sheets) {
        if (sheet.parentWindow == window->id()) {
            close(application);
            break;
        }
    }

    // A process that is gone can be waited for no longer.
    std::set<int64_t> live;
    for (wm::Window *other : m_shell->windowManager()->windows()) {
        if (other != window && !other->isShellWindow())
            live.insert(applicationOf(other));
    }
    for (auto it = m_waiting.begin(); it != m_waiting.end();) {
        if (live.count(*it))
            ++it;
        else
            it = m_waiting.erase(it);
    }
}

ShellWindow *NotRespondingSheets::sheet(int64_t application) const
{
    const auto it = m_sheets.find(application);
    return it == m_sheets.end() ? nullptr : it->second.window.get();
}

NotRespondingView *NotRespondingSheets::view(int64_t application) const
{
    const auto it = m_sheets.find(application);
    return it == m_sheets.end() ? nullptr : it->second.view;
}

uint64_t NotRespondingSheets::parentWindow(int64_t application) const
{
    const auto it = m_sheets.find(application);
    return it == m_sheets.end() ? 0 : it->second.parentWindow;
}

} // namespace Lingmo::Shell
