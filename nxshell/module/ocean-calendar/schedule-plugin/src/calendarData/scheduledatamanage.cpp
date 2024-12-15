// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduledatamanage.h"
#include "accountmanager.h"
#include "accountmanager.h"

#include <QJsonArray>
#include <QJsonDocument>

CScheduleDataManage *CScheduleDataManage::m_vscheduleDataManage = new CScheduleDataManage;

//
CSchedulesColor CScheduleDataManage::getScheduleColorByType(const QString &type)
{
    CSchedulesColor color;
    DTypeColor colorinfo;
    DScheduleType::Ptr typePtr = gAccounManager->getScheduleTypeByScheduleTypeId(type);
    QColor typeColor;
    if (!typePtr.isNull()) {
        typeColor = QColor(gAccounManager->getScheduleTypeByScheduleTypeId(type)->typeColor().colorCode());
    }
    color.orginalColor = typeColor;
    color.normalColor = color.orginalColor;
    color.normalColor.setAlphaF(0.2);
    color.pressColor = color.orginalColor;
    color.pressColor.setAlphaF(0.35);

    color.hoverColor = color.orginalColor;
    color.hoverColor.setAlphaF(0.3);

    color.hightColor = color.orginalColor;
    color.hightColor.setAlphaF(0.35);

    return color;
}

QColor CScheduleDataManage::getSystemActiveColor()
{
    return DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
}

QColor CScheduleDataManage::getTextColor()
{
    return DGuiApplicationHelper::instance()->applicationPalette().text().color();
}

void CScheduleDataManage::setTheMe(int type)
{
    m_theme = type;
}

CScheduleDataManage *CScheduleDataManage::getScheduleDataManage()
{
    return m_vscheduleDataManage;
}

CScheduleDataManage::CScheduleDataManage(QObject *parent)
    : QObject(parent)
{
}

CScheduleDataManage::~CScheduleDataManage()
{
}
