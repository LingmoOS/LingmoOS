// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginlistmodel.h"

#include <QDebug>
#include <QDir>
#include <QMetaType>
#include <QStandardPaths>
#include <QVariant>
#include <QtGlobal>
#include <QTranslator>
#include <QCoreApplication>

PluginListModel::PluginListModel(QObject *parent)
    : QAbstractListModel(parent)
{
#ifdef PLUGINPATH
    QStringList locations = QStandardPaths::standardLocations(
        QStandardPaths::AppDataLocation);
    locations.insert(0, PLUGINPATH);
#else
    const QStringList &locations = QStandardPaths::standardLocations(
        QStandardPaths::AppDataLocation);
#endif
    for (const auto &location : locations) {
        QDir dir(QString("%1/plugins").arg(location));
        for (auto d : dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
            qDebug() << dir.path();
            items << QString("file://%1/%2").arg(dir.path()).arg(d);
            // load translation
            QTranslator* translator = new QTranslator();
            if (translator->load(QLocale::system().name(), QString("%1/%2/translations").arg(dir.path()).arg(d))) {
                qApp->installTranslator(translator);
            }
        }
    }
}

PluginListModel::~PluginListModel() {}

int PluginListModel::rowCount(const QModelIndex &parent) const
{
    return items.length();
}

QVariant PluginListModel::data(const QModelIndex &index, int role) const
{
    return role == PathRole ? items[index.row()] : QAbstractItemModel::data(index, role);
}

QHash<int, QByteArray> PluginListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PathRole] = "path";
    return roles;
}
