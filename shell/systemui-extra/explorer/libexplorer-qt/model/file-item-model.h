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

#ifndef FILEITEMMODEL_H
#define FILEITEMMODEL_H

#include <QAbstractItemModel>
#include "explorer-core_global.h"
#include <QThread>



namespace Peony {
class GlobalSettings;
class FileItem;
class FileItemProxyFilterSortModel;
class FileInfo;
class FileManagerThread;
class FileInfosJob;

/*!
 * \brief The FileItemModel class
 * <br>
 * FileItemModel is the model of FileItem. For now, this class doesn't hold and
 * manage the data. This class is just supply the common interface to
 * QAbstractItemView based classes. FileItem must bind to a model instance, so
 * that it could tell the view how to show its or its children's data through
 * the model.
 * </br>
 * \bug
 * If we setRootItem() too frequently, there is a certain cause of crash.
 * I guess it is because the glib async callback call after the user data (enumerator)
 * has been deleted. I have no idea how to fix this bug now, maybe set a restriction
 * of frequency of model root change in UI is one solution.
 */
class PEONYCORESHARED_EXPORT FileItemModel : public QAbstractItemModel
{
    friend class FileItem;
    friend class FileItemProxyFilterSortModel;
    Q_OBJECT
public:
    enum ColumnType {
        FileName,
        ModifiedDate,
        FileType,
        FileSize,
        TrashOriginPath,
        Owner,
        Other
    };
    Q_ENUM(ColumnType)

    enum ItemRole {
        UriRole = Qt::UserRole
    };
    Q_ENUM(ItemRole)

    enum OperateType {
        Enumerate,
        Add,
        Change
    };
    Q_ENUM(OperateType)

    explicit FileItemModel(QObject *parent = nullptr);
    ~FileItemModel() override;

    /*!
     * \brief setPositiveResponse
     * \param positive
     * \details
     * FileItem provide 2 kinds of data query type for find children,
     * positive and inpositive. The positive query will report children
     * were found when the FileEnumerator::childrenUpdated() signal emitted,
     * this signal take a string list of uris enumerator found at that time (
     * usually a batch of async enumeration callbacked results).
     * and the inpositive query will report children being found the same time
     * enumerationFinished() emitted, and use FileEnumerator::getChildren() get
     * all children at once.
     *
     * The default positive type is inpositive.
     */
    void setPositiveResponse(bool positive = true) {
        m_is_positive = positive;
    }

    /*!
     * \brief isPositiveResponse
     * \return
     * \see setPositiveResponse()
     */
    bool isPositiveResponse() {
        return m_is_positive;
    }

    void setExpandable(bool expandable) {
        m_can_expand = expandable;
    }
    bool canExpandChildren() {
        return  m_can_expand;
    }

    const QString getRootUri();
    void setRootUri(const QString &uri);
    /*!
     * \brief setRootItem
     * \param item, the directory should be shown in view.
     * <br>
     * once setRootItem() is called, the current root and it children will be delete,
     * it will start a new enumerating and monitoring for new item.
     * </br>
     */
    void setRootItem(FileItem *item);
    /*!
     * \brief itemFromIndex
     * \param index
     * \return
     * \retval the item instance of index at model.
     */
    FileItem *itemFromIndex(const QModelIndex &index) const;
    /*!
     * \brief firstColumnIndex
     * \param item
     * \return
     * \retval the first column index(FileName) which crosponding this item.
     * \note Every index's internal data at same row is the same item.
     */
    QModelIndex firstColumnIndex(FileItem *item);
    /*!
     * \brief lastColumnIndex
     * \param item
     * \return
     * \retval the last column index which crosponding this item.
     * \note Every index's internal data at same row is the same item.
     */
    QModelIndex lastColumnIndex(FileItem *item);

    const QModelIndex indexFromUri(const QString &uri);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool hasChildren(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /*!
     * \brief canFetchMore
     * \param parent
     * \return
     * \retval true if tree view item has children and first expanded.
     * \retval false
     * <br>
     * QAbstractItemModel provide a lazy populate interface.
     * canFetchMore() is most happening in tree view. If a parent index has children,
     * hasChildren() should return true. Then tree view will set this parent index expandable.
     * if we expand the item, canFetchMore() will be called. we can dynamicly load our data after
     * set this method return true. After loading finished, we must set this method return fasle,
     * for telling the view data loading is done.
     * </br>
     * <br>
     * This desgin has a small defect. Once we expanded a parent index,
     * the data will not free until we delete them manually.
     * I know using custom view and model with a custom signal-control-mechanism can solve this problem,
     * but I don't want to let my model become sexceptional.
     * Just like QFileSystemModel does, I want my model perform well at all kinds of View,
     * whatever List, Icon, Tree or others.
     * </br>
     */
    bool canFetchMore(const QModelIndex &parent) const override;
    /*!
     * \brief fetchMore
     * \param parent
     * <br>
     * This method will shcedule when a parent index canFetchMore.
     * In this method, we usually load our data, and call beginInsertRows
     * to tell how many rows we has added into this index.
     * Then we need call endInsertRows(), and return. Actually, fetchMore just
     * tell model the new rowCount of parent index to control showing our newly data.
     * newly data will be shown after data() called, as same as other non-delayed data.
     * </br>
     */
    void fetchMore(const QModelIndex &parent) override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    QMimeData *mimeData(const QModelIndexList& indexes) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    void sendPathChangeRequest(const QString &destUri, const QString &sourceUri);

    void setShowFileExtensions(bool show);

private:
    const QModelIndex indexFromItemAndUri(FileItem *item, const QString &uri);

Q_SIGNALS:
    /*!
     * \brief findChildrenStarted
     * <br>
     * This signal is use for telling other object that the item has started enumerating.
     * An icon view(or list view) can connect this signal for setting cursor as style-loading.
     * </br>
     * \see findChildrenFinished()
     */
    void findChildrenStarted();
    /*!
     * \brief findChildrenFinished
     * <br>
     * This signal is use for telling other object that the item has finished loading.
     * An icon view(or list view) can connect this signal for setting cursor as normal.
     * </br>
     * \see findChildrenStarted().
     */
    void findChildrenFinished();

    /*!
     * \brief updated
     * <br>
     * when a 'folder' item children changed, this signal should be emit.
     * </br>
     * \note proxy model should connect this signal and start sort and filter again.
     */
    void updated();

    void selectRequest(const QStringList &uris);
    void changePathRequest(const QString &destUri, const QString &sourceUri);

    void signal_itemAdded(const QString& uri);/* 新增文件（夹），item创建完成 */
    void thumbnailUpdated(const QString& uri);

    void setUrisForBatchQueryInfos(const QStringList& uris, /*OperateType*/int operateType, FileItem *parentItem);
    void cancelBatchQuery();

public Q_SLOTS:
    /*!
     * \brief onFoundChildren
     * \param parent
     * \deprecated
     */
    void onFoundChildren(const QModelIndex &parent);
    /*!
     * \brief onItemAdded
     * \param item
     * \deprecated
     */
    void onItemAdded(FileItem *item);
    /*!
     * \brief onItemRemoved
     * \param item
     * \deprecated
     */
    void onItemRemoved(FileItem *item);

    void cancelFindChildren();

    void setRootIndex(const QModelIndex &index);

    void updateCurrentFilesThumbnails();

private:
    FileItem *m_root_item = nullptr;
    bool m_is_positive = false;
    bool m_can_expand = false;
    QString m_root_uri = "file:///";
    bool m_showFileExtension = true;

    FileManagerThread *m_fileManagerThread = nullptr;
    FileInfosJob *m_infosJob = nullptr;
};

class PEONYCORESHARED_EXPORT FileManagerThread : public QThread{
Q_OBJECT
public:
    explicit FileManagerThread();
    ~FileManagerThread();

    void batchQueryFileInfos(FileInfosJob *infosJob, /*FileItemModel::OperateType*/ int operateType, FileItem *parentItem);

protected:
    void run() override;


Q_SIGNALS:
    void finishQueryFileInfos(std::vector<std::shared_ptr<FileInfo> >& fileInfos, /*FileItemModel::OperateType*/int operateType, FileItem *parentItem);
    void setParamForBatchQueryInfos(/*std::shared_ptr<FileInfoJob>*/ FileInfosJob *infosJob, /*FileItemModel::OperateType*/int operateType, FileItem *parentItem);
};


}

#endif // FILEITEMMODEL_H
