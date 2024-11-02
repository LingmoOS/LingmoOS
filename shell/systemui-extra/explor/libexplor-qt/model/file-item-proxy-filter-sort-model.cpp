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

#include "file-item-model.h"
#include "file-item.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-meta-info.h"
#include "file-label-model.h"

#include "file-utils.h"
#include "file-operation-utils.h"

#include "global-settings.h"

#include <QDebug>
#include <QMessageBox>
#include <QDate>

#include <QLocale>
#include <QCollator>

#include <QRegularExpression>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>

using namespace Peony;

QLocale locale = QLocale(QLocale::system().name());
QCollator comparer = QCollator(locale);

const QString getModelDirectoryUri(FileItemProxyFilterSortModel *model)
{
    FileItemModel *srcModel = qobject_cast<FileItemModel *>(model->sourceModel());
    if (!srcModel) {
        qInfo()<<"source model not available now";
        return nullptr;
    }
    return srcModel->getRootUri();
}

void setModelMetaInfo(FileItemProxyFilterSortModel *model, const QString &key, const QVariant &value)
{
    auto uri = getModelDirectoryUri(model);
    if (uri.isEmpty()) {
        qCritical()<<"set model meta info failed, can not get model directory uri";
    } else {
        auto fileMetaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(model));
        if (fileMetaInfo) {
            fileMetaInfo->setMetaInfoVariant(key, value);
        } else {
            qCritical()<<"set model meta info failed, can not get model meta info.";
        }
    }
}

FileItemProxyFilterSortModel::FileItemProxyFilterSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(false);
    //enable number sort, like 100 is after 99
    comparer.setNumericMode(true);
    auto settings = GlobalSettings::getInstance();
    m_settings = settings;

    m_sortTimer = new QTimer(this);
    m_sortTimer->setSingleShot(true);
    connect(m_sortTimer, &QTimer::timeout, this, [=]{
        checkSortSettings();
        qDebug()<<"sort type:"<<m_sortType<<" sort order:"<<m_sortOrder<<" folder first:"<<m_folder_first;
        return QSortFilterProxyModel::sort(m_sortType, m_sortOrder);
    });

    m_show_hidden = settings->isExist(SHOW_HIDDEN_PREFERENCE)? settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (SHOW_HIDDEN_PREFERENCE == key) {
            m_show_hidden= GlobalSettings::getInstance()->getValue(key).toBool();
            invalidateFilter();
        }
    });

    //fix bug#174512, set hidden file should hidden in time
    connect(GlobalSettings::getInstance(), &GlobalSettings::updateHiddenFile, this, [=] (const QString& fileName) {
        qDebug() << "updateHiddenFile:"<<fileName;
        QTimer::singleShot(100, this, [=](){
            update();
        });
    });

    //黑白名单更新
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (! conn.isConnected()) {
        qCritical()<<"failed to init mDbusPeonyServer, can not connect to session dbus";
        return;
    }

    mDbusPeonyServer = new QDBusInterface("org.lingmo.explor",
                                      "/org/lingmo/explor",
                                      "org.lingmo.explor",
                                      QDBusConnection::sessionBus());

    if (! mDbusPeonyServer->isValid()){
        qCritical() << "Create /org/lingmo/explor Interface Failed " << QDBusConnection::systemBus().lastError();
        return;
    }

    //同步黑白名单数据
    syncBlackAndWhiteData();

    //接收到black_and_white_update信号则进行更新
    QDBusConnection::sessionBus().connect("org.lingmo.explor",
                                          "/org/lingmo/explor",
                                          "org.lingmo.explor",
                                          "black_and_white_update",
                                          this,
                                          SLOT(updateBlackAndWhiteList()));
}

void FileItemProxyFilterSortModel::setSourceModel(QAbstractItemModel *model)
{
    if (sourceModel())
        disconnect(sourceModel());
    QSortFilterProxyModel::setSourceModel(model);
    //FileItemModel *file_item_model = static_cast<FileItemModel*>(model);

    auto fileInfo = FileInfo::fromUri(getModelDirectoryUri(this));
    if (!fileInfo.get()->isEmptyInfo()) {
        auto job = new FileInfoJob(fileInfo);
        job->setAutoDelete();
        connect(job, &FileInfoJob::queryAsyncFinished, this, [=]{
            checkSortSettings();
            update();
        });
        job->queryAsync();
    } else {
        checkSortSettings();
    }

    connect(m_settings, &GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == USE_GLOBAL_DEFAULT_SORTING) {
            setUseGlobalSort(m_settings->getValue(key).toBool());
        }
    });

    //connect(file_item_model, &FileItemModel::updated, this, &FileItemProxyFilterSortModel::update);
}

FileItem *FileItemProxyFilterSortModel::itemFromIndex(const QModelIndex &proxyIndex)
{
    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    QModelIndex index = mapToSource(proxyIndex);
    return model->itemFromIndex(index);
}

QModelIndex FileItemProxyFilterSortModel::getSourceIndex(const QModelIndex &proxyIndex)
{
    return mapToSource(proxyIndex);
}

const QModelIndex FileItemProxyFilterSortModel::indexFromUri(const QString &uri)
{
    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    const QModelIndex sourceIndex = model->indexFromUri(uri);
    return mapFromSource(sourceIndex);
}

bool FileItemProxyFilterSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    //comment these improve code to fix disorder issue
    //and chinese first or folder first has not effect issue
    //Fix me with better solutions
//    if (left.data().isNull())
//        return sortOrder() == Qt::AscendingOrder? false: true;

//    if (right.data().isNull())
//        return sortOrder() == Qt::AscendingOrder? true: false;

//    if (left.column() != 0 || right.column() != 0) {
//        return true;
//    }

    //qDebug()<<left<<right;
    if (left.isValid() && right.isValid()) {
        FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
        auto leftItem = model->itemFromIndex(left);
        auto rightItem = model->itemFromIndex(right);
        if (!(leftItem->hasChildren() && rightItem->hasChildren())) {
            //make folder always has a higher order
            if ((leftItem->hasChildren() || rightItem->hasChildren()) && m_folder_first) {
                bool lesser = leftItem->hasChildren();
                if (sortOrder() == Qt::AscendingOrder)
                    return lesser;
                return !lesser;
            }
        }/*else if (sortColumn() != FileItemModel::ModifiedDate){
            goto default_sort;
        }*/

        //fix bug#97408,change indicator meanings
        //箭头向上为升序，向下为降序，与通常的理解对应
        //将所有排序对比跟之前的方式反过来，小于改为大于，返回true的改为false
        switch (sortColumn()) {
        case FileItemModel::FileName: {
            goto default_sort;
        }
        case FileItemModel::FileSize: {
//            qDebug() << "FileSize:" <<leftItem->m_info->displayName()<<leftItem->m_info->size()
//                     <<rightItem->m_info->displayName()<<rightItem->m_info->size();
            //fix refresh sort order change issue, use file name to compare, link to bug#92525
            if (leftItem->m_info->size() == rightItem->m_info->size())
            {
                goto default_sort;
            }
            return leftItem->m_info->size() > rightItem->m_info->size();
        }
        case FileItemModel::FileType: {
            //fix refresh sort order change issue, use file name to compare, link to bug#92525
            if (leftItem->m_info->fileType() == rightItem->m_info->fileType())
            {
                 goto default_sort;
            }
            return leftItem->m_info->fileType() > rightItem->m_info->fileType();
        }
        case FileItemModel::ModifiedDate: {
            //delete time sort in trash, fix bug#63093
            if (leftItem->uri().startsWith("trash://"))
            {
                if (leftItem->m_info->deletionDate() == rightItem->m_info->deletionDate())
                {
                    goto default_sort;
                }
                return leftItem->m_info->deletionDate() > rightItem->m_info->deletionDate();
            }

            if (Peony::GlobalSettings::getInstance()->getShowCreateTime()) {
                if (leftItem->m_info->createTime() == rightItem->m_info->createTime()) {
                    goto default_sort;
                }
                return leftItem->m_info->createTime() > rightItem->m_info->createTime();
            }

            //non trash files, use modifiedTime sort
            if (leftItem->m_info->modifiedTime() == rightItem->m_info->modifiedTime())
            {
                goto default_sort;
            }
            return leftItem->m_info->modifiedTime() > rightItem->m_info->modifiedTime();
        }
        case FileItemModel::TrashOriginPath: {
            auto leftString = leftItem->m_info->property("orig-path").toString();
            auto rightString = rightItem->m_info->property("orig-path").toString();
            if (leftString == rightString) {
                goto default_sort;
            }
            return comparer.compare(leftString, rightString) > 0;
        }
        default:
            break;
        }

default_sort:
        //function was deprecated
//        if (FileOperationUtils::leftNameIsDuplicatedFileOfRightName(leftItem->m_info->displayName(), rightItem->m_info->displayName())) {
//            return FileOperationUtils::leftNameLesserThanRightName(leftItem->info()->displayName(), rightItem->info()->displayName());
//        }

        //when other sort value is the same, use sort by name way，link to bug#92525
        //排序条件下的值相等时，使用名称进行排序，保持排序稳定，刷新后不改变, 关联bug#92525
        QString leftDisplayName = leftItem->m_info->displayName();
        QString rightDisplayName = rightItem->m_info->displayName();
        if (m_use_default_name_sort_order) {
            //fix chinese first sort wrong issue, link to bug#70836
            //fix bug#97408,change indicator meanings
            bool lesser = true;
            if(startWithChinese(leftDisplayName) && ! startWithChinese(rightDisplayName)) {
                // fix #103343
                lesser = true;
                if (sortOrder() == Qt::AscendingOrder)
                    return lesser;
                return !lesser;
            }
            else if(! startWithChinese(leftDisplayName) && startWithChinese(rightDisplayName)) {
                lesser = false;
                if (sortOrder() == Qt::AscendingOrder)
                    return lesser;
                return !lesser;
            }
            else
                return comparer.compare(leftDisplayName, rightDisplayName) > 0;
        }
        return comparer.compare(leftDisplayName, rightDisplayName) > 0;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool FileItemProxyFilterSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    //FIXME:
    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    //root
    auto childIndex = model->index(sourceRow, 0, sourceParent);
    if (childIndex.isValid()) {
        auto item = static_cast<FileItem*>(childIndex.internalPointer());

        /* task#63345 通过.hidden文件来设置隐藏文件和目录 */
        FileInfo* fileInfo =  item->m_info.get();/*FileInfo::fromUri(item->uri()).get()*/;
        bool isHidden = fileInfo->property(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN).toBool();
        //qDebug()<<"File view .hidden file hidden,uri:"<<item->uri()<<" isHidden:"<<isHidden;
        if(isHidden && !fileInfo->displayName().startsWith(".")){/* .xxx文件遵循是否显示隐藏文件的逻辑 */
           return false;
        }//end

        if(!item->shouldShow())
            return false;
        if (!m_show_hidden) {
            //qDebug()<<sourceRow<<item->m_info->displayName()<<model->rowCount(sourceParent);
            //QMessageBox::warning(nullptr, "filter", item->m_info->displayName());
            //qDebug()<<item->m_info->displayName();
            if (item->m_info->displayName() != nullptr) {
                if (item->m_info->displayName().at(0) == '.') {
                    //qDebug()<<sourceRow<<item->m_info->displayName()<<model->rowCount(sourceParent);
                    return false;
                }
            }
        }
        //regExp

        //special Volumn of 839 M upgrade part not show process
        auto targetUri = FileUtils::getTargetUri(item->uri());
        if (targetUri == "")
            targetUri = item->uri();
        if (targetUri.startsWith("file:///media/") && targetUri.endsWith("/2691-6AB8"))
            return false;

        //not show system orgin lost+found and sec_storage_data folder in DATA, fix bug#67084
        /* not show system orgin lost+found and bbox_logs folder in file:///data, fix bug#104820、bug#105714 */
        if(targetUri.endsWith("data/bbox_logs",Qt::CaseInsensitive))
            return false;
        if (targetUri.endsWith("data/lost+found",Qt::CaseInsensitive) || targetUri.endsWith("data/sec_storage_data",Qt::CaseInsensitive))
        {
            if (! item->m_info->canWrite() && ! item->m_info->canExecute())
               return false;
        }

        //FIXME use display name to hide 839 MB disk
        if (item->m_info->displayName().contains("839 MB"))
            return false;

        //check the file info filter conditions
        //qDebug()<<"start filter conditions check"<<item->m_info->displayName()<<item->m_info->type();
        if (! checkFileTypeFilter(item->m_info->type()))
            return false;
        if (! checkFileModifyTimeFilter(item->m_info->modifiedTime()))
            return false;
        if (! checkFileSizeOrTypeFilter(item->m_info->size(), item->m_info->isDir()))
            return false;

        //fix bug162927, desktop file should consider file name
        if (item->uri().endsWith(".desktop")){
            QString originName = item->uri().split("/").last();
            QString AppName = item->m_info->displayName();
            //对性能有影响，暂时屏蔽，后续再考虑优化
//           if (! item->info()->canExecute())
//               AppName = FileUtils::getApplicationName(item->uri());
           if (! checkFileNameFilter(AppName) && ! checkFileNameFilter(originName))
               return false;
        }
        else if (! checkFileNameFilter(item->m_info->displayName()))
            return false;

        //check the file label filter conditions
        if (m_label_name != "" || m_label_color != Qt::transparent)
        {
            QString uri = item->m_info->uri();
            if (m_label_name != "")
            {
                auto names = FileLabelModel::getGlobalModel()->getFileLabels(uri);
                if (! names.contains(m_label_name))
                    return false;
            }

            if (m_label_color != Qt::transparent)
            {
                auto colors = FileLabelModel::getGlobalModel()->getFileColors(uri);
                if (! colors.contains(m_label_color))
                    return false;
            }
        }

        //check multiple label filter conditions, file has any one of these label is accepted
        if(m_show_label_names.size() >0 || m_show_label_colors.size() >0)
        {
            bool bfind = false;
            QString uri = item->m_info->uri();
            if (m_show_label_names.size() >0 )
            {
                auto names = FileLabelModel::getGlobalModel()->getFileLabels(uri);
                for(auto temp : m_show_label_names)
                {
                    if(names.contains(temp))
                    {
                        bfind = true;
                        break;
                    }
                }
            }

            if (! bfind && m_show_label_colors.size() >0)
            {
                auto colors = FileLabelModel::getGlobalModel()->getFileColors(uri);
                for(auto temp : m_show_label_colors)
                {
                    if (colors.contains(temp))
                    {
                        bfind = true;
                        break;
                    }
                }
            }

            if (! bfind)
                return false;
        }

        //check the blur name, can use as search color labels
        if (m_blur_name != "")
        {
            QString uri = item->m_info->uri();
            auto names = FileLabelModel::getGlobalModel()->getFileLabels(uri);
            bool find = false;
            for(auto temp : names)
            {
                if ((m_case_sensitive && temp.indexOf(m_blur_name) >= 0) ||
                        (! m_case_sensitive && temp.toLower().indexOf(m_blur_name.toLower()) >= 0))
                {
                    find = true;
                    break;
                }
            }
            if (! find)
                return false;
        }

        if (!m_mimeTypeFilters.isEmpty()) {
            if (!m_mimeTypeFilters.contains(fileInfo->fileType())) {
                return false;
            }
        }

        if (!m_nameFilters.isEmpty()) {
            if (!fileInfo->isDir()) {
                bool contains = false;
                for (auto nameFilter : m_nameFilters) {

                    QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(nameFilter), this->filterCaseSensitivity()? QRegularExpression::NoPatternOption: QRegularExpression::CaseInsensitiveOption);
                    QRegularExpressionMatch match = rx.match(fileInfo->displayName());
                    if (match.hasMatch()) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    return false;
                }
            }
        }

        QDir::Filters dirFilters = QDir::Filters(m_dirFilters);
        if (m_dirFilters != -1) {
            bool showFiles = dirFilters & QDir::Files;
            bool showDirs = dirFilters & QDir::Dirs || dirFilters & QDir::AllDirs;
            if (!showFiles && !fileInfo->isDir()) {
                return false;
            }
            if (!showDirs && fileInfo->isDir()) {
                return false;
            }
        }

        //黑白名单处理
        if (item->m_info->isDesktopFile() && nullptr != item->m_info->desktopName()){
            if (m_bw_list_model != BW_LIST_NORMAL){
                bool exist = m_bwListInfo.contains(item->m_info->desktopName());
                if (m_bw_list_model == BW_LIST_BLACK){
                   return ! exist;
                }else if (m_bw_list_model == BW_LIST_WHITE){
                   return exist;
                }
            }
        }
    }

    return true;
}

bool FileItemProxyFilterSortModel::checkFileNameFilter(const QString &displayName) const
{
    if (m_file_name_list.size() == 0)
        return true;

    for(auto key:m_file_name_list)
    {
        //Filter criteria are not case sensitive. fix bug#92478
        if (displayName.contains(key, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

void FileItemProxyFilterSortModel::checkSortSettings()
{
    m_use_global_sort = m_settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool();
    if (m_use_global_sort) {
        m_show_hidden = m_settings->isExist(SHOW_HIDDEN_PREFERENCE)? m_settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
        m_use_default_name_sort_order = m_settings->isExist(SORT_CHINESE_FIRST)? m_settings->getValue(SORT_CHINESE_FIRST).toBool(): false;
        m_folder_first = m_settings->isExist(SORT_FOLDER_FIRST)? m_settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
    } else {
        auto info = FileInfo::fromUri(getModelDirectoryUri(this));
        auto fileMetaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
        if (fileMetaInfo && !info->isEmptyInfo()) {
            m_show_hidden = fileMetaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).isValid()? fileMetaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).toBool(): false;
            m_use_default_name_sort_order = fileMetaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).isValid()? fileMetaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).toBool(): true;
            m_folder_first = fileMetaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).isValid()? fileMetaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).toBool(): true;
        } else {
            qCritical()<<"could not get metainfo"<<getModelDirectoryUri(this);
            m_show_hidden = false;
            m_use_default_name_sort_order = true;
            m_folder_first = true;
        }
    }
}

void FileItemProxyFilterSortModel::syncBlackAndWhiteData()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.lingmo.explor", "/org/lingmo/explor",
                     "org.lingmo.explor", "getBlackAndWhiteModel");
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    m_bw_list_model = BW_LIST_NORMAL;
    if (response.type() == QDBusMessage::ReplyMessage){
        m_bw_list_model = response.arguments().takeFirst().toString();
        qDebug() << "getBlackAndWhiteModel:"<<m_bw_list_model;
    }

    QDBusMessage interface = QDBusMessage::createMethodCall("org.lingmo.explor", "/org/lingmo/explor",
                     "org.lingmo.explor", "getBWListInfo");
    QDBusMessage resp = QDBusConnection::sessionBus().call(interface);
    if (resp.type() == QDBusMessage::ReplyMessage){
        m_bwListInfo = resp.arguments().takeFirst().toStringList();
        qDebug() << "syncBlackAndWhiteData:"<<resp.arguments().length()<<m_bwListInfo;
    }
}

void FileItemProxyFilterSortModel::updateBlackAndWhiteList()
{
    syncBlackAndWhiteData();
    update();
}

void FileItemProxyFilterSortModel::setSelectionModeHint(QAbstractItemView::SelectionMode mode)
{
    setProperty("selectionMode", int(mode));
    Q_EMIT this->setSelectionModeChanged();
}

QVariant FileItemProxyFilterSortModel::getDirectorySettings(const QString &key)
{
    auto metaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
    if (metaInfo) {
        return metaInfo->getMetaInfoVariant(key);
    } else {
        qCritical()<<"can not get meta info"<<getModelDirectoryUri(this);
    }
    return QVariant();
}

void FileItemProxyFilterSortModel::setDirectorySettings(const QString &key, const QVariant &value)
{
    auto metaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
    if (metaInfo) {
        return metaInfo->setMetaInfoVariant(key, value);
    } else {
        qCritical()<<"can not set meta info"<<getModelDirectoryUri(this);
    }
}

bool FileItemProxyFilterSortModel::checkFileTypeFilter(QString type) const
{
    //qDebug()<<"m_show_file_type: "<<m_show_file_type<<" "<<type;
    //multiple condition, advance search and default search
    if (m_show_file_type == ALL_FILE && m_file_type_list.count() == 0
        || m_file_type_list.contains(ALL_FILE))
        return true;

    //support multiple file type choose
    QList<int> totalTypeList;
    if (m_file_type_list.count() > 0)
        totalTypeList.append(m_file_type_list);
    if (! totalTypeList.contains(m_show_file_type) && m_show_file_type != ALL_FILE)
        totalTypeList.append(m_show_file_type);

    for(int i=0; i<totalTypeList.count(); i++)
    {
        auto cur = totalTypeList[i];
        switch (cur)
        {
        case FILE_FOLDER:
        {
            if (type == Folder_Type)
                return true;
            break;
        }
        case PICTURE:
        {
            if (type.contains(Image_Type))
                return true;
            break;
        }
        case VIDEO:
        {
            if (type.contains(Video_Type))
                return true;
            break;
        }
        case TXT_FILE:
        {
            if (type.contains(Text_Type))
                return true;
            break;
        }
        case WPS_FILE:
        {
            if (type.contains(Wps_Type))
                return true;
            break;
        }
        case AUDIO:
        {
            if (type.contains(Audio_Type) || type.contains("application/x-smaf"))
                return true;
            break;
        }
        case OTHERS:
        {
            //exclude classfied types, show the rest other types
            if (type != Folder_Type && ! type.contains(Image_Type) && ! type.contains(Video_Type)
                    && ! type.contains(Text_Type) && !type.contains(Wps_Type) && ! type.contains(Audio_Type) && !type.contains("application/x-smaf"))
                return true;
            break;
        }
        default:
            break;
        }
    }

    return false;
}

bool FileItemProxyFilterSortModel::checkFileModifyTimeFilter(quint64 modifiedTime) const
{
    //qDebug()<<"checkFileModifyTimeFilter";
    //have no date filter
    if (m_modify_time_list.count() == 0 || m_modify_time_list.contains(ALL_FILE))
        return true;

    auto time = QDateTime::currentMSecsSinceEpoch();
    auto dateTime = QDateTime::fromMSecsSinceEpoch(time);
    //qDebug() << "cur QDateTime:" <<dateTime.toString(Qt::SystemLocaleShortDate);
    QDate date = dateTime.date();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    QDate md_date =  QDateTime::fromMSecsSinceEpoch(modifiedTime * 1000).date();
    //qDebug() << "modify time:" <<QDateTime::fromMSecsSinceEpoch(modifiedTime *1000).toString(Qt::SystemLocaleShortDate);
    int md_year= md_date.year();
    int md_month = md_date.month();
    int md_day = md_date.day();

    for(int i=0; i<m_modify_time_list.size(); i++)
    {
        auto cur = m_modify_time_list[i];
        switch(cur)
        {
        case TODAY:
        {
            if (year == md_year && month == md_month && day == md_day)
                return true;
            break;
        }
        case YESTERDAY:
        {
            QDate yesterday = date.addDays(-1);
            if(yesterday == md_date)/* 判断给定日期是否是昨天 */
                return true;
            break;
        }
        case THIS_WEEK:
        {
            QDate md_date(md_year, md_month, md_day);
            int week, md_week,week_year, md_week_year;
            week = date.weekNumber(&week_year);
            md_week = md_date.weekNumber(&md_week_year);
            if (week == md_week && week_year == md_week_year)
                return true;
            break;
        }
        case LAST_WEEK:
        {
            QDate monDate = date.addDays(-date.dayOfWeek() + 1);
            if(monDate.addDays(-7)<= md_date &&  md_date <= monDate.addDays(-1))/* 判断给定日期是否在上周 */
                return true;
            break;
        }
        case THIS_MONTH:
        {
            if (year == md_year && month == md_month)
                return true;
            break;
        }
        case LAST_MONTH:
        {
            int lastMonth = (month == 1) ? 12 : month - 1;
            if(md_month == lastMonth){/*  判断给定日期是否在上个月 */
                if((month == 1 && md_year == (year - 1)) || (month !=1 && year == md_year)){
                    return true;
                }
            }
            break;

        }
        case THIS_YEAR:
        {
            if (year == md_year)
                return true;
            break;
        }
        case LAST_YEAR:
        {
            int lastYear = year - 1;
            if(md_year == lastYear)/* 判断给定日期是否在去年 */
                return true;
            break;
        }
        default:
            break;
        }
    }

    return false;
}

bool FileItemProxyFilterSortModel::checkFileSizeFilter(quint64 size) const
{
    //qDebug()<<"checkFileSizeFilter: "<<m_show_file_size<<" "<<size;
    if (m_file_size_list.count() == 0 || m_file_size_list.contains(ALL_FILE))
        return true;

    for(int i=0; i<m_file_size_list.count(); i++)
    {
        auto cur = m_file_size_list[i];
        switch (cur)
        {
        case EMPTY: //[0K]
        {           
            if (size ==0 )
                return true;
            break;
        }
        case TINY: //（0-16K]
        {          
            if (size > 0  &&size <=16 * K_BASE)
                return true;
            break;
        }
        case SMALL:  //（16k-1M]
        {
            if(size > 16 * K_BASE && size <=K_BASE * K_BASE)
                return true;
            break;
        }
        case MEDIUM: //(1M-128M]
        {
            if(size > K_BASE * K_BASE && size <= 128 * K_BASE * K_BASE)
                return true;
            break;
        }
        case BIG:  //(128M-1G]
        {
            if(size > 128 * K_BASE * K_BASE && size <= K_BASE * K_BASE * K_BASE)
                return true;
            break;
        }
        case LARGE: //(1-4G]
        {
            if (size > K_BASE * K_BASE * K_BASE&& size <= 4*K_BASE * K_BASE * K_BASE)
                return true;
            break;
        }
        case GREAT: //>4G
        {
            if (size > 4*K_BASE * K_BASE * K_BASE)
                return true;
            break;
        }
        default:
            break;
        }

    }
    return false;
}

bool FileItemProxyFilterSortModel::checkFileSizeOrTypeFilter(quint64 size, bool isDir) const
{
    if (m_file_size_list.count() == 0 || m_file_size_list.contains(ALL_FILE))
        return true;
    else if(isDir)
        return false;

   return checkFileSizeFilter(size);
}

void FileItemProxyFilterSortModel::update()
{
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setUseGlobalSort(bool use)
{
    m_use_global_sort = use;
    checkSortSettings();
    update();
}

void FileItemProxyFilterSortModel::setShowHidden(bool showHidden)
{
    if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        GlobalSettings::getInstance()->setGSettingValue(SHOW_HIDDEN_PREFERENCE, showHidden);
    } else {
        setModelMetaInfo(this, SHOW_HIDDEN_PREFERENCE, showHidden);
    }

    m_show_hidden = showHidden;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setUseDefaultNameSortOrder(bool use)
{
    if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        GlobalSettings::getInstance()->setValue(SORT_CHINESE_FIRST, use);
    } else {
        setModelMetaInfo(this, SORT_CHINESE_FIRST, use);
    }

    m_use_default_name_sort_order = use;
    beginResetModel();
    sort(sortColumn()>0? sortColumn(): 0, sortOrder()==Qt::DescendingOrder? Qt::DescendingOrder: Qt::AscendingOrder);
    endResetModel();
}

void FileItemProxyFilterSortModel::setFolderFirst(bool folderFirst)
{
    if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        GlobalSettings::getInstance()->setValue(SORT_FOLDER_FIRST, folderFirst);
    } else {
        setModelMetaInfo(this, SORT_FOLDER_FIRST, folderFirst);
    }

    m_folder_first = folderFirst;
    beginResetModel();
    sort(sortColumn()>0? sortColumn(): 0, sortOrder()==Qt::DescendingOrder? Qt::DescendingOrder: Qt::AscendingOrder);
    endResetModel();
}

void FileItemProxyFilterSortModel::addFileNameFilter(QString key, bool updateNow)
{
    m_file_name_list.append(key);
    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::addFilterCondition(int option, int classify, bool updateNow)
{
    switch (option) {
    case 0:
        if (! m_file_type_list.contains(classify))
            m_file_type_list.append(classify);

        break;
    case 1:
        if (! m_file_size_list.contains(classify))
            m_file_size_list.append(classify);
        break;
    case 2:
        if (! m_modify_time_list.contains(classify))
            m_modify_time_list.append(classify);
        break;
    default:
        break;
    }

    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::removeFilterCondition(int option, int classify, bool updateNow)
{
    switch (option) {
    case 1:
        if (! m_file_type_list.contains(classify))
            m_file_type_list.removeOne(classify);

        break;
    case 2:
        if (! m_modify_time_list.contains(classify))
            m_modify_time_list.removeOne(classify);
        break;
    case 3:
        if (! m_file_size_list.contains(classify))
            m_file_size_list.removeOne(classify);
        break;
    default:
        break;
    }

    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::clearConditions()
{
    m_file_name_list.clear();
    m_file_type_list.clear();
    m_file_size_list.clear();
    m_modify_time_list.clear();

    m_mimeTypeFilters.clear();
    m_nameFilters.clear();
    m_dirFilters = -1;
}

void FileItemProxyFilterSortModel::setFilterConditions(int fileType, int modifyTime, int fileSize)
{
    m_show_file_type = fileType;
    m_show_file_size = fileSize;
    m_show_modify_time = modifyTime;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setFilterConditions(const QStringList &mimeTypeFilters, const QStringList &nameFilters,  QDir::Filters dirFilters, Qt::CaseSensitivity caseSensitivity)
{
    m_mimeTypeFilters = mimeTypeFilters;
    m_nameFilters = nameFilters;
    m_dirFilters = dirFilters;
    setFilterCaseSensitivity(caseSensitivity);
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setFilterLabelConditions(QString name, QColor color)
{
    m_label_name = name;
    m_label_color = color;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setMutipleLabelConditions(QStringList names, QList<QColor> colors)
{
    m_show_label_names.clear();
    m_show_label_colors.clear();

    for(auto name : names)
    {
        m_show_label_names.append(name);
    }
    for(auto color : colors)
    {
        m_show_label_colors.append(color);
    }
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setLabelBlurName(QString blurName, bool caseSensitive)
{
    m_blur_name = blurName;
    m_case_sensitive = caseSensitive;
    invalidateFilter();
}

bool FileItemProxyFilterSortModel::startWithChinese(const QString &displayName) const
{
    //NOTE: a newly created file might could not get display name soon.
    if (displayName.isEmpty()) {
        return false;
    }
    auto firstStrUnicode = displayName.at(0).unicode();
    return (firstStrUnicode <=0x9FA5 && firstStrUnicode >= 0x4E00);
}

QModelIndexList FileItemProxyFilterSortModel::getAllFileIndexes()
{
    //FIXME: how about the tree?
    QModelIndexList l;
    int i = 0;
    while (this->index(i, 0, QModelIndex()).isValid()) {
        auto index = this->index(i, 0, QModelIndex());
        if (m_show_hidden) {
            l<<index;
        } else {
            auto disyplayName = index.data(Qt::DisplayRole).toString();
            if (disyplayName.isEmpty()) {
                auto uri = this->index(i, 0, QModelIndex()).data(FileItemModel::UriRole).toString();
                disyplayName = FileUtils::getFileDisplayName(uri);
            }
            if (!disyplayName.startsWith(".")) {
                l<<index;
            }
        }

        i++;
    }
    return l;
}

QAbstractItemView::SelectionMode FileItemProxyFilterSortModel::getSelectionModeHint()
{
    if (property("selectionMode").isValid()) {
        return QAbstractItemView::SelectionMode(property("selectionMode").toInt());
    }
    return QAbstractItemView::NoSelection;
}

void FileItemProxyFilterSortModel::sort(int column, Qt::SortOrder order)
{
    m_sortType = column;
    m_sortOrder = order;
    if(!m_sortTimer->isActive()){
        m_sortTimer->start(50);
    }
}

int FileItemProxyFilterSortModel::expectedSortType()
{
    return m_sortType;
}

Qt::SortOrder FileItemProxyFilterSortModel::expectedSortOrder()
{
    return m_sortOrder;
}

void FileItemProxyFilterSortModel::manualUpdateExpectedSortInfo(int sortType, Qt::SortOrder order)
{
    m_sortType = sortType;
    m_sortOrder = order;
}

QStringList FileItemProxyFilterSortModel::getAllFileUris()
{
    QStringList l;
    auto indexes = getAllFileIndexes();
    for (auto index : indexes) {
        if (index.column() == 0)
            l<<index.data(FileItemModel::UriRole).toString();
    }
    return l;
}
