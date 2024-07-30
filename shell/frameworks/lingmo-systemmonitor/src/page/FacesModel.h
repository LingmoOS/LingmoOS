/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QAbstractListModel>
#include <qqmlregistration.h>

class QQuickItem;

class FaceLoader;
class PageDataObject;

class FacesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PageDataObject *pageData READ pageData WRITE setPageData NOTIFY pageDataChanged)
    QML_ELEMENT

    enum Roles {
        IdRole = Qt::UserRole,
    };

public:
    explicit FacesModel(QObject *parent = nullptr);

    Q_INVOKABLE QQuickItem *faceAtIndex(int row) const;

    PageDataObject *pageData() const;
    void setPageData(PageDataObject *pageData);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void pageDataChanged();

private:
    void findFaceLoaders(PageDataObject *pageData);

    PageDataObject *m_pageData = nullptr;
    QList<FaceLoader *> m_faceLoaders;
};
