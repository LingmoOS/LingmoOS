// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/settingswindow.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <optional>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/timer.h"
#include "ui/application.h"
#include "ui/color_provider.h"
#include "ui/event.h"
#include "ui/header_backdrop.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/window_header.h"
#include "widgets/metrics.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/sidebar.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kButtonSize = 32;
static constexpr double kButtonsSpacing = 1;
static constexpr size_t kMaxHistoryLength = 50;
static constexpr uint32_t kBackKey = 0x1008ff26;
static constexpr uint32_t kForwardKey = 0x1008ff27;
// The frosted sidebar's widening of the backdrop; FishUI's 1.2 was nearly
// clear, so the 70% tint alone read as plain transparency.
static constexpr double kSidebarBlurSigma = 20;

// The sidebar's width on whole device pixels, so its edge and the page's
// clip share one.
static double sidebarWidth(double scale)
{
    return std::round(metrics::sidebarWidth * scale) / scale;
}

// The header's own part before the window buttons: a RowLayout of back,
// forward and the page title, anchored from the sidebar's edge. The title is
// FishUI's AlignedLabel on the sidebar title's baseline; the buttons stay
// centred on its capitals.
class HeaderBar : public View
{
public:
    explicit HeaderBar(Sidebar* sidebar)
        : m_sidebar(sidebar)
    {
        m_row = addChild<RowLayout>();
        m_row->setSpacing(kButtonsSpacing);
        back = m_row->addChild<HeaderButton>(std::string(), kButtonSize);
        back->setGlyph("chevron-left", 16);
        back->setName("back");
        forward = m_row->addChild<HeaderButton>(std::string(), kButtonSize);
        forward->setGlyph("chevron-right", 16);
        forward->setName("forward");
        // Settings' NavigationButton plates.
        for (HeaderButton* button : {back, forward})
            button->setPlateAlphas({0.17, 0.26}, {0.21, 0.15});
        title = m_row->addChild<Label>();
        title->setName("title");
        title->setFontSize(metrics::points(metrics::headerTitlePointSize));
    }

    HeaderButton* back;
    HeaderButton* forward;
    Label* title;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const Window* window = this->window();
        const double scale = window ? window->scale() : 1;
        const auto snap = [scale](double value) { return std::round(value * scale) / scale; };
        // Header and sidebar share the window's top edge. The title reads low
        // on the sidebar title's baseline, so it sits 1px above it.
        const double titleRaise = 1;
        Label* sidebarTitle = m_sidebar->titleLabel();
        const double textBaseline = sidebarTitle->geometry().y + sidebarTitle->baselineOffset() - titleRaise;
        const double capCenter = snap(snap(textBaseline) - title->capHeight() / 2);
        const double buttonTop = capCenter + titleRaise - kButtonSize / 2;

        LayoutHints button{.preferredWidth = kButtonSize, .preferredHeight = kButtonSize,
                           .verticalAlignment = Alignment::Start};
        button.margins.top = buttonTop;
        if (!(back->layoutHints().margins == button.margins)) {
            back->setLayoutHints(button);
            forward->setLayoutHints(button);
        }
        LayoutHints text{.fillWidth = true, .verticalAlignment = Alignment::Start};
        text.margins = {theme.smallSpacing, snap(textBaseline - title->baselineOffset()), 0, 0};
        if (!(title->layoutHints().margins == text.margins))
            title->setLayoutHints(text);

        const bool active = !window || window->isActive();
        title->setColor(active ? theme.text : theme.disabledText);
        const double left = sidebarWidth(scale) + theme.smallSpacing * 1.5;
        m_row->setGeometry({left, 0, std::max(0.0, geometry().width - left - theme.smallSpacing),
                            geometry().height});
    }

private:
    Sidebar* m_sidebar;
    RowLayout* m_row;
};

// The window header over the pages: it moves the window, and passes wheels
// and touchpad scrolls on to the page beneath.
class SettingsHeader : public WindowHeader
{
public:
    explicit SettingsHeader(SettingsWindow& window)
        : WindowHeader(SettingsWindow::headerHeight)
        , m_window(window)
    {
    }

protected:
    bool scroll(const ScrollEvent& event) override
    {
        Page* page = m_window.currentPage();
        return page && page->scrollFromHeader(event);
    }

private:
    SettingsWindow& m_window;
};

class SettingsRoot : public View
{
public:
    explicit SettingsRoot(SettingsWindow& window)
    {
        sidebar = addChild<Sidebar>();
        pages = addChild<View>();
        pages->setClipsChildren(true);
        header = addChild<SettingsHeader>(window);
        bar = header->setContent<HeaderBar>(sidebar);
    }

    void setPage(std::unique_ptr<Page> page)
    {
        // The old page may be the one whose click got here; it goes once the
        // event is done with it.
        while (!pages->children().empty()) {
            std::shared_ptr<View> old = pages->takeChild(pages->children().front().get());
            Lingmo::base::EventLoop::current()->post([old] {});
        }
        Page* view = page.get();
        pages->addChild(std::move(page));
        // FishUI's HeaderBackdrop over the page, under the header.
        if (backdrop) {
            std::shared_ptr<View> old = takeChild(backdrop);
            Lingmo::base::EventLoop::current()->post([old] {});
        }
        backdrop = addChild<HeaderBackdrop>(view);
        backdrop->setBaseColor(backdropBase);
        raiseChild(header);
        requestLayout();
    }

    std::function<void()> onLayout;

    Sidebar* sidebar;
    View* pages;
    HeaderBackdrop* backdrop = nullptr;
    Color backdropBase;
    SettingsHeader* header;
    HeaderBar* bar;

protected:
    void layout() override
    {
        const Window* window = this->window();
        const double scale = window ? window->scale() : 1;
        const double width = geometry().width;
        const double height = geometry().height;
        const double left = sidebarWidth(scale);
        sidebar->setGeometry({0, 0, left, height});
        pages->setGeometry({left, 0, std::max(0.0, width - left), height});
        for (const auto& page : pages->children())
            page->setGeometry({0, 0, pages->geometry().width, height});
        if (backdrop)
            backdrop->setGeometry({left, 0, std::max(0.0, width - left), SettingsWindow::headerHeight});
        header->setGeometry({0, 0, width, SettingsWindow::headerHeight});
        if (onLayout)
            onLayout();
    }
};

SettingsWindow::SettingsWindow()
    : m_window(std::make_unique<Window>())
    , m_revealTimer(std::make_unique<Lingmo::base::Timer>())
{
    m_window->setTitle(tr("Settings"));
    m_window->resize({1000, 740});
    // Half of a 1280-wide screen, so the window can be split beside another.
    m_window->setMinimumSize({640, 600});
    // Under the key the Qt version's main.qml kept it.
    m_window->setRestoresSize("main");
    auto root = std::make_unique<SettingsRoot>(*this);
    m_root = root.get();
    m_sidebar = m_root->sidebar;
    m_back = m_root->bar->back;
    m_forward = m_root->bar->forward;
    m_title = m_root->bar->title;
    m_window->setContentView(std::move(root));
    m_root->onLayout = [this] { updateBackground(); };

    m_back->onClicked = [this] { goBack(); };
    m_forward->onClicked = [this] { goForward(); };
    m_sidebar->onActivated = [this](const std::string& page, const std::string& rootPage) {
        m_pendingPage.clear();
        navigate(page, rootPage);
    };
    m_window->addShortcut(AltModifier, key::Left, [this] { goBack(); });
    m_window->addShortcut(AltModifier, key::Right, [this] { goForward(); });
    m_window->addShortcut(NoModifier, kBackKey, [this] { goBack(); });
    m_window->addShortcut(NoModifier, kForwardKey, [this] { goForward(); });
    m_window->addShortcut(ControlModifier, key::letter('f'), [this] { m_sidebar->focusSearch(); });
    m_window->onStatesChanged = [this] {
        updateBackground();
        m_root->requestLayout();
        m_root->requestPaint();
    };
    m_window->onActiveChanged = [this] {
        updateBackground();
        // Views colour their text by it as they lay out.
        m_window->requestLayout();
        m_root->requestPaint();
        activeChanged.emit();
    };
    // A press anywhere outside the search field takes the focus off it.
    m_window->onPointerPressed = [this](PointF position, MouseButton) { m_sidebar->releaseSearch(position); };
    m_languageChanged = Application::instance()->languageChanged.connect([this] { retranslate(); });

    for (const PageInfo& page : pages()) {
        const bool sidebarPage = !page.category.empty();
        if (sidebarPage && page.initiallyAvailable)
            m_available.push_back(page.id);
        if (!page.watchAvailability)
            continue;
        m_unanswered.push_back(page.id);
        const std::string id = page.id;
        m_watches.push_back(page.watchAvailability(*this, [this, id](bool available) {
            const PageInfo* info = findPage(id);
            std::erase(m_unanswered, id);
            setAvailable(*info, available);
            if (!m_revealed && isReady())
                reveal();
        }));
    }
    updateSidebar();
    switchToIndex(0);

    // A service that never answers holds the window no longer than this.
    m_revealTimer->setSingleShot(true);
    m_revealTimer->onTimeout = [this] { reveal(); };
    m_revealTimer->start(std::chrono::milliseconds(500));
}

SettingsWindow::~SettingsWindow() = default;

bool SettingsWindow::isActive() const
{
    return m_window->isActive();
}

bool SettingsWindow::isReady() const
{
    return m_unanswered.empty();
}

void SettingsWindow::reveal()
{
    if (m_revealed)
        return;
    m_revealed = true;
    m_revealTimer->stop();
    m_window->show();
    revealedChanged.emit();
}

void SettingsWindow::setAvailable(const PageInfo& page, bool available)
{
    if (page.category.empty()) {
        m_sidebar->setSubPageAvailable(page.id, available);
        return;
    }
    const bool present = std::find(m_available.begin(), m_available.end(), page.id) != m_available.end();
    if (present == available)
        return;
    if (available)
        m_available.push_back(page.id);
    else
        std::erase(m_available, page.id);
    const bool currentRemoved = !available && m_historyIndex >= 0
            && m_history[size_t(m_historyIndex)].rootPage == page.id;
    updateSidebar();
    if (currentRemoved)
        switchToIndex(0);
    else if (available && m_pendingPage == page.id)
        switchToPage(page.id);
    updateHeader();
}

void SettingsWindow::updateSidebar()
{
    std::vector<const PageInfo*> entries;
    for (const PageInfo& page : pages()) {
        if (!page.category.empty() && std::find(m_available.begin(), m_available.end(), page.id) != m_available.end())
            entries.push_back(&page);
    }
    m_sidebar->setEntries(std::move(entries));
}

bool SettingsWindow::isInSidebar(const std::string& id) const
{
    const auto& entries = m_sidebar->entries();
    return std::any_of(entries.begin(), entries.end(), [&id](const PageInfo* page) { return page->id == id; });
}

// Skips entries whose sidebar page has since been removed, e.g. Bluetooth.
int SettingsWindow::historyStep(int direction) const
{
    for (int i = m_historyIndex + direction; i >= 0 && i < int(m_history.size()); i += direction) {
        if (isInSidebar(m_history[size_t(i)].rootPage))
            return i;
    }
    return -1;
}

void SettingsWindow::navigate(const std::string& page, const std::string& rootPage)
{
    if (m_historyIndex >= 0) {
        const HistoryEntry& current = m_history[size_t(m_historyIndex)];
        if (current.page == page && current.rootPage == rootPage)
            return;
    }
    m_history.resize(size_t(m_historyIndex + 1));
    m_history.push_back({page, rootPage});
    // A long session must not keep every step; the oldest go first.
    if (m_history.size() > kMaxHistoryLength)
        m_history.erase(m_history.begin(), m_history.end() - kMaxHistoryLength);
    showHistoryEntry(int(m_history.size()) - 1);
}

void SettingsWindow::showHistoryEntry(int index)
{
    m_historyIndex = index;
    const HistoryEntry& entry = m_history[size_t(index)];
    const PageInfo* info = findPage(entry.page);
    std::unique_ptr<Page> page;
    if (info && info->create)
        page = info->create(*this);
    if (!page)
        page = std::make_unique<PendingPage>(*this, info ? tr(info->title.c_str()) : entry.page);
    page->setName("page:" + entry.page);
    m_page = page.get();
    m_root->setPage(std::move(page));
    m_sidebar->setCurrent(entry.page, entry.rootPage);
    updateHeader();
}

void SettingsWindow::retranslate()
{
    m_window->setTitle(tr("Settings"));
    m_sidebar->retranslate();
    if (m_historyIndex >= 0)
        showHistoryEntry(m_historyIndex);
    else
        updateHeader();
}

void SettingsWindow::updateHeader()
{
    m_back->setEnabled(canGoBack());
    m_forward->setEnabled(canGoForward());
    m_title->setText(m_page ? m_page->title() : std::string());
}

void SettingsWindow::updateBackground()
{
    const Theme& theme = Theme::current();
    const bool active = m_window->isActive();
    const SizeF size = m_window->size();
    // The sidebar is a 70% tint over the window's background. Painting the
    // window background at 0.8 below it makes the tint composite to nearly
    // opaque, so over a compositor that draws glass the background stays
    // transparent and the sidebar tints the blurred desktop instead, as
    // FishUI's translucent window did.
    if (m_window->backgroundGlassAvailable()) {
        m_window->setBackgroundColor(Color{0, 0, 0, 0});
        // Frosted rather than FishUI's nearly clear lens: the sidebar's 70%
        // tint needs a visible Gaussian blur under it. The rest of the
        // material stays FishUI's, tinted 12% so the desktop's colours show.
        GlassMaterial glass;
        glass.blurSigma = kSidebarBlurSigma;
        if (theme.darkMode) {
            glass.tintRed = glass.tintGreen = 0.18;
            glass.tintBlue = 0.20;
        }
        m_window->setBackgroundGlass(glass);
        m_window->setTranslucentSubpixelText(false);
    } else {
        m_window->setBackgroundGlass(std::nullopt);
        m_window->setBackgroundColor(
            color(active ? ColorId::TranslucentWindowBackground : ColorId::TranslucentWindowBackgroundInactive));
        m_window->setTranslucentSubpixelText(true);
    }
    m_root->backdropBase = theme.darkMode ? theme.background : theme.secondBackground;
    if (m_root->backdrop)
        m_root->backdrop->setBaseColor(theme.darkMode ? theme.background : theme.secondBackground);
    m_window->setBackgroundBlur({0, 0, sidebarWidth(m_window->scale()), size.height}, theme.windowRadius);
}

void SettingsWindow::switchToIndex(int index)
{
    const auto& entries = m_sidebar->entries();
    if (index < 0 || index >= int(entries.size()))
        return;
    m_pendingPage.clear();
    navigate(entries[size_t(index)]->id, entries[size_t(index)]->id);
}

void SettingsWindow::switchToPage(const std::string& name, const std::string& activationToken)
{
    const PageInfo* page = nullptr;
    for (const PageInfo& info : pages()) {
        if (!info.name.empty() && info.name == name)
            page = &info;
    }
    if (page) {
        // Entries that come with hardware may join the sidebar after the request.
        m_pendingPage = !page->initiallyAvailable && !isInSidebar(page->id) ? page->id : std::string();
        if (page->category.empty()) {
            // A sub page opens under its sidebar page, as if reached from there.
            if (isInSidebar(page->parentId)) {
                m_pendingPage.clear();
                navigate(page->id, page->parentId);
            }
        } else if (isInSidebar(page->id)) {
            m_pendingPage.clear();
            navigate(page->id, page->id);
        }
    }
    // A minimized window is shown again.
    if (m_revealed)
        m_window->activate(activationToken);
}

void SettingsWindow::joinWifi(const std::string& accessPoint, const std::string& activationToken)
{
    m_pendingWifiJoin = accessPoint;
    switchToPage("wlan", activationToken);
    wifiJoinRequested.emit();
}

std::string SettingsWindow::takePendingWifiJoin()
{
    return std::exchange(m_pendingWifiJoin, {});
}

void SettingsWindow::pushPage(const std::string& id)
{
    if (m_historyIndex < 0)
        return;
    navigate(id, m_history[size_t(m_historyIndex)].rootPage);
}

void SettingsWindow::goBack()
{
    const int index = historyStep(-1);
    if (index >= 0)
        showHistoryEntry(index);
}

void SettingsWindow::goForward()
{
    const int index = historyStep(1);
    if (index >= 0)
        showHistoryEntry(index);
}

} // namespace settings
