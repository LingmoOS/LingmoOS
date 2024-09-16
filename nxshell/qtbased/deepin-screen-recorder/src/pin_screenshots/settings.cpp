// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

Settings *Settings::m_settingsInstance = nullptr;
Settings::Settings(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings("deepin/deepin-screen-recorder", "deepin-pin-screenshots", this);
}


Settings *Settings::instance()
{
    if (m_settingsInstance == nullptr) {
        m_settingsInstance = new Settings;
    }
    return m_settingsInstance;
}

void Settings::release()
{
    if (m_settingsInstance != nullptr) {
        delete m_settingsInstance;
        m_settingsInstance = nullptr;
    }
}

bool Settings::setSaveOption(const QPair<int, int> &value)
{
    m_settings->setValue("savePath", value.first);
    m_settings->setValue("saveFormat", value.second);
    return true;
}

QPair<int, int> Settings::getSaveOption()
{
    QPair<int, int> pair;
    pair.first = m_settings->value("savePath").toInt();
    pair.second = m_settings->value("saveFormat").toInt();
    return pair;
}

void Settings::setSavePath(const QString savePathDir)
{
    m_settings->setValue("savePathDir", savePathDir);
}

QString Settings::getSavePath()
{
    return m_settings->value("savePathDir").toString();
}

void Settings::setIsChangeSavePath(const bool isChange)
{
    m_settings->setValue("isChangeSavePath", isChange);
}

bool Settings::getIsChangeSavePath()
{
    return m_settings->value("isChangeSavePath").toBool();
}
