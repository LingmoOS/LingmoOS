/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include "../parser/backtraceparsergdb.h"

class GdbBacktraceLineTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    // rating() is often times somewhat misleading because it is an exclusive state
    // but in practice a frame may have multiple issues. for purposes of rating all
    // issues are considered equal. it's not ideal though, a frame that is missing
    // multiple elements is objectively worse than a frame that is just missing one.

    void testComplete()
    {
        BacktraceLineGdb line(
            "#7  0x00007f468b177bfa in KMime::DateFormatterPrivate::localized (t=t@entry=1579263464, shortFormat=shortFormat@entry=true, lang=...) at "
            "/usr/src/debug/kmime-19.12.1-lp151.150.1.x86_64/src/kmime_dateformatter.cpp:310\n");
        QCOMPARE(line.type(), BacktraceLine::StackFrame);
        QCOMPARE(line.frameNumber(), 7);
        QCOMPARE(line.functionName(), "KMime::DateFormatterPrivate::localized");
        QCOMPARE(line.fileName(), "/usr/src/debug/kmime-19.12.1-lp151.150.1.x86_64/src/kmime_dateformatter.cpp:310");
        QCOMPARE(line.libraryName(), "");
        QCOMPARE(line.rating(), BacktraceLine::Good);
    }

    void testPoorFile()
    {
        BacktraceLineGdb line("#41 0x00007f4684ae4e87 in g_main_context_dispatch () from /usr/lib64/libglib-2.0.so.0\n");
        QCOMPARE(line.type(), BacktraceLine::StackFrame);
        QCOMPARE(line.frameNumber(), 41);
        QCOMPARE(line.functionName(), "g_main_context_dispatch");
        QCOMPARE(line.fileName(), "");
        QCOMPARE(line.libraryName(), "/usr/lib64/libglib-2.0.so.0");
        QCOMPARE(line.rating(), BacktraceLine::MissingSourceFile);
    }

    void testNoFunctionPoorFile()
    {
        // Also uses 'at' keyword while referring to a library, this used to trip up
        // the parser and make it think there's a source file, when in reality there
        // is not.
        // Lacking a function name further tripped up the parsing because originally
        // couldn't deal with the function name being missing entirely.
        // As a result this line used to rate as 'Good' -.-
        // https://bugs.kde.org/show_bug.cgi?id=416923
        // https://bugs.kde.org/show_bug.cgi?id=418538
        { // glib
            BacktraceLineGdb line("#13 0x00007fe6059971b1 in  () at /usr/lib/libglib-2.0.so.0\n");
            QCOMPARE(line.type(), BacktraceLine::StackFrame);
            QCOMPARE(line.frameNumber(), 13);
            QCOMPARE(line.functionName(), "");
            QCOMPARE(line.fileName(), "");
            QCOMPARE(line.libraryName(), "/usr/lib/libglib-2.0.so.0");
            QCOMPARE(line.rating(), BacktraceLine::MissingFunction);
        }
        { // library without -2.0 (trips up suffix detection)
            BacktraceLineGdb line("#13 0x00007fe6059971b1 in  () at /usr/lib/libpackagekit-qt.so.12\n");
            QCOMPARE(line.type(), BacktraceLine::StackFrame);
            QCOMPARE(line.frameNumber(), 13);
            QCOMPARE(line.functionName(), "");
            QCOMPARE(line.fileName(), "");
            QCOMPARE(line.libraryName(), "/usr/lib/libpackagekit-qt.so.12");
            QCOMPARE(line.rating(), BacktraceLine::MissingFunction);
        }
        { // library without any soversion
            BacktraceLineGdb line("#13 0x00007fe6059971b1 in  () at /usr/lib/libpackagekit-qt.so\n");
            QCOMPARE(line.type(), BacktraceLine::StackFrame);
            QCOMPARE(line.frameNumber(), 13);
            QCOMPARE(line.functionName(), "");
            QCOMPARE(line.fileName(), "");
            QCOMPARE(line.libraryName(), "/usr/lib/libpackagekit-qt.so");
            QCOMPARE(line.rating(), BacktraceLine::MissingFunction);
        }
        { // library without any soversion but name suffix
            BacktraceLineGdb line("#13 0x00007fe6059971b1 in  () at /usr/lib/libpackagekit-1.0.so\n");
            QCOMPARE(line.type(), BacktraceLine::StackFrame);
            QCOMPARE(line.frameNumber(), 13);
            QCOMPARE(line.functionName(), "");
            QCOMPARE(line.fileName(), "");
            QCOMPARE(line.libraryName(), "/usr/lib/libpackagekit-1.0.so");
            QCOMPARE(line.rating(), BacktraceLine::MissingFunction);
        }
    }

    void testOnlyFunctionNofile()
    {
        BacktraceLineGdb line("#20 0x0000557e978c1b7e in _start ()\n");
        QCOMPARE(line.type(), BacktraceLine::StackFrame);
        QCOMPARE(line.frameNumber(), 20);
        QCOMPARE(line.functionName(), "_start");
        QCOMPARE(line.fileName(), "");
        QCOMPARE(line.libraryName(), "");
        QCOMPARE(line.rating(), BacktraceLine::MissingLibrary);
    }

    void testInferiorMarker()
    {
        BacktraceLineGdb line("[Inferior 1 (process 72692) detached]\n");
        QCOMPARE(line.type(), BacktraceLine::Unknown);
        QCOMPARE(line.rating(), BacktraceLine::InvalidRating);
    }

    void testThreadStart()
    {
        const QString input = QStringLiteral("Thread 35 (Thread 0x7f77f57fa700 (LWP 8133)):\n");
        BacktraceLineGdb line(input);
        QCOMPARE(line.type(), BacktraceLineGdb::ThreadStart);
        QCOMPARE(line.rating(), BacktraceLine::InvalidRating);
        QCOMPARE(line.toString(), input);
    }

    void testThreadIndicator()
    {
        const QString input = QStringLiteral("[Current thread is 1 (Thread 0x7f78847c7c80 (LWP 7806))]\n");
        BacktraceLineGdb line(input);
        QCOMPARE(line.type(), BacktraceLineGdb::ThreadIndicator);
        QCOMPARE(line.rating(), BacktraceLine::InvalidRating);
        QCOMPARE(line.toString(), input);
    }
};

QTEST_GUILESS_MAIN(GdbBacktraceLineTest)

#include "gdbbacktracelinetest.moc"
