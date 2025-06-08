/*
    SPDX-FileCopyrightText: 2014-2016 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../../common/osdaction.h"
#include "osdservice_interface.h"

#include <QCoreApplication>
#include <QDBusConnection>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QString name = QStringLiteral("org.kde.kscreen.osdService");
    const QString path = QStringLiteral("/org/kde/kscreen/osdService");
    auto osdService = new OrgKdeKscreenOsdServiceInterface(name, path, QDBusConnection::sessionBus());

    QDBusReply<int> reply = osdService->showActionSelector();

    if (!reply.isValid()) {
        qDebug() << "Error calling osdService:";
        qDebug() << reply.error();
        return 1;
    }

    auto actionEnum = QMetaEnum::fromType<KScreen::OsdAction::Action>();
    const char *value = actionEnum.valueToKey(reply.value());
    if (!value) {
        qDebug() << "Got invalid action" << reply.value();
        return 1;
    }
    qDebug() << "Selected Action" << value;
    return 0;
}
