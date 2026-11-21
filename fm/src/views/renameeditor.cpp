// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/renameeditor.h"
#include "views/style.h"

#include "base/i18n.h"
#include "base/strings.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/text_area.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

using Lingmo::i18n::tr;

namespace fm {

static constexpr double kPadding = 4;

class RenameField : public ui::TextArea
{
public:
    explicit RenameField(RenameEditor* editor)
        : m_editor(editor)
    {
        setFrameVisible(false);
        setPadding({kPadding, kPadding, kPadding, kPadding});
    }

protected:
    bool keyPress(const ui::KeyEvent& event) override
    {
        if (event.key == ui::key::Return || event.key == ui::key::KeypadEnter) {
            // Return while composing belongs to the input method.
            if (preedit().empty())
                m_editor->commit();
            return true;
        }
        if (event.key == ui::key::Escape) {
            m_editor->cancel();
            return true;
        }
        if (event.key == ui::key::Tab || event.key == style::key::Backtab) {
            m_editor->commit();
            return true;
        }
        return TextArea::keyPress(event);
    }

private:
    RenameEditor* m_editor;
};

RenameEditor::RenameEditor(std::string name, int selectionEnd, bool centered)
    : m_original(std::move(name))
    , m_selectionEnd(selectionEnd)
    , m_centered(centered)
{
    setName("renameEditor");
    m_field = addChild<RenameField>(this);
    m_field->setWrapMode(centered ? ui::TextArea::WrapMode::Wrap : ui::TextArea::WrapMode::NoWrap);
    m_field->setHorizontalAlignment(centered ? ui::TextArea::HorizontalAlignment::Center
                                             : ui::TextArea::HorizontalAlignment::Left);
    m_field->setVerticallyCentered(true);
    m_field->setSelectionColors(std::nullopt, ui::Color::rgb(0xffffff));
    m_field->setPersistentSelection(true);
    m_field->onTextChanged = [this] {
        if (!m_error.empty())
            setError({});
        place();
    };
}

RenameEditor::~RenameEditor() = default;

void RenameEditor::setFont(double pixelSize, int weight)
{
    m_field->setFont({}, pixelSize, weight);
    place();
}

void RenameEditor::setTarget(const ui::RectF& label, double viewWidth)
{
    m_label = label;
    m_viewWidth = viewWidth;
    place();
}

void RenameEditor::place()
{
    const ui::RectF& label = m_label;
    double width;
    double x;
    if (m_centered) {
        const double center = label.x + label.width / 2;
        // Shrinks at the edges without leaving the icon's centre.
        width = std::max(0.0, std::min(label.width + kPadding * 2,
                                       2 * std::min(center - 4, m_viewWidth - center - 4)));
        x = center - width / 2;
    } else {
        width = std::max(0.0, std::min(m_viewWidth - 8, std::max(120.0, label.width + 8)));
        x = std::max(4.0, std::min(m_viewWidth - width - 4, label.x - kPadding));
    }
    setGeometry({x, label.y - kPadding, width, m_field->heightForWidth(width)});
}

void RenameEditor::layout()
{
    m_field->setGeometry(localRect());
}

void RenameEditor::paint(ui::Painter& painter)
{
    const ui::Theme& theme = ui::Theme::current();
    const ui::Color border = m_error.empty() ? theme.highlight : style::kDanger;
    painter.drawRoundedRect(localRect(), {4, 0, ui::Corners::Circular}, theme.background, 1, border);
    // A parallel curve of the field, so the ring snaps where the field does.
    painter.save();
    painter.multiplyOpacity(0.25);
    painter.drawRoundedRect(localRect().adjusted(-2, -2, 2, 2), {4, -2, ui::Corners::Circular},
                            ui::Color {0, 0, 0, 0}, 2, border);
    painter.restore();
}

void RenameEditor::begin()
{
    m_field->setText(m_original);
    m_field->setFocus();
    m_field->select(0, size_t(std::clamp(m_selectionEnd, 0, int(m_original.size()))));
    place();
}

void RenameEditor::setError(std::string message)
{
    m_error = std::move(message);
    if (ui::Window* window = this->window()) {
        if (m_error.empty())
            window->unpinToolTip(this);
        else
            window->pinToolTip(this, localRect(), m_error);
    }
    requestPaint();
}

bool RenameEditor::commit()
{
    if (m_finishing)
        return true;

    const std::string text = m_field->text();
    // An emptied name keeps the old one, as Escape does.
    if (base::strings::trimmed(text).empty()) {
        cancel();
        return true;
    }

    if (text == "." || text == "..")
        setError(tr("Choose a name other than . or .."));
    else if (text.find_first_of(std::string_view("/\r\n\0", 4)) != std::string::npos)
        setError(tr("Names cannot contain / or line breaks."));
    else
        setError({});

    if (!m_error.empty()) {
        m_field->setFocus();
        return false;
    }

    m_finishing = true;
    if (text != m_original && onSubmitted)
        onSubmitted(text);
    finish();
    return true;
}

void RenameEditor::cancel()
{
    if (m_finishing)
        return;
    m_finishing = true;
    finish();
}

void RenameEditor::finish()
{
    if (ui::Window* window = this->window())
        window->unpinToolTip(this);
    if (onFinished)
        onFinished();
}

} // namespace fm
