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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#ifndef FILEITEMPROXYFILTERSORTMODEL_H
#define FILEITEMPROXYFILTERSORTMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QColor>
#include <QDir>
#include <QAbstractItemView>
#include <QTimer>
#include <QDBusInterface>


#include "explorer-core_global.h"

#define BW_LIST_NORMAL  "normal"
#define BW_LIST_BLACK   "blacklist"
#define BW_LIST_WHITE   "whitelist"

namespace Peony {

class GlobalSettings;

class FileItem;
class FileItemModel;

class PEONYCORESHARED_EXPORT FileItemProxyFilterSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum FilterFileType {
        ALL_TYPE,
        FILE_FOLDER,
        PICTURE,
        VIDEO,
        TXT_FILE,
        AUDIO,
        WPS_FILE,
        OTHERS
    };
    Q_ENUM(FilterFileType)
    enum FilterFileModifyTime {
        ALL_TIME,
        TODAY,
        YESTERDAY,
        THIS_WEEK,
        LAST_WEEK,
        THIS_MONTH,
        LAST_MONTH,
        THIS_YEAR,
        LAST_YEAR
    };
    Q_ENUM(FilterFileModifyTime)
    enum FilterFileSize {
        ALL_SIZE,
        EMPTY,
        TINY,
        SMALL,
        MEDIUM,
        BIG,
        LARGE,
        GREAT
    };
    Q_ENUM(FilterFileSize)

    const QString Folder_Type = "inode/directory";
    const QString Image_Type = "image/";
    const QString Video_Type = "video/";
    const QString Text_Type = "text/";
    const QString Wps_Type = "application/wps-office";
    const QString Audio_Type = "audio/";

    explicit FileItemProxyFilterSortModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *model) override;
    void setShowHidden(bool showHidden);
    void setUseDefaultNameSortOrder(bool use);
    void setFolderFirst(bool folderFirst);
    void setFilterConditions(int fileType=0, int modifyTime=0, int fileSize=0);
    void setFilterConditions(const QStringList &mimeTypeFilters, const QStringList &nameFilters, QDir::Filters dirFilters, Qt::CaseSensitivity caseSensitivity);

    //multiple filter conditions for new advance search
    void addFileNameFilter(QString key, bool updateNow = false);
    void addFilterCondition(int option, int classify, bool updateNow = false);
    void removeFilterCondition(int option, int classify, bool updateNow = false);
    void clearConditions();

    //set file label filter conditions, default value mean all files are accepted
    //use it without any paras can clear the filter conditions
    void setFilterLabelConditions(QString name = "", QColor color=Qt::transparent);
    //select multiple labels to filter files, file has any one of these label is accepted
    void setMutipleLabelConditions(QStringList names, QList<QColor> colors);
    //give blur name to search color labels, can set CaseSensitive or not
    void setLabelBlurName(QString blurName = "", bool CaseSensitive = false);

    FileItem *itemFromIndex(const QModelIndex &proxyIndex);
    QModelIndex getSourceIndex(const QModelIndex &proxyIndex);
    const QModelIndex indexFromUri(const QString &uri);

    QStringList getAllFileUris();
    QModelIndexList getAllFileIndexes();

    QAbstractItemView::SelectionMode getSelectionModeHint();

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    int expectedSortType();
    Qt::SortOrder expectedSortOrder();

    void manualUpdateExpectedSortInfo(int sortType, Qt::SortOrder order);

public Q_SLOTS:
    void update();
    void setUseGlobalSort(bool use);
    void checkSortSettings();
    void syncBlackAndWhiteData();
    void updateBlackAndWhiteList();

    void setSelectionModeHint(QAbstractItemView::SelectionMode mode);

Q_SIGNALS:
    void setSelectionModeChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    bool startWithChinese(const QString &displayName) const;
    bool checkFileTypeFilter(QString type) const;
    bool checkFileModifyTimeFilter(quint64 modifiedTime) const;
    bool checkFileSizeFilter(quint64 size) const;
    bool checkFileSizeOrTypeFilter(quint64 sizem, bool isDir) const;
    bool checkFileNameFilter(const QString &displayName) const;

    QVariant getDirectorySettings(const QString &key);
    void setDirectorySettings(const QString &key, const QVariant &value);

private:
    GlobalSettings *m_settings = nullptr;
    bool m_use_global_sort = true;

    bool m_show_hidden;
    bool m_use_default_name_sort_order;
    bool m_folder_first;
    bool m_case_sensitive = false;
    QString m_blur_name = "";
    QString m_label_name = "";
    QString m_bw_list_model = BW_LIST_NORMAL;
    QStringList m_bwListInfo;
    QColor m_label_color = Qt::transparent;
    const int ALL_FILE = 0;
    const quint64 K_BASE = 1024;
    int m_show_file_type=ALL_FILE, m_show_modify_time=ALL_FILE, m_show_file_size=ALL_FILE;
    QList<int> m_file_type_list, m_modify_time_list, m_file_size_list;
    QStringList m_file_name_list;
    QStringList m_show_label_names;
    QList<QColor> m_show_label_colors;

    //filters for file dialog
    QStringList m_mimeTypeFilters;
    QStringList m_nameFilters;
    int m_dirFilters = -1;

    QTimer *m_sortTimer = nullptr;
    int m_sortType = 0;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

    QDBusInterface *mDbusPeonyServer = nullptr;
};

}

#endif // FILEITEMPROXYFILTERSORTMODEL_H
