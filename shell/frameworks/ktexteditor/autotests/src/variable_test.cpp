/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variable_test.h"
#include "moc_variable_test.cpp"

#include <katedocument.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include <QStandardPaths>
#include <QTest>
#include <QUuid>

using namespace KTextEditor;

QTEST_MAIN(VariableTest)

VariableTest::VariableTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

VariableTest::~VariableTest()
{
}

void VariableTest::testReturnValues()
{
    auto editor = KTextEditor::Editor::instance();

    const QString name = QStringLiteral("Document:");
    auto func = [](const QStringView &, KTextEditor::View *) {
        return QString();
    };

    // exact matches
    QVERIFY(!editor->unregisterVariable(name));
    QVERIFY(editor->registerVariableMatch(name, QStringLiteral("Document Text"), func));
    QVERIFY(!editor->registerVariableMatch(name, QStringLiteral("Document Text"), func));
    QVERIFY(editor->unregisterVariable(name));
    QVERIFY(!editor->unregisterVariable(name));

    // prefix matches
    QVERIFY(!editor->unregisterVariable(name));
    QVERIFY(editor->registerVariablePrefix(name, QStringLiteral("Document Text"), func));
    QVERIFY(!editor->registerVariablePrefix(name, QStringLiteral("Document Text"), func));
    QVERIFY(editor->unregisterVariable(name));
    QVERIFY(!editor->unregisterVariable(name));
}

void VariableTest::testExactMatch_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("expectedText");

    QTest::newRow("World") << QStringLiteral("World") << QStringLiteral("World") << QStringLiteral("World");
    QTest::newRow("Smart World") << QStringLiteral("Smart World") << QStringLiteral("Smart World") << QStringLiteral("Smart World");
}

void VariableTest::testExactMatch()
{
    QFETCH(QString, text);
    QFETCH(QString, expected);
    QFETCH(QString, expectedText);

    auto editor = KTextEditor::Editor::instance();
    auto doc = editor->createDocument(nullptr);
    auto view = doc->createView(nullptr);
    doc->setText(text);

    const QString name = QStringLiteral("Doc:Text");
    auto func = [](const QStringView &, KTextEditor::View *view) {
        return view->document()->text();
    };

    QVERIFY(editor->registerVariableMatch(name, QStringLiteral("Document Text"), func));

    // expandVariable
    QString output;
    QVERIFY(editor->expandVariable(QStringLiteral("Doc:Text"), view, output));
    QCOMPARE(output, expected);

    // expandText
    output = editor->expandText(QStringLiteral("Hello %{Doc:Text}!"), view);
    QCOMPARE(output, QStringLiteral("Hello ") + expectedText + QLatin1Char('!'));

    output = editor->expandText(QStringLiteral("Hello %{Doc:Text} %{Doc:Text}!"), view);
    QCOMPARE(output, QStringLiteral("Hello ") + expectedText + QLatin1Char(' ') + expectedText + QLatin1Char('!'));

    QVERIFY(editor->unregisterVariable(QStringLiteral("Doc:Text")));

    delete doc;
}

void VariableTest::testPrefixMatch()
{
    auto editor = KTextEditor::Editor::instance();

    const QString prefix = QStringLiteral("Mirror:");
    auto func = [](const QStringView &text, KTextEditor::View *) {
        QStringView rest = text.right(text.size() - 7);
        QString out;
        for (auto it = rest.rbegin(); it != rest.rend(); ++it) {
            out += *it;
        }
        return out;
    };

    QVERIFY(editor->registerVariablePrefix(prefix, QStringLiteral("Reverse text"), func));

    QString output;
    QVERIFY(editor->expandVariable(QStringLiteral("Mirror:12345"), nullptr, output));
    QCOMPARE(output, QStringLiteral("54321"));

    output = editor->expandText(QStringLiteral("Countdown: %{Mirror:12345}"), nullptr);
    QCOMPARE(output, QStringLiteral("Countdown: 54321"));

    // Test recursive expansion
    output = editor->expandText(QStringLiteral("Countup: %{Mirror:%{Mirror:12345}}"), nullptr);
    QCOMPARE(output, QStringLiteral("Countup: 12345"));

    QVERIFY(editor->unregisterVariable(prefix));
}

void VariableTest::testRecursiveMatch()
{
    auto editor = KTextEditor::Editor::instance();
    auto doc = editor->createDocument(nullptr);
    auto view = doc->createView(nullptr);
    doc->setText(QStringLiteral("Text"));

    const QString name = QStringLiteral("Doc:Text");
    auto func = [](const QStringView &, KTextEditor::View *view) {
        return view->document()->text();
    };
    QVERIFY(editor->registerVariableMatch(name, QStringLiteral("Document Text"), func));

    // Test recursive expansion
    doc->setText(QStringLiteral("Text"));
    QString output = editor->expandText(QStringLiteral("Hello %{Doc:%{Doc:Text}}!"), view);
    QCOMPARE(output, QStringLiteral("Hello Text!"));

    QVERIFY(editor->unregisterVariable(name));
    delete doc;
}

void VariableTest::testBuiltins()
{
    auto editor = KTextEditor::Editor::instance();
    auto doc = editor->createDocument(nullptr);
    doc->openUrl(QUrl::fromLocalFile(QDir::homePath() + QStringLiteral("/kate-v5.tar.gz")));
    doc->setText(QStringLiteral("get an edge in editing\n:-)"));
    auto view = doc->createView(nullptr);
    view->setCursorPosition(KTextEditor::Cursor(1, 2));
    view->show();

    QString out;

    // Test invalid ones:
    out = editor->expandText(QStringLiteral("%{}"), view);
    QCOMPARE(out, QStringLiteral("%{}"));
    out = editor->expandText(QStringLiteral("%{"), view);
    QCOMPARE(out, QStringLiteral("%{"));
    out = editor->expandText(QStringLiteral("%{{}"), view);
    QCOMPARE(out, QStringLiteral("%{{}"));
    out = editor->expandText(QStringLiteral("%{{}}"), view);
    QCOMPARE(out, QStringLiteral("%{{}}"));

    // Document:FileBaseName
    out = editor->expandText(QStringLiteral("%{Document:FileBaseName}"), view);
    QCOMPARE(out, QStringLiteral("kate-v5"));

    // Document:FileExtension
    out = editor->expandText(QStringLiteral("%{Document:FileExtension}"), view);
    QCOMPARE(out, QStringLiteral("tar.gz"));

    // Document:FileName
    out = editor->expandText(QStringLiteral("%{Document:FileName}"), view);
    QCOMPARE(out, QStringLiteral("kate-v5.tar.gz"));

    // Document:FilePath
    out = editor->expandText(QStringLiteral("%{Document:FilePath}"), view);
    QCOMPARE(out, QFileInfo(view->document()->url().toLocalFile()).absoluteFilePath());

    // Document:Text
    out = editor->expandText(QStringLiteral("%{Document:Text}"), view);
    QCOMPARE(out, QStringLiteral("get an edge in editing\n:-)"));

    // Document:Path
    out = editor->expandText(QStringLiteral("%{Document:Path}"), view);
    QCOMPARE(out, QFileInfo(doc->url().toLocalFile()).absolutePath());

    // Document:NativeFilePath
    out = editor->expandText(QStringLiteral("%{Document:NativeFilePath}"), view);
    QCOMPARE(out, QDir::toNativeSeparators(QFileInfo(doc->url().toLocalFile()).absoluteFilePath()));

    // Document:NativePath
    out = editor->expandText(QStringLiteral("%{Document:NativePath}"), view);
    QCOMPARE(out, QDir::toNativeSeparators(QFileInfo(doc->url().toLocalFile()).absolutePath()));

    // Document:NativePath
    out = editor->expandText(QStringLiteral("%{Document:NativePath}"), view);
    QCOMPARE(out, QDir::toNativeSeparators(QFileInfo(doc->url().toLocalFile()).absolutePath()));

    // Document:Cursor:Line
    out = editor->expandText(QStringLiteral("%{Document:Cursor:Line}"), view);
    QCOMPARE(out, QStringLiteral("1"));

    // Document:Cursor:Column
    out = editor->expandText(QStringLiteral("%{Document:Cursor:Column}"), view);
    QCOMPARE(out, QStringLiteral("2"));

    // Document:Cursor:XPos
    out = editor->expandText(QStringLiteral("%{Document:Cursor:XPos}"), view);
    QVERIFY(out.toInt() > 0);

    // Document:Cursor:YPos
    out = editor->expandText(QStringLiteral("%{Document:Cursor:YPos}"), view);
    QVERIFY(out.toInt() > 0);

    view->setSelection(KTextEditor::Range(1, 0, 1, 3));
    // Document:Selection:Text
    out = editor->expandText(QStringLiteral("%{Document:Selection:Text}"), view);
    QCOMPARE(out, QStringLiteral(":-)"));

    // Document:Selection:StartLine
    out = editor->expandText(QStringLiteral("%{Document:Selection:StartLine}"), view);
    QCOMPARE(out, QStringLiteral("1"));

    // Document:Selection:StartColumn
    out = editor->expandText(QStringLiteral("%{Document:Selection:StartColumn}"), view);
    QCOMPARE(out, QStringLiteral("0"));

    // Document:Selection:EndLine
    out = editor->expandText(QStringLiteral("%{Document:Selection:EndLine}"), view);
    QCOMPARE(out, QStringLiteral("1"));

    // Document:Selection:EndColumn
    out = editor->expandText(QStringLiteral("%{Document:Selection:EndColumn}"), view);
    QCOMPARE(out, QStringLiteral("3"));

    // Document:RowCount
    out = editor->expandText(QStringLiteral("%{Document:RowCount}"), view);
    QCOMPARE(out, QStringLiteral("2"));

    // Document:Variable:<variable>, since KF 5.78
    qobject_cast<KTextEditor::DocumentPrivate *>(doc)->setVariable(QStringLiteral("cow-sound"), QStringLiteral("moo"));
    out = editor->expandText(QStringLiteral("%{Document:Variable:cow-sound}"), view);
    QCOMPARE(out, QStringLiteral("moo"));

    // Date:Locale
    out = editor->expandText(QStringLiteral("%{Date:Locale}"), view);
    QVERIFY(!out.isEmpty());

    // Date:ISO
    out = editor->expandText(QStringLiteral("%{Date:ISO}"), view);
    QVERIFY(!out.isEmpty());

    // Date:yyyy-MM-dd
    out = editor->expandText(QStringLiteral("%{Date:yyyy-MM-dd}"), view);
    QVERIFY(QDate::fromString(out, QStringLiteral("yyyy-MM-dd")).isValid());

    // Time:Locale
    out = editor->expandText(QStringLiteral("%{Time:Locale}"), view);
    QVERIFY(!out.isEmpty());

    // Time:ISO
    out = editor->expandText(QStringLiteral("%{Time:ISO}"), view);
    QVERIFY(!out.isEmpty());

    // Time:hh-mm-ss
    out = editor->expandText(QStringLiteral("%{Time:hh-mm-ss}"), view);
    QVERIFY(QTime::fromString(out, QStringLiteral("hh-mm-ss")).isValid());

    // ENV:KTE_ENV_VAR_TEST
    qputenv("KTE_ENV_VAR_TEST", "KTE_ENV_VAR_TEST_VALUE");
    out = editor->expandText(QStringLiteral("%{ENV:KTE_ENV_VAR_TEST}"), view);
    QCOMPARE(out, QStringLiteral("KTE_ENV_VAR_TEST_VALUE"));

    // JS:<code>
    out = editor->expandText(QStringLiteral("%{JS:3 + %{JS:2 + 1}}"), view);
    QCOMPARE(out, QStringLiteral("6"));

    // PercentEncoded: since 5.67
    out = editor->expandText(QStringLiteral("%{PercentEncoded:{a&b+c=d} \"}"), view);
    QCOMPARE(out, QStringLiteral("%7Ba%26b%2Bc%3Dd%7D%20%22"));

    // UUID
    out = editor->expandText(QStringLiteral("%{UUID}"), view);
    QCOMPARE(out.count(QLatin1Char('-')), 4);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
