// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "advancedpages.h"
#include "graph.h"
#include "base/i18n.h"
#include "ui/button.h"
#include "ui/clipboard.h"
#include "ui/label.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
using namespace Lingmo::ui;
using Lingmo::i18n::tr;
namespace Calculator {
namespace {
class IntegerKey : public Button {
public:
    explicit IntegerKey(const std::string& text) : Button(text)
    {
        for (const auto& child : children())
            if (auto* label = dynamic_cast<Label*>(child.get())) label->setFontSize(18);
    }
    std::function<void()> onEvaluate;
protected:
    bool keyPress(const KeyEvent& event) override
    {
        if (event.key == key::Return || event.key == key::KeypadEnter) {
            if (onEvaluate) onEvaluate();
            return true;
        }
        return Button::keyPress(event);
    }
};
}
std::string errorText(Error error)
{
    switch (error) {
    case Error::DivisionByZero: return tr("Cannot divide by zero");
    case Error::InvalidExpression: return tr("Check the expression");
    case Error::OutOfRange: return tr("Number is out of range");
    case Error::TooLong: return tr("Expression is too long");
    case Error::Domain: return tr("Value is outside the function domain");
    default: return {};
    }
}
class GraphCanvas : public View {
public:
    GraphCanvas() { setName("graph-canvas"); }
    std::string expression = "sin(x)";
    AngleUnit angle = AngleUnit::Radians;
    double cx = 0, cy = 0, hx = 10, hy = 5;
    Error error = Error::None;
    bool hasCurve = false;
    std::function<void()> onChanged;
    void rebuild()
    {
        m_graph = sampleGraph(expression, cx, cy, hx, hy, angle);
        error = m_graph.error;
        hasCurve = !m_graph.segments.empty();
        requestPaint();
        if (onChanged) onChanged();
    }
    void zoom(double factor)
    {
        hx = std::clamp(hx * factor, 0.001, 1000000.0);
        hy = std::clamp(hy * factor, 0.001, 1000000.0);
        rebuild();
    }
    void reset() { cx = cy = 0; hx = 10; hy = 5; rebuild(); }
    bool acceptsPointer() const override { return true; }
protected:
    void paint(Painter& painter) override
    {
        const auto rect = localRect();
        if (rect.width <= 0 || rect.height <= 0) return;
        painter.save();
        painter.clipRoundedRect(rect, 10);
        painter.fillRect(rect, Theme::current().secondBackground);
        auto map = [&](GraphPoint point) -> PointF {
            return {(point.x - cx + hx) * rect.width / (2 * hx),
                    (cy + hy - point.y) * rect.height / (2 * hy)};
        };
        const auto grid = Theme::current().text.withAlpha(0.08);
        for (int i = 1; i < 10; ++i) {
            const double x = rect.width * i / 10;
            const double y = rect.height * i / 10;
            painter.drawLine({x, 0}, {x, rect.height}, grid, painter.scale());
            painter.drawLine({0, y}, {rect.width, y}, grid, painter.scale());
        }
        const auto origin = map({0, 0});
        painter.drawLine({origin.x, 0}, {origin.x, rect.height}, Theme::current().text.withAlpha(0.4), painter.scale());
        painter.drawLine({0, origin.y}, {rect.width, origin.y}, Theme::current().text.withAlpha(0.4), painter.scale());
        for (const auto& segment : m_graph.segments)
            painter.drawLine(map(segment.from), map(segment.to), Theme::current().highlight, 2 * painter.scale());
        painter.restore();
    }
    bool scroll(const ScrollEvent& event) override
    {
        if (event.deltaY != 0) zoom(event.deltaY > 0 ? 1.15 : 1 / 1.15);
        return true;
    }
    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left) { m_dragging = true; m_last = event.position; }
    }
    void pointerMove(const PointerEvent& event) override
    {
        if (!m_dragging || geometry().width <= 0 || geometry().height <= 0) return;
        const auto delta = event.position - m_last;
        cx = std::clamp(cx - delta.x * 2 * hx / geometry().width, -1e9, 1e9);
        cy = std::clamp(cy + delta.y * 2 * hy / geometry().height, -1e9, 1e9);
        m_last = event.position;
        rebuild();
    }
    void pointerRelease(const PointerEvent&) override { m_dragging = false; }
private:
    GraphResult m_graph;
    bool m_dragging = false;
    PointF m_last;
};
GraphPage::GraphPage()
{
    setName("graph-page");
    m_field = addChild<TextField>("sin(x)");
    m_field->setName("graph-expression");
    m_field->setMaximumLength(512);
    m_plot = addChild<Button>(); m_plot->setName("graph-plot");
    m_plot->onClicked = [this] { plot(); };
    m_field->onAccepted = [this] { plot(); };
    m_field->onTextChanged = [this] {
        m_status->setText(tr("Press Plot to update the curve"));
        if (onChanged) onChanged();
    };
    m_angle = addChild<Button>("RAD");
    m_angle->onClicked = [this] {
        m_angleUnit = m_angleUnit == AngleUnit::Radians ? AngleUnit::Degrees : AngleUnit::Radians;
        m_angle->setText(m_angleUnit == AngleUnit::Radians ? "RAD" : "DEG");
        plot();
    };
    m_plus = addChild<Button>("+"); m_plus->setName("graph-zoom-in");
    m_minus = addChild<Button>("−"); m_minus->setName("graph-zoom-out");
    m_reset = addChild<Button>();
    m_plus->onClicked = [this] { m_canvas->zoom(0.8); };
    m_minus->onClicked = [this] { m_canvas->zoom(1.25); };
    m_reset->onClicked = [this] { m_canvas->reset(); };
    m_canvas = addChild<GraphCanvas>();
    m_status = addChild<Label>(); m_status->setWrap(true);
    m_range = addChild<Label>(); m_range->setName("graph-range"); m_range->setWrap(true);
    m_canvas->onChanged = [this] { updateStatus(); };
    retranslate();
    plot();
}
void GraphPage::retranslate()
{
    m_field->setPlaceholder(tr("Expression in x, for example sin(x)"));
    m_plot->setText(tr("Plot")); m_reset->setText(tr("Reset View"));
    m_plus->setToolTip(tr("Zoom In")); m_minus->setToolTip(tr("Zoom Out"));
}
void GraphPage::plot()
{
    m_canvas->expression = m_field->text(); m_canvas->angle = m_angleUnit;
    m_canvas->rebuild();
    if (onChanged) onChanged();
}
void GraphPage::updateStatus()
{
    m_status->setText(m_canvas->error != Error::None ? errorText(m_canvas->error)
        : !m_canvas->hasCurve ? tr("No curve in this range") : tr("Drag to pan · Scroll to zoom"));
    m_range->setText("x: [" + format(m_canvas->cx - m_canvas->hx) + ", " + format(m_canvas->cx + m_canvas->hx)
        + "]   y: [" + format(m_canvas->cy - m_canvas->hy) + ", " + format(m_canvas->cy + m_canvas->hy) + "]");
}
void GraphPage::clear() { m_field->clear(); plot(); }
void GraphPage::copy() { Clipboard::setText(m_field->text()); }
void GraphPage::paste() { m_field->paste(); }
bool GraphPage::hasText() const { return !m_field->text().empty(); }
bool GraphPage::canCalculate() const { return hasText(); }
void GraphPage::layout()
{
    const double w = geometry().width, h = geometry().height;
    m_field->setGeometry({0, 0, w - 96, 38});
    m_plot->setGeometry({w - 88, 0, 88, 38});
    m_angle->setGeometry({0, 50, 70, 32});
    m_plus->setGeometry({78, 50, 64, 32}); m_minus->setGeometry({150, 50, 64, 32});
    m_reset->setGeometry({222, 50, w - 222, 32});
    m_canvas->setGeometry({0, 94, w, std::max(80.0, h - 180)});
    m_range->setGeometry({0, h - 74, w, 32}); m_status->setGeometry({0, h - 36, w, 36});
}
ProgrammerPage::ProgrammerPage()
{
    setName("programmer-page");
    m_field = addChild<TextField>(); m_field->setName("programmer-expression"); m_field->setMaximumLength(512);
    m_result = addChild<Label>("0"); m_result->setName("programmer-result");
    m_result->setFontSize(26); m_result->setTextAlignment(Label::TextAlignment::Right); m_result->setElide(Label::Elide::Middle);
    m_conversions = addChild<Label>(); m_conversions->setName("programmer-conversions");
    m_conversions->setFontSize(13); m_conversions->setFontFamily("monospace"); m_conversions->setWrap(true); m_conversions->setWrapAnywhere(true);
    m_status = addChild<Label>(); m_status->setFontSize(12);
    m_hint = addChild<Label>(); m_hint->setFontSize(12); m_hint->setWrap(true);
    const std::array<unsigned, 4> bases = {16, 10, 8, 2};
    for (unsigned base : bases) {
        auto* button = addChild<Button>(); button->setName("base-" + std::to_string(base));
        button->onClicked = [this, base] { setBase(base); }; m_bases.push_back(button);
    }
    const std::vector<std::string> tokens = {
        "A", "B", "C", "D", "E", "F", "&", "|", "^", "~", "<<", ">>",
        "7", "8", "9", "/", "(", ")", "4", "5", "6", "*", "%", "⌫",
        "1", "2", "3", "-", "0", "+"};
    for (const auto& token : tokens) {
        auto* button = addChild<IntegerKey>(token); button->setName("integer-" + token);
        button->onEvaluate = [this] { calculate(); };
        m_keyTokens.push_back(token);
        button->onClicked = [this, token] { append(token); }; m_keys.push_back(button);
    }
    m_clear = addChild<Button>(); m_clear->onClicked = [this] { clear(); };
    m_calculate = addChild<Button>("="); m_calculate->setName("integer-equals"); m_calculate->onClicked = [this] { calculate(); };
    m_field->onTextChanged = [this] { m_completed = false; update(); };
    m_field->onAccepted = [this] { calculate(); };
    retranslate(); update();
}
void ProgrammerPage::retranslate()
{
    m_field->setPlaceholder(tr("Integer expression"));
    m_hint->setText(tr("Unsigned 64-bit · Wrapping arithmetic · Logical shifts"));
    m_clear->setText(tr("Clear"));
}
void ProgrammerPage::update()
{
    m_value = m_field->text().empty() ? IntegerResult{} : evaluateInteger(m_field->text(), m_base);
    m_result->setText(m_value.error == Error::None ? formatInteger(m_value.value, m_base) : "—");
    m_result->setToolTip(m_result->text());
    m_status->setText(errorText(m_value.error));
    if (m_value.error == Error::None) {
        std::string bits = formatInteger(m_value.value, 2);
        for (int i = static_cast<int>(bits.size()) - 4; i > 0; i -= 4) bits.insert(static_cast<size_t>(i), " ");
        m_conversions->setText("HEX  " + formatInteger(m_value.value, 16) + "\nDEC  " + formatInteger(m_value.value, 10)
            + "\nOCT  " + formatInteger(m_value.value, 8) + "\nBIN  " + bits
            + "\n" + tr("Signed") + "  " + std::to_string(std::bit_cast<int64_t>(m_value.value)));
    } else m_conversions->setText({});
    const std::array<unsigned, 4> bases = {16, 10, 8, 2};
    const std::array<std::string, 4> names = {"HEX", "DEC", "OCT", "BIN"};
    for (size_t i = 0; i < 4; ++i) m_bases[i]->setText((m_base == bases[i] ? "✓ " : "") + names[i]);
    for (size_t i = 0; i < m_keys.size(); ++i) {
        const auto& token = m_keyTokens[i];
        if (token.size() != 1) continue;
        const char c = token.front();
        if (c >= '0' && c <= '9') m_keys[i]->setEnabled(unsigned(c - '0') < m_base);
        else if (c >= 'A' && c <= 'F') m_keys[i]->setEnabled(m_base == 16);
    }
    m_calculate->setEnabled(canCalculate());
    if (onChanged) onChanged();
}
void ProgrammerPage::setBase(unsigned base)
{
    if (base != 2 && base != 8 && base != 10 && base != 16) return;
    // Preserve the numeric value across a base change; do not reinterpret the digits.
    if (!m_field->text().empty() && m_value.error != Error::None) return;
    const bool empty = m_field->text().empty();
    const auto value = m_value.value;
    m_base = base;
    if (!empty) m_field->setText(formatInteger(value, base));
    m_completed = !empty; update();
}
void ProgrammerPage::append(const std::string& text)
{
    auto expression = m_field->text();
    if (text == "⌫") { if (!expression.empty()) expression.pop_back(); }
    else {
        if (m_completed && (text.front() == '(' || (text.front() >= '0' && text.front() <= '9')
            || (text.front() >= 'A' && text.front() <= 'F'))) expression.clear();
        if (expression.size() + text.size() <= 512) expression += text;
    }
    m_field->setText(expression);
}
void ProgrammerPage::calculate()
{
    if (!canCalculate()) return;
    const auto expression = m_field->text();
    const auto answer = formatInteger(m_value.value, m_base);
    m_field->setText(answer); m_completed = true; update();
    if (onCalculated) onCalculated(expression, answer, m_base);
}
void ProgrammerPage::restore(const std::string& result, unsigned base)
{
    if (evaluateInteger(result, base).error != Error::None) return;
    m_base = base;
    m_field->setText(result); m_completed = true; update();
}
void ProgrammerPage::clear() { m_field->clear(); update(); }
void ProgrammerPage::copy() { if (m_value.error == Error::None) Clipboard::setText(formatInteger(m_value.value, m_base)); }
void ProgrammerPage::paste() { m_field->paste(); }
bool ProgrammerPage::hasText() const { return !m_field->text().empty(); }
bool ProgrammerPage::canCalculate() const { return hasText() && m_value.error == Error::None; }
void ProgrammerPage::layout()
{
    const double w = geometry().width, h = geometry().height;
    m_field->setGeometry({0, 0, w, 38}); m_result->setGeometry({0, 46, w, 40});
    for (size_t i = 0; i < 4; ++i) m_bases[i]->setGeometry({i * (w + 8) / 4, 94, (w - 24) / 4, 32});
    m_conversions->setGeometry({0, 136, w, 134}); m_status->setGeometry({0, 274, w, 22});
    const double keyWidth = (w - 5 * 6) / 6;
    const double keyHeight = std::max(28.0, (h - 388) / 5);
    for (size_t i = 0; i < m_keys.size(); ++i)
        m_keys[i]->setGeometry({(i % 6) * (keyWidth + 6), KeypadTop + (i / 6) * (keyHeight + 6), keyWidth, keyHeight});
    m_clear->setGeometry({0, h - 56, (w - 8) / 2, 32});
    m_calculate->setGeometry({(w + 8) / 2, h - 56, (w - 8) / 2, 32});
    m_hint->setGeometry({0, h - 20, w, 20});
}
}
