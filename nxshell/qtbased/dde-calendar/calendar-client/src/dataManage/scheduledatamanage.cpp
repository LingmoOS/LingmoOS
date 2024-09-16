// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduledatamanage.h"
#include "accountmanager.h"
#include "cscheduleoperation.h"
#include "accountmanager.h"

#include <QJsonArray>
#include <QJsonDocument>

CScheduleDataManage *CScheduleDataManage::m_vscheduleDataManage = nullptr;

//
CSchedulesColor CScheduleDataManage::getScheduleColorByType(const QString &typeId)
{
    CSchedulesColor color;
    DScheduleType::Ptr type = gAccountManager->getScheduleTypeByScheduleTypeId(typeId);
    QColor typeColor;
    if (nullptr != type) {
        typeColor = type->typeColor().colorCode();
    } else if (typeId =="other"){
            //如果类型不存在则设置一个默认颜色
            typeColor = QColor("#BA60FA");
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
    if (nullptr == m_vscheduleDataManage) {
        m_vscheduleDataManage = new CScheduleDataManage();
    }
    return m_vscheduleDataManage;
}

CScheduleDataManage::CScheduleDataManage(QObject *parent)
    : QObject(parent)
{
}

CScheduleDataManage::~CScheduleDataManage()
{
}
