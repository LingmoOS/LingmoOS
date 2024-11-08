/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "app-group-model.h"
#include "data-provider-manager.h"
#include "app-model.h"

namespace LingmoUIMenu {

class LabelGroupModelPrivate
{
public:
    explicit LabelGroupModelPrivate(AppModel *appModel) : appModel(appModel) {}
    AppModel *appModel{nullptr};
    bool      containLabel{false};
    QVector<LabelItem> labels;
    QVector<int>       labelIndex;
};

AppGroupModel::AppGroupModel(AppModel *appModel, QObject *parent) : QAbstractListModel(parent), d(new LabelGroupModelPrivate(appModel))
{
    reloadLabels();

    connect(DataProviderManager::instance(), &DataProviderManager::pluginChanged, this, &AppGroupModel::reloadLabels);
    connect(DataProviderManager::instance(), &DataProviderManager::labelChanged, this, &AppGroupModel::reloadLabels);
    connectSignals();
}

void AppGroupModel::reloadLabels()
{
    Q_EMIT beginResetModel();

    d->labels.clear();
    for (const auto &label : DataProviderManager::instance()->labels()) {
        if (label.isDisable()) {
            continue;
        }
        d->labels.append(label);
    }
    d->containLabel = !d->labels.isEmpty();
    d->labelIndex = QVector<int>(d->labels.size(), -1);

    Q_EMIT endResetModel();
    Q_EMIT containLabelChanged(d->containLabel);
}

int AppGroupModel::rowCount(const QModelIndex &parent) const
{
    return d->labels.count();
}

QVariant AppGroupModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= d->labels.size()) {
        return {};
    }

    switch (role) {
        case DataEntity::Type:
            return DataType::Label;
        case DataEntity::Icon:
            return "";
        case DataEntity::Name:
            return d->labels.at(i).displayName();
        case DataEntity::Comment:
            return {};
        case DataEntity::ExtraData: {
            int start = getLabelIndex(i) + 1;
            int end;
            if (i >= (d->labels.size() - 1)) {
                end = d->appModel->rowCount(QModelIndex());
            } else {
                end = getLabelIndex(i + 1);
            }

            return d->appModel->getApps(start, end);
        }
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> AppGroupModel::roleNames() const
{
    return d->appModel->roleNames();
}

void AppGroupModel::connectSignals()
{
    connect(d->appModel, &QAbstractItemModel::modelReset, this, [this] {
        if (!d->containLabel) {
            return;
        }
        beginResetModel();
        for (auto &item : d->labelIndex) {
            item = -1;
        }
        endResetModel();
    });
    connect(d->appModel, &QAbstractItemModel::rowsInserted, this, [this] (const QModelIndex &parent, int first, int last) {
        if (d->containLabel) {
            beginResetModel();
            endResetModel();
        }
    });

//    connect(d->appModel, &QAbstractItemModel::rowsRemoved,
//                      this,  &QAbstractItemModel::rowsRemoved);
//
//    connect(d->appModel, &QAbstractItemModel::rowsAboutToBeRemoved,
//                      this,  &QAbstractItemModel::rowsAboutToBeRemoved);
//
//    connect(d->appModel, &QAbstractItemModel::dataChanged,
//                      this, &QAbstractItemModel::dataChanged);
//
//    connect(d->appModel, &QAbstractItemModel::rowsMoved,
//                      this, &QAbstractItemModel::rowsMoved);
//
//    connect(d->appModel, &QAbstractItemModel::layoutChanged,
//                      this, &QAbstractItemModel::layoutChanged);
}

bool AppGroupModel::containLabel()
{
    return d->containLabel;
}

inline int AppGroupModel::getLabelIndex(int i) const
{
    int index = d->labelIndex.at(i);
    if (index < 0) {
        index = d->appModel->getLabelIndex(d->labels.at(i).id());
        d->labelIndex[i] = index;
    }
    return index;
}

void AppGroupModel::openApp(int labelIndex, int appIndex)
{
    int index = d->labelIndex.at(labelIndex);
    d->appModel->appClicked(++index + appIndex);
}

int AppGroupModel::getLabelIndex(const QString &labelId)
{
    for (int i = 0; i < d->labels.size(); ++i) {
        if (d->labels.at(i).id() == labelId) {
            return i;
        }
    }

    return -1;
}

inline void AppGroupModel::resetModel(QVector<LabelItem> &labels)
{
    d->labels.swap(labels);
    d->containLabel = !d->labels.isEmpty();
    d->labelIndex = QVector<int>(d->labels.size(), -1);
}

void AppGroupModel::openMenu(int labelIndex, int appIndex)
{
    int index = d->labelIndex.at(labelIndex);
    d->appModel->openMenu(++index + appIndex, MenuInfo::FullScreen);
}

} // LingmoUIMenu
