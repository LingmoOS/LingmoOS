// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/power_dialog.h"

#include <algorithm>
#include <cmath>

#include "compositor/surface.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "core/main_loop.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "ui/dialog_window.h"
#include "ui/embedded_window.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "wm/shell_surface.h"

using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

namespace Lingmo::Shell {

// FishUI.ConfirmDialog's metrics.
static constexpr double kMargin = 16;
static constexpr double kBottomMargin = 12;
static constexpr double kMinimumWidth = 336;
static constexpr double kTextSize = 14;
static constexpr double kButtonSpacing = 8;
// A press that moves this far moves the window.
static constexpr double kDragDistance = 10;
static constexpr int kDemiBold = 600;

static const char *const kText = trNoop("Every application is asked to quit first, so you can save your work.");

// The dialog's window content: the title and text, the buttons at the bottom
// right; a drag anywhere else moves the window. Cancel is the only way out,
// as the power dialog of every other desktop has it.
class PowerDialog::Content final : public ui::View
{
public:
    explicit Content(PowerDialog *dialog)
        : m_dialog(dialog)
    {
        m_title = addChild<ui::Label>();
        m_title->setFontSize(kTextSize);
        m_title->setWeight(kDemiBold);
        m_title->setWrap(true);
        m_text = addChild<ui::Label>(tr(kText));
        m_text->setFontSize(kTextSize);
        m_text->setWrap(true);
        m_text->setFixedLineHeight(20);
        m_text->setColor(ui::ColorId::DisabledText);
        m_cancel = addChild<ui::DialogButton>(tr("Cancel"));
        m_cancel->onClicked = [this] { m_dialog->finish(false); };
        m_confirm = addChild<ui::DialogButton>(std::string(), true);
        m_confirm->onClicked = [this] { m_dialog->finish(true); };
    }

    ui::DialogButton *cancel() const { return m_cancel; }

    void retranslate()
    {
        m_text->setText(tr(kText));
        m_cancel->setText(tr("Cancel"));
    }

    void setAction(Action action)
    {
        m_title->setText(PowerDialog::title(action));
        m_confirm->setText(PowerDialog::confirmText(action));
        requestLayout();
    }

    double width() const
    {
        const double actions = m_cancel->sizeHint().width + kButtonSpacing + m_confirm->sizeHint().width;
        return std::max(kMinimumWidth, actions + 2 * kMargin);
    }

    // The labels take the text width outright, so the window is never first
    // mapped one wrapped line short.
    double textWidth() const { return width() - 2 * kMargin - 2 * kMargin; }

    double height() const
    {
        const double texts = m_title->heightForWidth(textWidth()) + 4 + m_text->heightForWidth(textWidth());
        const double buttons = std::max(m_cancel->sizeHint().height, m_confirm->sizeHint().height);
        return std::ceil(kMargin + texts + kMargin + buttons + kBottomMargin);
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double textWidth = this->textWidth();
        const double titleHeight = m_title->heightForWidth(textWidth);
        m_title->setGeometry({kMargin, kMargin, textWidth, titleHeight});
        const double textHeight = m_text->heightForWidth(textWidth);
        m_text->setGeometry({kMargin, kMargin + titleHeight + 4, textWidth, textHeight});
        const ui::SizeF cancel = m_cancel->sizeHint();
        const ui::SizeF confirm = m_confirm->sizeHint();
        const double buttonsHeight = std::max(cancel.height, confirm.height);
        const double y = kMargin + titleHeight + 4 + textHeight + kMargin;
        const double right = width - kMargin;
        m_confirm->setGeometry({right - confirm.width, y + (buttonsHeight - confirm.height) / 2, confirm.width,
                                confirm.height});
        m_cancel->setGeometry({right - confirm.width - kButtonSpacing - cancel.width,
                               y + (buttonsHeight - cancel.height) / 2, cancel.width, cancel.height});
    }

    void paint(ui::Painter &painter) override
    {
        painter.fillRect(localRect(), ui::Theme::current().secondBackground);
    }

    void pointerPress(const ui::PointerEvent &event) override
    {
        if (event.button != ui::MouseButton::Left)
            return;
        m_pressed = true;
        m_pressPosition = event.position;
    }

    void pointerMove(const ui::PointerEvent &event) override
    {
        if (!m_pressed)
            return;
        const double dx = event.position.x - m_pressPosition.x;
        const double dy = event.position.y - m_pressPosition.y;
        if (std::hypot(dx, dy) < kDragDistance)
            return;
        m_pressed = false;
        if (ShellWindow *window = m_dialog->window(); window && window->shellSurface())
            window->shellSurface()->startMove();
    }

    void pointerRelease(const ui::PointerEvent &) override { m_pressed = false; }

private:
    PowerDialog *m_dialog;
    ui::Label *m_title;
    ui::Label *m_text;
    ui::DialogButton *m_cancel;
    ui::DialogButton *m_confirm;
    bool m_pressed = false;
    ui::PointF m_pressPosition;
};

PowerDialog::PowerDialog(Shell *shell)
    : m_shell(shell)
{
}

PowerDialog::~PowerDialog() = default;

std::string PowerDialog::title(Action action)
{
    switch (action) {
    case Action::Reboot:
        return tr("Restart now?");
    case Action::Shutdown:
        return tr("Shut down now?");
    case Action::Logout:
        break;
    }
    return tr("Log out now?");
}

std::string PowerDialog::confirmText(Action action)
{
    switch (action) {
    case Action::Reboot:
        return tr("Restart");
    case Action::Shutdown:
        return tr("Shut Down");
    case Action::Logout:
        break;
    }
    return tr("Log Out");
}

bool PowerDialog::isOpen() const
{
    return m_window && m_window->isShown();
}

void PowerDialog::open(Action action)
{
    m_action = action;
    if (!m_window) {
        m_window = std::make_unique<ShellWindow>(m_shell, wm::WindowRole::Normal);
        m_window->setDialog(true);
        auto content = std::make_unique<Content>(this);
        m_content = content.get();
        m_window->setContentView(std::move(content));
        // Kept above applications, as the compositor's always-on-top hint.
        m_window->surface()->setAlwaysOnTop(true);
        m_window->onCloseRequested = [this] { finish(false); };
        ui::EmbeddedWindow *window = m_window->window();
        window->addShortcut(ui::NoModifier, ui::key::Escape, [this] { finish(false); });
        // Return confirms even while Cancel holds the focus.
        window->addShortcut(ui::NoModifier, ui::key::Return, [this] { finish(true); });
        window->addShortcut(ui::NoModifier, ui::key::KeypadEnter, [this] { finish(true); });
    }
    m_content->setAction(action);
    m_window->shellSurface()->setTitle(title(action));
    resize();
    if (m_window->isShown())
        return;
    m_window->show();
    m_window->activate();
    m_content->cancel()->setFocus();
}

void PowerDialog::retranslate()
{
    if (!m_window)
        return;
    m_content->retranslate();
    open(m_action);
}

void PowerDialog::resize()
{
    m_window->setSize(base::Size(int(m_content->width()), int(m_content->height())));
}

void PowerDialog::close()
{
    if (!m_window)
        return;
    m_window->hide();
    // Not from inside its own event handling.
    std::shared_ptr<ShellWindow> doomed(std::move(m_window));
    m_content = nullptr;
    core::mainLoop()->post([doomed] {});
}

void PowerDialog::finish(bool accepted)
{
    if (!isOpen())
        return;
    close();
    if (accepted && onAccepted)
        onAccepted();
    else if (!accepted && onRejected)
        onRejected();
}

} // namespace Lingmo::Shell
