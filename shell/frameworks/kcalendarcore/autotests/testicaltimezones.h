/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2005 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTICALTIMEZONES_H
#define TESTICALTIMEZONES_H

#include <QObject>

class ICalTimeZonesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    /** @brief Check that the transitions for Prague are sensible.
     *
     * The database of transitions can vary (per Qt version and platform),
     * and the test is sensitive to which transition is picked.
     */
    void testPragueTransitions();
    void parse_data();
    void parse();
    void write();
};

#endif
