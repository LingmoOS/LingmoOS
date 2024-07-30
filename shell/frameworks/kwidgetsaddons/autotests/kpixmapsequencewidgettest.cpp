/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <KPixmapSequenceWidget>

#include <QTest>

#include "kguiitem.h"

static const QString INACTIVE_TEXT = QStringLiteral("Show Foo");
static const QString ACTIVE_TEXT = QStringLiteral("Hide Foo");

class KPixmapSequenceWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testConstructor()
    {
        KPixmapSequenceWidget w;
    }
};

QTEST_MAIN(KPixmapSequenceWidgetTest)

#include "kpixmapsequencewidgettest.moc"
