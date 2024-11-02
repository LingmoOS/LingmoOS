/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPITEMMODEL_H
#define DESKTOPITEMMODEL_H

#include <QAbstractListModel>
#include <QQueue>
#include <QPoint>
#include <memory>
#include "user-dir-manager.h"
#include "desktop-icon-view.h"

namespace Peony {

class FileEnumerator;
class FileInfo;
class FileWatcher;
class FileOperationInfo;

class DesktopItemModel : public QAbstractListModel
{
    friend class DesktopIconView;
    Q_OBJECT
public:
    enum Role {
        UriRole = Qt::UserRole,
        IsLinkRole = Qt::UserRole + 1
    };
    Q_ENUM(Role)

    explicit DesktopItemModel(QObject *parent = nullptr);
    ~DesktopItemModel() override;

    const QModelIndex indexFromUri(const QString &uri);
    const QString indexUri(const QModelIndex &index);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRow(int row, const QModelIndex &parent = QModelIndex());

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QMimeData *mimeData(const QModelIndexList& indexes) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    bool acceptDropAction() const;
    void setAcceptDropAction(bool acceptDropAction);
    Peony::DesktopIconView *getIconView(const QString &uri);

Q_SIGNALS:
    void requestLayoutNewItem(const QString &uri);
    void requestClearIndexWidget(const QStringList &uris = QStringList());
    void requestUpdateItemPositions(const QString &uri = nullptr);
    void refreshed();

   // void fileCreated(const QString &uri);

    void prepareRefresh();

public Q_SLOTS:
    void refresh();

protected Q_SLOTS:
    void onEnumerateFinished(bool successed);
    void clearFloatItems();

private:
    FileEnumerator *m_enumerator;
    QList<std::shared_ptr<FileInfo>> m_files;
    QList<std::shared_ptr<FileInfo>> m_querying_files;
    std::shared_ptr<FileWatcher> m_trash_watcher;
    std::shared_ptr<FileWatcher> m_desktop_watcher;
    std::shared_ptr<FileWatcher> m_thumbnail_watcher; //just handle the thumbnail created.

//    std::shared_ptr<FileWatcher> m_system_app_watcher;
//    std::shared_ptr<FileWatcher> m_andriod_app_watcher;

    QQueue<QString> m_new_file_info_query_queue;

    QStringList m_items_need_relayout;
    QStringList m_destoryItems;
    UserdirManager * m_dir_manager;

    std::shared_ptr<FileInfo> m_desktop_info;
    std::shared_ptr<FileOperationInfo> m_renaming_operation_info;

    QPair<QString, QPoint> m_renaming_file_pos;

    bool m_accept_drop_action = true;
    bool m_showFileExtension = true;

private:
    void refreshInternal();
};

}

#endif // DESKTOPITEMMODEL_H
