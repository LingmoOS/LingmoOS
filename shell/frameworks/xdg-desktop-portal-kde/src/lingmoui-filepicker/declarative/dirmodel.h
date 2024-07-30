// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2019-2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QUrl>
#include <QVariant>

#include <KDirModel>
#include <KDirSortFilterProxyModel>
class KDirLister;

class DirModel : public KDirSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(const QUrl &folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_PROPERTY(bool showDotFiles READ showDotFiles WRITE setShowDotFiles NOTIFY showDotFilesChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter NOTIFY nameFilterChanged)
    Q_PROPERTY(QStringList mimeFilters READ mimeFilters WRITE setMimeFilters RESET resetMimeFilters NOTIFY mimeFiltersChanged)

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    enum Roles {
        Name = Qt::UserRole + 1,
        Url,
        IconName,
        IsDir,
        IsLink,
        FileSize,
        MimeType,
        IsHidden,
        IsReadable,
        IsWritable,
        ModificationTime,
    };

    Q_ENUM(Roles)

    explicit DirModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QUrl folder() const;
    void setFolder(const QUrl &folder);

    bool showDotFiles() const;
    void setShowDotFiles(bool showDotFiles);

    bool isLoading() const;

    QString nameFilter() const;
    void setNameFilter(const QString &nameFilter);

    QStringList mimeFilters() const;
    void setMimeFilters(const QStringList &mimeFilters);
    void resetMimeFilters();

    QString lastError() const;

Q_SIGNALS:
    void folderChanged();
    void showDotFilesChanged();
    void isLoadingChanged();
    void nameFilterChanged();
    void mimeFiltersChanged();

    void lastErrorChanged();

private:
    KDirModel m_dirModel;
    KDirLister *m_lister;

    QString m_lastError;
};
