// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "pages/shortcuts/shortcuteditdialog.h"
#include "pages/shortcuts/shortcutkeys.h"
#include "pages/shortcuts/shortcutsmodel.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

// An action's shortcuts stacked at the end of its row: a Column positioner,
// each shortcut anchored to its right edge.
class AcceleratorStack : public View
{
public:
    void setAccelerators(const std::vector<ShortcutsModel::Accelerator>& accelerators)
    {
        while (!children().empty())
            takeChild(children().back().get());
        m_keys.clear();
        for (const ShortcutsModel::Accelerator& accelerator : accelerators)
            m_keys.push_back(addChild<ShortcutKeys>(accelerator.parts));
        requestLayout();
    }

    SizeF sizeHint() const override
    {
        double width = 0;
        double height = 0;
        for (size_t i = 0; i < m_keys.size(); ++i) {
            const SizeF keys = m_keys[i]->sizeHint();
            width = std::max(width, keys.width);
            height += (i > 0 ? metrics::xs : 0) + keys.height;
        }
        return {width, height};
    }

protected:
    void layout() override
    {
        double y = 0;
        for (ShortcutKeys* keys : m_keys) {
            const SizeF size = keys->sizeHint();
            keys->setDimmed(!isEnabled());
            keys->setGeometry({geometry().width - size.width, y, size.width, size.height});
            y += size.height + metrics::xs;
        }
    }

private:
    std::vector<ShortcutKeys*> m_keys;
};

// One action: what it does on the left, how to trigger it on the right.
class ShortcutRow : public ListRow
{
public:
    explicit ShortcutRow(const ShortcutsModel::Action& action)
        : ListRow(action.description)
    {
        m_none = addContent<Label>(tr("None"));
        m_none->setVisible(action.accelerators.empty());
        m_keys = addContent<AcceleratorStack>();
        // Air above and below, so a stack of two makes the row grow.
        m_keys->setLayoutHints({.margins = {0, metrics::xs, 0, metrics::xs}});
        m_keys->setAccelerators(action.accelerators);
    }

protected:
    void layout() override
    {
        m_none->setColor(Theme::current().disabledText);
        ListRow::layout();
    }

private:
    Label* m_none;
    AcceleratorStack* m_keys;
};

// A wrapped secondary text in a padded card.
class NoteCard : public Card
{
public:
    explicit NoteCard(std::string text)
    {
        m_label = addChild<Label>(std::move(text));
        m_label->setWrap(true);
        m_label->setLayoutHints({.fillWidth = true});
    }

protected:
    void layout() override
    {
        m_label->setColor(Theme::current().disabledText);
        Card::layout();
    }

private:
    Label* m_label;
};

class ShortcutsPage : public Page
{
public:
    explicit ShortcutsPage(SettingsWindow& window)
        : Page(window, tr("Keyboard Shortcuts"))
    {
        m_unavailable = content()->addChild<NoteCard>(
                tr("Keyboard shortcuts are managed by the Lingmo compositor, which is not running."));
        m_intro = content()->addChild<Footnote>();
        for (const std::string& category : ShortcutsModel::categories()) {
            Section* section = content()->addChild<Section>(tr(category.c_str()));
            m_sections.push_back(section);
            m_cards.push_back(section->addChild<Card>(Card::Style::List));
        }
        m_restoreAll = content()->addChild<StandardButton>(tr("Restore all defaults"));
        m_restoreAll->setName("shortcuts:restoreAll");
        m_restoreAll->setLayoutHints({.fillWidth = true});
        m_restoreAll->onClicked = [this] { confirmRestoreAll(); };

        m_changed = m_model.changed.connect([this] { update(); });
        update();
    }

private:
    void update()
    {
        const bool available = m_model.isAvailable();
        m_unavailable->setVisible(!available);
        m_intro->setVisible(available);
        m_intro->setText(m_model.commandKeys()
                                 ? tr("Click a system shortcut to change it. Shortcuts with both Control and Command "
                                      "are the safest choice: applications never get them.")
                                 : tr("Click a system shortcut to change it. Shortcuts with Meta are the safest "
                                      "choice: applications never use them."));
        const std::vector<std::string> categories = ShortcutsModel::categories();
        for (size_t i = 0; i < categories.size(); ++i) {
            m_sections[i]->setVisible(available);
            Card* card = m_cards[i];
            while (!card->children().empty())
                card->takeChild(card->children().back().get());
            for (const ShortcutsModel::Action& action : m_model.actions()) {
                if (action.category != categories[i])
                    continue;
                ShortcutRow* row = card->addChild<ShortcutRow>(action);
                row->setName("shortcut:" + action.name);
                row->onClicked = [this, name = action.name] { edit(name); };
            }
        }
        m_restoreAll->setVisible(available);
        m_restoreAll->setEnabled(m_model.hasCustomAccelerators());
    }

    void edit(const std::string& action)
    {
        m_editor = std::make_unique<ShortcutEditDialog>(settingsWindow(), m_model, action);
        ShortcutEditDialog* editor = m_editor.get();
        m_editor->onFinished = [this, editor] {
            // Not from inside the dialog's own close.
            base::EventLoop::current()->post([this, editor, alive = std::weak_ptr<int>(m_alive)] {
                if (alive.lock() && m_editor.get() == editor)
                    m_editor.reset();
            });
        };
        m_editor->show();
    }

    void confirmRestoreAll()
    {
        m_confirm = std::make_unique<ConfirmDialog>(&settingsWindow().window(), tr("Restore all defaults"),
                                                    tr("Every shortcut goes back to its default. Your changes will "
                                                       "be lost."));
        m_confirm->setCancelText(tr("Cancel"));
        m_confirm->setConfirmText(tr("Restore"));
        m_confirm->setDestructive(true);
        m_confirm->onAccepted = [this] { m_model.restoreAllDefaults(); };
        m_confirm->show();
    }

    ShortcutsModel m_model;
    base::Subscription m_changed;
    NoteCard* m_unavailable;
    Footnote* m_intro;
    std::vector<Section*> m_sections;
    std::vector<Card*> m_cards;
    StandardButton* m_restoreAll;
    std::unique_ptr<ShortcutEditDialog> m_editor;
    std::unique_ptr<ConfirmDialog> m_confirm;
    std::shared_ptr<int> m_alive = std::make_shared<int>(0);
};

} // namespace

static const PageRegistration registration("shortcuts", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<ShortcutsPage>(window);
});

} // namespace settings
