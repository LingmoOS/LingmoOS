/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_DOCUMENT_TEST_H
#define KATE_DOCUMENT_TEST_H

#include <QTest>

#include <ktexteditor/movingrange.h>

class MovingRangeInvalidator : public QObject
{
    Q_OBJECT
public:
    explicit MovingRangeInvalidator(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void addRange(KTextEditor::MovingRange *range)
    {
        m_ranges << range;
    }
    QList<KTextEditor::MovingRange *> ranges() const
    {
        return m_ranges;
    }

public Q_SLOTS:
    void aboutToInvalidateMovingInterfaceContent()
    {
        qDeleteAll(m_ranges);
        m_ranges.clear();
    }

private:
    QList<KTextEditor::MovingRange *> m_ranges;
};

/**
 * Provides slots to check data sent in specific signals. Slot names are derived from corresponding test names.
 */
class SignalHandler : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void slotMultipleLinesRemoved(KTextEditor::Document *, const KTextEditor::Range &, const QString &oldText)
    {
        QCOMPARE(oldText, QStringLiteral("line2\nline3\n"));
    }

    void slotNewlineInserted(KTextEditor::Document *, const KTextEditor::Range &range)
    {
        QCOMPARE(range, KTextEditor::Range(KTextEditor::Cursor(1, 4), KTextEditor::Cursor(2, 0)));
    }
};

class KateDocumentTest : public QObject
{
    Q_OBJECT

public:
    KateDocumentTest();
    ~KateDocumentTest() override;

public Q_SLOTS:
    void initTestCase();

private Q_SLOTS:
    void testWordWrap();
    void testWrapParagraph();
    void testReplaceQStringList();
    void testMovingInterfaceSignals();
    void testSetTextPerformance();
    void testRemoveTextPerformance();
    void testForgivingApiUsage();
    void testRemoveMultipleLines();
    void testInsertNewline();
    void testInsertAfterEOF();
    void testAutoBrackets();
    void testReplaceTabs();
    void testDigest();
    void testModelines();
    void testDefStyleNum();
    void testTypeCharsWithSurrogateAndNewLine();
    void testRemoveComposedCharacters();
    void testAutoReload();
    void testSearch();
    void testMatchingBracket_data();
    void testMatchingBracket();
    void testIndentOnPaste();
    void testAboutToSave();
    void testKeepUndoOverReload();
    void testToggleComment();
    void testInsertTextTooLargeColumn();
    void testBug468495();
    void testCursorToOffset();
    void testBug329247();
    void testBugTextInsertedRange();
};

#endif // KATE_DOCUMENT_TEST_H
