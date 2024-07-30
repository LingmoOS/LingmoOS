/*
    SPDX-FileCopyrightText: 2024 Jonathan Poelen <jonathan.poelen@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scripttester_test.h"
#include "../src/scripttester/scripttester_p.h"
#include "katedocument.h"
#include "moc_scripttester_test.cpp"

#include <QBuffer>
#include <QJSEngine>
#include <QProcess>
#include <QStandardPaths>
#include <QStringDecoder>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace
{
struct CompareData {
    QString program;
    QString expectedOutput;
};
}

static void compareOutput(const QString &suffixFile, QJSEngine &engine, KTextEditor::ScriptTester &scriptTester, QBuffer &buffer, CompareData d)
{
    buffer.open(QBuffer::WriteOnly);

    auto toString = [](const QJSValue &value) -> QString {
        if (!value.isError()) {
            return value.toString();
        }
        return value.toString() + u": "_s + value.property(u"stack"_s).toString();
    };

    scriptTester.resetCounters();

    /*
     * execute
     */
    QJSValue result = engine.evaluate(d.program, u"myfile"_s, 0);
    QCOMPARE(toString(result), u"function() { [native code] }"_s);

    QJSValue globalObject = engine.globalObject();
    QJSValue functions = engine.newQObject(&scriptTester);
    result = result.callWithInstance(functions, {globalObject});
    QCOMPARE(toString(result), u"undefined"_s);

    /*
     * write counters and flush
     */
    scriptTester.writeSummary();
    scriptTester.stream() << '\n';
    scriptTester.stream().flush();

    /*
     * extract output
     */
    const QString outputResult = QStringDecoder(QStringDecoder::Utf8)(buffer.data());

    /*
     * clear buffer
     */
    buffer.close();
    buffer.buffer().clear();

    /*
     * comparison
     */
    const bool equalResults = outputResult == d.expectedOutput;
    if (equalResults) {
        return;
    }

    /*
     * write buffers to files
     */
    const QString tempdir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString resultPath = tempdir + u"/scripttester_"_s + suffixFile + u"_result.txt"_s;
    const QString exptedPath = tempdir + u"/scripttester_"_s + suffixFile + u"_expected.txt"_s;
    QFile outFile;
    // result
    outFile.setFileName(resultPath);
    QVERIFY(outFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream(&outFile) << outputResult;
    outFile.close();
    // expected
    outFile.setFileName(exptedPath);
    QVERIFY(outFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream(&outFile) << d.expectedOutput;
    outFile.close();

    /*
     * elaborate diff output, if possible
     */
    const QString diffExecutable = QStandardPaths::findExecutable(u"diff"_s);
    if (!diffExecutable.isEmpty()) {
        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.start(diffExecutable, {u"-u"_s, exptedPath, resultPath});
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitCode(), 0);
    }
    /*
     * else: trivial output of mismatching characters, e.g. for windows testing without diff
     */
    else {
        qDebug() << "Trivial differences output as the 'diff' executable is not in the PATH";
        auto expectedLines = QStringView(d.expectedOutput).split(u'\n');
        auto outputLines = QStringView(outputResult).split(u'\n');
        for (qsizetype i = 0; i < expectedLines.size() || i < outputLines.size(); ++i) {
            QStringView expectedLine = i < expectedLines.size() ? expectedLines[i] : QStringView();
            QStringView outputLine = i < outputLines.size() ? outputLines[i] : QStringView();
            QCOMPARE(expectedLine, outputLine);
        }
        QCOMPARE(d.expectedOutput.size(), outputResult.size());
    }

    /*
     * if we arrive here, all lost!
     */
    QVERIFY(equalResults);
}

QTEST_MAIN(ScriptTesterTest)

#define testNewRow() (QTest::newRow(QString("line %1").arg(__LINE__).toAscii().data()))

ScriptTesterTest::ScriptTesterTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

ScriptTesterTest::~ScriptTesterTest()
{
}

void ScriptTesterTest::testDebug()
{
}

void ScriptTesterTest::testPrintExpression()
{
    using ScriptTester = KTextEditor::ScriptTester;
    using TextFormat = ScriptTester::DocumentTextFormat;
    using TestFormatOption = ScriptTester::TestFormatOption;
    // using PatternType = ScriptTester::PatternType;
    using DebugOption = ScriptTester::DebugOption;

    KTextEditor::DocumentPrivate doc(true, false);
    KTextEditor::ViewPrivate view(&doc, nullptr);
    QJSEngine engine;
    QBuffer buffer;
    ScriptTester scriptTester(&buffer,
                              ScriptTester::Format{
                                  .debugOptions = DebugOption::WriteLocation | DebugOption::WriteFunction,
                                  .testFormatOptions = TestFormatOption::None,
                                  .documentTextFormat = TextFormat::ReplaceNewLineAndTabWithLiteral,
                                  .documentTextFormatWithBlockSelection = TextFormat::ReplaceNewLineAndTabWithPlaceholder,
                                  .textReplacement =
                                      ScriptTester::Format::TextReplacement{
                                          .newLine = u'↵',
                                          .tab1 = u'—',
                                          .tab2 = u'⇥',
                                      },
                                  .fallbackPlaceholders =
                                      ScriptTester::Placeholders{
                                          .cursor = u'|',
                                          .selectionStart = u'[',
                                          .selectionEnd = u']',
                                          .secondaryCursor = u'┆',
                                          .secondarySelectionStart = u'❲',
                                          .secondarySelectionEnd = u'❳',
                                          .virtualText = u'·',
                                      },
                                  .colors =
                                      {
                                          .reset = u"</>"_s,
                                          .success = u"<success>"_s,
                                          .error = u"<error>"_s,
                                          .carret = u"<carret>"_s,
                                          .debugMarker = u"<debugMarker>"_s,
                                          .debugMsg = u"<debugMsg>"_s,
                                          .testName = u"<testName>"_s,
                                          .program = u"<program>"_s,
                                          .fileName = u"<fileName>"_s,
                                          .lineNumber = u"<lineNumber>"_s,
                                          .blockSelectionInfo = u"<blockSelectionInfo>"_s,
                                          .labelInfo = u"<labelInfo>"_s,
                                          .cursor = u"<cursor>"_s,
                                          .selection = u"<selection>"_s,
                                          .secondaryCursor = u"<secondaryCursor>"_s,
                                          .secondarySelection = u"<secondarySelection>"_s,
                                          .blockSelection = u"<blockSelection>"_s,
                                          .inSelection = u"<inSelection>"_s,
                                          .virtualText = u"<virtualText>"_s,
                                          .result = u"<result>"_s,
                                          .resultReplacement = u"<rep>"_s,
                                      },
                              },
                              ScriptTester::Paths{},
                              ScriptTester::TestExecutionConfig{},
                              ScriptTester::DiffCommand{},
                              ScriptTester::Placeholders{
                                  .cursor = u'|',
                                  .selectionStart = u'[',
                                  .selectionEnd = u']',
                                  .secondaryCursor = u'\0',
                                  .secondarySelectionStart = u'\0',
                                  .secondarySelectionEnd = u'\0',
                                  .virtualText = u'\0',
                              },
                              &engine,
                              &doc,
                              &view);

    // add debug() function
    QJSValue globalObject = engine.globalObject();
    QJSValue functions = engine.newQObject(&scriptTester);
    globalObject.setProperty(u"debug"_s, functions.property(u"debug"_s));

    auto makeProgram = [](QString program) -> QString {
        return u"(function(env, argv){"
               u"const TestFramework = this.loadModule('" JS_SCRIPTTESTER_DATA_DIR
               u"testframework.js');"
               u"TestFramework.init(this, env, 2);"
               u"var {lt, cmd, print, config, testCase, withInput} = TestFramework;"
               u""_s
            + program + u"})"_s;
    };

    compareOutput(u"testCase"_s,
                  engine,
                  scriptTester,
                  buffer,
                  CompareData{
                      .program = makeProgram(uR"(
        function foo() { return true; }
        config({blockSelection: 0})
        testCase('MyTest', () => {
            cmd(foo, 'abc\ndef', 'abc\ndef|') // no error
            cmd(foo, 'abc\ndef', 'abc\ndef|', { expected: 1 })
            cmd(foo, 'abc', 'abc\ndef|', { expected: {a:42} })
            config({virtualText: '@', blockSelection: 1})
            cmd(foo, 'abcxxxxxxxxx|[\ndaa aaa\ndaaaa]aaaaaef', 'abc@@@|\nabc\ndef')
            ;
        });
    )"_s),
                      .expectedOutput = uR"(<fileName>myfile</>:<lineNumber>5</>: <testName>MyTest</>: <error>Result differs
<error>cmd `</><program>foo() === {expectedResult}</><error>`</><blockSelectionInfo> [blockSelection=0]</>:
<labelInfo>  input:    </><result>abc</><rep>\n</><result>def</><cursor>|</>
<labelInfo>  output:   </><result>abc</><rep>\n</><result>def</><cursor>|</>
  ---------
  result:   <result>true</>
  expected: <result>1</>
            <carret>^~~</>

<fileName>myfile</>:<lineNumber>6</>: <testName>MyTest</>: <error>Output and Result differs
<error>cmd `</><program>foo() === {expectedResult}</><error>`</><blockSelectionInfo> [blockSelection=0]</>:
  input:    <result>abc</><cursor>|</>
  output:   <result>abc</><cursor>|</>
  expected: <result>abc</><rep>\n</><result>def</><cursor>|</>
               <carret>^~~</>
  ---------
  result:   <result>true</>
  expected: <result>{a: 42}</>
            <carret>^~~</>

<fileName>myfile</>:<lineNumber>8</>: <testName>MyTest</>: <error>Output differs
<error>cmd `</><program>foo()</><error>`</><blockSelectionInfo> [blockSelection=1]</>:
  input:    <result>abcxx</><blockSelection>[</><result><inSelection>xxxxxxx</><cursor><inSelection>|</><selection>]</><rep>↵</>
            <result>daa a</><blockSelection>[</><result><inSelection>aa</></><virtualText><inSelection>@@@@@</><blockSelection>]</><rep>↵</>
            <result>daaaa</><selection>[</><result><inSelection>aaaaaef</><blockSelection>]</>

  output:   <result>abcxx</><blockSelection>[</><result><inSelection>xxxxxxx</><cursor><inSelection>|</><selection>]</><rep>↵</>
               <carret>^~~</>
            <result>daa a</><blockSelection>[</><result><inSelection>aa</></><virtualText><inSelection>@@@@@</><blockSelection>]</><rep>↵</>
            <result>daaaa</><selection>[</><result><inSelection>aaaaaef</><blockSelection>]</>

  expected: <result>abc</></><virtualText>@@@</><cursor>|</><rep>↵</>
               <carret>^~~</>
            <result>abc</><rep>↵</>
            <result>def</>


Success: <success>1</>  Failure: <error>3</>
)"_s,
                  });

    compareOutput(u"withInput"_s,
                  engine,
                  scriptTester,
                  buffer,
                  CompareData{
                      .program = makeProgram(uR"(
        function foo() { return true; }
        config({blockSelection: 0})
        withInput('MyTest2', 'abc|', () => {
            print('print');
            cmd(foo, TestFramework.EXPECTED_OUTPUT_AS_INPUT); // no error
            cmd(foo, 'abc\ndef|');
            cmd(foo, 'abc\ndef|', { expected: {a:42} });
            lt(foo, 1);
        });
    )"_s),
                      .expectedOutput = uR"(<fileName>myfile</>:<lineNumber>4</><debugMsg>: </><debugMarker>PRINT:</><debugMsg> print</>
<fileName>myfile</>:<lineNumber>6</>: <testName>MyTest2</>: <error>Output differs
<error>cmd `</><program>foo()</><error>`</><blockSelectionInfo> [blockSelection=0]</>:
  input:    <result>abc</><cursor>|</>
  output:   <result>abc</><cursor>|</>
  expected: <result>abc</><rep>\n</><result>def</><cursor>|</>
               <carret>^~~</>

<fileName>myfile</>:<lineNumber>7</>: <testName>MyTest2</>: <error>Output and Result differs
<error>cmd `</><program>foo() === {expectedResult}</><error>`</><blockSelectionInfo> [blockSelection=0]</>:
  input:    <result>abc</><cursor>|</>
  output:   <result>abc</><cursor>|</>
  expected: <result>abc</><rep>\n</><result>def</><cursor>|</>
               <carret>^~~</>
  ---------
  result:   <result>true</>
  expected: <result>{a: 42}</>
            <carret>^~~</>

<fileName>myfile</>:<lineNumber>8</>: <testName>MyTest2</>: <error>Result differs
<error>test `</><program>foo() < {expected}</><error>`</><blockSelectionInfo> [blockSelection=0]</>:
  result:   <result>true</>
  expected: <result>1</>
            <carret>^~~</>


Success: <success>1</>  Failure: <error>3</>
)"_s,
                  });
}
