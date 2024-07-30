/*
    SPDX-FileCopyrightText: 2003-2005 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Charles Samuels <charles@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katesedcmd.h"

#include "katedocument.h"
#include "kateglobal.h"
#include "katepartdebug.h"
#include "kateview.h"

#include <KLocalizedString>

#include <QDir>
#include <QRegularExpression>
#include <QUrl>

KateCommands::SedReplace *KateCommands::SedReplace::m_instance = nullptr;

static int backslashString(const QString &haystack, const QString &needle, int index)
{
    int len = haystack.length();
    int searchlen = needle.length();
    bool evenCount = true;
    while (index < len) {
        if (haystack[index] == QLatin1Char('\\')) {
            evenCount = !evenCount;
        } else {
            // isn't a slash
            if (!evenCount) {
                if (QStringView(haystack).mid(index, searchlen) == needle) {
                    return index - 1;
                }
            }
            evenCount = true;
        }
        ++index;
    }

    return -1;
}

// exchange "\t" for the actual tab character, for example
static void exchangeAbbrevs(QString &str)
{
    // the format is (findreplace)*[nullzero]
    const char *magic = "a\x07t\tn\n";

    while (*magic) {
        int index = 0;
        char replace = magic[1];
        while ((index = backslashString(str, QString(QChar::fromLatin1(*magic)), index)) != -1) {
            str.replace(index, 2, QChar::fromLatin1(replace));
            ++index;
        }
        ++magic;
        ++magic;
    }
}

bool KateCommands::SedReplace::exec(class KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &r)
{
    qCDebug(LOG_KTE) << "SedReplace::execCmd( " << cmd << " )";
    if (r.isValid()) {
        qCDebug(LOG_KTE) << "Range: " << r;
    }

    int findBeginPos = -1;
    int findEndPos = -1;
    int replaceBeginPos = -1;
    int replaceEndPos = -1;
    QString delimiter;
    if (!parse(cmd, delimiter, findBeginPos, findEndPos, replaceBeginPos, replaceEndPos)) {
        return false;
    }

    const QStringView searchParamsString = QStringView(cmd).mid(cmd.lastIndexOf(delimiter));
    const bool noCase = searchParamsString.contains(QLatin1Char('i'));
    const bool repeat = searchParamsString.contains(QLatin1Char('g'));
    const bool interactive = searchParamsString.contains(QLatin1Char('c'));

    QString find = cmd.mid(findBeginPos, findEndPos - findBeginPos + 1);
    qCDebug(LOG_KTE) << "SedReplace: find =" << find;

    QString replace = cmd.mid(replaceBeginPos, replaceEndPos - replaceBeginPos + 1);
    exchangeAbbrevs(replace);
    qCDebug(LOG_KTE) << "SedReplace: replace =" << replace;

    if (find.isEmpty()) {
        // Nothing to do.
        return true;
    }

    KTextEditor::ViewPrivate *kateView = static_cast<KTextEditor::ViewPrivate *>(view);
    KTextEditor::DocumentPrivate *doc = kateView->doc();
    if (!doc) {
        return false;
    }
    // Only current line ...
    int startLine = kateView->cursorPosition().line();
    int endLine = kateView->cursorPosition().line();
    // ... unless a range was provided.
    if (r.isValid()) {
        startLine = r.start().line();
        endLine = r.end().line();
    }

    std::shared_ptr<InteractiveSedReplacer> interactiveSedReplacer(new InteractiveSedReplacer(doc, find, replace, !noCase, !repeat, startLine, endLine));

    if (interactive) {
        const bool hasInitialMatch = interactiveSedReplacer->currentMatch().isValid();
        if (!hasInitialMatch) {
            // Can't start an interactive sed replace if there is no initial match!
            msg = interactiveSedReplacer->finalStatusReportMessage();
            return false;
        }
        interactiveSedReplace(kateView, interactiveSedReplacer);
        return true;
    }

    interactiveSedReplacer->replaceAllRemaining();
    msg = interactiveSedReplacer->finalStatusReportMessage();

    return true;
}

bool KateCommands::SedReplace::interactiveSedReplace(KTextEditor::ViewPrivate *, std::shared_ptr<InteractiveSedReplacer>)
{
    qCDebug(LOG_KTE) << "Interactive sedreplace is only currently supported with Vi mode plus Vi emulated command bar.";
    return false;
}

bool KateCommands::SedReplace::parse(const QString &sedReplaceString,
                                     QString &destDelim,
                                     int &destFindBeginPos,
                                     int &destFindEndPos,
                                     int &destReplaceBeginPos,
                                     int &destReplaceEndPos)
{
    // valid delimiters are all non-word, non-space characters plus '_'
    static const QRegularExpression delim(QStringLiteral("^s\\s*([^\\w\\s]|_)"), QRegularExpression::UseUnicodePropertiesOption);
    auto match = delim.match(sedReplaceString);
    if (!match.hasMatch()) {
        return false;
    }

    const QString d = match.captured(1);
    qCDebug(LOG_KTE) << "SedReplace: delimiter is '" << d << "'";

    QRegularExpression splitter(QStringLiteral("^s\\s*") + d + QLatin1String("((?:[^\\\\\\") + d + QLatin1String("]|\\\\.)*)\\") + d
                                    + QLatin1String("((?:[^\\\\\\") + d + QLatin1String("]|\\\\.)*)(\\") + d + QLatin1String("[igc]{0,3})?$"),
                                QRegularExpression::UseUnicodePropertiesOption);
    match = splitter.match(sedReplaceString);
    if (!match.hasMatch()) {
        return false;
    }

    const QString find = match.captured(1);
    const QString replace = match.captured(2);

    destDelim = d;
    destFindBeginPos = match.capturedStart(1);
    destFindEndPos = match.capturedStart(1) + find.length() - 1;
    destReplaceBeginPos = match.capturedStart(2);
    destReplaceEndPos = match.capturedStart(2) + replace.length() - 1;

    return true;
}

KateCommands::SedReplace::InteractiveSedReplacer::InteractiveSedReplacer(KTextEditor::DocumentPrivate *doc,
                                                                         const QString &findPattern,
                                                                         const QString &replacePattern,
                                                                         bool caseSensitive,
                                                                         bool onlyOnePerLine,
                                                                         int startLine,
                                                                         int endLine)
    : m_findPattern(findPattern)
    , m_replacePattern(replacePattern)
    , m_onlyOnePerLine(onlyOnePerLine)
    , m_endLine(endLine)
    , m_doc(doc)
    , m_regExpSearch(doc)
    , m_caseSensitive(caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)
    , m_numReplacementsDone(0)
    , m_numLinesTouched(0)
    , m_lastChangedLineNum(-1)
    , m_currentSearchPos(KTextEditor::Cursor(startLine, 0))
{
}

KTextEditor::Range KateCommands::SedReplace::InteractiveSedReplacer::currentMatch()
{
    QList<KTextEditor::Range> matches = fullCurrentMatch();

    if (matches.isEmpty()) {
        return KTextEditor::Range::invalid();
    }

    if (matches.first().start().line() > m_endLine) {
        return KTextEditor::Range::invalid();
    }

    return matches.first();
}

void KateCommands::SedReplace::InteractiveSedReplacer::skipCurrentMatch()
{
    const KTextEditor::Range currentMatch = this->currentMatch();
    m_currentSearchPos = currentMatch.end();
    if (m_onlyOnePerLine && currentMatch.start().line() == m_currentSearchPos.line()) {
        m_currentSearchPos = KTextEditor::Cursor(m_currentSearchPos.line() + 1, 0);
    }
}

void KateCommands::SedReplace::InteractiveSedReplacer::replaceCurrentMatch()
{
    const KTextEditor::Range currentMatch = this->currentMatch();
    const QString currentMatchText = m_doc->text(currentMatch);
    const QString replacementText = replacementTextForCurrentMatch();

    m_doc->editStart();
    m_doc->removeText(currentMatch);
    m_doc->insertText(currentMatch.start(), replacementText);
    m_doc->editEnd();

    // Begin next search from directly after replacement.
    if (!replacementText.contains(QLatin1Char('\n'))) {
        const int moveChar = currentMatch.isEmpty() ? 1 : 0; // if the search was for \s*, make sure we advance a char
        const int col = currentMatch.start().column() + replacementText.length() + moveChar;

        m_currentSearchPos = KTextEditor::Cursor(currentMatch.start().line(), col);
    } else {
        m_currentSearchPos = KTextEditor::Cursor(currentMatch.start().line() + replacementText.count(QLatin1Char('\n')),
                                                 replacementText.length() - replacementText.lastIndexOf(QLatin1Char('\n')) - 1);
    }
    if (m_onlyOnePerLine) {
        // Drop down to next line.
        m_currentSearchPos = KTextEditor::Cursor(m_currentSearchPos.line() + 1, 0);
    }

    // Adjust end line down by the number of new newlines just added, minus the number taken away.
    m_endLine += replacementText.count(QLatin1Char('\n'));
    m_endLine -= currentMatchText.count(QLatin1Char('\n'));

    m_numReplacementsDone++;
    if (m_lastChangedLineNum != currentMatch.start().line()) {
        // Counting "swallowed" lines as being "touched".
        m_numLinesTouched += currentMatchText.count(QLatin1Char('\n')) + 1;
    }
    m_lastChangedLineNum = m_currentSearchPos.line();
}

void KateCommands::SedReplace::InteractiveSedReplacer::replaceAllRemaining()
{
    m_doc->editStart();
    while (currentMatch().isValid()) {
        replaceCurrentMatch();
    }
    m_doc->editEnd();
}

QString KateCommands::SedReplace::InteractiveSedReplacer::currentMatchReplacementConfirmationMessage()
{
    return i18n("replace with %1?", replacementTextForCurrentMatch().replace(QLatin1Char('\n'), QLatin1String("\\n")));
}

QString KateCommands::SedReplace::InteractiveSedReplacer::finalStatusReportMessage() const
{
    return i18ncp("%2 is the translation of the next message",
                  "1 replacement done on %2",
                  "%1 replacements done on %2",
                  m_numReplacementsDone,
                  i18ncp("substituted into the previous message", "1 line", "%1 lines", m_numLinesTouched));
}

const QList<KTextEditor::Range> KateCommands::SedReplace::InteractiveSedReplacer::fullCurrentMatch()
{
    if (m_currentSearchPos > m_doc->documentEnd()) {
        return QList<KTextEditor::Range>();
    }

    QRegularExpression::PatternOptions options;
    if (m_caseSensitive == Qt::CaseInsensitive) {
        options |= (QRegularExpression::CaseInsensitiveOption);
    }
    return m_regExpSearch.search(m_findPattern, KTextEditor::Range(m_currentSearchPos, m_doc->documentEnd()), false /* search backwards */, options);
}

QString KateCommands::SedReplace::InteractiveSedReplacer::replacementTextForCurrentMatch()
{
    const QList<KTextEditor::Range> captureRanges = fullCurrentMatch();
    QStringList captureTexts;
    captureTexts.reserve(captureRanges.size());
    for (KTextEditor::Range captureRange : captureRanges) {
        captureTexts << m_doc->text(captureRange);
    }
    const QString replacementText = m_regExpSearch.buildReplacement(m_replacePattern, captureTexts, 0);
    return replacementText;
}
