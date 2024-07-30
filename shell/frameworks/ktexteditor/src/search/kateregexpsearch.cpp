/*
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN includes
#include "kateregexpsearch.h"

#include "katepartdebug.h" // for LOG_KTE

#include <ktexteditor/document.h>
// END  includes

// Turn debug messages on/off here
// #define FAST_DEBUG_ENABLE

#ifdef FAST_DEBUG_ENABLE
#define FAST_DEBUG(x) qCDebug(LOG_KTE) << x
#else
#define FAST_DEBUG(x)
#endif

class KateRegExpSearch::ReplacementStream
{
public:
    struct counter {
        counter(int value, int minWidth)
            : value(value)
            , minWidth(minWidth)
        {
        }

        const int value;
        const int minWidth;
    };

    struct cap {
        cap(int n)
            : n(n)
        {
        }

        const int n;
    };

    enum CaseConversion {
        upperCase, ///< \U ... uppercase from now on
        upperCaseFirst, ///< \u ... uppercase the first letter
        lowerCase, ///< \L ... lowercase from now on
        lowerCaseFirst, ///< \l ... lowercase the first letter
        keepCase ///< \E ... back to original case
    };

public:
    ReplacementStream(const QStringList &capturedTexts);

    QString str() const
    {
        return m_str;
    }

    ReplacementStream &operator<<(const QString &);
    ReplacementStream &operator<<(const counter &);
    ReplacementStream &operator<<(const cap &);
    ReplacementStream &operator<<(CaseConversion);

private:
    const QStringList m_capturedTexts;
    CaseConversion m_caseConversion;
    QString m_str;
};

KateRegExpSearch::ReplacementStream::ReplacementStream(const QStringList &capturedTexts)
    : m_capturedTexts(capturedTexts)
    , m_caseConversion(keepCase)
{
}

KateRegExpSearch::ReplacementStream &KateRegExpSearch::ReplacementStream::operator<<(const QString &str)
{
    switch (m_caseConversion) {
    case upperCase:
        // Copy as uppercase
        m_str.append(str.toUpper());
        break;

    case upperCaseFirst:
        if (str.length() > 0) {
            m_str.append(str.at(0).toUpper());
            m_str.append(QStringView(str).mid(1));
            m_caseConversion = keepCase;
        }
        break;

    case lowerCase:
        // Copy as lowercase
        m_str.append(str.toLower());
        break;

    case lowerCaseFirst:
        if (str.length() > 0) {
            m_str.append(str.at(0).toLower());
            m_str.append(QStringView(str).mid(1));
            m_caseConversion = keepCase;
        }
        break;

    case keepCase: // FALLTHROUGH
    default:
        // Copy unmodified
        m_str.append(str);
        break;
    }

    return *this;
}

KateRegExpSearch::ReplacementStream &KateRegExpSearch::ReplacementStream::operator<<(const counter &c)
{
    // Zero padded counter value
    m_str.append(QStringLiteral("%1").arg(c.value, c.minWidth, 10, QLatin1Char('0')));

    return *this;
}

KateRegExpSearch::ReplacementStream &KateRegExpSearch::ReplacementStream::operator<<(const cap &cap)
{
    if (0 <= cap.n && cap.n < m_capturedTexts.size()) {
        (*this) << m_capturedTexts[cap.n];
    } else {
        // Insert just the number to be consistent with QRegExp ("\c" becomes "c")
        m_str.append(QString::number(cap.n));
    }

    return *this;
}

KateRegExpSearch::ReplacementStream &KateRegExpSearch::ReplacementStream::operator<<(CaseConversion caseConversion)
{
    m_caseConversion = caseConversion;

    return *this;
}

// BEGIN d'tor, c'tor
//
// KateSearch Constructor
//
KateRegExpSearch::KateRegExpSearch(const KTextEditor::Document *document)
    : m_document(document)
{
}

// helper structs for captures re-construction
struct TwoViewCursor {
    int index;
    int line;
    int col;
};

struct IndexPair {
    int openIndex;
    int closeIndex;
};

QList<KTextEditor::Range>
KateRegExpSearch::search(const QString &pattern, KTextEditor::Range inputRange, bool backwards, QRegularExpression::PatternOptions options)
{
    // Save regexes to avoid reconstructing regexes all the time
    static QRegularExpression preRegex;
    static QRegularExpression repairedRegex;

    // Returned if no matches are found
    QList<KTextEditor::Range> noResult(1, KTextEditor::Range::invalid());

    // Note that some methods in vimode (e.g. Searcher::findPatternWorker) rely on the
    // this method returning here if 'pattern' is empty.
    if (pattern.isEmpty() || inputRange.isEmpty() || !inputRange.isValid()) {
        return noResult;
    }

    // Always enable Unicode support
    options |= QRegularExpression::UseUnicodePropertiesOption;

    if (preRegex.pattern() != pattern || preRegex.patternOptions() != options) {
        preRegex = QRegularExpression(pattern, options);
    }

    // If repairPattern() is called on an invalid regex pattern it may cause asserts
    // in QString (e.g. if the pattern is just '\\', pattern.size() is 1, and repaierPattern
    // expects at least one character after a '\')
    if (!preRegex.isValid()) {
        return noResult;
    }

    // detect pattern type (single- or mutli-line)
    bool stillMultiLine;
    const QString repairedPattern = repairPattern(pattern, stillMultiLine);

    // Enable multiline mode, so that the ^ and $ metacharacters in the pattern
    // are allowed to match, respectively, immediately after and immediately
    // before any newline in the subject string, as well as at the very beginning
    // and at the very end of the subject string (see QRegularExpression docs).
    //
    // Whole lines are passed to QRegularExpression, so that e.g. if the inputRange
    // ends in the middle of a line, then a '$' won't match at that position. And
    // matches that are out of the inputRange are rejected.
    if (stillMultiLine) {
        options |= QRegularExpression::MultilineOption;
    }

    // check if anything changed at all
    if (repairedRegex.pattern() != repairedPattern || repairedRegex.patternOptions() != options) {
        repairedRegex.setPattern(repairedPattern);
        repairedRegex.setPatternOptions(options);
    }
    if (!repairedRegex.isValid()) {
        return noResult;
    }

    const int rangeStartLine = inputRange.start().line();
    const int rangeStartCol = inputRange.start().column();

    const int rangeEndLine = inputRange.end().line();
    const int rangeEndCol = inputRange.end().column();

    if (stillMultiLine) {
        const int rangeLineCount = rangeEndLine - rangeStartLine + 1;
        FAST_DEBUG("regular expression search (lines " << rangeStartLine << ".." << rangeEndLine << ")");

        const int docLineCount = m_document->lines();
        // nothing to do...
        if (rangeStartLine >= docLineCount) {
            return noResult;
        }

        QList<int> lineLens(rangeLineCount);
        int maxMatchOffset = 0;

        // all lines in the input range
        QString wholeRange;
        for (int i = 0; i < rangeLineCount; ++i) {
            const int docLineIndex = rangeStartLine + i;
            if (docLineIndex < 0 || docLineCount <= docLineIndex) { // invalid index
                return noResult;
            }

            const QString textLine = m_document->line(docLineIndex);
            lineLens[i] = textLine.length();
            wholeRange.append(textLine);

            // This check is needed as some parts in vimode rely on this behaviour.
            // We add an '\n' as a delimiter between lines in the range; but never after the
            // last line as that would add an '\n' that isn't there in the original text,
            // and can skew search results or hit an assert when accessing lineLens later
            // in the code.
            if (i != (rangeLineCount - 1)) {
                wholeRange.append(QLatin1Char('\n'));
            }

            // lineLens.at(i) + 1, because '\n' was added
            maxMatchOffset += (i == rangeEndLine) ? rangeEndCol : lineLens.at(i) + 1;

            FAST_DEBUG("  line" << i << "has length" << lineLens.at(i));
        }

        FAST_DEBUG("Max. match offset" << maxMatchOffset);

        QRegularExpressionMatch match;
        bool found = false;
        QRegularExpressionMatchIterator iter = repairedRegex.globalMatch(wholeRange, rangeStartCol);

        if (backwards) {
            while (iter.hasNext()) {
                QRegularExpressionMatch curMatch = iter.next();
                if (curMatch.capturedEnd() <= maxMatchOffset) {
                    match.swap(curMatch);
                    found = true;
                }
            }
        } else { /* forwards */
            QRegularExpressionMatch curMatch;
            if (iter.hasNext()) {
                curMatch = iter.next();
            }
            if (curMatch.capturedEnd() <= maxMatchOffset) {
                match.swap(curMatch);
                found = true;
            }
        }

        if (!found) {
            // no match
            FAST_DEBUG("not found");
            return noResult;
        }

        // Capture groups: save opening and closing indices and build a map,
        // the correct values will be written into it later
        QMap<int, TwoViewCursor *> indicesToCursors;
        const int numCaptures = repairedRegex.captureCount();
        QList<IndexPair> indexPairs(numCaptures + 1);
        for (int c = 0; c <= numCaptures; ++c) {
            const int openIndex = match.capturedStart(c);
            IndexPair &pair = indexPairs[c];
            if (openIndex == -1) {
                // An invalid index indicates an empty capture group
                pair.openIndex = -1;
                pair.closeIndex = -1;
                FAST_DEBUG("capture []");
            } else {
                const int closeIndex = match.capturedEnd(c);
                pair.openIndex = openIndex;
                pair.closeIndex = closeIndex;
                FAST_DEBUG("capture [" << pair.openIndex << ".." << pair.closeIndex << "]");

                // each key no more than once
                if (!indicesToCursors.contains(openIndex)) {
                    TwoViewCursor *twoViewCursor = new TwoViewCursor;
                    twoViewCursor->index = openIndex;
                    indicesToCursors.insert(openIndex, twoViewCursor);
                    FAST_DEBUG("  capture group start index added: " << openIndex);
                }
                if (!indicesToCursors.contains(closeIndex)) {
                    TwoViewCursor *twoViewCursor = new TwoViewCursor;
                    twoViewCursor->index = closeIndex;
                    indicesToCursors.insert(closeIndex, twoViewCursor);
                    FAST_DEBUG("  capture group end index added: " << closeIndex);
                }
            }
        }

        // find out where they belong
        int curRelLine = 0;
        int curRelCol = 0;
        int curRelIndex = 0;

        for (TwoViewCursor *twoViewCursor : std::as_const(indicesToCursors)) {
            // forward to index, save line/col
            const int index = twoViewCursor->index;
            FAST_DEBUG("resolving position" << index);

            while (curRelIndex <= index) {
                FAST_DEBUG("walk pos (" << curRelLine << "," << curRelCol << ") = " << curRelIndex << "relative, steps more to go" << index - curRelIndex);

                const int curRelLineLen = lineLens.at(curRelLine);
                const int curLineRemainder = curRelLineLen - curRelCol;
                const int lineFeedIndex = curRelIndex + curLineRemainder;
                if (index <= lineFeedIndex) {
                    if (index == lineFeedIndex) {
                        // on this line _at_ line feed
                        FAST_DEBUG("  on line feed");
                        const int absLine = curRelLine + rangeStartLine;
                        twoViewCursor->line = absLine;
                        twoViewCursor->col = curRelLineLen;

                        // advance to next line
                        const int advance = (index - curRelIndex) + 1;
                        ++curRelLine;
                        curRelCol = 0;
                        curRelIndex += advance;
                    } else { // index < lineFeedIndex
                        // on this line _before_ line feed
                        FAST_DEBUG("  before line feed");
                        const int diff = (index - curRelIndex);
                        const int absLine = curRelLine + rangeStartLine;
                        const int absCol = curRelCol + diff;
                        twoViewCursor->line = absLine;
                        twoViewCursor->col = absCol;

                        // advance on same line
                        const int advance = diff + 1;
                        curRelCol += advance;
                        curRelIndex += advance;
                    }
                    FAST_DEBUG("position(" << twoViewCursor->line << "," << twoViewCursor->col << ")");
                } else { // if (index > lineFeedIndex)
                    // not on this line
                    // advance to next line
                    FAST_DEBUG("  not on this line");
                    ++curRelLine;
                    curRelCol = 0;
                    const int advance = curLineRemainder + 1;
                    curRelIndex += advance;
                }
            }
        }

        // build result array
        QList<KTextEditor::Range> result(numCaptures + 1, KTextEditor::Range::invalid());
        for (int y = 0; y <= numCaptures; y++) {
            IndexPair &pair = indexPairs[y];
            if (!(pair.openIndex == -1 || pair.closeIndex == -1)) {
                const TwoViewCursor *const openCursors = indicesToCursors.value(pair.openIndex);
                const TwoViewCursor *const closeCursors = indicesToCursors.value(pair.closeIndex);
                const int startLine = openCursors->line;
                const int startCol = openCursors->col;
                const int endLine = closeCursors->line;
                const int endCol = closeCursors->col;
                FAST_DEBUG("range " << y << ": (" << startLine << ", " << startCol << ")..(" << endLine << ", " << endCol << ")");
                result[y] = KTextEditor::Range(startLine, startCol, endLine, endCol);
            }
        }

        // free structs allocated for indicesToCursors
        qDeleteAll(indicesToCursors);

        return result;
    } else {
        // single-line regex search (forwards and backwards)
        const int rangeStartCol = inputRange.start().column();
        const uint rangeEndCol = inputRange.end().column();

        const int rangeStartLine = inputRange.start().line();
        const int rangeEndLine = inputRange.end().line();

        const int forInit = backwards ? rangeEndLine : rangeStartLine;

        const int forInc = backwards ? -1 : +1;

        FAST_DEBUG("single line " << (backwards ? rangeEndLine : rangeStartLine) << ".." << (backwards ? rangeStartLine : rangeEndLine));

        for (int j = forInit; (rangeStartLine <= j) && (j <= rangeEndLine); j += forInc) {
            if (j < 0 || m_document->lines() <= j) {
                FAST_DEBUG("searchText | line " << j << ": no");
                return noResult;
            }

            const QString textLine = m_document->line(j);

            const int offset = (j == rangeStartLine) ? rangeStartCol : 0;
            const int endLineMaxOffset = (j == rangeEndLine) ? rangeEndCol : textLine.length();

            bool found = false;

            QRegularExpressionMatch match;

            if (backwards) {
                QRegularExpressionMatchIterator iter = repairedRegex.globalMatch(textLine, offset);
                while (iter.hasNext()) {
                    QRegularExpressionMatch curMatch = iter.next();
                    if (curMatch.capturedEnd() <= endLineMaxOffset) {
                        match.swap(curMatch);
                        found = true;
                    }
                }
            } else {
                match = repairedRegex.match(textLine, offset);
                if (match.hasMatch() && match.capturedEnd() <= endLineMaxOffset) {
                    found = true;
                }
            }

            if (found) {
                FAST_DEBUG("line " << j << ": yes");

                // build result array
                const int numCaptures = repairedRegex.captureCount();
                QList<KTextEditor::Range> result(numCaptures + 1);
                result[0] = KTextEditor::Range(j, match.capturedStart(), j, match.capturedEnd());

                FAST_DEBUG("result range " << 0 << ": (" << j << ", " << match.capturedStart() << ")..(" << j << ", " << match.capturedEnd() << ")");

                for (int y = 1; y <= numCaptures; ++y) {
                    const int openIndex = match.capturedStart(y);

                    if (openIndex == -1) {
                        result[y] = KTextEditor::Range::invalid();

                        FAST_DEBUG("capture []");
                    } else {
                        const int closeIndex = match.capturedEnd(y);

                        FAST_DEBUG("result range " << y << ": (" << j << ", " << openIndex << ")..(" << j << ", " << closeIndex << ")");

                        result[y] = KTextEditor::Range(j, openIndex, j, closeIndex);
                    }
                }
                return result;
            } else {
                FAST_DEBUG("searchText | line " << j << ": no");
            }
        }
    }
    return noResult;
}

/*static*/ QString KateRegExpSearch::escapePlaintext(const QString &text)
{
    return buildReplacement(text, QStringList(), 0, false);
}

/*static*/ QString KateRegExpSearch::buildReplacement(const QString &text, const QStringList &capturedTexts, int replacementCounter)
{
    return buildReplacement(text, capturedTexts, replacementCounter, true);
}

/*static*/ QString KateRegExpSearch::buildReplacement(const QString &text, const QStringList &capturedTexts, int replacementCounter, bool replacementGoodies)
{
    // get input
    const int inputLen = text.length();
    int input = 0; // walker index

    // prepare output
    ReplacementStream out(capturedTexts);

    while (input < inputLen) {
        switch (text[input].unicode()) {
        case L'\n':
            out << text[input];
            input++;
            break;

        case L'\\':
            if (input + 1 >= inputLen) {
                // copy backslash
                out << text[input];
                input++;
                break;
            }

            switch (text[input + 1].unicode()) {
            case L'0': // "\0000".."\0377"
                if (input + 4 >= inputLen) {
                    out << ReplacementStream::cap(0);
                    input += 2;
                } else {
                    bool stripAndSkip = false;
                    const ushort text_2 = text[input + 2].unicode();
                    if ((text_2 >= L'0') && (text_2 <= L'3')) {
                        const ushort text_3 = text[input + 3].unicode();
                        if ((text_3 >= L'0') && (text_3 <= L'7')) {
                            const ushort text_4 = text[input + 4].unicode();
                            if ((text_4 >= L'0') && (text_4 <= L'7')) {
                                int digits[3];
                                for (int i = 0; i < 3; i++) {
                                    digits[i] = 7 - (L'7' - text[input + 2 + i].unicode());
                                }
                                const int ch = 64 * digits[0] + 8 * digits[1] + digits[2];
                                out << QChar(ch);
                                input += 5;
                            } else {
                                stripAndSkip = true;
                            }
                        } else {
                            stripAndSkip = true;
                        }
                    } else {
                        stripAndSkip = true;
                    }

                    if (stripAndSkip) {
                        out << ReplacementStream::cap(0);
                        input += 2;
                    }
                }
                break;

            // single letter captures \x
            case L'1':
            case L'2':
            case L'3':
            case L'4':
            case L'5':
            case L'6':
            case L'7':
            case L'8':
            case L'9':
                out << ReplacementStream::cap(9 - (L'9' - text[input + 1].unicode()));
                input += 2;
                break;

            // multi letter captures \{xxxx}
            case L'{': {
                // allow {1212124}.... captures, see bug 365124 + testReplaceManyCapturesBug365124
                int capture = 0;
                int captureSize = 2;
                while ((input + captureSize) < inputLen) {
                    const ushort nextDigit = text[input + captureSize].unicode();
                    if ((nextDigit >= L'0') && (nextDigit <= L'9')) {
                        capture = (10 * capture) + (9 - (L'9' - nextDigit));
                        ++captureSize;
                        continue;
                    }
                    if (nextDigit == L'}') {
                        ++captureSize;
                        break;
                    }
                    break;
                }
                out << ReplacementStream::cap(capture);
                input += captureSize;
                break;
            }

            case L'E': // FALLTHROUGH
            case L'L': // FALLTHROUGH
            case L'l': // FALLTHROUGH
            case L'U': // FALLTHROUGH
            case L'u':
                if (!replacementGoodies) {
                    // strip backslash ("\?" -> "?")
                    out << text[input + 1];
                } else {
                    // handle case switcher
                    switch (text[input + 1].unicode()) {
                    case L'L':
                        out << ReplacementStream::lowerCase;
                        break;

                    case L'l':
                        out << ReplacementStream::lowerCaseFirst;
                        break;

                    case L'U':
                        out << ReplacementStream::upperCase;
                        break;

                    case L'u':
                        out << ReplacementStream::upperCaseFirst;
                        break;

                    case L'E': // FALLTHROUGH
                    default:
                        out << ReplacementStream::keepCase;
                    }
                }
                input += 2;
                break;

            case L'#':
                if (!replacementGoodies) {
                    // strip backslash ("\?" -> "?")
                    out << text[input + 1];
                    input += 2;
                } else {
                    // handle replacement counter
                    // eat and count all following hash marks
                    // each hash stands for a leading zero: \### will produces 001, 002, ...
                    int minWidth = 1;
                    while ((input + minWidth + 1 < inputLen) && (text[input + minWidth + 1].unicode() == L'#')) {
                        minWidth++;
                    }
                    out << ReplacementStream::counter(replacementCounter, minWidth);
                    input += 1 + minWidth;
                }
                break;

            case L'a':
                out << QChar(0x07);
                input += 2;
                break;

            case L'f':
                out << QChar(0x0c);
                input += 2;
                break;

            case L'n':
                out << QChar(0x0a);
                input += 2;
                break;

            case L'r':
                out << QChar(0x0d);
                input += 2;
                break;

            case L't':
                out << QChar(0x09);
                input += 2;
                break;

            case L'v':
                out << QChar(0x0b);
                input += 2;
                break;

            case L'x': // "\x0000".."\xffff"
                if (input + 5 >= inputLen) {
                    // strip backslash ("\x" -> "x")
                    out << text[input + 1];
                    input += 2;
                } else {
                    bool stripAndSkip = false;
                    const ushort text_2 = text[input + 2].unicode();
                    if (((text_2 >= L'0') && (text_2 <= L'9')) || ((text_2 >= L'a') && (text_2 <= L'f')) || ((text_2 >= L'A') && (text_2 <= L'F'))) {
                        const ushort text_3 = text[input + 3].unicode();
                        if (((text_3 >= L'0') && (text_3 <= L'9')) || ((text_3 >= L'a') && (text_3 <= L'f')) || ((text_3 >= L'A') && (text_3 <= L'F'))) {
                            const ushort text_4 = text[input + 4].unicode();
                            if (((text_4 >= L'0') && (text_4 <= L'9')) || ((text_4 >= L'a') && (text_4 <= L'f')) || ((text_4 >= L'A') && (text_4 <= L'F'))) {
                                const ushort text_5 = text[input + 5].unicode();
                                if (((text_5 >= L'0') && (text_5 <= L'9')) || ((text_5 >= L'a') && (text_5 <= L'f'))
                                    || ((text_5 >= L'A') && (text_5 <= L'F'))) {
                                    int digits[4];
                                    for (int i = 0; i < 4; i++) {
                                        const ushort cur = text[input + 2 + i].unicode();
                                        if ((cur >= L'0') && (cur <= L'9')) {
                                            digits[i] = 9 - (L'9' - cur);
                                        } else if ((cur >= L'a') && (cur <= L'f')) {
                                            digits[i] = 15 - (L'f' - cur);
                                        } else { // if ((cur >= L'A') && (cur <= L'F')))
                                            digits[i] = 15 - (L'F' - cur);
                                        }
                                    }

                                    const int ch = 4096 * digits[0] + 256 * digits[1] + 16 * digits[2] + digits[3];
                                    out << QChar(ch);
                                    input += 6;
                                } else {
                                    stripAndSkip = true;
                                }
                            } else {
                                stripAndSkip = true;
                            }
                        } else {
                            stripAndSkip = true;
                        }
                    }

                    if (stripAndSkip) {
                        // strip backslash ("\x" -> "x")
                        out << text[input + 1];
                        input += 2;
                    }
                }
                break;

            default:
                // strip backslash ("\?" -> "?")
                out << text[input + 1];
                input += 2;
            }
            break;

        default:
            out << text[input];
            input++;
        }
    }

    return out.str();
}

QString KateRegExpSearch::repairPattern(const QString &pattern, bool &stillMultiLine)
{
    // '\s' can make a pattern multi-line, it's replaced here with '[ \t]';
    // besides \s, the following characters can make a pattern multi-line:
    // \n, \x000A (Line Feed), \x????-\x????, \0012, \0???-\0???
    // a multi-line pattern must not pass as single-line, the other
    // way around will just result in slower searches and is therefore
    // not as critical

    const int inputLen = pattern.length();
    const QStringView patternView{pattern};

    // prepare output
    QString output;
    output.reserve(2 * inputLen + 1); // twice should be enough for the average case

    // parser state
    bool insideClass = false;

    stillMultiLine = false;
    int input = 0;
    while (input < inputLen) {
        if (insideClass) {
            // wait for closing, unescaped ']'
            switch (pattern[input].unicode()) {
            case L'\\':
                switch (pattern[input + 1].unicode()) {
                case L'x':
                    if (input + 5 < inputLen) {
                        // copy "\x????" unmodified
                        output.append(patternView.mid(input, 6));
                        input += 6;
                    } else {
                        // copy "\x" unmodified
                        output.append(patternView.mid(input, 2));
                        input += 2;
                    }
                    stillMultiLine = true;
                    break;

                case L'0':
                    if (input + 4 < inputLen) {
                        // copy "\0???" unmodified
                        output.append(patternView.mid(input, 5));
                        input += 5;
                    } else {
                        // copy "\0" unmodified
                        output.append(patternView.mid(input, 2));
                        input += 2;
                    }
                    stillMultiLine = true;
                    break;

                case L's':
                    // replace "\s" with "[ \t]"
                    output.append(QLatin1String(" \\t"));
                    input += 2;
                    break;

                case L'n':
                    stillMultiLine = true;
                    // FALLTROUGH
                    Q_FALLTHROUGH();

                default:
                    // copy "\?" unmodified
                    output.append(patternView.mid(input, 2));
                    input += 2;
                }
                break;

            case L']':
                // copy "]" unmodified
                insideClass = false;
                output.append(pattern[input]);
                ++input;
                break;

            default:
                // copy "?" unmodified
                output.append(pattern[input]);
                ++input;
            }
        } else {
            switch (pattern[input].unicode()) {
            case L'\\':
                switch (pattern[input + 1].unicode()) {
                case L'x':
                    if (input + 5 < inputLen) {
                        // copy "\x????" unmodified
                        output.append(patternView.mid(input, 6));
                        input += 6;
                    } else {
                        // copy "\x" unmodified
                        output.append(patternView.mid(input, 2));
                        input += 2;
                    }
                    stillMultiLine = true;
                    break;

                case L'0':
                    if (input + 4 < inputLen) {
                        // copy "\0???" unmodified
                        output.append(patternView.mid(input, 5));
                        input += 5;
                    } else {
                        // copy "\0" unmodified
                        output.append(patternView.mid(input, 2));
                        input += 2;
                    }
                    stillMultiLine = true;
                    break;

                case L's':
                    // replace "\s" with "[ \t]"
                    output.append(QLatin1String("[ \\t]"));
                    input += 2;
                    break;

                case L'n':
                    stillMultiLine = true;
                    // FALLTROUGH
                    Q_FALLTHROUGH();
                default:
                    // copy "\?" unmodified
                    output.append(patternView.mid(input, 2));
                    input += 2;
                }
                break;

            case L'[':
                // copy "[" unmodified
                insideClass = true;
                output.append(pattern[input]);
                ++input;
                break;

            default:
                // copy "?" unmodified
                output.append(pattern[input]);
                ++input;
            }
        }
    }
    return output;
}

// Kill our helpers again
#ifdef FAST_DEBUG_ENABLE
#undef FAST_DEBUG_ENABLE
#endif
#undef FAST_DEBUG
