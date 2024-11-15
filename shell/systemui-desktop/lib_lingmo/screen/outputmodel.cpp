/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "outputmodel.h"
#include <QProcess>
#include "./common/utils.h"

#include "confighandler.h"

#include <QRect>

OutputModel::OutputModel(ConfigHandler *configHandler)
    : QAbstractListModel(configHandler)
    , m_config(configHandler)
{
    connect(this, &OutputModel::dataChanged, this, &OutputModel::changed);
}

int OutputModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_outputs.count();
}

QVariant OutputModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_outputs.count()) {
        return QVariant();
    }

    const KScreen::OutputPtr &output = m_outputs[index.row()].ptr;
    switch (role) {
    case Qt::DisplayRole:
        return Utils::outputName(output);
    case EnabledRole:
        return output->isEnabled();
    case InternalRole:
        return output->type() == KScreen::Output::Type::Panel;
    case PrimaryRole:
        return output->isPrimary();
    case SizeRole:
        return output->geometry().size();
    case PositionRole:
        return m_outputs[index.row()].pos;
    case NormalizedPositionRole:
        return output->geometry().topLeft();
    case AutoRotateRole:
        return m_config->autoRotate(output);
    case AutoRotateOnlyInTabletModeRole:
        return m_config->autoRotateOnlyInTabletMode(output);
    case RotationRole:
        return output->rotation();
    case ScaleRole:
        return output->scale();
    case ResolutionIndexRole:
        return resolutionIndex(output);
    case ResolutionsRole:
        return resolutionsStrings(output);
    case RefreshRateIndexRole:
        return refreshRateIndex(output);
    case ReplicationSourceModelRole:
        return replicationSourceModel(output);
    case ReplicationSourceIndexRole:
        return replicationSourceIndex(index.row());
    case ReplicasModelRole:
        return replicasModel(output);
    case RefreshRatesRole:
        QVariantList ret;
        for (const auto rate : refreshRates(output)) {
            ret << QString("%1 Hz").arg(int(rate + 0.5));
        }
        return ret;
    }
    return QVariant();
}

bool OutputModel::setData(const QModelIndex &index,
                          const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= m_outputs.count()) {
        return false;
    }

    Output &output = m_outputs[index.row()];
    switch (role) {
    case PositionRole:
        if (value.canConvert<QPoint>()) {
            QPoint val = value.toPoint();
            if (output.pos == val) {
                return false;
            }

            snap(output, val);
            m_outputs[index.row()].pos = val;
            updatePositions();
            Q_EMIT positionChanged();
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        break;
    case EnabledRole:
        if (value.canConvert<bool>()) {
            return setEnabled(index.row(), value.toBool());
        }
        break;
    case PrimaryRole:
        if (value.canConvert<bool>()) {
            bool primary = value.toBool();
            if (output.ptr->isPrimary() == primary) {
                return false;
            }
            m_config->config()->setPrimaryOutput(output.ptr);
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        break;
    case ResolutionIndexRole:
        if (value.canConvert<int>()) {
            return setResolution(index.row(), value.toInt());
        }
        break;
    case RefreshRateIndexRole:
        if (value.canConvert<int>()) {
            return setRefreshRate(index.row(), value.toInt());
        }
        break;
    case AutoRotateRole:
        if (value.canConvert<bool>()) {
            return setAutoRotate(index.row(), value.value<bool>());
        }
        break;
    case AutoRotateOnlyInTabletModeRole:
        if (value.canConvert<bool>()) {
            return setAutoRotateOnlyInTabletMode(index.row(), value.value<bool>());
        }
        break;
    case RotationRole:
        if (value.canConvert<KScreen::Output::Rotation>()) {
            return setRotation(index.row(),
                               value.value<KScreen::Output::Rotation>());
        }
        break;
    case ReplicationSourceIndexRole:
        if (value.canConvert<int>()) {
            return setReplicationSourceIndex(index.row(), value.toInt() - 1);
        }
        break;
    case ScaleRole:
        bool ok;
        const qreal scale = value.toReal(&ok);
        if (ok && !qFuzzyCompare(output.ptr->scale(), scale)) {
            output.ptr->setScale(scale);
            m_config->setScale(output.ptr, scale);
            Q_EMIT sizeChanged();
            Q_EMIT dataChanged(index, index, {role, SizeRole});
            return true;
        }
        break;
    }
    return false;
}

QHash<int, QByteArray> OutputModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[EnabledRole] = "enabled";
    roles[InternalRole] = "internal";
    roles[PrimaryRole] = "primary";
    roles[SizeRole] = "size";
    roles[PositionRole] = "position";
    roles[NormalizedPositionRole] = "normalizedPosition";
    roles[AutoRotateRole] = "autoRotate";
    roles[AutoRotateOnlyInTabletModeRole] = "autoRotateOnlyInTabletMode";
    roles[RotationRole] = "rotation";
    roles[ScaleRole] = "scale";
    roles[ResolutionIndexRole] = "resolutionIndex";
    roles[ResolutionsRole] = "resolutions";
    roles[RefreshRateIndexRole] = "refreshRateIndex";
    roles[RefreshRatesRole] = "refreshRates";
    roles[ReplicationSourceModelRole] = "replicationSourceModel";
    roles[ReplicationSourceIndexRole] = "replicationSourceIndex";
    roles[ReplicasModelRole] = "replicasModel";
    return roles;
}

void OutputModel::add(const KScreen::OutputPtr &output)
{
    const int insertPos = m_outputs.count();
    Q_EMIT beginInsertRows(QModelIndex(), insertPos, insertPos);

    int i = 0;
    while (i < m_outputs.size()) {
        const QPoint pos = m_outputs[i].ptr->pos();
        if (output->pos().x() < pos.x()) {
            break;
        }
        if (output->pos().x() == pos.x() &&
                output->pos().y() < pos.y()) {
            break;
        }
        i++;
    }
    // Set the initial non-normalized position to be the normalized
    // position plus the current delta.
    QPoint pos = output->pos();
    if (!m_outputs.isEmpty()) {
        const QPoint delta = m_outputs[0].pos - m_outputs[0].ptr->pos();
        pos = output->pos() + delta;
    }
    m_outputs.insert(i, Output(output, pos));

    connect(output.data(), &KScreen::Output::modesChanged,
            this, [this, output](){
        roleChanged(output->id(), PrimaryRole);
    });
    Q_EMIT endInsertRows();

    // Update replications.
    for (int j = 0; j < m_outputs.size(); j++) {
        if (i == j) {
            continue;
        }
        QModelIndex index = createIndex(j, 0);
        // Calling this directly ignores possible optimization when the
        // refresh rate hasn't changed in fact. But that's ok.
        Q_EMIT dataChanged(index, index, {ReplicationSourceModelRole,
                                          ReplicationSourceIndexRole});
    }
}

void OutputModel::remove(int outputId)
{
    auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                           [outputId](const Output &output) {
            return output.ptr->id() == outputId;
    });
    if (it != m_outputs.end()) {
        const int index = it - m_outputs.begin();
        Q_EMIT beginRemoveRows(QModelIndex(), index, index);
        m_outputs.erase(it);
        Q_EMIT endRemoveRows();
    }
}

void OutputModel::resetPosition(const Output &output)
{
    if (output.posReset.x() < 0) {
        // KCM was closed in between.
        for (const Output &out : m_outputs) {
            if (out.ptr->id() == output.ptr->id()) {
                continue;
            }
            if (out.ptr->geometry().right() > output.ptr->pos().x()) {
                output.ptr->setPos(out.ptr->geometry().topRight());
            }
        }
    } else {
        output.ptr->setPos(/*output.ptr->pos() - */output.posReset);
    }
}

bool OutputModel::setEnabled(int outputIndex, bool enable)
{
    Output &output = m_outputs[outputIndex];

    if (output.ptr->isEnabled() == enable) {
        return false;
    }

    output.ptr->setEnabled(enable);

    if (enable) {
        resetPosition(output);

        setResolution(outputIndex, resolutionIndex(output.ptr));
        reposition();
    } else {
        output.posReset = output.ptr->pos();
    }

    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(index, index, {EnabledRole});
    return true;
}

inline bool refreshRateCompare(float rate1, float rate2)
{
    return qAbs(rate1 - rate2) < 0.5;
}

bool OutputModel::setResolution(int outputIndex, int resIndex)
{
    const Output &output = m_outputs[outputIndex];
    const auto resolutionList = resolutions(output.ptr);
    if (resIndex < 0 || resIndex >= resolutionList.size()) {
        return false;
    }
    const QSize size = resolutionList[resIndex];

    const float oldRate = output.ptr->currentMode() ? output.ptr->currentMode()->refreshRate() :
                                                      -1;
    const auto modes = output.ptr->modes();

    auto modeIt = std::find_if(modes.begin(), modes.end(),
                 [size, oldRate](const KScreen::ModePtr &mode) {
        // TODO: we don't want to compare against old refresh rate if
        //       refresh rate selection is auto.
        return mode->size() == size &&
                refreshRateCompare(mode->refreshRate(), oldRate);
    });

    if (modeIt == modes.end()) {
        // New resolution does not support previous refresh rate.
        // Get the highest one instead.
        float bestRefreshRate = 0;
        auto it = modes.begin();
        while (it != modes.end()) {
            if ((*it)->size() == size && (*it)->refreshRate() > bestRefreshRate) {
                modeIt = it;
            }
            it++;
        }
    }
    Q_ASSERT(modeIt != modes.end());

    const auto id = (*modeIt)->id();
    if (output.ptr->currentModeId() == id) {
        return false;
    }
    output.ptr->setCurrentModeId(id);

    QModelIndex index = createIndex(outputIndex, 0);
    // Calling this directly ignores possible optimization when the
    // refresh rate hasn't changed in fact. But that's ok.
    Q_EMIT dataChanged(index, index, {ResolutionIndexRole,
                                      SizeRole,
                                      RefreshRateIndexRole});
    Q_EMIT sizeChanged();
    return true;
}

bool OutputModel::setRefreshRate(int outputIndex, int refIndex)
{
    const Output &output = m_outputs[outputIndex];
    const auto rates = refreshRates(output.ptr);
    if (refIndex < 0 || refIndex >= rates.size()) {
        return false;
    }
    const float refreshRate = rates[refIndex];

    const auto modes = output.ptr->modes();
    const auto oldMode = output.ptr->currentMode();

    auto modeIt = std::find_if(modes.begin(), modes.end(),
                 [oldMode, refreshRate](const KScreen::ModePtr &mode) {
        // TODO: we don't want to compare against old refresh rate if
        //       refresh rate selection is auto.
        return mode->size() == oldMode->size() &&
                refreshRateCompare(mode->refreshRate(), refreshRate);
    });
    Q_ASSERT(modeIt != modes.end());

    if (refreshRateCompare(oldMode->refreshRate(), (*modeIt)->refreshRate())) {
        // no change
        return false;
    }
    output.ptr->setCurrentModeId((*modeIt)->id());
    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(index, index, {RefreshRateIndexRole});
    return true;
}

bool OutputModel::setAutoRotate(int outputIndex, bool value)
{
    Output &output = m_outputs[outputIndex];

    if (m_config->autoRotate(output.ptr) == value) {
        return false;
    }
    m_config->setAutoRotate(output.ptr, value);

    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(index, index, {AutoRotateRole});
    return true;
}

bool OutputModel::setAutoRotateOnlyInTabletMode(int outputIndex, bool value)
{
    Output &output = m_outputs[outputIndex];

    if (m_config->autoRotateOnlyInTabletMode(output.ptr) == value) {
        return false;
    }
    m_config->setAutoRotateOnlyInTabletMode(output.ptr, value);

    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(index, index, {AutoRotateOnlyInTabletModeRole});
    return true;
}

bool OutputModel::setRotation(int outputIndex, KScreen::Output::Rotation rotation)
{
    const Output &output = m_outputs[outputIndex];

    if (rotation != KScreen::Output::None
            && rotation != KScreen::Output::Left
            && rotation != KScreen::Output::Inverted
            && rotation != KScreen::Output::Right) {
        return false;
    }
    if (output.ptr->rotation() == rotation) {
        return false;
    }
    if(rotation == KScreen::Output::None)
    {
        QProcess::startDetached("xrandr --screen "+ QString::number(outputIndex) + " -o normal");
    }else if(rotation == KScreen::Output::Left)
    {
        QProcess::startDetached("xrandr --screen "+ QString::number(outputIndex) + " -o left");
    }else if(rotation == KScreen::Output::Inverted)
    {
        QProcess::startDetached("xrandr --screen "+ QString::number(outputIndex) + " -o inverted");
    }else if(rotation == KScreen::Output::Right)
    {
        QProcess::startDetached("xrandr --screen "+ QString::number(outputIndex) + " -o right");
    }
    //output.ptr->setRotation(rotation);

    //QModelIndex index = createIndex(outputIndex, 0);
    //Q_EMIT dataChanged(index, index, {RotationRole, SizeRole});
    //Q_EMIT sizeChanged();
    return true;

}

int OutputModel::resolutionIndex(const KScreen::OutputPtr &output) const
{
    const QSize currentResolution = output->enforcedModeSize();

    if (!currentResolution.isValid()) {
        return 0;
    }

    const auto sizes = resolutions(output);

    const auto it = std::find_if(sizes.begin(),
                                 sizes.end(),
                                 [currentResolution](const QSize &size) {
        return size == currentResolution;
    });
    if (it == sizes.end()) {
        return -1;
    }
    return it - sizes.begin();
}

int OutputModel::refreshRateIndex(const KScreen::OutputPtr &output) const
{
    if (!output->currentMode()) {
        return 0;
    }
    const auto rates = refreshRates(output);
    const float currentRate = output->currentMode()->refreshRate();

    const auto it = std::find_if(rates.begin(),
                                 rates.end(),
                                 [currentRate](float rate) {
        return refreshRateCompare(rate, currentRate);
    });
    if (it == rates.end()) {
        return 0;
    }
    return it - rates.begin();
}

static int greatestCommonDivisor(int a, int b) {
    if (b == 0) {
        return a;
    }
    return greatestCommonDivisor(b, a % b);
}

QVariantList OutputModel::resolutionsStrings(const KScreen::OutputPtr &output) const
{
    QVariantList ret;

    for (const QSize &size : resolutions(output)) {
        int divisor = greatestCommonDivisor(size.width(), size.height());

        // Prefer "16:10" over "8:5"
        if (size.height() / divisor == 5) {
            divisor /= 2;
        }

        const QString text = QString("%1x%2").arg(QString::number(size.width()))
                                                     .arg(QString::number(size.height()));

        ret << text;
    }

    return ret;
}

QVector<QSize> OutputModel::resolutions(const KScreen::OutputPtr &output) const
{
    QVector<QSize> hits;

    for (const auto &mode : output->modes()) {
        const QSize size = mode->size();
        if (!hits.contains(size)) {
            hits << size;
        }
    }
    std::sort(hits.begin(), hits.end(), [](const QSize &a, const QSize &b) {
        if (a.width() > b.width()) {
            return true;
        }
        if (a.width() == b.width() && a.height() > b.height()) {
            return true;
        }
        return false;
    });
    return hits;
}

QVector<float> OutputModel::refreshRates(const KScreen::OutputPtr
                                                  &output) const
{
    QVector<float> hits;

    QSize baseSize;
    if (output->currentMode()) {
        baseSize = output->currentMode()->size();
    } else if (output->preferredMode()) {
        baseSize = output->preferredMode()->size();
    }
    if (!baseSize.isValid()) {
        return hits;
    }

    for (const auto &mode : output->modes()) {
        if (mode->size() != baseSize) {
            continue;
        }
        const float rate = mode->refreshRate();
        if (std::find_if(hits.begin(), hits.end(),
                         [rate](float r) {
                             return refreshRateCompare(r, rate);
                         }) != hits.end()) {
            continue;
        }
        hits << rate;
    }
    return hits;
}

int OutputModel::replicationSourceId(const Output &output) const
{
    const KScreen::OutputPtr source = m_config->replicationSource(output.ptr);
    if (!source) {
        return 0;
    }
    return source->id();
}

QStringList OutputModel::replicationSourceModel(const KScreen::OutputPtr &output) const
{
    QStringList ret = { QObject::tr("None") };

    for (const auto &out : m_outputs) {
        if (out.ptr->id() != output->id()) {
            const int outSourceId = replicationSourceId(out);
            if (outSourceId == output->id()) {
                // 'output' is already source for replication, can't be replica itself
                return { QObject::tr("Replicated by other output") };
            }
            if (outSourceId) {
                // This 'out' is a replica. Can't be a replication source.
                continue;
            }
            ret.append(Utils::outputName(out.ptr));
        }
    }
    return ret;
}

bool OutputModel::setReplicationSourceIndex(int outputIndex, int sourceIndex)
{
    if (outputIndex <= sourceIndex) {
        sourceIndex++;
    }
    if (sourceIndex >= m_outputs.count()) {
        return false;
    }

    Output &output = m_outputs[outputIndex];
    const int oldSourceId = replicationSourceId(output);

    if (sourceIndex < 0) {
        if (oldSourceId == 0) {
            // no change
            return false;
        }
        m_config->setReplicationSource(output.ptr, nullptr);
        output.ptr->setExplicitLogicalSize(QSizeF());
        resetPosition(output);
    } else {
        const auto source = m_outputs[sourceIndex].ptr;
        if (oldSourceId == source->id()) {
            // no change
            return false;
        }
        m_config->setReplicationSource(output.ptr, source);
        output.posReset = output.ptr->pos();
        output.ptr->setPos(source->pos());
        output.ptr->setExplicitLogicalSize(source->explicitLogicalSize());
    }

    reposition();

    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(index, index, {ReplicationSourceIndexRole});

    if (oldSourceId != 0) {
        auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                               [oldSourceId](const Output &out) {
            return out.ptr->id() == oldSourceId;
        });
        if (it != m_outputs.end()) {
            QModelIndex index = createIndex(it - m_outputs.begin(), 0);
            Q_EMIT dataChanged(index, index, {ReplicationSourceModelRole, ReplicasModelRole});
        }
    }
    if (sourceIndex >= 0) {
        QModelIndex index = createIndex(sourceIndex, 0);
        Q_EMIT dataChanged(index, index, {ReplicationSourceModelRole, ReplicasModelRole});
    }
    return true;
}

int OutputModel::replicationSourceIndex(int outputIndex) const
{
    const int sourceId = replicationSourceId(m_outputs[outputIndex]);
    if (!sourceId) {
        return 0;
    }
    for (int i = 0; i < m_outputs.size(); i++) {
        const Output &output = m_outputs[i];
        if (output.ptr->id() == sourceId) {
            return i + (outputIndex > i ? 1 : 0);
        }
    }
    return 0;
}

QVariantList OutputModel::replicasModel(const KScreen::OutputPtr &output) const
{
    QVariantList ret;
    for (int i = 0; i < m_outputs.size(); i++) {
        const Output &out = m_outputs[i];
        if (out.ptr->id() != output->id()) {
            if (replicationSourceId(out) == output->id()) {
                ret << i;
            }
        }
    }
    return ret;
}

void OutputModel::roleChanged(int outputId, OutputRoles role)
{
    for (int i = 0; i < m_outputs.size(); i++) {
        Output &output = m_outputs[i];
        if (output.ptr->id() == outputId) {
            QModelIndex index = createIndex(i, 0);
            Q_EMIT dataChanged(index, index, {role});
            return;
        }
    }
}

bool OutputModel::positionable(const Output &output) const
{
    return output.ptr->isPositionable();
}

void OutputModel::reposition()
{
    int x = 0;
    int y = 0;

    // Find first valid output.
    for (const auto &out : m_outputs) {
        if (positionable(out)) {
            x = out.ptr->pos().x();
            y = out.ptr->pos().y();
            break;
        }
    }

    for (int i = 0; i < m_outputs.size(); i++) {
        if (!positionable(m_outputs[i])) {
            continue;
        }
        const QPoint &cmp = m_outputs[i].ptr->pos();

        if (cmp.x() < x) {
            x = cmp.x();
        }
        if (cmp.y() < y) {
            y = cmp.y();
        }
    }

    if (x == 0 && y == 0) {
        return;
    }

    for (int i = 0; i < m_outputs.size(); i++) {
        auto &out = m_outputs[i];
        out.ptr->setPos(out.ptr->pos() - QPoint(x, y));
        QModelIndex index = createIndex(i, 0);
        Q_EMIT dataChanged(index, index, {NormalizedPositionRole});
    }
    m_config->normalizeScreen();
}

QPoint OutputModel::originDelta() const
{
    int x = 0;
    int y = 0;

    // Find first valid output.
    for (const auto &out : m_outputs) {
        if (positionable(out)) {
            x = out.pos.x();
            y = out.pos.y();
            break;
        }
    }

    for (int i = 1; i < m_outputs.size(); i++) {
        if (!positionable(m_outputs[i])) {
            continue;
        }
        const QPoint &cmp = m_outputs[i].pos;

        if (cmp.x() < x) {
            x = cmp.x();
        }
        if (cmp.y() < y) {
            y = cmp.y();
        }
    }
    return QPoint(x, y);
}

void OutputModel::updatePositions()
{
    const QPoint delta = originDelta();
    for (int i = 0; i < m_outputs.size(); i++) {
        const auto &out = m_outputs[i];
        if (!positionable(out)) {
            continue;
        }
        const QPoint set = out.pos - delta;
        if (out.ptr->pos() != set) {
            out.ptr->setPos(set);
            QModelIndex index = createIndex(i, 0);
            Q_EMIT dataChanged(index, index, {NormalizedPositionRole});
        }
    }
    updateOrder();
}

void OutputModel::updateOrder()
{
    auto order = m_outputs;
    std::sort(order.begin(), order.end(), [](const Output &a, const Output &b) {
        const int xDiff = b.ptr->pos().x() - a.ptr->pos().x();
        const int yDiff = b.ptr->pos().y() - a.ptr->pos().y();
        if (xDiff > 0) {
            return true;
        }
        if (xDiff == 0 && yDiff > 0) {
            return true;
        }
        return false;
    });

    for (int i = 0; i < order.size(); i++) {
        for (int j = 0; j < m_outputs.size(); j++) {
            if (order[i].ptr->id() != m_outputs[j].ptr->id()) {
                continue;
            }
            if (i != j) {
                beginMoveRows(QModelIndex(), j, j, QModelIndex(), i);
                m_outputs.remove(j);
                m_outputs.insert(i, order[i]);
                endMoveRows();
            }
            break;
        }
    }

    // TODO: Could this be optimized by only outputs updating where replica indices changed?
    for (int i = 0; i < m_outputs.size(); i++) {
        QModelIndex index = createIndex(i, 0);
        Q_EMIT dataChanged(index, index, { ReplicasModelRole });
    }
}

bool OutputModel::normalizePositions()
{
    bool changed = false;
    for (int i = 0; i < m_outputs.size(); i++) {
        auto &output = m_outputs[i];
        if (output.pos == output.ptr->pos()) {
            continue;
        }
        if (!positionable(output)) {
            continue;
        }
        changed = true;
        auto index = createIndex(i, 0);
        output.pos = output.ptr->pos();
        Q_EMIT dataChanged(index, index, {PositionRole});
    }
    return changed;
}

bool OutputModel::positionsNormalized() const
{
    // There might be slight deviations because of snapping.
    return originDelta().manhattanLength() < 5;
}

const int s_snapArea = 80;

bool isVerticalClose(const QRect &rect1, const QRect &rect2)
{
    if (rect2.top() - rect1.bottom() > s_snapArea ) {
        return false;
    }
    if (rect1.top() - rect2.bottom() > s_snapArea ) {
        return false;
    }
    return true;
}

bool snapToRight(const QRect &target,
                              const QSize &size,
                              QPoint &dest)
{
    if (qAbs(target.right() - dest.x()) < s_snapArea) {
        // In snap zone for left to right snap.
        dest.setX(target.right() + 1);
        return true;
    }
    if (qAbs(target.right() - (dest.x() + size.width())) < s_snapArea) {
        // In snap zone for right to right snap.
        dest.setX(target.right() - size.width());
        return true;
    }
    return false;
}

bool snapToLeft(const QRect &target,
                             const QSize &size,
                             QPoint &dest)
{
    if (qAbs(target.left() - dest.x()) < s_snapArea) {
        // In snap zone for left to left snap.
        dest.setX(target.left());
        return true;
    }
    if (qAbs(target.left() - (dest.x() + size.width())) < s_snapArea) {
        // In snap zone for right to left snap.
        dest.setX(target.left() - size.width());
        return true;
    }
    return false;
}

bool snapToMiddle(const QRect &target,
                               const QSize &size,
                               QPoint &dest)
{
    const int outputMid = dest.y() + size.height() / 2;
    const int targetMid = target.top() + target.height() / 2;
    if (qAbs(targetMid - outputMid) < s_snapArea) {
        // In snap zone for middle to middle snap.
        dest.setY(targetMid - size.height() / 2);
        return true;
    }
    return false;
}

bool snapToTop(const QRect &target,
                            const QSize &size,
                            QPoint &dest)
{
    if (qAbs(target.top() - dest.y()) < s_snapArea) {
        // In snap zone for bottom to top snap.
        dest.setY(target.top());
        return true;
    }
    if (qAbs(target.top() - (dest.y() + size.height())) < s_snapArea) {
        // In snap zone for top to top snap.
        dest.setY(target.top() - size.height());
        return true;
    }
    return false;
}

bool snapToBottom(const QRect &target,
                               const QSize &size,
                               QPoint &dest)
{
    if (qAbs(target.bottom() - dest.y()) < s_snapArea) {
        // In snap zone for top to bottom snap.
        dest.setY(target.bottom() + 1);
        return true;
    }
    if (qAbs(target.bottom() - (dest.y() + size.height())) < s_snapArea) {
        // In snap zone for bottom to bottom snap.
        dest.setY(target.bottom() - size.height() + 1);
        return true;
    }
    return false;
}

bool snapVertical(const QRect &target,
                               const QSize &size,
                               QPoint &dest)
{
    if (snapToMiddle(target, size, dest)) {
        return true;
    }
    if (snapToBottom(target, size, dest)) {
        return true;
    }
    if (snapToTop(target, size, dest)) {
        return true;
    }
    return false;
}

void OutputModel::snap(const Output &output, QPoint &dest)
{
    const QSize size = output.ptr->geometry().size();
    for (const Output &out : m_outputs) {
        if (out.ptr->id() == output.ptr->id()) {
            // Can not snap to itself.
            continue;
        }
        if (!positionable(out)) {
            continue;
        }

        const QRect target(out.pos, out.ptr->geometry().size());

        if (!isVerticalClose(target, QRect(dest, size))) {
            continue;
        }

        // try snap left to right first
        if (snapToRight(target, size, dest)) {
            snapVertical(target, size, dest);
            continue;
        }
        if (snapToLeft(target, size, dest)) {
            snapVertical(target, size, dest);
            continue;
        }
        if (snapVertical(target, size, dest)) {
            continue;
        }
    }
}
