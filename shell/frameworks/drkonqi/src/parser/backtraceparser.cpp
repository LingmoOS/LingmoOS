/*
    SPDX-FileCopyrightText: 2009-2010 George Kiagiadakis <kiagiadakis.george@gmail.com>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backtraceparser_p.h"
#include "backtraceparsergdb.h"
#include "backtraceparserlldb.h"
#include "backtraceparsernull.h"
#include "drkonqi_parser_debug.h"

#include <QMetaEnum>
#include <QRegularExpression>

// factory
BacktraceParser *BacktraceParser::newParser(const QString &debuggerName, QObject *parent)
{
    if (debuggerName == QLatin1String("gdb")) {
        return new BacktraceParserGdb(parent);
    }
    if (debuggerName == QLatin1String("lldb")) {
        return new BacktraceParserLldb(parent);
    }
    return new BacktraceParserNull(parent);
}

BacktraceParser::BacktraceParser(QObject *parent)
    : QObject(parent)
    , d_ptr(nullptr)
{
}
BacktraceParser::~BacktraceParser()
{
    delete d_ptr;
}

void BacktraceParser::connectToGenerator(QObject *generator)
{
    connect(generator, SIGNAL(starting()), this, SLOT(resetState()));
    connect(generator, SIGNAL(newLine(QString)), this, SLOT(newLine(QString)));
    connect(generator, SIGNAL(newLine(QString)), this, SLOT(newLineInternal(QString)));
}

QString BacktraceParser::parsedBacktrace() const
{
    Q_D(const BacktraceParser);

    QString result;
    if (d) {
        for (QList<BacktraceLine>::const_iterator i = d->m_linesList.constBegin(), total = d->m_linesList.constEnd(); i != total; ++i) {
            result += i->toString();
        }
    }
    return result;
}

QList<BacktraceLine> BacktraceParser::parsedBacktraceLines() const
{
    Q_D(const BacktraceParser);
    return d ? d->m_linesList : QList<BacktraceLine>();
}

QString BacktraceParser::simplifiedBacktrace() const
{
    Q_D(const BacktraceParser);

    // if there is no cached usefulness, the data calculation function has not run yet.
    if (d && d->m_usefulness == InvalidUsefulness) {
        const_cast<BacktraceParser *>(this)->calculateRatingData();
    }

    // if there is no d, the debugger has not run yet, so we have no backtrace.
    return d ? d->m_simplifiedBacktrace : QString();
}

BacktraceParser::Usefulness BacktraceParser::backtraceUsefulness() const
{
    Q_D(const BacktraceParser);

    // if there is no cached usefulness, the data calculation function has not run yet.
    if (d && d->m_usefulness == InvalidUsefulness) {
        const_cast<BacktraceParser *>(this)->calculateRatingData();
    }

    // if there is no d, the debugger has not run yet,
    // so we can say that the (inexistent) backtrace is Useless.
    return d ? d->m_usefulness : Useless;
}

QStringList BacktraceParser::librariesWithMissingDebugSymbols() const
{
    Q_D(const BacktraceParser);

    // if there is no cached usefulness, the data calculation function has not run yet.
    if (d && d->m_usefulness == InvalidUsefulness) {
        const_cast<BacktraceParser *>(this)->calculateRatingData();
    }

    // if there is no d, the debugger has not run yet, so we have no libraries.
    return d ? d->m_librariesWithMissingDebugSymbols : QStringList();
}

bool BacktraceParser::hasCompositorCrashed() const
{
    Q_D(const BacktraceParser);

    // if there is no cached usefulness, the data calculation function has not run yet.
    if (d && d->m_usefulness == InvalidUsefulness) {
        const_cast<BacktraceParser *>(this)->calculateRatingData();
    }

    // if there is no d, the debugger has not run yet, so we don't know anything.
    return d ? d->m_compositorCrashed : false;
}

void BacktraceParser::resetState()
{
    // reset the state of the parser by getting a new instance of Private
    delete d_ptr;
    d_ptr = constructPrivate();
}

BacktraceParserPrivate *BacktraceParser::constructPrivate() const
{
    return new BacktraceParserPrivate;
}

/* This function returns true if the given stack frame line is the base of the backtrace
   and thus the parser should not rate any frames below that one. */
static bool lineIsStackBase(const BacktraceLine &line)
{
    // optimization. if there is no function name, do not bother to check it
    if (line.rating() == BacktraceLine::MissingEverything || line.rating() == BacktraceLine::MissingFunction)
        return false;

    // "start_thread" is the base frame for all threads except the main thread, FIXME "start_thread"
    // probably works only on linux
    // main() or kdemain() is the base for the main thread
    if (line.functionName() == QLatin1String("start_thread") || line.functionName() == QLatin1String("main")
        || line.functionName() == QLatin1String("kdemain")) {
        return true;
    }

    // HACK for better rating. we ignore all stack frames below any function that matches
    // the following regular expression. The functions that match this expression are usually
    //"QApplicationPrivate::notify_helper", "QApplication::notify" and similar, which
    // are used to send any kind of event to the Qt application. All stack frames below this,
    // with or without debug symbols, are useless to KDE developers, so we ignore them.
    const QRegularExpression re(QRegularExpression::anchoredPattern(QStringLiteral("(Q|K)(Core)?Application(Private)?::notify.*")));
    if (re.match(line.functionName()).hasMatch()) {
        return true;
    }

    // attempt to recognize crashes that happen after main has returned (bug 200993)
    if (line.functionName() == QLatin1String("~KCleanUpGlobalStatic") || line.functionName() == QLatin1String("~QGlobalStatic")
        || line.functionName() == QLatin1String("exit") || line.functionName() == QLatin1String("*__GI_exit"))
        return true;

    return false;
}

/* This function returns true if the given stack frame line is the top of the bactrace
   and thus the parser should not rate any frames above that one. This is used to avoid
   rating the stack frames of abort(), assert(), Q_ASSERT() and qCCritical(DRKONQI_PARSER_LOG) */
static bool lineIsStackTop(const BacktraceLine &line)
{
    // optimization. if there is no function name, do not bother to check it
    if (line.rating() == BacktraceLine::MissingEverything || line.rating() == BacktraceLine::MissingFunction)
        return false;

    if (line.functionName().startsWith(QLatin1String("qt_assert")) // qt_assert and qt_assert_x
        || line.functionName() == QLatin1String("qFatal") || line.functionName() == QLatin1String("abort")
        || line.functionName() == QLatin1String("*__GI_abort") || line.functionName() == QLatin1String("*__GI___assert_fail"))
        return true;

    return false;
}

/* This function returns true if the given stack frame line should be ignored from rating
   for some reason. Currently it ignores all libc/libstdc++/libpthread functions. */
static bool lineShouldBeIgnored(const BacktraceLine &line)
{
    if (line.libraryName().contains(QLatin1String("libc.so")) || line.libraryName().contains(QLatin1String("libstdc++.so"))
        || line.functionName().startsWith(QLatin1String("*__GI_")) // glibc2.9 uses *__GI_ as prefix
        || line.libraryName().contains(QLatin1String("libpthread.so")) || line.libraryName().contains(QLatin1String("libglib-2.0.so"))
        || line.functionName() == QLatin1String("__libc_start_main") // below main on apps without symbols
        || line.functionName() == QLatin1String("_start") // below main on apps without symbols
#ifdef Q_OS_MACOS
        || (line.libraryName().startsWith(QLatin1String("libsystem_")) && line.libraryName().endsWith(QLatin1String(".dylib")))
        || line.libraryName().contains(QLatin1String("Foundation`"))
#endif
        || line.libraryName().contains(QLatin1String("ntdll.dll")) || line.libraryName().contains(QLatin1String("kernel32.dll"))
        || line.functionName().contains(QLatin1String("_tmain")) || line.functionName() == QLatin1String("WinMain")) {
        return true;
    }

    return false;
}

static bool isFunctionUseful(const BacktraceLine &line)
{
    // We need the function name
    if (line.rating() == BacktraceLine::MissingEverything || line.rating() == BacktraceLine::MissingFunction) {
        return false;
    }

    // Misc ignores
    if (line.functionName() == QLatin1String("__kernel_vsyscall") || line.functionName() == QLatin1String("raise")
        || line.functionName() == QLatin1String("abort") || line.functionName() == QLatin1String("__libc_message")
        || line.functionName() == QLatin1String("thr_kill") /* *BSD */) {
        return false;
    }

    // Ignore core Qt functions
    //(QObject can be useful in some cases)
    if (line.functionName().startsWith(QLatin1String("QBasicAtomicInt::")) || line.functionName().startsWith(QLatin1String("QBasicAtomicPointer::"))
        || line.functionName().startsWith(QLatin1String("QAtomicInt::")) || line.functionName().startsWith(QLatin1String("QAtomicPointer::"))
        || line.functionName().startsWith(QLatin1String("QMetaObject::")) || line.functionName().startsWith(QLatin1String("QPointer::"))
        || line.functionName().startsWith(QLatin1String("QWeakPointer::")) || line.functionName().startsWith(QLatin1String("QSharedPointer::"))
        || line.functionName().startsWith(QLatin1String("QScopedPointer::")) || line.functionName().startsWith(QLatin1String("QMetaCallEvent::"))) {
        return false;
    }

    // Ignore core Qt containers misc functions
    if (line.functionName().endsWith(QLatin1String("detach")) || line.functionName().endsWith(QLatin1String("detach_helper"))
        || line.functionName().endsWith(QLatin1String("node_create")) || line.functionName().endsWith(QLatin1String("deref"))
        || line.functionName().endsWith(QLatin1String("ref")) || line.functionName().endsWith(QLatin1String("node_copy"))
        || line.functionName().endsWith(QLatin1String("d_func"))) {
        return false;
    }

    // Misc Qt stuff
    if (line.functionName() == QLatin1String("qt_message_output") || line.functionName() == QLatin1String("qt_message")
        || line.functionName() == QLatin1String("qFatal") || line.functionName().startsWith(QLatin1String("qGetPtrHelper"))
        || line.functionName().startsWith(QLatin1String("qt_meta_"))) {
        return false;
    }

    return true;
}

void BacktraceParser::calculateRatingData()
{
    Q_D(BacktraceParser);

    uint rating = 0, bestPossibleRating = 0, counter = 0;
    bool haveSeenStackBase = false;

    QListIterator<BacktraceLine> i(d->m_linesToRate);
    i.toBack(); // start from the end of the list

    while (i.hasPrevious()) {
        const BacktraceLine &line = i.previous();

        if (!d->m_compositorCrashed && line.toString().contains(QLatin1String("The Wayland connection broke. Did the Wayland compositor die"))) {
            d->m_compositorCrashed = true;
        }

        if (!i.hasPrevious() && line.rating() == BacktraceLine::MissingEverything) {
            // Under some circumstances, the very first stack frame is invalid (ex, calling a function
            // at an invalid address could result in a stack frame like "0x00000000 in ?? ()"),
            // which however does not necessarily mean that the backtrace has a missing symbol on
            // the first line. Here we make sure to ignore this line from rating. (bug 190882)
            break; // there are no more items anyway, just break the loop
        }

        if (lineIsStackBase(line)) {
            rating = bestPossibleRating = counter = 0; // restart rating ignoring any previous frames
            haveSeenStackBase = true;
        } else if (lineIsStackTop(line)) {
            break; // we have reached the top, no need to inspect any more frames
        }

        if (lineShouldBeIgnored(line)) {
            continue;
        }

        if (line.rating() == BacktraceLine::MissingFunction || line.rating() == BacktraceLine::MissingSourceFile) {
            d->m_librariesWithMissingDebugSymbols.append(line.libraryName().trimmed());
        }

        uint multiplier = ++counter; // give weight to the first lines
        rating += static_cast<uint>(line.rating()) * multiplier;
        bestPossibleRating += static_cast<uint>(BacktraceLine::BestRating) * multiplier;

        qCDebug(DRKONQI_PARSER_LOG) << line.rating() << line.toString();
    }

    d->m_librariesWithMissingDebugSymbols.removeDuplicates();

    // Generate a simplified backtrace
    //- Starts from the first useful function
    //- Max of 5 lines
    //- Replaces garbage with [...]
    // At the same time, grab the first three useful functions for search queries

    i.toFront(); // Reuse the list iterator
    int functionIndex = 0;
    bool firstUsefulFound = false;
    while (i.hasNext() && functionIndex < 5) {
        const BacktraceLine &line = i.next();
        if (!lineShouldBeIgnored(line) && isFunctionUseful(line)) { // Line is not garbage to use
            if (!firstUsefulFound) {
                firstUsefulFound = true;
            }
            // Save simplified backtrace line
            d->m_simplifiedBacktrace += line.toString();

            functionIndex++;
        } else if (firstUsefulFound) {
            // Add "[...]" if there are invalid functions in the middle
            if (!d->m_simplifiedBacktrace.endsWith(QLatin1String("[...]\n"))) {
                d->m_simplifiedBacktrace += QLatin1String("[...]\n");
            }
        }
    }

    // calculate rating
    d->m_usefulness = Useless;
    if (rating >= (bestPossibleRating * 0.90)) {
        d->m_usefulness = ReallyUseful;
    } else if (rating >= (bestPossibleRating * 0.70)) {
        d->m_usefulness = MayBeUseful;
    } else if (rating >= (bestPossibleRating * 0.40)) {
        d->m_usefulness = ProbablyUseless;
    }

    // if there is no stack base, the executable is probably stripped,
    // so we need to be more strict with rating
    if (!haveSeenStackBase) {
        // less than 1 stack frame is useless
        if (counter < 1) {
            d->m_usefulness = Useless;
            // more than 1 stack frames might have some value, so let's not be so strict, just lower the rating
        } else if (d->m_usefulness > Useless) {
            d->m_usefulness = (Usefulness)(d->m_usefulness - 1);
        }
    }

    qCDebug(DRKONQI_PARSER_LOG) << "Rating:" << rating << "out of" << bestPossibleRating
                                << "Usefulness:" << staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("Usefulness")).valueToKey(d->m_usefulness);
    qCDebug(DRKONQI_PARSER_LOG) << "90%:" << (bestPossibleRating * 0.90) << "70%:" << (bestPossibleRating * 0.70) << "40%:" << (bestPossibleRating * 0.40);
    qCDebug(DRKONQI_PARSER_LOG) << "Have seen stack base:" << haveSeenStackBase << "Lines counted:" << counter;
}

QString BacktraceParser::informationLines() const
{
    Q_D(const BacktraceParser);
    QString ret = d->m_infoLines.join(QLatin1Char('\n'));
    if (!ret.endsWith(QLatin1Char('\n')))
        ret += QLatin1Char('\n');
    return ret;
}

void BacktraceParser::newLineInternal(const QString &)
{
    Q_D(BacktraceParser);
    d->m_usefulness = InvalidUsefulness;
}

#include "moc_backtraceparser.cpp"
