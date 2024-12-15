// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnotificationutils.h"
#include <QJsonDocument>
#include <QJsonArray>

DNOTIFICATIONS_BEGIN_NAMESPACE

#define ACTION_SEGMENT ("|")
#define HINT_SEGMENT ("|")
#define KEYVALUE_SEGMENT ("!!!")

namespace Utils {
    QVariantMap string2Map(const QString &text)
    {
        QVariantMap map;

        QStringList keyValueList = text.split(HINT_SEGMENT);
        foreach (QString text, keyValueList) {
            QStringList list = text.split(KEYVALUE_SEGMENT);
            if (list.size() != 2)
                continue;
            QString key = list[0];
            QVariant value = QVariant::fromValue(list[1]);

            map.insert(key, value);
        }

        return map;
    }

    DNotificationData jsonObject2DNotificationData(const QJsonObject &obj)
    {
        QStringList actions = obj.value("action").toString().split(ACTION_SEGMENT);
        auto notification = DNotificationData {
                                                obj.value("name").toString(),
                                                obj.value("id").toString(),
                                                obj.value("icon").toString(),
                                                obj.value("summary").toString(),
                                                obj.value("body").toString(),
                                                actions,
                                                string2Map(obj.value("hint").toString()),
                                                obj.value("time").toString(),
                                                obj.value("replacesid").toString().toUInt(),
                                                obj.value("timeout").toString()
                                               };
        return notification;
    }

    QList<DNotificationData> json2DNotificationDatas(const QString &content)
    {
        QList<DNotificationData> result;
        QJsonArray jsonArray = QJsonDocument::fromJson(content.toLocal8Bit()).array();
        foreach (auto json, jsonArray) {
            QJsonObject jsonObj = json.toObject();
            auto notificationData = Utils::jsonObject2DNotificationData(jsonObj);
            result << notificationData;
        }
        return result;
    }

}

DNOTIFICATIONS_END_NAMESPACE
