// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "fileplacesmodel.h"

FilePlacesModel::FilePlacesModel(QObject *parent)
    : KFilePlacesModel(parent)
{
}

QHash<int, QByteArray> FilePlacesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("displayRole")},
        {KFilePlacesModel::UrlRole, QByteArrayLiteral("url")},
        {KFilePlacesModel::HiddenRole, QByteArrayLiteral("hidden")},
        {KFilePlacesModel::SetupNeededRole, QByteArrayLiteral("setupNeeded")},
        {KFilePlacesModel::FixedDeviceRole, QByteArrayLiteral("fixedDevice")},
        {KFilePlacesModel::CapacityBarRecommendedRole, QByteArrayLiteral("capacityBarRecommended")},
        {KFilePlacesModel::GroupRole, QByteArrayLiteral("group")},
        {KFilePlacesModel::IconNameRole, QByteArrayLiteral("iconName")},
        {KFilePlacesModel::GroupHiddenRole, QByteArrayLiteral("groupHidden")},
    };
}
