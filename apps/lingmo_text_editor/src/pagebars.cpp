// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pagebars.h"
#include "application.h"
#include "editorpage.h"
#include "editorsettings.h"
#include "editorview.h"
#include "editorwindow.h"
#include "regexp.h"
#include "textcodec.h"

#include "base/i18n.h"
#include "base/strings.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/progress_bar.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <cmath>

using namespace Lingmo::ui;
using namespace std::chrono_literals;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;
namespace strings = Lingmo::base::strings;

static const char *const lineEndingShortNames[] = {"LF", "CRLF", "CR"};

bool CardField::keyPress(const KeyEvent &event)
{
    if ((event.key == key::Return || event.key == key::KeypadEnter) && onReturn) {
        onReturn(event.modifiers & ShiftModifier);
        return true;
    }
    if (event.key == key::Escape && onEscape) {
        onEscape();
        return true;
    }
    return TextField::keyPress(event);
}

// Vertically centred in a row, at whole-pixel positions.
static double centreIn(double extent, double item)
{
    return std::round((extent - item) / 2);
}

StatusBar::StatusBar(EditorPage &page)
    : m_page(page)
{
    setName("statusBar");
    m_position = addItem("positionItem");
    m_zoom = addItem("zoomItem");
    m_overwrite = addItem("overwriteItem");
    m_readOnly = addItem("readOnlyItem");
    m_indentation = addItem("indentationItem");
    m_language = addItem("languageItem");
    m_lineEnding = addItem("lineEndingItem");
    m_encoding = addItem("encodingItem");

    m_position->onClicked = [this] { m_page.showGoToLine(); };
    m_zoom->onClicked = [this] {
        if (EditorWindow *window = m_page.editorWindow())
            window->zoomBy(0);
    };
    m_overwrite->onClicked = [this] { m_page.editor().setOverwriteMode(false); };
    m_readOnly->onClicked = [this] { m_page.document().setLocked(false); };
    m_indentation->onClickedAt = [this](PointF position) { openIndentMenu(position); };
    m_language->onClickedAt = [this](PointF position) { openLanguageMenu(position); };
    m_lineEnding->onClickedAt = [this](PointF position) { openLineEndingMenu(position); };
    m_encoding->onClickedAt = [this](PointF position) { openEncodingMenu(position); };
    update();
}

StatusBar::~StatusBar() = default;

RoundButton *StatusBar::addItem(const std::string &name)
{
    // A status item: plain text with a hover plate, opening its menu.
    RoundButton *item = addChild<RoundButton>(22);
    item->setName(name);
    Label *label = item->setLabel({}, 8);
    label->setFontSize(12);
    m_items.push_back(item);
    return item;
}

void StatusBar::update()
{
    const Document &document = m_page.document();
    // TRANSLATORS: the cursor position in the status bar; "Ln" and "Col" abbreviate line and column.
    std::string position = format(tr("Ln {line}, Col {column}"), {{"line", std::to_string(document.cursorLine() + 1)},
                                                                  {"column", std::to_string(document.visualColumn())}});
    if (document.hasSelection())
        // TRANSLATORS: {position} is the cursor position, such as "Ln 3, Col 7".
        position = format(trn("{position}  ({n} selected)", "{position}  ({n} selected)", uint64_t(document.selectionLength())),
                          {{"position", position}});
    m_position->label()->setText(position);

    const EditorWindow *window = m_page.editorWindow();
    const double zoom = window ? window->zoom() : 1;
    m_zoom->setVisible(window && zoom != 1);
    m_zoom->label()->setText(std::to_string(int(std::lround(zoom * 100))) + "%");
    m_overwrite->setVisible(m_page.editor().overwriteMode());
    // TRANSLATORS: a short status bar badge shown while typing overwrites the text after the cursor.
    m_overwrite->label()->setText(tr("OVR"));
    m_readOnly->setVisible(document.isLocked());
    m_readOnly->label()->setText(tr("Read-Only"));
    m_indentation->label()->setText(document.insertSpaces()
                                            ? format(tr("Spaces: {width}"), {{"width", std::to_string(document.indentWidth())}})
                                            : format(tr("Tab Width: {width}"), {{"width", std::to_string(document.tabWidth())}}));
    m_language->label()->setText(document.languageName());
    m_lineEnding->label()->setText(lineEndingShortNames[document.lineEnding()]);
    m_encoding->label()->setText(document.encoding() + (document.hasBom() ? " BOM" : ""));
    for (RoundButton *item : m_items)
        item->label()->setColor(ColorId::DisabledText);
    requestLayout();
}

void StatusBar::layout()
{
    // A row filling the bar, margins 8, spacing 2: the position first, a
    // filling spacer, then the rest.
    const double width = geometry().width;
    const double height = geometry().height;
    double x = 8;
    const SizeF first = m_position->sizeHint();
    m_position->setGeometry({x, centreIn(height, first.height), first.width, first.height});
    double right = width - 8;
    for (auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
        RoundButton *item = *it;
        if (item == m_position || !item->isVisible())
            continue;
        const SizeF size = item->sizeHint();
        right -= size.width;
        item->setGeometry({right, centreIn(height, size.height), size.width, size.height});
        right -= 2;
    }
}

void StatusBar::paint(Painter &painter)
{
    const Theme &theme = Theme::current();
    painter.fillRect(localRect(), theme.background);
    drawHairline(painter, 0, 0, geometry().width, theme.text.withAlpha(0.1));
}

void StatusBar::openIndentMenu(PointF position)
{
    Document &document = m_page.document();
    m_menu = std::make_unique<Menu>();
    const size_t spaces = m_menu->addItem(tr("Indent with Spaces"), [&document] { document.setInsertSpaces(true); });
    m_menu->setItemChecked(spaces, document.insertSpaces());
    const size_t tabs = m_menu->addItem(tr("Indent with Tabs"), [&document] { document.setInsertSpaces(false); });
    m_menu->setItemChecked(tabs, !document.insertSpaces());
    m_menu->addSeparator();
    for (const int width : {2, 3, 4, 8}) {
        const size_t item = m_menu->addItem(format(tr("Width: {width}"), {{"width", std::to_string(width)}}), [&document, width] {
            document.setIndentWidth(width);
            document.setTabWidth(width);
        });
        m_menu->setItemChecked(item, (document.insertSpaces() ? document.indentWidth() : document.tabWidth()) == width);
    }
    m_menu->popup(window(), position);
}

void StatusBar::openLanguageMenu(PointF position)
{
    Document &document = m_page.document();
    m_menu = std::make_unique<Menu>();
    for (const auto &[id, name] : m_page.application().languages()) {
        const size_t item = m_menu->addItem(name, [this, &document, id = id] {
            document.setLanguage(id);
            m_page.chosenLanguage = id;
        });
        m_menu->setItemChecked(item, document.language() == id);
    }
    m_menu->popup(window(), position);
}

void StatusBar::openLineEndingMenu(PointF position)
{
    Document &document = m_page.document();
    static const char *const names[] = {trNoop("Unix (LF)"), trNoop("Windows (CRLF)"), trNoop("Classic Mac (CR)")};
    m_menu = std::make_unique<Menu>();
    for (int i = 0; i < 3; ++i) {
        const size_t item = m_menu->addItem(tr(names[i]), [&document, i] { document.setLineEnding(i); });
        m_menu->setItemChecked(item, document.lineEnding() == i);
    }
    m_menu->popup(window(), position);
}

void StatusBar::openEncodingMenu(PointF position)
{
    Document &document = m_page.document();
    m_menu = std::make_unique<Menu>();
    Menu *reopenMenu = m_menu->addSubmenu(tr("Reopen with Encoding"));
    m_menu->setItemEnabled(0, !document.isUntitled());
    Menu *saveMenu = m_menu->addSubmenu(tr("Save with Encoding"));
    for (const EncodingInfo &info : TextCodec::encodings()) {
        const std::string name = info.name;
        const size_t reopenItem = reopenMenu->addItem(tr(info.label), [this, name] { m_page.setEncodingForReload(name); });
        reopenMenu->setItemChecked(reopenItem, document.encoding() == name);
        const size_t saveItem = saveMenu->addItem(tr(info.label), [&document, name] { document.setEncoding(name); });
        saveMenu->setItemChecked(saveItem, document.encoding() == name);
    }
    m_menu->addSeparator();
    const size_t bom = m_menu->addItem(tr("Byte Order Mark"), [&document] { document.setBom(!document.hasBom()); });
    m_menu->setItemEnabled(bom, strings::startsWith(document.encoding(), "UTF-"));
    m_menu->setItemChecked(bom, document.hasBom());
    m_menu->popup(window(), position);
}

InfoBar::InfoBar(EditorPage &page)
    : m_page(page)
    , m_label(addChild<Label>())
{
    setName("infoBar");
    m_label->setWrap(true);
    update();
}

bool InfoBar::warning() const
{
    return m_notice != Document::Restored && m_notice != Document::LargeFile && m_notice != Document::MixedLineEndings;
}

void InfoBar::update()
{
    Document &document = m_page.document();
    EditorPage &page = m_page;
    m_notice = document.notice();
    const std::string &detail = document.noticeDetail();
    const auto withDetail = [&detail](const char *text, const char *pattern) {
        return detail.empty() ? std::string(text) : format(pattern, {{"error", detail}});
    };

    std::vector<std::pair<std::string, std::function<void()>>> actions;
    const auto closeTab = [&page](bool ask) {
        return [&page, ask] {
            if (EditorWindow *window = page.editorWindow())
                window->closeTab(window->indexOfPage(&page), ask);
        };
    };
    const auto saveAs = [&page] {
        if (EditorWindow *window = page.editorWindow())
            window->saveAs(&page);
    };
    switch (m_notice) {
    case Document::Restored:
        m_message = tr("Unsaved changes from your last session were restored.");
        if (!document.isUntitled())
            actions.push_back({tr("Discard Changes"), [&page] { page.revert(); }});
        actions.push_back({tr("OK"), [&document] { document.dismissNotice(); }});
        break;
    case Document::ChangedOnDisk:
        m_message = tr("The file has changed on disk. Saving now would overwrite that change.");
        actions = {{tr("Keep My Version"), [&document] { document.keepLocalVersion(); }},
                   {tr("Reload"), [&document] { document.reload(); }}};
        break;
    case Document::Deleted:
        m_message = tr("The file was deleted or moved. Your text is still here.");
        actions = {{tr("Close"), closeTab(true)}, {tr("Save"), [&page] {
                                                       if (EditorWindow *window = page.editorWindow())
                                                           window->savePage(&page);
                                                   }}};
        break;
    case Document::Malformed:
        m_message = format(tr("Some characters could not be read as {encoding} and show as \xEF\xBF\xBD."),
                           {{"encoding", EditorApplication::encodingLabel(document.encoding())}});
        actions = {{tr("Dismiss"), [&document] { document.dismissNotice(); }},
                   {tr("Choose Encoding\xE2\x80\xA6"), [&page] { page.showEncodingMenu(); }}};
        break;
    case Document::Binary:
        m_message = tr("This file does not look like text. Editing it could damage it.");
        actions = {{tr("Close"), closeTab(false)}, {tr("Edit Anyway"), [&document] { document.setLocked(false); }}};
        break;
    case Document::ReadOnly:
        m_message = tr("You do not have permission to save this file.");
        actions = {{tr("Dismiss"), [&document] { document.dismissNotice(); }}, {tr("Save As\xE2\x80\xA6"), saveAs}};
        break;
    case Document::PermissionDenied:
        m_message = tr("You do not have permission to open this file.");
        actions = {{tr("Close"), closeTab(false)}, {tr("Retry"), [&document] { document.reload(); }}};
        break;
    case Document::LoadFailed:
        m_message = withDetail(tr("The file could not be opened."), tr("The file could not be opened. {error}"));
        actions = {{tr("Close"), closeTab(false)}, {tr("Retry"), [&document] { document.reload(); }}};
        break;
    case Document::TooLarge:
        m_message = tr("The file is too large to open (more than 256 MB).");
        actions = {{tr("Close"), closeTab(false)}};
        break;
    case Document::SaveFailed:
        m_message = withDetail(tr("The file could not be saved."), tr("The file could not be saved. {error}"));
        actions = {{tr("Dismiss"), [&document] { document.dismissNotice(); }}, {tr("Save As\xE2\x80\xA6"), saveAs}};
        break;
    case Document::LossyEncoding:
        m_message = format(tr("Some characters cannot be saved as {encoding}. Nothing was saved."),
                           {{"encoding", EditorApplication::encodingLabel(detail)}});
        actions = {{tr("Choose Encoding\xE2\x80\xA6"), [&page] { page.showEncodingMenu(); }},
                   {tr("Save as UTF-8"), [&document] { document.saveAsUtf8(); }}};
        break;
    case Document::LargeFile:
        m_message = tr("Syntax highlighting is off for this large file.");
        actions = {{tr("Dismiss"), [&document] { document.dismissNotice(); }},
                   {tr("Highlight Anyway"), [&document] { document.setLanguage(document.language()); }}};
        break;
    case Document::MalformedSave:
        m_message = format(tr("Saving would replace the characters that could not be read as {encoding}."),
                           {{"encoding", EditorApplication::encodingLabel(detail)}});
        actions = {{tr("Choose Encoding\xE2\x80\xA6"), [&page] { page.showEncodingMenu(); }},
                   {tr("Save Anyway"), [&document] { document.saveReplacingMalformed(); }}};
        break;
    case Document::MixedLineEndings:
        m_message = format(tr("This file mixes line endings. Saving will use {ending} throughout."),
                           {{"ending", lineEndingShortNames[document.lineEnding()]}});
        actions = {{tr("OK"), [&document] { document.dismissNotice(); }}};
        break;
    default:
        m_message.clear();
        break;
    }
    m_label->setText(m_message);

    for (TextButton *button : m_buttons)
        takeChild(button).reset();
    m_buttons.clear();
    m_actions.clear();
    for (size_t i = 0; i < actions.size(); ++i) {
        TextButton *button = addChild<TextButton>(actions[i].first, i + 1 < actions.size());
        // Each action is [label, function]; the last one is the default.
        const size_t index = i;
        button->onClicked = [this, index] {
            // The action may replace this bar's buttons; it runs from a copy.
            const std::function<void()> action = m_actions[index];
            action();
        };
        m_buttons.push_back(button);
        m_actions.push_back(std::move(actions[i].second));
    }
    setVisible(m_notice != Document::NoNotice);
    requestLayout();
    requestPaint();
}

double InfoBar::heightForWidth(double width) const
{
    double buttons = 0;
    double buttonHeight = 0;
    for (const TextButton *button : m_buttons) {
        const SizeF size = button->sizeHint();
        buttons += std::ceil(size.width) + 8;
        buttonHeight = std::max(buttonHeight, std::ceil(size.height));
    }
    const double labelWidth = std::max(0.0, width - 14 - 10 - buttons);
    const double layoutHeight = std::max(buttonHeight, std::ceil(m_label->heightForWidth(labelWidth)));
    return std::max(48.0, layoutHeight + 16);
}

void InfoBar::layout()
{
    // A row inset 14 on the left and 10 on the right, vertically centred, spacing 8.
    const double width = geometry().width;
    const double height = geometry().height;
    double right = width - 10;
    double layoutHeight = 0;
    std::vector<SizeF> sizes;
    for (const TextButton *button : m_buttons) {
        const SizeF size = button->sizeHint();
        sizes.push_back({std::ceil(size.width), std::ceil(size.height)});
        layoutHeight = std::max(layoutHeight, std::ceil(size.height));
    }
    double buttonsWidth = 0;
    for (const SizeF &size : sizes)
        buttonsWidth += size.width + 8;
    const double labelWidth = std::max(0.0, width - 14 - 10 - buttonsWidth);
    const double labelHeight = std::ceil(m_label->heightForWidth(labelWidth));
    layoutHeight = std::max(layoutHeight, labelHeight);
    const double top = rounding::centeredStart(height, layoutHeight);
    m_label->setGeometry({14, top + centreIn(layoutHeight, labelHeight), labelWidth, labelHeight});
    for (size_t i = sizes.size(); i-- > 0;) {
        right -= sizes[i].width;
        m_buttons[i]->setGeometry({right, top + centreIn(layoutHeight, sizes[i].height), sizes[i].width, sizes[i].height});
        right -= 8;
    }
}

void InfoBar::paint(Painter &painter)
{
    const Theme &theme = Theme::current();
    // The tint over the background, mixed by the tint's alpha.
    const Color tint = warning() ? accent::orange.withAlpha(0.14) : theme.highlight.withAlpha(0.1);
    const double a = tint.a / 255.0;
    const auto mix = [a](uint8_t base, uint8_t over) { return uint8_t(std::lround(base * (1 - a) + over * a)); };
    const Color background{mix(theme.background.r, tint.r), mix(theme.background.g, tint.g),
                           mix(theme.background.b, tint.b), theme.background.a};
    painter.fillRect(localRect(), background);
    drawHairline(painter, 0, geometry().height - 1 / painter.scale(), geometry().width, theme.text.withAlpha(0.1));
}

FindBar::FindBar(EditorPage &page)
    : m_page(page)
    , m_disclosure(addChild<RoundButton>(28))
    , m_field(addChild<CardField>())
    , m_count(addChild<Label>())
{
    setName("findBar");
    // Concentric with the fields inside; too small a card for continuous corners.
    setRadius(Theme::current().smallRadius + padding);
    setVisible(false);

    m_disclosure->setName("replaceToggle");
    m_disclosure->setGlyph("chevron-right", 16);
    m_disclosure->onClicked = [this] {
        setReplacing(!m_replacing);
        if (m_replacing)
            m_replaceField->setFocus();
    };

    m_field->setName("findField");
    m_field->onTextEdited = [this] { update(); };
    m_field->onReturn = [this](bool shift) { find(shift); };
    m_field->onEscape = [this] { close(); };

    m_count->setFontSize(12);
    m_caseToggle = addToggle("Aa");
    m_caseToggle->onClicked = [this] {
        EditorSettings &settings = m_page.application().settings();
        settings.findCaseSensitive = !settings.findCaseSensitive;
        settings.commit();
        update();
    };
    m_wordsToggle = addToggle("W");
    m_wordsToggle->onClicked = [this] {
        EditorSettings &settings = m_page.application().settings();
        settings.findWholeWords = !settings.findWholeWords;
        settings.commit();
        update();
    };
    m_regexToggle = addToggle(".*");
    m_regexToggle->onClicked = [this] {
        EditorSettings &settings = m_page.application().settings();
        settings.findRegex = !settings.findRegex;
        settings.commit();
        update();
    };
    m_selectionToggle = addToggle("[ ]");
    m_selectionToggle->onClicked = [this] {
        m_inSelection = !m_inSelection;
        update();
    };

    m_previous = addChild<RoundButton>(28);
    m_previous->setName("previousMatchButton");
    m_previous->setGlyph("chevron-up", 16);
    m_previous->onClicked = [this] { find(true); };
    m_next = addChild<RoundButton>(28);
    m_next->setGlyph("chevron-down", 16);
    m_next->onClicked = [this] { find(false); };
    m_close = addChild<RoundButton>(28);
    m_close->setName("closeFindButton");
    m_close->setGlyph("close", 16);
    m_close->onClicked = [this] { close(); };

    m_replaceField = addChild<CardField>();
    m_replaceField->setName("replaceField");
    m_replaceField->onReturn = [this](bool) { m_page.document().replaceCurrent(m_replaceField->text()); };
    m_replaceField->onEscape = [this] { close(); };
    m_replace = addChild<TextButton>();
    m_replace->setName("replaceButton");
    m_replace->onClicked = [this] { m_page.document().replaceCurrent(m_replaceField->text()); };
    m_replaceAll = addChild<TextButton>();
    m_replaceAll->setName("replaceAllButton");
    m_replaceAll->onClicked = [this] {
        const int count = m_page.document().replaceAll(m_replaceField->text());
        if (EditorWindow *window = m_page.editorWindow())
            window->showNotification(trn("Replaced {n} occurrence", "Replaced {n} occurrences", uint64_t(count)), 2000);
    };

    m_wrapped.setSingleShot(true);
    m_wrapped.setInterval(1200ms);
    m_wrapped.onTimeout = [this] { refresh(); };
    setTexts();
    setReplacing(false);
}

FindBar::~FindBar() = default;

void FindBar::setTexts()
{
    m_field->setPlaceholder(tr("Find"));
    m_caseToggle->setToolTip(tr("Match Case"));
    m_wordsToggle->setToolTip(tr("Whole Words"));
    m_regexToggle->setToolTip(tr("Regular Expression"));
    m_selectionToggle->setToolTip(tr("In Selection"));
    m_replaceField->setPlaceholder(tr("Replace with"));
    m_replace->setText(tr("Replace"));
    m_replaceAll->setText(tr("Replace All"));
}

void FindBar::retranslate()
{
    setTexts();
    refresh();
}

RoundButton *FindBar::addToggle(const std::string &text)
{
    // A two-state option inside the field, drawn as its own short label.
    RoundButton *toggle = addChild<RoundButton>(24);
    Label *label = toggle->setLabel(text, 0);
    label->setFontSize(11);
    label->setWeight(600);
    return toggle;
}

void FindBar::setReplacing(bool replacing)
{
    m_replacing = replacing;
    m_disclosure->setGlyph(replacing ? "chevron-down" : "chevron-right", 16);
    m_replaceField->setVisible(replacing);
    m_replace->setVisible(replacing);
    m_replaceAll->setVisible(replacing);
    if (isVisible())
        m_page.placeCards();
    m_page.requestLayout();
    requestLayout();
}

void FindBar::open(const std::string &text, bool replace)
{
    setReplacing(replace);
    setVisible(true);
    m_page.placeCards();
    if (!text.empty())
        m_field->setText(text);
    m_inSelection = false;
    update();
    if (replace && !m_field->text().empty())
        m_replaceField->setFocus();
    else
        m_field->setFocus();
    m_field->selectAll();
    m_page.requestLayout();
}

void FindBar::close(bool focusEditor)
{
    if (!isVisible())
        return;
    setVisible(false);
    m_page.placeCards();
    m_page.document().clearSearch();
    if (focusEditor)
        m_page.focusEditor();
    m_page.requestLayout();
}

void FindBar::update()
{
    const EditorSettings &settings = m_page.application().settings();
    Document &document = m_page.document();
    document.setSearch(m_field->text(), settings.findCaseSensitive, settings.findWholeWords, settings.findRegex,
                       m_inSelection);
    if (!m_field->text().empty() && !m_inSelection)
        document.findIncremental();
    refresh();
}

void FindBar::find(bool backwards)
{
    Document &document = m_page.document();
    if (document.searchText() != m_field->text() || !isVisible())
        update();
    if (document.findNext(backwards) && document.lastSearchWrapped()) {
        m_wrapped.start();
        refresh();
    }
}

void FindBar::refresh()
{
    const Theme &theme = Theme::current();
    const EditorSettings &settings = m_page.application().settings();
    const Document &document = m_page.document();
    const bool hasText = !m_field->text().empty();
    const bool invalid = hasText && !document.searchValid();
    const bool notFound = hasText && document.searchValid() && document.matchCount() == 0;

    m_field->setTextColor(notFound || invalid ? std::optional<Color>(accent::red) : std::nullopt);
    m_field->setToolTip(invalid ? document.searchError() : std::string());
    std::string count;
    if (hasText) {
        if (invalid)
            // TRANSLATORS: beside the find field, when the search is not a valid regular expression.
            count = tr("Invalid");
        else if (m_wrapped.isActive())
            // TRANSLATORS: beside the find field, when the search has continued from the other end of the document.
            count = tr("Wrapped");
        else if (document.matchCount() == 0)
            count = tr("No matches");
        else if (document.matchIndex() > 0)
            // TRANSLATORS: the current match and the number of matches, such as "2 of 5".
            count = format(tr("{index} of {count}"), {{"index", std::to_string(document.matchIndex())},
                                                      {"count", std::to_string(document.matchCount())}});
        else
            count = trn("{n} match", "{n} matches", uint64_t(document.matchCount()));
    }
    m_count->setText(count);
    m_count->setVisible(!count.empty());
    m_count->setColor(invalid || notFound ? ColorRecipe(accent::red) : ColorRecipe(ColorId::DisabledText));

    const auto setToggle = [&theme](RoundButton *toggle, bool on) {
        toggle->setBackgroundColor(on ? theme.highlight.withAlpha(0.18) : Color{0, 0, 0, 0});
        toggle->label()->setColor(on ? ColorId::Highlight : ColorId::Text);
    };
    setToggle(m_caseToggle, settings.findCaseSensitive);
    setToggle(m_wordsToggle, settings.findWholeWords);
    setToggle(m_regexToggle, settings.findRegex);
    setToggle(m_selectionToggle, m_inSelection);
    m_selectionToggle->setEnabled(m_inSelection || document.hasSelection());

    const bool matches = document.matchCount() > 0;
    m_previous->setEnabled(matches);
    m_next->setEnabled(matches);
    m_replace->setEnabled(matches && !document.isLocked());
    m_replaceAll->setEnabled(matches && !document.isLocked());
    requestLayout();
}

double FindBar::implicitHeight() const
{
    return 32 + (m_replacing ? 6 + 32 : 0) + padding * 2;
}

void FindBar::layout()
{
    const double width = geometry().width - padding * 2;
    const double x0 = padding;
    const double y0 = padding;

    // A row, spacing 2: disclosure, field, previous, next, close.
    const double buttonY = y0 + centreIn(32, 28);
    m_disclosure->setGeometry({x0, buttonY, 28, 28});
    const double fieldWidth = std::max(0.0, width - (28 + 2) * 4);
    const double fieldX = x0 + 28 + 2;
    m_field->setGeometry({fieldX, y0, fieldWidth, 32});
    double x = fieldX + fieldWidth + 2;
    for (RoundButton *button : {m_previous, m_next, m_close}) {
        button->setGeometry({x, buttonY, 28, 28});
        x += 28 + 2;
    }

    // Inside the field: the count and the toggles, anchored right 4, centred.
    std::vector<View *> inside;
    if (m_count->isVisible())
        inside.push_back(m_count);
    for (RoundButton *toggle : {m_caseToggle, m_wordsToggle, m_regexToggle, m_selectionToggle})
        inside.push_back(toggle);
    double insideWidth = 0;
    const SizeF countSize = m_count->sizeHint();
    const double countWidth = countSize.width + 4;
    if (m_count->isVisible())
        insideWidth += countWidth;
    insideWidth += 24 * 4;
    double ix = fieldX + fieldWidth - 4 - insideWidth;
    if (m_count->isVisible()) {
        m_count->setGeometry({ix, y0 + rounding::centeredStart(32, countSize.height), countSize.width, countSize.height});
        ix += countWidth;
    }
    for (RoundButton *toggle : {m_caseToggle, m_wordsToggle, m_regexToggle, m_selectionToggle}) {
        toggle->setGeometry({ix, y0 + rounding::centeredStart(32, 24), 24, 24});
        ix += 24;
    }
    m_field->setTextMargins(Theme::current().largeSpacing, insideWidth + 12);

    if (!m_replacing)
        return;
    // A row, spacing 6, ending where the close button's round plate does.
    const double plateInset = std::round(28 * 0.1);
    const double rowY = y0 + 32 + 6;
    const SizeF replaceSize = m_replace->sizeHint();
    const SizeF allSize = m_replaceAll->sizeHint();
    double right = x0 + width - plateInset;
    right -= std::ceil(allSize.width);
    m_replaceAll->setGeometry({right, rowY + centreIn(32, std::ceil(allSize.height)), std::ceil(allSize.width), std::ceil(allSize.height)});
    right -= 6 + std::ceil(replaceSize.width);
    m_replace->setGeometry({right, rowY + centreIn(32, std::ceil(replaceSize.height)), std::ceil(replaceSize.width),
                            std::ceil(replaceSize.height)});
    // Under the disclosure, so both fields start on one edge.
    const double replaceFieldX = x0 + (28 + 2 - 6) + 6;
    m_replaceField->setGeometry({replaceFieldX, rowY, std::max(0.0, right - 6 - replaceFieldX), 32});
}

GoToLineBar::GoToLineBar(EditorPage &page)
    : m_page(page)
    , m_field(addChild<CardField>())
    , m_close(addChild<RoundButton>(28))
{
    setName("goToLineBar");
    setRadius(Theme::current().smallRadius + padding);
    setVisible(false);
    m_field->setName("goToLineField");
    m_field->setValidator([](const std::string &text) {
        return std::all_of(text.begin(), text.end(), [](char c) { return (c >= '0' && c <= '9') || c == ':' || c == ',' || c == ' '; });
    });
    m_field->onTextEdited = [this] { refresh(); };
    m_field->onReturn = [this](bool) { accept(); };
    m_field->onEscape = [this] { close(); };
    m_close->setGlyph("close", 16);
    m_close->onClicked = [this] { close(); };
}

bool GoToLineBar::target(int *line, int *column) const
{
    static const RegExp pattern("^\\s*(\\d+)\\s*(?:[:,]\\s*(\\d+))?\\s*$");
    const std::string &text = m_field->text();
    const RegExp::Match match = pattern.match(text);
    if (!match.hasMatch())
        return false;
    const auto number = [](std::string_view digits) {
        return digits.size() > 9 ? -1 : strings::toNumber<int>(digits).value_or(-1);
    };
    *line = number(match.captured(text, 1));
    *column = match.start(2) >= 0 ? number(match.captured(text, 2)) : 1;
    return true;
}

bool GoToLineBar::valid() const
{
    int line = 0;
    int column = 0;
    return target(&line, &column) && line >= 1 && line <= m_page.document().lineCount();
}

void GoToLineBar::refresh()
{
    // TRANSLATORS: {count} is the number of lines; a column number can follow the line number after a colon.
    m_field->setPlaceholder(format(tr("Go to line (1\xE2\x80\x93{count}), :column"),
                                   {{"count", std::to_string(m_page.document().lineCount())}}));
    m_field->setTextColor(!m_field->text().empty() && !valid() ? std::optional<Color>(accent::red) : std::nullopt);
}

void GoToLineBar::open()
{
    setVisible(true);
    m_field->setText({});
    refresh();
    m_field->setFocus();
    m_page.requestLayout();
}

void GoToLineBar::close()
{
    if (!isVisible())
        return;
    setVisible(false);
    m_page.placeCards();
    m_page.focusEditor();
    m_page.requestLayout();
}

void GoToLineBar::accept()
{
    int line = 0;
    int column = 0;
    if (!valid() || !target(&line, &column))
        return;
    m_page.document().goToLine(line, column);
    close();
}

void GoToLineBar::layout()
{
    const double width = geometry().width - padding * 2;
    m_field->setGeometry({padding, padding, std::max(0.0, width - 2 - 28), 32});
    m_close->setGeometry({padding + width - 28, padding + centreIn(32, 28), 28, 28});
}

LoadingView::LoadingView(EditorPage &page)
    : m_page(page)
    , m_label(addChild<Label>())
    , m_progress(addChild<ProgressBar>())
    , m_cancel(addChild<TextButton>(tr("Cancel")))
{
    m_label->setElide(Label::Elide::Middle);
    m_cancel->onClicked = [this] {
        m_page.document().cancelLoading();
        if (EditorWindow *window = m_page.editorWindow())
            window->closeTab(window->indexOfPage(&m_page), false);
    };
}

void LoadingView::update()
{
    m_label->setText(format(tr("Opening \xE2\x80\x9C{name}\xE2\x80\x9D\xE2\x80\xA6"), {{"name", m_page.document().fileName()}}));
    m_progress->setValue(m_page.document().loadProgress());
    requestLayout();
}

void LoadingView::retranslate()
{
    m_cancel->setText(tr("Cancel"));
    update();
}

void LoadingView::layout()
{
    // A column centred in the editor, min(width - 64, 320) wide, spacing 12.
    const double width = std::min(geometry().width - 64, 320.0);
    const SizeF label = m_label->sizeHint();
    const double labelWidth = std::min(std::ceil(label.width), width);
    const double labelHeight = std::ceil(label.height);
    const SizeF progress = m_progress->sizeHint();
    const SizeF cancel = m_cancel->sizeHint();
    const double height = labelHeight + 12 + std::ceil(progress.height) + 12 + std::ceil(cancel.height);
    const double x = rounding::centeredStart(geometry().width, width);
    double y = rounding::centeredStart(geometry().height, height);
    m_label->setGeometry({x + std::round((width - labelWidth) / 2), y, labelWidth, labelHeight});
    y += labelHeight + 12;
    m_progress->setGeometry({x, y, width, std::ceil(progress.height)});
    y += std::ceil(progress.height) + 12;
    m_cancel->setGeometry({x + std::round((width - std::ceil(cancel.width)) / 2), y, std::ceil(cancel.width), std::ceil(cancel.height)});
}
