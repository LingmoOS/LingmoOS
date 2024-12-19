/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAUNCHERMODEL_H
#define LAUNCHERMODEL_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QLoggingCategory>
#include <QAbstractListModel>
#include <QSettings>
#include <QTimer>

#include "appitem.h"

class LauncherModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        AppIdRole = Qt::UserRole + 1,
        ApplicationRole,
        NameRole,
        GenericNameRole,
        CommentRole,
        IconNameRole,
        CategoriesRole,
        FilterInfoRole,
        PinnedRole,
        PinnedIndexRole,
        NewInstalledRole
    };
    Q_ENUM(Roles)

    enum Mode {
        NormalMode,
        SearchMode
    };
    Q_ENUM(Mode)

    explicit LauncherModel(QObject *parent = nullptr);
    ~LauncherModel();

    int count() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void search(const QString &key);
    Q_INVOKABLE void sendToDock(const QString &key);
    Q_INVOKABLE void sendToDesktop(const QString &key);
    Q_INVOKABLE void removeFromDock(const QString &desktop);

    int findById(const QString &id);

    static void refresh(LauncherModel *manager);

    Q_INVOKABLE void move(int from, int to, int page, int pageCount);
    Q_INVOKABLE void save();

    void delaySave();

public Q_SLOTS:
    Q_INVOKABLE bool launch(const QString &path);
    Q_INVOKABLE bool launch() { return launch(QString()); }

Q_SIGNALS:
    void countChanged();
    void refreshed();
    void applicationLaunched();

private Q_SLOTS:
    void onRefreshed();
    void onFileChanged(const QString &path);
    void addApp(const QString &fileName);
    void removeApp(const QString &fileName);

private:
    QList<AppItem> m_appItems;
    QList<AppItem> m_searchItems;

    QFileSystemWatcher *m_fileWatcher;

    QTimer m_saveTimer;
    QSettings m_settings;
    Mode m_mode;

    bool m_firstLoad;
};

#endif // LAUNCHERMODEL_H
