/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

// KF
#include <KConfigGroup>
#include <QDateTime>

class ConfigStorage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLunarPhaseShown MEMBER m_lunarPhaseShown NOTIFY lunarPhaseShownChanged)
    Q_PROPERTY(bool isSeasonShown MEMBER m_seasonShown NOTIFY seasonShownChanged)

public:
    explicit ConfigStorage(QObject *parent = nullptr);

    Q_INVOKABLE void save();

Q_SIGNALS:
    void lunarPhaseShownChanged();
    void seasonShownChanged();

private:
    KConfigGroup m_generalConfigGroup;

    bool m_lunarPhaseShown;
    bool m_seasonShown;
};

#endif
