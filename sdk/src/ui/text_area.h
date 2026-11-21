// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "ui/view.h"

namespace Lingmo::ui {

// A multi-line text area, laid out as Label lays out wrapped text;
// positions are UTF-8 byte offsets. Taller text than the area
// scrolls inside it. Read-only, the text can still be selected and copied.
class TextArea : public View
{
public:
    enum class WrapMode {
        NoWrap,
        // At word boundaries, as Text.WordWrap.
        WordWrap,
        // At any character, as Text.WrapAnywhere, for commands and paths.
        WrapAnywhere,
        // At word boundaries, inside a word that does not fit, as Text.Wrap.
        Wrap,
    };

    explicit TextArea(std::string text = {});
    ~TextArea() override;

    const std::string& text() const;
    // Moves the cursor to the end and clears the undo history.
    void setText(std::string text);
    void setPlaceholder(std::string placeholder);
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;
    // WordWrap unless set.
    void setWrapMode(WrapMode mode);
    // Around the text; 6 on every side unless set.
    void setPadding(const Margins& padding);
    // The line under the text; off for an area inside a card.
    void setFrameVisible(bool visible);
    // Logical pixels; 0 follows the theme.
    void setFontSize(double size);
    // A family (empty for the theme's), a pixel size (0 for the theme's) and
    // a weight, as a label's font for an editor over it.
    void setFont(const std::string& family, double pixelSize, int weight);

    enum class HorizontalAlignment {
        Left,
        Center,
        Right,
    };
    // Each line placed in the width by its advance.
    void setHorizontalAlignment(HorizontalAlignment alignment);
    // Text shorter than the area sits in its middle.
    void setVerticallyCentered(bool centered);
    // The text and caret; unset follows the theme.
    void setTextColor(std::optional<Color> color);
    // The selection's background and text colours; unset, the theme's.
    void setSelectionColors(std::optional<Color> background, std::optional<Color> text);
    // The selection stays drawn without the focus.
    void setPersistentSelection(bool persistent);

    size_t cursorPosition() const;
    void setCursorPosition(size_t position);
    void select(size_t anchor, size_t cursor);
    void selectAll();
    bool hasSelection() const;
    std::string selectedText() const;
    // What the input method is composing, shown at the cursor.
    const std::string& preedit() const;

    void cut();
    void copy();
    void paste();
    void undo();
    void redo();

    SizeF sizeHint() const override;
    double heightForWidth(double width) const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Text; }

    std::function<void()> onTextChanged;

protected:
    void paint(Painter& painter) override;
    void pointerPress(const PointerEvent& event) override;
    void pointerMove(const PointerEvent& event) override;
    void pointerRelease(const PointerEvent& event) override;
    void focusIn(FocusReason reason) override;
    void focusOut() override;
    bool keyPress(const KeyEvent& event) override;
    bool scroll(const ScrollEvent& event) override;
    bool acceptsTextInput() const override;
    TextInputState textInputState() const override;
    void textInput(const TextInputEvent& event) override;
    void layout() override;

private:
    struct Private;

    size_t offsetAt(PointF position) const;
    void showContextMenu(PointF position);
    void edited();
    void moved();
    void startBlinking();

    std::unique_ptr<Private> d;
};

} // namespace Lingmo::ui
