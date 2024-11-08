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

#include "app-letter-sort-plugin.h"
#include "app-data-manager.h"

#include <memory>

namespace LingmoUIMenu {

AppLetterSortPlugin::AppLetterSortPlugin()
{
    loadAppData();
    connect(AppDataManager::instance(), &AppDataManager::appAdded, this, &AppLetterSortPlugin::onAppAdded);
    connect(AppDataManager::instance(), &AppDataManager::appDeleted, this, &AppLetterSortPlugin::onAppDeleted);
    connect(AppDataManager::instance(), &AppDataManager::appUpdated, this, &AppLetterSortPlugin::onAppUpdated);
}

int AppLetterSortPlugin::index()
{
    return 0;
}

QString AppLetterSortPlugin::id()
{
    return "letterSort";
}

QString AppLetterSortPlugin::name()
{
    return tr("Letter Sort");
}

QString AppLetterSortPlugin::icon()
{
    return "image://appicon/lingmo-capslock-symbolic";
}

QString AppLetterSortPlugin::title()
{
    return tr("Letter Sort");
}

PluginGroup::Group AppLetterSortPlugin::group()
{
    return PluginGroup::SortMenuItem;
}

QVector<DataEntity> AppLetterSortPlugin::data()
{
    return m_appData;
}

void AppLetterSortPlugin::forceUpdate()
{
    updateAppData();
}

QVector<LabelItem> AppLetterSortPlugin::labels()
{
    return m_labels;
}

void AppLetterSortPlugin::onAppAdded(const QList<DataEntity> &apps)
{
    Q_UNUSED(apps);
    updateAppData();
}

void AppLetterSortPlugin::onAppDeleted(QStringList idList)
{
    Q_UNUSED(idList);
    updateAppData();
}

void AppLetterSortPlugin::onAppUpdated(const QList<DataEntity> &apps, bool totalUpdate)
{
    if (totalUpdate) {
        updateAppData();
    } else {
        Q_EMIT dataChanged(apps.toVector(), DataUpdateMode::Update);
    }
}

void AppLetterSortPlugin::updateAppData()
{
    loadAppData();
    sendChangeSignal();
}

void AppLetterSortPlugin::sendChangeSignal()
{
    Q_EMIT labelChanged();
    Q_EMIT dataChanged(m_appData);
}

void AppLetterSortPlugin::loadAppData()
{
    QMutexLocker locker(&m_mutex);
    m_appData.clear();
    m_labels.clear();
    QList<DataEntity> appData;
    appData = AppDataManager::instance()->normalApps();

    if (appData.isEmpty()) {
        return;
    }
    sortAppData(appData);
}

void AppLetterSortPlugin::sortAppData(QList<DataEntity> &apps)
{
    QVector<DataEntity> appVector[28];

    for (int i = 0; i < apps.length(); i++) {
        QString letter = apps.at(i).firstLetter();
        char firstLetter;

        if (!letter.isEmpty())
        {
            firstLetter = letter.at(0).toUpper().toLatin1();
        } else {
            firstLetter = '&';
        }

        if (firstLetter >= 'A' && firstLetter <= 'Z') {
            appVector[(static_cast<int>(firstLetter) - 65)].append(apps.at(i));
        } else if (firstLetter >= '1' && firstLetter <= '9') {
            appVector[26].append(apps.at(i));
        } else {
            appVector[27].append(apps.at(i));
        }
    }

    for (int i = 0; i < 26; i++) {
        addAppToLabel(QString(QChar(static_cast<char>(i + 65))), appVector[i]);
    }
    addAppToLabel(QString("#"), appVector[26]);
    addAppToLabel(QString("&"), appVector[27]);
}

void AppLetterSortPlugin::addAppToLabel(const QString &labelName, QVector<DataEntity> &apps)
{
    LabelItem labelItem;
    labelItem.setDisable(apps.isEmpty());
    labelItem.setIndex(m_labels.size());
    labelItem.setId(labelName);
    labelItem.setDisplayName(labelName);
    m_labels.append(labelItem);

    if (!apps.isEmpty()) {
        DataEntity dataEntity;
        dataEntity.setComment(tr("Open Letter Sort Menu"));
        dataEntity.setName(labelName);
        dataEntity.setId(labelName);
        dataEntity.setType(DataType::Label);
        m_appData.append(dataEntity);

        std::sort(apps.begin(), apps.end(), [](const DataEntity &a, const DataEntity &b) {
            return !(a.firstLetter().compare(b.firstLetter(), Qt::CaseInsensitive) > 0);
        });

        m_appData.append(apps);
    }
}



} // LingmoUIMenu
