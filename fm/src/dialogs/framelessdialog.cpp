// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/framelessdialog.h"

#include "base/event_loop.h"
#include "ui/header_button.h"
#include "ui/label.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/theme.h"

#include <cmath>

namespace fm {

class FramelessDialog::Root : public ui::View
{
public:
    explicit Root(FramelessDialog* dialog, bool closeButton)
        : m_dialog(dialog)
    {
        if (closeButton) {
            m_close = addChild<ui::HeaderButton>("close", 32);
            m_close->onClicked = [this] {
                if (m_dialog->onCloseRequested)
                    m_dialog->onCloseRequested();
            };
        }
    }

    void setHeaderTitle(const std::string& text)
    {
        if (!m_title) {
            m_title = addChild<ui::Label>();
            m_title->setFontSize(14);
            m_title->setVerticallyCentered(true);
        }
        m_title->setText(text);
        m_title->setColor(ui::ColorId::Text);
        requestLayout();
    }

    void setContent(std::unique_ptr<ui::View> content, double width, double padding)
    {
        if (m_content) {
            std::shared_ptr<View> old = takeChild(m_content);
        }
        m_content = content.get();
        addChild(std::move(content));
        if (m_close)
            raiseChild(m_close);
        m_width = width;
        m_horizontal = m_top = m_bottom = padding;
    }

    void setGeometryRules(double horizontal, double top, double bottom, double header, double buttonMargin)
    {
        m_horizontal = horizontal;
        m_top = top;
        m_bottom = bottom;
        m_header = header;
        m_buttonMargin = buttonMargin;
    }

    double fittedHeight() const
    {
        return m_content ? std::ceil(m_content->heightForWidth(m_width - m_horizontal * 2)) + m_header + m_top + m_bottom
                         : 0;
    }

    double width() const { return m_width; }
    void fitWidth()
    {
        if (m_content)
            m_width = std::ceil(m_content->sizeHint().width) + m_horizontal * 2;
    }
    void setContentWidth(double width) { m_width = width; }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double width = geometry().width - m_horizontal * 2;
        if (m_content)
            m_content->setGeometry({m_horizontal, m_header + m_top, width, m_content->heightForWidth(width)});
        if (m_close)
            m_close->setGeometry({geometry().width - m_buttonMargin - 32, m_buttonMargin, 32, 32});
        if (m_title)
            m_title->setGeometry({16, m_buttonMargin, std::max(0.0, geometry().width - 16 - 2 * m_buttonMargin - 32), 32});
    }

    // Dragging anywhere moves the window.
    void pointerPress(const ui::PointerEvent& event) override
    {
        if (event.button == ui::MouseButton::Left && window())
            window()->startMove();
    }

private:
    FramelessDialog* m_dialog;
    ui::View* m_content = nullptr;
    ui::HeaderButton* m_close = nullptr;
    ui::Label* m_title = nullptr;
    double m_width = 480;
    double m_horizontal = 16;
    double m_top = 16;
    double m_bottom = 16;
    double m_header = 0;
    double m_buttonMargin = 8;
};

FramelessDialog::FramelessDialog(std::string title, bool closeButton)
    : m_title(std::move(title))
    , m_closeButton(closeButton)
{
}

FramelessDialog::~FramelessDialog() = default;

void FramelessDialog::setContentFactory(std::function<std::unique_ptr<ui::View>()> factory, double width,
                                        double padding)
{
    m_factory = std::move(factory);
    m_width = width;
    m_padding = padding;
}

void FramelessDialog::setWidth(double width)
{
    m_width = width;
    fit();
}

void FramelessDialog::setHeader(double height, double buttonMargin)
{
    m_header = height;
    m_buttonMargin = buttonMargin;
}

void FramelessDialog::setHeaderTitle(std::string title)
{
    m_headerTitle = std::move(title);
    if (m_root)
        m_root->setHeaderTitle(m_headerTitle);
    if (m_window)
        m_window->setTitle(m_headerTitle);
}

void FramelessDialog::setPadding(double horizontal, double top, double bottom)
{
    m_horizontal = horizontal;
    m_top = top;
    m_bottom = bottom;
}

void FramelessDialog::fit()
{
    if (!m_window)
        return;
    if (m_width > 0)
        m_root->setContentWidth(m_width);
    const ui::SizeF size {m_root->width(), m_root->fittedHeight()};
    m_window->setMinimumSize(size);
    m_window->setMaximumSize(size);
    m_window->resize(size);
    m_root->requestLayout();
}

void FramelessDialog::show()
{
    if (m_window) {
        fit();
        m_window->activate();
        return;
    }
    m_window = std::make_unique<ui::Window>();
    m_window->setTitle(m_title);
    if (m_parent)
        m_window->setTransientParent(m_parent, m_modal);
    auto root = std::make_unique<Root>(this, m_closeButton);
    m_root = root.get();
    m_window->setContentView(std::move(root));
    m_window->setBackgroundColor(ui::ColorId::SecondBackground);
    m_window->onCloseRequested = [this] {
        if (onCloseRequested)
            onCloseRequested();
    };
    m_window->addShortcut(ui::NoModifier, ui::key::Escape, [this] {
        if (onCloseRequested)
            onCloseRequested();
    });
    if (m_factory)
        m_root->setContent(m_factory(), m_width, m_padding);
    m_root->setGeometryRules(m_horizontal < 0 ? m_padding : m_horizontal, m_top < 0 ? m_padding : m_top,
                             m_bottom < 0 ? m_padding : m_bottom, m_header, m_buttonMargin);
    // No width given: the content's own implicit width.
    if (!m_headerTitle.empty())
        m_root->setHeaderTitle(m_headerTitle);
    if (m_width <= 0)
        m_root->fitWidth();
    fit();
    m_window->show();
}

void FramelessDialog::setTitle(std::string title)
{
    m_title = std::move(title);
    if (m_window)
        m_window->setTitle(m_title);
}

void FramelessDialog::rebuildContent()
{
    if (!m_window || !m_factory)
        return;
    m_root->setContent(m_factory(), m_width, m_padding);
    m_root->setGeometryRules(m_horizontal < 0 ? m_padding : m_horizontal, m_top < 0 ? m_padding : m_top,
                             m_bottom < 0 ? m_padding : m_bottom, m_header, m_buttonMargin);
    if (m_width <= 0)
        m_root->fitWidth();
    fit();
}

void FramelessDialog::hide()
{
    if (!m_window)
        return;
    // Not from inside the window's own event handling.
    std::shared_ptr<ui::Window> gone(std::move(m_window));
    m_root = nullptr;
    base::EventLoop::current()->post([gone] {});
}

} // namespace fm
