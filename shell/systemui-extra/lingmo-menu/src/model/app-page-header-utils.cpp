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

#include "app-page-header-utils.h"
#include "data-provider-manager.h"

#include <QVector>
#include <utility>
#include <QQmlEngine>
#include <QAbstractListModel>
#include <QMenu>
#include <QPointer>

namespace LingmoUIMenu {

class ProviderModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {Id = 0, Name, Icon, Title, IsChecked};
    explicit ProviderModel(QVector<ProviderInfo>, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateCurrentPId(const QString &providerId);
    //在各个插件之间切换
    Q_INVOKABLE void reactivateProvider();
    Q_INVOKABLE void changeProvider(int i);
    Q_INVOKABLE QString getProviderIcon(int i);
    Q_INVOKABLE int currentProviderId();

Q_SIGNALS:
    void currentIndexChanged();
    void menuClosed();

private:
    int indexOfProvider(const QString &providerId);

private:
    int m_currentIndex = 0;
    QPointer<QMenu> m_menu;
    QString m_currentId;
    QVector<ProviderInfo> m_providers;
    QHash<int, QByteArray> m_roleNames;
};

// ====== ProviderModel ======
ProviderModel::ProviderModel(QVector<ProviderInfo> infos, QObject *parent)
: QAbstractListModel(parent), m_providers(std::move(infos))
{
    std::sort(m_providers.begin(), m_providers.end(), [=](const ProviderInfo &a, const ProviderInfo &b) {
        return a.index < b.index;
    });

    m_roleNames.insert(Id, "id");
    m_roleNames.insert(Name, "name");
    m_roleNames.insert(Icon, "icon");
    m_roleNames.insert(Title, "title");
    m_roleNames.insert(IsChecked, "isChecked");
}

int ProviderModel::rowCount(const QModelIndex &parent) const
{
    return m_providers.size();
}

QVariant ProviderModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row > m_providers.size()) {
        return {};
    }

    switch (role) {
        case Id:
            return m_providers.at(row).id;
        case Name:
            return m_providers.at(row).name;
        case Icon:
            return m_providers.at(row).icon;
        case Title:
            return m_providers.at(row).title;
        case IsChecked:
            return (m_currentId == m_providers.at(row).id);
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> ProviderModel::roleNames() const
{
    return m_roleNames;
}

void ProviderModel::updateCurrentPId(const QString &providerId)
{
    int index = indexOfProvider(providerId);
    if (index < 0) {
        return;
    }

    m_currentId = providerId;
    m_currentIndex = index;

    Q_EMIT beginResetModel();
    Q_EMIT endResetModel();
    Q_EMIT currentIndexChanged();
}

int ProviderModel::indexOfProvider(const QString &providerId)
{
    for (int i = 0; i < m_providers.size(); ++i) {
        if(providerId == m_providers.at(i).id) {
            return i;
        }
    }

    return -1;
}

void ProviderModel::changeProvider(int i)
{
    DataProviderManager::instance()->activateProvider(m_providers.at(i).id);
}

QString ProviderModel::getProviderIcon(int i)
{
    return data(createIndex(i, 0), Icon).toString();
}

int ProviderModel::currentProviderId()
{
    return m_currentIndex;
}

void ProviderModel::reactivateProvider()
{
    DataProviderManager::instance()->activateProvider(m_providers.at(m_currentIndex).id);
}

// ====== AppPageHeaderUtils ======
AppPageHeaderUtils::AppPageHeaderUtils(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<ProviderModel*>("ProviderModel*");
    qRegisterMetaType<PluginGroup::Group>("PluginGroup::Group");
    qmlRegisterUncreatableType<PluginGroup>("org.lingmo.menu.utils", 1, 0, "PluginGroup", "Use enums only.");

    for (int i = PluginGroup::Button; i <= PluginGroup::SortMenuItem; ++i) {
        //qDebug() << "==PluginGroup==" << PluginGroup::Group(i);
        auto *model = new ProviderModel(DataProviderManager::instance()->providers(PluginGroup::Group(i)), this);
        model->updateCurrentPId(DataProviderManager::instance()->activatedProvider());
        m_models.insert(PluginGroup::Group(i), model);
    }

    connect(DataProviderManager::instance(), &DataProviderManager::pluginChanged,
            this, &AppPageHeaderUtils::onPluginChanged);
}

void AppPageHeaderUtils::onPluginChanged(const QString &id, PluginGroup::Group group)
{
    ProviderModel *model = m_models.value(group);
    if (model) {
        model->updateCurrentPId(id);
        Q_EMIT pluginChanged(id);
    }
}

ProviderModel *AppPageHeaderUtils::model(PluginGroup::Group group) const
{
    return m_models.value(group);
}

void AppPageHeaderUtils::activateProvider(const QString &name) const
{
    DataProviderManager::instance()->activateProvider(name);
}

void AppPageHeaderUtils::startSearch(QString key) const
{
    DataProviderManager::instance()->forceUpdate(key);
}

QString AppPageHeaderUtils::getPluginTitle(QString id) const
{
    if (id.isEmpty()) {
        id = DataProviderManager::instance()->activatedProvider();
    }

    return DataProviderManager::instance()->providerInfo(id).title;
}

QString AppPageHeaderUtils::currentPluginId() const
{
    return DataProviderManager::instance()->activatedProvider();
}

} // LingmoUIMenu

#include "app-page-header-utils.moc"
