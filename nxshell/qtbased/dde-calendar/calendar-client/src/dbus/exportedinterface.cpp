// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exportedinterface.h"
#include "scheduledatamanage.h"
#include "calendarmainwindow.h"
#include "cscheduleoperation.h"
#include "accountmanager.h"
#include "units.h"
#include "compatibledata.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

ExportedInterface::ExportedInterface(QObject *parent)
    : Dtk::Core::DUtil::DExportedInterface(parent)
{
    m_object = parent;
}

QVariant ExportedInterface::invoke(const QString &action, const QString &parameters) const
{
    //对外接口数据设置
    DSchedule::Ptr info;
    Exportpara para;
    QString tstr = parameters;
    CScheduleOperation _scheduleOperation;

    if (!analysispara(tstr, info, para)) {
        return QVariant(false);
    }

    if (action == "CREATE") {
        // 创建日程
        if (info.isNull()) {
            return QVariant(false);
        }
        bool _createSucc = _scheduleOperation.createSchedule(info);
        //如果创建失败
        if (!_createSucc) {
            return QVariant(false);
        }
    } else if (action == "VIEW") {
        dynamic_cast<Calendarmainwindow *>(m_object)->viewWindow(para.viewType);
    } else if (action == "QUERY") {
        if (gLocalAccountItem) {
            DSchedule::Map scheduleMap = DSchedule::fromMapString(gLocalAccountItem->querySchedulesByExternal(para.ADTitleName, para.ADStartTime, para.ADEndTime));
            QString qstr = DDE_Calendar::getExternalSchedule(scheduleMap);
            return QVariant(qstr);
        } else {
            return "";
        }
    } else if (action == "CANCEL") {
        //对外接口删除日程
        QMap<QDate, DSchedule::List> out;
        //口查询日程
        if (gLocalAccountItem && gLocalAccountItem->querySchedulesByExternal(para.ADTitleName, para.ADStartTime, para.ADEndTime, out)) {
            //删除查询到的日程
            QMap<QDate, DSchedule::List>::const_iterator _iterator = nullptr;
            for (_iterator = out.constBegin(); _iterator != out.constEnd(); ++_iterator) {
                for (int i = 0 ; i < _iterator.value().size(); ++i) {
                    _scheduleOperation.deleteOnlyInfo(_iterator.value().at(i));
                }
            }
        } else {
            return QVariant(false);
        }
    }
    return QVariant(true);
}

bool ExportedInterface::analysispara(QString &parameters, DSchedule::Ptr &info, Exportpara &para) const
{
    //如果是创建则info有效
    //如果是其他则para有效
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(parameters.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    //数据反序列化
    info = DDE_Calendar::getScheduleByExported(parameters);

    if (rootObj.contains("ViewName")) {
        para.viewType = rootObj.value("ViewName").toInt();
    }
    if (rootObj.contains("ViewTime")) {
        para.viewTime = QDateTime::fromString(rootObj.value("ViewTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    if (rootObj.contains("ADTitleName")) {
        para.ADTitleName = rootObj.value("ADTitleName").toString();
    }
    if (rootObj.contains("ADStartTime")) {
        para.ADStartTime = QDateTime::fromString(rootObj.value("ADStartTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    if (rootObj.contains("ADEndTime")) {
        para.ADEndTime = QDateTime::fromString(rootObj.value("ADEndTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    return true;
}

