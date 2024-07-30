/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "networkitemslist.h"
#include "networkmodelitem.h"

NetworkItemsList::NetworkItemsList(QObject *parent)
    : QObject(parent)
{
}

NetworkItemsList::~NetworkItemsList()
{
    qDeleteAll(m_items);
}

bool NetworkItemsList::contains(const NetworkItemsList::FilterType type, const QString &parameter) const
{
    for (NetworkModelItem *item : m_items) {
        switch (type) {
        case NetworkItemsList::ActiveConnection:
            if (item->activeConnectionPath() == parameter) {
                return true;
            }
            break;
        case NetworkItemsList::Connection:
            if (item->connectionPath() == parameter) {
                return true;
            }
            break;
        case NetworkItemsList::Device:
            if (item->devicePath() == parameter) {
                return true;
            }
            break;
        case NetworkItemsList::Name:
            if (item->name() == parameter) {
                return true;
            }
            break;
        case NetworkItemsList::Ssid:
            if (item->ssid() == parameter) {
                return true;
            }
            break;
        case NetworkItemsList::Uuid:
            if (item->uuid() == parameter) {
                return true;
            }
            break;
        case NetworkItemsList::Type:
            break;
        default:
            break;
        }
    }

    return false;
}

int NetworkItemsList::count() const
{
    return m_items.count();
}

int NetworkItemsList::indexOf(NetworkModelItem *item) const
{
    return m_items.indexOf(item);
}

void NetworkItemsList::insertItem(NetworkModelItem *item)
{
    m_items << item;
}

NetworkModelItem *NetworkItemsList::itemAt(int index) const
{
    return m_items.at(index);
}

QList<NetworkModelItem *> NetworkItemsList::items() const
{
    return m_items;
}

void NetworkItemsList::removeItem(NetworkModelItem *item)
{
    m_items.removeAll(item);
}

QList<NetworkModelItem *>
NetworkItemsList::returnItems(const NetworkItemsList::FilterType type, const QString &parameter, const QString &additionalParameter) const
{
    QList<NetworkModelItem *> result;

    for (NetworkModelItem *item : m_items) {
        switch (type) {
        case NetworkItemsList::ActiveConnection:
            if (item->activeConnectionPath() == parameter) {
                result << item;
            }
            break;
        case NetworkItemsList::Connection:
            if (item->connectionPath() == parameter) {
                if (additionalParameter.isEmpty()) {
                    result << item;
                } else {
                    if (item->devicePath() == additionalParameter) {
                        result << item;
                    }
                }
            }
            break;
        case NetworkItemsList::Device:
            if (item->devicePath() == parameter) {
                result << item;
            }
            break;
        case NetworkItemsList::Name:
            if (item->name() == parameter) {
                result << item;
            }
            break;
        case NetworkItemsList::Ssid:
            if (item->ssid() == parameter) {
                if (additionalParameter.isEmpty()) {
                    result << item;
                } else {
                    if (item->devicePath() == additionalParameter) {
                        result << item;
                    }
                }
            }
            break;
        case NetworkItemsList::Uuid:
            if (item->uuid() == parameter) {
                result << item;
            }
            break;
        case NetworkItemsList::Type:
            break;
        }
    }

    return result;
}

QList<NetworkModelItem *> NetworkItemsList::returnItems(const NetworkItemsList::FilterType type,
                                                        NetworkManager::ConnectionSettings::ConnectionType typeParameter) const
{
    QList<NetworkModelItem *> result;

    for (NetworkModelItem *item : m_items) {
        if (type == NetworkItemsList::Type) {
            if (item->type() == typeParameter) {
                result << item;
            }
        }
    }
    return result;
}

#include "moc_networkitemslist.cpp"
