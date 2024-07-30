/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef THEMESMODEL_H
#define THEMESMODEL_H

#include <QAbstractListModel>

class ThemeMetadata;

class ThemesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentIndexChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
public:
    enum Roles {
        IdRole = Qt::UserRole,
        PathRole,
        AuthorRole,
        DescriptionRole,
        VersionRole,
        PreviewRole,
        EmailRole,
        WebsiteRole,
        LicenseRole,
        CopyrightRole,
        ThemeApiRole,
        ConfigFileRole,
        CurrentBackgroundRole,
        DeletableRole,
    };
    Q_ENUM(Roles)

    explicit ThemesModel(QObject *parent = nullptr);
    ~ThemesModel() Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentTheme() const;
    void setCurrentTheme(const QString &theme);
    int currentIndex() const;

public Q_SLOTS:
    void populate();
Q_SIGNALS:
    void currentIndexChanged();

private:
    void add(const QString &name, const QString &path);
    void dump(const QString &id, const QString &path);

    int m_currentIndex;
    QList<ThemeMetadata> mThemeList;
    QHash<QString, QString> m_currentWallpapers;
    QList<QString> m_customInstalledThemes;
};

#endif // THEMESMODEL_H
