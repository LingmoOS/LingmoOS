// Copyright (C) 2011 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DSettings>

void GenerateSettingTranslate()
{
    auto settingName = QObject::tr("Settings");
    auto systemprotectionName = QObject::tr("System Protection");
    auto alarm_switchName = QObject::tr("Show warnings when the usage exceeds the thresholds");
    auto alarm_cpu_usageName = QObject::tr("CPU warning threshold (%)");
    auto alarm_memory_usageName = QObject::tr("Memory warning threshold (%)");
    auto alarm_intervalName = QObject::tr("Warning interval (min)");
    auto messgaeName = QObject::tr("Notifications");
}
