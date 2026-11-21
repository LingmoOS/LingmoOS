// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/language/addlanguagedialog.h"

#include <chrono>
#include <cmath>

#include "base/i18n.h"
#include "pages/language/language.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/searchfield.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

// A language to pick: its own name, its name in the system language, and a
// check when it is the chosen one.
class LanguageRow : public ListRow
{
public:
    LanguageRow(const LanguageEntry& entry, bool checked)
        : code(entry.code)
    {
        ColumnLayout* names = addContent<ColumnLayout>();
        names->setSpacing(metrics::textSpacing);
        names->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});
        m_name = names->addChild<Label>(entry.name);
        m_name->setLayoutHints({.fillWidth = true});
        m_description = names->addChild<Label>(entry.description);
        m_description->setLayoutHints({.fillWidth = true});
        m_check = addContent<TrailingIcon>("check", 18);
        m_check->setVisible(checked);
        setName("language:" + entry.code);
    }

    const std::string code;

    void setChecked(bool checked)
    {
        m_check->setVisible(checked);
        requestLayout();
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_name->setFontSize(theme.fontSize());
        m_description->setFontSize(std::max(1.0, theme.fontSize() - 1));
        m_description->setColor(theme.disabledText);
        m_check->icon()->setColor(theme.highlight);
        ListRow::layout();
    }

private:
    Label* m_name;
    Label* m_description;
    TrailingIcon* m_check;
};

// Recoloured with the theme.
class DisabledLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

// Gives every child its whole rect: the list, and the label over it.
class Stack : public View
{
protected:
    void layout() override
    {
        for (const auto& child : children())
            child->setGeometry(localRect());
    }
};

// Centres a label over the list without scrolling with it.
class EmptyOverlay : public View
{
public:
    EmptyOverlay()
    {
        label = addChild<DisabledLabel>();
    }

    DisabledLabel* label;

protected:
    void layout() override
    {
        const SizeF size = label->sizeHint();
        label->setGeometry({rounding::centeredStart(geometry().width, size.width),
                            rounding::centeredStart(geometry().height, size.height), size.width, size.height});
    }
};

} // namespace

AddLanguageDialog::AddLanguageDialog(SettingsWindow& parent, Language& language)
    : FormDialog(parent, tr("Add Language"), 460)
    , m_language(language)
{
    setFixedHeight(560);
    // The page colour, so the card stands out as it does on a page.
    setBackgroundColor(ColorId::Background);

    m_search = content()->addChild<SearchField>();
    m_search->setPlaceholder(tr("Search languages"));
    m_search->setLayoutHints({.fillWidth = true});
    m_search->onTextChanged = [this] {
        select({});
        m_searchTimer.start();
    };
    m_searchTimer.setSingleShot(true);
    m_searchTimer.setInterval(std::chrono::milliseconds(100));
    m_searchTimer.onTimeout = [this] {
        m_language.search(m_search->text());
        m_scroll->setScrollPosition({0, 0});
    };

    Card* card = content()->addChild<Card>(Card::Style::List);
    card->setLayoutHints({.fillWidth = true, .fillHeight = true});
    Stack* stack = card->addChild<Stack>();
    stack->setLayoutHints({.fillWidth = true, .fillHeight = true});
    stack->setClipsChildren(true);
    m_scroll = stack->addChild<ScrollView>();
    m_scroll->setElastic(false);
    m_rows = m_scroll->setContent<ColumnLayout>();
    m_rows->setSpacing(0);
    m_empty = stack->addChild<EmptyOverlay>()->label;

    m_error = content()->addChild<Label>();
    m_error->setWrap(true);
    m_error->setLayoutHints({.fillWidth = true});

    RowLayout* buttons = addButtonRow();
    m_cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    m_cancel->onClicked = [this] { requestClose(); };
    m_add = buttons->addChild<DialogButton>(tr("Add"), true);
    m_add->onClicked = [this] {
        m_adding = true;
        updateStates();
        m_language.addLanguage(m_selectedCode);
    };

    m_subscriptions.push_back(m_language.availableLanguagesChanged.connect([this] { updateRows(); }));
    m_subscriptions.push_back(m_language.loadingChanged.connect([this] { updateRows(); }));
    m_subscriptions.push_back(m_language.errorMessageChanged.connect([this] { updateStates(); }));
    m_subscriptions.push_back(m_language.savingChanged.connect([this] {
        if (m_adding && !m_language.isSaving()) {
            m_adding = false;
            if (m_language.errorMessage().empty()) {
                close();
                return;
            }
        }
        updateStates();
    }));
    canClose = [this] { return !m_adding; };
    onShown = [this] { m_search->setFocus(); };

    m_language.search({});
    m_language.loadLanguages();
    updateRows();
}

void AddLanguageDialog::select(const std::string& code)
{
    m_selectedCode = code;
    for (const auto& child : m_rows->children()) {
        auto* row = static_cast<LanguageRow*>(child.get());
        row->setChecked(row->code == code);
    }
    updateStates();
}

void AddLanguageDialog::updateRows()
{
    const PointF position = m_scroll->scrollPosition();
    while (!m_rows->children().empty())
        m_rows->takeChild(m_rows->children().front().get());
    const std::vector<LanguageEntry> languages = m_language.availableLanguages();
    for (const LanguageEntry& entry : languages) {
        LanguageRow* row = m_rows->addChild<LanguageRow>(entry, entry.code == m_selectedCode);
        const std::string code = entry.code;
        row->onClicked = [this, code] { select(code); };
    }
    m_scroll->setScrollPosition(position);
    m_empty->setText(m_language.isLoading() ? tr("Loading languages…") : tr("No languages found"));
    m_empty->parent()->setVisible(languages.empty());
    updateStates();
}

void AddLanguageDialog::updateStates()
{
    m_search->setEnabled(!m_adding);
    m_scroll->setEnabled(!m_adding);
    m_cancel->setEnabled(!m_adding);
    m_add->setEnabled(!m_selectedCode.empty() && !m_language.isSaving());
    m_error->setText(m_language.errorMessage());
    m_error->setVisible(!m_language.errorMessage().empty());
}

} // namespace settings
