/*
    SPDX-FileCopyrightText: 2003-2005 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Charles Samuels <charles@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <vimode/cmds.h>

#include "globalstate.h"
#include "katecmd.h"
#include "kateview.h"
#include "kateviinputmode.h"
#include "marks.h"
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>
#include <vimode/inputmodemanager.h>
#include <vimode/modes/normalvimode.h>
#include <vimode/searcher.h>

#include <KLocalizedString>

#include <QRegularExpression>

using namespace KateVi;

// BEGIN ViCommands
Commands *Commands::m_instance = nullptr;

bool Commands::exec(KTextEditor::View *view, const QString &_cmd, QString &msg, const KTextEditor::Range &range)
{
    Q_UNUSED(range)
    // cast it hardcore, we know that it is really a kateview :)
    KTextEditor::ViewPrivate *v = static_cast<KTextEditor::ViewPrivate *>(view);

    if (!v) {
        msg = i18n("Could not access view");
        return false;
    }

    // create a list of args
    QStringList args(_cmd.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts));
    QString cmd(args.takeFirst());

    // ALL commands that takes no arguments.
    if (mappingCommands().contains(cmd)) {
        if (cmd.endsWith(QLatin1String("unmap"))) {
            if (args.count() == 1) {
                m_viInputModeManager->globalState()->mappings()->remove(modeForMapCommand(cmd), args.at(0));
                return true;
            } else {
                msg = i18n("Missing argument. Usage: %1 <from>", cmd);
                return false;
            }
        } else if (cmd == QLatin1String("nohlsearch") || cmd == QLatin1String("noh")) {
            m_viInputModeManager->searcher()->hideCurrentHighlight();
            return true;
        } else if (cmd == QLatin1String("set-hlsearch") || cmd == QLatin1String("set-hls")) {
            m_viInputModeManager->searcher()->enableHighlightSearch(true);
            return true;
        } else if (cmd == QLatin1String("set-nohlsearch") || cmd == QLatin1String("set-nohls")) {
            m_viInputModeManager->searcher()->enableHighlightSearch(false);
            return true;
        }
        if (args.count() == 1) {
            msg = m_viInputModeManager->globalState()->mappings()->get(modeForMapCommand(cmd), args.at(0), true);
            if (msg.isEmpty()) {
                msg = i18n("No mapping found for \"%1\"", args.at(0));
                return false;
            } else {
                msg = i18n("\"%1\" is mapped to \"%2\"", args.at(0), msg);
            }
        } else if (args.count() == 2) {
            Mappings::MappingRecursion mappingRecursion = (isMapCommandRecursive(cmd)) ? Mappings::Recursive : Mappings::NonRecursive;
            m_viInputModeManager->globalState()->mappings()->add(modeForMapCommand(cmd), args.at(0), args.at(1), mappingRecursion);
        } else {
            msg = i18n("Missing argument(s). Usage: %1 <from> [<to>]", cmd);
            return false;
        }

        return true;
    }

    NormalViMode *nm = m_viInputModeManager->getViNormalMode();

    if (cmd == QLatin1String("d") || cmd == QLatin1String("delete") || cmd == QLatin1String("j") || cmd == QLatin1String("c") || cmd == QLatin1String("change")
        || cmd == QLatin1String("<") || cmd == QLatin1String(">") || cmd == QLatin1String("y") || cmd == QLatin1String("yank")) {
        KTextEditor::Cursor start_cursor_position = v->cursorPosition();

        int count = 1;
        if (range.isValid()) {
            count = qAbs(range.end().line() - range.start().line()) + 1;
            v->setCursorPosition(KTextEditor::Cursor(qMin(range.start().line(), range.end().line()), 0));
        }

        static const QRegularExpression number(QStringLiteral("^(\\d+)$"));
        for (int i = 0; i < args.count(); i++) {
            auto match = number.match(args.at(i));
            if (match.hasMatch()) {
                count += match.captured(0).toInt() - 1;
            }

            QChar r = args.at(i).at(0);
            if (args.at(i).size() == 1
                && ((r >= QLatin1Char('a') && r <= QLatin1Char('z')) || r == QLatin1Char('_') || r == QLatin1Char('-') || r == QLatin1Char('+')
                    || r == QLatin1Char('*'))) {
                nm->setRegister(r);
            }
        }

        nm->setCount(count);

        if (cmd == QLatin1String("d") || cmd == QLatin1String("delete")) {
            nm->commandDeleteLine();
        }
        if (cmd == QLatin1String("j")) {
            nm->commandJoinLines();
        }
        if (cmd == QLatin1String("c") || cmd == QLatin1String("change")) {
            nm->commandChangeLine();
        }
        if (cmd == QLatin1String("<")) {
            nm->commandUnindentLine();
        }
        if (cmd == QLatin1String(">")) {
            nm->commandIndentLine();
        }
        if (cmd == QLatin1String("y") || cmd == QLatin1String("yank")) {
            nm->commandYankLine();
            v->setCursorPosition(start_cursor_position);
        }

        // TODO - should we resetParser, here? We'd have to make it public, if so.
        // Or maybe synthesise a KateViCommand to execute instead ... ?
        nm->setCount(0);

        return true;
    }

    if (cmd == QLatin1String("mark") || cmd == QLatin1String("ma") || cmd == QLatin1String("k")) {
        if (args.count() == 0) {
            if (cmd == QLatin1String("mark")) {
                // TODO: show up mark list;
            } else {
                msg = i18n("Wrong arguments");
                return false;
            }
        } else if (args.count() == 1) {
            QChar r = args.at(0).at(0);
            int line;
            if ((r >= QLatin1Char('a') && r <= QLatin1Char('z')) || r == QLatin1Char('_') || r == QLatin1Char('+') || r == QLatin1Char('*')) {
                if (range.isValid()) {
                    line = qMax(range.end().line(), range.start().line());
                } else {
                    line = v->cursorPosition().line();
                }

                m_viInputModeManager->marks()->setUserMark(r, KTextEditor::Cursor(line, 0));
            }
        } else {
            msg = i18n("Wrong arguments");
            return false;
        }
        return true;
    }

    // should not happen :)
    msg = i18n("Unknown command '%1'", cmd);
    return false;
}

bool Commands::supportsRange(const QString &range)
{
    static QStringList l;

    if (l.isEmpty()) {
        l << QStringLiteral("d") << QStringLiteral("delete") << QStringLiteral("j") << QStringLiteral("c") << QStringLiteral("change") << QStringLiteral("<")
          << QStringLiteral(">") << QStringLiteral("y") << QStringLiteral("yank") << QStringLiteral("ma") << QStringLiteral("mark") << QStringLiteral("k");
    }

    return l.contains(range.split(QLatin1Char(' ')).at(0));
}

KCompletion *Commands::completionObject(KTextEditor::View *view, const QString &cmd)
{
    Q_UNUSED(view)

    KTextEditor::ViewPrivate *v = static_cast<KTextEditor::ViewPrivate *>(view);

    if (v && (cmd == QLatin1String("nn") || cmd == QLatin1String("nnoremap"))) {
        QStringList l = m_viInputModeManager->globalState()->mappings()->getAll(Mappings::NormalModeMapping);

        KateCmdShellCompletion *co = new KateCmdShellCompletion();
        co->setItems(l);
        co->setIgnoreCase(false);
        return co;
    }
    return nullptr;
}

const QStringList &Commands::mappingCommands()
{
    static QStringList mappingsCommands;
    if (mappingsCommands.isEmpty()) {
        mappingsCommands << QStringLiteral("nmap") << QStringLiteral("nm") << QStringLiteral("noremap") << QStringLiteral("nnoremap") << QStringLiteral("nn")
                         << QStringLiteral("no") << QStringLiteral("vmap") << QStringLiteral("vm") << QStringLiteral("vnoremap") << QStringLiteral("vn")
                         << QStringLiteral("imap") << QStringLiteral("im") << QStringLiteral("inoremap") << QStringLiteral("ino") << QStringLiteral("cmap")
                         << QStringLiteral("cm") << QStringLiteral("cnoremap") << QStringLiteral("cno");

        mappingsCommands << QStringLiteral("nunmap") << QStringLiteral("vunmap") << QStringLiteral("iunmap") << QStringLiteral("cunmap");

        mappingsCommands << QStringLiteral("nohlsearch") << QStringLiteral("noh") << QStringLiteral("set-hlsearch") << QStringLiteral("set-hls")
                         << QStringLiteral("set-nohlsearch") << QStringLiteral("set-nohls");
    }
    return mappingsCommands;
}

Mappings::MappingMode Commands::modeForMapCommand(const QString &mapCommand)
{
    if (mapCommand.startsWith(u'v')) {
        if (mapCommand == u"vmap" || mapCommand == u"vm" || mapCommand == u"vn" || mapCommand == u"vnoremap" || mapCommand == u"vunmap") {
            return Mappings::VisualModeMapping;
        }
    }

    if (mapCommand.startsWith(u'i')) {
        if (mapCommand == u"imap" || mapCommand == u"im" || mapCommand == u"ino" || mapCommand == u"inoremap" || mapCommand == u"iunmap") {
            return Mappings::InsertModeMapping;
        }
    }

    if (mapCommand.startsWith(u'c')) {
        if (mapCommand == u"cmap" || mapCommand == u"cm" || mapCommand == u"cno" || mapCommand == u"cnoremap" || mapCommand == u"cunmap") {
            return Mappings::CommandModeMapping;
        }
    }

    // if (mapCommand == u"nunmap") {
    //     return Mappings::NormalModeMapping;
    // }
    return Mappings::NormalModeMapping;
}

bool Commands::isMapCommandRecursive(const QString &mapCommand)
{
    return mapCommand == u"nmap" || mapCommand == u"nm" //
        || mapCommand == u"vmap" || mapCommand == u"vm" //
        || mapCommand == u"imap" || mapCommand == u"im" //
        || mapCommand == u"cmap" || mapCommand == u"cm";
}

// END ViCommands

// BEGIN SedReplace
SedReplace *SedReplace::m_instance = nullptr;

bool SedReplace::interactiveSedReplace(KTextEditor::ViewPrivate *, std::shared_ptr<InteractiveSedReplacer> interactiveSedReplace)
{
    EmulatedCommandBar *emulatedCommandBar = m_viInputModeManager->inputAdapter()->viModeEmulatedCommandBar();
    emulatedCommandBar->startInteractiveSearchAndReplace(interactiveSedReplace);
    return true;
}
// END SedReplace
