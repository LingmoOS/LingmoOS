// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CHANGEJSONDATA_H
#define CHANGEJSONDATA_H

#include "jsondata.h"

class changejsondata : public JsonData
{
public:
    changejsondata();
    /**
     * @brief fromDateTime  获取语义from时间数据
     * @return  时间数据
     */
    SemanticsDateTime fromDateTime() const;
    /**
     * @brief setFromDateTime       设置语义from时间数据
     * @param fromDateTime      时间数据
     */
    void setFromDateTime(const SemanticsDateTime &fromDateTime);
    /**
     * @brief toDateTime      获取语义toDateTime时间数据
     * @return   时间数据
     */
    SemanticsDateTime toDateTime() const;
    /**
     * @brief setToDateTime        设置语义toDateTime时间数据
     * @param toDateTime   时间数据
     */
    void setToDateTime(const SemanticsDateTime &toDateTime);

    QString toPlaceStr() const;
    void setToPlaceStr(const QString &toPlaceStr);
    /**
     * @brief isVaild   判断json是否为原始数据
     * @return  true为原始数据
     */
    bool isVaild() const override;

private:
    void jsonObjResolve(const QJsonObject &jsobj) override;
    void setDefaultValue() override;
    void fromDatetimeJsonResolve(const QJsonObject &jsobj);
    void toDatetimeJsonResolve(const QJsonObject &jsobj);
    void toPlaceJsonResolve(const QJsonObject &jsobj);

private:
    /**
     * @brief m_fromDateTime   需要修改的时间信息
     */
    SemanticsDateTime m_fromDateTime {};
    /**
     * @brief m_toDateTime   修改到的时间信息
     */
    SemanticsDateTime m_toDateTime {};
    /**
     * @brief m_toPlaceStr   修改的日常内容
     */
    QString m_toPlaceStr {};
};

#endif // CHANGEJSONDATA_H
