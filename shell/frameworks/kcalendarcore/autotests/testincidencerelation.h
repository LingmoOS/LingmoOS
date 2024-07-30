/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTINCIDENCERELATION_H
#define TESTINCIDENCERELATION_H

#include <QObject>

class IncidenceRelationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRelations();
};

#endif
