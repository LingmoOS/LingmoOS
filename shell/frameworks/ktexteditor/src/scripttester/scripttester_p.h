/*
    SPDX-FileCopyrightText: 2024 Jonathan Poelen <jonathan.poelen@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_SCRIPT_TESTER_HELPERS_H
#define KTEXTEDITOR_SCRIPT_TESTER_HELPERS_H

#include <ktexteditor/cursor.h>
#include <ktexteditor/range.h>

#include "kateview.h"

#include <QFlags>
#include <QJSValue>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QStringView>
#include <QTextStream>

#include <vector>

class QJSEngine;

namespace KTextEditor
{

class DocumentPrivate;

/**
 * Enables unit tests to be run on js scripts (commands, indentations, libraries).
 *
 * Unit tests are written in javascript via a module called TestFramework.
 *
 * This class configures a document and a view through strings representing
 * the document text and the position of cursors and selections via special
 * characters called placeholders.
 *
 * For example, using \c setInput() with `"a[bc]|d"` will put the text "abcd"
 * in the document, a selection from the second to the third character and
 * place the cursor on the latter.
 */
class KTEXTEDITOR_NO_EXPORT ScriptTester : public QObject
{
    Q_OBJECT

public:
    /**
     * Controls the behavior of the debug function.
     */
    enum class DebugOption : unsigned char {
        None,
        /// Add location before log (file name and line number).
        WriteLocation = 1 << 0,
        /// Add function name before log .
        WriteFunction = 1 << 1,
        /// Add stacktrace after log.
        WriteStackTrace = 1 << 2,
        /// Forces writing every time \c debug is called.
        /// Otherwise, the log is only visible if the test fails.
        ForceFlush = 1 << 3,
    };
    Q_DECLARE_FLAGS(DebugOptions, DebugOption)

    enum class TestFormatOption : unsigned char {
        None,
        AlwaysWriteInputOutput = 1 << 0,
        AlwaysWriteLocation = 1 << 1,
        HiddenTestName = 1 << 2,
    };
    Q_DECLARE_FLAGS(TestFormatOptions, TestFormatOption)

    /**
     * Format used for input, output and expectedOutput of command display.
     */
    enum class DocumentTextFormat : unsigned char {
        /// No transformation.
        Raw,
        /// Formats to a valid javascript String.
        EscapeForDoubleQuote,
        /// Replace new line with \c \n and tab with \c \t.
        ReplaceNewLineAndTabWithLiteral,
        /// Replace new line and tab with a placeholder (see \ref Format::TextReplacement)
        ReplaceNewLineAndTabWithPlaceholder,
        /// Replace tab with a placeholder (see \ref Format::TextReplacement)
        ReplaceTabWithPlaceholder,
    };

    enum class PatternType : unsigned char {
        /// No filter, all tests will be executed.
        Inactive,
        /// Only tests whose pattern matches will be executed.
        Include,
        /// Tests whose pattern matches will be ignored.
        Exclude,
    };

    /**
     * Placeholders to represent cursor, selection and virtual text in a string.
     * A placeholder is used if it is activated (\c enabled)
     * and the character value is not 0.
     */
    struct Placeholders {
        QChar cursor;
        QChar selectionStart;
        QChar selectionEnd;
        QChar secondaryCursor;
        QChar secondarySelectionStart;
        QChar secondarySelectionEnd;
        // Text to insert "spaces" for block selection outside lines.
        // However, the new line must come after.
        QChar virtualText;

        /**
         * Check that a placeholder is not 0.
         */
        /// @{
        bool hasCursor() const
        {
            return cursor.unicode();
        }

        bool hasSelection() const
        {
            return selectionStart.unicode() && selectionEnd.unicode();
        }

        bool hasSecondaryCursor() const
        {
            return secondaryCursor.unicode();
        }

        bool hasSecondarySelection() const
        {
            return secondarySelectionStart.unicode() && secondarySelectionEnd.unicode();
        }

        bool hasVirtualText() const
        {
            return virtualText.unicode();
        }
        /// @}
    };

    /**
     * ANSI character sequence insert into output.
     */
    struct Colors {
        QString reset;
        /// Number of successful tests.
        QString success;
        /// Error in tests or exception.
        QString error;
        /// "^~~" sequence under error position.
        QString carret = error;
        /// "DEBUG:" prefix with \c debug().
        QString debugMarker = error;
        /// Debug message with \c debug().
        QString debugMsg = error;
        /// Name of the test passed to testCase and co.
        QString testName;
        /// Program paramater in \c cmd() / \c test(). Function name in stacktrace.
        QString program;
        QString fileName;
        QString lineNumber;
        /// [blockSelection=...] displayed as information in a check.
        QString blockSelectionInfo;
        /// "input", "output", "result" label when it is displayed as information and not as an error.
        QString labelInfo;
        /// Cursor placeholder.
        QString cursor;
        /// Selection placeholder.
        QString selection;
        /// Secondary cursor placeholder.
        QString secondaryCursor;
        /// Secondary selection placeholder.
        QString secondarySelection;
        /// Block selection placeholder.
        QString blockSelection;
        /// Text inside a selection. Style is added.
        QString inSelection;
        /// Virtual text placeholder.
        QString virtualText;
        /// Text representing the inputs and outputs of a test.
        QString result;
        /// Text replaced by \ref DocumentTextFormat / \ref TextReplacement.
        QString resultReplacement;
    };

    /**
     * Groups the options that control the display of tests.
     */
    struct Format {
        /**
         * Placeholder for \ref DocumentTextFormat.
         */
        struct TextReplacement {
            /// Character inserted at end of line.
            /// Useful for distinguishing end-of-line spaces.
            QChar newLine;
            /// Character used to replace a tab (repeated \c tabWidth times).
            QChar tab1;
            /// Character used to replace last char in tab.
            /// This distinguishes 2 successive tabs:
            /// For \p tab1 = '-' and \p tab2 = '>' with \c tabWidth = 4
            /// then \c '\t\t\t' is replaced by \c '--->--->--->'.
            QChar tab2;
        };

        DebugOptions debugOptions;
        TestFormatOptions testFormatOptions;
        DocumentTextFormat documentTextFormat;
        DocumentTextFormat documentTextFormatWithBlockSelection;
        TextReplacement textReplacement;
        /**
         * Placeholder used for display when \c cursor is identical to
         * \c selectionStart, \c selectionEnd, represents a new line or 0
         * (idem for secondary cursor).
         */
        Placeholders fallbackPlaceholders;
        Colors colors;
    };

    /**
     * Folder path for javascript scripts and data test.
     */
    struct Paths {
        /// Paths for \ref loadScript().
        QStringList scripts;
        /// Paths for \ref require() (KTextEditor JS API).
        QStringList libraries;
        /// Paths for \ref read() (KTextEditor JS API).
        QStringList files;
        /// Paths for \ref loadModule().
        QStringList modules;
        /// Base path for \ref testIndentFiles().
        QString indentBaseDir;
    };

    struct TestExecutionConfig {
        /**
         * maximum number of tests that can fail before the framework
         * returns a StopCaseError and execution stops.
         * Negative value or 0 means infinity.
         */
        int maxError = 0;

        /**
         * When true, xcmd() and xtest() functions will always return a failure.
         */
        bool xCheckAsFailure = false;

        /**
         * A pattern include or exclude test.
         */
        QRegularExpression pattern{};
        PatternType patternType = PatternType::Inactive;
    };

    /**
     * Diff command for \c testIndentFiles().
     */
    struct DiffCommand {
        QString path;
        QStringList args;
    };

    explicit ScriptTester(QIODevice *output,
                          const Format &format,
                          const Paths &paths,
                          const TestExecutionConfig &executionConfig,
                          const DiffCommand &diffCmd,
                          Placeholders placeholders,
                          QJSEngine *engine,
                          DocumentPrivate *doc,
                          ViewPrivate *view,
                          QObject *parent = nullptr);

    ScriptTester(const ScriptTester &) = delete;
    ScriptTester &operator=(const ScriptTester &) = delete;

    QTextStream &stream()
    {
        return m_stream;
    }

    // KTextEditor API
    //@{
    /// See \ref Paths.
    Q_INVOKABLE QString read(const QString &file);
    /// See \ref Paths.
    Q_INVOKABLE void require(const QString &file);
    /// See \ref DebugOption.
    Q_INVOKABLE void debug(const QString &msg);
    //@}

    // Keyboard
    //@{
    Q_INVOKABLE void type(const QString &str);
    Q_INVOKABLE void enter();
    //@}

    // Utility function
    //@{
    Q_INVOKABLE void paste(const QString &str);
    //@}

    /**
     * Test the indentation of all files in the \p dataDir folder.
     * This folder must contain subfolders with \c origin and \c expected files.
     * If the result is different, a file named \c actual will be written and
     * the difference will be displayed.
     * @param name name of test
     * @param dataDir path of directory
     * @param nthStack call stack line where to find test file name and line number
     * @return true when the indentation matches the expected file.
     */
    Q_INVOKABLE bool testIndentFiles(const QString &name, const QString &dataDir, int nthStack, bool exitOnError);

    /**
     * Load a javascript module.
     * @param fileName file name or path of js module. If the path starts with
     * "./" or "../", then the search is relative to the file calling the
     * function. Other relative paths are relative to \ref JSPath.modules.
     */
    Q_INVOKABLE QJSValue loadModule(const QString &fileName);

    /**
     * Load a javascript script command.
     * @param fileName file name or path of js script. If the path starts with
     * "./" or "../", then the search is relative to the file calling the
     * function. Other relative paths are relative to \ref JSPath.scripts.
     */
    Q_INVOKABLE void loadScript(const QString &fileName);

    /**
     * Like \c debug, but not buffered.
     */
    Q_INVOKABLE void print(const QString &msg);

    /**
     * Format and write an exception.
     */
    void writeException(const QJSValue &exception, QStringView prefix);

    /**
     * Displays test information such as the number of successes or failures.
     */
    void writeSummary();

    /**
     * Reset all counters.
     */
    void resetCounters();

    /**
     * @param name name of test
     * @param nthStack call stack line where to find test file name and line number
     * @return true if tests can start, otherwise false (when filtered)
     */
    Q_INVOKABLE bool startTestCase(const QString &name, int nthStack);

    /**
     * Config for \c Placeholders \ref DocumentPrivate and \ref ViewPrivate.
     * An empty string disables a placeholder.
     * A string in a fallback placeholder (cursor2, selection2, etc) resets it
     * to the default.
     */
    Q_INVOKABLE void setConfig(const QJSValue &config);

    /**
     * Reset the configuraion to the original state.
     */
    Q_INVOKABLE void resetConfig();

    /**
     * Saves the last state of the configuration to restore it later.
     */
    Q_INVOKABLE void pushConfig();

    /**
     * Restores the last saved configuration.
     */
    Q_INVOKABLE void popConfig();

    /**
     * Evaluates \p program and returns the result of the evaluation.
     * @param program
     * @return result of program
     */
    Q_INVOKABLE QJSValue evaluate(const QString &program);

    /**
     * Set a input document text, cursor and selection positions according to
     * its placeholders.
     *
     * - If no cursor is specified, then it will be placed at the end of the
     *   selection if present, or at the end of the document.
     *
     * - If several secondary cursors are specified, but none primary, then the
     *   first secondary cursor will be a primary cursor.
     *   The same applies to selection.
     *
     * - The start of a selection must be before the end of a selection.
     *   If a cursor is to be located on the 'start' part of the selection,
     *   then either it must be explicitly indicated, or the 'cursor'
     *   placeholder must have the same value as 'selectionStart'.
     *
     * - Several placeholders can have the same value, but they must not be
     *   identical to the 'virtualText' placeholder.
     *
     * - In the case of virtual text, the cursor is automatically placed at the
     *   end of the line if block selection is disabled. No text character
     *   other than the line feed may be present after a virtual text.
     *   Placeholders are of course permitted.
     *
     * @param input input document text with placeholders.
     * @param blockSelection indicates whether the display should use block selection
     */
    Q_INVOKABLE void setInput(const QString &input, bool blockSelection);

    /**
     * Move the expected output previously defined as input text value.
     * @param blockSelection indicates whether the display should use block selection
     */
    Q_INVOKABLE void moveExpectedOutputToInput(bool blockSelection);

    /**
     * Reset input document text to previously defined input value.
     * @param blockSelection indicates whether the display should use block selection
     */
    Q_INVOKABLE void reuseInput(bool blockSelection);

    /**
     * As \c reuseInput, but checks that block selection mode is possible.
     * @return true if block selection mode is compatible. Otherwise false.
     */
    Q_INVOKABLE bool reuseInputWithBlockSelection();

    /**
     * Same as \c setInput(), but for expected output document text.
     * @param expected output document text with placeholders.
     * @param blockSelection indicates whether the display should use block selection
     */
    Q_INVOKABLE void setExpectedOutput(const QString &expected, bool blockSelection);

    /**
     * Reset expected output document text to previously defined expected output value.
     * @param blockSelection indicates whether the display should use block selection
     */
    Q_INVOKABLE void reuseExpectedOutput(bool blockSelection);

    /**
     * Set expected output with the same value as input.
     * @param blockSelection indicates whether the display should use block selection
     */
    Q_INVOKABLE void copyInputToExpectedOutput(bool blockSelection);

    /**
     * Check that the output corresponds to the expected output.
     * @return bool \c true if the values are identical, otherwise \c false.
     */
    Q_INVOKABLE bool checkOutput();

    /**
     * Increment the success counter or the failure counter.
     * @param isSuccessNotAFailure \c true for the success counter, \c false for the failure counter
     * @param xcheck Boolean: true for an expected failure
     * @return \p isSuccessNotAFailure
     */
    Q_INVOKABLE bool incrementCounter(bool isSuccessNotAFailure, bool xcheck);

    /**
     * Increment the error counter.
     */
    Q_INVOKABLE void incrementError();

    /**
     * Increment the break test case counter.
     * This counter corresponds to the test cases that are stopped following a failure.
     */
    Q_INVOKABLE void incrementBreakOnError();

    /**
     * Return error and failure count.
     */
    Q_INVOKABLE int countError() const;

    /**
     * Check if the number of errors is too high.
     */
    Q_INVOKABLE bool hasTooManyErrors() const;

    /**
     * Start a check.
     * @return flags with 0x1 when \c writeTestResult must always be called
     * and 0x2 when \c writeTestExpression must always called
     */
    Q_INVOKABLE int startTest();

    /**
     * Complete a test.
     */
    Q_INVOKABLE void endTest(bool ok, bool showBlockSelection = false);

    /**
     * Write a test.
     * @param name name of test
     * @param type test type ("cmd" or "test")
     * @param nthStack call stack line where to find test file name and line number
     * @param program program used for test
     */
    Q_INVOKABLE void writeTestExpression(const QString &name, const QString &type, int nthStack, const QString &program);

    /**
     * Write a test aborted.
     * @param name name of test
     * @param nthStack call stack line where to find test file name and line number
     */
    Q_INVOKABLE void writeDualModeAborted(const QString &name, int nthStack);

    /**
     * Write a test result.
     * @param name name of test
     * @param type test type ("cmd" or "test")
     * @param nthStack call stack line where to find test file name and line number
     * @param program program used for test
     * @param msg user message
     * @param exception program exception
     * @param result program result in displayable format
     * @param expectedResult expected program result in a displayable format
     * @param options display options
     */
    Q_INVOKABLE void writeTestResult(const QString &name,
                                     const QString &type,
                                     int nthStack,
                                     const QString &program,
                                     const QString &msg,
                                     const QJSValue &exception,
                                     const QString &result,
                                     const QString &expectedResult,
                                     int options);

private:
    struct Replacements;
    struct TextItem;

    struct DocumentText {
        std::vector<TextItem> items;
        QString text;
        Cursor cursor;
        Range selection;
        // secondary cursor with selection
        QList<ViewPrivate::PlainSecondaryCursor> secondaryCursorsWithSelection;
        std::vector<ViewPrivate::PlainSecondaryCursor> secondaryCursors;
        int totalLine = 0;
        // used with checkMultiCursorCompatibility(), ignored for m_output
        //@{
        int totalCursor = 0;
        int totalSelection = 0;
        //@}
        bool hasFormattingItems = false;
        bool hasBlockSelectionItems = false;
        bool blockSelection = false;

        DocumentText();
        ~DocumentText();
        DocumentText(DocumentText &&) = default;
        DocumentText &operator=(DocumentText &&) = default;
        DocumentText &operator=(DocumentText const &) = default;

        QString setText(QStringView input, const Placeholders &placeholders);

        std::size_t addItems(QStringView str, int kind, QChar c);
        std::size_t addSelectionItems(QStringView str, int kind, QChar start, QChar end);

        void computeBlockSelectionItems();

        void insertFormattingItems(DocumentTextFormat format);

        void sortItems();
    };

    /**
     * Init config of m_view and m_doc.
     */
    void initDocConfig();
    void syncIndenter();

    /**
     * Init m_view and m_doc.
     */
    void initInputDoc();

    /**
     * Display file name and line number.
     * @param nthStack call stack line where to find test file name and line number
     */
    void writeLocation(int nthStack);

    void writeTestName(const QString &name);
    void writeTypeAndProgram(const QString &type, const QString &program);

    void writeDataTest(bool sameInputOutput);

    bool checkMultiCursorCompatibility(const DocumentText &doc, bool blockSelection, QString *err);

    struct EditorConfig {
        QString syntax;
        QString indentationMode;
        int indentationWidth;
        int tabWidth;
        bool replaceTabs;
        bool autoBrackets;

        bool updated;
        bool inherited;
    };

    static EditorConfig makeEditorConfig();

    struct Config {
        Placeholders fallbackPlaceholders;
        Placeholders placeholders;
        EditorConfig editorConfig;
    };

    QJSEngine *m_engine;
    DocumentPrivate *m_doc;
    ViewPrivate *m_view;
    DocumentText m_input;
    DocumentText m_output;
    DocumentText m_expected;
    Placeholders m_fallbackPlaceholders;
    Placeholders m_defaultPlaceholders;
    Placeholders m_placeholders;
    EditorConfig m_editorConfig;
    QTextStream m_stream;
    QString m_debugMsg;
    QString m_stringBuffer;
    Format m_format;
    Paths m_paths;
    std::vector<Config> m_configStack;
    QMap<QString, QString> m_libraryFiles;
    TestExecutionConfig m_executionConfig;
    DiffCommand m_diffCmd;
    bool m_diffCmdLoaded = false;
    bool m_hasDebugMessage = false;
    int m_successCounter = 0;
    int m_failureCounter = 0;
    int m_xSuccessCounter = 0;
    int m_xFailureCounter = 0;
    int m_skipedCounter = 0;
    int m_errorCounter = 0;
    int m_breakOnErrorCounter = 0;
    int m_dualModeAbortedCounter = 0;
    qsizetype m_startTime = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ScriptTester::DebugOptions)

} // namespace KTextEditor

#endif
