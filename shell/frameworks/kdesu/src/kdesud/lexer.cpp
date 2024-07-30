/* vi: ts=8 sts=4 sw=4

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    lexer.cpp: A lexer for the kdesud protocol. See kdesud.cpp for a
               description of the protocol.
*/

#include "lexer.h"

#include <ctype.h>

Lexer::Lexer(const QByteArray &input)
{
    m_Input = input;
    in = 0;
}

Lexer::~Lexer()
{
    // Erase buffers
    m_Input.fill('x');
    m_Output.fill('x');
}

QByteArray &Lexer::lval()
{
    return m_Output;
}

/*
 * lex() is the lexer. There is no end-of-input check here so that has to be
 * done by the caller.
 */

int Lexer::lex()
{
    char c;

    c = m_Input[in++];
    m_Output.fill('x');
    m_Output.resize(0);

    while (1) {
        // newline?
        if (c == '\n') {
            return '\n';
        }

        // No control characters
        if (iscntrl(c)) {
            return Tok_none;
        }

        if (isspace(c)) {
            while (isspace(c = m_Input[in++])) {
                ;
            }
        }

        // number?
        if (isdigit(c)) {
            m_Output += c;
            while (isdigit(c = m_Input[in++])) {
                m_Output += c;
            }
            in--;
            return Tok_num;
        }

        // quoted string?
        if (c == '"') {
            c = m_Input[in++];
            while ((c != '"') && !iscntrl(c)) {
                // handle escaped characters
                if (c == '\\') {
                    c = m_Input[in++];
                    if (iscntrl(c)) {
                        return Tok_none;
                    }
                    if (c == '^') {
                        c = m_Input[in++];
                        if ((c == '"') || iscntrl(c)) {
                            return Tok_none;
                        }
                        m_Output += c - '@';
                    } else {
                        m_Output += c;
                    }
                } else {
                    m_Output += c;
                }
                c = m_Input[in++];
            }
            if (c == '"') {
                return Tok_str;
            }
            return Tok_none;
        }

        // normal string
        while (!isspace(c) && !iscntrl(c)) {
            m_Output += c;
            c = m_Input[in++];
        }
        in--;

        // command?
        if (m_Output.length() <= 4) {
            if (m_Output == "EXEC") {
                return Tok_exec;
            }
            if (m_Output == "PASS") {
                return Tok_pass;
            }
            if (m_Output == "DEL") {
                return Tok_delCmd;
            }
            if (m_Output == "PING") {
                return Tok_ping;
            }
            if (m_Output == "EXIT") {
                return Tok_exit;
            }
            if (m_Output == "STOP") {
                return Tok_stop;
            }
            if (m_Output == "SET") {
                return Tok_set;
            }
            if (m_Output == "GET") {
                return Tok_get;
            }
            if (m_Output == "HOST") {
                return Tok_host;
            }
            if (m_Output == "SCHD") {
                return Tok_sched;
            }
            if (m_Output == "PRIO") {
                return Tok_prio;
            }
            if (m_Output == "DELV") {
                return Tok_delVar;
            }
            if (m_Output == "DELG") {
                return Tok_delGroup;
            }
            if (m_Output == "DELS") {
                return Tok_delSpecialKey;
            }
            if (m_Output == "GETK") {
                return Tok_getKeys;
            }
            if (m_Output == "CHKG") {
                return Tok_chkGroup;
            }
        }

        return Tok_str;
    }
}
