// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <memory>

#include <QAbstractListModel>

class PatientModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ItemRole {
        IndexRole = Qt::UserRole + 1,
        ObjectRole,
    };

    explicit PatientModel(const QMetaObject &mo, QObject *parent = nullptr);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const final;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] int role(const QByteArray &roleName) const;

    // Takes ownership.
    void addObject(std::unique_ptr<QObject> patient);

    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)
    bool ready() const;
    void setReady(bool ready);
    Q_SIGNAL void readyChanged();

private Q_SLOTS:
    void propertyChanged();

private:
    int initRoleNames(const QMetaObject &mo);
    void addDynamicRoleNames(int maxEnumValue, QObject *object);
    [[nodiscard]] QMetaMethod propertyChangedMetaMethod() const;

    QList<QObject *> m_objects;
    QHash<int, QByteArray> m_roles;
    QHash<int, QByteArray> m_objectProperties;
    QHash<int, int> m_signalIndexToProperties;
    bool m_ready = false;
};
