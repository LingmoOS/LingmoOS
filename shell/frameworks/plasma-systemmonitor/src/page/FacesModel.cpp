/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "FacesModel.h"

#include <QQmlListProperty>
#include <QQuickItem>

#include <KLocalizedString>

#include <faces/SensorFaceController.h>

#include "FaceLoader.h"
#include "PageDataObject.h"

FacesModel::FacesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> FacesModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}, {IdRole, "id"}};
}

int FacesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_faceLoaders.size() + 1;
}

QVariant FacesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        if (index.row() == m_faceLoaders.count()) {
            return i18nc("@info:placeholder", "No Chart");
        } else {
            return m_faceLoaders[index.row()]->controller()->title();
        }
    case IdRole:
        if (index.row() == m_faceLoaders.count()) {
            return "";
        } else {
            return m_faceLoaders[index.row()]->dataObject()->value(QStringLiteral("face")).toString();
        }
    }
    return QVariant();
}

QQuickItem *FacesModel::faceAtIndex(int row) const
{
    if (row == m_faceLoaders.count()) {
        return nullptr;
    }

    auto loader = m_faceLoaders.at(row);
    if (loader->controller()) {
        return loader->controller()->fullRepresentation();
    }

    return nullptr;
}

PageDataObject *FacesModel::pageData() const
{
    return m_pageData;
}

void FacesModel::setPageData(PageDataObject *pageData)
{
    if (pageData == m_pageData) {
        return;
    }
    beginResetModel();
    if (m_pageData) {
        disconnect(m_pageData, &PageDataObject::dirtyChanged, this, nullptr);
    }
    m_faceLoaders.clear();
    m_pageData = pageData;
    Q_EMIT pageDataChanged();
    if (pageData) {
        findFaceLoaders(pageData);
        connect(m_pageData, &PageDataObject::dirtyChanged, this, [this] {
            beginResetModel();
            m_faceLoaders.clear();
            findFaceLoaders(m_pageData);
            endResetModel();
        });
    }
    endResetModel();
}

void FacesModel::findFaceLoaders(PageDataObject *pageData)
{
    if (pageData->faceLoader()) {
        m_faceLoaders.append(pageData->faceLoader());
    } else {
        for (auto child : pageData->children()) {
            findFaceLoaders(child);
        }
    }
}

#include "moc_FacesModel.cpp"
