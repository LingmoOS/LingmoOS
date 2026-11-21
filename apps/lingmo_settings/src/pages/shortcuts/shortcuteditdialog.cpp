// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/shortcuts/shortcuteditdialog.h"

#include <memory>

#include <xkbcommon/xkbcommon-keysyms.h>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "pages/shortcuts/shortcutkeys.h"
#include "ui/dialog_window.h"
#include "ui/event.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kDialogWidth = 440;
static constexpr double kRecorderHeight = 48;
static constexpr Color kErrorColor = Color::rgb(0xE24B4B);

static uint32_t compositorModifiers(uint32_t modifiers)
{
    uint32_t result = 0;
    if (modifiers & ShiftModifier)
        result |= ShortcutsModel::ShiftDown;
    if (modifiers & ControlModifier)
        result |= ShortcutsModel::ControlDown;
    if (modifiers & AltModifier)
        result |= ShortcutsModel::AltDown;
    if (modifiers & SuperModifier)
        result |= ShortcutsModel::MetaDown;
    return result;
}

static bool isMetaKey(uint32_t keysym)
{
    return keysym == XKB_KEY_Meta_L || keysym == XKB_KEY_Meta_R || keysym == XKB_KEY_Super_L
            || keysym == XKB_KEY_Super_R;
}

namespace {

// Text in the accent colour that acts when clicked, dimmed while pressed.
class LinkLabel : public View
{
public:
    explicit LinkLabel(std::string text) { m_label = addChild<Label>(std::move(text)); }

    SizeF sizeHint() const override { return m_label->sizeHint(); }
    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_label->setColor(Theme::current().highlight);
        m_label->setGeometry(localRect());
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left)
            setPressed(true);
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        setPressed(false);
        if (!localRect().contains(event.position) || !onClicked)
            return;
        // After the event: the click rebuilds the rows this label is in.
        Lingmo::base::EventLoop::current()->post(
                [callback = onClicked, alive = std::weak_ptr<int>(m_alive)] {
                    if (alive.lock())
                        callback();
                });
    }

private:
    void setPressed(bool pressed)
    {
        m_pressed = pressed;
        m_label->setOpacity(pressed ? 0.6 : 1);
    }

    Label* m_label;
    bool m_pressed = false;
    std::shared_ptr<int> m_alive = std::make_shared<int>(0);
};

} // namespace

// Takes every key while recording, so the shortcut being pressed lands here.
class ShortcutRecorder : public View
{
public:
    explicit ShortcutRecorder(ShortcutEditDialog& dialog)
        : m_dialog(dialog)
    {
        m_row = addChild<RowLayout>();
        m_row->setSpacing(metrics::rowSpacing);
        m_prompt = m_row->addChild<Label>();
        m_keys = m_row->addChild<ShortcutKeys>();
        m_row->addChild<View>()->setLayoutHints({.fillWidth = true});
        m_cancel = m_row->addChild<LinkLabel>(tr("Cancel"));
        m_cancel->onClicked = [this] { m_dialog.stopRecording(); };
    }

    void setState(bool replacing, const std::vector<std::string>& parts)
    {
        m_prompt->setText(replacing ? tr("Press the new shortcut…") : tr("Press the shortcut…"));
        m_prompt->setVisible(parts.empty());
        m_keys->setParts(parts);
        m_keys->setVisible(!parts.empty());
        requestLayout();
    }

    SizeF sizeHint() const override { return {0, kRecorderHeight}; }
    bool acceptsFocus() const override { return m_dialog.isRecording(); }

protected:
    void layout() override
    {
        m_prompt->setColor(Theme::current().text);
        // The RowLayout fills the field, inset by 12 at the sides.
        m_row->setGeometry(localRect().adjusted(metrics::m, 0, -metrics::m, 0));
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, themeCorners()}, theme.highlight.withAlpha(0.12),
                                1, theme.highlight);
    }

    bool keyPress(const KeyEvent& event) override
    {
        m_dialog.keyPressed(event);
        return true;
    }

    bool keyRelease(const KeyEvent& event) override
    {
        m_dialog.keyReleased(event);
        return true;
    }

    void focusOut() override
    {
        if (m_dialog.isRecording())
            m_dialog.stopRecording();
    }

private:
    ShortcutEditDialog& m_dialog;
    RowLayout* m_row;
    Label* m_prompt;
    ShortcutKeys* m_keys;
    LinkLabel* m_cancel;
};

ShortcutEditDialog::ShortcutEditDialog(SettingsWindow& parent, ShortcutsModel& model, std::string action)
    : FormDialog(parent, model.descriptionFor(action), kDialogWidth)
    , m_model(model)
    , m_action(std::move(action))
{
    ColumnLayout* column = content();
    m_noShortcut = column->addChild<Label>(tr("No shortcut. Add one below."));
    // The accelerators' rows, spaced as the dialog's own items are.
    m_list = column->addChild<ColumnLayout>();
    m_list->setSpacing(metrics::dialogSpacing);
    m_list->setLayoutHints({.fillWidth = true});
    m_recorder = column->addChild<ShortcutRecorder>(*this);
    m_recorder->setName("shortcutdialog:recorder");
    m_recorder->setLayoutHints({.fillWidth = true});
    m_messageLabel = column->addChild<Label>();
    m_messageLabel->setName("shortcutdialog:message");
    m_messageLabel->setWrap(true);
    m_messageLabel->setFontSize(metrics::points(9));
    m_messageLabel->setLayoutHints({.fillWidth = true});

    RowLayout* buttons = column->addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.fillWidth = true, .margins = {0, metrics::xs, 0, 0}});
    m_add = buttons->addChild<DialogButton>(tr("Add shortcut"));
    m_add->onClicked = [this] { startRecording(std::nullopt); };
    m_restore = buttons->addChild<DialogButton>(tr("Restore default"));
    m_restore->onClicked = [this] {
        const ShortcutsModel::Result result = m_model.restoreDefault(m_action);
        m_message = result.message;
        m_messageIsWarning = true;
        update();
    };
    buttons->addChild<View>()->setLayoutHints({.fillWidth = true});

    onClosed = [this] {
        stopRecording();
        if (onFinished)
            onFinished();
    };
    m_modelChanged = m_model.changed.connect([this] { refresh(); });
    refresh();
}

ShortcutEditDialog::~ShortcutEditDialog()
{
    stopRecording();
}

void ShortcutEditDialog::refresh()
{
    const ShortcutsModel::Action* action = m_model.action(m_action);
    m_accelerators = action ? action->accelerators : std::vector<ShortcutsModel::Accelerator>();
    m_isDefault = action ? action->isDefault() : true;
    update();
}

void ShortcutEditDialog::update()
{
    m_noShortcut->setColor(ColorId::DisabledText);
    m_noShortcut->setVisible(m_accelerators.empty() && !m_recording);

    while (!m_list->children().empty())
        m_list->takeChild(m_list->children().back().get());
    m_list->setVisible(!m_accelerators.empty());
    for (const ShortcutsModel::Accelerator& accelerator : m_accelerators) {
        RowLayout* row = m_list->addChild<RowLayout>();
        row->setSpacing(metrics::rowSpacing);
        row->setLayoutHints({.fillWidth = true});
        const bool beingReplaced = m_recording && m_replacing && m_replacing->keysym == accelerator.keysym
                && m_replacing->modifiers == accelerator.modifiers;
        ShortcutKeys* keys = row->addChild<ShortcutKeys>(accelerator.parts);
        keys->setDimmed(beingReplaced);
        if (accelerator.type == "user") {
            Label* custom = row->addChild<Label>(tr("Custom"));
            custom->setColor(ColorId::DisabledText);
            custom->setFontSize(metrics::points(8));
        }
        row->addChild<View>()->setLayoutHints({.fillWidth = true});
        if (!m_recording) {
            LinkLabel* change = row->addChild<LinkLabel>(tr("Change"));
            change->onClicked = [this, accelerator] { startRecording(accelerator); };
            LinkLabel* remove = row->addChild<LinkLabel>(tr("Remove"));
            remove->onClicked = [this, accelerator] {
                const ShortcutsModel::Result result =
                        m_model.removeAccelerator(m_action, accelerator.keysym, accelerator.modifiers);
                m_message = result.message;
                m_messageIsWarning = false;
                update();
            };
        }
    }

    m_recorder->setVisible(m_recording);
    m_recorder->setState(m_replacing.has_value(), m_pendingParts);

    m_messageLabel->setVisible(!m_message.empty());
    m_messageLabel->setText(m_message);
    m_messageLabel->setColor(m_messageIsWarning ? ColorRecipe(ColorId::Text) : ColorRecipe(kErrorColor));

    m_add->setEnabled(!m_recording && m_accelerators.size() < 5);
    m_restore->setVisible(!m_isDefault);
    m_restore->setEnabled(!m_recording);
}

void ShortcutEditDialog::startRecording(std::optional<ShortcutsModel::Accelerator> replacing)
{
    m_replacing = std::move(replacing);
    m_pendingParts.clear();
    m_metaTapCandidate = false;
    m_message.clear();
    m_recording = true;
    m_model.setRecording(true);
    update();
    m_recorder->setFocus();
}

void ShortcutEditDialog::stopRecording()
{
    if (!m_recording)
        return;
    m_recording = false;
    m_replacing.reset();
    m_pendingParts.clear();
    m_model.setRecording(false);
    update();
}

void ShortcutEditDialog::keyPressed(const KeyEvent& event)
{
    // Holding the key must not turn into the second press that confirms a
    // conflict or a warning.
    if (event.repeat)
        return;
    uint32_t modifiers = compositorModifiers(event.modifiers);
    if (ShortcutsModel::isModifierKey(event.key)) {
        // The key's own modifier is not in the state yet when it goes down.
        modifiers |= ShortcutsModel::modifierForKey(event.key);
        m_pendingParts = m_model.modifierParts(modifiers);
        m_metaTapCandidate = isMetaKey(event.key) && modifiers == ShortcutsModel::MetaDown;
        update();
        return;
    }
    m_metaTapCandidate = false;
    if (event.key == key::Escape && modifiers == 0) {
        stopRecording();
        return;
    }
    submit(event.keycode, ShortcutsModel::serviceKeysym(event.key), modifiers,
           m_model.partsForKey(event.keycode, event.key, modifiers));
}

void ShortcutEditDialog::keyReleased(const KeyEvent& event)
{
    if (!m_metaTapCandidate || !isMetaKey(event.key))
        return;
    m_metaTapCandidate = false;
    submit(event.keycode, ShortcutsModel::serviceKeysym(event.key), 0,
           m_model.modifierParts(ShortcutsModel::MetaDown));
}

void ShortcutEditDialog::submit(uint32_t keycode, uint32_t keysym, uint32_t modifiers, std::vector<std::string> parts)
{
    m_pendingParts = std::move(parts);
    const ShortcutsModel::Result result =
            m_replacing ? m_model.replaceAccelerator(m_action, m_replacing->keysym, m_replacing->modifiers, keycode,
                                                     keysym, modifiers)
                        : m_model.addAccelerator(m_action, keycode, keysym, modifiers);
    if (result.result == "success") {
        m_message.clear();
        stopRecording();
        return;
    }
    m_message = result.message;
    m_messageIsWarning = result.result == "conflict-can-override" || result.result == "non-meta-accelerator-warning";
    update();
}

} // namespace settings
