/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "krichtextedittest.h"

#include <KColorScheme>
#include <krichtextedit.h>

#include <QFont>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTest>
#include <QTextCursor>
#include <QTextList>

QTEST_MAIN(KRichTextEditTest)

void KRichTextEditTest::testLinebreaks()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Enter the text with keypresses, for some strange reason a normal setText() or
    // setPlainText() call doesn't do the trick
    QTest::keyClicks(&edit, QStringLiteral("a\r\r"));
    edit.setTextUnderline(true);
    QTest::keyClicks(&edit, QStringLiteral("b\r\r\rc"));
    QCOMPARE(edit.toPlainText(), QStringLiteral("a\n\nb\n\n\nc"));

    QString html = edit.toCleanHtml();
    edit.clear();
    edit.setHtml(html);
    QCOMPARE(edit.toPlainText(), QStringLiteral("a\n\nb\n\n\nc"));
}

void KRichTextEditTest::testUpdateLinkAdd()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Add text, apply initial formatting, and add a link
    QTextCursor cursor = edit.textCursor();
    cursor.insertText(QStringLiteral("Test"));
    QTextCharFormat charFormat = cursor.charFormat();
    // Note that QTextEdit doesn't use the palette. Black is black.
    QCOMPARE(charFormat.foreground().color().name(), QColor(Qt::black).name());

    cursor.select(QTextCursor::BlockUnderCursor);
    edit.setTextCursor(cursor);
    edit.setTextBold(true);
    edit.setTextItalic(true);
    edit.updateLink(QStringLiteral("http://www.kde.org"), QStringLiteral("KDE"));

    // Validate text and formatting
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.toPlainText(), QStringLiteral("KDE "));
    QCOMPARE(edit.fontItalic(), true);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    QCOMPARE(edit.fontUnderline(), true);
    charFormat = cursor.charFormat();
    QCOMPARE(charFormat.foreground(), QBrush(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color()));
    QCOMPARE(charFormat.underlineColor(), KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
    QCOMPARE(charFormat.underlineStyle(), QTextCharFormat::SingleUnderline);
}

void KRichTextEditTest::testUpdateLinkRemove()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Add text, apply initial formatting, and add a link
    QTextCursor cursor = edit.textCursor();
    cursor.insertText(QStringLiteral("Test"));
    cursor.select(QTextCursor::BlockUnderCursor);
    edit.setTextCursor(cursor);
    edit.setTextBold(true);
    edit.setTextItalic(true);
    edit.updateLink(QStringLiteral("http://www.kde.org"), QStringLiteral("KDE"));

    // Remove link and validate formatting
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    edit.updateLink(QString(), QStringLiteral("KDE"));
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::WordUnderCursor);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.toPlainText(), QStringLiteral("KDE "));
    QCOMPARE(edit.fontItalic(), true);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    QCOMPARE(edit.fontUnderline(), false);
    QTextCharFormat charFormat = cursor.charFormat();
    QCOMPARE(charFormat.foreground().color().name(), QColor(Qt::black).name());
    QCOMPARE(charFormat.underlineColor().name(), QColor(Qt::black).name());
    QCOMPARE(charFormat.underlineStyle(), QTextCharFormat::NoUnderline);
}

void KRichTextEditTest::testHTMLLineBreaks()
{
    KRichTextEdit edit;
    edit.enableRichTextMode();

    // Create the following text:
    // A
    //
    // B
    QTest::keyClicks(&edit, QStringLiteral("a\r"));

    edit.setTextUnderline(true);

    QTest::keyClicks(&edit, QStringLiteral("\rb"));

    QString html = edit.toCleanHtml();

    // The problem we have is that we need to "fake" being a viewer such
    // as Thunderbird or MS-Outlook to unit test our html line breaks.
    // For now, we'll parse the 6th line (the empty one) and make sure it has the proper format
    // The first four (4) HTML code lines are DOCTYPE through <body> declaration

    const QStringList lines = html.split(QLatin1Char('\n'));

    //  for (int idx=0; idx<lines.size(); idx++) {
    //    qDebug() << ( idx + 1 ) << " : " << lines.at( idx );
    //  }

    QCOMPARE(lines.size(), 10);

    const QString &line6 = lines.at(lines.size() - 2);

    // make sure that this is an empty <p> line
    QVERIFY(line6.startsWith(QStringLiteral("<p style=\"-qt-paragraph-type:empty;")));

    // make sure that empty lines have the &nbsp; inserted
    QVERIFY2(line6.endsWith(QStringLiteral(">&nbsp;</p>")),
             "Empty lines must have &nbsp; or otherwise 3rd party "
             "viewers render those as non-existing lines");
}

void KRichTextEditTest::testHTMLOrderedLists()
{
    // The problem we have is that we need to "fake" being a viewer such
    // as Thunderbird or MS-Outlook to unit test our html lists.
    // For now, we'll parse the 6th line (the <ol> element) and make sure it has the proper format

    KRichTextEdit edit;
    edit.enableRichTextMode();

    edit.setTextUnderline(true);

    // create a numbered (ordered) list
    QTextCursor cursor = edit.textCursor();
    cursor.insertList(QTextListFormat::ListDecimal);

    QTest::keyClicks(&edit, QStringLiteral("a\rb\rc\r"));

    QString html = edit.toCleanHtml();

    const QStringList lines = html.split(QLatin1Char('\n'));

    //  Uncomment this section in case the first test fails to see if the HTML
    //  rendering has actually introduced a bug, or merely a problem with the unit test itself
    //
    //  for (int idx=0; idx<lines.size(); idx++) {
    //    qDebug() << ( idx + 1 ) << " : " << lines.at( idx );
    //  }

    QCOMPARE(lines.size(), 13);

    // this is the <ol> declaration line
    const QString &line6 = lines.at(lines.size() - 4);

    //  qDebug() << line6;

    const QRegularExpression re(QStringLiteral("<ol.*?margin-left: 0px.*?><li"));
    QVERIFY2(!re.match(line6, 0).hasMatch(),
             "margin-left: 0px specified for ordered lists "
             "removes numbers in 3rd party viewers ");
}

void KRichTextEditTest::testHTMLUnorderedLists()
{
    // The problem we have is that we need to "fake" being a viewer such
    // as Thunderbird or MS-Outlook to unit test our html lists.
    // For now, we'll parse the 6th line (the <ul> element) and make sure it has the proper format
    // The first four (4) HTML code lines are DOCTYPE through <body> declaration

    KRichTextEdit edit;
    edit.enableRichTextMode();

    edit.setTextUnderline(true);

    // create a numbered (ordered) list
    QTextCursor cursor = edit.textCursor();
    cursor.insertList(QTextListFormat::ListDisc);

    QTest::keyClicks(&edit, QStringLiteral("a\rb\rc\r"));

    QString html = edit.toCleanHtml();

    const QStringList lines = html.split(QLatin1Char('\n'));

    //  Uncomment this section in case the first test fails to see if the HTML
    //  rendering has actually introduced a bug, or merely a problem with the unit test itself
    //
    //  for (int idx=0; idx<lines.size(); idx++) {
    //    qDebug() << ( idx + 1 ) << " : " << lines.at( idx );
    //  }

    QCOMPARE(lines.size(), 13);

    // this is the <ol> declaration line
    const QString &line6 = lines.at(lines.size() - 4);

    //  qDebug() << line6;

    const QRegularExpression re(QStringLiteral("<ul.*?margin-left: 0px.*?><li"));
    QVERIFY2(!re.match(line6, 0).hasMatch(),
             "margin-left: 0px specified for unordered lists "
             "removes numbers in 3rd party viewers ");
}

void KRichTextEditTest::testHeading()
{
    KRichTextEdit edit;
    // Create two lines, make second one a heading
    QTest::keyClicks(&edit, QStringLiteral("a\rb"));
    // Make sure heading actually changes
    edit.setHeadingLevel(1);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 1);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // Make sure it doesn't clutter undo stack (a single undo is sufficient)
    edit.undo();
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));

    // Set heading & keep writing, the text remains a heading
    edit.setHeadingLevel(2);
    QTest::keyClicks(&edit, QStringLiteral("cd"));
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));

    // Now add a new line, make sure it's no longer a heading
    QTest::keyClick(&edit, '\r');
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));

    // Make sure creating new line is also undoable
    edit.undo();
    QCOMPARE(edit.textCursor().position(), 5);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));

    // Add a new line and some more text, make sure it's still normal
    QTest::keyClicks(&edit, QStringLiteral("\ref"));
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));

    // Go to beginning of this line, press Backspace -> lines should be merged,
    // current line should become a heading
    QTest::keyClick(&edit, Qt::Key_Home);
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // Make sure this is also undoable
    edit.undo();
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));
    // Return it back
    QTest::keyClick(&edit, Qt::Key_Backspace);
    // The line is now "bcd|ef", "|" is cursor. Press Enter, the second line should remain a heading
    QTest::keyClick(&edit, Qt::Key_Return);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // Change the heading level back to normal
    edit.setHeadingLevel(0);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));
    // Go to end of previous line, press Delete -> lines should be merged again
    QTextCursor cursor = edit.textCursor();
    cursor.movePosition(QTextCursor::PreviousBlock);
    cursor.movePosition(QTextCursor::EndOfBlock);
    edit.setTextCursor(cursor);
    QTest::keyClick(&edit, Qt::Key_Delete);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 2);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // Make sure this is also undoable
    edit.undo();
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 0);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Normal));

    // Now playing with selection. The contents are currently:
    // ---
    // a
    // bcd
    // ef
    // gh
    // ---
    // Let's add a new line 'gh', select everything between "c" and "g"
    // and change heading. It should apply to both lines
    QTest::keyClicks(&edit, QStringLiteral("\rgh"));
    cursor.setPosition(4, QTextCursor::MoveAnchor);
    cursor.setPosition(10, QTextCursor::KeepAnchor);
    edit.setTextCursor(cursor);
    edit.setHeadingLevel(5);
    // In the end, both lines should change the heading (even before the selection, i.e. 'b'!)
    cursor.setPosition(3);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 5);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
    // (and after the selection, i.e. 'f'!)
    cursor.setPosition(11);
    edit.setTextCursor(cursor);
    QCOMPARE(edit.textCursor().blockFormat().headingLevel(), 5);
    QCOMPARE(edit.fontWeight(), static_cast<int>(QFont::Bold));
}

void KRichTextEditTest::testRulerScroll()
{
    // This is a test for bug 195828
    KRichTextEdit edit;
    // Add some lines, so that scroll definitely appears
    for (int i = 0; i < 100; i++) {
        QTest::keyClicks(&edit, QStringLiteral("New line\r"));
    }
    // Widget has to be shown for the scrollbar to be adjusted
    edit.show();
    // Ensure the scrollbar actually appears
    QVERIFY(edit.verticalScrollBar()->value() > 0);

    edit.insertHorizontalRule();
    // Make sure scrollbar didn't jump to the top
    QVERIFY(edit.verticalScrollBar()->value() > 0);
}

void KRichTextEditTest::testNestedLists()
{
    KRichTextEdit edit;
    // Simplest test: create a list with a single element
    QTest::keyClicks(&edit, QStringLiteral("el1"));
    edit.setListStyle(-static_cast<int>(QTextListFormat::ListSquare));
    QVERIFY(edit.textCursor().currentList());
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListSquare);
    // It should not be indentable, as there is nothing above
    QVERIFY(!edit.canIndentList());
    // But it should be dedentable
    QVERIFY(edit.canDedentList());
    // Press enter, a new element should be added
    QTest::keyClicks(&edit, QStringLiteral("\rel2"));
    QVERIFY(edit.textCursor().currentList());
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListSquare);
    // Change indentation
    edit.indentListMore();
    edit.setListStyle(-static_cast<int>(QTextListFormat::ListCircle));
    QCOMPARE(edit.textCursor().currentList()->format().indent(), 2);
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListCircle);
    // And another one; let's then change the style of "3" and see if "2" have also changed style
    QTest::keyClicks(&edit, QStringLiteral("\rel3"));
    edit.setListStyle(-static_cast<int>(QTextListFormat::ListDecimal));
    edit.moveCursor(QTextCursor::PreviousBlock);
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListDecimal);
    // Now add another element, and dedent it, so the list should look like following:
    // [] el1
    //    1. el2
    //    2. el3
    // [] el4
    edit.moveCursor(QTextCursor::End);
    QTest::keyClicks(&edit, QStringLiteral("\rel4"));
    edit.indentListLess();
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListSquare);
    // Let's change the style to disc and see if first element have also changed the style
    edit.setListStyle(-static_cast<int>(QTextListFormat::ListDisc));
    edit.moveCursor(QTextCursor::Start);
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListDisc);
    // Now let's play with selection. First we add couple subelements below, so the list is:
    // *  el1
    //    1. el2
    //    2. el3
    // *  el4
    //    o  el5
    //    o  el6
    edit.moveCursor(QTextCursor::End);
    QTest::keyClicks(&edit, QStringLiteral("\rel5"));
    edit.indentListMore();
    edit.setListStyle(-static_cast<int>(QTextListFormat::ListCircle));
    QTest::keyClicks(&edit, QStringLiteral("\rel6"));

    // Let's select (el3-el5) and indent them. It should become:
    // * el1
    //   1. el2
    //      1. el3
    //   2. el4
    //      o  el5
    //   3. el6
    QTextCursor cursor(edit.document());
    cursor.setPosition(9);
    cursor.setPosition(17, QTextCursor::KeepAnchor);
    edit.setTextCursor(cursor);
    edit.indentListMore();
    edit.moveCursor(QTextCursor::End);
    QCOMPARE(edit.textCursor().currentList()->count(), 3);
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListDecimal);
    // Now select el2-el5 and dedent them. It should become:
    // *  el1
    // *  el2
    //    1. el3
    // *  el4
    //    o  el5
    //    o  el6
    cursor.setPosition(6);
    cursor.setPosition(18, QTextCursor::KeepAnchor);
    edit.setTextCursor(cursor);
    edit.indentListLess();
    edit.moveCursor(QTextCursor::End);
    QCOMPARE(edit.textCursor().currentList()->count(), 2);
    QCOMPARE(edit.textCursor().currentList()->format().style(), QTextListFormat::ListCircle);
    // point at "el4"
    cursor.setPosition(13);
    QCOMPARE(cursor.currentList()->count(), 3);
    QCOMPARE(cursor.currentList()->format().style(), QTextListFormat::ListDisc);
    // Select el4 && el5, dedent it, so el4 becomes a simple text:
    // *  el1
    // *  el2
    //    1. el3
    // el4
    // o  el5
    //    o  el6
    cursor.setPosition(17, QTextCursor::KeepAnchor);
    edit.setTextCursor(cursor);
    edit.indentListLess();
    // point cursor at "el4"
    cursor.setPosition(13);
    QVERIFY(!cursor.currentList());
    // point at "el5", make sure it's a separate list now
    cursor.setPosition(16);
    QCOMPARE(cursor.currentList()->count(), 1);
    QCOMPARE(cursor.currentList()->format().style(), QTextListFormat::ListCircle);
    // Make sure the selection is not dedentable anymore
    QVERIFY(!edit.canDedentList());
}

#include "moc_krichtextedittest.cpp"
