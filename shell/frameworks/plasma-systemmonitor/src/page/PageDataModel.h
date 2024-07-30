/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <memory>
#include <qqmlregistration.h>

class PageDataObject;

class PageDataModel : public QAbstractListModel
{
    Q_OBJECT
    //     Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(PageDataObject *data READ dataObject WRITE setDataObject NOTIFY dataObjectChanged)
    QML_ELEMENT

public:
    enum Roles {
        DataRole = Qt::UserRole + 1,
    };
    Q_ENUM(Roles)

    explicit PageDataModel(QObject *parent = nullptr);

    PageDataObject *dataObject() const;
    void setDataObject(PageDataObject *newData);
    Q_SIGNAL void dataObjectChanged();

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE int countObjects(const QVariantMap &properties);

    //     void classBegin() override;
    //     void componentComplete() override;

private:
    PageDataObject *m_data = nullptr;
};
