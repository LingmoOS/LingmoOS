// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "searchwindow.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/clipboard.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/popup_glass.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
using namespace Lingmo::ui;
using Lingmo::I18n::tr;
namespace Search {
namespace {
constexpr double width = 720;
constexpr double barHeight = 64;
constexpr double resultHeight = 60;
const char* categoryName(Category category) {
    switch (category) {
    case Category::Applications: return tr("Applications");
    case Category::Files: return tr("Files");
    case Category::Settings: return tr("Settings");
    case Category::Calculator: return tr("Calculator");
    default: return tr("All results");
    }
}
const char* kindName(Kind kind) {
    switch (kind) {
    case Kind::Application: return tr("Application");
    case Kind::File: return tr("File or folder");
    case Kind::Setting: return tr("Settings");
    case Kind::Calculation: return tr("Copy result");
    }
    return "";
}
class FilterButton : public View {
public:
    explicit FilterButton(std::string glyph) { m_icon = addChild<Icon>(std::move(glyph), 22); }
    std::function<void()> onClicked;
    bool selected = false;
    bool acceptsFocus() const override { return true; }
    bool acceptsPointer() const override { return true; }
protected:
    void layout() override {
        m_icon->setGeometry({(geometry().width - 22) / 2, (geometry().height - 22) / 2, 22, 22});
        m_icon->setColor(selected ? Theme::current().highlight : Theme::current().text);
    }
    void paint(Painter& painter) override {
        if (selected || m_hover || hasVisibleFocus())
            painter.fillEllipse(localRect().shrunk(Margins::all(5)), Theme::current().highlight.withAlpha(selected ? 0.18 : 0.08));
    }
    void pointerEnter() override { m_hover = true; requestPaint(); }
    void pointerLeave() override { m_hover = false; requestPaint(); }
    void pointerPress(const PointerEvent& event) override { m_pressed = event.button == MouseButton::Left; }
    void pointerRelease(const PointerEvent& event) override {
        if (m_pressed && event.button == MouseButton::Left && localRect().contains(event.position)) if (auto action = onClicked) action();
        m_pressed = false;
    }
    bool keyPress(const KeyEvent& event) override {
        if (event.key != key::Return && event.key != key::Space) return false;
        if (auto action = onClicked) action(); return true;
    }
private:
    Icon* m_icon;
    bool m_hover = false, m_pressed = false;
};
class SearchField : public TextField {
public:
    std::function<bool(const KeyEvent&)> onSearchKey;
protected:
    bool keyPress(const KeyEvent& event) override {
        if (onSearchKey && onSearchKey(event)) return true;
        return TextField::keyPress(event);
    }
};
class ResultRow : public View {
public:
    ResultRow() {
        m_icon = addChild<Icon>("grid", 23);
        m_themeIcon = addChild<ThemeIcon>("", 28);
        m_title = addChild<Label>(); m_title->setWeight(500); m_title->setFontSize(15);
        m_detail = addChild<Label>(); m_detail->setFontSize(12); m_detail->setElide(Label::Elide::Middle);
        m_kind = addChild<Label>(); m_kind->setFontSize(12); m_kind->setTextAlignment(Label::TextAlignment::Right);
        m_kind->setVerticallyCentered(true);
    }
    void bind(const Result& result, bool selected) {
        m_selected = selected;
        const bool themed = result.kind == Kind::Application && !result.icon.empty()
            && !themeIconPath(result.icon, 28, window() ? window()->scale() : 1).empty();
        m_themeIcon->setIconName(result.icon);
        m_themeIcon->setVisible(themed); m_icon->setVisible(!themed);
        m_icon->setIconName(result.kind == Kind::Application ? "grid" : result.kind == Kind::File ? "folder"
            : result.kind == Kind::Setting ? "search-settings" : "search-calculator");
        m_title->setText(result.title);
        m_detail->setText(result.subtitle.empty() ? kindName(result.kind) : result.subtitle);
        m_kind->setText(selected ? "↵" : kindName(result.kind));
        setToolTip(result.subtitle); requestLayout(); requestPaint();
    }
    std::function<void()> onActivate;
    bool acceptsPointer() const override { return true; }
protected:
    void layout() override {
        const double w = geometry().width;
        m_themeIcon->setGeometry({14, 16, 28, 28});
        m_icon->setGeometry({16, 18, 23, 23}); m_icon->setColor(Theme::current().highlight);
        m_title->setGeometry({54, 9, w - 196, 24}); m_detail->setGeometry({54, 34, w - 196, 19});
        m_kind->setGeometry({w - 130, 0, 112, 60});
        m_detail->setColor(Theme::current().disabledText); m_kind->setColor(Theme::current().disabledText);
    }
    void paint(Painter& painter) override {
        if (m_selected || m_hover) painter.fillRoundedRect(localRect(), 12, Theme::current().highlight.withAlpha(m_selected ? 0.15 : 0.06));
    }
    void pointerEnter() override { m_hover = true; requestPaint(); }
    void pointerLeave() override { m_hover = false; requestPaint(); }
    void pointerPress(const PointerEvent& event) override { m_pressed = event.button == MouseButton::Left; }
    void pointerRelease(const PointerEvent& event) override {
        if (m_pressed && event.button == MouseButton::Left && localRect().contains(event.position)) if (auto action = onActivate) action();
        m_pressed = false;
    }
private:
    Icon* m_icon;
    ThemeIcon* m_themeIcon;
    Label *m_title, *m_detail, *m_kind;
    bool m_selected = false, m_hover = false, m_pressed = false;
};
void registerGlyphs() {
    static bool done = false; if (done) return; done = true;
    Icon::registerGlyph("search-settings", R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M4 6h16M4 12h16M4 18h16"/><circle cx="9" cy="6" r="2"/><circle cx="16" cy="12" r="2"/><circle cx="8" cy="18" r="2"/></g></svg>)");
    Icon::registerGlyph("search-calculator", R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><rect x="5" y="3" width="14" height="18" rx="2"/><path d="M8 7h8M8 12h1m5 0h2M8 16h1m5 0h2"/></g></svg>)");
}
}
class SearchView : public View {
public:
    explicit SearchView(SearchWindow& owner) : m_owner(owner) {
        registerGlyphs();
        m_searchIcon = addChild<Icon>("search", 23);
        m_field = addChild<SearchField>(); m_field->setName("search-query"); m_field->setFrameVisible(false);
        m_field->setFontSize(22); m_field->setMaximumLength(512); m_field->setContextMenuEnabled(false);
        m_field->onTextEdited = [this] { m_owner.setQuery(m_field->text()); };
        m_field->onSearchKey = [this](const KeyEvent& event) {
            if (event.key == key::Escape) { m_owner.hide(); return true; }
            if (event.key == key::Down) { m_owner.moveSelection(1); return true; }
            if (event.key == key::Up) { m_owner.moveSelection(-1); return true; }
            if (event.key == key::Return || event.key == key::KeypadEnter) { m_owner.activateSelected(); return true; }
            return false;
        };
        const std::vector<std::string> glyphs = {"grid", "folder", "search-settings", "search-calculator"};
        for (size_t i = 0; i < glyphs.size(); ++i) {
            auto* button = addChild<FilterButton>(glyphs[i]); button->setName("category-" + std::to_string(i + 1));
            button->onClicked = [this, i] {
                const auto category = static_cast<Category>(i + 1);
                m_owner.setCategory(m_owner.category() == category ? Category::All : category);
                focus();
            };
            m_filters.push_back(button);
        }
        m_list = addChild<ListView>(); m_list->setName("search-results"); m_list->setRowHeight(resultHeight); m_list->setSpacing(2);
        m_list->createItem = [] { return std::make_unique<ResultRow>(); };
        m_list->bindItem = [this](View* view, size_t row) {
            if (row >= m_owner.m_response.results.size()) return;
            auto* result = static_cast<ResultRow*>(view); result->setName("result-" + std::to_string(row));
            result->bind(m_owner.m_response.results[row], row == m_owner.m_selected);
            result->onActivate = [this, row] { m_owner.activate(row); };
        };
        m_heading = addChild<Label>(); m_heading->setFontSize(12); m_heading->setName("results-heading");
        m_empty = addChild<Label>(); m_empty->setTextAlignment(Label::TextAlignment::Center); m_empty->setWrap(true); m_empty->setName("search-message");
        m_footer = addChild<Label>(); m_footer->setFontSize(11);
    }
    void focus() { m_field->setFocus(); }
    void refresh() {
        if (m_field->text() != m_owner.m_query) m_field->setText(m_owner.m_query);
        m_field->setPlaceholder(m_owner.category() == Category::All ? tr("Search Lingmo") : categoryName(m_owner.category()));
        for (size_t i = 0; i < m_filters.size(); ++i) {
            m_filters[i]->selected = m_owner.m_category == static_cast<Category>(i + 1);
            m_filters[i]->setToolTip(categoryName(static_cast<Category>(i + 1)));
            m_filters[i]->requestLayout(); m_filters[i]->requestPaint();
        }
        const bool expanded = !m_owner.m_query.empty() || !m_owner.m_error.empty();
        m_list->setVisible(expanded); m_heading->setVisible(expanded); m_footer->setVisible(expanded);
        m_list->setRowCount(m_owner.m_response.results.size()); m_list->reload();
        m_heading->setText(m_owner.m_searching ? tr("Searching…") : categoryName(m_owner.m_category));
        m_empty->setVisible(expanded && (m_owner.m_response.results.empty() || !m_owner.m_error.empty()));
        m_empty->setText(!m_owner.m_error.empty() ? m_owner.m_error : m_owner.m_searching ? tr("Searching…") : tr("No results. Try another name."));
        m_footer->setText(m_owner.m_response.fileLimitReached ? tr("File search limited to common folders. Refine your query.")
            : tr("↑ ↓ Select    ↵ Open / copy    Esc Close"));
        requestLayout(); requestPaint();
    }
    void revealSelected() { if (!m_owner.m_response.results.empty()) m_list->scrollToRow(m_owner.m_selected); }
protected:
    void layout() override {
        const double w = geometry().width, h = geometry().height;
        const double step = w < 600 ? 48 : 64;
        const double fieldWidth = w - 4 * step - 16;
        m_searchIcon->setGeometry({20, 20, 23, 23}); m_searchIcon->setColor(Theme::current().disabledText);
        m_field->setGeometry({48, 8, fieldWidth - 64, 48});
        for (size_t i = 0; i < m_filters.size(); ++i) m_filters[i]->setGeometry({fieldWidth + 16 + i * step, (64 - (step - 8)) / 2, step - 8, step - 8});
        m_heading->setGeometry({24, 92, w - 48, 22}); m_heading->setColor(Theme::current().disabledText);
        m_list->setGeometry({12, 120, w - 24, std::max(0.0, h - 162)});
        m_empty->setGeometry({24, 132, w - 48, 68});
        m_footer->setGeometry({24, h - 28, w - 48, 20}); m_footer->setColor(Theme::current().disabledText);
        if (auto* surface = window()) {
            m_glass = surface->backgroundGlassAvailable();
            surface->setBackgroundColor(Color{0, 0, 0, 0}); surface->setTranslucentSubpixelText(false);
            surface->setBackgroundGlass(m_glass ? std::optional<GlassMaterial>(popupGlass(Theme::current().darkMode)) : std::nullopt);
            Path body; body.addRoundedRect({0, 0, fieldWidth, 64}, 32, 32);
            for (size_t i = 0; i < m_filters.size(); ++i) body.addEllipse(m_filters[i]->geometry());
            if (h > barHeight) body.addRoundedRect({0, 80, w, h - 80}, 18, 18);
            surface->setBodyShape(body);
            surface->setBackgroundBlur(localRect(), 18); surface->setBackgroundBlurShape(body);
        }
    }
    void paint(Painter& painter) override {
        const double w = geometry().width, h = geometry().height;
        const auto& theme = Theme::current();
        if (!m_glass) {
            painter.fillRoundedRect({0, 0, w - 4 * (w < 600 ? 48 : 64) - 16, 64}, 32, theme.secondBackground);
            for (auto* filter : m_filters) painter.fillEllipse(filter->geometry(), theme.secondBackground);
        }
        if (h > barHeight) painter.fillRoundedRect({0, 80, w, h - 80}, 18, theme.background);
    }
    bool keyPress(const KeyEvent& event) override {
        if (event.key == key::Escape) { m_owner.hide(); return true; }
        return false;
    }
private:
    SearchWindow& m_owner;
    Icon* m_searchIcon;
    SearchField* m_field;
    std::vector<FilterButton*> m_filters;
    ListView* m_list;
    Label *m_heading, *m_empty, *m_footer;
    bool m_glass = false;
};
SearchWindow::SearchWindow(bool liveProviders, Lingmo::base::dbus::Connection* runtimeBus)
    : m_runtimeBus(runtimeBus ? runtimeBus : Lingmo::ui::Application::instance()->sessionBus())
{
    m_debounce.setSingleShot(true); m_debounce.setInterval(std::chrono::milliseconds(150));
    m_debounce.onTimeout = [this] { performSearch(); };
    if (liveProviders) {
        m_registry = &Lingmo::base::ApplicationRegistry::instance();
        m_appsChanged = m_registry->changed.connect([this] { if (m_visible) queueSearch(); });
        using Directory = Lingmo::base::paths::UserDirectory;
        for (auto d : {Directory::Desktop, Directory::Documents, Directory::Downloads, Directory::Pictures, Directory::Music, Directory::Videos}) {
            const auto path = Lingmo::base::paths::userDirectory(d);
            // An XDG directory set to HOME means disabled; never recursively scan all HOME.
            const auto normalized = std::filesystem::path(path).lexically_normal();
            if (normalized != std::filesystem::path(Lingmo::base::paths::home()).lexically_normal()
                && normalized != normalized.root_path()) m_roots.push_back(normalized.string());
        }
    }
    m_languageChanged = Lingmo::ui::Application::instance()->languageChanged.connect([this] { if (m_window) m_window->setTitle(tr("Search")); queueSearch(); refresh(); });
    m_screensChanged = Lingmo::ui::Application::instance()->screensChanged.connect([this] { if (m_visible) updateSize(); });
    refresh();
}
void SearchWindow::createWindow()
{
    m_window = std::make_unique<Window>();
    m_window->setTitle(tr("Search")); m_window->setSkipTaskbar(true); m_window->setAlwaysOnTop(true);
    m_window->setUserResizable(false); m_window->resize({width, barHeight});
    auto view = std::make_unique<SearchView>(*this); m_view = view.get(); m_window->setContentView(std::move(view));
    m_window->onCloseRequested = [this] { hide(); };
    m_window->onActiveChanged = [this, surface = m_window.get()] {
        if (m_window.get() != surface) return;
        if (surface->isActive()) m_wasActive = true;
        else if (m_visible && m_wasActive) hide();
    };
}
SearchWindow::~SearchWindow() { if (m_cancel) *m_cancel = true; m_job.cancel(); }
void SearchWindow::show(const std::string& token) {
    if (m_cancel) *m_cancel = true;
    ++m_generation; m_job.cancel(); m_debounce.stop(); m_searching = false;
    m_launch = {}; m_launching = false;
    if (!m_window) createWindow();
    m_visible = true; m_wasActive = false; m_query.clear(); m_error.clear(); m_response = {}; m_selected = 0;
    refresh(); updateSize(); m_window->show(); m_view->focus(); m_window->activate(token);
}
void SearchWindow::hide() {
    if (m_cancel) *m_cancel = true;
    ++m_generation; m_debounce.stop(); m_job.cancel(); m_launch = {};
    m_visible = false; m_launching = false; m_searching = false; m_wasActive = false;
    // Unmap Wayland surfaces by destroying the window after the input callback.
    // Keep the process alive for clipboard ownership and the registry cache.
    if (m_window) {

        auto retired = std::shared_ptr<Window>(std::move(m_window));
        Lingmo::ui::Application::instance()->loop().post([retired] {});
    }
    m_view = nullptr;
    m_response = {}; m_query.clear(); m_error.clear();
}
void SearchWindow::toggle(const std::string& token) { if (m_visible) hide(); else show(token); }
void SearchWindow::setTestSources(std::vector<Application> apps, std::vector<std::string> roots) { m_testApps = std::move(apps); m_roots = std::move(roots); }
void SearchWindow::setQuery(const std::string& query) {
    if (query.size() > 512) return;
    m_query = query.find_first_not_of(" \t\r\n") == std::string::npos ? std::string() : query;
    m_error.clear(); m_selected = 0; queueSearch();
}
void SearchWindow::setCategory(Category category) { m_category = category; m_selected = 0; queueSearch(); }
void SearchWindow::queueSearch() {
    if (m_cancel) *m_cancel = true;
    ++m_generation; m_job.cancel(); m_debounce.stop(); m_response = {}; m_error.clear();
    m_launch = {}; m_launching = false;
    m_searching = m_visible && !m_query.empty();
    if (m_searching) m_debounce.start();
    refresh();
}
void SearchWindow::performSearch() {
    if (!m_visible || m_query.empty()) return;
    Request request; request.query = m_query; request.category = m_category; request.roots = m_roots;
    request.settings = settingsCatalog([](const char* text) { return std::string(tr(text)); });
    if (m_registry) for (const auto& entry : m_registry->applications()) {
        if (entry->id == "lingmo-search.desktop") continue;
        request.applications.push_back({entry->id, entry->name, entry->genericName, entry->comment, entry->icon, entry->keywords});
    } else request.applications = m_testApps;
    request.canceled = m_cancel = std::make_shared<std::atomic_bool>(false);
    const auto generation = m_generation;
    m_job = m_workers.run([request] { return search(request); }, [weak = m_weak.getWeakPtr(), generation](Response result) {
        if (!weak || !weak->m_visible || generation != weak->m_generation) return;
        weak->m_response = std::move(result); weak->m_searching = false; weak->m_selected = 0; weak->refresh();
    });
}
void SearchWindow::updateSize() {
    if (!m_window) return;
    auto screens = Lingmo::ui::Application::instance()->screens();
    double availableWidth = width, availableHeight = 760;
    Lingmo::ui::Application::Screen screen;
    if (!screens.empty()) {
        screen = screens.front();
        for (const auto& item : screens) if (item.primary) { screen = item; break; }
        const auto area = screen.workArea.width > 0 ? screen.workArea : screen.geometry;
        availableWidth = std::max(320.0, area.width - 32);
        availableHeight = std::max(240.0, area.height - 64);
    }
    const double w = std::min(width, availableWidth);
    const bool expanded = !m_query.empty() || !m_error.empty();
    const double rows = std::max(1.0, std::min(6.0, double(m_response.results.size())));
    const double h = expanded ? std::min(availableHeight, 172 + rows * 62) : barHeight;
    if (m_window->size() != SizeF{w, h}) m_window->resize({w, h});
    if (!screens.empty()) {
        const auto area = screen.workArea.width > 0 ? screen.workArea : screen.geometry;
        PlacementAnchor placement;
        placement.rect = {area.x + area.width / 2, area.y + std::min(area.height * 0.30, std::max(0.0, area.height - h)), 1, 1};
        placement.anchor = PopupAnchor::Center; placement.gravity = PopupAnchor::Bottom;
        m_window->setPlacementAnchor(placement);
    }
}
void SearchWindow::refresh() { if (m_view) m_view->refresh(); if (m_visible) updateSize(); }
void SearchWindow::moveSelection(int delta) {
    const auto count = m_response.results.size(); if (!count) return;
    m_selected = size_t((int64_t(m_selected) + int64_t(count) + delta % int64_t(count)) % int64_t(count)); refresh(); m_view->revealSelected();
}
void SearchWindow::activateSelected() { activate(m_selected); }
void SearchWindow::activate(size_t index) {
    if (m_launching || index >= m_response.results.size() || !m_visible) return;
    const auto result = m_response.results[index];
    if (result.kind == Kind::Calculation) { Clipboard::setText(result.id); hide(); return; }
    m_launching = true;
    m_window->requestActivationToken([weak = m_weak.getWeakPtr(), result, generation = m_generation](const std::string& token) {
        if (weak && weak->m_visible && generation == weak->m_generation) weak->launch(result, token);
    });
}
void SearchWindow::operationFailed(const std::string& error) {
    m_launching = false; m_error = std::string(tr("Could not open result")) + ": " + error;
    m_response = {}; refresh();
}
void SearchWindow::launch(const Result& result, const std::string& token) {
    auto* bus = m_runtimeBus;
    if (!bus) { operationFailed(tr("Application runtime unavailable")); return; }
    Lingmo::base::dbus::Proxy runtime(bus, "org.lingmo.AppRuntime", "/AppRuntime", "org.lingmo.AppRuntime");
    runtime.setTimeout(std::chrono::seconds(5));
    const auto done = [weak = m_weak.getWeakPtr(), generation = m_generation](const Lingmo::base::dbus::Reply& reply) {
        if (!weak || !weak->m_visible || generation != weak->m_generation) return;
        if (reply.isError() || reply.value<uint32_t>().value_or(0) == 0)
            weak->operationFailed(reply.isError() ? reply.error().message : tr("Application runtime unavailable"));
        else weak->hide();
    };
    if (result.kind == Kind::Application)
        m_launch = runtime.callAsync("launchApplication", done, result.id, std::vector<std::string>{}, token);
    else {
        std::vector<std::string> command;
        std::string id;
        if (result.kind == Kind::Setting) { command = {"lingmo-settings", "-m", result.id}; id = "lingmo-settings"; }
        else {
            std::error_code error;
            const auto type = std::filesystem::symlink_status(result.id, error);
            if (error || std::filesystem::is_symlink(type) || (!std::filesystem::is_regular_file(type) && !std::filesystem::is_directory(type))) {
                operationFailed(tr("The file is no longer available")); return;
            }
            command = {"xdg-open", Lingmo::base::url::fromLocalFile(result.id)}; id = "lingmo-search-open";
        }
        m_launch = runtime.callAsync("launchCommand", done, id, command, std::string(), token);
    }
}
}
