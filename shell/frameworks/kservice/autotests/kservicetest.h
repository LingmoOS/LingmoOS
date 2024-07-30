/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KSERVICETEST_H
#define KSERVICETEST_H

#include <kservice_export.h>

#include <QAtomicInt>
#include <QObject>

class KServiceTest : public QObject
{
    Q_OBJECT
public:
    KServiceTest()
        : m_sycocaUpdateDone(0)
    {
    }
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testByName();
    void testConstructorFullPath();
    void testConstructorKDesktopFile();
    void testCopyConstructor();
    void testCopyInvalidService();
    void testProperty();
    void testAllServices();
    void testSubseqConstraints();
    void testByStorageId();
    void testActionsAndDataStream();
    void testServiceGroups();
    void testDeletingService();
    void testReaderThreads();
    void testThreads();
    void testCompleteBaseName();
    void testEntryPathToName();
    void testMimeType();
    void testProtocols();
    void testUntranslatedNames();

    void testAliasFor();
    void testServiceActionService();
    void testStartupNotify();

private:
    void runKBuildSycoca(bool noincremental = false);

    QAtomicInt m_sycocaUpdateDone;
    bool m_hasNonCLocale;
};

#endif
