// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Lingmo::ui {

// The text of an editor with its cursor and selection. Positions are UTF-8
// byte offsets that always sit on grapheme boundaries, so a cursor never
// splits an accented letter, an emoji sequence or a Hangul syllable.
class EditableText
{
public:
    EditableText();

    const std::string& text() const { return m_text; }
    // Counts the changes of the text.
    uint64_t revision() const { return m_revision; }
    // Clears the selection and the undo history.
    void setText(std::string text);

    size_t cursor() const { return m_cursor; }
    size_t anchor() const { return m_anchor; }
    bool hasSelection() const { return m_cursor != m_anchor; }
    size_t selectionStart() const { return std::min(m_cursor, m_anchor); }
    size_t selectionEnd() const { return std::max(m_cursor, m_anchor); }
    std::string selectedText() const;

    // Snapped to the grapheme boundary at or before the position.
    void setCursor(size_t position, bool keepAnchor = false);
    void select(size_t anchor, size_t cursor);
    void selectAll();

    // Replaces the selection, which may be empty. Typing in a row undoes as one.
    // With a maximum length in UTF-16 units, the inserted text is cut back
    // by graphemes to fit; nothing replacing nothing records nothing.
    void insert(std::string_view text, bool typing = false, size_t maximumLength = 0);
    void erase(size_t start, size_t end);
    // Delete the selection, or the grapheme or word next to the cursor.
    void deleteBackward(bool word);
    void deleteForward(bool word);

    size_t previousGrapheme(size_t position) const;
    size_t nextGrapheme(size_t position) const;
    // The start of the word before the position, past any spaces.
    size_t previousWordStart(size_t position) const;
    // The end of the word after the position, past any spaces.
    size_t nextWordEnd(size_t position) const;
    // The word, or run of spaces, around the position.
    std::pair<size_t, size_t> wordAt(size_t position) const;
    size_t snapped(size_t position) const;

    bool canUndo() const { return m_undoIndex > 0; }
    bool canRedo() const { return m_undoIndex + 1 < m_history.size(); }
    void undo();
    void redo();

private:
    struct State
    {
        std::string text;
        size_t cursor;
        size_t anchor;
    };

    enum class Edit {
        None,
        Typing,
        Other,
    };

    void replace(size_t start, size_t end, std::string_view text, Edit edit, size_t maximumLength = 0);
    // How much of the text, cut on a grapheme boundary, replaces the range
    // without the whole going over the maximum length.
    size_t fittingLength(std::string_view text, size_t start, size_t end, size_t maximumLength) const;
    void updateBoundaries();
    bool isSpaceAt(size_t position) const;
    void record(Edit edit);

    std::string m_text;
    uint64_t m_revision = 0;
    size_t m_cursor = 0;
    size_t m_anchor = 0;
    // One flag per byte and one past the end.
    std::vector<bool> m_graphemeStarts;
    std::vector<bool> m_wordBoundaries;

    std::vector<State> m_history;
    size_t m_undoIndex = 0;
    Edit m_lastEdit = Edit::None;
};

} // namespace Lingmo::ui
