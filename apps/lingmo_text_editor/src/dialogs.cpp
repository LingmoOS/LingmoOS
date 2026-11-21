// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs.h"
#include "application.h"
#include "editorpage.h"
#include "editorsettings.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/process.h"
#include "ui/application.h"
#include "ui/check_box.h"
#include "ui/color_provider.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/scroll_view.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/window_header.h"

#include <algorithm>
#include <cmath>
#include <map>

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;

// Destroys a window from the event loop, after the event being handled.
static void releaseLater(std::unique_ptr<Window> &window)
{
    if (!window)
        return;
    std::shared_ptr<Window> doomed(std::move(window));
    Lingmo::ui::Application::instance()->loop().post([doomed] {});
}

// One device pixel at any scale, snapped as a RoundedRectangle.
static void hairline(Painter &painter, double x, double y, double width, Color color)
{
    painter.drawRoundedRect({x, y, width, 1 / painter.scale()}, {0, 0, Corners::Circular}, color);
}

class SaveChangesDialog::Content : public View
{
public:
    Content(SaveChangesDialog &dialog, const std::string &title, const std::vector<EditorPage *> &pages)
        : m_column(addChild<Column>())
        , m_header(addChild<WindowHeader>(44))
    {
        m_column->setMargins({16, 16, 16, 12});
        m_column->setSpacing(16);

        auto *texts = m_column->addChild<Column>();
        texts->setSpacing(4);
        texts->setLayoutHints({.fillWidth = true});
        // Clear of the close button in the window's corner.
        texts->setMargins({0, 0, 32, 0});
        Label *titleLabel = texts->addChild<Label>(title);
        titleLabel->setFontSize(14);
        titleLabel->setWeight(600);
        titleLabel->setWrap(true);
        titleLabel->setLayoutHints({.fillWidth = true});
        Label *text = texts->addChild<Label>(tr("If you don't save, your changes will be lost."));
        text->setFontSize(14);
        text->setColor(ColorId::DisabledText);
        text->setWrap(true);
        text->setLayoutHints({.fillWidth = true});

        if (pages.size() > 1) {
            auto *boxes = m_column->addChild<Column>();
            boxes->setSpacing(4);
            boxes->setLayoutHints({.fillWidth = true});
            for (EditorPage *page : pages) {
                const std::string folder = page->document().folder();
                CheckBox *box = boxes->addChild<CheckBox>(page->title() + (folder.empty() ? "" : "  \xE2\x80\x94  " + folder));
                box->setChecked(std::find(dialog.m_checked.begin(), dialog.m_checked.end(), page) != dialog.m_checked.end());
                box->onToggled = [&dialog, page](bool checked) {
                    std::erase(dialog.m_checked, page);
                    if (checked)
                        dialog.m_checked.push_back(page);
                    if (Content *content = dynamic_cast<Content *>(dialog.m_window ? dialog.m_window->contentView() : nullptr))
                        content->m_save->setEnabled(!dialog.m_checked.empty());
                };
            }
        }

        auto *actions = m_column->addChild<Row>();
        actions->setSpacing(8);
        actions->setLayoutHints({.alignment = Alignment::End});
        DialogButton *cancel = actions->addChild<DialogButton>(tr("Cancel"));
        cancel->onClicked = [&dialog] { dialog.finish(false); };
        DialogButton *discard = actions->addChild<DialogButton>(pages.size() > 1 ? tr("Discard All") : tr("Don't Save"));
        discard->setDestructive(true);
        discard->onClicked = [&dialog] {
            for (EditorPage *page : dialog.m_pages) {
                if (!dialog.lifetimeOf(page).expired())
                    page->document().removeDraft();
            }
            dialog.finish(true);
        };
        m_save = actions->addChild<DialogButton>(tr("Save"), true);
        m_save->onClicked = [&dialog] { dialog.saveChecked(); };
        m_save->setEnabled(!dialog.m_checked.empty());

        m_header->setButtonMargin(8);
        m_header->setMinimizeVisible(false);
        m_header->setMaximizeVisible(false);
    }

    DialogButton *saveButton() const { return m_save; }

    double heightForWidth(double width) const override { return m_column->heightForWidth(width); }

protected:
    void layout() override
    {
        m_column->setGeometry(localRect());
        m_header->setGeometry({0, 0, geometry().width, 44});
    }

private:
    Column *m_column;
    WindowHeader *m_header;
    DialogButton *m_save = nullptr;
};

SaveChangesDialog::SaveChangesDialog(Window *parent)
    : m_parent(parent)
{
}

SaveChangesDialog::~SaveChangesDialog() = default;

void SaveChangesDialog::ask(const std::vector<EditorPage *> &pages, std::function<void()> then)
{
    release();
    m_pages = pages;
    m_lifetimes.clear();
    for (EditorPage *page : pages)
        m_lifetimes.push_back(page->lifetime());
    m_checked = pages;
    m_action = std::move(then);

    m_window = std::make_unique<Window>();
    m_window->setBackgroundColor(ColorId::SecondBackground);
    m_window->setTransientParent(m_parent, true);
    m_window->onCloseRequested = [this] { finish(false); };
    buildContent();
    m_window->addShortcut(NoModifier, key::Escape, [this] { finish(false); });
    // Return saves, as the highlighted button says.
    for (const uint32_t enter : {key::Return, key::KeypadEnter}) {
        m_window->addShortcut(NoModifier, enter, [this] {
            if (!m_checked.empty())
                saveChecked();
        });
    }
    m_window->resize({380, 0});
    m_window->setMinimumSize({380, 0});
    m_window->setMaximumSize({380, 16777215});
    m_window->setFitsContentHeight(true);
    m_window->show();
}

void SaveChangesDialog::buildContent()
{
    const std::string title = m_pages.size() == 1
            ? format(tr("Save changes to \xE2\x80\x9C{name}\xE2\x80\x9D?"), {{"name", m_pages.front()->title()}})
            : trn("Save changes to {n} document?", "Save changes to {n} documents?", m_pages.size());
    m_window->setTitle(title);
    auto content = std::make_unique<Content>(*this, title, m_pages);
    DialogButton *save = content->saveButton();
    m_window->setContentView(std::move(content));
    save->setFocus();
}

void SaveChangesDialog::retranslate()
{
    // Hidden while the checked pages are saved, when the question is answered.
    if (!m_window || m_window->isHidden())
        return;
    // A page closed meanwhile has no title to list; the question stays as it is.
    for (const std::weak_ptr<bool> &lifetime : m_lifetimes) {
        if (lifetime.expired())
            return;
    }
    buildContent();
}

std::weak_ptr<bool> SaveChangesDialog::lifetimeOf(const EditorPage *page) const
{
    const auto it = std::find(m_pages.begin(), m_pages.end(), page);
    return it == m_pages.end() ? std::weak_ptr<bool>() : m_lifetimes[size_t(it - m_pages.begin())];
}

void SaveChangesDialog::release()
{
    releaseLater(m_window);
}

void SaveChangesDialog::finish(bool proceed)
{
    std::function<void()> then = std::move(m_action);
    m_action = nullptr;
    release();
    if (proceed && then)
        then();
}

// Saves the checked documents one after another; a cancelled Save As stops it.
void SaveChangesDialog::saveChecked()
{
    // Another window may close one of the pages while they are saved.
    using TrackedPage = std::pair<EditorPage *, std::weak_ptr<bool>>;
    auto queue = std::make_shared<std::vector<TrackedPage>>();
    for (EditorPage *page : m_checked)
        queue->emplace_back(page, lifetimeOf(page));
    std::vector<TrackedPage> unchecked;
    for (EditorPage *page : m_pages) {
        if (std::find(m_checked.begin(), m_checked.end(), page) == m_checked.end())
            unchecked.emplace_back(page, lifetimeOf(page));
    }
    if (m_window)
        m_window->hide();
    // Only the save waiting to finish holds the chain; it holds itself weakly.
    auto next = std::make_shared<std::function<void()>>();
    const std::weak_ptr<std::function<void()>> self = next;
    *next = [this, queue, unchecked, self, alive = std::weak_ptr<bool>(m_alive)] {
        if (alive.expired())
            return;
        if (queue->empty()) {
            for (const auto &[page, lifetime] : unchecked) {
                if (!lifetime.expired())
                    page->document().removeDraft();
            }
            finish(true);
            return;
        }
        const auto [page, lifetime] = queue->front();
        queue->erase(queue->begin());
        if (!onSaveRequested) {
            finish(false);
            return;
        }
        if (lifetime.expired()) {
            if (const auto again = self.lock())
                (*again)();
            return;
        }
        onSaveRequested(page, [next = self.lock()](bool saved) {
            if (saved)
                (*next)();
        });
    };
    (*next)();
}

static constexpr double rowHeight = 44;
static constexpr double rowInset = 14;

// A label on the left, its control on the right, a hairline below unless last.
class SettingRow : public View
{
public:
    SettingRow(std::string text, bool last)
        : m_label(addChild<Label>(std::move(text)))
        , m_last(last)
    {
    }

    template<typename T, typename... Args>
    T *setControl(double width, Args &&...args)
    {
        T *control = addChild<T>(std::forward<Args>(args)...);
        m_control = control;
        m_controlWidth = width;
        return control;
    }

    SizeF sizeHint() const override { return {0, rowHeight}; }

protected:
    void layout() override
    {
        const SizeF label = m_label->sizeHint();
        m_label->setGeometry({rowInset, rounding::centeredStart(rowHeight, label.height), label.width, label.height});
        if (m_control) {
            const SizeF size = m_control->sizeHint();
            const double width = m_controlWidth > 0 ? m_controlWidth : size.width;
            // At the right edge, centred vertically.
            m_control->setGeometry({geometry().width - rowInset - width, rounding::centeredStart(rowHeight, size.height), width,
                                    size.height});
        }
    }

    void paint(Painter &painter) override
    {
        if (!m_last)
            hairline(painter, rowInset, rowHeight - 1 / painter.scale(), geometry().width - 2 * rowInset,
                     Theme::current().text.withAlpha(0.08));
    }

private:
    Label *m_label;
    View *m_control = nullptr;
    double m_controlWidth = 0;
    bool m_last;
};

// A rounded card in the second background with rows in a column.
class Card : public View
{
public:
    void addRow(View *row) { m_rows.push_back(row); }

    double heightForWidth(double width) const override
    {
        double height = 0;
        for (const View *row : m_rows)
            height += row->heightForWidth(width);
        return height;
    }
    SizeF sizeHint() const override { return {0, heightForWidth(0)}; }

protected:
    void layout() override
    {
        double y = 0;
        for (View *row : m_rows) {
            const double height = row->heightForWidth(geometry().width);
            row->setGeometry({0, y, geometry().width, height});
            y += height;
        }
    }

    void paint(Painter &painter) override
    {
        const Theme &theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.bigRadius, 0, themeCorners()}, theme.secondBackground);
    }

private:
    std::vector<View *> m_rows;
};

class PreferencesDialog::Content : public View
{
public:
    explicit Content(EditorApplication &application)
        : m_application(application)
        , m_settings(application.settings())
    {
        m_families = {tr("System Monospace")};
        for (const std::string &family : application.monospaceFamilies())
            m_families.push_back(family);

        Card *fonts = addCard();
        ComboBox *font = addRow(fonts, tr("Font"), false)->setControl<ComboBox>(200, m_families);
        const auto familyAt = std::find(m_families.begin() + 1, m_families.end(), m_settings.fontFamily);
        font->setCurrentIndex(m_settings.fontFamily.empty() || familyAt == m_families.end() ? 0 : int(familyAt - m_families.begin()));
        font->onActivated = [this](int index) {
            m_settings.fontFamily = index == 0 ? std::string() : m_families[size_t(index)];
            m_settings.commit();
        };
        std::vector<std::string> sizeNames;
        for (const int size : sizes)
            // TRANSLATORS: the system's default font size, first in a list of sizes in points.
            sizeNames.push_back(size == 0 ? tr("System") : format(tr("{size} pt"), {{"size", std::to_string(size)}}));
        ComboBox *size = addRow(fonts, tr("Size"), true)->setControl<ComboBox>(200, sizeNames);
        const auto sizeAt = std::find(std::begin(sizes), std::end(sizes), int(std::lround(m_settings.fontSize)));
        size->setCurrentIndex(sizeAt == std::end(sizes) || m_settings.fontSize != double(*sizeAt) ? 0 : int(sizeAt - std::begin(sizes)));
        size->onActivated = [this](int index) {
            m_settings.fontSize = sizes[index];
            m_settings.commit();
        };

        Card *indentation = addCard();
        std::vector<std::string> widthNames;
        for (const int width : tabWidths)
            widthNames.push_back(std::to_string(width));
        ComboBox *tabWidth = addRow(indentation, tr("Tab Width"), false)->setControl<ComboBox>(200, widthNames);
        const auto widthAt = std::find(std::begin(tabWidths), std::end(tabWidths), m_settings.tabWidth);
        tabWidth->setCurrentIndex(widthAt == std::end(tabWidths) ? 0 : int(widthAt - std::begin(tabWidths)));
        tabWidth->onActivated = [this](int index) {
            m_settings.tabWidth = tabWidths[index];
            m_settings.commit();
        };
        addSwitch(indentation, tr("Insert Spaces Instead of Tabs"), &EditorSettings::insertSpaces, false);
        addSwitch(indentation, tr("Automatic Indentation"), &EditorSettings::autoIndent, true);

        Card *view = addCard();
        addSwitch(view, tr("Wrap Lines"), &EditorSettings::wordWrap, false);
        addSwitch(view, tr("Line Numbers"), &EditorSettings::lineNumbers, false);
        addSwitch(view, tr("Highlight Current Line"), &EditorSettings::highlightCurrentLine, false);
        addSwitch(view, tr("Show Whitespace"), &EditorSettings::showWhitespace, false);
        addSwitch(view, tr("Match Brackets"), &EditorSettings::matchBrackets, true);

        Card *session = addCard();
        addSwitch(session, tr("Restore Session on Start"), &EditorSettings::restoreSession, true);

        m_connection = m_settings.changed.connect([this] { sync(); });
    }

    double heightForWidth(double width) const override
    {
        double height = 0;
        for (const Card *card : m_cards)
            height += card->heightForWidth(width);
        return height + Theme::current().largeSpacing * double(m_cards.size() - 1);
    }

protected:
    void layout() override
    {
        double y = 0;
        for (Card *card : m_cards) {
            const double height = card->heightForWidth(geometry().width);
            card->setGeometry({0, y, geometry().width, height});
            y += height + Theme::current().largeSpacing;
        }
    }

private:
    static constexpr int sizes[] = {0, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24};
    static constexpr int tabWidths[] = {2, 3, 4, 8};

    Card *addCard()
    {
        Card *card = addChild<Card>();
        m_cards.push_back(card);
        return card;
    }

    SettingRow *addRow(Card *card, std::string text, bool last)
    {
        SettingRow *row = card->addChild<SettingRow>(std::move(text), last);
        card->addRow(row);
        return row;
    }

    void addSwitch(Card *card, std::string text, bool EditorSettings::*member, bool last)
    {
        Switch *toggle = addRow(card, std::move(text), last)->setControl<Switch>(0);
        toggle->setChecked(m_settings.*member);
        toggle->onToggled = [this, member](bool checked) {
            m_settings.*member = checked;
            m_settings.commit();
        };
        m_switches.emplace_back(toggle, member);
    }

    void sync()
    {
        for (const auto &[toggle, member] : m_switches)
            toggle->setChecked(m_settings.*member);
    }

    EditorApplication &m_application;
    EditorSettings &m_settings;
    std::vector<std::string> m_families;
    std::vector<Card *> m_cards;
    std::vector<std::pair<Switch *, bool EditorSettings::*>> m_switches;
    Lingmo::base::Subscription m_connection;
};

PreferencesDialog::PreferencesDialog(Window *parent, EditorApplication &application)
    : m_parent(parent)
    , m_application(application)
{
}

PreferencesDialog::~PreferencesDialog() = default;

Window *PreferencesDialog::window() const
{
    return m_dialog ? m_dialog->window() : nullptr;
}

void PreferencesDialog::show()
{
    if (m_dialog && m_dialog->window()) {
        m_dialog->window()->activate();
        return;
    }
    m_dialog = std::make_unique<DialogWindow>(m_parent, tr("Settings"), 460);
    m_dialog->setContent<Content>(m_application);
    m_dialog->window()->setBackgroundColor(ColorId::Background);
    m_dialog->show();
}

// The shortcuts in their native text: modifiers first, joined by "+".
struct ShortcutRow
{
    const char *text;
    const char *keys;
};

struct ShortcutSection
{
    const char *title;
    std::vector<ShortcutRow> rows;
};

static const std::vector<ShortcutSection> &shortcutSections()
{
    static const std::vector<ShortcutSection> sections{
        {trNoop("Files and Tabs"),
         {{trNoop("New tab"), "Ctrl+T"},
          {trNoop("New window"), "Ctrl+N"},
          {trNoop("Open a file"), "Ctrl+O"},
          {trNoop("Save, save as"), "Ctrl+S, Ctrl+Shift+S"},
          {trNoop("Save all"), "Ctrl+Alt+S"},
          {trNoop("Close the tab, the window"), "Ctrl+W, Ctrl+Shift+W"},
          {trNoop("Reopen the closed tab"), "Ctrl+Shift+T"},
          {trNoop("Next or previous tab"), "Ctrl+PgDown, Ctrl+PgUp"},
          {trNoop("Move the tab"), "Ctrl+Shift+PgDown, Ctrl+Shift+PgUp"},
          {trNoop("Go to tab 1 to 9"), "Alt+1, Alt+9"},
          {trNoop("Quit"), "Ctrl+Q"}}},
        {trNoop("Editing"),
         {{trNoop("Undo, redo"), "Ctrl+Z, Ctrl+Shift+Z"},
          {trNoop("Cut, copy, paste"), "Ctrl+X, Ctrl+C, Ctrl+V"},
          {trNoop("Delete the word before or after"), "Ctrl+Backspace, Ctrl+Del"},
          {trNoop("Indent or unindent"), "Tab, Shift+Tab"},
          {trNoop("Move the line up or down"), "Alt+Up, Alt+Down"},
          {trNoop("Duplicate the line"), "Ctrl+Alt+D"},
          {trNoop("Delete the line"), "Ctrl+D"},
          {trNoop("Uppercase, lowercase"), "Ctrl+U, Ctrl+Shift+U"},
          {trNoop("Overwrite mode"), "Ins"}}},
        {trNoop("Find and Move"),
         {{trNoop("Find, replace"), "Ctrl+F, Ctrl+H"},
          {trNoop("Next or previous match"), "F3, Shift+F3"},
          {trNoop("Go to line"), "Ctrl+I, Ctrl+L"},
          {trNoop("Word left or right"), "Ctrl+Left, Ctrl+Right"},
          {trNoop("Start or end of the document"), "Ctrl+Home, Ctrl+End"}}},
        {trNoop("View"),
         {{trNoop("Zoom in, out, reset"), "Ctrl+=, Ctrl+-, Ctrl+0"},
          {trNoop("Word wrap"), "Alt+W"},
          {trNoop("Full screen"), "F11"},
          {trNoop("Settings"), "Ctrl+,"},
          {trNoop("Keyboard shortcuts"), "Ctrl+?"}}},
    };
    return sections;
}

// One read-only row of a card: the label, then its value in secondary text.
class InfoRow : public View
{
public:
    InfoRow(std::string text, std::string value, bool last)
        : m_text(addChild<Label>(std::move(text)))
        , m_value(addChild<Label>(std::move(value)))
        , m_last(last)
    {
        m_value->setColor(ColorId::DisabledText);
        m_value->setTextAlignment(Label::TextAlignment::Right);
    }

    double heightForWidth(double) const override { return 44; }
    SizeF sizeHint() const override { return {0, 44}; }

protected:
    void layout() override
    {
        // A row, margins 16, spacing 12: the label fills, the value is at most 65% wide.
        const double width = geometry().width;
        const SizeF value = m_value->sizeHint();
        const double valueWidth = std::min(std::ceil(value.width), std::round(width * 0.65));
        const double valueX = width - 16 - valueWidth;
        const SizeF text = m_text->sizeHint();
        m_text->setGeometry({16, std::round((44 - std::ceil(text.height)) / 2), std::max(0.0, valueX - 12 - 16),
                             std::ceil(text.height)});
        m_value->setGeometry({valueX, std::round((44 - std::ceil(value.height)) / 2), valueWidth, std::ceil(value.height)});
    }

    void paint(Painter &painter) override
    {
        if (m_last)
            return;
        hairline(painter, 16, 44 - 1 / painter.scale(), geometry().width - 32, color(ColorId::Separator));
    }

private:
    Label *m_text;
    Label *m_value;
    bool m_last;
};

class ShortcutsDialog::Content : public View
{
public:
    Content()
        : m_scroll(addChild<ScrollView>())
    {
        m_scroll->setElastic(false);
        m_list = m_scroll->setContent<List>();
    }

    double heightForWidth(double) const override { return 600 - 44 - 16; }

protected:
    void layout() override
    {
        // The scroll area spans the window under the title, past the dialog's padding.
        m_scroll->setGeometry({-16, 0, geometry().width + 32, geometry().height + 16});
    }

private:
    class List : public View
    {
    public:
        List()
        {
            for (const ShortcutSection &section : shortcutSections()) {
                Label *title = addChild<Label>(tr(section.title));
                title->setColor(ColorId::DisabledText);
                title->setElide(Label::Elide::Right);
                Card *card = addChild<Card>();
                for (size_t i = 0; i < section.rows.size(); ++i) {
                    InfoRow *row = card->addChild<InfoRow>(tr(section.rows[i].text), section.rows[i].keys,
                                                           i + 1 == section.rows.size());
                    card->addRow(row);
                }
                m_sections.push_back({title, card});
            }
        }

        double heightForWidth(double width) const override
        {
            // A column at x 16, y 4, 32 narrower, spacing 12 between sections
            // and 8 inside them; 20 more below.
            double height = 0;
            for (size_t i = 0; i < m_sections.size(); ++i) {
                height += std::ceil(m_sections[i].first->sizeHint().height) + 8
                        + m_sections[i].second->heightForWidth(width - 32);
                if (i + 1 < m_sections.size())
                    height += 12;
            }
            return height + 20;
        }

    protected:
        void layout() override
        {
            const double width = geometry().width - 32;
            double y = 4;
            for (const auto &[title, card] : m_sections) {
                const double titleHeight = std::ceil(title->sizeHint().height);
                title->setGeometry({16 + 16, y, std::max(0.0, width - 32), titleHeight});
                y += titleHeight + 8;
                const double height = card->heightForWidth(width);
                card->setGeometry({16, y, width, height});
                y += height + 12;
            }
        }

    private:
        std::vector<std::pair<Label *, Card *>> m_sections;
    };

    ScrollView *m_scroll;
    List *m_list;
};

ShortcutsDialog::ShortcutsDialog(Window *parent)
    : m_parent(parent)
{
}

ShortcutsDialog::~ShortcutsDialog() = default;

Window *ShortcutsDialog::window() const
{
    return m_dialog ? m_dialog->window() : nullptr;
}

void ShortcutsDialog::show()
{
    if (m_dialog && m_dialog->window()) {
        m_dialog->window()->activate();
        return;
    }
    m_dialog = std::make_unique<DialogWindow>(m_parent, tr("Keyboard Shortcuts"), 480);
    m_dialog->setContent<Content>();
    m_dialog->window()->setBackgroundColor(ColorId::Background);
    DialogWindow *dialog = m_dialog.get();
    m_dialog->window()->addShortcut(NoModifier, key::Escape, [dialog] { dialog->close(); });
    m_dialog->show();
}

static constexpr const char *projectLink = "https://github.com/LingmoOS/text-editor";

class AboutDialog::Content : public View
{
public:
    Content()
        : m_header(addChild<WindowHeader>(40))
        , m_icon(addChild<ThemeIcon>("accessories-text-editor", 64))
        , m_name(addChild<Label>(tr("Text Editor")))
        , m_description(addChild<Label>(tr("A simple editor for plain text")))
        , m_link(addChild<Label>())
    {
        m_header->setMinimizeVisible(false);
        m_header->setMaximizeVisible(false);
        // 14 points.
        m_name->setFontSize(14.0 * 4 / 3);
        m_link->setSpans({{projectLink, 0, Theme::current().highlight, false, true, projectLink}});
        m_link->setCursorShape(CursorShape::Pointer);
    }

    double heightForWidth(double) const override { return 40 + columnHeight() + 40; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double width = geometry().width;
        m_header->setGeometry({0, 0, width, 40});
        // A column under the header, spacing 5: icon, a 12 high gap, the labels.
        double y = 40;
        m_icon->setGeometry({rounding::centeredStart(width, 64), y, 64, 64});
        y += 64 + 5 + 12 + 5;
        for (Label *label : {m_name, m_description, m_link}) {
            const SizeF size = label->sizeHint();
            label->setGeometry({std::round((width - std::ceil(size.width)) / 2), y, std::ceil(size.width),
                                std::ceil(size.height)});
            y += std::ceil(size.height) + 5;
        }
    }

    void pointerRelease(const PointerEvent &event) override
    {
        if (m_link->geometry().contains(event.position))
            Lingmo::base::Process::startDetached("xdg-open", {projectLink}, {});
    }

private:
    double columnHeight() const
    {
        return 64 + 5 + 12 + 5 + std::ceil(m_name->sizeHint().height) + 5 + std::ceil(m_description->sizeHint().height)
                + 5 + std::ceil(m_link->sizeHint().height);
    }

    WindowHeader *m_header;
    ThemeIcon *m_icon;
    Label *m_name;
    Label *m_description;
    Label *m_link;
};

AboutDialog::AboutDialog(Window *parent)
    : m_parent(parent)
{
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::show()
{
    if (m_window) {
        m_window->activate();
        return;
    }
    m_window = std::make_unique<Window>();
    m_window->setTitle(tr("Text Editor"));
    m_window->setTransientParent(m_parent, true);
    auto content = std::make_unique<Content>();
    const double height = content->heightForWidth(300);
    m_window->setContentView(std::move(content));
    m_window->resize({300, height});
    m_window->setMinimumSize({300, height});
    m_window->setMaximumSize({300, height});
    m_window->onCloseRequested = [this] { close(); };
    m_window->show();
}

void AboutDialog::close()
{
    if (!m_window)
        return;
    m_window->hide();
    releaseLater(m_window);
}
