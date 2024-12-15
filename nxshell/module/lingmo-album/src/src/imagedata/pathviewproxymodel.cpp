// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathviewproxymodel.h"
#include "types.h"
#include "imageinfo.h"
#include "imagesourcemodel.h"

#include <QDebug>

PathViewProxyModel::IndexInfo::IndexInfo(const IndexInfo &other)
    : url { other.url }
    , index { other.index }
    , frameCount { other.frameCount }
    , frameIndex { other.frameIndex }
{
}

/**
   @class PathViewProxyModel
   @brief 视图代理模型，用于大图展示控件
    此模型并非纯数据模型，会同界面进行较为复杂的交互提供动画效果。
    模型构造一组类似环状链表的结构，当前视图中图片增减时，
    会自动在反方向插入数据，以达到在 PathView 中无缝切换图片的效果
 */
PathViewProxyModel::PathViewProxyModel(ImageSourceModel *srcModel, QObject *parent)
    : QAbstractListModel { parent }
    , sourceModel(srcModel)
{
    Q_ASSERT_X(nullptr != sourceModel, "init proxy model", "must contain source data");

    // 设置默认的环长度
    const int defaultCount = 5;
    setQueueCount(defaultCount);
}

PathViewProxyModel::~PathViewProxyModel() { }

QHash<int, QByteArray> PathViewProxyModel::roleNames() const
{
    return { { Types::ImageUrlRole, "imageUrl" }, { Types::FrameIndexRole, "frameIndex" } };
}

QVariant PathViewProxyModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::ParentIsInvalid | CheckIndexOption::IndexIsValid)) {
        return {};
    }

    auto infoPtr = indexQueue[index.row()];
    switch (role) {
        case Types::ImageUrlRole:
            return infoPtr ? infoPtr->url : QUrl();
        case Types::FrameIndexRole:
            return infoPtr ? infoPtr->frameIndex : 0;
        default:
            break;
    }

    return {};
}

bool PathViewProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // Do nothing
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

int PathViewProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return indexQueue.size();
}

int PathViewProxyModel::currentIndex() const
{
    return currentProxyIdx;
}

void PathViewProxyModel::setCurrentIndex(int index)
{
    if (index != currentProxyIdx) {
        currentProxyIdx = index;
        Q_EMIT currentIndexChanged(currentProxyIdx);
    }
}

/**
   @brief 跳转动画结束时，同步状态
 */
void PathViewProxyModel::syncState()
{
    if (Current != jumpFlag) {
        refreshBothSideData();
    }
}

/**
   @brief 设置当前指向的图片源索引 \a sourceIndex 和帧索引 \a frameIndex，
    若图片在当前图片左右两侧，则仅需利用 movePrevoius() 和 moveNext() 调整在数据环
    中当前指向位置; 间距超过 1 的图片会将下一张图片替换为索引指向的图片，并触发跳转动画，
    动画结束后再更新两侧的图片数据。

   @section 循环移动示例
    向前移动时，需要按环更新尾部的数据，以下示例中 < 表示索引逐步降低，> 表示索引递增增加
    当前指向代理索引为2，源数据索引为 13。

    Proxy index         0   1   2   3   4
    Index direction         <   ^   >
    Source index        11  12  13  14  15
    Proxy index         0   1   2   3   4
    Index direction     <   ^   >       *
    Source index        11  12  13  14  10
    * 此时 4 号索引被替换 * 变更为前置索引，更新为更前的源索引 10

    在 QML 组件 PathView 中，以此流程完成循环展示图片。
 */
void PathViewProxyModel::setCurrentSourceIndex(int sourceIndex, int frameIndex)
{
    if (indexQueue.isEmpty()) {
        return;
    }

    DistanceType type = distance(sourceIndex, frameIndex);

    // 若跳转中途更新代理索引(打断当前动画)立即更新图片状态
    if (Current != type && Current != jumpFlag) {
        refreshBothSideData();
    }

    switch (type) {
        case Previous:
            movePrevoius();
            break;
        case Next:
            moveNext();
            break;
        case OutOfRange:
            // 触发跳转动画设置
            jumpToIndex(sourceIndex, frameIndex);
            break;
        default:
            // Current / Invalid
            break;
    }
}

/**
   @brief 移动到上一张图片，更新当前指向的环索引
    将当前图片最右侧的图片进行更新为左侧区间的数据。
 */
void PathViewProxyModel::movePrevoius()
{
    // 同步 view 的当前位置
    setCurrentIndex(previousPorxyIdx(currentProxyIdx));

    // 移动至前一张图片，附近的索引不变，右侧区间 - 1, 左侧区间 + 1
    int changeIndex = (currentProxyIdx + radius + 1) % maxCount;
    const IndexInfoPtr &baseInfo = indexQueue[nextProxyIdx(changeIndex)];
    updateIndexInfo(changeIndex, createPreviousIndexInfo(baseInfo));
}

/**
   @brief 移动到下一张图片，更新当前指向的环索引
    将当前图片最左侧的图片进行更新为右侧区间的数据。
 */
void PathViewProxyModel::moveNext()
{
    setCurrentIndex(nextProxyIdx(currentProxyIdx));

    int changeIndex = (currentProxyIdx + radius) % maxCount;
    const IndexInfoPtr &baseInfo = indexQueue[previousPorxyIdx(changeIndex)];

    updateIndexInfo(changeIndex, createNextIndexInfo(baseInfo));
}

/**
   @brief 重置模型索引数据，指向源模型的数据将被重置，
   @note 当处于边界时，索引数据将写入空数据，以在切换中不会显示到环另一侧的
 */
void PathViewProxyModel::resetModel(int sourceIndex, int frameIndex)
{
    beginResetModel();

    // 无需通知
    currentProxyIdx = 0;
    jumpFlag = Current;
    indexQueue.clear();
    indexQueue.append(infoFromIndex(sourceIndex, frameIndex));

    QList<IndexInfoPtr> prependQueue;

    while ((indexQueue.size() + prependQueue.size()) < maxCount) {
        // 首次进入时，前面的节点不填充，而是追加到尾部，保证首张图片索引指向 0
        if (prependQueue.isEmpty()) {
            prependQueue.prepend(createPreviousIndexInfo(indexQueue.first()));
        } else {
            prependQueue.prepend(createPreviousIndexInfo(prependQueue.first()));
        }

        // 区间右侧数据直接填充
        indexQueue.append(createNextIndexInfo(indexQueue.last()));
    }

    // 追加区间左侧的数据
    indexQueue.append(prependQueue);

    endResetModel();
}

/**
   @brief 移除当前图片，并更新图片索引信息
 */
void PathViewProxyModel::deleteCurrent()
{
    if (indexQueue.isEmpty()) {
        return;
    }

    // 特殊处理，此时源数据无文件
    if (0 == sourceModel->rowCount()) {
        beginResetModel();
        setCurrentIndex(0);
        indexQueue.clear();
        endResetModel();
        return;
    }

    const IndexInfoPtr &current = indexQueue[currentProxyIdx];
    if (!current) {
        return;
    }

    // 源数据模型移除当前图片，默认将后续图片前移，currentIndex将不会变更，手动提示更新
    bool atEnd = (current->index >= sourceModel->rowCount());
    if (atEnd) {
        // 获取之前的索引
        int previousIdx = previousPorxyIdx(currentProxyIdx);

        // 如果删除的是尾部数据，则索引会 - 1 . INT_MAX 会将帧索引指向尾帧
        auto previous = infoFromIndex(current->index - 1, INT_MAX);
        updateIndexInfo(previousIdx, previous);
        // 更新完成数据后再更新界面，触发动画
        setCurrentIndex(previousIdx);

        // 更新当前索引后再更新两侧数据
        refreshBothSideData();
    } else {
        int nextIdx = nextProxyIdx(currentProxyIdx);
        // NOTE: 源数据模型移除当前图片后，index不会变更
        auto next = infoFromIndex(current->index, 0);
        updateIndexInfo(nextIdx, next);

        setCurrentIndex(nextIdx);

        refreshBothSideData();
    }
}

/**
   @brief 设置当前数据缓存的环队列长度
   @warning 环长度不小于3且必须为奇数
 */
void PathViewProxyModel::setQueueCount(int count)
{
    static int minLimit = 3;
    bool valid = (count >= minLimit) && !!(count % 2);
    Q_ASSERT(valid);

    maxCount = count;
    radius = qFloor(maxCount / 2);
}

/**
   @brief 打印当前的队列缓存信息
 */
void PathViewProxyModel::dumpInfo()
{
    qWarning() << "[ProxyModel] Currnet proxy index:" << currentProxyIdx;
    for (int i = 0; i < indexQueue.size(); ++i) {
        const IndexInfoPtr &info = indexQueue.at(i);
        if (info) {
            qWarning() << QString("[ProxyModel] index: %1, count: %2, frameIndex: %3, Url: %4 %5")
                              .arg(info->index)
                              .arg(info->frameCount)
                              .arg(info->frameIndex)
                              .arg(info->url.toString())
                              .arg((i == currentProxyIdx) ? "  <--" : "");
        } else {
            qWarning() << QString("[ProxyModel] Empty index info");
        }
    }
}

/**
   @brief 从当前索引位置跳转到指定图片源索引 \a sourceIndex 和 \a frameIndex
 */
void PathViewProxyModel::jumpToIndex(int sourceIndex, int frameIndex)
{
    const IndexInfoPtr &current = indexQueue[currentProxyIdx];
    if (!current) {
        qWarning() << "No current data" << currentProxyIdx;
        dumpInfo();
        return;
    }

    int jumpIndex = currentProxyIdx;
    DistanceType flag = Current;
    if (sourceIndex >= current->index && frameIndex >= current->frameIndex) {
        flag = Next;
        jumpIndex = nextProxyIdx(currentProxyIdx);
    } else {
        flag = Previous;
        jumpIndex = previousPorxyIdx(currentProxyIdx);
    }

    // 更新跳转图片的源数据
    IndexInfoPtr jumpInfo = infoFromIndex(sourceIndex, frameIndex);
    updateIndexInfo(jumpIndex, jumpInfo);

    // 触发跳转动画
    setCurrentIndex(jumpIndex);

    // currentIndex 的变更会判断 jumpFlag 触发 jumpFinished() ，
    // 因此 jumpFlag 的变更在 currentIndexChanged() 之后触发
    jumpFlag = flag;
}

/**
   @brief 重新刷新两侧数据，用于动画结束等场景刷新
 */
void PathViewProxyModel::refreshBothSideData()
{
    // 根据当前图片的位置，更新两侧半径的数据
    int previousIndex = currentProxyIdx;
    int nextIndex = currentProxyIdx;

    for (int i = 0; i < radius; ++i) {
        // 根据上此索引位置数据更新当前数据，中心向两侧增长
        IndexInfoPtr previousInfo = createPreviousIndexInfo(indexQueue[previousIndex]);
        previousIndex = previousPorxyIdx(previousIndex);
        updateIndexInfo(previousIndex, previousInfo);

        IndexInfoPtr nextInfo = createNextIndexInfo(indexQueue[nextIndex]);
        nextIndex = nextProxyIdx(nextIndex);
        updateIndexInfo(nextIndex, nextInfo);
    }

    // 取消状态
    jumpFlag = Current;
}

/**
   @return 返回源索引为 \a soureIndex 和 \a frameIndex 与当前图片的相对距离
 */
PathViewProxyModel::DistanceType PathViewProxyModel::distance(int sourceIndex, int frameIndex)
{
    if (sourceIndex < 0 || sourceIndex >= sourceModel->rowCount()) {
        return Invalid;
    }

    if (indexQueue.isEmpty()) {
        return Invalid;
    }

    const IndexInfoPtr &current = indexQueue[currentProxyIdx];
    if (!current) {
        return Invalid;
    }

    int range = OutOfRange;
    int indexDis = sourceIndex - current->index;
    switch (indexDis) {
        case Current:
            range = frameIndex - current->frameIndex;
            break;
        case Previous: {
            const IndexInfoPtr &previous = indexQueue[previousPorxyIdx(currentProxyIdx)];
            if (previous) {
                range = -(previous->frameCount - frameIndex) - current->frameIndex;
            }
        } break;
        case Next:
            range = current->frameCount - current->frameIndex + frameIndex;
            break;
        default:
            break;
    }

    // 判断范围
    if (Previous <= range && range <= Next) {
        return static_cast<DistanceType>(range);
    }
    return OutOfRange;
}

/**
   @return 返回源索引为 \a sourceIndex 指向的图片文件路径
 */
QUrl PathViewProxyModel::sourcePath(int sourceIndex)
{
    if (0 <= sourceIndex && sourceIndex < sourceModel->rowCount()) {
        return sourceModel->data(sourceModel->index(sourceIndex), Types::ImageUrlRole).toUrl();
    }

    return {};
}

/**
   @return 返回索引 \a base 之前的索引，由于代理数据为环状结构，
    因此索引会在 0 ~ (rowCount() - 1) 之间循环
 */
int PathViewProxyModel::previousPorxyIdx(int base)
{
    return (base - 1 + indexQueue.size()) % indexQueue.size();
}

/**
   @return 返回索引 \a base 之后的索引，由于代理数据为环状结构，
    因此索引会在 0 ~ (rowCount() - 1) 之间循环
 */
int PathViewProxyModel::nextProxyIdx(int base)
{
    return (base + 1) % indexQueue.size();
}

/**
   @return 根据源索引 \a sourceIndex 和 \a frameIndex 返回索引信息 IndexInfo，
    索引信息包含当前指向的数据源路径 / sourceIndex / frameIndex / 帧总数信息。
 */
PathViewProxyModel::IndexInfoPtr PathViewProxyModel::infoFromIndex(int sourceIndex, int frameIndex)
{
    QUrl url = sourcePath(sourceIndex);
    if (url.isEmpty()) {
        return {};
    }

    ImageInfo imageInfo(url);
    if (ImageInfo::Loading == imageInfo.status()) {
        qWarning() << "Not ready image data";
        asyncUpdateLoadInfo(url, sourceIndex, frameIndex);
    }

    IndexInfoPtr info = IndexInfoPtr::create();
    info->url = url;
    info->index = sourceIndex;
    info->frameCount = qMax(1, imageInfo.frameCount());
    info->frameIndex = qBound(0, frameIndex, info->frameCount - 1);

    return info;
}

/**
   @brief 异步更新加载图片路径 \a url 源索引 \a sourceIndex 帧索引 \a frameIndex 的数据
    此函数一般仅会在打开应用时触发，此时后端数据还未加载完成。
 */
void PathViewProxyModel::asyncUpdateLoadInfo(const QUrl &url, int sourceIndex, int frameIndex)
{
    // 异步更新加载信息，一般仅初次加载存在此问题
    if (0 == frameIndex) {
        ImageInfo *delayInfo = new ImageInfo(url, this);
        connect(delayInfo, &ImageInfo::statusChanged, this, [this, delayInfo, sourceIndex]() {
            // 仅更新加载成功的多页图数据，其它类型的数据无需关注，默认的 frameIndex 都是 0
            if (ImageInfo::Ready == delayInfo->status() && Types::MultiImage == delayInfo->type()) {
                auto current = indexQueue[currentProxyIdx];
                if (!current) {
                    delayInfo->deleteLater();
                    return;
                }

                if (sourceIndex == current->index) {
                    // 若为当前图片，仅需更新当前图片帧总数和两侧的数据
                    current->frameCount = delayInfo->frameCount();
                    refreshBothSideData();
                } else if (sourceIndex < current->index) {
                    int previousIndex = currentProxyIdx;
                    for (int i = 0; i < radius; ++i) {
                        IndexInfoPtr previousInfo = createPreviousIndexInfo(indexQueue[previousIndex]);
                        previousIndex = previousPorxyIdx(previousIndex);
                        updateIndexInfo(previousIndex, previousInfo);
                    }
                } else {
                    int nextIndex = currentProxyIdx;
                    for (int i = 0; i < radius; ++i) {
                        IndexInfoPtr nextInfo = createNextIndexInfo(indexQueue[nextIndex]);
                        nextIndex = nextProxyIdx(nextIndex);
                        updateIndexInfo(nextIndex, nextInfo);
                    }
                }
            }

            delayInfo->deleteLater();
        });
    }
}

/**
   @return 返回索引信息 \a baseInfo 之前图片的索引信息，若 \a baseInfo 为空，返回的也是 nullptr
 */
PathViewProxyModel::IndexInfoPtr PathViewProxyModel::createPreviousIndexInfo(const IndexInfoPtr &baseInfo)
{
    if (!baseInfo) {
        return {};
    }

    if (baseInfo->frameIndex > 0) {
        IndexInfoPtr previousFrame = IndexInfoPtr::create(*baseInfo);
        previousFrame->frameIndex--;
        return previousFrame;
    }

    IndexInfoPtr previous = infoFromIndex(baseInfo->index - 1);
    if (previous) {
        previous->frameIndex = previous->frameCount - 1;
    }
    return previous;
}

/**
   @return 返回索引信息 \a baseInfo 之后图片的索引信息，若 \a baseInfo 为空，返回的也是 nullptr
 */
PathViewProxyModel::IndexInfoPtr PathViewProxyModel::createNextIndexInfo(const IndexInfoPtr &baseInfo)
{
    if (!baseInfo) {
        return {};
    }

    if (baseInfo->frameCount - 1 > baseInfo->frameIndex) {
        IndexInfoPtr nextFrame = IndexInfoPtr::create(*baseInfo);
        nextFrame->frameIndex++;
        return nextFrame;
    }

    IndexInfoPtr next = infoFromIndex(baseInfo->index + 1);
    return next;
}

/**
   @brief 更新索引 \a proxyInfo 指向的信息为 \a info ，同时抛出 dataChanged() 信号
    以通知 view 更新
 */
void PathViewProxyModel::updateIndexInfo(int proxyIndex, const IndexInfoPtr &info)
{
    Q_ASSERT(0 <= proxyIndex && proxyIndex < indexQueue.size());
    indexQueue[proxyIndex] = info;

    QModelIndex changeModelIndex = index(proxyIndex);
    Q_EMIT dataChanged(changeModelIndex, changeModelIndex, { Types::ImageUrlRole, Types::FrameIndexRole });
}
