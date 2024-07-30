/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "searchmode.h"

#include "../globalstate.h"
#include "../history.h"
#include "katedocument.h"
#include "kateview.h"
#include <vimode/inputmodemanager.h>
#include <vimode/modes/modebase.h>

#include <KColorScheme>

#include <QApplication>
#include <QLineEdit>

using namespace KateVi;

namespace
{
bool isCharEscaped(const QString &string, int charPos)
{
    if (charPos == 0) {
        return false;
    }
    int numContiguousBackslashesToLeft = 0;
    charPos--;
    while (charPos >= 0 && string[charPos] == QLatin1Char('\\')) {
        numContiguousBackslashesToLeft++;
        charPos--;
    }
    return ((numContiguousBackslashesToLeft % 2) == 1);
}

QString toggledEscaped(const QString &originalString, QChar escapeChar)
{
    int searchFrom = 0;
    QString toggledEscapedString = originalString;
    do {
        const int indexOfEscapeChar = toggledEscapedString.indexOf(escapeChar, searchFrom);
        if (indexOfEscapeChar == -1) {
            break;
        }
        if (!isCharEscaped(toggledEscapedString, indexOfEscapeChar)) {
            // Escape.
            toggledEscapedString.replace(indexOfEscapeChar, 1, QLatin1String("\\") + escapeChar);
            searchFrom = indexOfEscapeChar + 2;
        } else {
            // Unescape.
            toggledEscapedString.remove(indexOfEscapeChar - 1, 1);
            searchFrom = indexOfEscapeChar;
        }
    } while (true);

    return toggledEscapedString;
}

int findPosOfSearchConfigMarker(const QString &searchText, const bool isSearchBackwards)
{
    const QChar searchConfigMarkerChar = (isSearchBackwards ? QLatin1Char('?') : QLatin1Char('/'));
    for (int pos = 0; pos < searchText.length(); pos++) {
        if (searchText.at(pos) == searchConfigMarkerChar) {
            if (!isCharEscaped(searchText, pos)) {
                return pos;
            }
        }
    }
    return -1;
}

bool isRepeatLastSearch(const QString &searchText, const bool isSearchBackwards)
{
    const int posOfSearchConfigMarker = findPosOfSearchConfigMarker(searchText, isSearchBackwards);
    if (posOfSearchConfigMarker != -1) {
        if (QStringView(searchText).left(posOfSearchConfigMarker).isEmpty()) {
            return true;
        }
    }
    return false;
}

bool shouldPlaceCursorAtEndOfMatch(const QString &searchText, const bool isSearchBackwards)
{
    const int posOfSearchConfigMarker = findPosOfSearchConfigMarker(searchText, isSearchBackwards);
    if (posOfSearchConfigMarker != -1) {
        if (searchText.length() > posOfSearchConfigMarker + 1 && searchText.at(posOfSearchConfigMarker + 1) == QLatin1Char('e')) {
            return true;
        }
    }
    return false;
}

QString withSearchConfigRemoved(const QString &originalSearchText, const bool isSearchBackwards)
{
    const int posOfSearchConfigMarker = findPosOfSearchConfigMarker(originalSearchText, isSearchBackwards);
    if (posOfSearchConfigMarker == -1) {
        return originalSearchText;
    } else {
        return originalSearchText.left(posOfSearchConfigMarker);
    }
}
}

QString KateVi::vimRegexToQtRegexPattern(const QString &vimRegexPattern)
{
    QString qtRegexPattern = vimRegexPattern;
    qtRegexPattern = toggledEscaped(qtRegexPattern, QLatin1Char('('));
    qtRegexPattern = toggledEscaped(qtRegexPattern, QLatin1Char(')'));
    qtRegexPattern = toggledEscaped(qtRegexPattern, QLatin1Char('+'));
    qtRegexPattern = toggledEscaped(qtRegexPattern, QLatin1Char('|'));
    qtRegexPattern = ensuredCharEscaped(qtRegexPattern, QLatin1Char('?'));
    {
        // All curly brackets, except the closing curly bracket of a matching pair where the opening bracket is escaped,
        // must have their escaping toggled.
        bool lookingForMatchingCloseBracket = false;
        QList<int> matchingClosedCurlyBracketPositions;
        for (int i = 0; i < qtRegexPattern.length(); i++) {
            if (qtRegexPattern[i] == QLatin1Char('{') && isCharEscaped(qtRegexPattern, i)) {
                lookingForMatchingCloseBracket = true;
            }
            if (qtRegexPattern[i] == QLatin1Char('}') && lookingForMatchingCloseBracket && qtRegexPattern[i - 1] != QLatin1Char('\\')) {
                matchingClosedCurlyBracketPositions.append(i);
            }
        }
        if (matchingClosedCurlyBracketPositions.isEmpty()) {
            // Escape all {'s and }'s - there are no matching pairs.
            qtRegexPattern = toggledEscaped(qtRegexPattern, QLatin1Char('{'));
            qtRegexPattern = toggledEscaped(qtRegexPattern, QLatin1Char('}'));
        } else {
            // Ensure that every chunk of qtRegexPattern that does *not* contain a curly closing bracket
            // that is matched have their { and } escaping toggled.
            QString qtRegexPatternNonMatchingCurliesToggled;
            int previousNonMatchingClosedCurlyPos = 0; // i.e. the position of the last character which is either
            // not a curly closing bracket, or is a curly closing bracket
            // that is not matched.
            for (int matchingClosedCurlyPos : std::as_const(matchingClosedCurlyBracketPositions)) {
                QString chunkExcludingMatchingCurlyClosed =
                    qtRegexPattern.mid(previousNonMatchingClosedCurlyPos, matchingClosedCurlyPos - previousNonMatchingClosedCurlyPos);
                chunkExcludingMatchingCurlyClosed = toggledEscaped(chunkExcludingMatchingCurlyClosed, QLatin1Char('{'));
                chunkExcludingMatchingCurlyClosed = toggledEscaped(chunkExcludingMatchingCurlyClosed, QLatin1Char('}'));
                qtRegexPatternNonMatchingCurliesToggled += chunkExcludingMatchingCurlyClosed + qtRegexPattern[matchingClosedCurlyPos];
                previousNonMatchingClosedCurlyPos = matchingClosedCurlyPos + 1;
            }
            QString chunkAfterLastMatchingClosedCurly = qtRegexPattern.mid(matchingClosedCurlyBracketPositions.last() + 1);
            chunkAfterLastMatchingClosedCurly = toggledEscaped(chunkAfterLastMatchingClosedCurly, QLatin1Char('{'));
            chunkAfterLastMatchingClosedCurly = toggledEscaped(chunkAfterLastMatchingClosedCurly, QLatin1Char('}'));
            qtRegexPatternNonMatchingCurliesToggled += chunkAfterLastMatchingClosedCurly;

            qtRegexPattern = qtRegexPatternNonMatchingCurliesToggled;
        }
    }

    // All square brackets, *except* for those that are a) unescaped; and b) form a matching pair, must be
    // escaped.
    bool lookingForMatchingCloseBracket = false;
    int openingBracketPos = -1;
    QList<int> matchingSquareBracketPositions;
    for (int i = 0; i < qtRegexPattern.length(); i++) {
        if (qtRegexPattern[i] == QLatin1Char('[') && !isCharEscaped(qtRegexPattern, i) && !lookingForMatchingCloseBracket) {
            lookingForMatchingCloseBracket = true;
            openingBracketPos = i;
        }
        if (qtRegexPattern[i] == QLatin1Char(']') && lookingForMatchingCloseBracket && !isCharEscaped(qtRegexPattern, i)) {
            lookingForMatchingCloseBracket = false;
            matchingSquareBracketPositions.append(openingBracketPos);
            matchingSquareBracketPositions.append(i);
        }
    }

    if (matchingSquareBracketPositions.isEmpty()) {
        // Escape all ['s and ]'s - there are no matching pairs.
        qtRegexPattern = ensuredCharEscaped(qtRegexPattern, QLatin1Char('['));
        qtRegexPattern = ensuredCharEscaped(qtRegexPattern, QLatin1Char(']'));
    } else {
        // Ensure that every chunk of qtRegexPattern that does *not* contain one of the matching pairs of
        // square brackets have their square brackets escaped.
        QString qtRegexPatternNonMatchingSquaresMadeLiteral;
        int previousNonMatchingSquareBracketPos = 0; // i.e. the position of the last character which is
        // either not a square bracket, or is a square bracket but
        // which is not matched.
        for (int matchingSquareBracketPos : std::as_const(matchingSquareBracketPositions)) {
            QString chunkExcludingMatchingSquareBrackets =
                qtRegexPattern.mid(previousNonMatchingSquareBracketPos, matchingSquareBracketPos - previousNonMatchingSquareBracketPos);
            chunkExcludingMatchingSquareBrackets = ensuredCharEscaped(chunkExcludingMatchingSquareBrackets, QLatin1Char('['));
            chunkExcludingMatchingSquareBrackets = ensuredCharEscaped(chunkExcludingMatchingSquareBrackets, QLatin1Char(']'));
            qtRegexPatternNonMatchingSquaresMadeLiteral += chunkExcludingMatchingSquareBrackets + qtRegexPattern[matchingSquareBracketPos];
            previousNonMatchingSquareBracketPos = matchingSquareBracketPos + 1;
        }
        QString chunkAfterLastMatchingSquareBracket = qtRegexPattern.mid(matchingSquareBracketPositions.last() + 1);
        chunkAfterLastMatchingSquareBracket = ensuredCharEscaped(chunkAfterLastMatchingSquareBracket, QLatin1Char('['));
        chunkAfterLastMatchingSquareBracket = ensuredCharEscaped(chunkAfterLastMatchingSquareBracket, QLatin1Char(']'));
        qtRegexPatternNonMatchingSquaresMadeLiteral += chunkAfterLastMatchingSquareBracket;

        qtRegexPattern = qtRegexPatternNonMatchingSquaresMadeLiteral;
    }

    qtRegexPattern.replace(QLatin1String("\\>"), QLatin1String("\\b"));
    qtRegexPattern.replace(QLatin1String("\\<"), QLatin1String("\\b"));

    return qtRegexPattern;
}

QString KateVi::ensuredCharEscaped(const QString &originalString, QChar charToEscape)
{
    QString escapedString = originalString;
    for (int i = 0; i < escapedString.length(); i++) {
        if (escapedString[i] == charToEscape && !isCharEscaped(escapedString, i)) {
            escapedString.replace(i, 1, QLatin1String("\\") + charToEscape);
        }
    }
    return escapedString;
}

QString KateVi::withCaseSensitivityMarkersStripped(const QString &originalSearchTerm)
{
    // Only \C is handled, for now - I'll implement \c if someone asks for it.
    int pos = 0;
    QString caseSensitivityMarkersStripped = originalSearchTerm;
    while (pos < caseSensitivityMarkersStripped.length()) {
        if (caseSensitivityMarkersStripped.at(pos) == QLatin1Char('C') && isCharEscaped(caseSensitivityMarkersStripped, pos)) {
            caseSensitivityMarkersStripped.remove(pos - 1, 2);
            pos--;
        }
        pos++;
    }
    return caseSensitivityMarkersStripped;
}

QStringList KateVi::reversed(const QStringList &originalList)
{
    QStringList reversedList = originalList;
    std::reverse(reversedList.begin(), reversedList.end());
    return reversedList;
}

SearchMode::SearchMode(EmulatedCommandBar *emulatedCommandBar,
                       MatchHighlighter *matchHighlighter,
                       InputModeManager *viInputModeManager,
                       KTextEditor::ViewPrivate *view,
                       QLineEdit *edit)
    : ActiveMode(emulatedCommandBar, matchHighlighter, viInputModeManager, view)
    , m_edit(edit)
{
}

void SearchMode::init(SearchMode::SearchDirection searchDirection)
{
    m_searchDirection = searchDirection;
    m_startingCursorPos = view()->cursorPosition();
}

bool SearchMode::handleKeyPress(const QKeyEvent *keyEvent)
{
    Q_UNUSED(keyEvent);
    return false;
}

void SearchMode::editTextChanged(const QString &newText)
{
    QString qtRegexPattern = newText;
    const bool searchBackwards = (m_searchDirection == SearchDirection::Backward);
    const bool placeCursorAtEndOfMatch = shouldPlaceCursorAtEndOfMatch(qtRegexPattern, searchBackwards);
    if (isRepeatLastSearch(qtRegexPattern, searchBackwards)) {
        qtRegexPattern = viInputModeManager()->searcher()->getLastSearchPattern();
    } else {
        qtRegexPattern = withSearchConfigRemoved(qtRegexPattern, searchBackwards);
        qtRegexPattern = vimRegexToQtRegexPattern(qtRegexPattern);
    }

    // Decide case-sensitivity via SmartCase (note: if the expression contains \C, the "case-sensitive" marker, then
    // we will be case-sensitive "by coincidence", as it were.).
    bool caseSensitive = true;
    if (qtRegexPattern.toLower() == qtRegexPattern) {
        caseSensitive = false;
    }

    qtRegexPattern = withCaseSensitivityMarkersStripped(qtRegexPattern);

    m_currentSearchParams.pattern = qtRegexPattern;
    m_currentSearchParams.isCaseSensitive = caseSensitive;
    m_currentSearchParams.isBackwards = searchBackwards;
    m_currentSearchParams.shouldPlaceCursorAtEndOfMatch = placeCursorAtEndOfMatch;

    // The "count" for the current search is not shared between Visual & Normal mode, so we need to pick
    // the right one to handle the counted search.
    int c = viInputModeManager()->getCurrentViModeHandler()->getCount();
    KTextEditor::Range match = viInputModeManager()->searcher()->findPattern(m_currentSearchParams,
                                                                             m_startingCursorPos,
                                                                             c,
                                                                             false /* Don't add incremental searches to search history */);

    if (match.isValid()) {
        // The returned range ends one past the last character of the match, so adjust.
        KTextEditor::Cursor realMatchEnd = KTextEditor::Cursor(match.end().line(), match.end().column() - 1);
        if (realMatchEnd.column() == -1) {
            realMatchEnd = KTextEditor::Cursor(realMatchEnd.line() - 1, view()->doc()->lineLength(realMatchEnd.line() - 1));
        }
        moveCursorTo(placeCursorAtEndOfMatch ? realMatchEnd : match.start());
        setBarBackground(SearchMode::MatchFound);
    } else {
        moveCursorTo(m_startingCursorPos);
        if (!m_edit->text().isEmpty()) {
            setBarBackground(SearchMode::NoMatchFound);
        } else {
            setBarBackground(SearchMode::Normal);
        }
    }

    if (!viInputModeManager()->searcher()->isHighlightSearchEnabled())
        updateMatchHighlight(match);
}

void SearchMode::deactivate(bool wasAborted)
{
    // "Deactivate" can be called multiple times between init()'s, so only reset the cursor once!
    if (m_startingCursorPos.isValid()) {
        if (wasAborted) {
            moveCursorTo(m_startingCursorPos);
        }
    }
    m_startingCursorPos = KTextEditor::Cursor::invalid();
    setBarBackground(SearchMode::Normal);
    // Send a synthetic keypress through the system that signals whether the search was aborted or
    // not.  If not, the keypress will "complete" the search motion, thus triggering it.
    // We send to KateViewInternal as it updates the status bar and removes the "?".
    const Qt::Key syntheticSearchCompletedKey = (wasAborted ? static_cast<Qt::Key>(0) : Qt::Key_Enter);
    QKeyEvent syntheticSearchCompletedKeyPress(QEvent::KeyPress, syntheticSearchCompletedKey, Qt::NoModifier);
    m_isSendingSyntheticSearchCompletedKeypress = true;
    QApplication::sendEvent(view()->focusProxy(), &syntheticSearchCompletedKeyPress);
    m_isSendingSyntheticSearchCompletedKeypress = false;
    if (!wasAborted) {
        // Search was actually executed, so store it as the last search.
        viInputModeManager()->searcher()->setLastSearchParams(m_currentSearchParams);
    }
    // Append the raw text of the search to the search history (i.e. without conversion
    // from Vim-style regex; without case-sensitivity markers stripped; etc.
    // Vim does this even if the search was aborted, so we follow suit.
    viInputModeManager()->globalState()->searchHistory()->append(m_edit->text());
    viInputModeManager()->searcher()->patternDone(wasAborted);
}

CompletionStartParams SearchMode::completionInvoked(Completer::CompletionInvocation invocationType)
{
    Q_UNUSED(invocationType);
    return activateSearchHistoryCompletion();
}

void SearchMode::completionChosen()
{
    // Choose completion with Enter/ Return -> close bar (the search will have already taken effect at this point), marking as not aborted .
    close(false);
}

CompletionStartParams SearchMode::activateSearchHistoryCompletion()
{
    return CompletionStartParams::createModeSpecific(reversed(viInputModeManager()->globalState()->searchHistory()->items()), 0);
}

void SearchMode::setBarBackground(SearchMode::BarBackgroundStatus status)
{
    QPalette barBackground(m_edit->palette());
    switch (status) {
    case MatchFound: {
        KColorScheme::adjustBackground(barBackground, KColorScheme::PositiveBackground);
        break;
    }
    case NoMatchFound: {
        KColorScheme::adjustBackground(barBackground, KColorScheme::NegativeBackground);
        break;
    }
    case Normal: {
        barBackground = QPalette();
        break;
    }
    }
    m_edit->setPalette(barBackground);
}
