/*
    SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CUTTLEFISHICONMODEL_H
#define CUTTLEFISHICONMODEL_H

#include <QAbstractListModel>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>

namespace CuttleFish
{
class IconModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    enum Roles {
        FileName = Qt::UserRole + 1,
        IconName,
        Icon,
        FullPath,
        Category,
        Scalable,
        Sizes,
        Type,
        Theme,
    };

    explicit IconModel(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList inOtherThemes(const QString &iconName, int size);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QString key(int role) const;

    void add(const QFileInfo &info, const QString &cat);
    void remove(const QString &iconFile);

    QStringList categories() const;

    bool loading();

    void load();

    Q_INVOKABLE void output(const QString &text);

    Q_INVOKABLE void openContainingFolder(const QString &filename);

Q_SIGNALS:
    void categoriesChanged();
    void loadingChanged();

private:
    QHash<int, QByteArray> m_roleNames;

    QStringList m_icons;
    QStringList m_categories;
    QHash<QString, QVariantMap> m_data;
    QHash<QString, QString> m_categoryTranslations;

    bool m_loading;
    QString categoryFromPath(const QString &path);
};

} // namespace

#endif // CUTTLEFISHICONMODEL_H
