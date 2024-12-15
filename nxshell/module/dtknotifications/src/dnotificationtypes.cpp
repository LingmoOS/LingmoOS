// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dnotificationtypes.h"

DNOTIFICATIONS_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const DNotificationData &nofiticationData)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '('
                    << "name:" << nofiticationData.appName << ", " << "id:" << nofiticationData.id << ", "
                    << "icon:" << nofiticationData.appIcon << ", " << "summary:" << nofiticationData.summary << ", "
                    << "body:" << nofiticationData.body << ", " << "actions:" << nofiticationData.actions << ", "
                    << "hints:" << nofiticationData.hints << ", " << "ctime:" << nofiticationData.ctime << ", "
                    << "replacesid:" << nofiticationData.replacesId << ", " << "timeout:" << nofiticationData.timeout
                    << ')';
    return debug;
}

QDebug operator<<(QDebug debug, const ServerInformation &serverInformation)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << serverInformation.name << serverInformation.vendor
                    << serverInformation.version << serverInformation.spec_version;
    return debug;
}

DNOTIFICATIONS_END_NAMESPACE
