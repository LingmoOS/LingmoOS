/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef FILELABELMODEL_H
#define FILELABELMODEL_H

#include <QAbstractListModel>
#include <QSettings>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <QColor>
#include <explorer-core_global.h>
#include <QSortFilterProxyModel>

#define PEONY_FILE_LABEL_IDS "explorer-file-label-ids"

class FileLabelItem;

class PEONYCORESHARED_EXPORT FileLabelModel : public QAbstractListModel
{
    Q_OBJECT

public:
    static FileLabelModel *getGlobalModel();

    const QStringList getLabels();
    const QList<QColor> getColors();

    int lastLabelId();

    bool addLabel(const QString &label, const QColor &color, bool isInit = false);
    void removeLabel(int id);
    void setLabelName(int id, const QString &name);
    void setLabelColor(int id, const QColor &color);

    void addLabelToFile(const QString &uri, int labelId);
    void removeFileLabel(const QString &uri, int labelId = -1);
    void removeFileLabel(const QVector<QString> &uris, int labelId = -1);

    const QList<int> getFileLabelIds(const QString &uri);
    const QStringList getFileLabels(const QString &uri);
    const QList<QColor> getFileColors(const QString &uri);
    FileLabelItem *itemFromId(int id);
    FileLabelItem *itemFormIndex(const QModelIndex &index);
    FileLabelItem *getItemByRow(int row);

    QList<FileLabelItem *> getAllFileLabelItems();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    QSet<QString> getFileUrisFromLabelId(int labelId);
    QString getLabelNameFromLabelId(int id);
    int getLabelIdFromLabelName(const QString &colorName);


Q_SIGNALS:
    void fileLabelChanged(const QString &uri);
    void fileLabelAdded(const QString &uri, bool successed);
    void fileLabelRemoved(const QString &uri, bool successed);
    void fileLabelRenamed(const QString oldUri, const QString newUri);

public Q_SLOTS:
    void setName(FileLabelItem *item, const QString &name);
    void setColor(FileLabelItem *item, const QColor &color);
    void setValidInSidebar(FileLabelItem *item, bool isChecked);
    void setValidInMenu(FileLabelItem *item, bool isChecked);
    void renameFileLabel(const QString oldUri, const QString newUri);
    void updateLabesForAllFilesById(int id);

protected:
    void initLabelItems();
    void addId();

private:
    explicit FileLabelModel(QObject *parent = nullptr);
    ~FileLabelModel();

    QSettings *m_label_settings;

    QList<FileLabelItem *> m_labels;
    QMap<int, QSet<QString> > m_globalLabelMap;
    QMutex m_mutex;
};

class PEONYCORESHARED_EXPORT FileLabelItem : public QObject
{
    friend class FileLabelModel;
    Q_OBJECT
public:
    explicit FileLabelItem(QObject *parent = nullptr);

    int id();
    const QString name();
    const QColor color();

    void setName(const QString &name);
    void setColor(const QColor &color);

    bool isValidInSidebar();
    bool isValidInMenu();

    void setValidInSidebar(bool isChecked);
    void setValidInMenu(bool isChecked);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void colorChanged(const QColor &color);

private:
    int m_id = -1; //invalid
    QString m_name = nullptr;
    QColor m_color = Qt::transparent;
    bool m_isValidInSidebar = false;
    bool m_isValidInMenu = false;
};

class PEONYCORESHARED_EXPORT FileLableProxyFilterSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FileLableProxyFilterSortModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};
#endif // FILELABELMODEL_H
