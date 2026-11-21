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

// Own
#include "TerminalView.h"

// System
#include <algorithm>
#include <cmath>
#include <cstring>

#include <unicode/uchar.h>
#include <xkbcommon/xkbcommon.h>

#include "base/strings.h"
#include "ui/clipboard.h"
#include "ui/drag.h"
#include "ui/painter.h"
#include "ui/window.h"

// Konsole
#include "ColorScheme.h"
#include "Filter.h"
#include "HistorySearch.h"
#include "LineChars.h"
#include "Screen.h"
#include "ScreenWindow.h"
#include "TerminalCharacterDecoder.h"
#include "TerminalFont.h"
#include "ui/bitmap.h"
#include "ui/bitmap_painter.h"
#include "ui/glyph_run.h"
#include "TerminalSession.h"
#include "Vt102Emulation.h"
#include "konsole_wcwidth.h"
#include "tools.h"

using namespace Konsole;
namespace ui = Lingmo::ui;
namespace strings = Lingmo::base::strings;
using Lingmo::base::Point;

#ifndef loc
#define loc(X,Y) ((Y)*_columns+(X))
#endif

static const int TEXT_BLINK_DELAY = 500;
// std::max(QApplication::cursorFlashTime(), 1000) / 2.
static const int CURSOR_BLINK_INTERVAL = 500;
// QApplication::doubleClickInterval().
static const int DOUBLE_CLICK_INTERVAL = 400;
// QApplication::startDragDistance().
static const int START_DRAG_DISTANCE = 10;
// QApplication::wheelScrollLines().
static const int WHEEL_SCROLL_LINES = 3;

TerminalView::TerminalView()
    : _font(std::make_unique<TerminalFont>())
{
    setName("terminal");
    setClipsChildren(true);
    setTracksHover(true);
    updatePointerCursor();

    _blinkTimer.onTimeout = [this] { blinkEvent(); };
    _blinkCursorTimer.onTimeout = [this] { blinkCursorTimeout(); };
    _tripleClickTimer.setSingleShot(true);
    _tripleClickTimer.onTimeout = [this] { _possibleTripleClick = false; };
    _bellTimer.setSingleShot(true);
    _bellTimer.onTimeout = [this] { _allowBell = true; };
    _swapTimer.setSingleShot(true);
    _swapTimer.onTimeout = [this] { swapColorTable(); };

    setColorTable(base_color_table);
}

TerminalView::~TerminalView()
{
    _windowConnections.clear();
    _sessionConnections.clear();
    if (_session)
        _session->removeView(this);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                 Session                                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::setSession(TerminalSession* session)
{
    if (_session == session)
        return;

    _sessionConnections.clear();
    _session = session;
    if (!session)
        return;

    // Session::addView().
    Emulation* emulation = session->emulation();
    _sessionConnections.push_back(emulation->programUsesMouseChanged.connect([this](bool usesMouse) {
        setUsesMouse(usesMouse);
    }));
    setUsesMouse(emulation->programUsesMouse());
    _sessionConnections.push_back(emulation->programBracketedPasteModeChanged.connect([this](bool on) {
        _bracketedPasteMode = on;
    }));
    _bracketedPasteMode = emulation->programBracketedPasteMode();
    setScreenWindow(session->createWindow());
    _sessionConnections.push_back(session->bellRequest.connect([this] { bell(); }));

    session->setViewSize(this, _lines, _columns);
    requestPaint();
}

void TerminalView::setScreenWindow(ScreenWindow* window)
{
    _windowConnections.clear();
    _screenWindow = window;

    if (window) {
        _windowConnections.push_back(window->outputChanged.connect([this] { updateLineProperties(); }));
        _windowConnections.push_back(window->outputChanged.connect([this] { updateImage(); }));
        _windowConnections.push_back(window->scrollToEnd.connect([this] { scrollToEnd(); }));
        _windowConnections.push_back(window->cursorRevealRequested.connect([this] { revealCursor(); }));
        // Drives copyAvailable, and with it hasSelection.
        _windowConnections.push_back(window->selectionChanged.connect([this] { selectionChanged(); }));
        window->setWindowLines(_lines);
    }
}

void TerminalView::setUsesMouse(bool on)
{
    if (_mouseMarks != on) {
        _mouseMarks = on;
        updatePointerCursor();
        usesMouseChanged();
    }
}

void TerminalView::selectionChanged()
{
    requestPaint();
    copyAvailable(_screenWindow && !_screenWindow->selectedText(false).empty());
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                  Colors                                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::setColorTable(const ColorEntry table[])
{
    for (int i = 0; i < TABLE_COLORS; i++)
        _colorTable[i] = table[i];
    requestPaint();
}

void TerminalView::setColorScheme(const std::string& name)
{
    if (name == _colorScheme)
        return;

    // Looked up by name: listing the available schemes first would parse every file.
    const ColorScheme* cs = ColorSchemeManager::instance()->findColorScheme(name);
    if (!cs)
        cs = ColorSchemeManager::instance()->defaultColorScheme();

    ColorEntry table[TABLE_COLORS];
    cs->getColorTable(table);
    setColorTable(table);
    _colorScheme = name;
}

ColorSchemeInfo TerminalView::colorSchemeInfo(const std::string& name)
{
    ColorSchemeInfo info;
    const ColorScheme* cs = ColorSchemeManager::instance()->findColorScheme(name);
    if (!cs)
        return info;

    ColorEntry table[TABLE_COLORS];
    cs->getColorTable(table);

    info.valid = true;
    info.description = cs->description();
    info.background = table[DEFAULT_BACK_COLOR].color;
    info.foreground = table[DEFAULT_FORE_COLOR].color;
    for (int i = 0; i < 8; ++i)
        info.colors[size_t(i)] = table[i + 2].color;
    for (int i = 0; i < 8; ++i)
        info.colors[size_t(i + 8)] = table[i + 2 + BASE_COLORS].color;
    return info;
}

void TerminalView::setBackgroundOpacity(double opacity)
{
    opacity = std::clamp(opacity, 0.0, 1.0);
    if (std::abs(_opacity - opacity) < 1e-12)
        return;
    _opacity = opacity;
    requestPaint();
}

void TerminalView::swapColorTable()
{
    ColorEntry color = _colorTable[1];
    _colorTable[1] = _colorTable[0];
    _colorTable[0] = color;
    _colorsInverted = !_colorsInverted;
    requestPaint();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                   Font                                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::setTerminalFont(const std::string& family, double pointSize)
{
    if (family == _fontFamily && pointSize == _fontPointSize && _metricsScale != 0)
        return;
    _fontFamily = family;
    _fontPointSize = pointSize;
    _metricsScale = 0;
    updateFontMetrics();
}

// TerminalDisplay::fontChange(): the cell from the font at the window's scale.
void TerminalView::updateFontMetrics()
{
    const double scale = window() ? window()->scale() : 1;
    if (_metricsScale == scale)
        return;
    _metricsScale = scale;

    _font->setFont(_fontFamily, _fontPointSize);
    _metrics = _font->metrics();
    _metrics.height += _lineSpacing;
    if (_metrics.width < 1)
        _metrics.width = 1;

    fontMetricsChanged();
    propagateSize();
    requestPaint();
}

void TerminalView::setBlinkingCursor(bool blink)
{
    _hasBlinkingCursor = blink;

    if (blink && !_blinkCursorTimer.isActive() && hasKeyboard())
        _blinkCursorTimer.start(std::chrono::milliseconds(CURSOR_BLINK_INTERVAL));

    if (!blink && _blinkCursorTimer.isActive()) {
        _blinkCursorTimer.stop();
        if (_cursorBlinking)
            blinkCursorEvent();
        else
            _cursorBlinking = false;
    }
}

void TerminalView::setTerminalCursorShape(int shape)
{
    _cursorShape = std::clamp(shape, 0, 2);
    requestPaint();
}

void TerminalView::setFullCursorHeight(bool full)
{
    _fullCursorHeight = full;
    requestPaint();
}

void TerminalView::blinkEvent()
{
    _blinking = !_blinking;
    requestPaint();
}

void TerminalView::blinkCursorEvent()
{
    _cursorBlinking = !_cursorBlinking;
    requestPaint();
}

void TerminalView::blinkCursorTimeout()
{
    // Without the keyboard the cursor shows in its unfocused state; the next
    // paint with it starts blinking again.
    if (!hasKeyboard()) {
        _blinkCursorTimer.stop();
        if (_cursorBlinking)
            blinkCursorEvent();
        return;
    }
    blinkCursorEvent();
}

bool TerminalView::hasKeyboard() const
{
    const ui::Window* window = this->window();
    return hasFocus() && window->hasKeyboardFocus();
}

void TerminalView::focusIn(ui::FocusReason)
{
    if (_hasBlinkingCursor && hasKeyboard())
        _blinkCursorTimer.start(std::chrono::milliseconds(CURSOR_BLINK_INTERVAL));
    requestPaint();

    if (_hasBlinker)
        _blinkTimer.start(std::chrono::milliseconds(TEXT_BLINK_DELAY));
}

void TerminalView::focusOut()
{
    // The cursor shows while out of focus, drawn in its unfocused state.
    _cursorBlinking = false;
    requestPaint();
    _blinkCursorTimer.stop();

    if (_blinking)
        blinkEvent();

    _blinkTimer.stop();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                 Resizing                                  */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::layout()
{
    if (!_scroller && window())
        _scroller = std::make_unique<ui::Scroller>(window(), this);

    updateFontMetrics();
    updateImageSize();
    requestPaint();
}

void TerminalView::propagateSize()
{
    if (!_image.empty())
        updateImageSize();
}

void TerminalView::calcGeometry()
{
    const int width = int(geometry().width);
    const int height = int(geometry().height);

    _leftMargin = _leftBaseMargin;
    _contentWidth = width - 2 * _leftBaseMargin;

    _topMargin = _topBaseMargin + kRound(_scrollOffset);
    _contentHeight = height - 2 * _topBaseMargin + /* mysterious */ 1;

    // ensure that display is always at least one column wide
    _columns = std::max(1, _contentWidth / _metrics.width);
    _usedColumns = std::min(_usedColumns, _columns);

    // ensure that display is always at least one line high
    _lines = std::max(1, _contentHeight / _metrics.height);
    _usedLines = std::min(_usedLines, _lines);
}

void TerminalView::clearImage()
{
    // We initialize _image[_imageSize] too. See makeImage()
    for (int i = 0; i <= _imageSize; i++) {
        _image[size_t(i)].character = ' ';
        _image[size_t(i)].foregroundColor = CharacterColor(COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR);
        _image[size_t(i)].backgroundColor = CharacterColor(COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR);
        _image[size_t(i)].rendition = DEFAULT_RENDITION;
    }
}

void TerminalView::makeImage()
{
    calcGeometry();

    // One line more for the line below the window, see _hasLineBelow.
    _imageSize = (_lines + 1) * _columns;

    // We over-commit one character so that we can be more relaxed in dealing with
    // certain boundary conditions: _image[_imageSize] is a valid but unused position
    _image.assign(size_t(_imageSize + 1), Character());

    clearImage();
}

void TerminalView::updateImageSize()
{
    if (_scroller) {
        _scroller->cancel(false);
        _scrollOffset = 0;
    }

    std::vector<Character> oldimg = std::move(_image);
    const int oldlin = _lines;
    const int oldcol = _columns;

    makeImage();

    // copy the old image to reduce flicker
    const int lines = std::min(oldlin, _lines);
    const int columns = std::min(oldcol, _columns);

    if (!oldimg.empty()) {
        for (int line = 0; line < lines; line++) {
            std::memcpy(static_cast<void*>(&_image[size_t(_columns * line)]), &oldimg[size_t(oldcol * line)],
                        size_t(columns) * sizeof(Character));
        }
    }

    if (_screenWindow)
        _screenWindow->setWindowLines(_lines);

    if (oldlin != _lines || oldcol != _columns) {
        if (_session)
            _session->setViewSize(this, _lines, _columns);
        sizeChanged();
    }
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                             Display Operations                            */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::updateLineProperties()
{
    if (!_screenWindow)
        return;

    _lineProperties = _screenWindow->getLineProperties();
}

void TerminalView::updateImage()
{
    if (!_screenWindow)
        return;

    if (_image.empty()) {
        // Create _image.
        // The emitted size change also leads to getImage being recreated, so do this first.
        updateImageSize();
    }

    // Scroll just far enough for output that pushed the cursor out of view.
    if (_followCursor && !_screenWindow->trackOutput()) {
        const int below = cursorLineInWindow() - _screenWindow->windowLines() + 1;
        if (below > 0)
            _screenWindow->scrollTo(_screenWindow->currentLine() + below);
        if (_screenWindow->atEndOfOutput()) {
            _screenWindow->setTrackOutput(true);
            _followCursor = false;
        }
    }

    Character* const newimg = _screenWindow->getImage();
    const int lines = _screenWindow->windowLines();
    const int columns = _screenWindow->windowColumns();

    setScroll(_screenWindow->currentLine(), _screenWindow->lineCount());

    _hasBlinker = false;

    const int linesToUpdate = std::min(_lines, std::max(0, lines));
    const int columnsToUpdate = std::min(_columns, std::max(0, columns));

    for (int y = 0; y < linesToUpdate; ++y) {
        Character* currentLine = &_image[size_t(y * _columns)];
        const Character* const newLine = &newimg[y * columns];

        for (int x = 0; x < columnsToUpdate; ++x) {
            if ((newLine[x].rendition & RE_BLINK) != 0)
                _hasBlinker = true;
        }

        // replace the line of characters in the old _image with the
        // current line of the new _image
        std::memcpy(static_cast<void*>(currentLine), newLine, size_t(columnsToUpdate) * sizeof(Character));
    }

    _usedLines = linesToUpdate;
    _usedColumns = columnsToUpdate;

    const int lineBelow = _screenWindow->currentLine() + lines;
    _hasLineBelow = lineBelow < _screenWindow->lineCount();
    if (_hasLineBelow) {
        std::vector<Character> below(static_cast<size_t>(columns));
        _screenWindow->screen()->getImage(below.data(), columns, lineBelow, lineBelow);
        std::memcpy(static_cast<void*>(&_image[size_t(linesToUpdate * _columns)]), below.data(),
                    size_t(columnsToUpdate) * sizeof(Character));
    }

    requestPaint();

    if (_hasBlinker && !_blinkTimer.isActive())
        _blinkTimer.start(std::chrono::milliseconds(TEXT_BLINK_DELAY));
    if (!_hasBlinker && _blinkTimer.isActive()) {
        _blinkTimer.stop();
        _blinking = false;
    }
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                Scrollbar                                  */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::setScrollBarValueQuietly(int value)
{
    _scrollValue = std::clamp(value, 0, _scrollMaximum);
}

// ScrollBar::setValue(): changes are what scrollBarPositionChanged() follows.
void TerminalView::setScrollbarValue(int value)
{
    const int newValue = std::clamp(value, 0, _scrollMaximum);
    if (newValue == _scrollValue)
        return;
    _scrollValue = newValue;
    scrollbarParamsChanged();
    refreshLinkHover();
    scrollBarPositionChanged();
}

void TerminalView::scrollBarPositionChanged()
{
    if (!_screenWindow)
        return;

    if (!_applyingScroll) {
        if (_scroller)
            _scroller->cancel(false);
        setScrollOffset(0);
    }
    _followCursor = false;

    _screenWindow->scrollTo(_scrollValue);

    // if the thumb has been moved to the bottom of the _scrollBar then set
    // the display to automatically track new output,
    // that is, scroll down automatically
    // to how new _lines as they are added
    const bool atEndOfOutput = (_scrollValue == _scrollMaximum);
    _screenWindow->setTrackOutput(atEndOfOutput);

    updateImage();

    scrollbarValueChanged();
}

void TerminalView::setScroll(int cursor, int slines)
{
    // Before the first layout the display is a one-line fallback, so the whole screen would read as scrollback.
    const int maxScrollValue = geometry().height > 0 ? std::max(0, slines - _lines) : 0;
    const int boundedCursor = std::clamp(cursor, 0, maxScrollValue);

    if (_scrollMaximum == maxScrollValue && _scrollValue == boundedCursor)
        return;

    const int previousValue = _scrollValue;
    const bool rangeChanged = _scrollMaximum != maxScrollValue;
    _scrollMaximum = maxScrollValue;
    setScrollBarValueQuietly(_scrollValue);
    const bool clamped = _scrollValue != previousValue;
    setScrollBarValueQuietly(boundedCursor);
    if (rangeChanged || clamped || _scrollValue != previousValue) {
        scrollbarParamsChanged();
        refreshLinkHover();
    }

    // The view moved under a resting partial scroll, or the scrollback went away.
    if (maxScrollValue == 0 || (_scrollValue != previousValue && !(_scroller && _scroller->isActive()))) {
        if (_scroller)
            _scroller->cancel(false);
        setScrollOffset(0);
    }
}

int TerminalView::cursorLineInWindow() const
{
    Screen* screen = _screenWindow->screen();
    return screen->getHistLines() + screen->getCursorY() - _screenWindow->currentLine();
}

// A short scrollback over a mostly empty screen: jumping to the end would
// push the prompt up and show the empty lines instead.
void TerminalView::revealCursor()
{
    if (!_screenWindow)
        return;

    const int line = cursorLineInWindow();
    if (line < 0 || line >= _screenWindow->windowLines()) {
        scrollToEnd();
        return;
    }

    _followCursor = !_screenWindow->atEndOfOutput();
}

void TerminalView::scrollToEnd()
{
    _followCursor = false;
    if (_scroller)
        _scroller->cancel(false);
    setScrollOffset(0);

    if (_scrollValue != _scrollMaximum) {
        setScrollBarValueQuietly(_scrollMaximum);
        scrollbarParamsChanged();
    }

    _screenWindow->scrollTo(_scrollValue + 1);
    _screenWindow->setTrackOutput(_screenWindow->atEndOfOutput());
}

double TerminalView::scrollPixelPosition() const
{
    return _scrollValue * double(_metrics.height) - _scrollOffset;
}

double TerminalView::scrollPixelMaximum() const
{
    return _scrollMaximum * double(_metrics.height);
}

// Past either end the whole text moves with the stretch.
void TerminalView::setScrollPixelPosition(double position)
{
    const int maximum = _scrollMaximum;
    const double bounded = std::clamp(position, 0.0, maximum * double(_metrics.height));
    const int line = std::min(maximum, int(bounded / _metrics.height));

    _applyingScroll = true;
    setScrollbarValue(line);
    _applyingScroll = false;
    setScrollOffset(line * double(_metrics.height) - position);
}

void TerminalView::setScrollOffset(double offset)
{
    _scrollOffset = offset;
    const int topMargin = _topBaseMargin + kRound(offset);
    if (topMargin == _topMargin)
        return;

    _topMargin = topMargin;
    requestPaint();
}

int TerminalView::drawnLines() const
{
    return _usedLines + (_topMargin < _topBaseMargin && _hasLineBelow ? 1 : 0);
}

// The scroller's axis: the scrollback in pixels.
ui::Scroller::Axis TerminalView::horizontalAxis() const
{
    return {};
}

ui::Scroller::Axis TerminalView::verticalAxis() const
{
    return {0, scrollPixelMaximum(), geometry().height, _scrollMaximum > 0};
}

ui::PointF TerminalView::position() const
{
    return {0, scrollPixelPosition()};
}

void TerminalView::applyPosition(ui::PointF position)
{
    setScrollPixelPosition(position.y);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                  Mouse                                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::sendMouse(int button, int column, int line, int eventType)
{
    if (_session)
        _session->emulation()->sendMouseEvent(button, column, line, eventType);
}

void TerminalView::getCharacterPosition(ui::PointF point, int& line, int& column) const
{
    line = int((point.y - _topMargin) / _metrics.height);
    if (line < 0)
        line = 0;
    if (line >= _usedLines)
        line = _usedLines - 1;

    const int x = int(point.x + _metrics.width / 2 - _leftMargin);
    column = x / _metrics.width;

    if (column < 0)
        column = 0;

    // the column value returned can be equal to _usedColumns, which
    // is the position just after the last character displayed in a line.
    //
    // this is required so that the user can select characters in the right-most
    // column (or left-most for right-to-left input)
    if (column > _usedColumns)
        column = _usedColumns;
}

void TerminalView::getCharacterCellAt(ui::PointF point, int& line, int& column) const
{
    getCharacterPosition({point.x - _metrics.width / 2.0, point.y}, line, column);
    column = std::clamp(column, 0, std::max(0, _usedColumns - 1));
}

// QMouseEvent::pos(): the position rounded to whole pixels.
static ui::PointF eventPosition(double x, double y)
{
    return {double(kRound(x)), double(kRound(y))};
}

void TerminalView::simulateMousePress(double x, double y, ui::MouseButton button, uint32_t modifiers)
{
    const ui::PointF pos = eventPosition(x, y);
    if (_possibleTripleClick && button == ui::MouseButton::Left) {
        mouseTripleClick(pos);
        return;
    }

    // contentsRect().contains(ev->pos()).
    if (pos.x < 0 || pos.y < 0 || pos.x > int(geometry().width) - 1 || pos.y > int(geometry().height) - 1)
        return;

    if (!_screenWindow)
        return;

    int charLine;
    int charColumn;
    getCharacterPosition(pos, charLine, charColumn);
    Point point(charColumn, charLine);

    if (button == ui::MouseButton::Left) {
        _lineSelectionMode = false;
        _wordSelectionMode = false;

        const bool selected = _screenWindow->isSelected(point.x(), point.y());

        if (selected) {
            // The user clicked inside selected text
            _dragState = DragPending;
            _dragStart = pos;
        } else {
            // No reason to ever start a drag event
            _dragState = DragNone;

            _preserveLineBreaks = !((modifiers & ui::ControlModifier) && !(modifiers & ui::AltModifier));
            _columnSelectionMode = (modifiers & ui::AltModifier) && (modifiers & ui::ControlModifier);

            if (_mouseMarks || (modifiers & ui::ShiftModifier)) {
                _screenWindow->clearSelection();

                point.setY(point.y() + _scrollValue);
                _iPntSel = _pntSel = point;
                _actSel = 1; // left mouse button pressed but nothing selected yet.
            } else {
                sendMouse(0, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 0);
            }
        }
    } else if (button == ui::MouseButton::Middle) {
        if (_mouseMarks || (modifiers & ui::ShiftModifier))
            emitSelection(true, modifiers & ui::ControlModifier);
        else
            sendMouse(1, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 0);
    } else if (button == ui::MouseButton::Right) {
        if (!_mouseMarks && !(modifiers & ui::ShiftModifier))
            sendMouse(2, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 0);
    }
}

void TerminalView::simulateMouseMove(double x, double y, uint32_t buttons, uint32_t modifiers)
{
    const ui::PointF pos = eventPosition(x, y);
    int charLine = 0;
    int charColumn = 0;
    getCharacterPosition(pos, charLine, charColumn);

    if (buttons == 0)
        return;

    // if the terminal is interested in mouse movements
    // then emit a mouse movement signal, unless the shift
    // key is being held down, which overrides this.
    if (!_mouseMarks && !(modifiers & ui::ShiftModifier)) {
        int button = 3;
        if (buttons & (1u << int(ui::MouseButton::Left)))
            button = 0;
        if (buttons & (1u << int(ui::MouseButton::Middle)))
            button = 1;
        if (buttons & (1u << int(ui::MouseButton::Right)))
            button = 2;

        sendMouse(button, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 1);
        return;
    }

    if (_dragState == DragPending) {
        // we had a mouse down, but haven't confirmed a drag yet
        // if the mouse has moved sufficiently, we will confirm
        const int distance = START_DRAG_DISTANCE;
        if (pos.x > _dragStart.x + distance || pos.x < _dragStart.x - distance
            || pos.y > _dragStart.y + distance || pos.y < _dragStart.y - distance) {
            // we've left the drag square, we can start a real drag operation now
            doDrag();
        }
        return;
    } else if (_dragState == DragDragging) {
        return;
    }

    if (_actSel == 0)
        return;

    // don't extend selection while pasting
    if (buttons & (1u << int(ui::MouseButton::Middle)))
        return;

    extendSelection(pos);
}

void TerminalView::doDrag()
{
    _dragState = DragDragging;

    const std::string text = _screenWindow ? _screenWindow->selectedText(_preserveLineBreaks) : std::string();
    ui::DragSource source;
    source.mimeTypes = {"text/plain;charset=utf-8", "text/plain"};
    source.data = [text](const std::string&) { return text; };
    source.actions = ui::CopyAction;
    source.onFinished = [this](ui::DropAction) { _dragState = DragNone; };
    startDrag(std::move(source));
}

void TerminalView::extendSelection(ui::PointF position)
{
    if (!_screenWindow)
        return;

    const int scroll = _scrollValue;

    // we're in the process of moving the mouse with the left button pressed
    // the mouse cursor will kept caught within the bounds of the text in
    // this widget.
    int linesBeyondWidget = 0;

    // QRect(left, top, width, height) with right = left + width - 1.
    const int textLeft = _leftMargin;
    const int textTop = _topMargin;
    const int textRight = textLeft + _usedColumns * _metrics.width - 1 - 1;
    const int textBottom = textTop + _usedLines * _metrics.height - 1 - 1;

    // Adjust position within text area bounds.
    const int oldX = int(position.x);
    const int oldY = int(position.y);
    ui::PointF pos{double(std::clamp(oldX, textLeft, std::max(textLeft, textRight))),
                   double(std::clamp(oldY, textTop, std::max(textTop, textBottom)))};

    if (oldY > textBottom) {
        linesBeyondWidget = (oldY - textBottom) / _metrics.height;
        setScrollbarValue(_scrollValue + linesBeyondWidget + 1); // scrollforward
    }
    if (oldY < textTop) {
        linesBeyondWidget = (textTop - oldY) / _metrics.height;
        setScrollbarValue(_scrollValue - linesBeyondWidget - 1); // history
    }

    int charColumn = 0;
    int charLine = 0;
    getCharacterPosition(pos, charLine, charColumn);

    Point here(charColumn, charLine);
    Point ohere;
    Point iPntSelCorr = _iPntSel;
    iPntSelCorr.setY(iPntSelCorr.y() - _scrollValue);
    Point pntSelCorr = _pntSel;
    pntSelCorr.setY(pntSelCorr.y() - _scrollValue);
    bool swapping = false;

    if (_wordSelectionMode) {
        // Extend to word boundaries
        int i;
        char32_t selClass;

        const bool left_not_right = (here.y() < iPntSelCorr.y()
                                     || (here.y() == iPntSelCorr.y() && here.x() < iPntSelCorr.x()));
        const bool old_left_not_right = (pntSelCorr.y() < iPntSelCorr.y()
                                         || (pntSelCorr.y() == iPntSelCorr.y() && pntSelCorr.x() < iPntSelCorr.x()));
        swapping = left_not_right != old_left_not_right;

        // Find left (left_not_right ? from here : from start)
        Point left = left_not_right ? here : iPntSelCorr;
        i = loc(left.x(), left.y());
        if (i >= 0 && i <= _imageSize) {
            selClass = charClass(_image[size_t(i)]);
            while (((left.x() > 0) || (left.y() > 0 && (_lineProperties[size_t(left.y() - 1)] & LINE_WRAPPED)))
                   && charClass(_image[size_t(i - 1)]) == selClass) {
                i--;
                if (left.x() > 0) {
                    left.setX(left.x() - 1);
                } else {
                    left.setX(_usedColumns - 1);
                    left.setY(left.y() - 1);
                }
            }
        }

        // Find left (left_not_right ? from start : from here)
        Point right = left_not_right ? iPntSelCorr : here;
        i = loc(right.x(), right.y());
        if (i >= 0 && i <= _imageSize) {
            selClass = charClass(_image[size_t(i)]);
            while (((right.x() < _usedColumns - 1)
                    || (right.y() < _usedLines - 1 && (_lineProperties[size_t(right.y())] & LINE_WRAPPED)))
                   && charClass(_image[size_t(i + 1)]) == selClass) {
                i++;
                if (right.x() < _usedColumns - 1) {
                    right.setX(right.x() + 1);
                } else {
                    right.setX(0);
                    right.setY(right.y() + 1);
                }
            }
        }

        // Pick which is start (ohere) and which is extension (here)
        if (left_not_right) {
            here = left;
            ohere = right;
        } else {
            here = right;
            ohere = left;
        }
        ohere.setX(ohere.x() + 1);
    }

    if (_lineSelectionMode) {
        // Extend to complete line
        const bool above_not_below = (here.y() < iPntSelCorr.y());

        Point above = above_not_below ? here : iPntSelCorr;
        Point below = above_not_below ? iPntSelCorr : here;

        while (above.y() > 0 && (_lineProperties[size_t(above.y() - 1)] & LINE_WRAPPED))
            above.setY(above.y() - 1);
        while (below.y() < _usedLines - 1 && (_lineProperties[size_t(below.y())] & LINE_WRAPPED))
            below.setY(below.y() + 1);

        above.setX(0);
        below.setX(_usedColumns - 1);

        // Pick which is start (ohere) and which is extension (here)
        if (above_not_below) {
            here = above;
            ohere = below;
        } else {
            here = below;
            ohere = above;
        }

        const Point newSelBegin(ohere.x(), ohere.y());
        swapping = !(_tripleSelBegin == newSelBegin);
        _tripleSelBegin = newSelBegin;

        ohere.setX(ohere.x() + 1);
    }

    int offset = 0;
    if (!_wordSelectionMode && !_lineSelectionMode) {
        const bool left_not_right = (here.y() < iPntSelCorr.y()
                                     || (here.y() == iPntSelCorr.y() && here.x() < iPntSelCorr.x()));
        const bool old_left_not_right = (pntSelCorr.y() < iPntSelCorr.y()
                                         || (pntSelCorr.y() == iPntSelCorr.y() && pntSelCorr.x() < iPntSelCorr.x()));
        swapping = left_not_right != old_left_not_right;

        // Find left (left_not_right ? from here : from start)
        const Point left = left_not_right ? here : iPntSelCorr;

        // Find left (left_not_right ? from start : from here)
        const Point right = left_not_right ? iPntSelCorr : here;

        // Pick which is start (ohere) and which is extension (here)
        if (left_not_right) {
            here = left;
            ohere = right;
            offset = 0;
        } else {
            here = right;
            ohere = left;
            offset = -1;
        }
    }

    if ((here == pntSelCorr) && (scroll == _scrollValue))
        return; // not moved

    if (here == ohere)
        return; // It's not left, it's not right.

    if (_actSel < 2 || swapping) {
        if (_columnSelectionMode && !_lineSelectionMode && !_wordSelectionMode)
            _screenWindow->setSelectionStart(ohere.x(), ohere.y(), true);
        else
            _screenWindow->setSelectionStart(ohere.x() - 1 - offset, ohere.y(), false);
    }

    _actSel = 2; // within selection
    _pntSel = here;
    _pntSel.setY(_pntSel.y() + _scrollValue);

    if (_columnSelectionMode && !_lineSelectionMode && !_wordSelectionMode)
        _screenWindow->setSelectionEnd(here.x(), here.y());
    else
        _screenWindow->setSelectionEnd(here.x() + offset, here.y());
}

void TerminalView::simulateMouseRelease(double x, double y, ui::MouseButton button, uint32_t modifiers)
{
    if (!_screenWindow)
        return;

    int charLine;
    int charColumn;
    getCharacterPosition(eventPosition(x, y), charLine, charColumn);

    if (button == ui::MouseButton::Left) {
        if (_dragState == DragPending) {
            // We had a drag event pending but never confirmed.  Kill selection
            _screenWindow->clearSelection();
        } else {
            if (_actSel > 1)
                setSelection(_screenWindow->selectedText(_preserveLineBreaks));

            _actSel = 0;

            //FIXME: emits a release event even if the mouse is
            //       outside the range. The procedure used in `mouseMoveEvent'
            //       applies here, too.

            if (!_mouseMarks && !(modifiers & ui::ShiftModifier))
                sendMouse(0, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 2);
        }
        _dragState = DragNone;
    }

    if (!_mouseMarks
        && ((button == ui::MouseButton::Right && !(modifiers & ui::ShiftModifier))
            || button == ui::MouseButton::Middle)) {
        sendMouse(button == ui::MouseButton::Middle ? 1 : 2, charColumn + 1,
                  charLine + 1 + _scrollValue - _scrollMaximum, 2);
    }
}

void TerminalView::simulateMouseDoubleClick(double x, double y, ui::MouseButton button, uint32_t modifiers)
{
    if (button != ui::MouseButton::Left)
        return;
    if (!_screenWindow)
        return;

    int charLine = 0;
    int charColumn = 0;
    getCharacterPosition(eventPosition(x, y), charLine, charColumn);

    // pass on double click as two clicks.
    if (!_mouseMarks && !(modifiers & ui::ShiftModifier)) {
        // Send just _ONE_ click event, since the first click of the double click
        // was already sent by the click handler
        sendMouse(0, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 0); // left button
        return;
    }

    _screenWindow->clearSelection();
    _iPntSel = Point(charColumn, charLine + _scrollValue);

    _wordSelectionMode = true;

    int beginColumn, beginLine, endColumn, endLine;
    findWordBounds(charColumn, charLine, beginColumn, beginLine, endColumn, endLine);

    _screenWindow->setSelectionStart(beginColumn, beginLine, false);

    _actSel = 2; // within selection

    _screenWindow->setSelectionEnd(endColumn, endLine);

    setSelection(_screenWindow->selectedText(_preserveLineBreaks));

    _possibleTripleClick = true;

    _tripleClickTimer.start(std::chrono::milliseconds(DOUBLE_CLICK_INTERVAL));
}

void TerminalView::findWordBounds(int column, int line, int& beginColumn, int& beginLine, int& endColumn,
                                  int& endLine) const
{
    beginColumn = endColumn = column;
    beginLine = endLine = line;
    int i = loc(column, line);

    const char32_t selClass = charClass(_image[size_t(i)]);

    // find the start of the word
    int x = column;
    while (((x > 0) || (beginLine > 0 && (_lineProperties[size_t(beginLine - 1)] & LINE_WRAPPED)))
           && charClass(_image[size_t(i - 1)]) == selClass) {
        i--;
        if (x > 0) {
            x--;
        } else {
            x = _usedColumns - 1;
            beginLine--;
        }
    }

    beginColumn = x;

    // find the end of the word
    i = loc(column, line);
    x = column;
    while (((x < _usedColumns - 1) || (endLine < _usedLines - 1 && (_lineProperties[size_t(endLine)] & LINE_WRAPPED)))
           && charClass(_image[size_t(i + 1)]) == selClass) {
        i++;
        if (x < _usedColumns - 1) {
            x++;
        } else {
            x = 0;
            endLine++;
        }
    }

    endColumn = x;

    // In word selection mode don't select @ (64) if at end of word.
    if (char16_t(_image[size_t(i)].character) == u'@' && endColumn - beginColumn > 0
        && (_image[size_t(i)].rendition & RE_EXTENDED_CHAR) == 0) {
        endColumn = x - 1;
    }
}

bool TerminalView::isSelectedAt(double x, double y)
{
    if (!_screenWindow)
        return false;

    int charLine = 0;
    int charColumn = 0;
    getCharacterCellAt({x, y}, charLine, charColumn);
    return _screenWindow->isSelected(charColumn, charLine);
}

bool TerminalView::selectWordAt(double x, double y)
{
    if (!_screenWindow || _image.empty())
        return false;

    int charLine = 0;
    int charColumn = 0;
    getCharacterCellAt({x, y}, charLine, charColumn);

    if (konsole_isspace(char16_t(_image[size_t(loc(charColumn, charLine))].character)))
        return false;

    int beginColumn, beginLine, endColumn, endLine;
    findWordBounds(charColumn, charLine, beginColumn, beginLine, endColumn, endLine);

    _screenWindow->clearSelection();
    _screenWindow->setSelectionStart(beginColumn, beginLine, false);
    _screenWindow->setSelectionEnd(endColumn, endLine);
    setSelection(_screenWindow->selectedText(_preserveLineBreaks));
    return true;
}

void TerminalView::mouseTripleClick(ui::PointF position)
{
    if (!_screenWindow)
        return;

    int charLine;
    int charColumn;
    getCharacterPosition(position, charLine, charColumn);
    _iPntSel = Point(charColumn, charLine);

    _screenWindow->clearSelection();

    _lineSelectionMode = true;
    _wordSelectionMode = false;

    _actSel = 2; // within selection

    while (_iPntSel.y() > 0 && (_lineProperties[size_t(_iPntSel.y() - 1)] & LINE_WRAPPED))
        _iPntSel.setY(_iPntSel.y() - 1);

    // SelectWholeLine.
    _screenWindow->setSelectionStart(0, _iPntSel.y(), false);
    _tripleSelBegin = Point(0, _iPntSel.y());

    while (_iPntSel.y() < _lines - 1 && (_lineProperties[size_t(_iPntSel.y())] & LINE_WRAPPED))
        _iPntSel.setY(_iPntSel.y() + 1);

    _screenWindow->setSelectionEnd(_columns - 1, _iPntSel.y());

    setSelection(_screenWindow->selectedText(_preserveLineBreaks));

    _iPntSel.setY(_iPntSel.y() + _scrollValue);
}

// QChar::isLetterOrNumber() of a UTF-16 unit.
static bool isLetterOrNumber(char16_t ch)
{
    switch (u_charType(ch)) {
    case U_UPPERCASE_LETTER:
    case U_LOWERCASE_LETTER:
    case U_TITLECASE_LETTER:
    case U_MODIFIER_LETTER:
    case U_OTHER_LETTER:
    case U_DECIMAL_DIGIT_NUMBER:
    case U_LETTER_NUMBER:
    case U_OTHER_NUMBER:
        return true;
    default:
        return false;
    }
}

// QString::contains(QChar, Qt::CaseInsensitive): QChar::toCaseFolded on both.
static bool containsIgnoringCase(const std::u16string& text, char16_t ch)
{
    const UChar32 folded = u_foldCase(ch, U_FOLD_CASE_DEFAULT);
    return std::any_of(text.begin(), text.end(), [folded](char16_t c) {
        return u_foldCase(c, U_FOLD_CASE_DEFAULT) == folded;
    });
}

char32_t TerminalView::charClass(const Character& ch) const
{
    if (ch.rendition & RE_EXTENDED_CHAR) {
        ushort extendedCharLength = 0;
        const uint* chars = ExtendedCharTable::instance.lookupExtendedChar(ch.character, extendedCharLength);
        if (chars && extendedCharLength > 0) {
            std::wstring str;
            for (ushort nchar = 0; nchar < extendedCharLength; nchar++)
                str.push_back(wchar_t(chars[nchar]));
            std::u16string s;
            TerminalCharacterDecoder::appendWide(s, str);
            // QString::contains(QString, CaseInsensitive).
            std::u16string foldedWords;
            std::u16string foldedText;
            for (char16_t c : _wordCharacters)
                foldedWords.push_back(char16_t(u_foldCase(c, U_FOLD_CASE_DEFAULT)));
            for (char16_t c : s)
                foldedText.push_back(char16_t(u_foldCase(c, U_FOLD_CASE_DEFAULT)));
            if (foldedWords.find(foldedText) != std::u16string::npos)
                return U'a';
            bool allLetterOrNumber = true;
            for (size_t i = 0; allLetterOrNumber && i < s.size(); ++i)
                allLetterOrNumber = isLetterOrNumber(s[i]);
            return allLetterOrNumber ? U'a' : char32_t(s.empty() ? 0 : s[0]);
        }
        return 0;
    }

    const char16_t qch = char16_t(ch.character);
    if (konsole_isspace(qch))
        return U' ';
    if (isLetterOrNumber(qch) || containsIgnoringCase(_wordCharacters, qch))
        return U'a';
    return qch;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                  Wheel                                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */

bool TerminalView::scroll(const ui::ScrollEvent& event)
{
    // A tab moved to another window brings its view along; the scroller ticks on the window's frames.
    if (_scroller && _scroller->window() != window())
        _scroller = window() ? std::make_unique<ui::Scroller>(window(), this) : nullptr;

    // A touchpad drags the scrollback by the pixel, coasts and stretches at the ends.
    if (_mouseMarks && event.precise && _scrollMaximum > 0 && _scroller) {
        _scroller->handleScroll(event);
        return true;
    }

    // Qt's angle delta: positive for a wheel turned away, towards the top.
    const int angleDelta = event.precise ? -kRound(event.deltaY) : -event.value120Y;
    if (angleDelta == 0)
        return true;

    // if the terminal program is not interested mouse events
    // then send the event to the scrollbar if the slider has room to move
    // or otherwise send simulated up / down key presses to the terminal program
    // for the benefit of programs such as 'less'
    if (_mouseMarks) {
        int lines;
        if (event.precise && _metrics.height > 0) {
            // Touchpads report pixels: one line per line of text scrolled.
            _wheelPixelRemainder += angleDelta;
            lines = _wheelPixelRemainder / _metrics.height;
            _wheelPixelRemainder -= lines * _metrics.height;
        } else {
            // A wheel notch is 120; high-resolution wheels send fractions of one.
            _wheelAngleRemainder += angleDelta * WHEEL_SCROLL_LINES;
            lines = _wheelAngleRemainder / 120;
            _wheelAngleRemainder -= lines * 120;
        }
        if (lines == 0)
            return true;

        if (_scrollMaximum > 0) {
            setScrollbarValue(_scrollValue - lines);
        } else if (_session && _session->emulation()->usesAlternateScreen()) {
            // A full-screen program without mouse reporting, such as less, scrolls
            // on Up / Down; at a prompt those would recall the shell's history.
            KeyEvent keyScrollEvent{lines > 0 ? Key_Up : Key_Down, NoModifier, std::string()};
            for (int i = 0; i < std::abs(lines); i++)
                keyPressed(keyScrollEvent, false);
        }
    } else {
        // terminal program wants notification of mouse activity
        int charLine;
        int charColumn;
        const ui::Window* window = this->window();
        (void)window;
        getCharacterPosition(_lastPointer, charLine, charColumn);

        sendMouse(angleDelta > 0 ? 4 : 5, charColumn + 1, charLine + 1 + _scrollValue - _scrollMaximum, 0);
    }
    return true;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                Keyboard                                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::keyPressed(KeyEvent& event, bool fromPaste)
{
    if (_session)
        _session->emulation()->sendKeyEvent(&event, fromPaste);
}

void TerminalView::restartCursorBlink()
{
    if (_hasBlinkingCursor) {
        _blinkCursorTimer.start(std::chrono::milliseconds(CURSOR_BLINK_INTERVAL));
        if (_cursorBlinking)
            blinkCursorEvent();
        else
            _cursorBlinking = false;
    }
}

void TerminalView::sendKeyEvent(KeyEvent& event)
{
    _actSel = 0; // Key stroke implies a screen update, so TerminalDisplay won't
                 // know where the current selection is.

    restartCursorBlink();
    keyPressed(event, false);
}

// libxkbcommon's control transformation, which Qt's key text carries.
static char32_t toControl(char32_t c)
{
    if ((c >= '@' && c < 0x7f) || c == ' ')
        return c & 0x1f;
    if (c == '2')
        return 0;
    if (c >= '3' && c <= '7')
        return c - ('3' - 0x1b);
    if (c == '8')
        return 0x7f;
    if (c == '/')
        return '_' & 0x1f;
    return c;
}

KeyEvent TerminalView::keyEventFor(const ui::KeyEvent& event)
{
    KeyEvent key;
    if (event.modifiers & ui::ShiftModifier)
        key.modifiers |= ShiftModifier;
    if (event.modifiers & ui::ControlModifier)
        key.modifiers |= ControlModifier;
    if (event.modifiers & ui::AltModifier)
        key.modifiers |= AltModifier;
    if (event.modifiers & ui::SuperModifier)
        key.modifiers |= MetaModifier;

    // The toolkit's key is the lower-case keysym; the keypad's keep their own.
    uint32_t keysym = event.key;
    if (isKeypadKeysym(keysym))
        key.modifiers |= KeypadModifier;

    // Qt's text is xkb's, control characters included, which the toolkit leaves out.
    key.text = event.text;
    if (key.text.empty()) {
        char32_t c = xkb_keysym_to_utf32(keysym);
        if (c != 0 && (event.modifiers & ui::ControlModifier) && c < 0x80)
            c = toControl(c);
        if (c != 0 && (c < 0x20 || c == 0x7f))
            strings::appendUtf8(key.text, c);
    }

    // The text without the control transformation names the key.
    std::string plain;
    if (char32_t c = xkb_keysym_to_utf32(keysym); c >= 0x20 && c != 0x7f)
        strings::appendUtf8(plain, c);
    key.key = keyCodeFromKeysym(keysym, key.modifiers, plain);
    // Shift+Tab reaches the program as Backtab, as Qt names it.
    if (key.key == Key_Tab && (key.modifiers & ShiftModifier))
        key.key = Key_Backtab;
    return key;
}

bool TerminalView::keyPress(const ui::KeyEvent& event)
{
    if (event.key == XKB_KEY_Control_L || event.key == XKB_KEY_Control_R) {
        _ctrlHeld = true;
        updatePointerCursor();
    }
    if (onShortcut && onShortcut(event))
        return true;

    KeyEvent key = keyEventFor(event);
    sendKeyEvent(key);
    return true;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                               Input method                                */
/*                                                                           */
/* ------------------------------------------------------------------------- */

ui::TextInputState TerminalView::textInputState() const
{
    ui::TextInputState state;
    const Point cursor = _screenWindow ? _screenWindow->cursorPosition() : Point(0, 0);
    state.cursorRectangle = {double(_leftMargin + _metrics.width * cursor.x()),
                             double(_topMargin + _metrics.height * cursor.y()), double(_metrics.width),
                             double(_metrics.height)};
    const ui::PointF origin = mapToWindow({0, 0});
    state.cursorRectangle.x += origin.x;
    state.cursorRectangle.y += origin.y;
    // ImhNoPredictiveText | ImhNoAutoUppercase.
    state.hints = ui::NoTextHint;
    return state;
}

void TerminalView::textInput(const ui::TextInputEvent& event)
{
    if (!event.commit.empty()) {
        KeyEvent key{0, NoModifier, event.commit};
        keyPressed(key, false);
    }

    std::wstring preedit;
    for (char32_t c : strings::utf8ToUtf32(event.preedit))
        preedit.push_back(wchar_t(c));
    _preedit = preedit;
    requestPaint();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                               Clipboard                                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::setSelection(const std::string& text)
{
    if (ui::Clipboard::supportsSelection())
        ui::Clipboard::setText(text, ui::ClipboardMode::Selection);
}

bool TerminalView::hasSelection() const
{
    return _screenWindow && !_screenWindow->selectedText(false).empty();
}

void TerminalView::selectAll()
{
    if (!_screenWindow)
        return;

    _screenWindow->selectAll();
    setSelection(_screenWindow->selectedText(_preserveLineBreaks));
}

std::string TerminalView::selectedText() const
{
    return _screenWindow ? _screenWindow->selectedText(_preserveLineBreaks) : std::string();
}

void TerminalView::copyClipboard()
{
    if (!_screenWindow)
        return;

    const std::string text = _screenWindow->selectedText(_preserveLineBreaks);
    if (!text.empty())
        ui::Clipboard::setText(text);
}

void TerminalView::pasteClipboard()
{
    emitSelection(false, false);
}

void TerminalView::pasteSelection()
{
    emitSelection(true, false);
}

void TerminalView::emitSelection(bool useXselection, bool appendReturn)
{
    if (!_screenWindow)
        return;

    // Paste Clipboard by simulating keypress events
    auto paste = [weak = std::weak_ptr<bool>(_alive), this, appendReturn](std::string text) {
        if (!weak.lock())
            return;
        pasteText(text, appendReturn);
    };
    ui::Clipboard::requestText(paste, useXselection ? ui::ClipboardMode::Selection : ui::ClipboardMode::Clipboard);
}

// QString::replace(QRegularExpression("\\r+$"), ""): trailing carriage returns.
static void trimTrailingReturns(std::u16string& text)
{
    while (!text.empty() && text.back() == u'\r')
        text.pop_back();
}

static void replaceAll(std::u16string& text, const std::u16string& from, const std::u16string& to)
{
    size_t position = 0;
    while ((position = text.find(from, position)) != std::u16string::npos) {
        text.replace(position, from.size(), to);
        position += to.size();
    }
}

void TerminalView::pasteText(const std::string& utf8, bool appendReturn)
{
    if (!_screenWindow)
        return;

    std::u16string text = strings::utf8ToUtf16(utf8);
    if (text.empty())
        return;

    replaceAll(text, u"\r\n", u"\n");
    std::replace(text.begin(), text.end(), u'\n', u'\r');

    if (_trimPastedTrailingNewlines)
        trimTrailingReturns(text);

    if (_confirmMultilinePaste && text.find(u'\r') != std::u16string::npos) {
        _pendingPaste = text;
        _pendingPasteAppendReturn = appendReturn;
        _pendingPasteIsDrop = false;
        std::u16string shown = text;
        std::replace(shown.begin(), shown.end(), u'\r', u'\n');
        multilinePasteRequested(strings::utf16ToUtf8(shown));
        return;
    }

    sendPaste(text, appendReturn);
}

void TerminalView::sendPaste(std::u16string text, bool appendReturn)
{
    if (!_screenWindow)
        return;

    bracketText(text);

    // appendReturn is intentionally handled _after_ enclosing texts with brackets as
    // that feature is used to allow execution of commands immediately after paste.
    if (appendReturn)
        text.push_back(u'\r');

    KeyEvent e{0, NoModifier, strings::utf16ToUtf8(text)};
    keyPressed(e, true); // expose as a big fat keypress event

    _screenWindow->clearSelection();
}

void TerminalView::bracketText(std::u16string& text) const
{
    if (_bracketedPasteMode) {
        // An end marker inside the text would end the paste there and run the rest as typed.
        for (size_t end; (end = text.find(u"\033[201~")) != std::u16string::npos;)
            text.erase(end, 6);
        text.insert(0, u"\033[200~");
        text.append(u"\033[201~");
    }
}

void TerminalView::confirmPaste()
{
    if (_pendingPaste.empty())
        return;

    const std::u16string text = _pendingPaste;
    _pendingPaste.clear();

    if (_pendingPasteIsDrop) {
        const std::string bytes = strings::utf16ToUtf8(text);
        if (_session)
            _session->emulation()->sendString(bytes.c_str());
    } else {
        sendPaste(text, _pendingPasteAppendReturn);
    }
}

void TerminalView::dropText(const std::string& utf8)
{
    std::u16string dropText = strings::utf8ToUtf16(utf8);
    replaceAll(dropText, u"\r\n", u"\n");
    std::replace(dropText.begin(), dropText.end(), u'\n', u'\r');
    if (_trimPastedTrailingNewlines)
        trimTrailingReturns(dropText);
    if (_confirmMultilinePaste && dropText.find(u'\r') != std::u16string::npos) {
        _pendingPaste = dropText;
        _pendingPasteIsDrop = true;
        std::u16string shown = dropText;
        std::replace(shown.begin(), shown.end(), u'\r', u'\n');
        multilinePasteRequested(strings::utf16ToUtf8(shown));
        return;
    }

    const std::string bytes = strings::utf16ToUtf8(dropText);
    if (_session)
        _session->emulation()->sendString(bytes.c_str());
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                  Bell                                     */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::bell()
{
    if (_bellMode == NoBell)
        return;

    //limit the rate at which bells can occur
    //...mainly for sound effects where rapid bells in sequence
    //produce a horrible noise
    if (_allowBell) {
        _allowBell = false;
        _bellTimer.start(std::chrono::milliseconds(500));

        if (_bellMode == SystemBeepBell) {
            // QApplication::beep() does nothing on Wayland.
        } else if (_bellMode == NotifyBell) {
            notifyBell();
        } else if (_bellMode == VisualBell) {
            swapColorTable();
            _swapTimer.start(std::chrono::milliseconds(200));
        }
    }
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                  Links                                    */
/*                                                                           */
/* ------------------------------------------------------------------------- */

UrlFilter::HotSpot* TerminalView::linkHotSpotAt(TerminalImageFilterChain& chain, double x, double y)
{
    if (_image.empty())
        return nullptr;

    int charLine = 0;
    int charColumn = 0;
    getCharacterCellAt({x, y}, charLine, charColumn);

    // Matched on demand against the text on display.
    chain.addFilter(new UrlFilter());
    chain.setImage(_image.data(), _lines, _columns, _lineProperties);
    chain.process();

    return dynamic_cast<UrlFilter::HotSpot*>(chain.hotSpotAt(charLine, charColumn));
}

std::string TerminalView::linkAt(double x, double y)
{
    TerminalImageFilterChain chain;
    UrlFilter::HotSpot* spot = linkHotSpotAt(chain, x, y);
    return spot ? spot->url() : std::string();
}

std::vector<TerminalView::Segment> TerminalView::linkUnderlineAt(double x, double y)
{
    TerminalImageFilterChain chain;
    UrlFilter::HotSpot* spot = linkHotSpotAt(chain, x, y);
    if (!spot)
        return {};

    // Placed like the underline paintFilters() would draw.
    std::vector<Segment> segments;
    for (int line = spot->startLine(); line <= spot->endLine(); ++line) {
        const int startColumn = line == spot->startLine() ? spot->startColumn() : 0;
        int endColumn = _columns;
        if (line == spot->endLine()) {
            endColumn = spot->endColumn();
        } else {
            while (endColumn > startColumn
                   && konsole_isspace(char16_t(_image[size_t(loc(endColumn - 1, line))].character)))
                endColumn--;
        }

        const int bottom = (line + 1) * _metrics.height - 1 + _topMargin;
        // QFontMetrics::underlinePos() is a whole number.
        const int underline = bottom - _metrics.descent + kRound(_metrics.underlinePos);
        segments.push_back({double(startColumn * _metrics.width + _leftBaseMargin), double(underline),
                            double((endColumn - startColumn) * _metrics.width)});
    }

    return segments;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                  Find                                     */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TerminalView::find(const std::string& text, bool forwards, bool fromSelection, bool caseSensitive)
{
    if (!_screenWindow || !_session)
        return;

    if (text.empty()) {
        clearFind();
        return;
    }

    Emulation* emulation = _session->emulation();
    int startColumn = 0;
    int startLine = emulation->lineCount();

    if (fromSelection && hasSelection()) {
        if (forwards) {
            _screenWindow->screen()->getSelectionEnd(startColumn, startLine);
            startColumn++;
        } else {
            _screenWindow->screen()->getSelectionStart(startColumn, startLine);
        }
    } else if (forwards) {
        startLine = 0;
    }

    const RegExp regExp(RegExp::escape(strings::utf8ToUtf16(text)),
                        caseSensitive ? RegExp::NoOption : RegExp::CaseInsensitive);
    HistorySearch search(emulation, regExp, forwards, startColumn, startLine);
    const std::optional<HistorySearch::Result> match = search.search();

    if (match) {
        // Centre the match rather than pinning it to the top edge.
        _screenWindow->scrollTo(std::max(0, match->startLine - _screenWindow->windowLines() / 2));
        _screenWindow->setTrackOutput(false);
        _screenWindow->notifyOutputChanged();
        _screenWindow->setSelectionStart(match->startColumn, match->startLine - _screenWindow->currentLine(), false);
        _screenWindow->setSelectionEnd(match->endColumn, match->endLine - _screenWindow->currentLine());
        findResult(true);
    } else {
        _screenWindow->clearSelection();
        findResult(false);
    }
}

void TerminalView::clearFind()
{
    if (_screenWindow)
        _screenWindow->clearSelection();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                                 Painting                                  */
/*                                                                           */
/* ------------------------------------------------------------------------- */

namespace
{

// What TerminalGpuNode did for a frame: cell backgrounds, the cursor and box
// drawing as device-pixel rects, glyphs at whole device pixels.
class FramePainter
{
public:
    FramePainter(ui::Painter& painter, ui::PointF origin)
        : m_painter(painter)
        , m_scale(painter.scale())
        , m_origin{origin.x * painter.scale(), origin.y * painter.scale()}
        , m_device(painter.toDevice(origin.x), painter.toDevice(origin.y))
    {
    }

    double scale() const { return m_scale; }

    // Window device pixels from the painter's origin, as fillDeviceRect and
    // GlyphRenderer place them.
    int fromWindowX(int device) const { return device - m_device.x(); }
    int fromWindowY(int device) const { return device - m_device.y(); }

    int deviceX(double logical) const { return kRound(m_origin.x + logical * m_scale); }
    int deviceY(double logical) const { return kRound(m_origin.y + logical * m_scale); }

    // Device pixels of the window, filled exactly.
    void fill(const Lingmo::base::Rect& device, const Color& color)
    {
        if (device.isEmpty() || color.a == 0)
            return;
        ui::fillDeviceRect(m_painter,
                           {device.x() - m_device.x(), device.y() - m_device.y(), device.width(), device.height()},
                           color);
    }

    // Filled after the glyphs: decorations, box drawing, thin cursors.
    void fillOver(const Lingmo::base::Rect& device, const Color& color)
    {
        if (!device.isEmpty() && color.a != 0)
            m_over.push_back({device, color});
    }

    void flushOver()
    {
        for (const auto& [rect, color] : m_over)
            fill(rect, color);
        m_over.clear();
    }

    ui::Painter& painter() { return m_painter; }

private:
    ui::Painter& m_painter;
    double m_scale;
    ui::PointF m_origin;
    Lingmo::base::Point m_device;
    std::vector<std::pair<Lingmo::base::Rect, Color>> m_over;
};

} // namespace

static Lingmo::base::Rect rectFromCorners(int left, int top, int right, int bottom)
{
    // QRect(QPoint, QPoint): both corners inside.
    return Lingmo::base::Rect(left, top, std::max(0, right - left + 1), std::max(0, bottom - top + 1));
}

// drawLineChar() as device pixel rects. Edges go through the same mapping as
// the cells, so strokes cover what QPainter's aliased lines would, and meet
// the neighbouring cells' strokes exactly.
static void addLineChar(FramePainter& frame, double left, double top, int w, int h, int scaleX, int scaleY,
                        uint8_t code, bool bold, const Color& color)
{
    const uint32_t toDraw = LineChars[code];
    const int cx = w / 2;
    const int cy = h / 2;
    // A bold stroke is three pixels wide, and its square caps reach one further.
    const int grow = bold ? 1 : 0;

    auto rect = [&](int x0, int y0, int x1, int y1) {
        const int left0 = frame.deviceX(left + std::clamp(x0, 0, w) * scaleX);
        const int top0 = frame.deviceY(top + std::clamp(y0, 0, h) * scaleY);
        const int right = std::max(left0 + 1, frame.deviceX(left + std::clamp(x1, 0, w) * scaleX));
        const int bottom = std::max(top0 + 1, frame.deviceY(top + std::clamp(y1, 0, h) * scaleY));
        frame.fillOver(Lingmo::base::Rect(left0, top0, right - left0, bottom - top0), color);
    };
    // Both ends inclusive, as QPainter::drawLine() takes them.
    auto vertical = [&](int x, int y0, int y1) { rect(x - grow, y0 - grow, x + 1 + grow, y1 + 1 + grow); };
    auto horizontal = [&](int y, int x0, int x1) { rect(x0 - grow, y - grow, x1 + 1 + grow, y + 1 + grow); };
    auto point = [&](int x, int y) { rect(x - grow, y - grow, x + 1 + grow, y + 1 + grow); };

    if (toDraw & TopL)
        vertical(cx - 1, 0, cy - 2);
    if (toDraw & TopC)
        vertical(cx, 0, cy - 2);
    if (toDraw & TopR)
        vertical(cx + 1, 0, cy - 2);

    if (toDraw & BotL)
        vertical(cx - 1, cy + 2, h - 1);
    if (toDraw & BotC)
        vertical(cx, cy + 2, h - 1);
    if (toDraw & BotR)
        vertical(cx + 1, cy + 2, h - 1);

    if (toDraw & LeftT)
        horizontal(cy - 1, 0, cx - 2);
    if (toDraw & LeftC)
        horizontal(cy, 0, cx - 2);
    if (toDraw & LeftB)
        horizontal(cy + 1, 0, cx - 2);

    if (toDraw & RightT)
        horizontal(cy - 1, cx + 2, w - 1);
    if (toDraw & RightC)
        horizontal(cy, cx + 2, w - 1);
    if (toDraw & RightB)
        horizontal(cy + 1, cx + 2, w - 1);

    if (toDraw & Int11)
        point(cx - 1, cy - 1);
    if (toDraw & Int12)
        point(cx, cy - 1);
    if (toDraw & Int13)
        point(cx + 1, cy - 1);

    if (toDraw & Int21)
        point(cx - 1, cy);
    if (toDraw & Int22)
        point(cx, cy);
    if (toDraw & Int23)
        point(cx + 1, cy);

    if (toDraw & Int31)
        point(cx - 1, cy + 1);
    if (toDraw & Int32)
        point(cx, cy + 1);
    if (toDraw & Int33)
        point(cx + 1, cy + 1);
}

// drawOtherLineChar(): the dashes, arcs and diagonals LineChars has no entry
// for. The glyph atlas rasterised them with an unantialiased QPainter scaled
// to the device, a pen of one logical pixel (three for bold), so they are
// drawn the same way into a bitmap and its pixels filled as device rects.
static void addOtherLineChar(FramePainter& frame, double left, double top, int w, int h, int scaleX, int scaleY,
                             uint8_t code, bool bold, const Color& color)
{
    const double scale = frame.scale();
    const int padX = int(std::ceil(w * scale));
    const int padY = int(std::ceil(h * scale / 2));
    ui::Bitmap bitmap(int(std::ceil(w * scale)) + 2 * padX, int(std::ceil(h * scale)) + 2 * padY,
                      ui::Bitmap::Format::Alpha8);
    if (bitmap.isNull())
        return;
    ui::BitmapPainter painter(bitmap);
    painter.translate(padX, padY);
    painter.scale(scale, scale);
    const double penWidth = bold ? 3 : 1;
    const Color ink{255, 255, 255, 255};
    auto line = [&](int x0, int y0, int x1, int y1) {
        painter.drawLine({double(x0), double(y0)}, {double(x1), double(y1)}, ink, penWidth, ui::LineCap::Square);
    };

    const int cx = w / 2;
    const int cy = h / 2;
    const int ex = w - 1;
    const int ey = h - 1;

    // Double dashes
    if (0x4C <= code && code <= 0x4F) {
        const int xHalfGap = std::max(w / 15, 1);
        const int yHalfGap = std::max(h / 15, 1);
        switch (code) {
        case 0x4D: // BOX DRAWINGS HEAVY DOUBLE DASH HORIZONTAL
            line(0, cy - 1, cx - xHalfGap - 1, cy - 1);
            line(0, cy + 1, cx - xHalfGap - 1, cy + 1);
            line(cx + xHalfGap, cy - 1, ex, cy - 1);
            line(cx + xHalfGap, cy + 1, ex, cy + 1);
            [[fallthrough]];
        case 0x4C: // BOX DRAWINGS LIGHT DOUBLE DASH HORIZONTAL
            line(0, cy, cx - xHalfGap - 1, cy);
            line(cx + xHalfGap, cy, ex, cy);
            break;
        case 0x4F: // BOX DRAWINGS HEAVY DOUBLE DASH VERTICAL
            line(cx - 1, 0, cx - 1, cy - yHalfGap - 1);
            line(cx + 1, 0, cx + 1, cy - yHalfGap - 1);
            line(cx - 1, cy + yHalfGap, cx - 1, ey);
            line(cx + 1, cy + yHalfGap, cx + 1, ey);
            [[fallthrough]];
        case 0x4E: // BOX DRAWINGS LIGHT DOUBLE DASH VERTICAL
            line(cx, 0, cx, cy - yHalfGap - 1);
            line(cx, cy + yHalfGap, cx, ey);
            break;
        }
    }

    // Rounded corner characters
    else if (0x6D <= code && code <= 0x70) {
        const int r = w * 3 / 8;
        const int d = 2 * r;
        // QPainter::drawArc(x, y, d, d, start, 90 degrees).
        auto arc = [&](int x, int y, double startDegrees) {
            ui::Path path;
            path.arcTo({double(x), double(y), double(d), double(d)}, startDegrees, 90);
            painter.strokePath(path, ink, penWidth, ui::LineCap::Square, ui::LineJoin::Bevel);
        };
        switch (code) {
        case 0x6D: // BOX DRAWINGS LIGHT ARC DOWN AND RIGHT
            line(cx, cy + r, cx, ey);
            line(cx + r, cy, ex, cy);
            arc(cx, cy, 90);
            break;
        case 0x6E: // BOX DRAWINGS LIGHT ARC DOWN AND LEFT
            line(cx, cy + r, cx, ey);
            line(0, cy, cx - r, cy);
            arc(cx - d, cy, 0);
            break;
        case 0x6F: // BOX DRAWINGS LIGHT ARC UP AND LEFT
            line(cx, 0, cx, cy - r);
            line(0, cy, cx - r, cy);
            arc(cx - d, cy - d, 270);
            break;
        case 0x70: // BOX DRAWINGS LIGHT ARC UP AND RIGHT
            line(cx, 0, cx, cy - r);
            line(cx + r, cy, ex, cy);
            arc(cx, cy - d, 180);
            break;
        }
    }

    // Diagonals
    else if (0x71 <= code && code <= 0x73) {
        switch (code) {
        case 0x71: // BOX DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT
            line(ex, 0, 0, ey);
            break;
        case 0x72: // BOX DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT
            line(0, 0, ex, ey);
            break;
        case 0x73: // BOX DRAWINGS LIGHT DIAGONAL CROSS
            line(ex, 0, 0, ey);
            line(0, 0, ex, ey);
            break;
        }
    }

    // Each run of covered pixels in a row as one rect, stretched as the atlas
    // quad was for double-width and double-height lines.
    const int cellX = frame.deviceX(left);
    const int cellY = frame.deviceY(top);
    for (int y = 0; y < bitmap.height(); ++y) {
        int x = 0;
        while (x < bitmap.width()) {
            if (bitmap.alpha(x, y) < 128) {
                ++x;
                continue;
            }
            const int start = x;
            while (x < bitmap.width() && bitmap.alpha(x, y) >= 128)
                ++x;
            frame.fillOver(Lingmo::base::Rect(cellX + (start - padX) * scaleX, cellY + (y - padY) * scaleY,
                                                (x - start) * scaleX, scaleY),
                           color);
        }
    }
}

// Mirrors TerminalGpuNode::build(), which mirrored TerminalDisplay::paint().
void TerminalView::paint(ui::Painter& painter)
{
    // The window repaints its focus view when the keyboard comes or goes.
    if (hasKeyboard()) {
        if (_hasBlinkingCursor && !_blinkCursorTimer.isActive())
            _blinkCursorTimer.start(std::chrono::milliseconds(CURSOR_BLINK_INTERVAL));
    } else if (_blinkCursorTimer.isActive()) {
        _blinkCursorTimer.stop();
        _cursorBlinking = false;
    }

    if (_image.empty() || geometry().width <= 0 || geometry().height <= 0)
        return;

    FramePainter frame(painter, painter.offset());

    const int fw = _metrics.width;
    const int fh = _metrics.height;
    const double dpr = frame.scale();
    const int lineThickness = std::max(1, kRound(dpr));
    const double underlinePos = _metrics.underlinePos;
    const double strikeOutPos = _metrics.strikeOutPos;
    const double overlinePos = _metrics.overlinePos;
    const int decorationThickness = std::max(1, kRound(_metrics.lineWidth * dpr));
    const Color frameBackground = _colorTable[DEFAULT_BACK_COLOR].color;

    if (_opacity > 0) {
        frame.fill(rectFromCorners(frame.deviceX(0), frame.deviceY(0), frame.deviceX(geometry().width) - 1,
                                   frame.deviceY(geometry().height) - 1),
                   frameBackground.withAlpha(_opacity));
    }

    // The cursor; returns whether the text under it takes the cell's background colour.
    auto addCursor = [&](double left, double top, double width, double scaleY, const Color& foreground) {
        const Color color = foreground;
        const double height = (fh - (_fullCursorHeight ? 0 : _lineSpacing - 1)) * scaleY;
        const Lingmo::base::Rect rect = rectFromCorners(frame.deviceX(left), frame.deviceY(top),
                                                          frame.deviceX(left + width) - 1,
                                                          frame.deviceY(top + height) - 1);

        switch (_cursorShape) {
        case 1:
            frame.fillOver(Lingmo::base::Rect(rect.x(), rect.bottom() - lineThickness, rect.width(), lineThickness),
                           color);
            return false;
        case 2:
            frame.fillOver(Lingmo::base::Rect(rect.x(), rect.y(), lineThickness, rect.height()), color);
            return false;
        default:
            frame.fill(rect, color);
            return true;
        }
    };

    auto addDecorations = [&](const Lingmo::base::Rect& cell, double top, int scaleY, bool underline,
                              bool strikeOut, bool overline, const Color& color) {
        const double baseline = top + (_metrics.ascent + _lineSpacing) * scaleY;
        auto line = [&](double y) {
            frame.fillOver(Lingmo::base::Rect(cell.x(), frame.deviceY(y), cell.width(), decorationThickness * scaleY),
                           color);
        };
        if (underline)
            line(baseline + underlinePos * scaleY);
        if (strikeOut)
            line(baseline - strikeOutPos * scaleY);
        if (overline)
            line(baseline - overlinePos * scaleY);
    };

    auto styleOf = [&](const Character& c) {
        int style = TerminalFont::Regular;
        if ((c.rendition & RE_BOLD) && _boldIntense)
            style |= TerminalFont::Bold;
        if ((c.rendition & RE_ITALIC) && _italicEnabled)
            style |= TerminalFont::Italic;
        return style;
    };

    const Point cursorPos = _screenWindow ? _screenWindow->cursorPosition() : Point(0, 0);
    const int preeditCells = _preedit.empty() ? 0 : string_width(_preedit);
    const int usedLines = drawnLines();

    // The atlas drew a glyph with its origin at the cell's left and its baseline
    // (ascent + line spacing) * dpr below the row's top, stretched from there.
    struct Glyph
    {
        std::u32string cluster;
        int style;
        Color color;
        int x;
        int rowTop;
        int scaleX;
        int scaleY;
    };
    std::vector<Glyph> glyphs;

    for (int y = 0; y < usedLines; ++y) {
        const LineProperty property = y < int(_lineProperties.size()) ? _lineProperties[size_t(y)] : LineProperty(0);
        const int scaleX = (property & LINE_DOUBLEWIDTH) ? 2 : 1;
        const int scaleY = (property & LINE_DOUBLEHEIGHT) ? 2 : 1;
        const double top = _topMargin + y * fh;
        const int rowTop = frame.deviceY(top);
        const int rowBottom = frame.deviceY(top + fh * scaleY);
        const Character* line = &_image[size_t(y * _columns)];

        for (int x = 0; x < _usedColumns; ++x) {
            const Character& c = line[x];
            const int cells = (x + 1 < _usedColumns && line[x + 1].character == 0) ? 2 : 1;

            // The preedit string covers these cells.
            if (preeditCells && y == cursorPos.y() && x >= cursorPos.x() && x < cursorPos.x() + preeditCells) {
                x += cells - 1;
                continue;
            }

            const double left = _leftMargin + x * fw * scaleX;
            const double width = fw * scaleX * cells;
            const Lingmo::base::Rect cell = rectFromCorners(frame.deviceX(left), rowTop,
                                                              frame.deviceX(left + width) - 1, rowBottom - 1);
            const Color foreground = c.foregroundColor.color(_colorTable);
            const Color background = c.backgroundColor.color(_colorTable);

            if (background != frameBackground)
                frame.fill(cell, background);

            bool invert = false;
            if ((c.rendition & RE_CURSOR) && !_cursorBlinking)
                invert = addCursor(left, top, width, scaleY, foreground);

            x += cells - 1;

            if (_blinking && (c.rendition & RE_BLINK))
                continue;

            const Color color = invert ? background : foreground;
            const int style = styleOf(c);
            addDecorations(cell, top, scaleY, c.rendition & RE_UNDERLINE, false, false, color);

            if (!c.character || c.character == L' ')
                continue;

            if (_drawLineChars && c.isLineChar()) {
                const uint8_t code = uint8_t(c.character & 0xff);
                const bool bold = (c.rendition & RE_BOLD) && _boldIntense;
                if (LineChars[code])
                    addLineChar(frame, left, top, fw, fh, scaleX, scaleY, code, bold, color);
                else
                    addOtherLineChar(frame, left, top, fw, fh, scaleX, scaleY, code, bold, color);
                continue;
            }

            std::u32string cluster;
            if (c.rendition & RE_EXTENDED_CHAR) {
                ushort length = 0;
                const uint* chars = ExtendedCharTable::instance.lookupExtendedChar(c.character, length);
                if (chars)
                    cluster.assign(chars, chars + length);
            } else {
                cluster.push_back(char32_t(c.character));
            }
            if (!cluster.empty())
                glyphs.push_back({std::move(cluster), style, color, cell.x(), rowTop, scaleX, scaleY});
        }

        // Both halves of a double-height line hold the same text; the first draws it.
        if (scaleY == 2)
            ++y;
    }

    // drawInputMethodPreeditString(): the default background, the cursor, then the text.
    if (preeditCells && cursorPos.y() < usedLines) {
        const double left = _leftMargin + cursorPos.x() * fw;
        const double top = _topMargin + cursorPos.y() * fh;
        const int rowTop = frame.deviceY(top);
        const Character& style = _image[size_t(cursorPos.y() * _columns + cursorPos.x())];

        Color background = _colorTable[DEFAULT_BACK_COLOR].color;
        background = background.withAlpha(_opacity);
        frame.fill(rectFromCorners(frame.deviceX(left), rowTop, frame.deviceX(left + fw * preeditCells) - 1,
                                   frame.deviceY(top + fh) - 1),
                   background);

        bool invert = false;
        if (!_cursorBlinking)
            invert = addCursor(left, top, fw * preeditCells, 1, _colorTable[DEFAULT_FORE_COLOR].color);

        const Color color = (invert ? style.backgroundColor : style.foregroundColor).color(_colorTable);
        const int glyphStyle = styleOf(style);
        int column = 0;
        size_t i = 0;
        while (i < _preedit.size()) {
            std::u32string cluster(1, char32_t(_preedit[i]));
            const int cells = std::max(1, konsole_wcwidth(_preedit[i]));
            for (++i; i < _preedit.size() && konsole_wcwidth(_preedit[i]) == 0; ++i)
                cluster.push_back(char32_t(_preedit[i]));

            glyphs.push_back({std::move(cluster), glyphStyle, color, frame.deviceX(left + column * fw), rowTop, 1,
                              1});
            column += cells;
        }
    }

    // Runs of one style, colour and row in one call; a stretched glyph alone, about its own cell.
    const double baselineOffset = (_metrics.ascent + _lineSpacing) * dpr;
    std::vector<ui::GlyphCluster> run;
    for (size_t i = 0; i < glyphs.size();) {
        const Glyph& first = glyphs[i];
        run.clear();
        size_t end = i;
        do {
            run.push_back({glyphs[end].cluster, double(frame.fromWindowX(glyphs[end].x))});
            ++end;
        } while (end < glyphs.size() && first.scaleX == 1 && first.scaleY == 1 && glyphs[end].scaleX == 1
                 && glyphs[end].scaleY == 1 && glyphs[end].style == first.style && glyphs[end].color == first.color
                 && glyphs[end].rowTop == first.rowTop);

        const int rowTop = frame.fromWindowY(first.rowTop);
        _font->draw(painter, first.style, first.color, dpr, rowTop + baselineOffset, run,
                    ui::SizeF{double(first.scaleX), double(first.scaleY)},
                    ui::PointF{double(frame.fromWindowX(first.x)), double(rowTop)});
        i = end;
    }

    frame.flushOver();

    // The link under the pointer: underlined on hover, as one-pixel Rectangles.
    for (const Segment& segment : _linkSegments)
        painter.fillRect({segment.x, segment.y, segment.width, 1}, _linkColor);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                     The item's MouseArea and key handler                  */
/*                                                                           */
/* ------------------------------------------------------------------------- */

static uint32_t buttonBit(ui::MouseButton button)
{
    return 1u << int(button);
}

void TerminalView::updatePointerCursor()
{
    if (!_linkSegments.empty() && _ctrlHeld)
        View::setCursorShape(ui::CursorShape::Pointer);
    else
        View::setCursorShape(terminalUsesMouse() ? ui::CursorShape::Default : ui::CursorShape::Text);
}

void TerminalView::setLinkColor(Color color)
{
    _linkColor = color;
    if (!_linkSegments.empty())
        requestPaint();
}

// Only a move to another cell can change the link under the pointer.
void TerminalView::updateLinkHover(ui::PointF position, uint32_t modifiers)
{
    const bool ctrlHeld = (modifiers & ui::ControlModifier) != 0;
    if (ctrlHeld != _ctrlHeld) {
        _ctrlHeld = ctrlHeld;
        updatePointerCursor();
    }
    _hoverPosition = position;

    const std::pair<int, int> cell{int(std::floor(position.x / std::max(1, _metrics.width))),
                                   int(std::floor(position.y / std::max(1, _metrics.height)))};
    if (cell == _hoverCell)
        return;

    _hoverCell = cell;
    _linkSegments = linkUnderlineAt(position.x, position.y);
    updatePointerCursor();
    requestPaint();
}

void TerminalView::refreshLinkHover()
{
    if (_hoverPosition.x >= 0) {
        _linkSegments = linkUnderlineAt(_hoverPosition.x, _hoverPosition.y);
        updatePointerCursor();
        requestPaint();
    }
}

void TerminalView::clearLinkHover()
{
    _hoverPosition = {-1, -1};
    _hoverCell = {-1, -1};
    if (!_linkSegments.empty()) {
        _linkSegments.clear();
        updatePointerCursor();
        requestPaint();
    }
}

void TerminalView::hoverMove(const ui::PointerEvent& event)
{
    _lastPointer = event.position;
    if (_buttons == 0)
        updateLinkHover(event.position, event.modifiers);
}

void TerminalView::hoverChanged()
{
    if (!isHovered())
        clearLinkHover();
}

void TerminalView::pointerPress(const ui::PointerEvent& event)
{
    const double x = event.position.x;
    const double y = event.position.y;
    _buttons |= buttonBit(event.button);

    _menuPress = event.button == ui::MouseButton::Right
            && (!terminalUsesMouse() || (event.modifiers & ui::ShiftModifier));

    _pressedLink = event.button == ui::MouseButton::Left && (event.modifiers & ui::ControlModifier)
            ? linkAt(x, y)
            : std::string();
    if (!_pressedLink.empty())
        return;

    if (!_menuPress)
        simulateMousePress(x, y, event.button, event.modifiers);

    // The second press of a double click, as Qt Quick delivers it after the press.
    if (event.clickCount == 2)
        simulateMouseDoubleClick(x, y, event.button, event.modifiers);
}

void TerminalView::pointerMove(const ui::PointerEvent& event)
{
    _lastPointer = event.position;
    if (!_menuPress && _pressedLink.empty())
        simulateMouseMove(event.position.x, event.position.y, _buttons, event.modifiers);
}

void TerminalView::pointerRelease(const ui::PointerEvent& event)
{
    const double x = event.position.x;
    const double y = event.position.y;
    _buttons &= ~buttonBit(event.button);

    if (!_pressedLink.empty()) {
        const std::string link = std::exchange(_pressedLink, std::string());
        if (onLinkActivated)
            onLinkActivated(link);
        return;
    }

    if (_menuPress) {
        _menuPress = false;
        if (onContextMenu)
            onContextMenu(x, y);
        return;
    }

    simulateMouseRelease(x, y, event.button, event.modifiers);

    // A right click the program took stays its own.
    if (event.button == ui::MouseButton::Left && x >= 0 && y >= 0 && x < geometry().width && y < geometry().height)
        setFocus(ui::FocusReason::Mouse);
}

bool TerminalView::keyRelease(const ui::KeyEvent& event)
{
    if (event.key == XKB_KEY_Control_L || event.key == XKB_KEY_Control_R) {
        _ctrlHeld = false;
        updatePointerCursor();
    }
    return false;
}
