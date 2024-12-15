// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowmanager.h"
#include <QDebug>

WindowManager::WindowManager(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WindowManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_WindowList.size();
}

QVariant WindowManager::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= m_WindowList.size()) {
        return QVariant();
    }

    const AppRuntimeInfo &info = m_WindowList[index.row()];
    switch (role) {
    case NameRole:
        return info.name;
    case IdRole:
        return info.id;
    case StartTimeRole:
        return info.startTime.toMSecsSinceEpoch();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WindowManager::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[IdRole] = "id";
    roles[StartTimeRole] = "startTime";
    return roles;
}

void WindowManager::setWindowInfoForeground(const QString &name, uint id) {
    bool found = false;
    for (const auto &info : m_WindowList) {
        if (info.name == name) {
            found = true;
            break;
        }
    }
    if (!found) {
        AppRuntimeInfo info;
        info.name = "ForegroundApp："+name;
        info.id = id;
        info.startTime = QDateTime::currentDateTime();
        beginInsertRows(QModelIndex(), m_WindowList.size(), m_WindowList.size());
        m_WindowList.append(info);
        endInsertRows();
    }

}

void WindowManager::setWindowInfoBackground(const QString &name, uint id)
{
    bool found = false;
    for (const auto &info : m_WindowList) {
        if (info.name == name) {
            found = true;
            break;
        }
    }
    if (!found) {
        AppRuntimeInfo info;
        info.name = "BackgroundApp："+name;
        info.id = id;
        info.startTime = QDateTime::currentDateTime();
        beginInsertRows(QModelIndex(), m_WindowList.size(), m_WindowList.size());
        m_WindowList.append(info);
        endInsertRows();
    }
}

void WindowManager::WindowDestroyInfo(uint id)
{
    // Find the item to remove, matching only by id
    for (int i = 0; i < m_WindowList.size(); ++i) {
        if (m_WindowList[i].id == id) {
            // Start removing the row
            beginRemoveRows(QModelIndex(), i, i);
            m_WindowList.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

void WindowManager::setWindowInfoActive(uint id, const QString &name)
{
    for (int i = 0; i < m_WindowList.size(); ++i) {
        if (m_WindowList[i].id == id) {
            m_ActiveId = id;
            m_WindowList[i].name = "ForegroundApp："+name;
            emit dataChanged(index(i), index(i), {NameRole, IdRole, StartTimeRole});
            return;
        }
    }
}

void WindowManager::setWindowInfoInActive(uint id, const QString &name)
{
    for (int i = 0; i < m_WindowList.size(); ++i) {
        if (m_ActiveId!=0&&m_WindowList[i].id == m_ActiveId) {
            m_WindowList[i].name = "BackgroundApp："+m_WindowList[i].name.split("：")[1];
            m_ActiveId=0;
            emit dataChanged(index(i), index(i), {NameRole, IdRole, StartTimeRole});
            return;
        }
    }
}


