// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TREELAND_WALLPAPER_H
#define TREELAND_WALLPAPER_H

#include <QAbstractListModel>
#include <QHash>
#include <QQmlEngine>

struct WallpaperCardModelPrivate;

class WallpaperCardModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged FINAL)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)
    Q_PROPERTY(bool showAll READ showAll WRITE setShowAll NOTIFY showAllChanged FINAL)
    Q_PROPERTY(int count READ dataCount NOTIFY dataCountChanged FINAL)
    QML_ELEMENT
public:
    enum WallpaperCardRoles { ImageSourceRole = Qt::UserRole + 1 };
    Q_ENUM(WallpaperCardRoles)

    WallpaperCardModel(const WallpaperCardModel &) = delete;
    WallpaperCardModel &operator=(const WallpaperCardModel &) = delete;
    WallpaperCardModel(QObject *parent = nullptr);
    ~WallpaperCardModel() override;

    QString directory();
    void setDirectory(const QString &directory);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] int currentIndex() const;
    void setCurrentIndex(int index);
    [[nodiscard]] bool showAll() const;
    void setShowAll(bool enable);
    [[nodiscard]] int dataCount() const;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role = Qt::DisplayRole) const override;
    Q_INVOKABLE void append(const QString &path);
    Q_INVOKABLE void remove(int index);

Q_SIGNALS:
    void directoryChanged(const QString &directory);
    void currentIndexChanged(int index);
    void showAllChanged(bool enable);
    void dataCountChanged(int count);

private:
    WallpaperCardModelPrivate *d{ nullptr };
};

#endif // TREELAND_WALLPAPER_H
