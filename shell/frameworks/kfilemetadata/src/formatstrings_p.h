/*
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KFILEMETADATA_FORMATSTRINGS_P_H
#define KFILEMETADATA_FORMATSTRINGS_P_H

#include <QString>
#include <QVariant>

namespace KFileMetaData {

class Q_DECL_HIDDEN FormatStrings
{
public:
    static QString toStringFunction(const QVariant& value);

    static QString formatDouble(const QVariant& value);

    static QString formatDate(const QVariant& value);

    static QString formatDuration(const QVariant& value);

    static QString formatBitRate(const QVariant& value);

    static QString formatSampleRate(const QVariant& value);

    static QString formatOrientationValue(const QVariant& value);

    static QString formatPhotoFlashValue(const QVariant& value);

    static QString formatAsDegree(const QVariant& value);

    static QString formatAsMeter(const QVariant& value);

    static QString formatAsMilliMeter(const QVariant& value);

    static QString formatAsFrameRate(const QVariant& value);

    static QString formatPhotoTime(const QVariant& value);

    static QString formatPhotoExposureBias(const QVariant& value);

    static QString formatAspectRatio(const QVariant& value);

    static QString formatAsFNumber(const QVariant& value);

};

}

#endif
