// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dnotificationtypes.h"
#include <QString>
#include <QJsonObject>

DNOTIFICATIONS_BEGIN_NAMESPACE

namespace Utils {
    QVariantMap string2Map(const QString &text);
    QList<DNotificationData> json2DNotificationDatas(const QString & content);
    DNotificationData jsonObject2DNotificationData(const QJsonObject &obj);
}  // namespace Utils

DNOTIFICATIONS_END_NAMESPACE
