// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHVIEWPROXYMODEL_H
#define PATHVIEWPROXYMODEL_H

#include <QAbstractListModel>
#include <QPointer>
#include <QUrl>
#include <QList>

#include <QPointF>

class ImageSourceModel;
// 用于 PathView 的代理数据模型
class PathViewProxyModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged FINAL)

public:
    explicit PathViewProxyModel(ImageSourceModel *srcModel, QObject *parent = nullptr);
    ~PathViewProxyModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int currentIndex() const;
    void setCurrentIndex(int index);
    Q_SIGNAL void currentIndexChanged(int index);

    Q_INVOKABLE void syncState();

    void setCurrentSourceIndex(int sourceIndex, int frameIndex);
    void movePrevoius();
    void moveNext();
    void resetModel(int sourceIndex, int frameIndex);
    void deleteCurrent();

    void setQueueCount(int count);

    void dumpInfo();

private:
    // 和当前索引相对距离类型
    enum DistanceType {
        Previous = -1,      // 前一张图片
        Current = 0,        // 当前图片
        Next = 1,           // 后一张图片
        OutOfRange = 0x80,  // 超过当前处理边界
        Invalid = 0x100,   // 不在源数据范围内
    };

    struct IndexInfo
    {
        QUrl url;
        int index { 0 };
        int frameCount { 1 };
        int frameIndex { 0 };

        IndexInfo() = default;
        IndexInfo(const IndexInfo &other);
    };
    using IndexInfoPtr = QSharedPointer<IndexInfo>;

    void jumpToIndex(int sourceIndex, int frameIndex);
    void refreshBothSideData();

    QUrl sourcePath(int sourceIndex);
    DistanceType distance(int sourceIndex, int frameIndex);
    int previousPorxyIdx(int base);
    int nextProxyIdx(int base);

    IndexInfoPtr infoFromIndex(int sourceIndex, int frameIndex = 0);
    void asyncUpdateLoadInfo(const QUrl &url, int sourceIndex, int frameIndex = 0);
    IndexInfoPtr createPreviousIndexInfo(const IndexInfoPtr &baseInfo);
    IndexInfoPtr createNextIndexInfo(const IndexInfoPtr &baseInfo);

    void updateIndexInfo(int proxyIndex, const IndexInfoPtr &info);

private:
    int maxCount { 0 };  // 固定索引区间长度，即便图片数量小于此长度
    int radius { 0 };    // 区间半径

    QPointer<ImageSourceModel> sourceModel;  // 源数据模型
    QList<IndexInfoPtr> indexQueue;          // 当前图片之前的队列区间  [左侧，当前图片，右侧]

    int currentProxyIdx { 0 };  // 当前代理索引，作为 indexQueue 的探针

    DistanceType jumpFlag { Current };  // 索引跳转的标记，同时指定方向 用于动画数据控制
};

#endif  // PATHVIEWPROXYMODEL_H
