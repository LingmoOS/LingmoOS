// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "commonstruct.h"

#include <QAbstractTableModel>
#include <QFileInfoList>
#include <QDir>

class MimeTypeDisplayManager;
class QItemSelection;

class DataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DataModel(QObject *parent = nullptr);
    ~DataModel() override;

public:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    void sort(int column, Qt::SortOrder order) override;

    /**
     * @brief setFileEntry  刷新列表数据
     * @param listEntry     需要显示的数据
     */
    void refreshFileEntry(const QList<FileEntry> &listEntry);

    /**
     * @brief getSelectIndex    根据传入的名称获取选中的多行
     * @param listName          需要选中名称
     */
    QItemSelection getSelectItem(const QStringList &listName = QStringList());

    /**
     * @brief getListEntryIndex 获取对应文件名的QModelIndex
     * @param listName
     * @return
     */
    QModelIndex getListEntryIndex(const QString &listName);
private:
    QList<FileEntry> m_listEntry;       // 显示的数据
    MimeTypeDisplayManager *m_pMimetype; // 类型管理

    // 表头
    QStringList m_listColumn = QStringList() << QObject::tr("Name") << QObject::tr("Time modified") << QObject::tr("Type") << QObject::tr("Size");
};

#endif // DATAMODEL_H
