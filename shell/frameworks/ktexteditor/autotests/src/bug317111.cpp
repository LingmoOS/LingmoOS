/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Gerald Senarclens de Grancy <oss@senarclens.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bug317111.h"

#include <katedocument.h>
#include <kateview.h>
#include <kmainwindow.h>

#include <QJSEngine>
#include <QStandardPaths>
#include <QTest>

#include "testutils.h"

QTEST_MAIN(BugTest)

using namespace KTextEditor;

BugTest::BugTest()
    : QObject()
{
}

BugTest::~BugTest()
{
}

void BugTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void BugTest::cleanupTestCase()
{
}

void BugTest::tryCrash()
{
    // set up document and view
    KMainWindow *toplevel = new KMainWindow();
    KTextEditor::DocumentPrivate *doc = new KTextEditor::DocumentPrivate(true, false, toplevel);
    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc->createView(nullptr));
    bool outputWasCustomised = false;
    TestScriptEnv *env = new TestScriptEnv(doc, outputWasCustomised);
    const QUrl url = QUrl::fromLocalFile(QLatin1String(TEST_DATA_DIR "bug317111.txt"));
    doc->openUrl(url);

    // load buggy script
    QFile scriptFile(QLatin1String(JS_DATA_DIR "commands/utils.js"));
    QVERIFY(scriptFile.exists());
    QVERIFY(scriptFile.open(QFile::ReadOnly));
    QJSValue result = env->engine()->evaluate(QString::fromLatin1(scriptFile.readAll()), scriptFile.fileName());
    QVERIFY2(!result.isError(), result.toString().toUtf8().constData());

    // view must be visible...
    view->show();
    view->resize(900, 800);
    view->setCursorPosition(Cursor(0, 0));

    // evaluate test-script
    qDebug() << "attempting crash by calling KTextEditor::DocumentPrivate::defStyle(-1, 0)";
    QFile sourceFile(QLatin1String(TEST_DATA_DIR "bug317111.js"));
    QVERIFY(sourceFile.open(QFile::ReadOnly));
    QTextStream stream(&sourceFile);
    QString code = stream.readAll();
    sourceFile.close();
    // execute script
    result = env->engine()->evaluate(code, QStringLiteral(TEST_DATA_DIR "bug317111.txt"), 1);
    QVERIFY2(!result.isError(), result.toString().toUtf8().constData());

    qDebug() << "PASS (no crash)";
}

#include "moc_bug317111.cpp"
