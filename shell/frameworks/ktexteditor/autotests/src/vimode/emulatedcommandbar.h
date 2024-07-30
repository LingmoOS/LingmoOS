/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2011 Kuzmich Svyatoslav
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EMULATEDCOMMANDBAR_TEST_H
#define EMULATEDCOMMANDBAR_TEST_H

#include "base.h"

class QCompleter;
class QLabel;
class QColor;

namespace Kate
{
class TextRange;
}

class EmulatedCommandBarTest : public BaseTest
{
    Q_OBJECT

private Q_SLOTS:
    void EmulatedCommandBarTests();

private:
    QCompleter *emulatedCommandBarCompleter();

    void verifyCommandBarCompletionVisible();
    void verifyCommandBarCompletionsMatches(const QStringList &expectedCompletionList);
    void verifyCommandBarCompletionContains(const QStringList &expectedCompletionList);
    QLabel *emulatedCommandTypeIndicator();
    void verifyCursorAt(KTextEditor::Cursor expectedCursorPos);

    void clearSearchHistory();
    QStringList searchHistory();
    void clearCommandHistory();
    QStringList commandHistory();
    void clearReplaceHistory();
    QStringList replaceHistory();

    QList<Kate::TextRange *> rangesOnFirstLine();
    void verifyTextEditBackgroundColour(const QColor &expectedBackgroundColour);
    QLabel *commandResponseMessageDisplay();
    void waitForEmulatedCommandBarToHide(long int timeout);
    void verifyShowsNumberOfReplacementsAcrossNumberOfLines(int numReplacements, int acrossNumLines);
};

class FailsIfSlotNotCalled : public QObject
{
    Q_OBJECT
public:
    FailsIfSlotNotCalled();
    ~FailsIfSlotNotCalled() override;
public Q_SLOTS:
    void slot();

private:
    bool m_slotWasCalled = false;
};

class FailsIfSlotCalled : public QObject
{
    Q_OBJECT
public:
    explicit FailsIfSlotCalled(const QString &failureMessage);
public Q_SLOTS:
    void slot();

private:
    const QString m_failureMessage;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
