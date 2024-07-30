/*
    SPDX-FileCopyrightText: 2003-2005 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Charles Samuels <charles@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecmds.h"

#include "kateautoindent.h"
#include "katecmd.h"
#include "katedocument.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "katesyntaxmanager.h"
#include "kateview.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QRegularExpression>

// BEGIN CoreCommands
KateCommands::CoreCommands *KateCommands::CoreCommands::m_instance = nullptr;

// this returns whether the string s could be converted to
// a bool value, one of on|off|1|0|true|false. the argument val is
// set to the extracted value in case of success
static bool getBoolArg(const QString &t, bool *val)
{
    bool res(false);
    QString s = t.toLower();
    res = (s == QLatin1String("on") || s == QLatin1String("1") || s == QLatin1String("true"));
    if (res) {
        *val = true;
        return true;
    }
    res = (s == QLatin1String("off") || s == QLatin1String("0") || s == QLatin1String("false"));
    if (res) {
        *val = false;
        return true;
    }
    return false;
}

bool KateCommands::CoreCommands::help(KTextEditor::View *, const QString &cmd, QString &msg)
{
    QString realcmd = cmd.trimmed();
    if (realcmd == QLatin1String("indent")) {
        msg = i18n(
            "<p>indent</p>"
            "<p>Indents the selected lines or the current line</p>");
        return true;
    } else if (realcmd == QLatin1String("unindent")) {
        msg = i18n(
            "<p>unindent</p>"
            "<p>Unindents the selected lines or current line.</p>");
        return true;
    } else if (realcmd == QLatin1String("cleanindent")) {
        msg = i18n(
            "<p>cleanindent</p>"
            "<p>Cleans up the indentation of the selected lines or current line according to the indentation settings in the document. </p>");
        return true;
    } else if (realcmd == QLatin1String("comment")) {
        msg = i18n(
            "<p>comment</p>"
            "<p>Inserts comment markers to make the selection or selected lines or current line a comment according to the text format as defined by the "
            "syntax highlight definition for the document.</p>");
        return true;
    } else if (realcmd == QLatin1String("uncomment")) {
        msg = i18n(
            "<p>uncomment</p>"
            "<p>Removes comment markers from the selection or selected lines or current line according to the text format as defined by the syntax highlight "
            "definition for the document.</p>");
        return true;
    } else if (realcmd == QLatin1String("goto")) {
        msg = i18n(
            "<p>goto <b>line number</b></p>"
            "<p>This command navigates to the specified line number.</p>");
        return true;
    } else if (realcmd == QLatin1String("set-indent-pasted-text")) {
        msg = i18n(
            "<p>set-indent-pasted-text <b>enable</b></p>"
            "<p>If enabled, indentation of text pasted from the clipboard is adjusted using the current indenter.</p>"
            "<p>Possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("kill-line")) {
        msg = i18n("Deletes the current line.");
        return true;
    } else if (realcmd == QLatin1String("set-tab-width")) {
        msg = i18n(
            "<p>set-tab-width <b>width</b></p>"
            "<p>Sets the tab width to the number <b>width</b></p>");
        return true;
    } else if (realcmd == QLatin1String("set-replace-tab")) {
        msg = i18n(
            "<p>set-replace-tab <b>enable</b></p>"
            "<p>If enabled, tabs are replaced with spaces as you type.</p>"
            "<p>Possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-show-tabs")) {
        msg = i18n(
            "<p>set-show-tabs <b>enable</b></p>"
            "<p>If enabled, TAB characters and trailing whitespace will be visualized by a small dot.</p>"
            "<p>Possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-remove-trailing-spaces")) {
        msg = i18n(
            "<p>set-remove-trailing-spaces <b>mode</b></p>"
            "<p>Removes the trailing spaces in the document depending on the <b>mode</b>.</p>"
            "<p>Possible values:"
            "<ul>"
            "<li><b>none</b>: never remove trailing spaces.</li>"
            "<li><b>modified</b>: remove trailing spaces only of modified lines.</li>"
            "<li><b>all</b>: remove trailing spaces in the entire document.</li>"
            "</ul></p>");
        return true;
    } else if (realcmd == QLatin1String("set-indent-width")) {
        msg = i18n(
            "<p>set-indent-width <b>width</b></p>"
            "<p>Sets the indentation width to the number <b>width</b>. Used only if you are indenting with spaces.</p>");
        return true;
    } else if (realcmd == QLatin1String("set-indent-mode")) {
        msg = i18n(
            "<p>set-indent-mode <b>mode</b></p>"
            "<p>The mode parameter is a value as seen in the Tools - Indentation menu</p>");
        return true;
    } else if (realcmd == QLatin1String("set-auto-indent")) {
        msg = i18n(
            "<p>set-auto-indent <b>enable</b></p>"
            "<p>Enable or disable autoindentation.</p>"
            "<p>possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-line-numbers")) {
        msg = i18n(
            "<p>set-line-numbers <b>enable</b></p>"
            "<p>Sets the visibility of the line numbers pane.</p>"
            "<p> possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-folding-markers")) {
        msg = i18n(
            "<p>set-folding-markers <b>enable</b></p>"
            "<p>Sets the visibility of the folding markers pane.</p>"
            "<p> possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-icon-border")) {
        msg = i18n(
            "<p>set-icon-border <b>enable</b></p>"
            "<p>Sets the visibility of the icon border.</p>"
            "<p> possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-word-wrap")) {
        msg = i18n(
            "<p>set-word-wrap <b>enable</b></p>"
            "<p>Enables dynamic word wrap according to <b>enable</b></p>"
            "<p> possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-word-wrap-column")) {
        msg = i18n(
            "<p>set-word-wrap-column <b>width</b></p>"
            "<p>Sets the line width for hard wrapping to <b>width</b>. This is used if you are having your text wrapped automatically.</p>");
        return true;
    } else if (realcmd == QLatin1String("set-replace-tabs-save")) {
        msg = i18n(
            "<p>set-replace-tabs-save <b>enable</b></p>"
            "<p>When enabled, tabs will be replaced with whitespace whenever the document is saved.</p>"
            "<p> possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("set-highlight")) {
        msg = i18n(
            "<p>set-highlight <b>highlight</b></p>"
            "<p>Sets the syntax highlighting system for the document. The argument must be a valid highlight name, as seen in the Tools → Highlighting menu. "
            "This command provides an autocompletion list for its argument.</p>");
        return true;
    } else if (realcmd == QLatin1String("set-mode")) {
        msg = i18n(
            "<p>set-mode <b>mode</b></p>"
            "<p>Sets the mode as seen in Tools - Mode</p>");
        return true;
    } else if (realcmd == QLatin1String("set-show-indent")) {
        msg = i18n(
            "<p>set-show-indent <b>enable</b></p>"
            "<p>If enabled, indentation will be visualized by a vertical dotted line.</p>"
            "<p> possible true values: 1 on true<br/>"
            "possible false values: 0 off false</p>");
        return true;
    } else if (realcmd == QLatin1String("print")) {
        msg = i18n("<p>Open the Print dialog to print the current document.</p>");
        return true;
    } else {
        return false;
    }
}

bool KateCommands::CoreCommands::exec(KTextEditor::View *view, const QString &_cmd, QString &errorMsg, const KTextEditor::Range &range)
{
#define KCC_ERR(s)                                                                                                                                             \
    {                                                                                                                                                          \
        errorMsg = s;                                                                                                                                          \
        return false;                                                                                                                                          \
    }
    // cast it hardcore, we know that it is really a kateview :)
    KTextEditor::ViewPrivate *v = static_cast<KTextEditor::ViewPrivate *>(view);

    if (!v) {
        KCC_ERR(i18n("Could not access view"));
    }

    // create a list of args
    QStringList args(_cmd.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts));
    QString cmd(args.takeFirst());

    // ALL commands that takes no arguments.
    if (cmd == QLatin1String("indent")) {
        if (range.isValid()) {
            v->doc()->editStart();
            for (int line = range.start().line(); line <= range.end().line(); line++) {
                v->doc()->indent(KTextEditor::Range(line, 0, line, 0), 1);
            }
            v->doc()->editEnd();
        } else {
            v->indent();
        }
        return true;
    } else if (cmd == QLatin1String("unindent")) {
        if (range.isValid()) {
            v->doc()->editStart();
            for (int line = range.start().line(); line <= range.end().line(); line++) {
                v->doc()->indent(KTextEditor::Range(line, 0, line, 0), -1);
            }
            v->doc()->editEnd();
        } else {
            v->unIndent();
        }
        return true;
    } else if (cmd == QLatin1String("cleanindent")) {
        if (range.isValid()) {
            v->doc()->editStart();
            for (int line = range.start().line(); line <= range.end().line(); line++) {
                v->doc()->indent(KTextEditor::Range(line, 0, line, 0), 0);
            }
            v->doc()->editEnd();
        } else {
            v->cleanIndent();
        }
        return true;
    } else if (cmd == QLatin1String("fold")) {
        return (v->textFolding().newFoldingRange(range.isValid() ? range : v->selectionRange(), Kate::TextFolding::Persistent | Kate::TextFolding::Folded)
                != -1);
    } else if (cmd == QLatin1String("tfold")) {
        return (v->textFolding().newFoldingRange(range.isValid() ? range : v->selectionRange(), Kate::TextFolding::Folded) != -1);
    } else if (cmd == QLatin1String("unfold")) {
        QList<QPair<qint64, Kate::TextFolding::FoldingRangeFlags>> startingRanges = v->textFolding().foldingRangesStartingOnLine(v->cursorPosition().line());
        bool unfolded = false;
        for (int i = 0; i < startingRanges.size(); ++i) {
            if (startingRanges[i].second & Kate::TextFolding::Folded) {
                unfolded = v->textFolding().unfoldRange(startingRanges[i].first) || unfolded;
            }
        }
        return unfolded;
    } else if (cmd == QLatin1String("comment")) {
        if (range.isValid()) {
            v->doc()->editStart();
            for (int line = range.start().line(); line <= range.end().line(); line++) {
                v->doc()->comment(v, line, 0, KTextEditor::DocumentPrivate::Comment);
            }
            v->doc()->editEnd();
        } else {
            v->comment();
        }
        return true;
    } else if (cmd == QLatin1String("uncomment")) {
        if (range.isValid()) {
            v->doc()->editStart();
            for (int line = range.start().line(); line <= range.end().line(); line++) {
                v->doc()->comment(v, line, 0, KTextEditor::DocumentPrivate::UnComment);
            }
            v->doc()->editEnd();
        } else {
            v->uncomment();
        }
        return true;
    } else if (cmd == QLatin1String("kill-line")) {
        if (range.isValid()) {
            v->doc()->editStart();
            for (int line = range.start().line(); line <= range.end().line(); line++) {
                v->doc()->removeLine(range.start().line());
            }
            v->doc()->editEnd();
        } else {
            v->killLine();
        }
        return true;
    } else if (cmd == QLatin1String("print")) {
        v->print();
        return true;
    }

    // ALL commands that take a string argument
    else if (cmd == QLatin1String("set-indent-mode") || cmd == QLatin1String("set-highlight") || cmd == QLatin1String("set-mode")) {
        // need at least one item, otherwise args.first() crashes
        if (args.isEmpty()) {
            KCC_ERR(i18n("Missing argument. Usage: %1 <value>", cmd));
        }

        if (cmd == QLatin1String("set-indent-mode")) {
            v->doc()->config()->setIndentationMode(args.join(QLatin1Char(' ')));
            v->doc()->rememberUserDidSetIndentationMode();
            return true;
        } else if (cmd == QLatin1String("set-highlight")) {
            if (v->doc()->setHighlightingMode(args.join(QLatin1Char(' ')))) {
                static_cast<KTextEditor::DocumentPrivate *>(v->doc())->setDontChangeHlOnSave();
                return true;
            }

            KCC_ERR(i18n("No such highlighting '%1'", args.first()));
        } else if (cmd == QLatin1String("set-mode")) {
            if (v->doc()->setMode(args.first())) {
                return true;
            }

            KCC_ERR(i18n("No such mode '%1'", args.first()));
        }
    }
    // ALL commands that takes exactly one integer argument.
    else if (cmd == QLatin1String("set-tab-width") || cmd == QLatin1String("set-indent-width") || cmd == QLatin1String("set-word-wrap-column")
             || cmd == QLatin1String("goto")) {
        // find a integer value > 0
        if (args.isEmpty()) {
            KCC_ERR(i18n("Missing argument. Usage: %1 <value>", cmd));
        }
        bool ok;
        int val(args.first().toInt(&ok, 10)); // use base 10 even if the string starts with '0'
        if (!ok)
            KCC_ERR(i18n("Failed to convert argument '%1' to integer.", args.first()));

        if (cmd == QLatin1String("set-tab-width")) {
            if (val < 1) {
                KCC_ERR(i18n("Width must be at least 1."));
            }
            v->doc()->config()->setTabWidth(val);
        } else if (cmd == QLatin1String("set-indent-width")) {
            if (val < 1) {
                KCC_ERR(i18n("Width must be at least 1."));
            }
            v->doc()->config()->setIndentationWidth(val);
        } else if (cmd == QLatin1String("set-word-wrap-column")) {
            if (val < 2) {
                KCC_ERR(i18n("Column must be at least 1."));
            }
            v->doc()->setWordWrapAt(val);
        } else if (cmd == QLatin1String("goto")) {
            if (args.first().at(0) == QLatin1Char('-') || args.first().at(0) == QLatin1Char('+')) {
                // if the number starts with a minus or plus sign, add/subtract the number
                val = v->cursorPosition().line() + val;
            } else {
                val--; // convert given line number to the internal representation of line numbers
            }

            // constrain cursor to the range [0, number of lines]
            if (val < 0) {
                val = 0;
            } else if (val > v->doc()->lines() - 1) {
                val = v->doc()->lines() - 1;
            }

            v->setCursorPosition(KTextEditor::Cursor(val, 0));
            return true;
        }
        return true;
    }

    // ALL commands that takes 1 boolean argument.
    else if (cmd == QLatin1String("set-icon-border") || cmd == QLatin1String("set-folding-markers") || cmd == QLatin1String("set-indent-pasted-text")
             || cmd == QLatin1String("set-line-numbers") || cmd == QLatin1String("set-replace-tabs") || cmd == QLatin1String("set-show-tabs")
             || cmd == QLatin1String("set-word-wrap") || cmd == QLatin1String("set-wrap-cursor") || cmd == QLatin1String("set-replace-tabs-save")
             || cmd == QLatin1String("set-show-indent")) {
        if (args.isEmpty()) {
            KCC_ERR(i18n("Usage: %1 on|off|1|0|true|false", cmd));
        }
        bool enable = false;
        KateDocumentConfig *const config = v->doc()->config();
        if (getBoolArg(args.first(), &enable)) {
            if (cmd == QLatin1String("set-icon-border")) {
                v->setIconBorder(enable);
            } else if (cmd == QLatin1String("set-folding-markers")) {
                v->setFoldingMarkersOn(enable);
            } else if (cmd == QLatin1String("set-line-numbers")) {
                v->setLineNumbersOn(enable);
            } else if (cmd == QLatin1String("set-show-indent")) {
                v->renderer()->setShowIndentLines(enable);
            } else if (cmd == QLatin1String("set-indent-pasted-text")) {
                config->setIndentPastedText(enable);
            } else if (cmd == QLatin1String("set-replace-tabs")) {
                config->setReplaceTabsDyn(enable);
            } else if (cmd == QLatin1String("set-show-tabs")) {
                config->setShowTabs(enable);
            } else if (cmd == QLatin1String("set-show-trailing-spaces")) {
                config->setShowSpaces(enable ? KateDocumentConfig::Trailing : KateDocumentConfig::None);
            } else if (cmd == QLatin1String("set-word-wrap")) {
                v->doc()->setWordWrap(enable);
            }

            return true;
        } else
            KCC_ERR(i18n("Bad argument '%1'. Usage: %2 on|off|1|0|true|false", args.first(), cmd));
    } else if (cmd == QLatin1String("set-remove-trailing-spaces")) {
        // need at least one item, otherwise args.first() crashes
        if (args.count() != 1) {
            KCC_ERR(i18n("Usage: set-remove-trailing-spaces 0|-|none or 1|+|mod|modified or 2|*|all"));
        }

        QString tmp = args.first().toLower().trimmed();
        if (tmp == QLatin1String("1") || tmp == QLatin1String("modified") || tmp == QLatin1String("mod") || tmp == QLatin1String("+")) {
            v->doc()->config()->setRemoveSpaces(1);
        } else if (tmp == QLatin1String("2") || tmp == QLatin1String("all") || tmp == QLatin1String("*")) {
            v->doc()->config()->setRemoveSpaces(2);
        } else {
            v->doc()->config()->setRemoveSpaces(0);
        }
    }

    // unlikely..
    KCC_ERR(i18n("Unknown command '%1'", cmd));
}

bool KateCommands::CoreCommands::supportsRange(const QString &range)
{
    static QStringList l;

    if (l.isEmpty()) {
        l << QStringLiteral("indent") << QStringLiteral("unindent") << QStringLiteral("cleanindent") << QStringLiteral("comment") << QStringLiteral("uncomment")
          << QStringLiteral("kill-line") << QStringLiteral("fold") << QStringLiteral("tfold");
    }

    return l.contains(range);
}

KCompletion *KateCommands::CoreCommands::completionObject(KTextEditor::View *view, const QString &cmd)
{
    Q_UNUSED(view)

    if (cmd == QLatin1String("set-highlight")) {
        QStringList l;
        l.reserve(KateHlManager::self()->modeList().size());
        const auto modeList = KateHlManager::self()->modeList();
        for (const auto &hl : modeList) {
            l << hl.name();
        }

        KateCmdShellCompletion *co = new KateCmdShellCompletion();
        co->setItems(l);
        co->setIgnoreCase(true);
        return co;
    } else if (cmd == QLatin1String("set-remove-trailing-spaces")) {
        QStringList l;
        l << QStringLiteral("none") << QStringLiteral("modified") << QStringLiteral("all");

        KateCmdShellCompletion *co = new KateCmdShellCompletion();
        co->setItems(l);
        co->setIgnoreCase(true);
        return co;
    } else if (cmd == QLatin1String("set-indent-mode")) {
        QStringList l = KateAutoIndent::listIdentifiers();
        KateCmdShellCompletion *co = new KateCmdShellCompletion();
        co->setItems(l);
        co->setIgnoreCase(true);
        return co;
    }

    return nullptr;
}
// END CoreCommands

// BEGIN Character
KateCommands::Character *KateCommands::Character::m_instance = nullptr;

bool KateCommands::Character::help(class KTextEditor::View *, const QString &cmd, QString &msg)
{
    if (cmd.trimmed() == QLatin1String("char")) {
        msg = i18n(
            "<p> char <b>identifier</b> </p>"
            "<p>This command allows you to insert literal characters by their numerical identifier, in decimal, octal or hexadecimal form.</p>"
            "<p>Examples:<ul>"
            "<li>char <b>234</b></li>"
            "<li>char <b>0x1234</b></li>"
            "</ul></p>");
        return true;
    }
    return false;
}

bool KateCommands::Character::exec(KTextEditor::View *view, const QString &_cmd, QString &, const KTextEditor::Range &)
{
    QString cmd = _cmd;

    // hex, octal, base 9+1
    static const QRegularExpression num(QStringLiteral("^char *(0?x[0-9A-Fa-f]{1,4}|0[0-7]{1,6}|[0-9]{1,5})$"));
    const QRegularExpressionMatch match = num.match(cmd);
    if (!match.hasMatch()) {
        return false;
    }

    cmd = match.captured(1);

    // identify the base

    unsigned short int number = 0;
    int base = 10;
    if (cmd.startsWith(QLatin1Char('x'))) {
        cmd.remove(0, 1);
        base = 16;
    } else if (cmd.startsWith(QLatin1String("0x"))) {
        cmd.remove(0, 2);
        base = 16;
    } else if (cmd[0] == QLatin1Char('0')) {
        base = 8;
    }
    bool ok;
    number = cmd.toUShort(&ok, base);
    if (!ok || number == 0) {
        return false;
    }
    if (number <= 255) {
        char buf[2];
        buf[0] = (char)number;
        buf[1] = 0;

        view->document()->insertText(view->cursorPosition(), QString::fromLatin1(buf));
    } else {
        // do the unicode thing
        QChar c(number);

        view->document()->insertText(view->cursorPosition(), QString(&c, 1));
    }

    return true;
}

// END Character

// BEGIN Date
KateCommands::Date *KateCommands::Date::m_instance = nullptr;

bool KateCommands::Date::help(class KTextEditor::View *, const QString &cmd, QString &msg)
{
    if (cmd.trimmed() == QLatin1String("date")) {
        msg = i18n(
            "<p>date or date <b>format</b></p>"
            "<p>Inserts a date/time string as defined by the specified format, or the format yyyy-MM-dd hh:mm:ss if none is specified.</p>"
            "<p>Possible format specifiers are:"
            "<table>"
            "<tr><td>d</td><td>The day as number without a leading zero (1-31).</td></tr>"
            "<tr><td>dd</td><td>The day as number with a leading zero (01-31).</td></tr>"
            "<tr><td>ddd</td><td>The abbreviated localized day name (e.g. 'Mon'..'Sun').</td></tr>"
            "<tr><td>dddd</td><td>The long localized day name (e.g. 'Monday'..'Sunday').</td></tr>"
            "<tr><td>M</td><td>The month as number without a leading zero (1-12).</td></tr>"
            "<tr><td>MM</td><td>The month as number with a leading zero (01-12).</td></tr>"
            "<tr><td>MMM</td><td>The abbreviated localized month name (e.g. 'Jan'..'Dec').</td></tr>"
            "<tr><td>yy</td><td>The year as two digit number (00-99).</td></tr>"
            "<tr><td>yyyy</td><td>The year as four digit number (1752-8000).</td></tr>"
            "<tr><td>h</td><td>The hour without a leading zero (0..23 or 1..12 if AM/PM display).</td></tr>"
            "<tr><td>hh</td><td>The hour with a leading zero (00..23 or 01..12 if AM/PM display).</td></tr>"
            "<tr><td>m</td><td>The minute without a leading zero (0..59).</td></tr>"
            "<tr><td>mm</td><td>The minute with a leading zero (00..59).</td></tr>"
            "<tr><td>s</td><td>The second without a leading zero (0..59).</td></tr>"
            "<tr><td>ss</td><td>The second with a leading zero (00..59).</td></tr>"
            "<tr><td>z</td><td>The milliseconds without leading zeroes (0..999).</td></tr>"
            "<tr><td>zzz</td><td>The milliseconds with leading zeroes (000..999).</td></tr>"
            "<tr><td>AP</td><td>Use AM/PM display. AP will be replaced by either \"AM\" or \"PM\".</td></tr>"
            "<tr><td>ap</td><td>Use am/pm display. ap will be replaced by either \"am\" or \"pm\".</td></tr>"
            "</table></p>");
        return true;
    }
    return false;
}

bool KateCommands::Date::exec(KTextEditor::View *view, const QString &cmd, QString &, const KTextEditor::Range &)
{
    if (!cmd.startsWith(QLatin1String("date"))) {
        return false;
    }

    if (QDateTime::currentDateTime().toString(cmd.mid(5, cmd.length() - 5)).length() > 0) {
        view->document()->insertText(view->cursorPosition(), QDateTime::currentDateTime().toString(cmd.mid(5, cmd.length() - 5)));
    } else {
        view->document()->insertText(view->cursorPosition(), QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
    }

    return true;
}

// END Date
