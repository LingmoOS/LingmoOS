/*
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef APPIMAGEEXTRACTORTEST_H
#define APPIMAGEEXTRACTORTEST_H

#include <QObject>

class AppImageExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void test();
};

#endif
