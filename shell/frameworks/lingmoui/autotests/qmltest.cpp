/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QQmlEngine>
#include <QtQuickTest>

#ifdef STATIC_MODULE
#include "lingmouiplugin.h"
Q_IMPORT_PLUGIN(LingmoUIPlugin)
#endif

class LingmoUISetup : public QObject
{
    Q_OBJECT

public:
    LingmoUISetup()
    {
    }

public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
#ifdef STATIC_MODULE
        LingmoUIPlugin::getInstance().registerTypes(engine);
#else
        Q_UNUSED(engine)
#endif
    }
};

QUICK_TEST_MAIN_WITH_SETUP(LingmoUI, LingmoUISetup)

#include "qmltest.moc"
