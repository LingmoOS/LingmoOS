/*
    SPDX-FileCopyrightText: 2008-2009 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <QList>
#include <QObject>
#include <QPair>
#include <QString>

#include <ktexteditor/document.h>
#include <sonnet/backgroundchecker.h>
#include <sonnet/speller.h>

namespace KTextEditor
{
class DocumentPrivate;
}

class KateSpellCheckManager : public QObject
{
    Q_OBJECT

    typedef QPair<KTextEditor::Range, QString> RangeDictionaryPair;

public:
    explicit KateSpellCheckManager(QObject *parent = nullptr);
    ~KateSpellCheckManager() override;

    static QStringList suggestions(const QString &word, const QString &dictionary);

    void ignoreWord(const QString &word, const QString &dictionary);
    void addToDictionary(const QString &word, const QString &dictionary);

    /**
     * 'r2' is a subrange of 'r1', which is extracted from 'r1' and the remaining ranges are returned
     **/
    static QList<KTextEditor::Range> rangeDifference(KTextEditor::Range r1, KTextEditor::Range r2);

Q_SIGNALS:
    /**
     * These signals are used to propagate the dictionary changes to the
     * BackgroundChecker instance in other components (e.g. onTheFlyChecker).
     */
    void wordAddedToDictionary(const QString &word);
    void wordIgnored(const QString &word);

public:
    static QList<QPair<KTextEditor::Range, QString>> spellCheckLanguageRanges(KTextEditor::DocumentPrivate *doc, KTextEditor::Range range);

    QList<QPair<KTextEditor::Range, QString>> spellCheckWrtHighlightingRanges(KTextEditor::DocumentPrivate *doc,
                                                                              KTextEditor::Range range,
                                                                              const QString &dictionary = QString(),
                                                                              bool singleLine = false,
                                                                              bool returnSingleRange = false);
    QList<QPair<KTextEditor::Range, QString>> spellCheckRanges(KTextEditor::DocumentPrivate *doc, KTextEditor::Range range, bool singleLine = false);

    static void replaceCharactersEncodedIfNecessary(const QString &newWord, KTextEditor::DocumentPrivate *doc, KTextEditor::Range replacementRange);

private:
    static void trimRange(KTextEditor::DocumentPrivate *doc, KTextEditor::Range &r);
};

#endif
