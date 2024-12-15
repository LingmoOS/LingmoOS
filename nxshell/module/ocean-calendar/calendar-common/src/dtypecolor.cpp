// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtypecolor.h"

#include "units.h"
#include "commondef.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DTypeColor::DTypeColor()
    : m_colorID("")
    , m_colorCode("")
    , m_privilege(PriUser)
{
}

DTypeColor::DTypeColor(const DTypeColor &typeColor)
    : m_colorID(typeColor.colorID())
    , m_colorCode(typeColor.colorCode())
    , m_privilege(typeColor.privilege())
    , m_dtCreate(typeColor.dtCreate())
{
}

QString DTypeColor::colorCode() const
{
    return m_colorCode;
}

void DTypeColor::setColorCode(const QString &colorCode)
{
    m_colorCode = colorCode;
}

QString DTypeColor::colorID() const
{
    return m_colorID;
}

void DTypeColor::setColorID(const QString &colorID)
{
    m_colorID = colorID;
}

DTypeColor::Privilege DTypeColor::privilege() const
{
    return m_privilege;
}

void DTypeColor::setPrivilege(const Privilege &privilege)
{
    m_privilege = privilege;
}

bool DTypeColor::isSysColorInfo()
{
    return this->privilege() == PriSystem;
}

bool DTypeColor::operator!=(const DTypeColor &color) const
{
    return this->colorID() != color.colorID() || this->colorCode() != this->colorCode() || this->privilege() != this->privilege();
}

DTypeColor::List DTypeColor::fromJsonString(const QString &colorJson)
{
    DTypeColor::List colorList;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(colorJson.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return colorList;
    }
    QJsonArray rootArr = jsonDoc.array();
    foreach (auto json, rootArr) {
        QJsonObject colorObj = json.toObject();
        DTypeColor::Ptr typeColor = DTypeColor::Ptr(new DTypeColor);
        if (colorObj.contains("colorID")) {
            typeColor->setColorID(colorObj.value("colorID").toString());
        }
        if (colorObj.contains("colorCode")) {
            typeColor->setColorCode(colorObj.value("colorCode").toString());
        }
        if (colorObj.contains("privilege")) {
            typeColor->setPrivilege(static_cast<Privilege>(colorObj.value("privilege").toInt()));
        }
        if (colorObj.contains("dtCreate")) {
            typeColor->setDtCreate(dtFromString(colorObj.value("dtCreate").toString()));
        }
        colorList.append(typeColor);
    }
    return colorList;
}

QString DTypeColor::toJsonString(const DTypeColor::List &colorList)
{
    QJsonArray rootArr;
    foreach (auto color, colorList) {
        QJsonObject colorObj;
        colorObj.insert("colorID", color->colorID());
        colorObj.insert("colorCode", color->colorCode());
        colorObj.insert("privilege", color->privilege());
        colorObj.insert("dtCreate", dtToString(color->dtCreate()));
        rootArr.append(colorObj);
    }
    QJsonDocument jsonDoc;
    jsonDoc.setArray(rootArr);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

QDateTime DTypeColor::dtCreate() const
{
    return m_dtCreate;
}

void DTypeColor::setDtCreate(const QDateTime &dtCreate)
{
    m_dtCreate = dtCreate;
}

bool operator<(const DTypeColor::Ptr &tc1, const DTypeColor::Ptr &tc2)
{
    if (tc1->privilege() != tc2->privilege()) {
        return tc1->privilege() < tc2->privilege();
    }

    if (tc1->dtCreate() != tc2->dtCreate()) {
        return tc1->dtCreate() < tc2->dtCreate();
    }
    return true;
}
