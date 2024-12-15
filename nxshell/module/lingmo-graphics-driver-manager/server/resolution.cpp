// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resolution.h"

#include <QDebug>


Resolution::Resolution()
{
    
}


bool Resolution::isNull() {
    return m_resName.isEmpty();
}

Resolution::Resolution(const QJsonObject &info)
{

    QJsonValue shouldDisplay = info.value("display");
    if (shouldDisplay == QJsonValue::Undefined) {
        m_display = true;
    } else {
        m_display = shouldDisplay.toBool();
    }

    m_resId = info["id"].toInt();
    m_keepGLTest = info["keep_gltest"].toBool();
    m_resName = info["name"].toString();
    m_resdriver = info["driver"].toString();
    m_resTitle = info["title"].toString();
    m_resDescription = info["description"].toString();
    m_installScript = info["install"].toString();
    m_removeScript = info["remove"].toString();
}
