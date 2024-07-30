/*
    SPDX-FileCopyrightText: 2020 Aleksei Nikiforov <darktemplar@basealt.ru>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "config-kdesudtest.h"

#include <QObject>
#include <QTest>

#include "../lexer.h"

namespace KDESu
{
class KdeSudTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
    }

    // copy of KDESu::Client::escape
    QByteArray escape(const QByteArray &str)
    {
        QByteArray copy;
        copy.reserve(str.size() + 4);
        copy.append('"');
        for (const uchar c : str) {
            if (c < 32) {
                copy.append('\\');
                copy.append('^');
                copy.append(c + '@');
            } else {
                if (c == '\\' || c == '"') {
                    copy.append('\\');
                }
                copy.append(c);
            }
        }
        copy.append('"');
        return copy;
    }

    void commandWithDoubleQuotes()
    {
        // Process command like in KDESu::Client::exec
        QByteArray cmd;
        cmd = "EXEC ";
        cmd += escape("bash -c \"ls -la\"");
        cmd += ' ';
        cmd += escape("testuser");
        cmd += '\n';

        // Now handle command like in ConnectionHandler::doCommand
        Lexer l(cmd);
        QVERIFY(l.lex() == Lexer::Tok_exec);

        QVERIFY(l.lex() == Lexer::Tok_str);
        QVERIFY(l.lval() == "bash -c \"ls -la\"");

        QVERIFY(l.lex() == Lexer::Tok_str);
        QVERIFY(l.lval() == "testuser");

        QVERIFY(l.lex() == '\n');
    }
};
}

#include <kdesudtest.moc>
QTEST_MAIN(KDESu::KdeSudTest)
