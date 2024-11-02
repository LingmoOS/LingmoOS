/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

#include "favorite-extension.h"
#include "app-data-manager.h"
#include "app-manager.h"
#include "commons.h"
#include "settings.h"
#include "event-track.h"

#include <QDebug>
#include <QAbstractListModel>
#include <application-info.h>
#include <menu-manager.h>

namespace LingmoUIMenu {

class FavoriteMenuProvider : public MenuProvider
{
public:
    int index() override { return 512; }
    bool isSupport(const RequestType &type) override;
    QList<QAction *> generateActions(QObject *parent, const QVariant &data, const MenuInfo::Location &location, const QString &locationId) override;
};

class FavoriteAppsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RoleMessage
    {
        Id = Qt::UserRole,
        Icon = Qt::UserRole + 1,
        Name = Qt::UserRole + 2
    };

    explicit FavoriteAppsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void setFavoriteAppsData(QVector<DataEntity> &apps);

    Q_INVOKABLE void openMenu(const int &index);
public Q_SLOTS:
    void exchangedAppsOrder(int startIndex, int endIndex);
    void onStyleChanged(const GlobalSetting::Key &key);

private:
    QVector<DataEntity> m_favoriteAppsData;
    LingmoUISearch::ApplicationInfo *m_appInfoTable = nullptr;
};

FavoriteExtension::FavoriteExtension(QObject *parent) : MenuExtensionIFace(parent)
{
    qRegisterMetaType<LingmoUIMenu::FavoriteAppsModel*>("FavoriteAppsModel*");

    MenuManager::instance()->registerMenuProvider(new FavoriteMenuProvider);

    m_favoriteAppsModel = new FavoriteAppsModel(this);
    m_data.insert("favoriteAppsModel", QVariant::fromValue(m_favoriteAppsModel));

    updateFavoriteData();
    connect(AppDataManager::instance(),&AppDataManager::favoriteAppChanged, this,&FavoriteExtension::updateFavoriteData);
    connect(GlobalSetting::instance(), &GlobalSetting::styleChanged, m_favoriteAppsModel, &FavoriteAppsModel::onStyleChanged);
}

FavoriteExtension::~FavoriteExtension()
{

}

void FavoriteExtension::receive(QVariantMap data)
{
    QString path = data.value("id").toString();
    openFavoriteApp(path);
}

int FavoriteExtension::index()
{
    return 0;
}

QString FavoriteExtension::name()
{
    return tr("Favorite");
}

QUrl FavoriteExtension::url()
{
    return {"qrc:///qml/extensions/FavoriteExtension.qml"};
}

QVariantMap FavoriteExtension::data()
{
    return m_data;
}

void FavoriteExtension::openFavoriteApp(const QString &path)
{
    AppManager::instance()->launchApp(path);
}

void FavoriteExtension::updateFavoriteData()
{
    QVector<DataEntity> favoriteApps = AppDataManager::instance()->favoriteApps();
    m_favoriteAppsModel->setFavoriteAppsData(favoriteApps);
}

FavoriteAppsModel::FavoriteAppsModel(QObject *parent) : QAbstractListModel(parent)
{
    m_appInfoTable = new LingmoUISearch::ApplicationInfo(this);
}

int FavoriteAppsModel::rowCount(const QModelIndex &parent) const
{
    return m_favoriteAppsData.count();
}

QHash<int, QByteArray> FavoriteAppsModel::roleNames() const
{
    QHash<int,QByteArray> names;
    names.insert(Id,"id");
    names.insert(Icon,"icon");
    names.insert(Name,"name");
    return names;
}

QVariant FavoriteAppsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row > m_favoriteAppsData.count()) {
        return {};
    }

    switch (role) {
        case Id:
            return m_favoriteAppsData.at(row).id();
        case Icon:
            return m_favoriteAppsData.at(row).icon();
        case Name:
            return m_favoriteAppsData.at(row).name();
        default:
            break;
    }
    return {};
}

void FavoriteAppsModel::setFavoriteAppsData(QVector<DataEntity> &apps)
{
    beginResetModel();
    m_favoriteAppsData.swap(apps);
    endResetModel();
}

void FavoriteAppsModel::openMenu(const int &index)
{
    if (index < 0 || index >= m_favoriteAppsData.size()) {
        return;
    }

    MenuManager::instance()->showMenu(m_favoriteAppsData.at(index), MenuInfo::Extension, "favorite");
}

void FavoriteAppsModel::exchangedAppsOrder(int startIndex, int endIndex)
{
    int endNum = m_favoriteAppsData.at(endIndex).favorite();
    QString startId = m_favoriteAppsData.at(startIndex).id();
    AppDataManager::instance()->changedFavoriteOrderSignal(startId, endNum);
}

void FavoriteAppsModel::onStyleChanged(const GlobalSetting::Key &key)
{
    if (key == GlobalSetting::IconThemeName) {
        beginResetModel();
        endResetModel();
    }
}

bool FavoriteMenuProvider::isSupport(const MenuProvider::RequestType &type)
{
    return type == DataType;
}

QList<QAction *>
FavoriteMenuProvider::generateActions(QObject *parent, const QVariant &data, const MenuInfo::Location &location,
                                      const QString &locationId)
{
    if (!parent) {
        return {};
    }

    DataEntity app = data.value<DataEntity>();
    if (app.type() != DataType::Normal) {
        return {};
    }

    QList<QAction *> list;

    switch (location) {
        case MenuInfo::AppList:
        case MenuInfo::FolderPage:
        case MenuInfo::Extension: {
            if (app.favorite() == 0) {
                list << new QAction(QObject::tr("Fix to favorite"), parent);
                QObject::connect(list.last(), &QAction::triggered, parent, [app] {
                    Q_EMIT AppDataManager::instance()->fixToFavoriteSignal(app.id(), 1);
                    EventTrack::instance()->sendDefaultEvent("fix_to_favorite", "Right-click Menu");
                });
            } else if (locationId == "favorite") {
                list << new QAction(QObject::tr("Remove from favorite"), parent);
                QObject::connect(list.last(), &QAction::triggered, parent, [app] {
                    Q_EMIT AppDataManager::instance()->fixToFavoriteSignal(app.id(), 0);
                    EventTrack::instance()->sendDefaultEvent("remove_from_favorite", "Right-click Menu");
                });
            }
            break;
        }
        case MenuInfo::FullScreen:
        case MenuInfo::FullScreenFolder:
        default:
            break;
    }

    return list;
}

} // LingmoUIMenu

#include "favorite-extension.moc"
