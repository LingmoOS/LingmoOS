// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGELOCATIONMODEL_H
#define IMAGELOCATIONMODEL_H

#include "types.h"

#include <QAbstractListModel>
#include <QStringList>

class ImageDataModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(Types::ModelType modelType READ modelType WRITE setModelType NOTIFY modelTypeChanged)
    Q_PROPERTY(int albumId READ albumId WRITE setAlbumId NOTIFY albumIdChanged)
    Q_PROPERTY(QString keyWord READ keyWord WRITE setKeyWord NOTIFY keyWordChanged)
    Q_PROPERTY(QString devicePath READ devicePath WRITE setDevicePath NOTIFY devicePathChanged)
    Q_PROPERTY(QString dayToken READ dayToken WRITE setDayToken NOTIFY dayTokenChanged)
    Q_PROPERTY(QString importTitle READ importTitle WRITE setImportTitle NOTIFY importTitleChanged)

public:
    explicit ImageDataModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Types::ModelType modelType() const;
    void setModelType(Types::ModelType modelType);

    int albumId() const;
    void setAlbumId(int albumId);

    QString keyWord();
    void setKeyWord(QString keyWord);

    QString devicePath();
    void setDevicePath(QString devicePath);

    QString dayToken();
    void setDayToken(QString dayToken);

    QString importTitle();
    void setImportTitle(QString importTitle);

    DBImgInfo dataForIndex(const QModelIndex &index) const;

    Q_INVOKABLE void loadData(Types::ItemType type = Types::All);

signals:
    void modelTypeChanged();
    void albumIdChanged();
    void keyWordChanged();
    void devicePathChanged();
    void dayTokenChanged();
    void importTitleChanged();

private:
    Types::ModelType m_modelType;
    int m_albumID;
    QString m_devicePath;
    QString m_keyWord;
    QString m_dayToken;
    QString m_importTitle;

    QList<QPair<QByteArray, QString>> m_locations;
    DBImgInfoList m_infoList;
};

#endif // IMAGELOCATIONMODEL_H
