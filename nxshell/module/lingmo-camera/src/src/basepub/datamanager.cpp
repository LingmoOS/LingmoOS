// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datamanager.h"
#include "../capplication.h"

DataManager *DataManager::m_dataManager = nullptr;

QString &DataManager::getstrFileName()
{
    return m_strFileName;
}

enum DeviceStatus DataManager::getdevStatus()
{
    return m_devStatus;
}

void DataManager::setdevStatus(enum DeviceStatus devStatus)
{
    m_devStatus = devStatus;
}

void DataManager::setNowTabIndex(uint tabindex)
{
    m_tabIndexNow = tabindex;
}

uint DataManager::getNowTabIndex()
{
    return m_tabIndexNow;
}

void DataManager::setEncodeEnv(EncodeEnv env)
{
    m_encodeEnv = env;
}

EncodeEnv DataManager::encodeEnv()
{
#ifdef UNITTEST
    return FFmpeg_Env;
#endif
    return m_encodeEnv;
}

void DataManager::setEncExists(bool status)
{
    m_H264EncoderExists = status;
}

bool DataManager::encExists(){
    return m_H264EncoderExists;
}

DataManager *DataManager::instance()
{
    if (m_dataManager == nullptr) {
        m_dataManager = new DataManager;
    }
    return m_dataManager;
}

DataManager::DataManager()
{
    m_bTabEnter = false;
    m_tabIndex = 0;
    m_tabIndexNow = 0;
    m_strFileName.clear();
    m_videoCount = 0;
    m_encodeEnv = GStreamer_Env;
    m_devStatus = DeviceStatus::NOCAM;
    m_H264EncoderExists = false;
}

