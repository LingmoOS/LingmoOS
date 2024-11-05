/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEITEM_H
#define FILEITEM_H

#include "explorer-core_global.h"
#include <memory>

#include <QObject>
#include <QVector>
#include <QHash>

class QTimer;

namespace Peony {

class FileInfo;
class FileItemModel;
class FileWatcher;
class FileItemProxyFilterSortModel;
class FileEnumerator;
class BatchProcessItems;
class ExtraInfoRecorder;

/*!
 * \brief The FileItem class
 * <br>
 * FileItem is the absctract item class contract with FileItemModel.
 * The different from FileInfo to FileItem is that FileItem has concept of
 * children and parent. This makes FileItem instance has a tree struction and
 * can represent a tree item in a view(non-tree as well). Other different is
 * that FileItem instance is not shared. You can hold many FileItem instances
 * crosponding to the same FileInfo, but they are allocated in their own memory
 * space. Every FileItem instance which has children will aslo support
 * monitoring. When find the children of the item, it will start a monitor for
 * this directory.
 * </br>
 * \note
 * Actually, every FileItem instance should bind with an model instance,
 * otherwise it will be useless.
 */
class PEONYCORESHARED_EXPORT FileItem : public QObject
{
    friend class FileItemProxyFilterSortModel;
    friend class FileItemModel;
    Q_OBJECT
public:
    explicit FileItem(std::shared_ptr<Peony::FileInfo> info,
                      FileItem *parentItem = nullptr,
                      FileItemModel *model = nullptr,
                      QObject *parent = nullptr);
    ~FileItem();

    const QString uri();
    const std::shared_ptr<FileInfo> info() {
        return m_info;
    }

    bool operator == (const FileItem &item);

    QVector<FileItem*> *findChildrenSync();
    void findChildrenAsync();

    /*!
     * \brief firstColumnIndex
     * \return first column index of item in model.
     * \see FileItemModel::firstColumnIndex().
     */
    QModelIndex firstColumnIndex();
    /*!
     * \brief lastColumnIndex
     * \return last column index of item in model.
     * \see FileItemModel::lastColumnIndex()
     */
    QModelIndex lastColumnIndex();

    bool hasChildren();
    bool shouldShow();

Q_SIGNALS:
    void cancelFindChildren();
    void childAdded(const QString &uri);
    void childRemoved(const QString &uri);
    void deleted(const QString &thisUri);
    void renamed(const QString &oldUri, const QString &newUri);

public Q_SLOTS:
    void onChildAdded(const QString &uri);
    void onChildRemoved(const QString &uri);
    void onDeleted(const QString &thisUri);
    void onRenamed(const QString &oldUri, const QString &newUri);
    void onChanged(const QString &uri);

    void onUpdateDirectoryRequest();

    void clearChildren();

protected:
    /*!
     * \brief getChildFromUri
     * \param uri
     * \return child item
     * \note
     * This is ususally used when fileCreated() and fileDeleted() happened,
     * and item must has parent item.
     */
    FileItem *getChildFromUri(QString uri);

    /*!
     * \brief updateInfoSync
     * <br>
     * Update the item info synchously.
     * </br>
     * \note
     * This is ususally used when fileCreated() and fileDeleted() happened,
     * and item must has parent item.
     */
    void updateInfoSync();
    /*!
     * \brief updateInfoAsync
     * <br>
     * Update the item info asynchously.
     * </br>
     * This is ususally used when fileCreated() and fileDeleted() happened,
     * and item must has parent item.
     */
    void updateInfoAsync();

    void removeChildren();
    void batchRemoveItems();

    void showFilesForBurningOnRTypeDisc();/* udf刻录与文管适配，R类型光盘遍历家目录下的“.cache/LingmoTransitBurner/”获取缓冲数据;将缓冲数据显示在光盘挂载目录下 */

private:
    void connectFunc();
    void childrenUpdateOfEnumerate(const QStringList &uris, bool isEnding);


private:
    FileItem *m_parent = nullptr;
    std::shared_ptr<Peony::FileInfo> m_info;
    QVector<FileItem*> *m_children = nullptr;
    QHash<QString, FileItem*> m_uri_item_hash; /* <key:uri,value:fileItem> 必须与m_children同增减！！！ */
    FileItemModel *m_model = nullptr;
    std::shared_ptr<ExtraInfoRecorder> m_extraInfoRecorder = nullptr;/* 记录文件缩略图和角标等附加信息的引用 */

    bool m_expanded = false;
    bool m_isRTypeDisc = false;
    bool m_isEndOfEnumerate = false;

    std::shared_ptr<FileWatcher> m_watcher = nullptr;
    std::shared_ptr<FileWatcher> m_rTypeDiscWatcher = nullptr;/* R类型光盘刻录缓冲数据的监听 */
    std::shared_ptr<FileWatcher> m_thumbnail_watcher = nullptr;

    QStringList m_ending_uris;
    QStringList m_waiting_add_queue;
    QStringList m_waiting_update_queue;
    QStringList m_uris_to_be_removed;

    QTimer *m_idle = nullptr;
    QTimer *m_addChildTimer = nullptr;
    QTimer *m_changeChildTimer = nullptr;

    QThread *m_batchProcessThread = nullptr;
    BatchProcessItems *m_batchProcessItems = nullptr;

    /*!
     * \brief m_async_count
     * <br>
     * when enumerate children finished, we start a async job for update children info.
     * this count is record the current last un-updated children count.
     * while all job finished, the count will clear, and we can insert the rows to model.
     * </br>
     */
    int m_async_count = 0;

    /*!
     * \brief m_backend_enumerator
     * \note
     * only used in directory not support monitor.
     */
    FileEnumerator *m_backend_enumerator;
};


class BatchProcessItems: public QObject{
    Q_OBJECT

public:
    BatchProcessItems();
    ~BatchProcessItems();

    void setBatchRemoveParam(const QStringList& uris_to_be_removed, const QHash<QString, FileItem*>& uri_item_hash, QVector<FileItem*> *children);

Q_SIGNALS:
    void removeItemsFinished(QVector<FileItem*> *children, const QHash<QString, FileItem*> &uri_item_hash, const QVector<QString>& needHandleLabelUris);


public Q_SLOTS:
    void slot_removeItems();

private:
    QStringList m_uris_to_be_removed;
    QHash<QString, FileItem*> m_uri_item_hash;
    QVector<FileItem*> *m_children = nullptr;
};

/* 该类暂用于为了兼容性能优化 批量处理后析构缩略图和角标 */
class ExtraInfoRecorder{
public:
    explicit ExtraInfoRecorder(const QString &uri);
    ~ExtraInfoRecorder();

public:
    const QString uri();

private:
    QString m_uri;

};
}

#endif // FILEITEM_H
