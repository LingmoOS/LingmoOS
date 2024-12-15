// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datamodel.h"
#include "uistruct.h"
#include "mimetypedisplaymanager.h"
#include "mimetypes.h"
#include "uitools.h"

#include <QDateTime>
#include <QDebug>
#include <QMimeDatabase>
#include <QIcon>
#include <QCollator>
#include <QElapsedTimer>
#include <QItemSelection>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

DataModel::DataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_pMimetype = new MimeTypeDisplayManager(this);
}

DataModel::~DataModel()
{

}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if (Qt::DisplayRole == role) {
        return m_listColumn[section];           // 返回表头对应列的内容
    } else if (Qt::TextAlignmentRole == role) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);  // 表头左对齐
    }

    return QVariant();
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    int iRow = index.row();
    int iColumn = index.column();

    FileEntry entry = m_listEntry[iRow];

    // 文件类型     文件夹子文件数目/文件大小
    QString strSize;    // 显示大小（项）

    if (entry.isDirectory) {
        strSize = QString::number(entry.qSize) + " " + tr("item(s)");
    } else {
        strSize = UiTools::humanReadableSize(entry.qSize, 1);
    }

    switch (role) {
    // 显示数据
    case Qt::DisplayRole: {
        switch (iColumn) {
        case DC_Name: {
            if(entry.strAlias.isEmpty() || entry.strAlias.isNull()) {
                return entry.strFileName;
            } else {
                return entry.strAlias;
            }
        }
        case DC_Time: {
            return QDateTime::fromTime_t(entry.uLastModifiedTime).toString("yyyy/MM/dd hh:mm:ss");
        }
        case DC_Type: {
            CustomMimeType mimetype = determineMimeType(entry.strFullPath);  // 根据全路径获取类型
            return m_pMimetype->displayName(mimetype.name());
        }
        case DC_Size: {
            return strSize; // 第四列绑定大小（文件夹为子文件数目，文件为大小）
        }
        }
        break;
    }
    // 数据信息
    case Qt::UserRole: {
        return QVariant::fromValue(entry); // 每一列绑定FileEntry数据
    }
    // 图标数据
    case Qt::DecorationRole: {
        if (DC_Name == iColumn) {
            QMimeDatabase db;
            QIcon icon;
            // 根据类型获取文件类型图标
            entry.isDirectory ? icon = QIcon::fromTheme(db.mimeTypeForName(QStringLiteral("inode/directory")).iconName()).pixmap(24, 24)
                                       : icon = QIcon::fromTheme(db.mimeTypeForFile(entry.strFileName).iconName()).pixmap(24, 24);

            // 如果获取到的图标为空，则默认给一个名称为 "empty" 的图标
            if (icon.isNull()) {
                icon = QIcon::fromTheme("empty").pixmap(24, 24);
            }

#ifdef DTKWIDGET_CLASS_DSizeMode
            if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
                return icon;
            } else {
                return icon.pixmap(16, 16);
            }
#else
            return icon;
#endif
        }

        return QVariant();
    }
    }

    return QVariant();
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_listEntry.count();
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_listColumn.count();
}

void DataModel::refreshFileEntry(const QList<FileEntry> &listEntry)
{
    beginResetModel();//在开始添加此函数
    m_listEntry = listEntry;
    endResetModel();  //在结束前添加此函数
}

QItemSelection DataModel::getSelectItem(const QStringList &listName)
{
    QItemSelection selection;

    for (int i = 0; i < m_listEntry.size(); ++i) {
        QModelIndex index = this->index(i, DC_Name);
        // 处理是否需要选中的文件
        if (listName.contains(index.data(Qt::DisplayRole).toString())) {
            QModelIndex left = this->index(index.row(), DC_Name);
            QModelIndex right = this->index(index.row(), DC_Size);
            QItemSelection sel(left, right);
            selection.merge(sel, QItemSelectionModel::Select);  // 多选合并
        }
    }

    return selection;
}

QModelIndex DataModel::getListEntryIndex(const QString &listName)
{
    for (int i = 0; i < m_listEntry.size(); ++i) {
        QModelIndex index = this->index(i, DC_Name);
        // 处理是否需要选中的文件
        if ((index.data(Qt::DisplayRole).toString()) == listName) {
            return index;
        }
    }

    return  QModelIndex();
}

void DataModel::sort(int column, Qt::SortOrder order)
{
    if (0 > column || 3 < column) {
        return;
    }

//    qInfo() << "更新排序";

    beginResetModel();//在开始添加此函数
    std::stable_sort(m_listEntry.begin(), m_listEntry.end(), [&](FileEntry entrya, FileEntry entryb) -> bool {
        //文件与目录分开排序,目录始终在前
        if (entrya.isDirectory && !entryb.isDirectory)
        {
            return true;
        }
        if (!entrya.isDirectory && entryb.isDirectory)
        {
            return false;
        }

        switch (column)
        {
        case DC_Name: {
            QCollator sortCollator;
            sortCollator.setNumericMode(true);  // 打开数字排序模式
            sortCollator.setCaseSensitivity(Qt::CaseInsensitive);

            // 对首字母是汉字进行额外处理
            if(!(entrya.strFileName.isEmpty() || entrya.strFileName.isNull())
                 &&!(entryb.strFileName.isEmpty() || entryb.strFileName.isNull())) {
                if (entrya.strFileName.at(0).script() == QChar::Script_Han) {
                    if (entryb.strFileName.at(0).script() != QChar::Script_Han) {
                        return Qt::DescendingOrder == order;
                    }
                } else if (entryb.strFileName.at(0).script() == QChar::Script_Han) {
                    return order != Qt::DescendingOrder;
                }
            }
            // 默认排序
            return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(entrya.strFileName, entryb.strFileName) < 0)) == 0x01;
        }
        case DC_Time: {
            // 比较文件最后一次修改时间
            if (Qt::AscendingOrder == order) {
                return (entrya.uLastModifiedTime < entryb.uLastModifiedTime);
            } else {
                return (entrya.uLastModifiedTime > entryb.uLastModifiedTime);
            }
        }
        case DC_Type: {

            CustomMimeType mimeLeftType = determineMimeType(entrya.strFullPath);
            CustomMimeType mimeRightType = determineMimeType(entryb.strFullPath);

            // 比较显示类型
            QCollator col;
            if (Qt::AscendingOrder == order) {
                return (col.compare(m_pMimetype->displayName(mimeLeftType.name()), m_pMimetype->displayName(mimeRightType.name())) < 0);
            } else {
                return (col.compare(m_pMimetype->displayName(mimeLeftType.name()), m_pMimetype->displayName(mimeRightType.name())) > 0);
            }
        }
        case DC_Size: {
            if (Qt::AscendingOrder == order) {
                return (entrya.qSize < entryb.qSize);
            } else {
                return (entrya.qSize > entryb.qSize);
            }
        }
        default:
            return true;
        }
    });

    endResetModel();  //在结束前添加此函数
}
