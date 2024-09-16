// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "queryjsondata.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonDocument>

#include "../globaldef.h"

QueryJsonData::QueryJsonData()
    : JsonData()
{
}
