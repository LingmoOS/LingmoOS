// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JUNGLE_SORTMODEL_H
#define JUNGLE_SORTMODEL_H

#include "types.h"
#include "roles.h"
#include "itemviewadapter.h"
#include "unionimage/unionimage_global.h"

#include <QItemSelectionModel>
#include <QJsonArray>
#include <QSize>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <QTimer>
#include <QPointer>

class ThumbnailModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QByteArray sortRoleName READ sortRoleName WRITE setSortRoleName)
    Q_PROPERTY(bool containImages READ containImages WRITE setContainImages NOTIFY containImagesChanged)
    Q_PROPERTY(QList<int> selectedIndexes READ selectedIndexes NOTIFY selectedIndexesChanged)
    Q_PROPERTY(QJsonArray selectedUrls READ selectedUrls NOTIFY selectedIndexesChanged)
    Q_PROPERTY(QJsonArray selectedPaths READ selectedPaths NOTIFY selectedIndexesChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY srcModelReseted)
    Q_PROPERTY(Types::ModelType modelType READ modelType)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QObject *viewAdapter READ viewAdapter WRITE setViewAdapter NOTIFY viewAdapterChanged)
public:
    enum Status {
        None,
        Ready,
        Listing,
        Canceled,
    };
    Q_ENUM(Status)

    explicit ThumbnailModel(QObject *parent = nullptr);
    ~ThumbnailModel() override;

    QByteArray sortRoleName() const;
    void setSortRoleName(const QByteArray &name);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    Status status() const;

    QObject *viewAdapter() const;
    void setViewAdapter(QObject *adapter);

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    bool containImages();

    Q_INVOKABLE Types::ModelType modelType() const;
    Q_INVOKABLE bool isSelected(int row);
    Q_INVOKABLE void setSelected(int indexValue);
    Q_INVOKABLE void toggleSelected(int indexValue);
    Q_INVOKABLE void setRangeSelected(int anchor, int to);
    Q_INVOKABLE void updateSelection(const QVariantList &rows, bool toggle);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void pinSelection();
    Q_INVOKABLE void unpinSelection();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE int proxyIndex(const int &indexValue);
    Q_INVOKABLE int sourceIndex(const int &indexValue);
    Q_INVOKABLE QJsonArray allUrls();
    Q_INVOKABLE QJsonArray allPaths();
    Q_INVOKABLE QJsonArray selectedUrls();
    Q_INVOKABLE QJsonArray selectedPaths();
    Q_INVOKABLE QList<int> selectedIndexes();
    Q_INVOKABLE int indexForUrl(const QString &url);
    Q_INVOKABLE QList<int> indexesForUrls(const QStringList &urls);
    Q_INVOKABLE int indexForFilePath(const QString &filePath);

    Q_INVOKABLE QVariant data(int index, const QString &role);

    Q_INVOKABLE void refresh(int type = 0);

    Q_INVOKABLE void selectUrls(const QStringList &urls);

    DBImgInfo indexForData(const QModelIndex &index) const;

protected Q_SLOTS:
    void setContainImages(bool);
    void showPreview(const QString &path);
    void changeSelection(const QItemSelection &selected, const QItemSelection &deselected);

signals:
    void containImagesChanged();
    void selectedIndexesChanged();
    void srcModelReseted() const;
    void statusChanged() const;
    void viewAdapterChanged();
    void selectionChanged() const;

private:
    void setStatus(Status status);

private:
    QByteArray m_sortRoleName;
    Status m_status = Status::None;

    QItemSelectionModel *m_selectionModel;
    QItemSelection m_pinnedSelection;

    QPointer<ItemViewAdapter> m_viewAdapter;

    QTimer *m_previewTimer;
    QSize m_screenshotSize;
    bool m_containImages;
};

#endif // JUNGLE_SORTMODEL_H
