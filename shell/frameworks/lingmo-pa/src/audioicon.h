/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef AUDIOICON_H
#define AUDIOICON_H

#include <QObject>
#include <QString>

class AudioIcon : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(int HIGH_UPPER_BOUND MEMBER HIGH_UPPER_BOUND CONSTANT)
    Q_PROPERTY(int VERY_HIGH_UPPER_BOUND MEMBER VERY_HIGH_UPPER_BOUND CONSTANT)
    static constexpr int HIGH_UPPER_BOUND = 100;
    static constexpr int VERY_HIGH_UPPER_BOUND = 125;

public:
    Q_INVOKABLE static QString forVolume(int volume, bool muted, QString prefix);
    Q_INVOKABLE static QString forFormFactor(QString formFactor);
};

#endif // AUDIOICON_H
