// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubblemodel.h"

#include <notifysetting.h>

#include "bubbleitem.h"

#include <QTimer>
#include <QLoggingCategory>
#include <QDateTime>
#include <QImage>
#include <QTemporaryFile>
#include <QUrl>

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}

namespace notification {

BubbleModel::BubbleModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_updateTimeTipTimer(new QTimer(this))
{
    m_updateTimeTipTimer->setInterval(1000);
    m_updateTimeTipTimer->setSingleShot(false);
    BubbleMaxCount = NotifySetting::instance()->bubbleCount();

    connect(m_updateTimeTipTimer, &QTimer::timeout, this, &BubbleModel::updateBubbleTimeTip);
    connect(NotifySetting::instance(), &NotifySetting::contentRowCountChanged, this, &BubbleModel::updateContentRowCount);
    connect(NotifySetting::instance(), &NotifySetting::bubbleCountChanged, this, &BubbleModel::updateBubbleCount);
}

BubbleModel::~BubbleModel()
{
    qDeleteAll(m_bubbles);
    m_bubbles.clear();
}

void BubbleModel::push(BubbleItem *bubble)
{
    if (!m_updateTimeTipTimer->isActive()) {
        m_updateTimeTipTimer->start();
    }

    bool more = displayRowCount() >= BubbleMaxCount;
    if (more) {
        beginRemoveRows(QModelIndex(), BubbleMaxCount - 1, BubbleMaxCount - 1);
        endRemoveRows();
    }
    beginInsertRows(QModelIndex(), 0, 0);
    m_bubbles.prepend(bubble);
    endInsertRows();

    updateLevel();
}

bool BubbleModel::isReplaceBubble(const BubbleItem *bubble) const
{
    return replaceBubbleIndex(bubble) >= 0;
}

BubbleItem *BubbleModel::replaceBubble(BubbleItem *bubble)
{
    Q_ASSERT(isReplaceBubble(bubble));
    const auto replaceIndex = replaceBubbleIndex(bubble);
    const auto oldBubble = m_bubbles[replaceIndex];
    m_bubbles.replace(replaceIndex, bubble);
    Q_EMIT dataChanged(index(replaceIndex), index(replaceIndex));

    return oldBubble;
}

void BubbleModel::clear()
{
    if (m_bubbles.count() <= 0)
        return;
    beginResetModel();
    qDeleteAll(m_bubbles);
    m_bubbles.clear();
    endResetModel();
    m_delayBubbles.clear();
    m_delayRemovedBubble = -1;

    updateLevel();
    m_updateTimeTipTimer->stop();
}

QList<BubbleItem *> BubbleModel::items() const
{
    return m_bubbles;
}

void BubbleModel::remove(int index)
{
    if (index < 0 || index >= m_bubbles.size())
        return;

    if (index >= rowCount(QModelIndex())) {
        auto bubble = m_bubbles.takeAt(index);
        bubble->deleteLater();
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    auto bubble = m_bubbles.takeAt(index);
    bubble->deleteLater();
    endRemoveRows();

    if (m_bubbles.count() >= BubbleMaxCount) {
        beginInsertRows(QModelIndex(), displayRowCount() - 1, displayRowCount() - 1);
        endInsertRows();
    }
    updateLevel();
}

void BubbleModel::remove(const BubbleItem *bubble)
{
    const auto index = m_bubbles.indexOf(bubble);
    if (index >= 0) {
        remove(index);
    }
}

BubbleItem *BubbleModel::removeById(qint64 id)
{
    if (id == m_delayRemovedBubble) {
        // Delayed remove
        if (!m_delayBubbles.contains(id)) {
            m_delayBubbles.append(id);
        }
        return nullptr;
    }
    for (const auto &item : m_bubbles) {
        if (item->id() == id) {
            m_delayBubbles.removeAll(id);
            remove(m_bubbles.indexOf(item));
            return item;
        }
    }

    return nullptr;
}

BubbleItem *BubbleModel::bubbleItem(int bubbleIndex) const
{
    if (bubbleIndex < 0 || bubbleIndex >= items().count())
        return nullptr;

    return items().at(bubbleIndex);
}

int BubbleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return displayRowCount();
}

QVariant BubbleModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row >= m_bubbles.size() || !index.isValid())
        return {};

    switch (role) {
    case BubbleModel::AppName:
        return m_bubbles[row]->appName();
    case BubbleModel::Id:
        return m_bubbles[row]->id();
    case BubbleModel::Body:
        return m_bubbles[row]->body();
    case BubbleModel::Summary:
        return m_bubbles[row]->summary();
    case BubbleModel::IconName:
        return m_bubbles[row]->appIcon();
    case BubbleModel::Level:
        return m_bubbles[row]->level();
    case BubbleModel::CTime:
        return m_bubbles[row]->ctime();
    case BubbleModel::TimeTip:
        return m_bubbles[row]->timeTip();
    case BubbleModel::BodyImagePath:
        return m_bubbles[row]->bodyImagePath();
    case BubbleModel::OverlayCount:
        return overlayCount();
    case BubbleModel::DefaultAction:
        return m_bubbles[row]->defaultAction();
    case BubbleModel::Actions:
        return m_bubbles[row]->actions();
    case BubbleModel::Urgency:
        return m_bubbles[row]->urgency();
    case BubbleModel::ContentRowCount:
        return NotifySetting::instance()->contentRowCount();
    default:
        break;
    }
    return {};
}

QHash<int, QByteArray> BubbleModel::roleNames() const
{
    QHash<int, QByteArray> mapRoleNames;
    mapRoleNames[BubbleModel::AppName] = "appName";
    mapRoleNames[BubbleModel::Id] = "id";
    mapRoleNames[BubbleModel::Body] = "body";
    mapRoleNames[BubbleModel::Summary] = "summary";
    mapRoleNames[BubbleModel::IconName] = "iconName";
    mapRoleNames[BubbleModel::Level] = "level";
    mapRoleNames[BubbleModel::CTime] = "ctime";
    mapRoleNames[BubbleModel::TimeTip] = "timeTip";
    mapRoleNames[BubbleModel::Urgency] = "urgency";
    mapRoleNames[BubbleModel::BodyImagePath] = "bodyImagePath";
    mapRoleNames[BubbleModel::OverlayCount] = "overlayCount";
    mapRoleNames[BubbleModel::DefaultAction] = "defaultAction";
    mapRoleNames[BubbleModel::Actions] = "actions";
    mapRoleNames[BubbleModel::ContentRowCount] = "contentRowCount";
    return mapRoleNames;
}

int BubbleModel::displayRowCount() const
{
    return qMin(m_bubbles.count(), BubbleMaxCount);
}

int BubbleModel::overlayCount() const
{
    return qMin(m_bubbles.count() - displayRowCount(), OverlayMaxCount);
}

void BubbleModel::updateBubbleCount(int count)
{
    if (count == BubbleMaxCount)
        return;

    int currentRowCount = rowCount(QModelIndex());

    if (count < currentRowCount) {
        beginRemoveRows(QModelIndex(), count, currentRowCount - 1);
        endRemoveRows();
    } else if (count > currentRowCount) {
        int maxInsertCount = std::min(count, (int)m_bubbles.size());
        beginInsertRows(QModelIndex(), currentRowCount, maxInsertCount - 1);
        endInsertRows();
    }

    BubbleMaxCount = count;

    layoutChanged();
    updateLevel();
}

qint64 BubbleModel::delayRemovedBubble() const
{
    return m_delayRemovedBubble;
}

void BubbleModel::setDelayRemovedBubble(qint64 newDelayRemovedBubble)
{
    if (m_delayRemovedBubble == newDelayRemovedBubble)
        return;
    const auto oldDelayRemovedBubble = m_delayRemovedBubble;
    if (m_delayBubbles.contains(oldDelayRemovedBubble)) {
        // Remove last delayed bubble.
        QTimer::singleShot(DelayRemovBubbleTime, this, [this, oldDelayRemovedBubble]() {
            removeById(oldDelayRemovedBubble);
        });
    }

    m_delayRemovedBubble = newDelayRemovedBubble;
    emit delayRemovedBubbleChanged();
}

int BubbleModel::replaceBubbleIndex(const BubbleItem *bubble) const
{
    if (bubble->isReplace()) {
        for (int i = 0; i < m_bubbles.size(); i++) {
            auto item = m_bubbles[i];
            if (item->appName() != bubble->appName())
                continue;

            if (item->id() == bubble->id()) {
                return i;
            }
        }
    }
    return -1;
}

void BubbleModel::updateLevel()
{
    if (m_bubbles.isEmpty())
        return;

    int lastBubbleMaxIndex = BubbleMaxCount - 1;
    for (int i = 0; i < displayRowCount(); i++) {
        auto item = m_bubbles.at(i);
        item->setLevel(i == lastBubbleMaxIndex ? 1 + overlayCount() : 1);
    }
    Q_EMIT dataChanged(index(0), index(displayRowCount() - 1), {BubbleModel::Level});
}

void BubbleModel::updateBubbleTimeTip()
{
    if (m_bubbles.isEmpty()) {
        m_updateTimeTipTimer->stop();
        return;
    }

    for (auto item : m_bubbles) {
        qint64 diff = QDateTime::currentMSecsSinceEpoch() - item->ctime();
        diff /= 1000; // secs
        if (diff >= 60) {
            QString timeTip;
            timeTip = tr("%1 minutes ago").arg(diff / 60);
            item->setTimeTip(timeTip);
        };
    }

    Q_EMIT dataChanged(index(0), index(m_bubbles.size() - 1), {BubbleModel::TimeTip});
}

void BubbleModel::updateContentRowCount(int rowCount)
{
    if (m_contentRowCount == rowCount)
        return;

    m_contentRowCount = rowCount;

    if (!m_bubbles.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(m_bubbles.size() - 1), {BubbleModel::ContentRowCount});
    }
}
}
