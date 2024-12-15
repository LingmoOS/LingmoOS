// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "changejsondata.h"

#include "../globaldef.h"

#include <QJsonObject>

changejsondata::changejsondata()
    : JsonData()
{
}

void changejsondata::jsonObjResolve(const QJsonObject &jsobj)
{
    if (jsobj[JSON_NAME] == JSON_FROMDATETIME) {
        fromDatetimeJsonResolve(jsobj);
        return;
    }
    if (jsobj[JSON_NAME] == JSON_TODATETIME) {
        toDatetimeJsonResolve(jsobj);
        return;
    }
    if (jsobj[JSON_NAME] == JSON_TOPLACE) {
        toPlaceJsonResolve(jsobj);
        return;
    }
}

void changejsondata::setDefaultValue()
{
    JsonData::setDefaultValue();
    m_fromDateTime.clear();
    m_toDateTime.clear();
    m_toPlaceStr.clear();
}

void changejsondata::fromDatetimeJsonResolve(const QJsonObject &jsobj)
{
    setFromDateTime(suggestDatetimeResolve(jsobj));
}

void changejsondata::toDatetimeJsonResolve(const QJsonObject &jsobj)
{
    setToDateTime(suggestDatetimeResolve(jsobj));
}

void changejsondata::toPlaceJsonResolve(const QJsonObject &jsobj)
{
    setToPlaceStr(jsobj[JSON_VALUE].toString());
}

QString changejsondata::toPlaceStr() const
{
    return m_toPlaceStr;
}

void changejsondata::setToPlaceStr(const QString &toPlaceStr)
{
    m_toPlaceStr = toPlaceStr;
}

bool changejsondata::isVaild() const
{
    return  this->m_toDateTime.suggestDatetime.size() ==0 
            && this->m_fromDateTime.suggestDatetime.size()==0 && this->toPlaceStr().isEmpty()
            && JsonData::isVaild();
}

SemanticsDateTime changejsondata::toDateTime() const
{
    return m_toDateTime;
}

void changejsondata::setToDateTime(const SemanticsDateTime &toDateTime)
{
    m_toDateTime = toDateTime;
}

SemanticsDateTime changejsondata::fromDateTime() const
{
    return m_fromDateTime;
}

void changejsondata::setFromDateTime(const SemanticsDateTime &fromDateTime)
{
    m_fromDateTime = fromDateTime;
}
