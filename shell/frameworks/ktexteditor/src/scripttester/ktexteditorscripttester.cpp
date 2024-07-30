/*
    SPDX-FileCopyrightText: 2024 Jonathan Poelen <jonathan.poelen@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateconfig.h"
#include "katedocument.h"
#include "katescriptdocument.h"
#include "katescriptview.h"
#include "kateview.h"
#include "ktexteditor_version.h"
#include "scripttester_p.h"

#include <chrono>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QStandardPaths>
#include <QVarLengthArray>
#include <QtEnvironmentVariables>
#include <QtLogging>

using namespace Qt::Literals::StringLiterals;

namespace
{

constexpr QStringView operator""_sv(const char16_t *str, size_t size) noexcept
{
    return QStringView(str, size);
}

using ScriptTester = KTextEditor::ScriptTester;
using TextFormat = ScriptTester::DocumentTextFormat;
using TestFormatOption = ScriptTester::TestFormatOption;
using PatternType = ScriptTester::PatternType;
using DebugOption = ScriptTester::DebugOption;

constexpr inline ScriptTester::Format::TextReplacement defaultTextReplacement{
    .newLine = u'↵',
    .tab1 = u'—',
    .tab2 = u'⇥',
};
constexpr inline ScriptTester::Placeholders defaultPlaceholder{
    .cursor = u'|',
    .selectionStart = u'[',
    .selectionEnd = u']',
    .secondaryCursor = u'\0',
    .secondarySelectionStart = u'\0',
    .secondarySelectionEnd = u'\0',
    .virtualText = u'\0',
};
constexpr inline ScriptTester::Placeholders defaultFallbackPlaceholders{
    .cursor = u'|',
    .selectionStart = u'[',
    .selectionEnd = u']',
    .secondaryCursor = u'┆',
    .secondarySelectionStart = u'❲',
    .secondarySelectionEnd = u'❳',
    .virtualText = u'·',
};

enum class DualMode {
    Dual,
    NoBlockSelection,
    BlockSelection,
    DualIsAlwaysDual,
    AlwaysDualIsDual,
};

struct ScriptTesterQuery {
    ScriptTester::Format format{.debugOptions = DebugOption::WriteLocation | DebugOption::WriteFunction,
                                .testFormatOptions = TestFormatOption::None,
                                .documentTextFormat = TextFormat::ReplaceNewLineAndTabWithLiteral,
                                .documentTextFormatWithBlockSelection = TextFormat::ReplaceNewLineAndTabWithPlaceholder,
                                .textReplacement = defaultTextReplacement,
                                .fallbackPlaceholders = defaultFallbackPlaceholders,
                                .colors = {
                                    .reset = u"\033[m"_s,
                                    .success = u"\033[32m"_s,
                                    .error = u"\033[31m"_s,
                                    .carret = u"\033[31m"_s,
                                    .debugMarker = u"\033[31;1m"_s,
                                    .debugMsg = u"\033[31m"_s,
                                    .testName = u"\x1b[36m"_s,
                                    .program = u"\033[32m"_s,
                                    .fileName = u"\x1b[34m"_s,
                                    .lineNumber = u"\x1b[35m"_s,
                                    .blockSelectionInfo = u"\x1b[37m"_s,
                                    .labelInfo = u"\x1b[37m"_s,
                                    .cursor = u"\x1b[40;1;33m"_s,
                                    .selection = u"\x1b[40;1;33m"_s,
                                    .secondaryCursor = u"\x1b[40;33m"_s,
                                    .secondarySelection = u"\x1b[40;33m"_s,
                                    .blockSelection = u"\x1b[40;37m"_s,
                                    .inSelection = u"\x1b[4m"_s,
                                    .virtualText = u"\x1b[40;37m"_s,
                                    .result = u"\x1b[40m"_s,
                                    .resultReplacement = u"\x1b[40;36m"_s,
                                }};

    ScriptTester::Paths paths{
        .scripts = {},
        .libraries = {u":/ktexteditor/script/libraries"_s},
        .files = {},
        .modules = {},
        .indentBaseDir = {},
    };

    ScriptTester::TestExecutionConfig executionConfig;

    ScriptTester::DiffCommand diff;

    QString preamble;
    QStringList argv;

    struct Variable {
        QString key;
        QString value;
    };

    QList<Variable> variables;

    QStringList fileNames;

    QByteArray xdgDataDirs;

    DualMode dualMode = DualMode::Dual;

    bool showPreamble = false;
    bool extendedDebug = false;
    bool restoreXdgDataDirs = false;
    bool asText = false;
};

struct TrueColor {
    char ansi[11];
    char isBg : 1;
    char len : 7;

    /**
     * @return parsed color with \c len == 0 when there is an error.
     */
    static TrueColor fromRGB(QStringView color, bool isBg)
    {
        auto toHex = [](QChar c) {
            if (c <= u'9' && c >= u'0') {
                return c.unicode() - u'0';
            }
            if (c <= u'f' && c >= u'a') {
                return c.unicode() - u'a';
            }
            if (c <= u'F' && c >= u'A') {
                return c.unicode() - u'A';
            }
            return 0;
        };

        TrueColor trueColor;

        int r = 0;
        int g = 0;
        int b = 0;
        // format: #rgb
        if (color.size() == 4) {
            r = toHex(color[1]);
            g = toHex(color[2]);
            b = toHex(color[3]);
            r = (r << 4) + r;
            g = (g << 4) + g;
            b = (b << 4) + b;
        }
        // format: #rrggbb
        else if (color.size() == 7) {
            r = (toHex(color[1]) << 4) + toHex(color[2]);
            g = (toHex(color[3]) << 4) + toHex(color[4]);
            b = (toHex(color[5]) << 4) + toHex(color[6]);
        }
        // invalid format
        else {
            trueColor.len = 0;
            return trueColor;
        }

        auto *p = trueColor.ansi;
        auto pushComponent = [&](int color) {
            if (color > 99) {
                *p++ = "0123456789"[color / 100];
                color /= 10;
                *p++ = "0123456789"[color / 10];
            } else if (color > 9) {
                *p++ = "0123456789"[color / 10];
            }
            *p++ = "0123456789"[color % 10];
        };
        pushComponent(r);
        *p++ = ';';
        pushComponent(g);
        *p++ = ';';
        pushComponent(b);
        trueColor.len = p - trueColor.ansi;
        trueColor.isBg = isBg;

        return trueColor;
    }

    QLatin1StringView sv() const
    {
        return QLatin1StringView{ansi, len};
    }
};

/**
 * Parse a comma-separated list of color, style, ansi-sequence.
 * @param str string to parse
 * @param defaultColor default colors and styles
 * @param ok failure is reported by setting *ok to false
 * @return ansi sequence
 */
QString toANSIColor(QStringView str, const QString &defaultColor, bool *ok)
{
    QString result;
    QVarLengthArray<TrueColor, 8> trueColors;

    if (!str.isEmpty()) {
        qsizetype totalLen = 0;
        bool hasDefaultColor = !defaultColor.isEmpty();
        auto colors = str.split(u',', Qt::SkipEmptyParts);
        if (colors.isEmpty()) {
            result = defaultColor;
            return result;
        }

        /*
         * Parse colors.
         * TrueColor are replaced by empty QStringViews and pushed to the trueColors array.
         */
        for (auto &color : colors) {
            // ansi code
            if (color[0] <= u'9' && color[0] >= u'0') {
                // check ansi sequence
                for (auto c : color) {
                    if (c > u'9' && c < u'0' && c != u';') {
                        *ok = false;
                        return result;
                    }
                }
            } else {
                const bool isBg = color.startsWith(u"bg=");
                auto s = isBg ? color.sliced(3) : color;
                const bool isBright = s.startsWith(u"bright-");
                s = isBright ? s.sliced(7) : s;
                using SVs = const QStringView[];

                // true color
                if (s[0] == u'#') {
                    if (isBright) {
                        *ok = false;
                        return result;
                    }
                    const auto trueColor = TrueColor::fromRGB(s, isBg);
                    if (!trueColor.len) {
                        *ok = false;
                        return result;
                    }
                    totalLen += 5 + trueColor.len;
                    trueColors += trueColor;
                    color = QStringView();
                    // colors
                } else if (s == u"black"_sv) {
                    color = SVs{u"30"_sv, u"40"_sv, u"90"_sv, u"100"_sv}[isBg + isBright * 2];
                } else if (s == u"red"_sv) {
                    color = SVs{u"31"_sv, u"41"_sv, u"91"_sv, u"101"_sv}[isBg + isBright * 2];
                } else if (s == u"green"_sv) {
                    color = SVs{u"32"_sv, u"42"_sv, u"92"_sv, u"102"_sv}[isBg + isBright * 2];
                } else if (s == u"yellow"_sv) {
                    color = SVs{u"33"_sv, u"43"_sv, u"93"_sv, u"103"_sv}[isBg + isBright * 2];
                } else if (s == u"blue"_sv) {
                    color = SVs{u"34"_sv, u"44"_sv, u"94"_sv, u"104"_sv}[isBg + isBright * 2];
                } else if (s == u"magenta"_sv) {
                    color = SVs{u"35"_sv, u"45"_sv, u"95"_sv, u"105"_sv}[isBg + isBright * 2];
                } else if (s == u"cyan"_sv) {
                    color = SVs{u"36"_sv, u"46"_sv, u"96"_sv, u"106"_sv}[isBg + isBright * 2];
                } else if (s == u"white"_sv) {
                    color = SVs{u"37"_sv, u"47"_sv, u"97"_sv, u"107"_sv}[isBg + isBright * 2];
                    // styles
                } else if (!isBg && !isBright && s == u"bold"_sv) {
                    color = u"1"_sv;
                } else if (!isBg && !isBright && s == u"dim"_sv) {
                    color = u"2"_sv;
                } else if (!isBg && !isBright && s == u"italic"_sv) {
                    color = u"3"_sv;
                } else if (!isBg && !isBright && s == u"underline"_sv) {
                    color = u"4"_sv;
                } else if (!isBg && !isBright && s == u"reverse"_sv) {
                    color = u"7"_sv;
                } else if (!isBg && !isBright && s == u"strike"_sv) {
                    color = u"9"_sv;
                } else if (!isBg && !isBright && s == u"doubly-underlined"_sv) {
                    color = u"21"_sv;
                } else if (!isBg && !isBright && s == u"overlined"_sv) {
                    color = u"53"_sv;
                    // error
                } else {
                    *ok = false;
                    return result;
                }
            }

            totalLen += color.size() + 1;
        }

        if (hasDefaultColor) {
            totalLen += defaultColor.size() - 2;
        }

        result.reserve(totalLen + 2);

        if (!hasDefaultColor) {
            result += u"\x1b["_sv;
        } else {
            result += defaultColor;
            result.back() = u';';
        }

        /*
         * Concat colors to result
         */
        auto const *trueColorIt = trueColors.constData();
        for (const auto &color : std::as_const(colors)) {
            if (!color.isEmpty()) {
                result += color;
            } else {
                result += trueColorIt->isBg ? u"48;2;"_sv : u"38;2;"_sv;
                result += trueColorIt->sv();
                ++trueColorIt;
            }
            result += u';';
        }

        result.back() = u'm';
    } else {
        result = defaultColor;
    }

    return result;
}

void initCommandLineParser(QCoreApplication &app, QCommandLineParser &parser)
{
    auto tr = [&app](char const *s) {
        return app.translate("KateScriptTester", s);
    };

    const auto translatedFolder = tr("folder");
    const auto translatedOption = tr("option");
    const auto translatedPattern = tr("pattern");
    const auto translatedPlaceholder = tr("character");
    const auto translatedColors = tr("colors");

    parser.setApplicationDescription(tr("Command line utility for testing Kate's command scripts."));
    parser.addPositionalArgument(tr("file.js"), tr("Test files to run. If file.js represents a folder, this is equivalent to `path/*.js`."), tr("file.js..."));

    parser.addOptions({
        // input
        // @{
        {{u"t"_s, u"text"_s}, tr("Files are treated as javascript code rather than file names.")},
        // @}

        // error
        // @{
        {{u"e"_s, u"max-error"_s}, tr("Maximum number of tests that can fail before stopping.")},
        {u"q"_s, tr("Alias of --max-error=1.")},
        {{u"E"_s, u"expected-failure-as-failure"_s}, tr("functions xcmd() and xtest() will always fail.")},
        // @}

        // paths
        // @{
        {{u"s"_s, u"script"_s},
         tr("Shorcut for --command=${script}/commands --command=${script}/indentation --library=${script}/library --file=${script}/files."),
         translatedFolder},
        {{u"c"_s, u"command"_s}, tr("Adds a search folder for loadScript()."), translatedFolder},
        {{u"l"_s, u"library"_s}, tr("Adds a search folder for require() (KTextEditor JS API)."), translatedFolder},
        {{u"r"_s, u"file"_s}, tr("Adds a search folder for read() (KTextEditor JS API)."), translatedFolder},
        {{u"m"_s, u"module"_s}, tr("Adds a search folder for loadModule()."), translatedFolder},
        {{u"I"_s, u"indent-data-test"_s}, tr("Set indentation base directory for indentFiles()."), translatedFolder},
        // @}

        // diff command
        //@{
        {{u"D"_s, u"diff-path"_s}, tr("Path of diff command."), tr("path")},
        {{u"A"_s, u"diff-arg"_s}, tr("Argument for diff command. Call this option several times to set multiple parameters."), translatedOption},
        //@}

        // output format
        //@{
        {{u"d"_s, u"debug"_s},
         tr("Concerning the display of the debug() function. Can be used multiple times to change multiple options.\n"
            "- location: displays the file and line number of the call (enabled by default)\n"
            "- function: displays the name of the function that uses debug() (enabled by default)\n"
            "- stacktrace: show the call stack after the debug message\n"
            "- flush: debug messages are normally buffered and only displayed in case of error. This option removes buffering\n"
            "- extended: debug() can take several parameters of various types such as Array or Object. This behavior is specific and should not be exploited "
            "in final code\n"
            "- no-location: inverse of location\n"
            "- no-function: inverse of function\n"
            "- no-stacktrace: inverse of stacktrace\n"
            "- no-flush: inverse of flush\n"
            "- all: enable all\n"
            "- none: disable all"),
         translatedOption},
        {{u"H"_s, u"hidden-name"_s}, tr("Do not display test names.")},
        {{u"p"_s, u"parade"_s}, tr("Displays all tests run or skipped. By default, only error tests are displayed.")},
        {{u"V"_s, u"verbose"_s}, tr("Displays input and ouput on each tests. By default, only error tests are displayed.")},
        {{u"f"_s, u"format"_s},
         tr("Defines the document text display format:\n"
            "- raw: no transformation\n"
            "- js: display in literal string in javascript format\n"
            "- literal: replaces new lines and tabs with \\n and \\t (default)\n"
            "- placeholder: replaces new lines and tabs with placeholders specified by --newline and --tab\n"
            "- placeholder2: replaces tabs with the placeholder specified by --tab\n"),
         translatedOption},
        {{u"F"_s, u"block-format"_s}, tr("Same as --format, but with block selection text."), translatedOption},
        //@}

        // filter
        //@{
        {{u"k"_s, u"filter"_s}, tr("Only runs tests whose name matches a regular expression."), translatedPattern},
        {u"K"_s, tr("Only runs tests whose name does not matches a regular expression."), translatedPattern},
        //@}

        // placeholders
        //@{
        {{u"T"_s, u"tab"_s},
         tr("Character used to replace a tab in the test display with --format=placeholder. If 2 characters are given, the second corresponds the last "
            "character replaced. --tab='->' with tabWith=4 gives '--->'."),
         translatedPlaceholder},
        {{u"N"_s, u"nl"_s, u"newline"_s}, tr("Character used to replace a new line in the test display with --format=placeholder."), translatedPlaceholder},
        {{u"P"_s, u"placeholders"_s},
         tr("Characters used to represent cursors or selections when the test does not specify any, or when the same character represents more than one thing. "
            "In order:\n"
            "- cursor\n"
            "- selection start\n"
            "- selection end\n"
            "- secondary cursor\n"
            "- secondary selection start\n"
            "- secondary selection end\n"
            "- virtual text"),
         tr("symbols")},
        //@}

        // setup
        //@{
        {{u"b"_s, u"dual"_s},
         tr("Change DUAL_MODE and ALWAYS_DUAL_MODE constants behavior:\n"
            "- noblock: never block selection (equivalent to setConfig({blockSelection=0}))\n"
            "- block: always block selection (equivalent to setConfig({blockSelection=1}))\n"
            "- always-dual: DUAL_MODE = ALWAYS_DUAL_MODE\n"
            "- no-always-dual: ALWAYS_DUAL_MODE = DUAL_MODE\n"
            "- dual: default behavior"),
         tr("arg")},

        {u"B"_s, tr("Alias of --dual=noblock.")},

        {u"arg"_s, tr("Argument add to 'argv' variable in test scripts. Call this option several times to set multiple parameters."), tr("arg")},

        {u"preamble"_s,
         tr("Uses a different preamble than the default. The result must be a function whose first parameter is the global environment, second is 'argv' array "
            "and 'this' refers to the internal object.\n"
            "The {CODE} substring will be replaced by the test code."),
         tr("js-source")},

        {u"print-preamble"_s, tr("Show preamble.")},

        {u"x"_s,
         tr("To ensure that tests are not disrupted by system files, the XDG_DATA_DIRS environment variable is replaced by a non-existent folder.\n"
            "Unfortunately, indentation files are not accessible with indentFiles(), nor are syntax files, according to the KSyntaxHighlighting compilation "
            "method.\n"
            "This option cancels the value replacement.")},

        {u"X"_s, tr("force a value for XDG_DATA_DIRS and ignore -x."), tr("path")},

        {{u"S"_s, u"set-variable"_s},
         tr("Set document variables before running a test file. This is equivalent to `document.setVariable(key, value)` at the start of the file. Call this "
            "option several times to set multiple parameters."),
         tr("key=var")},
        //@}

        // color parameters
        //@{
        {u"no-color"_s, tr("No color on the output")},

        {u"color-reset"_s, tr("Sequence to reset color and style."), translatedColors},
        {u"color-success"_s, tr("Color for success."), translatedColors},
        {u"color-error"_s, tr("Color for error or exception."), translatedColors},
        {u"color-carret"_s, tr("Color for '^~~' under error position."), translatedColors},
        {u"color-debug-marker"_s, tr("Color for 'DEBUG:' and 'PRINT:' prefixes inserted with debug(), print() and printSep()."), translatedColors},
        {u"color-debug-message"_s, tr("Color for message with debug()."), translatedColors},
        {u"color-test-name"_s, tr("Color for name of the test."), translatedColors},
        {u"color-program"_s, tr("Color for program paramater in cmd() / test() and function name in stacktrace."), translatedColors},
        {u"color-file"_s, tr("Color for file name."), translatedColors},
        {u"color-line"_s, tr("Color for line number."), translatedColors},
        {u"color-block-selection-info"_s, tr("Color for [blockSelection=...] in a check."), translatedColors},
        {u"color-label-info"_s, tr("Color for 'input', 'output', 'result' label when it is displayed as information and not as an error."), translatedColors},
        {u"color-cursor"_s, tr("Color for cursor placeholder."), translatedColors},
        {u"color-selection"_s, tr("Color for selection placeholder."), translatedColors},
        {u"color-secondary-cursor"_s, tr("Color for secondary cursor placeholder."), translatedColors},
        {u"color-secondary-selection"_s, tr("Color for secondary selection placeholder."), translatedColors},
        {u"color-block-selection"_s, tr("Color for block selection placeholder."), translatedColors},
        {u"color-in-selection"_s, tr("Style added for text inside a selection."), translatedColors},
        {u"color-virtual-text"_s, tr("Color for virtual text placeholder."), translatedColors},
        {u"color-replacement"_s, tr("Color for text replaced by --format=placeholder."), translatedColors},
        {u"color-text-result"_s, tr("Color for text representing the inputs and outputs."), translatedColors},
        {u"color-result"_s,
         tr("Color added to all colors used to display a result:\n"
            "--color-cursor\n"
            "--color-selection\n"
            "--color-secondary-cursor\n"
            "--color-secondary-selection\n"
            "--color-block-selection\n"
            "--color-virtual-text\n"
            "--color-replacement\n"
            "--color-text-result."),
         translatedColors},
        //@}
    });
}

struct CommandLineParseResult {
    enum class Status { Ok, Error, VersionRequested, HelpRequested };
    Status statusCode = Status::Ok;
    QString errorString = {};
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, ScriptTesterQuery *query)
{
    using Status = CommandLineParseResult::Status;

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    if (!parser.parse(QCoreApplication::arguments()))
        return {Status::Error, parser.errorText()};

    if (parser.isSet(u"v"_s))
        return {Status::VersionRequested};

    if (parser.isSet(u"h"_s))
        return {Status::HelpRequested};

    query->asText = parser.isSet(u"t"_s);

    if (parser.isSet(u"q"_s)) {
        query->executionConfig.maxError = 1;
    }
    if (parser.isSet(u"e"_s)) {
        bool ok = true;
        query->executionConfig.maxError = parser.value(u"e"_s).toInt(&ok);
        if (!ok) {
            return {Status::Error, u"--max-error: invalid number"_s};
        }
    }
    query->executionConfig.xCheckAsFailure = parser.isSet(u"E"_s);

    if (parser.isSet(u"s"_s)) {
        auto addPath = [](QStringList &l, QString path) {
            if (QFile::exists(path)) {
                l.append(path);
            }
        };
        const auto paths = parser.values(u"s"_s);
        for (const auto &path : paths) {
            addPath(query->paths.scripts, path + u"/command"_sv);
            addPath(query->paths.scripts, path + u"/indentation"_sv);
            addPath(query->paths.libraries, path + u"/library"_sv);
            addPath(query->paths.files, path + u"/files"_sv);
        }
    }

    auto setPaths = [&parser](QStringList &l, QString opt) {
        if (parser.isSet(opt)) {
            const auto paths = parser.values(opt);
            for (const auto &path : paths) {
                l.append(path);
            }
        }
    };

    setPaths(query->paths.scripts, u"c"_s);
    setPaths(query->paths.libraries, u"l"_s);
    setPaths(query->paths.files, u"r"_s);
    setPaths(query->paths.modules, u"m"_s);
    query->paths.indentBaseDir = parser.value(u"I"_s);

    if (parser.isSet(u"d"_s)) {
        const auto value = parser.value(u"d"_s);
        if (value == u"location"_sv) {
            query->format.debugOptions |= DebugOption::WriteLocation;
        } else if (value == u"function"_sv) {
            query->format.debugOptions |= DebugOption::WriteFunction;
        } else if (value == u"stacktrace"_sv) {
            query->format.debugOptions |= DebugOption::WriteStackTrace;
        } else if (value == u"flush"_sv) {
            query->format.debugOptions |= DebugOption::ForceFlush;
        } else if (value == u"extended"_sv) {
            query->extendedDebug = true;
        } else if (value == u"no-location"_sv) {
            query->format.debugOptions.setFlag(DebugOption::WriteLocation, false);
        } else if (value == u"no-function"_sv) {
            query->format.debugOptions.setFlag(DebugOption::WriteFunction, false);
        } else if (value == u"no-stacktrace"_sv) {
            query->format.debugOptions.setFlag(DebugOption::WriteStackTrace, false);
        } else if (value == u"no-flush"_sv) {
            query->format.debugOptions.setFlag(DebugOption::ForceFlush, false);
        } else if (value == u"no-extended"_sv) {
            query->extendedDebug = false;
        } else if (value == u"all"_sv) {
            query->extendedDebug = true;
            query->format.debugOptions = DebugOption::WriteLocation | DebugOption::WriteFunction | DebugOption::WriteStackTrace | DebugOption::ForceFlush;
        } else if (value == u"none"_sv) {
            query->extendedDebug = false;
            query->format.debugOptions = {};
        } else {
            return {Status::Error, u"--debug: invalid value"_s};
        }
    }

    if (parser.isSet(u"H"_s)) {
        query->format.testFormatOptions |= TestFormatOption::HiddenTestName;
    }
    if (parser.isSet(u"p"_s)) {
        query->format.testFormatOptions |= TestFormatOption::AlwaysWriteLocation;
    }
    if (parser.isSet(u"V"_s)) {
        query->format.testFormatOptions |= TestFormatOption::AlwaysWriteInputOutput;
    }

    auto setFormat = [&parser](TextFormat &textFormat, const QString &opt) {
        if (parser.isSet(opt)) {
            const auto value = parser.value(opt);
            if (value == u"raw"_sv) {
                textFormat = TextFormat::Raw;
            } else if (value == u"js"_sv) {
                textFormat = TextFormat::EscapeForDoubleQuote;
            } else if (value == u"placeholder"_sv) {
                textFormat = TextFormat::ReplaceNewLineAndTabWithPlaceholder;
            } else if (value == u"placeholder2"_sv) {
                textFormat = TextFormat::ReplaceTabWithPlaceholder;
            } else if (value == u"literal"_sv) {
                textFormat = TextFormat::ReplaceNewLineAndTabWithLiteral;
            } else {
                return false;
            }
        }
        return true;
    };
    if (!setFormat(query->format.documentTextFormat, u"f"_s)) {
        return {Status::Error, u"--format: invalid value"_s};
    }
    if (!setFormat(query->format.documentTextFormatWithBlockSelection, u"F"_s)) {
        return {Status::Error, u"--block-format: invalid value"_s};
    }

    auto setPattern = [&parser, &query](QString opt, PatternType patternType) {
        if (parser.isSet(opt)) {
            query->executionConfig.pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::UseUnicodePropertiesOption);
            query->executionConfig.pattern.setPattern(parser.value(opt));
            if (!query->executionConfig.pattern.isValid()) {
                return false;
            }
            query->executionConfig.patternType = patternType;
        }
        return true;
    };

    if (!setPattern(u"k"_s, PatternType::Include)) {
        return {Status::Error, u"-k: "_sv + query->executionConfig.pattern.errorString()};
    }
    if (!setPattern(u"K"_s, PatternType::Exclude)) {
        return {Status::Error, u"-K: "_sv + query->executionConfig.pattern.errorString()};
    }

    if (parser.isSet(u"T"_s)) {
        const auto tab = parser.value(u"T"_s);
        if (tab.size() == 0) {
            query->format.textReplacement.tab1 = defaultTextReplacement.tab1;
            query->format.textReplacement.tab2 = defaultTextReplacement.tab2;
        } else {
            query->format.textReplacement.tab1 = tab[0];
            query->format.textReplacement.tab2 = (tab.size() == 1) ? query->format.textReplacement.tab1 : tab[1];
        }
    }

    auto getChar = [](const QString &str, qsizetype i, QChar c = QChar()) {
        return str.size() > i ? str[i] : c;
    };

    if (parser.isSet(u"N"_s)) {
        const auto nl = parser.value(u"N"_s);
        query->format.textReplacement.newLine = getChar(nl, 0, query->format.textReplacement.newLine);
    }

    if (parser.isSet(u"P"_s)) {
        const auto symbols = parser.value(u"P"_s);
        auto &ph = query->format.fallbackPlaceholders;
        ph.cursor = getChar(symbols, 0, defaultFallbackPlaceholders.cursor);
        ph.selectionStart = getChar(symbols, 1, defaultFallbackPlaceholders.selectionStart);
        ph.selectionEnd = getChar(symbols, 2, defaultFallbackPlaceholders.selectionEnd);
        ph.secondaryCursor = getChar(symbols, 3, defaultFallbackPlaceholders.secondaryCursor);
        ph.secondarySelectionStart = getChar(symbols, 4, defaultFallbackPlaceholders.secondarySelectionStart);
        ph.secondarySelectionEnd = getChar(symbols, 5, defaultFallbackPlaceholders.secondarySelectionEnd);
        ph.virtualText = getChar(symbols, 6, defaultFallbackPlaceholders.virtualText);
    }

    if (parser.isSet(u"B"_s)) {
        query->dualMode = DualMode::NoBlockSelection;
    }

    if (parser.isSet(u"b"_s)) {
        const auto mode = parser.value(u"b"_s);
        if (mode == u"noblock"_sv) {
            query->dualMode = DualMode::NoBlockSelection;
        } else if (mode == u"block"_sv) {
            query->dualMode = DualMode::BlockSelection;
        } else if (mode == u"always-dual"_sv) {
            query->dualMode = DualMode::DualIsAlwaysDual;
        } else if (mode == u"no-always-dual"_sv) {
            query->dualMode = DualMode::AlwaysDualIsDual;
        } else if (mode == u"dual"_sv) {
            query->dualMode = DualMode::Dual;
        } else {
            return {Status::Error, u"--dual: invalid value"_s};
        }
        query->dualMode = DualMode::NoBlockSelection;
    }

    query->argv = parser.values(u"arg"_s);

    if (parser.isSet(u"preamble"_s)) {
        query->preamble = parser.value(u"preamble"_s);
    }

    query->showPreamble = parser.isSet(u"print-preamble"_s);

    if (parser.isSet(u"X"_s)) {
        query->xdgDataDirs = parser.value(u"X"_s).toUtf8();
        query->restoreXdgDataDirs = true;
    } else {
        query->restoreXdgDataDirs = parser.isSet(u"x"_s);
    }

    if (parser.isSet(u"S"_s)) {
        const auto variables = parser.values(u"S"_s);
        query->variables.resize(variables.size());
        auto it = query->variables.begin();
        for (const auto &kv : variables) {
            auto pos = QStringView(kv).indexOf(u'=');
            if (pos >= 0) {
                it->key = kv.sliced(0, pos);
                it->value = kv.sliced(pos + 1);
            } else {
                it->key = kv;
            }
            ++it;
        }
    }

    query->diff.path = parser.isSet(u"D"_s) ? parser.value(u"D"_s) : u"diff"_s;

    const bool noColor = parser.isSet(u"no-color"_s);

    if (parser.isSet(u"A"_s)) {
        query->diff.args = parser.values(u"A"_s);
    } else {
        query->diff.args.push_back(u"-u"_s);
        if (!noColor) {
            query->diff.args.push_back(u"--color"_s);
        }
    }

    if (noColor) {
        query->format.colors.reset.clear();
        query->format.colors.success.clear();
        query->format.colors.error.clear();
        query->format.colors.carret.clear();
        query->format.colors.debugMarker.clear();
        query->format.colors.debugMsg.clear();
        query->format.colors.testName.clear();
        query->format.colors.program.clear();
        query->format.colors.fileName.clear();
        query->format.colors.lineNumber.clear();
        query->format.colors.labelInfo.clear();
        query->format.colors.blockSelectionInfo.clear();
        query->format.colors.cursor.clear();
        query->format.colors.selection.clear();
        query->format.colors.secondaryCursor.clear();
        query->format.colors.secondarySelection.clear();
        query->format.colors.blockSelection.clear();
        query->format.colors.inSelection.clear();
        query->format.colors.virtualText.clear();
        query->format.colors.result.clear();
        query->format.colors.resultReplacement.clear();
    } else {
        QString defaultResultColor;
        QString optWithError;
        auto setColor = [&](QString &color, QString opt) {
            if (parser.isSet(opt)) {
                bool ok = true;
                color = toANSIColor(parser.value(opt), defaultResultColor, &ok);
                if (!ok) {
                    optWithError = opt;
                }
                return true;
            }
            return false;
        };

        setColor(query->format.colors.reset, u"color-reset"_s);
        setColor(query->format.colors.success, u"color-success"_s);
        setColor(query->format.colors.error, u"color-error"_s);
        setColor(query->format.colors.carret, u"color-carret"_s);
        setColor(query->format.colors.debugMarker, u"color-debug-marker"_s);
        setColor(query->format.colors.debugMsg, u"color-debug-message"_s);
        setColor(query->format.colors.testName, u"color-test-name"_s);
        setColor(query->format.colors.program, u"color-program"_s);
        setColor(query->format.colors.fileName, u"color-file"_s);
        setColor(query->format.colors.lineNumber, u"color-line"_s);
        setColor(query->format.colors.labelInfo, u"color-label-info"_s);
        setColor(query->format.colors.blockSelectionInfo, u"color-block-selection-info"_s);
        setColor(query->format.colors.inSelection, u"color-in-selection"_s);

        if (!setColor(defaultResultColor, u"color-result"_s)) {
            defaultResultColor = u"\x1b[40m"_s;
        }
        const bool hasDefault = defaultResultColor.size();
        const QStringView ansiBg = QStringView(defaultResultColor.constData(), hasDefault ? defaultResultColor.size() - 1 : 0);
        if (!setColor(query->format.colors.cursor, u"color-cursor"_s) && hasDefault) {
            query->format.colors.cursor = ansiBg % u";1;33m"_sv;
        }
        if (!setColor(query->format.colors.selection, u"color-selection"_s) && hasDefault) {
            query->format.colors.selection = ansiBg % u";1;33m"_sv;
        }
        if (!setColor(query->format.colors.secondaryCursor, u"color-secondary-cursor"_s) && hasDefault) {
            query->format.colors.secondaryCursor = ansiBg % u";33m"_sv;
        }
        if (!setColor(query->format.colors.secondarySelection, u"color-secondary-selection"_s) && hasDefault) {
            query->format.colors.secondarySelection = ansiBg % u";33m"_sv;
        }
        if (!setColor(query->format.colors.blockSelection, u"color-block-selection"_s) && hasDefault) {
            query->format.colors.blockSelection = ansiBg % u";37m"_sv;
        }
        if (!setColor(query->format.colors.virtualText, u"color-virtual-text"_s) && hasDefault) {
            query->format.colors.virtualText = ansiBg % u";37m"_sv;
        }
        if (!setColor(query->format.colors.result, u"color-text-result"_s) && hasDefault) {
            query->format.colors.result = defaultResultColor;
        }
        if (!setColor(query->format.colors.resultReplacement, u"color-replacement"_s) && hasDefault) {
            query->format.colors.resultReplacement = ansiBg % u";36m"_sv;
        }

        if (!optWithError.isEmpty()) {
            return {Status::Error, u"--"_sv % optWithError % u": invalid color"_sv};
        }
    }

    query->fileNames = parser.positionalArguments();

    return {Status::Ok};
}

void addTextStyleProperties(QJSValue &obj)
{
    using TextStyle = KSyntaxHighlighting::Theme::TextStyle;
    obj.setProperty(u"dsNormal"_s, TextStyle::Normal);
    obj.setProperty(u"dsKeyword"_s, TextStyle::Keyword);
    obj.setProperty(u"dsFunction"_s, TextStyle::Function);
    obj.setProperty(u"dsVariable"_s, TextStyle::Variable);
    obj.setProperty(u"dsControlFlow"_s, TextStyle::ControlFlow);
    obj.setProperty(u"dsOperator"_s, TextStyle::Operator);
    obj.setProperty(u"dsBuiltIn"_s, TextStyle::BuiltIn);
    obj.setProperty(u"dsExtension"_s, TextStyle::Extension);
    obj.setProperty(u"dsPreprocessor"_s, TextStyle::Preprocessor);
    obj.setProperty(u"dsAttribute"_s, TextStyle::Attribute);
    obj.setProperty(u"dsChar"_s, TextStyle::Char);
    obj.setProperty(u"dsSpecialChar"_s, TextStyle::SpecialChar);
    obj.setProperty(u"dsString"_s, TextStyle::String);
    obj.setProperty(u"dsVerbatimString"_s, TextStyle::VerbatimString);
    obj.setProperty(u"dsSpecialString"_s, TextStyle::SpecialString);
    obj.setProperty(u"dsImport"_s, TextStyle::Import);
    obj.setProperty(u"dsDataType"_s, TextStyle::DataType);
    obj.setProperty(u"dsDecVal"_s, TextStyle::DecVal);
    obj.setProperty(u"dsBaseN"_s, TextStyle::BaseN);
    obj.setProperty(u"dsFloat"_s, TextStyle::Float);
    obj.setProperty(u"dsConstant"_s, TextStyle::Constant);
    obj.setProperty(u"dsComment"_s, TextStyle::Comment);
    obj.setProperty(u"dsDocumentation"_s, TextStyle::Documentation);
    obj.setProperty(u"dsAnnotation"_s, TextStyle::Annotation);
    obj.setProperty(u"dsCommentVar"_s, TextStyle::CommentVar);
    obj.setProperty(u"dsRegionMarker"_s, TextStyle::RegionMarker);
    obj.setProperty(u"dsInformation"_s, TextStyle::Information);
    obj.setProperty(u"dsWarning"_s, TextStyle::Warning);
    obj.setProperty(u"dsAlert"_s, TextStyle::Alert);
    obj.setProperty(u"dsOthers"_s, TextStyle::Others);
    obj.setProperty(u"dsError"_s, TextStyle::Error);
}

/**
 * Timestamp in milliseconds.
 */
static qsizetype timeNowInMs()
{
    auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
}

QtMessageHandler originalHandler = nullptr;
/**
 * Remove messages from kf.sonnet.core when no backend is found.
 */
static void filterMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (originalHandler && context.category != std::string_view("kf.sonnet.core")) {
        originalHandler(type, context, msg);
    }
}

} // anonymous namespace

int main(int ac, char **av)
{
    ScriptTesterQuery query;
    query.xdgDataDirs = qgetenv("XDG_DATA_DIRS");

    qputenv("QT_QPA_PLATFORM", "offscreen"); // equivalent to `-platform offscreen` in cli
    // Set an unknown folder for XDG_DATA_DIRS so that KateScriptManager::collect()
    // does not retrieve system scripts.
    // If the variable is empty, QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)
    // returns /usr/local/share and /usr/share
    qputenv("XDG_DATA_DIRS", "/XDG_DATA_DIRS_unknown_folder");
    QStandardPaths::setTestModeEnabled(true);
    originalHandler = qInstallMessageHandler(filterMessageOutput);

    /*
     * App
     */

    QApplication app(ac, av);
    QCoreApplication::setApplicationName(u"katescripttester"_s);
    QCoreApplication::setOrganizationDomain(u"kde.org"_s);
    QCoreApplication::setOrganizationName(u"KDE"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(KTEXTEDITOR_VERSION_STRING));

    /*
     * Cli parser
     */

    QCommandLineParser parser;
    initCommandLineParser(app, parser);

    using Status = CommandLineParseResult::Status;
    CommandLineParseResult parseResult = parseCommandLine(parser, &query);
    switch (parseResult.statusCode) {
    case Status::Ok:
        if (!query.showPreamble && query.fileNames.isEmpty()) {
            std::fputs("No test file specified.\nUse -h / --help for more details.\n", stderr);
            return 1;
        }
        break;
    case Status::Error:
        std::fputs(qPrintable(parseResult.errorString), stderr);
        std::fputs("\nUse -h / --help for more details.\n", stderr);
        return 2;
    case Status::VersionRequested:
        parser.showVersion();
        return 0;
    case Status::HelpRequested:
        std::fputs(qPrintable(parser.helpText()), stdout);
        std::fputs(R"(
Colors:
  Comma-separated list of values:
  - color name: black, green, yellow, blue, magenta, cyan, white
  - bright color name: bright-${color name}
  - rgb: #fff or #ffffff (use trueColor sequence)
  - background color: bg=${color name} or bg=bright-${color name} bg=${rgb}
  - style: bold, dim, italic, underline, reverse, strike, doubly-underline, overlined
  - ANSI sequence: number sequence with optional ';'
)",
                   stdout);
        return 0;
    }

    /*
     * Init Preamble
     */

    // no new line so that the lines indicated by evaluate correspond to the user code
    auto jsInjectionStart1 =
        u"(function(env, argv){"
        u"const TestFramework = this.loadModule(':/ktexteditor/scripttester/testframework.js');"
        u"const {REUSE_LAST_INPUT, REUSE_LAST_EXPECTED_OUTPUT} = TestFramework;"
        u"const AS_INPUT = TestFramework.EXPECTED_OUTPUT_AS_INPUT;"
        u"var {calleeWrapper, config, print, printSep, testCase, sequence, withInput, keys,"
        u"     indentFiles, test, xtest, eqvTrue, eqvFalse, eqTrue, eqFalse, error, errorMsg,"
        u"     errorType, hasError, eqv, is, eq, ne, lt, gt, le, ge, cmd, xcmd, type, xtype"
        u"    } = TestFramework;"
        u"var c = TestFramework.sanitizeTag;"
        u"var lazyfn = (fn, ...args) => new TestFramework.LazyFunc(fn, ...args);"
        u"var fn = lazyfn;"
        u"var lazyarg = (arg) => new TestFramework.LazyArg(arg);"
        u"var arg = lazyarg;"
        u"var loadScript = this.loadScript;"
        u"var loadModule = this.loadModule;"
        u"var paste = (str) => this.paste(str);"
        u"env.editor = TestFramework.editor;" // init editor
        u"var document = calleeWrapper('document', env.document);"
        u"var editor = calleeWrapper('editor', env.editor);"
        u"var view = calleeWrapper('view', env.view);"
        u""_sv;
    auto debugSetup = query.extendedDebug ? u"debug = testFramework.debug;"_sv : u""_sv;
    // clang-format off
    auto dualModeSetup = query.dualMode == DualMode::Dual
            ? u"const DUAL_MODE = TestFramework.DUAL_MODE;"
              u"const ALWAYS_DUAL_MODE = TestFramework.ALWAYS_DUAL_MODE;"_sv
        : query.dualMode == DualMode::NoBlockSelection
            ? u"const DUAL_MODE = 0;"
              u"const ALWAYS_DUAL_MODE = 0;"_sv
        : query.dualMode == DualMode::BlockSelection
            ? u"const DUAL_MODE = 1;"
              u"const ALWAYS_DUAL_MODE = 1;"_sv
        : query.dualMode == DualMode::DualIsAlwaysDual
            ? u"const DUAL_MODE = TestFramework.ALWAYS_DUAL_MODE;"
              u"const ALWAYS_DUAL_MODE = TestFramework.ALWAYS_DUAL_MODE;"_sv
        // : query.dualMode == DualMode::AlwaysDualIsDual
            : u"const DUAL_MODE = TestFramework.DUAL_MODE;"
              u"const ALWAYS_DUAL_MODE = TestFramework.DUAL_MODE;"_sv;
    // clang-format on
    auto jsInjectionStart2 =
        u"var kbd = TestFramework.init(this, env, DUAL_MODE);"
        u"try { void function(){"_sv;
    auto jsInjectionEnd =
        u"\n}() }"
        u"catch (e) {"
        u"if (e !== TestFramework.STOP_CASE_ERROR) {"
        u"throw e;"
        u"}"
        u"}"
        u"})\n"
        u""_sv;

    if (!query.preamble.isEmpty()) {
        const auto pattern = u"{CODE}"_sv;
        const QStringView preamble = query.preamble;
        auto pos = preamble.indexOf(pattern);
        if (pos <= -1) {
            std::fputs("missing {CODE} with --preamble\n", stderr);
            return 2;
        }
        jsInjectionStart1 = preamble.sliced(0, pos);
        jsInjectionEnd = preamble.sliced(pos + pattern.size());
        jsInjectionStart2 = QStringView();
        dualModeSetup = QStringView();
        debugSetup = QStringView();
    }

    auto makeProgram = [&](QStringView source) -> QString {
        return jsInjectionStart1 % debugSetup % dualModeSetup % jsInjectionStart2 % u'\n' % source % jsInjectionEnd;
    };

    if (query.showPreamble) {
        std::fputs(qPrintable(makeProgram(u"{CODE}"_sv)), stdout);
        return 0;
    }

    if (query.restoreXdgDataDirs) {
        qputenv("XDG_DATA_DIRS", query.xdgDataDirs);
    }

    /*
     * KTextEditor objects
     */

    KTextEditor::DocumentPrivate doc(true, false);
    KTextEditor::ViewPrivate view(&doc, nullptr);

    QJSEngine engine;

    KateScriptView viewObj(&engine);
    viewObj.setView(&view);

    KateScriptDocument docObj(&engine);
    docObj.setDocument(&doc);

    /*
     * ScriptTester object
     */

    QFile output;
    output.open(stderr, QIODevice::WriteOnly);
    ScriptTester scriptTester(&output, query.format, query.paths, query.executionConfig, query.diff, defaultPlaceholder, &engine, &doc, &view);

    /*
     * JS API
     */

    QJSValue globalObject = engine.globalObject();
    QJSValue functions = engine.newQObject(&scriptTester);

    globalObject.setProperty(u"read"_s, functions.property(u"read"_s));
    globalObject.setProperty(u"require"_s, functions.property(u"require"_s));
    globalObject.setProperty(u"debug"_s, functions.property(u"debug"_s));

    globalObject.setProperty(u"view"_s, engine.newQObject(&viewObj));
    globalObject.setProperty(u"document"_s, engine.newQObject(&docObj));
    // editor object is defined later in testframwork.js

    addTextStyleProperties(globalObject);

    // View and Document expose JS Range objects in the API, which will fail to work
    // if Range is not included. range.js includes cursor.js
    scriptTester.require(u"range.js"_s);

    engine.evaluate(QStringLiteral(
        // translation functions (return untranslated text)
        "function i18n(text, ...arg) { return text; }\n"
        "function i18nc(context, text, ...arg) { return text; }\n"
        "function i18np(singular, plural, number, ...arg) { return number > 1 ? plural : singular; }\n"
        "function i18ncp(context, singular, plural, number, ...arg) { return number > 1 ? plural : singular; }\n"
        // editor object, defined in testframwork.js and built before running a test
        "var editor = undefined;"));

    /*
     * Run function
     */

    auto jsArgv = engine.newArray(query.argv.size());
    for (quint32 i = 0; i < query.argv.size(); ++i) {
        jsArgv.setProperty(i, QJSValue(query.argv.constData()[i]));
    }

    const auto &colors = query.format.colors;

    qsizetype delayInMs = 0;
    bool resetConfig = false;
    auto runProgram = [&](const QString &fileName, const QString &source) {
        auto result = engine.evaluate(makeProgram(source), fileName, 0);
        if (!result.isError()) {
            if (resetConfig) {
                scriptTester.resetConfig();
            }
            resetConfig = true;

            for (const auto &variable : std::as_const(query.variables)) {
                doc.setVariable(variable.key, variable.value);
            }

            const auto start = timeNowInMs();
            result = result.callWithInstance(functions, {globalObject, jsArgv});
            delayInMs += timeNowInMs() - start;
            if (!result.isError()) {
                return;
            }
        }

        scriptTester.incrementError();
        scriptTester.stream() << colors.error << result.toString() << colors.reset << u'\n';
        scriptTester.writeException(result, u"| "_sv);
        scriptTester.stream().flush();
    };

    QFile file;
    auto runJsFile = [&](const QString &fileName) {
        file.setFileName(fileName);
        bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
        const QString content = ok ? QTextStream(&file).readAll() : QString();
        ok = (ok && file.error() == QFileDevice::NoError);
        if (!ok) {
            scriptTester.incrementError();
            scriptTester.stream() << colors.fileName << fileName << colors.reset << ": "_L1 << colors.error << file.errorString() << colors.reset << u'\n';
            scriptTester.stream().flush();
        }
        file.close();
        file.unsetError();
        if (ok) {
            runProgram(fileName, content);
        }
    };

    /*
     * Read file and run
     */

    const auto &fileNames = query.fileNames;
    for (const auto &fileName : fileNames) {
        if (query.asText) {
            runProgram(u"file%1.js"_s.arg(&fileName - fileNames.data() + 1), fileName);
        } else if (!QFileInfo(fileName).isDir()) {
            runJsFile(fileName);
        } else {
            QDirIterator it(fileName, {u"*.js"_s}, QDir::Files);
            while (it.hasNext() && !scriptTester.hasTooManyErrors()) {
                runJsFile(it.next());
            }
        }

        if (scriptTester.hasTooManyErrors()) {
            break;
        }
    }

    /*
     * Result
     */

    if (scriptTester.hasTooManyErrors()) {
        scriptTester.stream() << colors.error << "Too many error"_L1 << colors.reset << u'\n';
    }

    scriptTester.writeSummary();
    scriptTester.stream() << "  Duration: "_L1 << delayInMs << "ms\n"_L1;
    scriptTester.stream().flush();

    return scriptTester.countError() ? 1 : 0;
}
