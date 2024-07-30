/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KTRANSCRIPTTEST_H
#define KTRANSCRIPTTEST_H

#include <QLibrary>
#include <QObject>

class KTranscript;

/**
 * @brief Test the KTranscript plugin
 *
 * Loads the KTranscript plugin and runs tests on its scripting capabilities.
 *
 * The main difference to the test ktranscriptcleantest is that it does so using
 * a single instance of the KTranscript implementation due to the plugin
 * using Q_GLOBAL_STATIC
 *
 * ktranscriptcleantest on the other hand creates and destroys the instance between
 * tests. Test that require a "clean slate" can be added there.
 */
class KTranscriptTest : public QObject
{
    Q_OBJECT
public:
    KTranscriptTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void test_data();
    void test();

private:
    QLibrary m_library;
    KTranscript *m_transcript;
};

#endif /* KTRANSCRIPTTEST_H */
