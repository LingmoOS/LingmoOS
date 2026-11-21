// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include "codelayout.h"
#include "document.h"
#include "syntaxtheme.h"

#include "base/elapsed_timer.h"
#include "base/signal.h"
#include "base/timer.h"
#include "ui/glyph_run.h"
#include "ui/scroller.h"
#include "ui/view.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Lingmo::ui {
class ScrollBar;
}

// Prefix sums of the visual line counts, for mapping between document lines
// and their vertical positions in O(log n).
class VisualLineIndex
{
public:
    void build(const std::vector<int> &counts);
    void add(int line, int delta);
    // Visual lines before `line`.
    int64_t before(int line) const;
    int64_t total() const { return m_total; }
    // The line containing visual line `visual`; `offset` is its index within it.
    int lineAt(int64_t visual, int *offset) const;
    int size() const { return m_size; }

private:
    std::vector<int64_t> m_tree;
    int m_size = 0;
    int64_t m_total = 0;
};

// Draws a Document and edits it: only the lines in view are laid out, so a
// file of any size scrolls and types as fast as a short one. It scrolls
// itself, with the shared touchpad physics, wheel steps and overlay scroll
// bars.
class EditorView : public Lingmo::ui::View
{
public:
    EditorView();
    ~EditorView() override;

    Document *document() const { return m_document; }
    void setDocument(Document *document);
    // The family empty takes the system's monospace font.
    void setFont(const Lingmo::ui::FontSpec &font);
    const Lingmo::ui::FontSpec &font() const { return m_font; }
    bool wordWrap() const { return m_wordWrap; }
    void setWordWrap(bool wrap);
    bool lineNumbers() const { return m_lineNumbers; }
    void setLineNumbers(bool show);
    void setHighlightCurrentLine(bool highlight);
    void setShowWhitespace(bool show);
    void setMatchBrackets(bool match);
    bool autoIndent() const { return m_autoIndent; }
    void setAutoIndent(bool indent);
    bool overwriteMode() const { return m_overwrite; }
    void setOverwriteMode(bool overwrite);
    void setDarkMode(bool dark);
    void setAccentColor(Lingmo::ui::Color color);
    Lingmo::ui::Color backgroundColor() const { return m_theme.background; }
    void setSidePadding(double padding);
    // Scroll bars drawn for the text's background: light on a dark one.
    void setScrollBarDarkMode(bool dark);
    // A panel floating over the top right of the view, from `left` to the
    // right edge and `height` down: the cursor is revealed clear of it,
    // scrolling above the first line when it has to.
    void setOcclusion(double left, double height);

    double contentX() const { return m_contentX; }
    double contentY() const { return m_contentY; }
    void setContentPosition(double x, double y);
    double contentWidth() const { return m_contentWidth; }
    double contentHeight() const { return m_contentHeight; }
    double lineHeight() const { return m_lineHeight; }
    double gutterWidth() const;
    // In the view's coordinates.
    Lingmo::ui::RectF cursorRectangle() const { return m_caret; }

    void cut();
    void copy();
    void paste();
    void ensureCursorVisible(bool center = false);
    // The text position under a point of this view.
    TextPosition positionAt(Lingmo::ui::PointF point);
    // Scrolls so `line` (from 0) is at the top, e.g. when a session is restored.
    void scrollToLine(double line);
    double topLine() const;
    // A press over the text, as the context menu opens from one.
    void typeText(std::string_view text);

    // Lays out what is in view now, as a frame would; for tests.
    void updateNow();

    Lingmo::base::Signal<> optionsChanged;
    Lingmo::base::Signal<> overwriteModeChanged;
    Lingmo::base::Signal<> metricsChanged;
    Lingmo::base::Signal<> scrolled;
    std::function<void(Lingmo::ui::PointF position)> onContextMenuRequested;
    std::function<void(int step)> onZoomRequested;
    std::function<void(const std::vector<std::string> &paths)> onFilesDropped;

    Lingmo::ui::CursorShape cursorShape() const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter &painter) override;
    void pointerPress(const Lingmo::ui::PointerEvent &event) override;
    void pointerMove(const Lingmo::ui::PointerEvent &event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent &event) override;
    void hoverMove(const Lingmo::ui::PointerEvent &event) override;
    bool keyPress(const Lingmo::ui::KeyEvent &event) override;
    bool scroll(const Lingmo::ui::ScrollEvent &event) override;
    bool gesture(const Lingmo::ui::GestureEvent &event) override;
    void focusIn(Lingmo::ui::FocusReason reason) override;
    void focusOut() override;
    bool acceptsTextInput() const override;
    Lingmo::ui::TextInputState textInputState() const override;
    void textInput(const Lingmo::ui::TextInputEvent &event) override;
    bool canDrop(const Lingmo::ui::DropOffer &offer) const override;
    void dragEnter(const Lingmo::ui::DragEvent &event) override;
    uint32_t dragUpdate(const Lingmo::ui::DragEvent &event) override;
    void dragLeave() override;
    uint32_t drop(const Lingmo::ui::DragEvent &event) override;

private:
    class ScrollTarget;

    struct Segment
    {
        int start = 0;
        int length = 0;
        int firstVisual = 0;
        std::unique_ptr<CodeLayout> layout;
    };

    // The laid-out form of a line in or near view.
    struct LineCache
    {
        std::vector<Segment> segments;
        int visualLines = 1;
        double width = 0;
        bool provisionalSyntax = false;
        // No colours or input method text: segments can be reused as they are.
        bool plain = true;
        int preeditColumn = -1;
    };

    struct LineInfo
    {
        uint32_t id = 0;
        int visualLines = 1;
        int columns = 0;
        bool exact = false;
    };

    struct PlannedLine
    {
        int line;
        uint32_t id;
        double top;
        LineCache *cache;
    };

    struct Rect
    {
        Lingmo::ui::RectF rect;
        Lingmo::ui::Color color;
    };

    void onLinesChanged(int line, int removed, int added);
    void onSyntaxChanged(int first, int last);
    void onCursorChanged();
    void invalidateLayouts(bool estimates);
    void updateMetrics();
    void updateTheme();
    void polish();
    void plan();

    int estimateVisualLines(const std::string &text, int columns) const;
    int displayColumns(const std::string &text) const;
    double wrapWidth() const;
    double textLeft() const;
    double toPixel(double value) const;
    LineCache *ensureLayout(int line);
    void buildLayout(int line, LineCache *cache, LineCache *previous = nullptr);
    void setLineVisualCount(int line, int count);
    void ensureIndex();
    void recountWidth();
    void dropFarCaches(int first, int last);
    void computeExactHeights();

    double lineTop(int line) const;
    bool locate(const TextPosition &position, LineCache **cache, const Segment **segment, int *textLine, int *visual);
    Lingmo::ui::RectF caretRect(const TextPosition &position);
    TextPosition hitTest(Lingmo::ui::PointF point);
    std::vector<Lingmo::ui::RectF> rangeRects(int line, LineCache *cache, int from, int to, bool toLineEnd);
    void addWhitespaceMarks(int line, LineCache *cache);
    int columnInLayout(const LineCache *cache, const Segment &segment, const TextPosition &position) const;

    void moveCursor(const TextPosition &position, bool select, bool keepPreferredX = false);
    TextPosition moveVertically(const TextPosition &from, int lines);
    TextPosition visualLineEdge(const TextPosition &from, bool end);
    TextRange unitAt(const TextPosition &position) const;
    void selectByUnit(const TextPosition &position);
    void autoScroll();
    void updatePrimarySelection();
    void resetBlink();
    void applyScroll(double x, double y);
    void updateScrollBars();
    void startTextDrag();

    Document *m_document = nullptr;
    std::vector<Lingmo::base::Subscription> m_connections;
    Lingmo::ui::FontSpec m_font;
    std::shared_ptr<CodeFont> m_codeFont;
    bool m_wordWrap = true;
    bool m_lineNumbers = true;
    bool m_highlightCurrentLine = false;
    bool m_showWhitespace = false;
    bool m_matchBrackets = true;
    bool m_autoIndent = true;
    bool m_overwrite = false;
    bool m_dark = false;
    Lingmo::ui::Color m_accent = Lingmo::ui::Color::rgb(0x3385FF);
    SyntaxTheme m_theme;
    double m_sidePadding = 12;
    double m_occludedLeft = 0;
    double m_occludedHeight = 0;

    double m_contentX = 0;
    double m_contentY = 0;
    double m_contentWidth = 0;
    double m_contentHeight = 0;

    double m_dpr = 1;
    double m_lineHeight = 20;
    double m_charWidth = 8;
    double m_digitWidth = 8;
    double m_spaceWidth = 8;
    double m_ascent = 14;
    double m_lineInset = 3;
    int m_maxColumns = 0;
    bool m_maxColumnsDirty = false;
    double m_widestLaidOut = 0;
    double m_laidOutWidth = -1;

    std::vector<LineInfo> m_lines;
    uint32_t m_nextId = 1;
    VisualLineIndex m_index;
    bool m_indexDirty = true;
    std::unordered_map<uint32_t, std::unique_ptr<LineCache>> m_caches;
    // Long lines just edited: their unchanged segments are reused.
    std::unordered_map<uint32_t, std::unique_ptr<LineCache>> m_staleCaches;
    Lingmo::base::Timer m_exactTimer;
    int m_exactCursor = 0;

    bool m_planned = false;
    std::vector<PlannedLine> m_plan;
    std::unordered_map<int, std::unique_ptr<CodeLayout>> m_numberLayouts;
    std::vector<Rect> m_underlay;
    Lingmo::ui::RectF m_caret;
    Lingmo::ui::RectF m_dropCaret;
    int m_currentLineNumber = -1;

    // The x the cursor keeps while moving up and down.
    double m_preferredX = -1;
    std::string m_preedit;
    // Bytes into the preedit; negative hides the cursor.
    int m_preeditCursor = 0;

    bool m_caretVisible = true;
    Lingmo::base::Timer m_blinkTimer;
    Lingmo::base::ElapsedTimer m_blinkStarted;
    Lingmo::base::Timer m_autoScrollTimer;
    Lingmo::ui::PointF m_lastMouse;
    bool m_selecting = false;
    int m_selectUnit = 0;
    TextRange m_unitAnchor;
    Lingmo::ui::PointF m_pressPos;
    bool m_maybeDrag = false;
    bool m_dragging = false;
    Lingmo::base::Timer m_primaryTimer;
    double m_pinch = 1;
    double m_pinchAccumulated = 0;

    std::unique_ptr<ScrollTarget> m_scrollTarget;
    std::unique_ptr<Lingmo::ui::Scroller> m_scroller;
    Lingmo::ui::ScrollBar *m_verticalBar;
    Lingmo::ui::ScrollBar *m_horizontalBar;
    // Clipboard and drop answers come from other clients, maybe after the view is gone.
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

#endif
