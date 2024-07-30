/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestHelper.h"

#include <helpersupport.h>

#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <qplatformdefs.h>

ActionReply TestHelper::echoaction(QVariantMap args)
{
    qDebug() << "Echo action running";
    ActionReply reply = ActionReply::SuccessReply();
    reply.setData(args);

    return reply;
}

ActionReply TestHelper::standardaction(QVariantMap args)
{
    qDebug() << "Standard action running";
    if (args.contains(QLatin1String("fail")) && args[QLatin1String("fail")].toBool()) {
        return ActionReply::HelperErrorReply();
    }

    return ActionReply::SuccessReply();
}

ActionReply TestHelper::longaction(QVariantMap args)
{
    Q_UNUSED(args);
    qDebug() << "Long action running. Don't be scared, this action takes 2 seconds to complete";

    for (int i = 1; i <= 100; i++) {
        if (HelperSupport::isStopped()) {
            break;
        }
        if (i == 50) {
            QVariantMap map;
            map.insert(QLatin1String("Answer"), 42);
            HelperSupport::progressStep(map);
        }
        HelperSupport::progressStep(i);
        QThread::usleep(20000);
    }

    return ActionReply::SuccessReply();
}

ActionReply TestHelper::failingaction(QVariantMap args)
{
    Q_UNUSED(args)
    return ActionReply::HelperErrorReply();
}

#include "moc_TestHelper.cpp"
