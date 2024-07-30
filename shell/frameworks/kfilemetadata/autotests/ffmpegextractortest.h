/*
    SPDX-FileCopyrightText: 2019 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FFMPEGEXTRACTORTEST_H
#define FFMPEGEXTRACTORTEST_H

#include <QObject>
#include <QMimeDatabase>

namespace KFileMetaData {

class ffmpegExtractorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNoExtraction();
    void testVideoProperties();
    void testVideoProperties_data();
    void testMetaData();
    void testMetaData_data();

private:
    QMimeDatabase mimeDb;
};
} // namespace KFileMetaData

#endif // FFMPEGEXTRACTORTEST_H
