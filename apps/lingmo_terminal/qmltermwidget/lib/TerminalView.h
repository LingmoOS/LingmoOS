/*
    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "ui/scroller.h"
#include "ui/view.h"

#include "Character.h"
#include "CharacterColor.h"
#include "Emulation.h"
#include "Filter.h"
#include "KeyCodes.h"

namespace Konsole
{

class ScreenWindow;
class TerminalSession;
class TerminalImageFilterChain;
class TerminalFont;

// The cell metrics of the terminal font at a scale, as QFontMetrics gave
// them to the display.
struct CellMetrics
{
    int width = 1;
    int height = 1;
    int ascent = 1;
    int descent = 0;
    double underlinePos = 1;
    double strikeOutPos = 1;
    double overlinePos = 1;
    double lineWidth = 1;
    bool fixedPitch = true;
};

// A colour scheme as the window chrome reads it.
struct ColorSchemeInfo
{
    bool valid = false;
    std::string description;
    Color background;
    Color foreground;
    // The eight colours, then their intense versions.
    std::array<Color, 16> colors{};
};

// What Konsole's TerminalDisplay and the QML terminal item did: shows a
// ScreenWindow of a session's emulation, sends it keys, the mouse and pastes,
// and keeps the selection, the scrollback position and the cursor.
class TerminalView : public Lingmo::ui::View, public Lingmo::ui::Scroller::Target
{
public:
    enum BellMode
    {
        SystemBeepBell = 0,
        NotifyBell = 1,
        VisualBell = 2,
        NoBell = 3
    };

    TerminalView();
    ~TerminalView() override;

    void setSession(TerminalSession* session);
    TerminalSession* session() const { return _session; }

    // Bundled schemes, by file name; an unknown one gives the default table.
    void setColorScheme(const std::string& name);
    const std::string& colorScheme() const { return _colorScheme; }
    static ColorSchemeInfo colorSchemeInfo(const std::string& name);

    void setTerminalFont(const std::string& family, double pointSize);
    void setBlinkingCursor(bool blink);
    bool isCursorBlinking() const { return _blinkCursorTimer.isActive(); }
    void setTerminalCursorShape(int shape);
    void setBellMode(int mode) { _bellMode = mode; }
    void setConfirmMultilinePaste(bool confirm) { _confirmMultilinePaste = confirm; }
    void setFullCursorHeight(bool full);
    // The cells' default background over the window's; 0 leaves it to the window.
    void setBackgroundOpacity(double opacity);

    int lines() const { return _lines; }
    int columns() const { return _columns; }
    const CellMetrics& cellMetrics() const { return _metrics; }
    // The program in front asked for the mouse.
    bool terminalUsesMouse() const { return !_mouseMarks; }

    int scrollbarValue() const { return _scrollValue; }
    int scrollbarMinimum() const { return 0; }
    int scrollbarMaximum() const { return _scrollMaximum; }
    void setScrollbarValue(int value);

    void selectAll();
    std::string selectedText() const;
    bool hasSelection() const;
    void copyClipboard();
    void pasteClipboard();
    void pasteSelection();
    void pasteText(const std::string& text, bool appendReturn = false);
    // Text dropped on the view, sent as it is once confirmed.
    void dropText(const std::string& text);
    // The paste multilinePasteRequested held back.
    void confirmPaste();
    bool isSelectedAt(double x, double y);
    bool selectWordAt(double x, double y);

    // The link under the point, opened as it would be; empty for none.
    std::string linkAt(double x, double y);
    struct Segment
    {
        double x;
        double y;
        double width;
    };
    // Where the link under the point is underlined, one segment per line.
    std::vector<Segment> linkUnderlineAt(double x, double y);

    void find(const std::string& text, bool forwards, bool fromSelection, bool caseSensitive);
    void clearFind();

    // The mouse as the item's MouseArea passed it on: positions in the view.
    void simulateMousePress(double x, double y, Lingmo::ui::MouseButton button, uint32_t modifiers);
    void simulateMouseMove(double x, double y, uint32_t buttons, uint32_t modifiers);
    void simulateMouseRelease(double x, double y, Lingmo::ui::MouseButton button, uint32_t modifiers);
    void simulateMouseDoubleClick(double x, double y, Lingmo::ui::MouseButton button, uint32_t modifiers);

    // Where the pointer is, for the wheel's position.
    void setPointerPosition(Lingmo::ui::PointF position) { _lastPointer = position; }

    // A key the window's own shortcuts left, as Qt's QKeyEvent had it.
    void sendKeyEvent(KeyEvent& event);
    static KeyEvent keyEventFor(const Lingmo::ui::KeyEvent& event);

    Lingmo::base::Signal<> notifyBell;
    Lingmo::base::Signal<const std::string&> multilinePasteRequested;
    Lingmo::base::Signal<> scrollbarParamsChanged;
    Lingmo::base::Signal<> scrollbarValueChanged;
    Lingmo::base::Signal<> usesMouseChanged;
    Lingmo::base::Signal<bool> copyAvailable;
    Lingmo::base::Signal<bool> findResult;
    Lingmo::base::Signal<> sizeChanged;
    Lingmo::base::Signal<> fontMetricsChanged;

    // What the item's MouseArea and key handler in the page did around it:
    // the window's shortcuts see keys first, a right click (or Shift+right
    // click under a program using the mouse) asks for the menu on release,
    // and a Ctrl+click on a link opens it on release.
    std::function<bool(const Lingmo::ui::KeyEvent&)> onShortcut;
    std::function<void(double x, double y)> onContextMenu;
    std::function<void(const std::string& url)> onLinkActivated;
    // The colour links under the pointer are underlined in.
    void setLinkColor(Color color);

    bool acceptsFocus() const override { return true; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    void pointerPress(const Lingmo::ui::PointerEvent& event) override;
    void pointerMove(const Lingmo::ui::PointerEvent& event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent& event) override;
    void hoverMove(const Lingmo::ui::PointerEvent& event) override;
    void hoverChanged() override;
    bool keyRelease(const Lingmo::ui::KeyEvent& event) override;
    void focusIn(Lingmo::ui::FocusReason reason) override;
    void focusOut() override;
    bool keyPress(const Lingmo::ui::KeyEvent& event) override;
    bool scroll(const Lingmo::ui::ScrollEvent& event) override;
    bool acceptsTextInput() const override { return true; }
    Lingmo::ui::TextInputState textInputState() const override;
    void textInput(const Lingmo::ui::TextInputEvent& event) override;

private:
    // Scroller::Target, the scrollback in pixels on the vertical axis.
    Lingmo::ui::Scroller::Axis horizontalAxis() const override;
    Lingmo::ui::Scroller::Axis verticalAxis() const override;
    Lingmo::ui::PointF position() const override;
    void applyPosition(Lingmo::ui::PointF position) override;

    void setScreenWindow(ScreenWindow* window);
    void updateFontMetrics();
    void setColorTable(const ColorEntry table[]);
    void updateImage();
    void updateLineProperties();
    void updateImageSize();
    void makeImage();
    void clearImage();
    void calcGeometry();
    void propagateSize();
    void setScroll(int cursor, int lines);
    void scrollBarPositionChanged();
    void setScrollBarValueQuietly(int value);
    int cursorLineInWindow() const;
    void revealCursor();
    void scrollToEnd();
    void setScrollOffset(double offset);
    double scrollPixelPosition() const;
    double scrollPixelMaximum() const;
    void setScrollPixelPosition(double position);
    int drawnLines() const;
    void blinkEvent();
    void blinkCursorEvent();
    void blinkCursorTimeout();
    bool hasKeyboard() const;
    void restartCursorBlink();
    void bell();
    void swapColorTable();
    void setUsesMouse(bool on);
    void selectionChanged();

    void getCharacterPosition(Lingmo::ui::PointF point, int& line, int& column) const;
    void getCharacterCellAt(Lingmo::ui::PointF point, int& line, int& column) const;
    void extendSelection(Lingmo::ui::PointF position);
    void findWordBounds(int column, int line, int& beginColumn, int& beginLine, int& endColumn, int& endLine) const;
    char32_t charClass(const Character& ch) const;
    void mouseTripleClick(Lingmo::ui::PointF position);
    void doDrag();
    void setSelection(const std::string& text);
    void emitSelection(bool useXselection, bool appendReturn);
    void sendPaste(std::u16string text, bool appendReturn);
    void bracketText(std::u16string& text) const;
    void sendMouse(int button, int column, int line, int eventType);
    void keyPressed(KeyEvent& event, bool fromPaste);
    UrlFilter::HotSpot* linkHotSpotAt(TerminalImageFilterChain& chain, double x, double y);

    TerminalSession* _session = nullptr;
    ScreenWindow* _screenWindow = nullptr;
    std::vector<Lingmo::base::Subscription> _windowConnections;
    std::vector<Lingmo::base::Subscription> _sessionConnections;

    std::string _colorScheme;
    ColorEntry _colorTable[TABLE_COLORS];
    bool _colorsInverted = false;
    double _opacity = 1;

    std::unique_ptr<TerminalFont> _font;
    std::string _fontFamily = "Monospace";
    double _fontPointSize = 12;
    double _metricsScale = 0;
    CellMetrics _metrics;
    int _lineSpacing = 0;
    bool _boldIntense = true;
    bool _italicEnabled = true;
    bool _drawLineChars = true;
    bool _fullCursorHeight = false;
    int _cursorShape = 0;

    int _leftBaseMargin = 1;
    int _topBaseMargin = 1;
    int _leftMargin = 1;
    int _topMargin = 1;
    int _lines = 1;
    int _columns = 1;
    int _usedLines = 1;
    int _usedColumns = 1;
    int _contentHeight = 1;
    int _contentWidth = 1;

    std::vector<Character> _image;
    int _imageSize = 0;
    std::vector<LineProperty> _lineProperties;
    bool _hasLineBelow = false;

    // The scroll bar Konsole kept internally: the window's first line.
    int _scrollValue = 0;
    int _scrollMaximum = 0;
    double _scrollOffset = 0;
    bool _applyingScroll = false;
    bool _followCursor = false;
    std::unique_ptr<Lingmo::ui::Scroller> _scroller;
    int _wheelPixelRemainder = 0;
    int _wheelAngleRemainder = 0;
    Lingmo::ui::PointF _lastPointer;

    // Selection.
    bool _mouseMarks = true;
    bool _bracketedPasteMode = false;
    int _actSel = 0;
    Lingmo::base::Point _iPntSel;
    Lingmo::base::Point _pntSel;
    Lingmo::base::Point _tripleSelBegin;
    bool _wordSelectionMode = false;
    bool _lineSelectionMode = false;
    bool _preserveLineBreaks = true;
    bool _columnSelectionMode = false;
    bool _possibleTripleClick = false;
    Lingmo::base::Timer _tripleClickTimer;
    std::u16string _wordCharacters = u":@-./_~";

    // A press on selected text drags it out once the pointer moves far enough.
    enum DragState { DragNone, DragPending, DragDragging };
    DragState _dragState = DragNone;
    Lingmo::ui::PointF _dragStart;

    // Paste.
    bool _confirmMultilinePaste = false;
    bool _trimPastedTrailingNewlines = false;
    std::u16string _pendingPaste;
    bool _pendingPasteAppendReturn = false;
    bool _pendingPasteIsDrop = false;

    // Blinking.
    bool _blinking = false;
    bool _hasBlinker = false;
    bool _cursorBlinking = false;
    bool _hasBlinkingCursor = false;
    Lingmo::base::Timer _blinkTimer;
    Lingmo::base::Timer _blinkCursorTimer;

    // Bell.
    int _bellMode = SystemBeepBell;
    bool _allowBell = true;
    Lingmo::base::Timer _bellTimer;
    Lingmo::base::Timer _swapTimer;

    // The input method's preedit, shown at the cursor.
    std::wstring _preedit;

    // The MouseArea's state.
    uint32_t _buttons = 0;
    bool _menuPress = false;
    std::string _pressedLink;

    // The link under the pointer, underlined while hovered.
    std::vector<Segment> _linkSegments;
    bool _ctrlHeld = false;
    Lingmo::ui::PointF _hoverPosition{-1, -1};
    std::pair<int, int> _hoverCell{-1, -1};
    Color _linkColor;
    void updateLinkHover(Lingmo::ui::PointF position, uint32_t modifiers);
    void refreshLinkHover();
    void clearLinkHover();
    void updatePointerCursor();

    // Answers from the clipboard check it before touching the view.
    std::shared_ptr<bool> _alive = std::make_shared<bool>(true);
};

}

#endif // TERMINALVIEW_H
