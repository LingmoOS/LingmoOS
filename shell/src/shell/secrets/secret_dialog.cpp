// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/secrets/secret_dialog.h"

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

namespace Lingmo::Shell {

static constexpr double kCloseButton = 32;
static constexpr double kCloseMargin = 8;
static constexpr double kTitleGap = 4;
static constexpr double kButtonSpacing = 8;
// A press that moves this far moves the window.
static constexpr double kDragDistance = 10;
static constexpr int kDemiBold = 600;

// --- SecretDialog::Frame ---

class SecretDialog::Frame final : public ui::View
{
public:
    explicit Frame(SecretDialog *dialog)
        : m_dialog(dialog)
    {
        m_close = addChild<ui::HeaderButton>("close", kCloseButton);
        m_close->onClicked = [this] { m_dialog->dismiss(); };
        m_title = addChild<ui::Label>();
        m_title->setFontSize(kTextSize);
        m_title->setWeight(kDemiBold);
        m_title->setWrap(true);
        m_body = addChild<ui::Label>();
        m_body->setFontSize(kTextSize);
        m_body->setFixedLineHeight(kBodyLineHeight);
        m_body->setWrap(true);
        m_cancel = addChild<ui::DialogButton>(tr("Cancel"));
        m_cancel->onClicked = [this] { m_dialog->dismiss(); };
        m_primary = addChild<ui::DialogButton>(std::string(), true);
        m_primary->onClicked = [this] { m_dialog->accept(); };
    }

    ui::Label *title() const { return m_title; }
    ui::Label *body() const { return m_body; }
    ui::DialogButton *cancel() const { return m_cancel; }
    ui::DialogButton *primary() const { return m_primary; }

    void setRows(std::vector<Row> rows)
    {
        m_rows = std::move(rows);
        requestLayout();
    }

    // The window's height: the layout and the margins around it.
    double contentHeight() { return std::ceil(arrange(false)); }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override { arrange(true); }

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
        if (std::hypot(event.position.x - m_pressPosition.x, event.position.y - m_pressPosition.y) < kDragDistance)
            return;
        m_pressed = false;
        if (ShellWindow *window = m_dialog->window(); window && window->shellSurface())
            window->shellSurface()->startMove();
    }

    void pointerRelease(const ui::PointerEvent &) override { m_pressed = false; }

private:
    // FishUI's dialog column: margins 16, spacing 16, the text
    // column (title, 4, body), the rows, the buttons at the right. Returns
    // the height; places the views when `apply`.
    double arrange(bool apply)
    {
        const ui::Theme &theme = ui::Theme::current();
        const double width = kWidth;
        const double inner = width - 2 * kMargin;
        if (apply) {
            m_body->setColor(theme.disabledText);
            m_close->setGeometry({width - kCloseMargin - kCloseButton, kCloseMargin, kCloseButton, kCloseButton});
        }

        double y = kMargin;
        const double titleHeight = m_title->heightForWidth(kTextWidth);
        const double bodyHeight = m_body->heightForWidth(kTextWidth);
        if (apply) {
            m_title->setGeometry({kMargin, y, kTextWidth, titleHeight});
            m_body->setGeometry({kMargin, y + titleHeight + kTitleGap, kTextWidth, bodyHeight});
        }
        y += titleHeight + kTitleGap + bodyHeight;

        for (const Row &row : m_rows) {
            if (!row.view->isVisible())
                continue;
            y += row.spacing;
            const ui::SizeF hint = row.view->sizeHint();
            const double rowWidth = row.fillWidth ? inner : hint.width;
            const double height = row.fillWidth ? row.view->heightForWidth(rowWidth) : hint.height;
            if (apply)
                row.view->setGeometry({kMargin, y, rowWidth, height});
            y += height;
        }

        y += kSpacing;
        const ui::SizeF cancel = m_cancel->sizeHint();
        const ui::SizeF primary = m_primary->isVisible() ? m_primary->sizeHint() : ui::SizeF();
        const double buttonsHeight = std::max(cancel.height, primary.height);
        if (apply) {
            double right = width - kMargin;
            if (m_primary->isVisible()) {
                m_primary->setGeometry({right - primary.width, y + (buttonsHeight - primary.height) / 2,
                                        primary.width, primary.height});
                right -= primary.width + kButtonSpacing;
            }
            m_cancel->setGeometry({right - cancel.width, y + (buttonsHeight - cancel.height) / 2, cancel.width,
                                   cancel.height});
        }
        return y + buttonsHeight + kMargin;
    }

    SecretDialog *m_dialog;
    ui::HeaderButton *m_close;
    ui::Label *m_title;
    ui::Label *m_body;
    ui::DialogButton *m_cancel;
    ui::DialogButton *m_primary;
    std::vector<Row> m_rows;
    bool m_pressed = false;
    ui::PointF m_pressPosition;
};

// --- SecretDialog ---

SecretDialog::SecretDialog(Shell *shell)
    : m_shell(shell)
{
}

SecretDialog::~SecretDialog() = default;

bool SecretDialog::isOpen() const
{
    return m_window && m_window->isShown();
}

ui::DialogButton *SecretDialog::cancelButton() const
{
    return m_frame ? m_frame->cancel() : nullptr;
}

ui::DialogButton *SecretDialog::primaryButton() const
{
    return m_frame ? m_frame->primary() : nullptr;
}

void SecretDialog::open()
{
    if (!m_window) {
        m_window = std::make_unique<ShellWindow>(m_shell, wm::WindowRole::Normal);
        m_window->setDialog(true);
        m_window->setProtectedFromControl(true);
        auto frame = std::make_unique<Frame>(this);
        m_frame = frame.get();
        build(m_frame);
        m_window->setContentView(std::move(frame));
        // Above applications.
        m_window->surface()->setAlwaysOnTop(true);
        m_window->onCloseRequested = [this] { dismiss(); };
        m_window->window()->addShortcut(ui::NoModifier, ui::key::Escape, [this] { dismiss(); });
    }
    refresh();
    if (!m_window->isShown())
        m_window->show();
    m_window->activate();
    if (ui::View *view = initialFocus())
        view->setFocus();
}

void SecretDialog::close()
{
    if (!m_window)
        return;
    m_window->hide();
    // Not from inside its own event handling.
    std::shared_ptr<ShellWindow> doomed(std::move(m_window));
    m_frame = nullptr;
    core::mainLoop()->post([doomed] {});
}

void SecretDialog::dismiss()
{
    if (!isOpen())
        return;
    close();
    if (onDismissed)
        onDismissed();
}

void SecretDialog::setTitle(const std::string &title)
{
    if (!m_frame)
        return;
    m_frame->title()->setText(title);
    if (m_window && m_window->shellSurface())
        m_window->shellSurface()->setTitle(title);
}

void SecretDialog::setText(const std::string &text)
{
    if (m_frame)
        m_frame->body()->setText(text);
}

void SecretDialog::setRows(std::vector<Row> rows)
{
    if (m_frame)
        m_frame->setRows(std::move(rows));
}

void SecretDialog::retranslate()
{
    if (!m_frame)
        return;
    m_frame->cancel()->setText(tr("Cancel"));
    retranslateRows();
    refresh();
}

void SecretDialog::refresh()
{
    if (!m_window || !m_frame)
        return;
    update();
    m_window->setSize(base::Size(int(kWidth), int(m_frame->contentHeight())));
    m_frame->requestLayout();
}

} // namespace Lingmo::Shell
