/*
    SPDX-FileCopyrightText: 2009 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "spellcheck.h"

#include <QHash>
#include <QTimer>
#include <QtAlgorithms>

#include <KActionCollection>
#include <ktexteditor/view.h>

#include "katedocument.h"
#include "katehighlight.h"

KateSpellCheckManager::KateSpellCheckManager(QObject *parent)
    : QObject(parent)
{
}

KateSpellCheckManager::~KateSpellCheckManager() = default;

QStringList KateSpellCheckManager::suggestions(const QString &word, const QString &dictionary)
{
    Sonnet::Speller speller;
    speller.setLanguage(dictionary);
    return speller.suggest(word);
}

void KateSpellCheckManager::ignoreWord(const QString &word, const QString &dictionary)
{
    Sonnet::Speller speller;
    speller.setLanguage(dictionary);
    speller.addToSession(word);
    Q_EMIT wordIgnored(word);
}

void KateSpellCheckManager::addToDictionary(const QString &word, const QString &dictionary)
{
    Sonnet::Speller speller;
    speller.setLanguage(dictionary);
    speller.addToPersonal(word);
    Q_EMIT wordAddedToDictionary(word);
}

QList<KTextEditor::Range> KateSpellCheckManager::rangeDifference(KTextEditor::Range r1, KTextEditor::Range r2)
{
    Q_ASSERT(r1.contains(r2));
    QList<KTextEditor::Range> toReturn;
    KTextEditor::Range before(r1.start(), r2.start());
    KTextEditor::Range after(r2.end(), r1.end());
    if (!before.isEmpty()) {
        toReturn.push_back(before);
    }
    if (!after.isEmpty()) {
        toReturn.push_back(after);
    }
    return toReturn;
}

namespace
{
bool lessThanRangeDictionaryPair(const QPair<KTextEditor::Range, QString> &s1, const QPair<KTextEditor::Range, QString> &s2)
{
    return s1.first.end() <= s2.first.start();
}
}

QList<QPair<KTextEditor::Range, QString>> KateSpellCheckManager::spellCheckLanguageRanges(KTextEditor::DocumentPrivate *doc, KTextEditor::Range range)
{
    QString defaultDict = doc->defaultDictionary();
    QList<RangeDictionaryPair> toReturn;
    QList<QPair<KTextEditor::MovingRange *, QString>> dictionaryRanges = doc->dictionaryRanges();
    if (dictionaryRanges.isEmpty()) {
        toReturn.push_back(RangeDictionaryPair(range, defaultDict));
        return toReturn;
    }
    QList<KTextEditor::Range> splitQueue;
    splitQueue.push_back(range);
    while (!splitQueue.isEmpty()) {
        bool handled = false;
        KTextEditor::Range consideredRange = splitQueue.takeFirst();
        for (QList<QPair<KTextEditor::MovingRange *, QString>>::iterator i = dictionaryRanges.begin(); i != dictionaryRanges.end(); ++i) {
            KTextEditor::Range languageRange = *((*i).first);
            KTextEditor::Range intersection = languageRange.intersect(consideredRange);
            if (intersection.isEmpty()) {
                continue;
            }
            toReturn.push_back(RangeDictionaryPair(intersection, (*i).second));
            splitQueue += rangeDifference(consideredRange, intersection);
            handled = true;
            break;
        }
        if (!handled) {
            // 'consideredRange' did not intersect with any dictionary range, so we add it with the default dictionary
            toReturn.push_back(RangeDictionaryPair(consideredRange, defaultDict));
        }
    }
    // finally, we still have to sort the list
    std::stable_sort(toReturn.begin(), toReturn.end(), lessThanRangeDictionaryPair);
    return toReturn;
}

QList<QPair<KTextEditor::Range, QString>> KateSpellCheckManager::spellCheckWrtHighlightingRanges(KTextEditor::DocumentPrivate *document,
                                                                                                 KTextEditor::Range range,
                                                                                                 const QString &dictionary,
                                                                                                 bool singleLine,
                                                                                                 bool returnSingleRange)
{
    QList<QPair<KTextEditor::Range, QString>> toReturn;
    if (range.isEmpty()) {
        return toReturn;
    }

    KateHighlighting *highlighting = document->highlight();

    QList<KTextEditor::Range> rangesToSplit;
    if (!singleLine || range.onSingleLine()) {
        rangesToSplit.push_back(range);
    } else {
        const int startLine = range.start().line();
        const int startColumn = range.start().column();
        const int endLine = range.end().line();
        const int endColumn = range.end().column();
        for (int line = startLine; line <= endLine; ++line) {
            const int start = (line == startLine) ? startColumn : 0;
            const int end = (line == endLine) ? endColumn : document->lineLength(line);
            KTextEditor::Range toAdd(line, start, line, end);
            if (!toAdd.isEmpty()) {
                rangesToSplit.push_back(toAdd);
            }
        }
    }
    for (QList<KTextEditor::Range>::iterator i = rangesToSplit.begin(); i != rangesToSplit.end(); ++i) {
        KTextEditor::Range rangeToSplit = *i;
        KTextEditor::Cursor begin = KTextEditor::Cursor::invalid();
        const int startLine = rangeToSplit.start().line();
        const int startColumn = rangeToSplit.start().column();
        const int endLine = rangeToSplit.end().line();
        const int endColumn = rangeToSplit.end().column();
        bool inSpellCheckArea = false;
        for (int line = startLine; line <= endLine; ++line) {
            const auto kateTextLine = document->kateTextLine(line);
            const int start = (line == startLine) ? startColumn : 0;
            const int end = (line == endLine) ? endColumn : kateTextLine.length();
            for (int i = start; i < end;) { // WARNING: 'i' has to be incremented manually!
                int attr = kateTextLine.attribute(i);
                const KatePrefixStore &prefixStore = highlighting->getCharacterEncodingsPrefixStore(attr);
                QString prefixFound = prefixStore.findPrefix(kateTextLine, i);
                if (!document->highlight()->attributeRequiresSpellchecking(static_cast<unsigned int>(attr)) && prefixFound.isEmpty()) {
                    if (i == start) {
                        ++i;
                        continue;
                    } else if (inSpellCheckArea) {
                        KTextEditor::Range spellCheckRange(begin, KTextEditor::Cursor(line, i));
                        // work around Qt bug 6498
                        trimRange(document, spellCheckRange);
                        if (!spellCheckRange.isEmpty()) {
                            toReturn.push_back(RangeDictionaryPair(spellCheckRange, dictionary));
                            if (returnSingleRange) {
                                return toReturn;
                            }
                        }
                        begin = KTextEditor::Cursor::invalid();
                        inSpellCheckArea = false;
                    }
                } else if (!inSpellCheckArea) {
                    begin = KTextEditor::Cursor(line, i);
                    inSpellCheckArea = true;
                }
                if (!prefixFound.isEmpty()) {
                    i += prefixFound.length();
                } else {
                    ++i;
                }
            }
        }
        if (inSpellCheckArea) {
            KTextEditor::Range spellCheckRange(begin, rangeToSplit.end());
            // work around Qt bug 6498
            trimRange(document, spellCheckRange);
            if (!spellCheckRange.isEmpty()) {
                toReturn.push_back(RangeDictionaryPair(spellCheckRange, dictionary));
                if (returnSingleRange) {
                    return toReturn;
                }
            }
        }
    }

    return toReturn;
}

QList<QPair<KTextEditor::Range, QString>> KateSpellCheckManager::spellCheckRanges(KTextEditor::DocumentPrivate *doc, KTextEditor::Range range, bool singleLine)
{
    QList<RangeDictionaryPair> toReturn;
    QList<RangeDictionaryPair> languageRangeList = spellCheckLanguageRanges(doc, range);
    for (QList<RangeDictionaryPair>::iterator i = languageRangeList.begin(); i != languageRangeList.end(); ++i) {
        const RangeDictionaryPair &p = *i;
        toReturn += spellCheckWrtHighlightingRanges(doc, p.first, p.second, singleLine);
    }
    return toReturn;
}

void KateSpellCheckManager::replaceCharactersEncodedIfNecessary(const QString &newWord, KTextEditor::DocumentPrivate *doc, KTextEditor::Range replacementRange)
{
    const int attr = doc->kateTextLine(replacementRange.start().line()).attribute(replacementRange.start().column());
    if (!doc->highlight()->getCharacterEncodings(attr).isEmpty() && doc->containsCharacterEncoding(replacementRange)) {
        doc->replaceText(replacementRange, newWord);
        doc->replaceCharactersByEncoding(KTextEditor::Range(replacementRange.start(), replacementRange.start() + KTextEditor::Cursor(0, newWord.length())));
    } else {
        doc->replaceText(replacementRange, newWord);
    }
}

void KateSpellCheckManager::trimRange(KTextEditor::DocumentPrivate *doc, KTextEditor::Range &r)
{
    if (r.isEmpty()) {
        return;
    }
    KTextEditor::Cursor cursor = r.start();
    while (cursor < r.end()) {
        if (doc->lineLength(cursor.line()) > 0 && !doc->characterAt(cursor).isSpace() && doc->characterAt(cursor).category() != QChar::Other_Control) {
            break;
        }
        cursor.setColumn(cursor.column() + 1);
        if (cursor.column() >= doc->lineLength(cursor.line())) {
            cursor.setPosition(cursor.line() + 1, 0);
        }
    }
    r.setStart(cursor);
    if (r.isEmpty()) {
        return;
    }

    cursor = r.end();
    KTextEditor::Cursor prevCursor = cursor;
    // the range cannot be empty now
    do {
        prevCursor = cursor;
        if (cursor.column() <= 0) {
            cursor.setPosition(cursor.line() - 1, doc->lineLength(cursor.line() - 1));
        } else {
            cursor.setColumn(cursor.column() - 1);
        }
        if (cursor.column() < doc->lineLength(cursor.line()) && !doc->characterAt(cursor).isSpace()
            && doc->characterAt(cursor).category() != QChar::Other_Control) {
            break;
        }
    } while (cursor > r.start());
    r.setEnd(prevCursor);
}

#include "moc_spellcheck.cpp"
