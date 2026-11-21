// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/colors.h"
#include "views/pathbar.h"
#include "core/mounts.h"
#include "model/foldermodel.h"
#include "views/completionpopup.h"
#include "views/style.h"

#include "base/i18n.h"
#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>
#include <regex>

namespace fm {

using ui::rounding::centeredStart;
using Lingmo::i18n::tr;

static constexpr double kSmallSpacing = 6;
static constexpr double kLargeSpacing = 12;

struct Breadcrumbs::CrumbView
{
    ui::Label* label;
    ui::Icon* chevron;
    double x = 0;
    double width = 0;
    bool folded = false;
};

Breadcrumbs::Breadcrumbs(bool openActions)
    : m_openActions(openActions)
{
    setName("breadcrumbs");
    setClipsChildren(true);
    setTracksHover(true);
    m_foldLabel = addChild<ui::Label>("…");
    m_foldChevron = addChild<ui::Icon>("chevron-right", 10);
}

Breadcrumbs::~Breadcrumbs() = default;

void Breadcrumbs::setCrumbs(std::vector<Crumb> crumbs)
{
    m_crumbs = std::move(crumbs);
    while (m_views.size() > m_crumbs.size()) {
        std::shared_ptr<View> label = takeChild(m_views.back().label);
        std::shared_ptr<View> chevron = takeChild(m_views.back().chevron);
        m_views.pop_back();
    }
    while (m_views.size() < m_crumbs.size()) {
        CrumbView view;
        view.label = addChild<ui::Label>();
        view.chevron = addChild<ui::Icon>("chevron-right", 10);
        m_views.push_back(view);
    }
    for (size_t i = 0; i < m_crumbs.size(); ++i)
        m_views[i].label->setText(m_crumbs[i].name);
    requestLayout();
    requestPaint();
}

double Breadcrumbs::highlightHeight() const
{
    return m_foldLabel->sizeHint().height + kSmallSpacing;
}

// The folding laid out at once: the widest names that fit, folding
// the middle from the root side, then the root, then all but the current.
void Breadcrumbs::relayout()
{
    const size_t n = m_crumbs.size();
    const double sideMargin = kSmallSpacing;
    const double crumbPadding = kLargeSpacing;
    const double separatorWidth = 10 + kSmallSpacing;
    const double gridUnit = style::fontHeight();
    const double ancestorMax = gridUnit * 8;
    const double ancestorMin = gridUnit * 4;
    const double currentMin = gridUnit * 2;

    std::vector<double> natural;
    for (const CrumbView& view : m_views)
        natural.push_back(view.label->sizeHint().width);

    const auto crumbWidth = [&](size_t i, double middle) {
        const double text = i == n - 1 ? natural[i] : std::min(natural[i], i == 0 ? ancestorMax : middle);
        return text + crumbPadding;
    };
    const double foldWidth = m_foldLabel->sizeHint().width + crumbPadding;
    const auto totalWidth = [&](std::pair<int, int> fold, double middle) {
        double width = 0;
        int count = 0;
        if (fold.first >= 0) {
            width += foldWidth;
            ++count;
        }
        for (size_t i = 0; i < n; ++i) {
            if (int(i) >= fold.first && int(i) <= fold.second)
                continue;
            width += crumbWidth(i, middle);
            ++count;
        }
        return width + separatorWidth * std::max(0, count - 1);
    };

    std::vector<std::pair<int, int>> candidates {{-1, -1}};
    for (int last = 1; last <= int(n) - 3; ++last)
        candidates.push_back({1, last});
    if (n >= 3)
        candidates.push_back({0, int(n) - 3});
    if (n >= 2)
        candidates.push_back({0, int(n) - 2});

    const double available = geometry().width - sideMargin * 2;
    std::pair<int, int> fold = candidates.back();
    double middle = ancestorMax;
    for (const auto& candidate : candidates) {
        if (totalWidth(candidate, ancestorMax) <= available) {
            fold = candidate;
            break;
        }
        if (totalWidth(candidate, ancestorMin) <= available) {
            // The longest names give way first: the widest cap that fits.
            double low = ancestorMin;
            double high = ancestorMax;
            while (high - low > 0.5) {
                const double mid = (low + high) / 2;
                if (totalWidth(candidate, mid) <= available)
                    low = mid;
                else
                    high = mid;
            }
            fold = candidate;
            middle = std::floor(low);
            break;
        }
    }

    const double others = n > 0 ? totalWidth(fold, middle) - crumbWidth(n - 1, middle) : 0;
    m_currentTextWidth = n > 0 ? std::min(natural[n - 1], std::max(currentMin, available - others - crumbPadding)) : 0;
    m_middleTextWidth = middle;
    m_foldFirst = fold.first;
    m_foldLast = fold.second;

    double x = sideMargin;
    for (size_t j = 0; j < n; ++j) {
        if (int(j) == fold.first) {
            m_foldX = x;
            x += foldWidth + separatorWidth;
        }
        CrumbView& view = m_views[j];
        view.folded = int(j) >= fold.first && int(j) <= fold.second;
        if (view.folded)
            continue;
        view.x = x;
        const double textWidth = j == n - 1 ? m_currentTextWidth : std::min(natural[j], j == 0 ? ancestorMax : middle);
        view.width = textWidth + crumbPadding;
        x += view.width + separatorWidth;
    }
}

void Breadcrumbs::layout()
{
    relayout();
    const ui::Theme& theme = ui::Theme::current();
    const ui::Color ancestor = style::textAlpha(0.6);
    const double height = geometry().height - 4;
    const size_t n = m_crumbs.size();

    for (size_t i = 0; i < n; ++i) {
        CrumbView& view = m_views[i];
        const bool selected = i == n - 1;
        view.label->setVisible(!view.folded);
        view.chevron->setVisible(!view.folded && !selected);
        if (view.folded)
            continue;
        view.label->setColor(selected ? theme.text : ancestor);
        // The end of a name, often a version or a suffix, tells folders apart.
        view.label->setElide(selected ? ui::Label::Elide::Middle : ui::Label::Elide::Right);
        const ui::SizeF hint = view.label->sizeHint();
        const double textWidth = view.width - kLargeSpacing;
        view.label->setGeometry({view.x + centeredStart(view.width, textWidth), 2 + centeredStart(height, hint.height),
                                 textWidth, hint.height});
        view.label->setToolTip(view.label->isElided() ? m_crumbs[i].name : std::string());
        view.chevron->setColor(ancestor);
        view.chevron->setGeometry({view.x + view.width + kSmallSpacing / 2, 2 + centeredStart(height, 10), 10, 10});
    }

    const bool folded = m_foldFirst >= 0;
    m_foldLabel->setVisible(folded);
    m_foldChevron->setVisible(folded);
    if (folded) {
        const ui::SizeF hint = m_foldLabel->sizeHint();
        const double width = hint.width + kLargeSpacing;
        m_foldLabel->setColor(ancestor);
        m_foldLabel->setGeometry({m_foldX + centeredStart(width, hint.width), 2 + centeredStart(height, hint.height),
                                  hint.width, hint.height});
        m_foldChevron->setColor(ancestor);
        m_foldChevron->setGeometry({m_foldX + width + kSmallSpacing / 2, 2 + centeredStart(height, 10), 10, 10});
    }
}

void Breadcrumbs::paint(ui::Painter& painter)
{
    const ui::Theme& theme = ui::Theme::current();
    const ui::Color pressed = ui::color(ui::ColorId::ItemPressed);
    const double height = geometry().height - 4;
    const double plate = highlightHeight();
    const auto drawPlate = [&](double x, double width) {
        painter.drawSmoothRectangle({x, 2 + (height - plate) / 2, width, plate}, theme.smallRadius, pressed);
    };
    if (m_pressed >= 0 && size_t(m_pressed) < m_views.size() && !m_views[size_t(m_pressed)].folded)
        drawPlate(m_views[size_t(m_pressed)].x, m_views[size_t(m_pressed)].width);
    if (m_foldFirst >= 0 && (m_pressed == -2 || m_foldMenuOpen))
        drawPlate(m_foldX, m_foldLabel->sizeHint().width + kLargeSpacing);
}

int Breadcrumbs::crumbAt(ui::PointF point) const
{
    if (point.y < 2 || point.y > geometry().height - 2)
        return -1;
    for (size_t i = 0; i < m_views.size(); ++i) {
        const CrumbView& view = m_views[i];
        if (!view.folded && point.x >= view.x && point.x < view.x + view.width)
            return int(i);
    }
    if (m_foldFirst >= 0 && point.x >= m_foldX && point.x < m_foldX + m_foldLabel->sizeHint().width + kLargeSpacing)
        return -2;
    return -1;
}

void Breadcrumbs::pointerPress(const ui::PointerEvent& event)
{
    m_pressed = crumbAt(event.position);
    m_pressedButton = event.button;
    m_pressPosition = event.position;
    m_dragged = false;
    requestPaint();
}

void Breadcrumbs::pointerMove(const ui::PointerEvent& event)
{
    if (m_dragged)
        return;
    if (std::abs(event.position.x - m_pressPosition.x) > 4 || std::abs(event.position.y - m_pressPosition.y) > 4) {
        m_dragged = true;
        if (onDragStarted)
            onDragStarted();
    }
}

void Breadcrumbs::pointerRelease(const ui::PointerEvent& event)
{
    const int pressed = m_pressed;
    m_pressed = -1;
    requestPaint();
    if (!m_dragged && pressed == -1 && event.button == ui::MouseButton::Left && onEmptyClicked) {
        onEmptyClicked();
        return;
    }
    if (m_dragged || pressed == -1 || crumbAt(event.position) != pressed)
        return;

    if (pressed == -2) {
        m_menu = std::make_unique<ui::Menu>();
        for (int i = m_foldFirst; i <= m_foldLast; ++i) {
            const Crumb crumb = m_crumbs[size_t(i)];
            m_menu->addItem(crumb.name, [this, crumb] {
                if (onItemClicked)
                    onItemClicked(crumb.path);
            });
        }
        m_foldMenuOpen = true;
        m_menu->onClosed = [this] {
            m_foldMenuOpen = false;
            requestPaint();
        };
        m_menu->popup(window(), mapToWindow(event.position));
        return;
    }

    const std::string path = m_crumbs[size_t(pressed)].path;
    if (m_pressedButton == ui::MouseButton::Right && m_openActions) {
        m_menu = std::make_unique<ui::Menu>();
        m_menu->addItem(tr("Open In New Tab"), [this, path] {
            if (onOpenInNewTab)
                onOpenInNewTab(path);
        });
        m_menu->addItem(tr("Open In New Window"), [this, path] {
            if (onOpenInNewWindow)
                onOpenInNewWindow(path);
        });
        m_menu->popup(window(), mapToWindow(event.position));
    } else if (m_pressedButton == ui::MouseButton::Middle && m_openActions) {
        if (onOpenInNewTab)
            onOpenInNewTab(path);
    } else if (m_pressedButton == ui::MouseButton::Left) {
        if (onItemClicked)
            onItemClicked(path);
    }
}

void Breadcrumbs::hoverMove(const ui::PointerEvent&)
{
}

void Breadcrumbs::hoverChanged()
{
}

// The editor: keys go to the completion first; losing the keys closes it.
class PathEditor : public ui::TextField
{
public:
    explicit PathEditor(PathBar* bar)
        : m_bar(bar)
    {
    }

protected:
    void focusOut() override
    {
        TextField::focusOut();
        m_bar->editorFocusChanged(false);
    }
    bool keyPress(const ui::KeyEvent& event) override
    {
        if (m_bar->editorKey(event))
            return true;
        return TextField::keyPress(event);
    }

private:
    PathBar* m_bar;
};

// The clear button inside the editor while a search shows.
class ClearButton : public ui::Icon
{
public:
    ClearButton()
        : Icon("clear", 16)
    {
    }

    std::function<void()> onClicked;
    bool acceptsPointer() const override { return true; }

protected:
    void pointerRelease(const ui::PointerEvent&) override
    {
        if (onClicked)
            onClicked();
    }
};

PathBar::PathBar()
{
    setName("pathBar");
    // Clicks open the editor or a folder; drags move the window.
    setMovesWindow(true);
    m_crumbs = addChild<Breadcrumbs>(true);
    m_crumbs->setMovesWindow(true);
    m_crumbs->onItemClicked = [this](const std::string& path) {
        if (onItemClicked)
            onItemClicked(path);
    };
    m_crumbs->onOpenInNewTab = [this](const std::string& path) {
        if (onOpenInNewTab)
            onOpenInNewTab(path);
    };
    m_crumbs->onOpenInNewWindow = [this](const std::string& path) {
        if (onOpenInNewWindow)
            onOpenInNewWindow(path);
    };
    m_crumbs->onEmptyClicked = [this] { openEditor(); };
    // A window opened on a new mount can come before the mount's own listing does.
    m_mountsChanged = MountsClient::instance().mountsChanged.connect([this] {
        if (!m_url.empty())
            updateUrl(m_url);
    });

    auto* editor = addChild<PathEditor>(this);
    m_editor = editor;
    m_editor->setName("pathEditor");
    m_editor->setFrameVisible(false);
    m_editor->setPlaceholder(tr("Search or Enter Path"));
    m_editor->setSelectByMouse(true);
    m_editor->setVisible(false);
    auto* clear = addChild<ClearButton>();
    m_clear = clear;
    clear->onClicked = [this] {
        endSearch();
        if (onFinished)
            onFinished();
    };

    m_debounce.setSingleShot(true);
    m_debounce.setInterval(std::chrono::milliseconds(150));
    m_debounce.onTimeout = [this] {
        if (m_model)
            m_model->setSearchText(m_editor->text());
    };

    m_editor->onTextEdited = [this] {
        if (isPath(m_editor->text())) {
            m_debounce.stop();
            if (searching())
                m_model->setSearchText({});
        } else {
            m_debounce.start();
        }
        if (m_completion)
            m_completion->edited(isPath(m_editor->text()));
    };
    m_editor->onTextChanged = [this] {
        if (m_completion)
            m_completion->textChanged();
        layout();
    };
    m_editor->onAccepted = [this] {
        m_debounce.stop();
        const std::string text = m_editor->text();
        if (isPath(text)) {
            if (onEditorAccepted)
                onEditorAccepted(text);
            closeEditor();
        } else if (m_model) {
            m_model->setSearchText(text);
        }
        if (onFinished)
            onFinished();
    };

    m_completion = std::make_unique<CompletionPopup>(m_editor);
    m_completion->onActivated = [this](const std::string& path) {
        if (onEditorAccepted)
            onEditorAccepted(path);
        closeEditor();
    };
}

PathBar::~PathBar() = default;

bool PathBar::isPath(const std::string& text)
{
    // File names cannot hold a slash, so text with one is a path to go to.
    static const std::regex scheme("^(trash|file|computer):");
    return text.find('/') != std::string::npos || text.starts_with('~') || std::regex_search(text, scheme);
}

bool PathBar::searching() const
{
    return m_model && !m_model->searchText().empty();
}

bool PathBar::isEditing() const
{
    return m_editing;
}

void PathBar::setFolderModel(FolderModel* model)
{
    m_model = model;
    m_searchChanged.reset();
    if (model) {
        m_searchChanged = model->searchChanged.connect([this] {
            // Ended from elsewhere: Back, or a folder opened from the results.
            if (!m_debounce.isActive() && !m_editor->hasFocus())
                m_editor->setText(m_model->searchText());
            updateVisibility();
        });
    }
    updateVisibility();
}

void PathBar::updateUrl(const std::string& url)
{
    std::string resolved;
    std::vector<Crumb> crumbs = pathCrumbs(url, &resolved);
    m_url = resolved;
    m_crumbs->setCrumbs(std::move(crumbs));
}

void PathBar::retranslate()
{
    m_editor->setPlaceholder(tr("Search or Enter Path"));
}

void PathBar::updateVisibility()
{
    const bool editorShown = m_editing || searching();
    m_editor->setVisible(editorShown);
    m_crumbs->setVisible(!editorShown);
    m_clear->setVisible(editorShown && searching());
    requestLayout();
    requestPaint();
}

void PathBar::openEditor()
{
    m_debounce.stop();
    m_completion->reset();
    m_editor->setText(m_url);
    m_editing = true;
    updateVisibility();
    m_editor->setFocus();
    m_editor->selectAll();
}

void PathBar::openSearch()
{
    m_debounce.stop();
    m_completion->reset();
    m_editor->setText(searching() ? m_model->searchText() : std::string());
    m_editing = true;
    updateVisibility();
    m_editor->setFocus();
    m_editor->selectAll();
}

void PathBar::endSearch()
{
    m_debounce.stop();
    m_editing = false;
    m_editor->setText({});
    if (searching())
        m_model->setSearchText({});
    updateVisibility();
}

// Shows the current tab's search, if it has one, without the keys.
void PathBar::closeEditor()
{
    m_debounce.stop();
    m_editing = false;
    m_editor->setText(searching() ? m_model->searchText() : std::string());
    m_completion->reset();
    updateVisibility();
}

void PathBar::editorFocusChanged(bool focused)
{
    if (!focused && m_editing) {
        m_editing = false;
        updateVisibility();
    }
    if (!focused)
        m_completion->hide();
}

bool PathBar::editorKey(const ui::KeyEvent& event)
{
    if (m_completion->handleKey(event, isPath(m_editor->text())))
        return true;
    if (event.key == ui::key::Escape) {
        endSearch();
        if (onFinished)
            onFinished();
        return true;
    }
    return false;
}

void PathBar::layout()
{
    const ui::Theme& theme = ui::Theme::current();
    const ui::RectF rect = localRect();
    m_crumbs->setGeometry(rect);

    // Lower than the header buttons, centred on them.
    const double height = 28;
    m_editor->setGeometry({0, centeredStart(rect.height, height), rect.width, height});
    m_editor->setTextColor(ui::color(FmColor::PathEditorText));
    const double clearX = rect.width - kSmallSpacing * 1.5 - 16;
    m_clear->setGeometry({clearX, centeredStart(rect.height, 16), 16, 16});
    static_cast<ui::Icon*>(m_clear)->setColor(theme.disabledText);
    m_editor->setTextMargins(kLargeSpacing, m_clear->isVisible() ? rect.width - clearX + kSmallSpacing : kLargeSpacing);
}

void PathBar::paint(ui::Painter& painter)
{
    if (!m_editor->isVisible())
        return;
    const ui::Theme& theme = ui::Theme::current();
    const ui::RectF field = m_editor->geometry();
    const ui::Color background = ui::color(FmColor::PathEditorBackground);
    // A search left showing keeps a faint outline, so it still reads as a field.
    const ui::Color border = m_editor->hasFocus() ? theme.highlight : style::textAlpha(0.15);
    painter.drawRoundedRect(field, {theme.smallRadius, 0, ui::Corners::Circular}, background, 1, border);
}

// Clicking anywhere that is not a crumb opens the editor.
void PathBar::pointerPress(const ui::PointerEvent& event)
{
    m_dragged = false;
    m_press = event.position;
}

void PathBar::pointerMove(const ui::PointerEvent& event)
{
    if (std::abs(event.position.x - m_press.x) > 4 || std::abs(event.position.y - m_press.y) > 4)
        m_dragged = true;
}

void PathBar::pointerRelease(const ui::PointerEvent& event)
{
    if (!m_dragged && event.button == ui::MouseButton::Left)
        openEditor();
}

} // namespace fm
