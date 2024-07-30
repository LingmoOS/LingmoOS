/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "flatpakpermission.h"

#include <QAbstractListModel>
#include <QList>
#include <QPointer>
#include <QString>
#include <QUrl>

class FlatpakReferencesModel;
class FlatpakPermissionModel;

// Slightly similar to FlatpakResource from libdiscover
class FlatpakReference : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT FINAL)
    Q_PROPERTY(QString displayName READ displayName CONSTANT FINAL)
    Q_PROPERTY(QUrl iconSource READ iconSource CONSTANT FINAL)

public:
    explicit FlatpakReference(FlatpakReferencesModel *parent,
                              const QString &flatpakName,
                              const QString &arch,
                              const QString &branch,
                              const QString &version,
                              const QString &displayName,
                              const QUrl &iconSource,
                              const QStringList &metadataAndOverridesFiles);

    FlatpakReferencesModel *parent() const;

    QString arch() const;
    QString branch() const;
    QString version() const;

    QUrl iconSource() const;

    const QStringList &metadataAndOverridesFiles() const;
    QStringList defaultsFiles() const;
    const QString &userLevelPerAppOverrideFile() const;

    QString displayName() const;
    QString flatpakName() const;
    QString ref() const;

    FlatpakPermissionModel *permissionsModel();
    void setPermissionsModel(FlatpakPermissionModel *model);

    void load();
    void save();
    void defaults();
    bool isSaveNeeded() const;
    bool isDefaults() const;

Q_SIGNALS:
    void needsLoad();
    void settingsChanged();

private:
    // ID of a ref constitutes of these three members:
    QString m_flatpakName;
    QString m_arch;
    QString m_branch;
    // Human-readable version string.
    QString m_version;
    // Human-readable app name, only exists for installed apps.
    // Might be empty, in which case code should fallback to flatpakName.
    QString m_displayName;

    QUrl m_iconSource;
    // List of metadata and overrides files, in the order they should be
    // loaded and merged, starting from base app metadata and ending with
    // per-app user-level override.
    QStringList m_metadataAndOverridesFiles;

    QPointer<FlatpakPermissionModel> m_permissionsModel;
};

class FlatpakReferencesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FlatpakReferencesModel(QObject *parent = nullptr);

    enum Roles { Name = Qt::UserRole + 1, Version, Icon, Ref };
    Q_ENUM(Roles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void load(int index);
    void save(int index);
    void defaults(int index);
    bool isSaveNeeded(int index) const;
    bool isDefaults(int index) const;

    const QList<FlatpakReference *> &references() const;

Q_SIGNALS:
    void needsLoad();
    void settingsChanged();

private:
    QList<FlatpakReference *> m_references;
};
