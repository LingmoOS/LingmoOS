/*
    SPDX-FileCopyrightText: 2014 Kevin Krammer <krammer@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KTRANSCRIPTCLEANTEST_H
#define KTRANSCRIPTCLEANTEST_H

#include <QLibrary>
#include <QObject>

class KTranscript;

/**
 * @brief Test the KTranscript implementation class
 *
 * Runs tests on the KTranscriptImp scripting facility.
 *
 * The main difference to the test ktranscripttest is that it
 * creates a new instance of KTranscriptImp for each test while
 * the main test re-uses one instance due to internal use of
 * Q_GLOBAL_STATIC
 *
 * Test that require a "clean slate" can be added here, tests that do not
 * should be added to both.
 */
class KTranscriptCleanTest : public QObject
{
    Q_OBJECT
public:
    KTranscriptCleanTest();

private Q_SLOTS:
    void init();
    void cleanup();

    void test_data();
    void test();

private:
    QLibrary m_library;
    KTranscript *m_transcript;
};

#endif /* KTRANSCRIPTCLEANTEST_H */
