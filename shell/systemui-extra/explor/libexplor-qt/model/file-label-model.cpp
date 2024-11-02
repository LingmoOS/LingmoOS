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

#include "file-label-model.h"

#include "file-meta-info.h"
#include "file-info.h"
#include "audio-play-manager.h"

#include <QMessageBox>
#include <QUrl>

static FileLabelModel *global_instance = nullptr;
static QMap<int, QString> standardLabelNames;

FileLabelModel::FileLabelModel(QObject *parent)
    : QAbstractListModel(parent)
{
    standardLabelNames.insert(1, tr("Red"));
    standardLabelNames.insert(2, tr("Orange"));
    standardLabelNames.insert(3, tr("Yellow"));
    standardLabelNames.insert(4, tr("Green"));
    standardLabelNames.insert(5, tr("Blue"));
    standardLabelNames.insert(6, tr("Purple"));

    m_label_settings = new QSettings(QSettings::UserScope, "org.lingmo", "explor-qt", this);
    if (m_label_settings->value("lastid").isNull()) {
        //adjsut color value to design instead of Qt define color,task#25507
        QColor Red(0xFA6056);
        QColor Orange(0xF8A34C);
        QColor Yellow(0xF7CE52);
        QColor Green(0x5FD065);
        QColor Blue(0x478EF8);
        QColor Purple(0xB470D5);
        //QColor Gray(0x9D9DA0);
        //init settings
        addLabel(tr("Red"), Red, true);
        addLabel(tr("Orange"), Orange, true);
        addLabel(tr("Yellow"), Yellow, true);
        addLabel(tr("Green"), Green, true);
        addLabel(tr("Blue"), Blue, true);
        addLabel(tr("Purple"), Purple, true);
        //addLabel(tr("Transparent"), Qt::transparent);
    } else {
        initLabelItems();
    }

    connect(this, &FileLabelModel::fileLabelRenamed, this, &FileLabelModel::renameFileLabel);
}

FileLabelModel::~FileLabelModel()
{

}

FileLabelModel *FileLabelModel::getGlobalModel()
{
    if (!global_instance) {
        global_instance = new FileLabelModel;
    }
    return global_instance;
}

const QStringList FileLabelModel::getLabels()
{
    QStringList l;

    int size = m_label_settings->beginReadArray("labels");
    for (int i = 0; i < size; i++) {
        m_label_settings->setArrayIndex(i);
        if (m_label_settings->value("visible").toBool()) {
            l<<m_label_settings->value("label").toString();
        }
    }
    m_label_settings->endArray();

    return l;
}

const QList<QColor> FileLabelModel::getColors()
{
    QList<QColor> l;

    int size = m_label_settings->beginReadArray("labels");
    for (int i = 0; i < size; i++) {
        m_label_settings->setArrayIndex(i);
        if (m_label_settings->value("visible").toBool()) {
            l<<qvariant_cast<QColor>(m_label_settings->value("color"));
        }
    }
    m_label_settings->endArray();

    return l;
}

int FileLabelModel::lastLabelId()
{
    if (m_label_settings->value("lastid").isNull()) {
        return 0;
    } else {
        return m_label_settings->value("lastid").toInt();
    }
}

bool FileLabelModel::addLabel(const QString &label, const QColor &color, bool isInit)
{
    beginResetModel();

    if (getLabels().contains(label) || getColors().contains(color)) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        QMessageBox::critical(nullptr, tr("Error"), tr("Label or color is duplicated."));
        return false;
    }

    int lastid = lastLabelId();
    m_label_settings->beginWriteArray("labels");
    m_label_settings->setArrayIndex(lastid + 1);
    m_label_settings->setValue("label", label);
    m_label_settings->setValue("color", color);
    m_label_settings->setValue("visible", true);
    m_label_settings->setValue("sidebar", true);
    m_label_settings->setValue("menu", isInit);
    m_label_settings->endArray();

    auto item = new FileLabelItem(this);
    item->m_id = lastid + 1;
    item->m_name = label;
    item->m_color = color;
    item->m_isValidInSidebar = true;
    item->m_isValidInMenu = isInit;
    m_labels.append(item);

    addId();

//    connect(item, &FileLabelItem::nameChanged, this, [=](const QString &name) {
//        m_label_settings->beginWriteArray("labels");
//        m_label_settings->setArrayIndex(item->id());
//        m_label_settings->setValue("label", name);
//        m_label_settings->endArray();
//        m_label_settings->sync();
//    });

//    connect(item, &FileLabelItem::colorChanged, this, [=](const QColor &color) {
//        m_label_settings->beginWriteArray("labels");
//        m_label_settings->setArrayIndex(item->id());
//        m_label_settings->setValue("color", color);
//        m_label_settings->endArray();
//        m_label_settings->sync();
//    });

    endResetModel();
    return true;
}

void FileLabelModel::removeLabel(int id)
{
    beginResetModel();

    for (auto item : m_labels) {
        if (item->id() == id) {
            m_labels.removeOne(item);
            item->deleteLater();
            break;
        }
    }

    m_label_settings->beginWriteArray("labels", lastLabelId() + 1);
    m_label_settings->setArrayIndex(id);
    m_label_settings->setValue("visible", false);
    m_label_settings->endArray();
    m_label_settings->sync();

    Q_EMIT dataChanged(QModelIndex(), QModelIndex());

    endResetModel();
    updateLabesForAllFilesById(id);
}

void FileLabelModel::setLabelName(int id, const QString &name)
{
    for (auto item : m_labels) {
        if (item->id() == id) {
            item->setName(name);
            int row = m_labels.indexOf(item);
            Q_EMIT dataChanged(index(row), index(row));
            break;
        }
    }
}

void FileLabelModel::setLabelColor(int id, const QColor &color)
{
    for (auto item : m_labels) {
        if (item->id() == id) {
            item->setColor(color);
            int row = m_labels.indexOf(item);
            Q_EMIT dataChanged(index(row), index(row));
            updateLabesForAllFilesById(id);
            break;
        }
    }
}

const QList<int> FileLabelModel::getFileLabelIds(const QString &uri)
{
    QList<int> l;
    auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
    if (! metaInfo || metaInfo->getMetaInfoVariant(PEONY_FILE_LABEL_IDS).isNull())
        return l;
    auto labels = metaInfo->getMetaInfoStringList(PEONY_FILE_LABEL_IDS);
    for (auto label : labels) {
        l<<label.toInt();
    }
    return l;
}

const QStringList FileLabelModel::getFileLabels(const QString &uri)
{
    QStringList l;
    auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
    if (! metaInfo || metaInfo->getMetaInfoVariant(PEONY_FILE_LABEL_IDS).isNull())
        return l;
    auto labels = metaInfo->getMetaInfoStringList(PEONY_FILE_LABEL_IDS);
    for (auto label : labels) {
        auto id = label.toInt();
        auto item = itemFromId(id);
        if (item) {
            l<<item->name();
        }
    }
    return l;
}

const QList<QColor> FileLabelModel::getFileColors(const QString &uri)
{
    QList<QColor> l;
    auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
    if (! metaInfo || metaInfo->getMetaInfoVariant(PEONY_FILE_LABEL_IDS).isNull())
        return l;
    auto labels = metaInfo->getMetaInfoStringList(PEONY_FILE_LABEL_IDS);
    for (auto label : labels) {
        auto id = label.toInt();
        auto item = itemFromId(id);
        if (item) {
            l<<item->color();
        }
    }
    return l;
}

FileLabelItem *FileLabelModel::itemFromId(int id)
{
    for (auto item : this->m_labels) {
        if (id == item->id()) {
            return item;
        }
    }
    return nullptr;
}

FileLabelItem *FileLabelModel::itemFormIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        return m_labels.at(index.row());
    }
    return nullptr;
}

FileLabelItem *FileLabelModel::getItemByRow(int row)
{
    if (row >= 0) {
        return m_labels.at(row);
    }
    return nullptr;
}

QList<FileLabelItem *> FileLabelModel::getAllFileLabelItems()
{
    return m_labels;
}

void FileLabelModel::addLabelToFile(const QString &uri, int labelId)
{
    QMutexLocker lock(&m_mutex);

    /* add时更新全局标识 */
    m_label_settings->beginGroup("global labels");
    auto iter = m_globalLabelMap.find(labelId);
    QSet<QString> uriSet;
    if(iter != m_globalLabelMap.end()){
        uriSet = m_globalLabelMap.value(labelId);
    }
    uriSet.insert(uri);
    m_globalLabelMap.insert(labelId, uriSet);
    m_label_settings->setValue(QString::number(labelId), QVariant(m_globalLabelMap.value(labelId).toList()));
    m_label_settings->sync();
    m_label_settings->endGroup();

    /* 同步全局标记 */
    QUrl url(uri);
    QString labelUri = QString("label:///").append(getLabelNameFromLabelId(labelId)) + url.path() + "?schema=" + url.scheme();
    Q_EMIT fileLabelAdded(labelUri, true);//end

    auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
    if (!metaInfo) {
        return;
    }
    QStringList labelIds;
    if (metaInfo && !metaInfo->getMetaInfoVariant(PEONY_FILE_LABEL_IDS).isNull())
        labelIds = metaInfo->getMetaInfoStringList(PEONY_FILE_LABEL_IDS);
    labelIds<<QString::number(labelId);
    labelIds.removeDuplicates();
    metaInfo->setMetaInfoStringList(PEONY_FILE_LABEL_IDS, labelIds);
    Q_EMIT fileLabelChanged(uri);
    Q_EMIT fileLabelChanged(labelUri);/* 更新标识模式界面的该文件 */


}

void FileLabelModel::removeFileLabel(const QString &uri, int labelId)
{
    QMutexLocker lock(&m_mutex);
    auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
    if (! metaInfo)
        return;

    QList<int> labelIds;
    if (labelId <= 0) {/* 删除所有标记 */
        labelIds = getFileLabelIds(uri);
        metaInfo->removeMetaInfo(PEONY_FILE_LABEL_IDS);
    } else {/* 去掉颜色勾选 */
        labelIds.append(labelId);
        if (metaInfo->getMetaInfoVariant(PEONY_FILE_LABEL_IDS).isNull())
            return;
        QStringList labelIds = metaInfo->getMetaInfoStringList(PEONY_FILE_LABEL_IDS);
        labelIds.removeOne(QString::number(labelId));
        metaInfo->setMetaInfoStringList(PEONY_FILE_LABEL_IDS, labelIds);
    }
    Q_EMIT fileLabelChanged(uri);


    /* remove时更新全局标识 */
    m_label_settings->beginGroup("global labels");
    for(auto &id: labelIds){
        auto iter = m_globalLabelMap.find(id);
        if(iter != m_globalLabelMap.end()){
            QSet<QString> uriSet;
            uriSet = m_globalLabelMap.value(id);
            if(!uriSet.contains(uri))
                continue;
            uriSet.remove(uri);
            m_globalLabelMap.insert(id, uriSet);
            m_label_settings->setValue(QString::number(id), QVariant(m_globalLabelMap.value(id).toList()));
            m_label_settings->sync();
        }
        /* 同步全局标记 */
        QUrl url(uri);
        QString labelUri = QString("label:///").append(getLabelNameFromLabelId(id)) + url.path() + "?schema=" + url.scheme();
        Q_EMIT fileLabelRemoved(labelUri, true);
    }
    m_label_settings->endGroup();
}

void FileLabelModel::removeFileLabel(const QVector<QString> &uris, int labelId)
{
    for(auto& uri :uris){
        removeFileLabel(uri, labelId);
    }
}

int FileLabelModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return m_labels.size();
}

QVariant FileLabelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    switch (role) {
    case Qt::DisplayRole: {
        return m_labels.at(index.row())->name();
    }
    case Qt::DecorationRole: {
        return m_labels.at(index.row())->color();
    }
    case Qt::UserRole: {
        return m_labels.at(index.row())->id();
    }
    case Qt::ToolTipRole: {
        return m_labels.at(index.row())->name();
    }
    default:
        return QVariant();
    }
}

bool FileLabelModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        auto name = value.toString();
        if (name.isEmpty()) {
            return false;
        }
        if (getLabels().contains(name)) {
            Peony::AudioPlayManager::getInstance()->playWarningAudio();
            QMessageBox::critical(nullptr, tr("Error"), tr("Label or color is duplicated."));
            return false;
        }
        this->setLabelName(m_labels.at(index.row())->id(), name);
        Q_EMIT dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags FileLabelModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}

bool FileLabelModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool FileLabelModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

QSet<QString> FileLabelModel::getFileUrisFromLabelId(int labelId)
{
    QSet<QString> uriSet = m_globalLabelMap.value(labelId);
    return uriSet;
}

int FileLabelModel::getLabelIdFromLabelName(const QString &colorName)
{
    for (auto item : m_labels) {
        if (item->name() == colorName) {
            return item->id();
        }
    }
    return 0;
}

QString FileLabelModel::getLabelNameFromLabelId(int id)
{
    for (auto item : m_labels) {
        if (item->id() == id) {
            return item->name();
        }
    }
    return QString();
}

void FileLabelModel::setName(FileLabelItem *item, const QString &name)
{
    m_label_settings->beginWriteArray("labels", lastLabelId() + 1);
    m_label_settings->setArrayIndex(item->id());
    m_label_settings->setValue("label", name);
    m_label_settings->setValue("isNameModified", true);
    m_label_settings->endArray();
    m_label_settings->sync();
}

void FileLabelModel::setColor(FileLabelItem *item, const QColor &color)
{
    m_label_settings->beginWriteArray("labels", lastLabelId() + 1);
    m_label_settings->setArrayIndex(item->id());
    m_label_settings->setValue("color", color);
    m_label_settings->endArray();
    m_label_settings->sync();
}

void FileLabelModel::setValidInSidebar(FileLabelItem *item, bool isChecked)
{
    m_label_settings->beginWriteArray("labels", lastLabelId() + 1);
    m_label_settings->setArrayIndex(item->id());
    m_label_settings->setValue("sidebar", isChecked);
    m_label_settings->endArray();
    m_label_settings->sync();

    int row = m_labels.indexOf(item);
    Q_EMIT dataChanged(index(row), index(row));
}

void FileLabelModel::setValidInMenu(FileLabelItem *item, bool isChecked)
{
    m_label_settings->beginWriteArray("labels", lastLabelId() + 1);
    m_label_settings->setArrayIndex(item->id());
    m_label_settings->setValue("menu", isChecked);
    m_label_settings->endArray();
    m_label_settings->sync();

    int row = m_labels.indexOf(item);
    Q_EMIT dataChanged(index(row), index(row));
}

#include <QtConcurrent>
void FileLabelModel::renameFileLabel(const QString oldUri, const QString newUri)
{
    qDebug() << "rename file label -- old:" << oldUri << "  ==  new:" << newUri;
    QtConcurrent::run([=]() {
        QList<int> labelIds = getFileLabelIds(oldUri);
        removeFileLabel(oldUri);
        for(auto &id: labelIds){
            if(id <= 0)
                continue;
            addLabelToFile(newUri, id);
        }

    });
}

void FileLabelModel::updateLabesForAllFilesById(int id)
{
    QSet<QString> uriSet = getFileUrisFromLabelId(id);

    for (auto uri : uriSet) {
        QUrl url(uri);
        QString labelUri = QString("label:///").append(getLabelNameFromLabelId(id)) + url.path() + "?schema=" + url.scheme();
        Q_EMIT fileLabelChanged(uri);
        Q_EMIT fileLabelChanged(labelUri);/* 更新标识模式界面的该文件 */
    }
}

void FileLabelModel::initLabelItems()
{
    beginResetModel();
    auto size = m_label_settings->beginReadArray("labels");
    for (int i = 0; i < size; i++) {
        m_label_settings->setArrayIndex(i);
        bool visible = m_label_settings->value("visible").toBool();
        if (visible) {
            auto name = m_label_settings->value("label").toString();
            bool isNameModified = m_label_settings->value("isNameModified").toBool();
            if (!isNameModified) {
                // try find locale label name
                auto tmp = standardLabelNames.value(i);
                if (!tmp.isEmpty())
                    name = tmp;
            }
            auto color = qvariant_cast<QColor>(m_label_settings->value("color"));

            auto item = new FileLabelItem(this);
            item->m_id = i;
            item->m_name = name;
            item->m_color = color;
            item->m_isValidInSidebar = m_label_settings->contains("sidebar") ? m_label_settings->value("sidebar").toBool() : true;
            item->m_isValidInMenu = m_label_settings->contains("menu") ? m_label_settings->value("menu").toBool() : true;
            m_labels.append(item);
        }
    }
    m_label_settings->endArray();
    endResetModel();

    m_label_settings->beginWriteArray("labels", lastLabelId() + 1);
    for (int i = 0; i < m_labels.size(); i++) {
        m_label_settings->setArrayIndex(m_labels[i]->id());
        m_label_settings->setValue("sidebar", m_labels[i]->isValidInSidebar());
        m_label_settings->setValue("menu", m_labels[i]->isValidInMenu());
    }
    m_label_settings->endArray();

    m_label_settings->beginGroup("global labels");
    QStringList keys = m_label_settings->allKeys();
    for(const QString &key: keys){
        QSet<QString> uriSet = m_label_settings->value(key).toStringList().toSet();
        m_globalLabelMap.insert(key.toInt(), uriSet);
    }
    m_label_settings->endGroup();
}

void FileLabelModel::addId()
{
    int lastid = lastLabelId();
    m_label_settings->setValue("lastid", lastid + 1);
    m_label_settings->sync();
}

//FileLabelItem
FileLabelItem::FileLabelItem(QObject *parent)
{
    //should be initialized in model.
}

int FileLabelItem::id()
{
    return m_id;
}

const QString FileLabelItem::name()
{
    return m_name;
}

const QColor FileLabelItem::color()
{
    return m_color;
}

void FileLabelItem::setName(const QString &name)
{
    m_name = name;
    if (m_id >= 0) {
        if (global_instance)
            global_instance->setName(this, name);
        Q_EMIT nameChanged(name);
    }
}

void FileLabelItem::setColor(const QColor &color)
{
    if (color.blackF() == 1) {
        auto black = color;
        black.setRgbF(0.01, 0.01, 0.01);
        setColor(black);
        return;
    }
    m_color = color;
    if (m_id >= 0) {
        if (global_instance)
            global_instance->setColor(this, color);
        Q_EMIT colorChanged(color);
    }
}
bool FileLabelItem::isValidInSidebar()
{
    return m_isValidInSidebar;

}

bool FileLabelItem::isValidInMenu()
{
    return m_isValidInMenu;
}

void FileLabelItem::setValidInSidebar(bool isChecked)
{
    m_isValidInSidebar = isChecked;
    if (m_id >= 0) {
        if (global_instance)
            global_instance->setValidInSidebar(this, isChecked);
    }
}

void FileLabelItem::setValidInMenu(bool isChecked)
{
    m_isValidInMenu = isChecked;
    if (m_id >= 0) {
        if (global_instance)
            global_instance->setValidInMenu(this, isChecked);
    }
}

FileLableProxyFilterSortModel::FileLableProxyFilterSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool FileLableProxyFilterSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    //FIXME:
    FileLabelModel *model = static_cast<FileLabelModel*>(sourceModel());
    //root
    auto childIndex = model->index(sourceRow, 0, sourceParent);
    if (childIndex.isValid()) {
        auto item = static_cast<FileLabelItem*>(model->itemFormIndex(childIndex));
        if (item)
           return item->isValidInSidebar();
    }
    return true;
}
