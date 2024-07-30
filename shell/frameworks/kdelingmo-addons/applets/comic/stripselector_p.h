/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRIP_SELECTOR_P_H
#define STRIP_SELECTOR_P_H

#include "stripselector.h"

#include <QString>

class StringStripSelector : public StripSelector
{
public:
    explicit StringStripSelector(QObject *parent = nullptr);
    ~StringStripSelector() override;

    void select(const ComicData &currentStrip) override;
};

class NumberStripSelector : public StripSelector
{
public:
    explicit NumberStripSelector(QObject *parent = nullptr);
    ~NumberStripSelector() override;

    void select(const ComicData &currentStrip) override;
};

class DateStripSelector : public StripSelector
{
    Q_OBJECT

public:
    explicit DateStripSelector(QObject *parent = nullptr);
    ~DateStripSelector() override;

    void select(const ComicData &currentStrip) override;

private Q_SLOTS:
    void slotChosenDay(const QDate &date);

private:
    QString mFirstIdentifierSuffix;
};

#endif
