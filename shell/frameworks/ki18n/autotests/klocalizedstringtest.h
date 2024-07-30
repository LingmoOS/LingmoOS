/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLOCALIZEDSTRINGTEST_H
#define KLOCALIZEDSTRINGTEST_H

#include <QObject>
#include <QTemporaryDir>
#include <ki18n_export.h>

class KLocalizedStringTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void correctSubs();
    void wrongSubs();
    void removeAcceleratorMarker();
    void miscMethods();
    void translateToFrenchLowlevel();
    void translateToFrench();
    void addCustomDomainPath();

    void testThreads();

    void testLocalizedTranslator();
    void semanticTags();
    void setFormatForMarker();
    void multipleLanguages();
    void untranslatedText();
    void brokenTags();
    void brokenStructTagUsages();

    void testLazy();

private:
    bool m_hasFrench;
    bool m_hasCatalan;
    QTemporaryDir m_tempDir;
    bool compileCatalogs(const QStringList &catalogs, const QDir &dataDir, const QString &language);
};

#endif // KLOCALIZEDSTRINGTEST_H
