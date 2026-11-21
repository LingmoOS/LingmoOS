// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculatorwindow.h"
#include "advancedpages.h"
#include "base/i18n.h"
#include "ui/about_dialog.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/clipboard.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/popup_glass.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/window_header.h"
#include <algorithm>
#include <array>
#include <functional>
using namespace Lingmo::ui;
using Lingmo::i18n::tr;
namespace Calculator {
namespace {
constexpr double kKeyboardTop = 220;
constexpr double kMaterialGap = 8;

std::string modeName(Mode mode)
{
    switch (mode) {
    case Mode::Scientific: return tr("Scientific");
    case Mode::Graph: return tr("Function Graph");
    case Mode::Programmer: return tr("Programmer");
    default: return tr("Standard");
    }
}
std::string displayText(std::string text)
{
    for (size_t p = 0; (p = text.find('*', p)) != std::string::npos; p += 2) text.replace(p, 1, "×");
    for (size_t p = 0; (p = text.find('/', p)) != std::string::npos; p += 2) text.replace(p, 1, "÷");
    return text;
}
class CenteredHeader : public WindowHeader {
public:
    CenteredHeader() : WindowHeader(44)
    {
        m_title = addChild<Label>(tr("Calculator"));
        m_title->setName("window-title");
        m_title->setTextAlignment(Label::TextAlignment::Center);
        m_title->setVerticallyCentered(true);
    }
    void retranslate() { m_title->setText(tr("Calculator")); }
protected:
    void layout() override
    {
        WindowHeader::layout();
        // Symmetric insets preserve the window buttons and center on the whole window.
        m_title->setGeometry({104, 0, std::max(0.0, geometry().width - 208), 44});
    }
private:
    Label* m_title;
};
class KeyButton : public Button {
public:
    explicit KeyButton(std::string text, bool keypad = false)
        : Button(text), m_accent(text == "=")
    {
        if (keypad) for (const auto& child : children())
            if (auto* label = dynamic_cast<Label*>(child.get())) label->setFontSize(22);
    }
protected:
    bool keyPress(const KeyEvent& event) override
    {
        if (event.key == key::Return || event.key == key::KeypadEnter) return false;
        return Button::keyPress(event);
    }
    void paint(Painter& painter) override
    {
        Button::paint(painter);
        if (m_accent) painter.fillRoundedRect(localRect(), Theme::current().mediumRadius,
                                             Theme::current().highlight.withAlpha(0.18));
    }
private:
    bool m_accent;
};
}
class CalculatorView : public View {
public:
    explicit CalculatorView(CalculatorWindow& owner) : m_owner(owner)
    {
        setName("calculator");
        m_header = addChild<CenteredHeader>();
        // Compact fallback in the title bar. Hidden after global-menu registration succeeds.
        m_menu = addChild<Button>("⋯"); m_menu->setName("local-menu");
        m_menu->onClicked = [this] { m_owner.showMenu(m_menu); };
        m_historyBack = addChild<Button>(); m_historyBack->setName("history-back");
        m_historyBack->onClicked = [&owner] { owner.toggleHistory(); };
        m_expression = addChild<Label>(); m_expression->setName("expression");
        m_expression->setTextAlignment(Label::TextAlignment::Right); m_expression->setElide(Label::Elide::Middle);
        m_display = addChild<Label>("0"); m_display->setName("display");
        m_display->setTextAlignment(Label::TextAlignment::Right); m_display->setVerticallyCentered(true);
        m_display->setElide(Label::Elide::Middle); m_display->setFontFeatures({{"tnum", 1}});
        m_status = addChild<Label>(); m_status->setTextAlignment(Label::TextAlignment::Right); m_status->setFontSize(12);
        m_clear = addChild<KeyButton>("AC"); m_clear->setName("clear"); m_clear->onClicked = [&owner] { owner.clear(); };
        m_back = addChild<KeyButton>("⌫"); m_back->setName("backspace"); m_back->onClicked = [&owner] { owner.backspace(); };
        const std::array<std::string, 20> labels = {"(", ")", "%", "÷", "7", "8", "9", "×", "4", "5", "6", "−", "1", "2", "3", "+", "±", "0", ".", "="};
        const std::array<std::string, 20> tokens = {"(", ")", "%", "/", "7", "8", "9", "*", "4", "5", "6", "-", "1", "2", "3", "+", "sign", "0", ".", "="};
        for (size_t i = 0; i < labels.size(); ++i) {
            auto* button = addChild<KeyButton>(labels[i], true); button->setName("key-" + tokens[i]);
            button->onClicked = [&owner, token = tokens[i]] {
                if (token == "=") owner.calculate();
                else if (token == "sign") owner.negate();
                else owner.input(token);
            };
            m_keys.push_back(button);
        }
        const std::array<std::string, 15> scientific = {"sin(", "cos(", "tan(", "sqrt(", "^", "asin(", "acos(", "atan(", "ln(", "log(", "pi", "e", "!", "abs(", "angle"};
        for (const auto& token : scientific) {
            auto* button = addChild<KeyButton>(token == "angle" ? "RAD" : token);
            button->setName("science-" + token);
            button->onClicked = [&owner, token] { if (token == "angle") owner.toggleAngleUnit(); else owner.input(token); };
            m_scientific.push_back(button);
        }
        m_historyTitle = addChild<Label>(); m_historyTitle->setFontSize(22);
        m_clearHistory = addChild<Button>(); m_clearHistory->onClicked = [&owner] { owner.clearHistory(); };
        m_historyScroll = addChild<ScrollView>(); m_historyScroll->setName("history-list");
        m_graph = addChild<GraphPage>(); m_programmer = addChild<ProgrammerPage>();
        m_graph->onChanged = [&owner] { owner.refreshMenus(); };
        m_programmer->onChanged = [&owner] { owner.refreshMenus(); };
        m_programmer->onCalculated = [&owner](const std::string& expression, const std::string& result, unsigned base) { owner.recordInteger(expression, result, base); };
    }
    bool acceptsFocus() const override { return true; }
    GraphPage& graph() { return *m_graph; }
    ProgrammerPage& programmer() { return *m_programmer; }
    void refresh(const Model& model, Mode mode, bool history, bool globalMenu)
    {
        m_currentMode = mode; m_showHistory = history;
        const bool math = !history && (mode == Mode::Standard || mode == Mode::Scientific);
        m_header->retranslate();
        m_menu->setVisible(!globalMenu); m_menu->setToolTip(tr("Menu"));
        m_historyBack->setText(tr("Back")); m_historyBack->setVisible(history);
        for (View* view : std::vector<View*>{m_expression, m_display, m_status, m_clear, m_back}) view->setVisible(math);
        for (auto* key : m_keys) key->setVisible(math);
        for (auto* key : m_scientific) key->setVisible(math && mode == Mode::Scientific);
        m_graph->setVisible(!history && mode == Mode::Graph);
        m_programmer->setVisible(!history && mode == Mode::Programmer);
        m_historyTitle->setVisible(history); m_historyScroll->setVisible(history); m_clearHistory->setVisible(history);
        m_historyTitle->setText(tr("History")); m_clearHistory->setText(tr("Clear History"));
        m_clearHistory->setEnabled(!model.history().empty());
        m_clear->setToolTip(tr("Clear (Escape)")); m_back->setToolTip(tr("Delete last digit (Backspace)"));
        m_display->setText(model.expression().empty() ? "0" : displayText(model.expression()));
        m_display->setToolTip(displayText(model.expression()));
        m_display->setFontSize(model.expression().size() > 18 ? 22 : model.expression().size() > 12 ? 30 : 38);
        const auto preview = model.preview();
        m_expression->setText(model.completed() ? displayText(model.calculation()) + " ="
            : preview.empty() ? modeName(mode) : "= " + preview);
        m_status->setText(errorText(model.error()));
        m_scientific.back()->setText(model.angleUnit() == AngleUnit::Radians ? "RAD" : "DEG");
        m_graph->retranslate(); m_programmer->retranslate();
        std::string signature = tr("No calculations yet");
        for (const auto& entry : model.history()) signature += '\n' + entry.expression + '=' + entry.result + ':' + std::to_string(entry.base) + (entry.integer ? "i" : "r");
        if (signature != m_historySignature) {
            m_historySignature = signature;
            auto list = std::make_unique<Column>(); list->setSpacing(8);
            if (model.history().empty()) {
                auto* label = list->addChild<Label>(tr("No calculations yet"));
                label->setLayoutHints({.fillWidth = true});
            }
            for (size_t i = 0; i < model.history().size(); ++i) {
                const auto& entry = model.history()[i];
                const auto text = (entry.integer ? "[" + std::to_string(entry.base) + "] " : "") + displayText(entry.expression) + " = " + entry.result;
                auto* row = list->addChild<Button>(text); row->setName("history-" + std::to_string(i));
                row->setToolTip(text); row->setLayoutHints({.fillWidth = true, .preferredHeight = 48});
                row->onClicked = [this, i] { m_owner.recallHistory(i); };
            }
            m_historyScroll->setContentView(std::move(list));
        }
        requestLayout(); requestPaint();
    }
protected:
    void layout() override
    {
        const auto r = localRect();
        m_header->setGeometry({0, 0, r.width, 44});
        m_menu->setGeometry({12, 6, 40, 32});
        m_expression->setGeometry({20, 52, r.width - 40, 24});
        m_display->setGeometry({20, 82, r.width - 40, 58}); m_status->setGeometry({20, 144, r.width - 40, 22});
        const double half = (r.width - 40) / 2;
        m_clear->setGeometry({16, 176, half, 32}); m_back->setGeometry({24 + half, 176, half, 32});
        double top = kKeyboardTop;
        if (m_currentMode == Mode::Scientific) {
            const double width = (r.width - 64) / 5;
            for (size_t i = 0; i < m_scientific.size(); ++i)
                m_scientific[i]->setGeometry({16 + (i % 5) * (width + 8), top + (i / 5) * 40, width, 32});
            top += 124;
        }
        const double width = (r.width - 56) / 4;
        const double height = std::max(28.0, (r.height - top - 48) / 5);
        for (size_t i = 0; i < m_keys.size(); ++i)
            m_keys[i]->setGeometry({16 + (i % 4) * (width + 8), top + (i / 4) * (height + 8), width, height});
        m_historyBack->setGeometry({20, 56, 76, 32});
        m_historyTitle->setGeometry({108, 54, std::max(0.0, r.width - 268), 36});
        m_clearHistory->setGeometry({r.width - 148, 56, 128, 32});
        m_historyScroll->setGeometry({20, 108, r.width - 40, std::max(0.0, r.height - 128)});
        m_graph->setGeometry({16, 52, r.width - 32, r.height - 68});
        m_programmer->setGeometry({16, 52, r.width - 32, r.height - 68});
        updateSurfaceMaterial();
    }
    void paint(Painter& painter) override
    {
        const auto rect = localRect();
        const auto& theme = Theme::current();
        const double split = glassHeight();
        // The compositor provides the glass behind transparent upper pixels.
        // Painting the keyboard body fully opaque keeps its controls visually stable.
        if (!m_glassAvailable)
            painter.fillRect({0, 0, rect.width, split}, theme.secondBackground);
        else if (window() && !window()->isActive())
            painter.fillRect({0, 0, rect.width, split}, theme.background.withAlpha(0.12));
        painter.fillRect({0, split, rect.width, rect.height - split}, theme.background);
        const double line = 1 / painter.scale();
        painter.fillRect({0, split, rect.width, line}, theme.text.withAlpha(theme.darkMode ? 0.12 : 0.08));
    }
    bool keyPress(const KeyEvent& event) override
    {
        if (event.modifiers & (ControlModifier | AltModifier | SuperModifier)) return false;
        if (m_showHistory) {
            if (event.key == key::Escape) { m_owner.toggleHistory(); return true; }
            return false;
        }
        if (m_currentMode == Mode::Graph || m_currentMode == Mode::Programmer) return false;
        if (event.key == key::Return || event.key == key::KeypadEnter || event.text == "=") m_owner.calculate();
        else if (event.key == key::Escape || event.key == key::Delete) m_owner.clear();
        else if (event.key == key::BackSpace) m_owner.backspace();
        else if (!event.text.empty() && event.text.find_first_not_of("0123456789.+-*/()%!^abcdefghijklmnopqrstuvwxyzE") == std::string::npos) m_owner.input(event.text);
        else return false;
        return true;
    }
private:
    double glassHeight() const
    {
        double height = 44;
        if (m_showHistory) height = 100;
        else if (m_currentMode == Mode::Standard || m_currentMode == Mode::Scientific)
            height = kKeyboardTop - kMaterialGap;
        else if (m_currentMode == Mode::Programmer)
            height = m_programmer->geometry().y + ProgrammerPage::KeypadTop - kMaterialGap;
        return std::clamp(height, 0.0, localRect().height);
    }
    void updateSurfaceMaterial()
    {
        auto* surface = window();
        if (!surface) return;
        m_glassAvailable = surface->backgroundGlassAvailable();
        const auto& theme = Theme::current();
        surface->setBackgroundColor(m_glassAvailable ? Color{0, 0, 0, 0} : theme.background);
        surface->setTranslucentSubpixelText(!m_glassAvailable);
        if (m_glassAvailable) {
            surface->setBackgroundGlass(popupGlass(theme.darkMode));
            // Keep the compositor's outline and shadow around the complete window.
            // Only the upper region exposes glass; the rest is covered by the solid body.
            surface->setBackgroundBlur(localRect(), theme.windowRadius);
        } else {
            surface->setBackgroundGlass(std::nullopt);
            surface->setBackgroundBlur({}, 0);
        }
    }
    CalculatorWindow& m_owner;
    CenteredHeader* m_header;
    Button *m_menu, *m_clearHistory, *m_historyBack;
    KeyButton *m_clear, *m_back;
    Label *m_expression, *m_display, *m_status, *m_historyTitle;
    ScrollView* m_historyScroll;
    GraphPage* m_graph;
    ProgrammerPage* m_programmer;
    std::vector<KeyButton*> m_keys, m_scientific;
    Mode m_currentMode = Mode::Standard;
    bool m_showHistory = false;
    bool m_glassAvailable = false;
    std::string m_historySignature;
};
CalculatorWindow::CalculatorWindow()
{
    m_window.setTitle(tr("Calculator")); m_window.setMinimumSize({380, 600}); m_window.resize({400, 620});
    auto view = std::make_unique<CalculatorView>(*this); m_view = view.get(); m_window.setContentView(std::move(view));
    m_about = std::make_unique<AboutDialog>(&m_window);
    m_window.onActiveChanged = [this] { m_window.requestPaint(); };
    m_window.onStatesChanged = [this] { m_window.requestLayout(); m_window.requestPaint(); };
    m_window.addShortcut(ControlModifier, key::letter('c'), [this] { copy(); });
    m_window.addShortcut(ControlModifier, key::letter('v'), [this] { paste(); });
    m_window.addShortcut(ControlModifier, key::letter('h'), [this] { toggleHistory(); });
    m_window.addShortcut(ControlModifier, key::letter('q'), [] { Application::instance()->quit(); });
    m_window.addShortcut(ControlModifier, key::letter('l'), [this] { clear(); });
    for (int i = 0; i < 4; ++i)
        m_window.addShortcut(ControlModifier, uint32_t('1' + i), [this, i] { setMode(static_cast<Mode>(i)); });
    if (auto* bus = Application::instance()->sessionBus())
        m_globalMenu = std::make_unique<GlobalMenu>(bus, [this] { return menuEntries(); }, std::function<void()>{});
    if (m_globalMenu)
        m_menuAvailability = m_globalMenu->availabilityChanged.connect([this](bool) { refresh(); });
    m_language = Application::instance()->languageChanged.connect([this] { m_window.setTitle(tr("Calculator")); refresh(); });
    m_clipboard = Clipboard::changed().connect([this](ClipboardMode mode) { if (mode == ClipboardMode::Clipboard) refreshMenus(); });
    refresh();
}
CalculatorWindow::~CalculatorWindow() = default;
void CalculatorWindow::show() { m_window.show(); m_view->setFocus(); }
void CalculatorWindow::refreshMenus() { if (m_globalMenu) m_globalMenu->update(); }
void CalculatorWindow::refresh() { m_view->refresh(m_model, m_mode, m_historyVisible, m_globalMenu && m_globalMenu->isAvailable()); refreshMenus(); }
void CalculatorWindow::input(const std::string& text) { m_model.input(text); refresh(); }
void CalculatorWindow::calculate()
{
    if (m_historyVisible) return;
    if (m_mode == Mode::Graph) m_view->graph().plot();
    else if (m_mode == Mode::Programmer) m_view->programmer().calculate();
    else { m_model.calculate(); refresh(); }
}
void CalculatorWindow::clear()
{
    if (m_historyVisible) return;
    if (m_mode == Mode::Graph) m_view->graph().clear();
    else if (m_mode == Mode::Programmer) m_view->programmer().clear();
    else { m_model.clear(); refresh(); }
}
void CalculatorWindow::backspace() { m_model.backspace(); refresh(); }
void CalculatorWindow::negate() { m_model.negate(); refresh(); }
void CalculatorWindow::toggleAngleUnit()
{
    m_model.setAngleUnit(m_model.angleUnit() == AngleUnit::Radians ? AngleUnit::Degrees : AngleUnit::Radians);
    refresh();
}
void CalculatorWindow::copy()
{
    if (m_historyVisible) return;
    if (m_mode == Mode::Graph) m_view->graph().copy();
    else if (m_mode == Mode::Programmer) m_view->programmer().copy();
    else Clipboard::setText(m_model.expression().empty() ? "0" : m_model.expression());
}
void CalculatorWindow::paste()
{
    if (m_historyVisible) return;
    if (m_mode == Mode::Graph) { m_view->graph().paste(); return; }
    if (m_mode == Mode::Programmer) { m_view->programmer().paste(); return; }
    Clipboard::requestText([weak = m_weak.getWeakPtr(), mode = m_mode](std::string text) {
        if (weak && weak->m_mode == mode && !weak->m_historyVisible) { weak->m_model.paste(text); weak->refresh(); }
    });
}
void CalculatorWindow::setMode(Mode mode)
{
    const bool resize = !m_window.states().maximized && !m_window.states().fullScreen;
    if (resize) m_modeSizes[static_cast<size_t>(m_mode)] = m_window.size();
    m_mode = mode; m_historyVisible = false;
    const SizeF minimum = mode == Mode::Standard ? SizeF{380, 600}
        : mode == Mode::Scientific ? SizeF{460, 740} : mode == Mode::Graph ? SizeF{600, 600} : SizeF{520, 780};
    const auto size = m_modeSizes[static_cast<size_t>(mode)];
    m_window.setMinimumSize(minimum);
    if (resize)
        m_window.resize({std::max(size.width, minimum.width), std::max(size.height, minimum.height)});
    refresh(); m_view->setFocus();
}
void CalculatorWindow::toggleHistory()
{
    m_historyVisible = !m_historyVisible;
    refresh(); m_view->setFocus();
}
void CalculatorWindow::recallHistory(size_t index)
{
    if (index >= m_model.history().size()) return;
    const auto entry = m_model.history()[index];
    if (entry.integer) {
        setMode(Mode::Programmer);
        m_view->programmer().restore(entry.result, entry.base);
        return;
    }
    if (!m_model.recall(index)) return;
    m_historyVisible = false;
    if (m_mode == Mode::Graph || m_mode == Mode::Programmer) setMode(Mode::Scientific);
    else refresh();
    m_view->setFocus();
}
void CalculatorWindow::recordInteger(const std::string& expression, const std::string& result, unsigned base)
{
    m_model.addIntegerHistory(expression, result, base);
    refresh();
}
void CalculatorWindow::clearHistory() { m_model.clearHistory(); refresh(); }
void CalculatorWindow::showAbout()
{
    m_about->setName(tr("Calculator")); m_about->setIconSource("lingmo-calculator");
    m_about->setDescription(tr("Standard, scientific, graphing and 64-bit programmer calculator. History stays in this session."));
    m_about->show();
}
std::vector<GlobalMenu::Entry> CalculatorWindow::menuEntries()
{
    using E = GlobalMenu::Entry;
    auto item = [](std::string label, std::function<void()> action, std::vector<std::string> shortcut = {}) {
        E entry; entry.label = std::move(label); entry.action = std::move(action); entry.shortcut = std::move(shortcut); return entry;
    };
    const bool math = m_mode == Mode::Standard || m_mode == Mode::Scientific;
    const bool hasText = math ? !m_model.expression().empty() : m_mode == Mode::Graph ? m_view->graph().hasText() : m_view->programmer().hasText();
    E app; app.label = tr("File");
    app.children = {item(tr("Clear"), [this] { clear(); }, {"Control", "l"}), E{.separator = true},
        item(tr("Quit"), [] { Application::instance()->quit(); }, {"Control", "q"})};
    app.children[0].enabled = !m_historyVisible && hasText;
    E edit; edit.label = tr("Edit");
    edit.children = {item(tr("Copy"), [this] { copy(); }, {"Control", "c"}), item(tr("Paste"), [this] { paste(); }, {"Control", "v"}),
        E{.separator = true}, item(tr("Delete Last Digit"), [this] { backspace(); }, {"BackSpace"})};
    edit.children[0].enabled = !m_historyVisible && hasText && (m_mode != Mode::Programmer || m_view->programmer().canCalculate());
    edit.children[1].enabled = !m_historyVisible && Clipboard::hasText();
    edit.children[3].enabled = !m_historyVisible && math && hasText;
    E modes; modes.label = tr("Mode");
    for (int i = 0; i < 4; ++i) modes.children.push_back(item(modeName(static_cast<Mode>(i)),
        [this, i] { setMode(static_cast<Mode>(i)); }, {"Control", std::to_string(i + 1)}));
    for (int i = 0; i < 4; ++i) {
        modes.children[i].checkable = true;
        modes.children[i].checked = m_mode == static_cast<Mode>(i);
    }
    E calc; calc.label = tr("Calculation");
    calc.children = {item(m_mode == Mode::Graph ? tr("Plot") : tr("Calculate"), [this] { calculate(); }, {"Return"}),
        item(tr("Change Sign"), [this] { negate(); }), item(m_model.angleUnit() == AngleUnit::Radians ? tr("Use Degrees") : tr("Use Radians"), [this] { toggleAngleUnit(); }),
        E{.separator = true}, item(m_historyVisible ? tr("Back") : tr("History"), [this] { toggleHistory(); }, {"Control", "h"}),
        item(tr("Clear History"), [this] { clearHistory(); })};
    calc.children[0].enabled = !m_historyVisible && (math ? !m_model.completed() && evaluate(m_model.expression(), m_model.angleUnit()).error == Error::None
        : m_mode == Mode::Graph ? m_view->graph().canCalculate() : m_view->programmer().canCalculate());
    calc.children[1].enabled = !m_historyVisible && math;
    calc.children[2].visible = m_mode == Mode::Scientific;
    calc.children[2].enabled = !m_historyVisible;
    calc.children[5].enabled = !m_model.history().empty();
    E help; help.label = tr("Help"); help.children = {item(tr("About Calculator"), [this] { showAbout(); })};
    return {std::move(app), std::move(edit), std::move(modes), std::move(calc), std::move(help)};
}
void CalculatorWindow::showModeMenu(View* anchor)
{
    m_localMenu = std::make_unique<Menu>();
    for (int i = 0; i < 4; ++i) {
        const auto mode = static_cast<Mode>(i);
        const auto id = m_localMenu->addItem(modeName(mode), [this, mode] { setMode(mode); }, "Ctrl+" + std::to_string(i + 1));
        m_localMenu->setItemChecked(id, m_mode == mode);
    }
    m_localMenu->popupFor(anchor);
}
void CalculatorWindow::showMenu(View* anchor)
{
    m_localMenu = std::make_unique<Menu>();
    for (const auto& group : menuEntries()) {
        auto* submenu = m_localMenu->addSubmenu(group.label);
        for (const auto& entry : group.children) {
            if (!entry.visible) continue;
            if (entry.separator) submenu->addSeparator();
            else { const auto id = submenu->addItem(entry.label, entry.action); submenu->setItemEnabled(id, entry.enabled); if (entry.checkable) submenu->setItemChecked(id, entry.checked); }
        }
    }
    m_localMenu->popupFor(anchor);
}
}
