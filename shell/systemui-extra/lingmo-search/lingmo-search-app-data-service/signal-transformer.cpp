/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "signal-transformer.h"

QMutex SignalTransformer::s_mutex;

SignalTransformer *SignalTransformer::getTransformer()
{
    static SignalTransformer *instance = new SignalTransformer();
    return instance;
}

void SignalTransformer::transform()
{
    switch (m_signalType) {
    case Delete:
        if (!m_items2BDelete.isEmpty()) {
            Q_EMIT this->appDBItemsDelete(m_items2BDelete);
            m_items2BDelete.clear();
        }
        break;
    case Insert:
        if (!m_items2BInsert.isEmpty()) {
            Q_EMIT this->appDBItemsAdd(m_items2BInsert);
            m_items2BInsert.clear();
        }
        break;
    case UpdateAll:
        if (!m_items2BUpdateAll.isEmpty()) {
            Q_EMIT this->appDBItemsUpdateAll(m_items2BUpdateAll);
            m_items2BUpdateAll.clear();
        }
        break;
    case Update:
        if (!m_items2BUpdate.isEmpty()) {
            Q_EMIT this->appDBItemsUpdate(m_items2BUpdate);
            m_items2BUpdate.clear();
        }
        break;
    default:
        break;
    }
}

void SignalTransformer::handleItemInsert(const QString &desktopFilePath)
{
    QMutexLocker locker(&s_mutex);
    if ((m_signalType ^ SignalType::Invalid) && (m_signalType ^ SignalType::Insert)) {
        transform();
    }
    m_signalType = SignalType::Insert;
    m_items2BInsert.append(desktopFilePath);
}

void SignalTransformer::handleItemUpdate(const ApplicationInfoMap &item)
{
    QMutexLocker locker(&s_mutex);
    if ((m_signalType ^ SignalType::Invalid) && (m_signalType ^ SignalType::Update)) {
        transform();
    }
    m_signalType = SignalType::Update;
    for(auto it = item.constBegin(); it != item.constEnd(); it++) {
        ApplicationPropertyMap propertyinfo = it.value();
        for (auto i = propertyinfo.constBegin(); i != propertyinfo.constEnd(); i++) {
            m_items2BUpdate[it.key()].insert(i.key(), i.value());
        }
    }
}

void SignalTransformer::handleItemUpdateAll(const QString &desktopFilePath)
{
    QMutexLocker locker(&s_mutex);
    if ((m_signalType ^ SignalType::Invalid) && (m_signalType ^ SignalType::UpdateAll)) {
        transform();
    }
    m_signalType = SignalType::UpdateAll;
    m_items2BUpdateAll.append(desktopFilePath);
}

void SignalTransformer::handleItemDelete(const QString &desktopFilePath)
{
    QMutexLocker locker(&s_mutex);
    if ((m_signalType ^ SignalType::Invalid) && (m_signalType ^ SignalType::Delete)) {
        transform();
    }
    m_signalType = SignalType::Delete;
    m_items2BDelete.append(desktopFilePath);
}

void SignalTransformer::handleSignalTransform()
{
    QMutexLocker locker(&s_mutex);
    if (!m_items2BInsert.isEmpty()) {
        Q_EMIT this->appDBItemsAdd(m_items2BInsert);
        m_items2BInsert.clear();
    }

    if (!m_items2BUpdate.isEmpty()) {
        Q_EMIT this->appDBItemsUpdate(m_items2BUpdate);
        m_items2BUpdate.clear();
    }

    if (!m_items2BUpdateAll.isEmpty()) {
        Q_EMIT this->appDBItemsUpdateAll(m_items2BUpdateAll);
        m_items2BUpdateAll.clear();
    }

    if (!m_items2BDelete.isEmpty()) {
        Q_EMIT this->appDBItemsDelete(m_items2BDelete);
        m_items2BDelete.clear();
    }
}

SignalTransformer::SignalTransformer(QObject *parent) : QObject(parent)
{
    connect(AppDBManager::getInstance(), &AppDBManager::appDBItemAdd, this, &SignalTransformer::handleItemInsert);
    connect(AppDBManager::getInstance(), &AppDBManager::appDBItemUpdate, this, &SignalTransformer::handleItemUpdate);
    connect(AppDBManager::getInstance(), &AppDBManager::appDBItemDelete, this, &SignalTransformer::handleItemDelete);
    connect(AppDBManager::getInstance(), &AppDBManager::appDBItemUpdateAll, this, &SignalTransformer::handleItemUpdateAll);
    connect(AppDBManager::getInstance(), &AppDBManager::finishHandleAppDB, this, &SignalTransformer::handleSignalTransform);
}

