/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001, 2003 Peter Kelly <pmk@post.com>
    SPDX-FileCopyrightText: 2003, 2004 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2004 Dirk Mueller <mueller@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Leo Savernik <l.savernik@aon.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "testutils.h"

#include "kateconfig.h"
#include "katedocument.h"
#include "katescripthelpers.h"
#include "kateview.h"
#include "ktexteditor/cursor.h"
#include "ktexteditor/range.h"

#include <QObject>
#include <QQmlEngine>
#include <QTest>

// END Includes

// BEGIN TestScriptEnv

TestScriptEnv::TestScriptEnv(KTextEditor::DocumentPrivate *part, bool &cflag)
    : m_engine(nullptr)
    , m_viewObj(nullptr)
    , m_docObj(nullptr)
    , m_output(nullptr)
{
    m_engine = new QJSEngine(this);

    // export read & require function and add the require guard object
    QJSValue functions = m_engine->newQObject(new Kate::ScriptHelper(m_engine));
    m_engine->globalObject().setProperty(QStringLiteral("functions"), functions);
    m_engine->globalObject().setProperty(QStringLiteral("read"), functions.property(QStringLiteral("read")));
    m_engine->globalObject().setProperty(QStringLiteral("require"), functions.property(QStringLiteral("require")));
    m_engine->globalObject().setProperty(QStringLiteral("require_guard"), m_engine->newObject());

    // export debug function
    m_engine->globalObject().setProperty(QStringLiteral("debug"), functions.property(QStringLiteral("debug")));

    // export translation functions
    m_engine->globalObject().setProperty(QStringLiteral("i18n"), functions.property(QStringLiteral("_i18n")));
    m_engine->globalObject().setProperty(QStringLiteral("i18nc"), functions.property(QStringLiteral("_i18nc")));
    m_engine->globalObject().setProperty(QStringLiteral("i18np"), functions.property(QStringLiteral("_i18np")));
    m_engine->globalObject().setProperty(QStringLiteral("i18ncp"), functions.property(QStringLiteral("_i18ncp")));

    KTextEditor::ViewPrivate *view = qobject_cast<KTextEditor::ViewPrivate *>(part->widget());

    m_viewObj = new KateViewObject(m_engine, view);
    QJSValue sv = m_engine->newQObject(m_viewObj);

    m_engine->globalObject().setProperty(QStringLiteral("view"), sv);
    m_engine->globalObject().setProperty(QStringLiteral("v"), sv);

    m_docObj = new KateDocumentObject(m_engine, view->doc());
    QJSValue sd = m_engine->newQObject(m_docObj);

    m_engine->globalObject().setProperty(QStringLiteral("document"), sd);
    m_engine->globalObject().setProperty(QStringLiteral("d"), sd);

    m_output = new OutputObject(view, cflag);
    QJSValue so = m_engine->newQObject(m_output);

    m_engine->globalObject().setProperty(QStringLiteral("output"), so);
    m_engine->globalObject().setProperty(QStringLiteral("out"), so);
    m_engine->globalObject().setProperty(QStringLiteral("o"), so);
}

TestScriptEnv::~TestScriptEnv()
{
    // delete explicitly, as the parent is the KTE::Document kpart, which is
    // reused for all tests. Hence, we explicitly have to delete the bindings.
    delete m_output;
    m_output = nullptr;
    delete m_docObj;
    m_docObj = nullptr;
    delete m_viewObj;
    m_viewObj = nullptr;

    // delete this too, although this should also be automagically be freed
    delete m_engine;
    m_engine = nullptr;

    //   kDebug() << "deleted";
}
// END TestScriptEnv

// BEGIN KateViewObject

KateViewObject::KateViewObject(QJSEngine *engine, KTextEditor::ViewPrivate *view)
    : KateScriptView(engine)
{
    setView(view);
}

KateViewObject::~KateViewObject()
{
    //   kDebug() << "deleted";
}

// Implements a function that calls an edit function repeatedly as specified by
// its first parameter (once if not specified).
#define REP_CALL(func)                                                                                                                                         \
    void KateViewObject::func(int cnt)                                                                                                                         \
    {                                                                                                                                                          \
        while (cnt--) {                                                                                                                                        \
            view()->func();                                                                                                                                    \
        }                                                                                                                                                      \
    }
REP_CALL(keyReturn)
REP_CALL(backspace)
REP_CALL(deleteWordLeft)
REP_CALL(keyDelete)
REP_CALL(deleteWordRight)
REP_CALL(transpose)
REP_CALL(cursorLeft)
REP_CALL(shiftCursorLeft)
REP_CALL(cursorRight)
REP_CALL(shiftCursorRight)
REP_CALL(wordLeft)
REP_CALL(shiftWordLeft)
REP_CALL(wordRight)
REP_CALL(shiftWordRight)
REP_CALL(home)
REP_CALL(shiftHome)
REP_CALL(end)
REP_CALL(shiftEnd)
REP_CALL(up)
REP_CALL(shiftUp)
REP_CALL(down)
REP_CALL(shiftDown)
REP_CALL(scrollUp)
REP_CALL(scrollDown)
REP_CALL(topOfView)
REP_CALL(shiftTopOfView)
REP_CALL(bottomOfView)
REP_CALL(shiftBottomOfView)
REP_CALL(pageUp)
REP_CALL(shiftPageUp)
REP_CALL(pageDown)
REP_CALL(shiftPageDown)
REP_CALL(top)
REP_CALL(shiftTop)
REP_CALL(bottom)
REP_CALL(shiftBottom)
REP_CALL(toMatchingBracket)
REP_CALL(shiftToMatchingBracket)
#undef REP_CALL

void KateViewObject::type(const QString &str)
{
    view()->doc()->typeChars(view(), str);
}

void KateViewObject::paste(const QString &str)
{
    view()->doc()->paste(view(), str);
}

void KateViewObject::setAutoBrackets(bool enable)
{
    view()->config()->setValue(KateViewConfig::AutoBrackets, enable);
}

void KateViewObject::replaceTabs(bool enable)
{
    view()->doc()->config()->setValue(KateDocumentConfig::ReplaceTabsWithSpaces, enable);
}

#define ALIAS(alias, func)                                                                                                                                     \
    void KateViewObject::alias(int cnt)                                                                                                                        \
    {                                                                                                                                                          \
        func(cnt);                                                                                                                                             \
    }
ALIAS(enter, keyReturn)
ALIAS(cursorPrev, cursorLeft)
ALIAS(left, cursorLeft)
ALIAS(prev, cursorLeft)
ALIAS(shiftCursorPrev, shiftCursorLeft)
ALIAS(shiftLeft, shiftCursorLeft)
ALIAS(shiftPrev, shiftCursorLeft)
ALIAS(cursorNext, cursorRight)
ALIAS(right, cursorRight)
ALIAS(next, cursorRight)
ALIAS(shiftCursorNext, shiftCursorRight)
ALIAS(shiftRight, shiftCursorRight)
ALIAS(shiftNext, shiftCursorRight)
ALIAS(wordPrev, wordLeft)
ALIAS(shiftWordPrev, shiftWordLeft)
ALIAS(wordNext, wordRight)
ALIAS(shiftWordNext, shiftWordRight)
#undef ALIAS

// END KateViewObject

// BEGIN KateDocumentObject

KateDocumentObject::KateDocumentObject(QJSEngine *engine, KTextEditor::DocumentPrivate *doc)
    : KateScriptDocument(engine)
{
    setDocument(doc);
}

KateDocumentObject::~KateDocumentObject()
{
    //   kDebug() << "deleted";
}
// END KateDocumentObject

// BEGIN OutputObject
OutputObject::OutputObject(KTextEditor::ViewPrivate *v, bool &cflag)
    : view(v)
    , cflag(cflag)
{
}

OutputObject::~OutputObject()
{
    //   kDebug() << "deleted";
}

void OutputObject::output(bool cp, bool ln)
{
    QString str;
    //   FIXME: This is not available with QtQml, but not sure if we need it
    //    for (int i = 0; i < context()->argumentCount(); ++i) {
    //        QJSValue arg = context()->argument(i);
    //        str += arg.toString();
    //    }

    if (cp) {
        KTextEditor::Cursor c = view->cursorPosition();
        str += QLatin1Char('(') + QString::number(c.line()) + QLatin1Char(',') + QString::number(c.column()) + QLatin1Char(')');
    }

    if (ln) {
        str += QLatin1Char('\n');
    }

    view->insertText(str);

    cflag = true;
}

void OutputObject::write()
{
    output(false, false);
}

void OutputObject::writeln()
{
    output(false, true);
}

void OutputObject::writeLn()
{
    output(false, true);
}

void OutputObject::print()
{
    output(false, false);
}

void OutputObject::println()
{
    output(false, true);
}

void OutputObject::printLn()
{
    output(false, true);
}

void OutputObject::writeCursorPosition()
{
    output(true, false);
}

void OutputObject::writeCursorPositionln()
{
    output(true, true);
}

void OutputObject::cursorPosition()
{
    output(true, false);
}

void OutputObject::cursorPositionln()
{
    output(true, true);
}

void OutputObject::cursorPositionLn()
{
    output(true, true);
}

void OutputObject::pos()
{
    output(true, false);
}

void OutputObject::posln()
{
    output(true, true);
}

void OutputObject::posLn()
{
    output(true, true);
}
// END OutputObject

#include "moc_testutils.cpp"
