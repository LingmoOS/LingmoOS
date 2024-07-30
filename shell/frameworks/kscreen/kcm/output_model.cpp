/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "output_model.h"

#include <cstdint>

#include <kscreen/edid.h>
#include <kscreen/mode.h>

#include "../common/utils.h"
#include "config_handler.h"

#include <KLocalizedString>

#include <QRect>
#include <numeric>

OutputModel::OutputModel(ConfigHandler *configHandler)
    : QAbstractListModel(configHandler)
    , m_config(configHandler)
{
    connect(m_config->config().data(), &KScreen::Config::prioritiesChanged, this, [this]() {
        if (rowCount() > 0) {
            Q_EMIT dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0), {PriorityRole});
        }
    });
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
    case Qt::DisplayRole: {
        const bool shouldShowSerialNumber = std::any_of(m_outputs.cbegin(), m_outputs.cend(), [output](const OutputModel::Output &other) {
            return other.ptr->id() != output->id() // avoid same output
                && other.ptr->edid() && output->edid() //
                && other.ptr->edid()->vendor() == output->edid()->vendor() //
                && other.ptr->edid()->name() == output->edid()->name(); // model
        });
        const bool shouldShowConnector =
            shouldShowSerialNumber && std::any_of(m_outputs.cbegin(), m_outputs.cend(), [output](const OutputModel::Output &other) {
                return other.ptr->id() != output->id() // avoid same output
                    && other.ptr->edid()->serial() == output->edid()->serial();
            });
        return Utils::outputName(output, shouldShowSerialNumber, shouldShowConnector);
    }
    case EnabledRole:
        return output->isEnabled();
    case InternalRole:
        return output->type() == KScreen::Output::Type::Panel;
    case PriorityRole:
        return output->priority();
    case SizeRole:
        return output->geometry().size();
    case PositionRole:
        return m_outputs[index.row()].pos;
    case NormalizedPositionRole:
        return output->geometry().topLeft();
    case AutoRotateRole:
        return static_cast<uint32_t>(output->autoRotatePolicy());
    case RotationRole:
        return output->rotation();
    case ScaleRole:
        return output->scale();
    case ResolutionIndexRole:
        return resolutionIndex(output);
    case ResolutionsRole:
        return resolutionsStrings(output);
    case ResolutionRole:
        return resolution(output);
    case RefreshRateIndexRole:
        return refreshRateIndex(output);
    case ReplicationSourceModelRole:
        return replicationSourceModel(output);
    case ReplicationSourceIndexRole:
        return replicationSourceIndex(index.row());
    case ReplicasModelRole:
        return replicasModel(output);
    case RefreshRatesRole: {
        QVariantList ret;
        const auto rates = refreshRates(output);
        for (const auto rate : rates) {
            ret << i18n("%1 Hz", int(rate + 0.5));
        }
        return ret;
    }
    case CapabilitiesRole:
        return static_cast<uint32_t>(output->capabilities());
    case OverscanRole:
        return output->overscan();
    case VrrPolicyRole:
        return static_cast<uint32_t>(output->vrrPolicy());
    case RgbRangeRole:
        return static_cast<uint32_t>(output->rgbRange());
    case InteractiveMoveRole:
        return m_outputs[index.row()].moving;
    case IccProfileRole:
        return output->iccProfilePath();
    case HdrRole:
        return output->isHdrEnabled() && output->isWcgEnabled();
    case SdrBrightnessRole:
        return output->sdrBrightness();
    case MaxBrightnessRole:
        return output->maxPeakBrightnessOverride().value_or(output->maxPeakBrightness());
    case SdrGamutWideness:
        return output->sdrGamutWideness();
    case ColorProfileSource:
        return static_cast<uint32_t>(output->colorProfileSource());
    }
    return QVariant();
}

bool OutputModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
    case PriorityRole:
        if (value.canConvert<uint32_t>()) {
            const uint32_t priority = value.toUInt();
            if (output.ptr->priority() == priority) {
                return false;
            }
            m_config->config()->setOutputPriority(output.ptr, priority);
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
    case ResolutionRole:
        // unimplemented
        return false;
        break;
    case AutoRotateRole:
        if (value.canConvert<uint32_t>()) {
            Output &output = m_outputs[index.row()];
            const auto policy = static_cast<KScreen::Output::AutoRotatePolicy>(value.toUInt());
            if (output.ptr->autoRotatePolicy() == policy) {
                return false;
            }
            output.ptr->setAutoRotatePolicy(policy);
            Q_EMIT dataChanged(index, index, {AutoRotateRole});
        }
        break;
    case RotationRole:
        if (value.canConvert<KScreen::Output::Rotation>()) {
            return setRotation(index.row(), value.value<KScreen::Output::Rotation>());
        }
        break;
    case ReplicationSourceIndexRole:
        if (value.canConvert<int>()) {
            return setReplicationSourceIndex(index.row(), value.toInt() - 1);
        }
        break;
    case ScaleRole: {
        bool ok;
        const qreal scale = value.toReal(&ok);
        if (ok && !qFuzzyCompare(output.ptr->scale(), scale)) {
            const auto oldSize = output.ptr->explicitLogicalSizeInt();

            output.ptr->setScale(scale);

            const auto newSize = m_config->config()->logicalSizeForOutputInt(*output.ptr);
            output.ptr->setExplicitLogicalSize(newSize);

            maintainSnapping(output, oldSize, newSize);

            Q_EMIT sizeChanged();
            Q_EMIT dataChanged(index, index, {role, SizeRole});
            return true;
        }
        break;
    }
    case OverscanRole:
        if (value.canConvert<uint32_t>()) {
            Output &output = m_outputs[index.row()];
            const uint32_t overscan = value.toUInt();
            if (output.ptr->overscan() == overscan) {
                return false;
            }
            output.ptr->setOverscan(overscan);
            m_config->setOverscan(output.ptr, overscan);
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        break;
    case VrrPolicyRole:
        if (value.canConvert<uint32_t>()) {
            Output &output = m_outputs[index.row()];
            const auto policy = static_cast<KScreen::Output::VrrPolicy>(value.toUInt());
            if (output.ptr->vrrPolicy() == policy) {
                return false;
            }
            output.ptr->setVrrPolicy(policy);
            m_config->setVrrPolicy(output.ptr, policy);
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        break;
    case RgbRangeRole:
        if (value.canConvert<uint32_t>()) {
            Output &output = m_outputs[index.row()];
            const auto range = static_cast<KScreen::Output::RgbRange>(value.toUInt());
            if (output.ptr->rgbRange() == range) {
                return false;
            }
            output.ptr->setRgbRange(range);
            m_config->setRgbRange(output.ptr, range);
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        break;
    case InteractiveMoveRole:
        if (value.canConvert<bool>()) {
            m_outputs[index.row()].moving = value.toBool();
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        break;
    case IccProfileRole:
        m_outputs[index.row()].ptr->setIccProfilePath(value.toString());
        Q_EMIT dataChanged(index, index, {role});
        return true;
    case HdrRole:
        output.ptr->setHdrEnabled(value.toBool());
        output.ptr->setWcgEnabled(value.toBool());
        Q_EMIT dataChanged(index, index, {role});
        return true;
    case SdrBrightnessRole:
        output.ptr->setSdrBrightness(value.toUInt());
        Q_EMIT dataChanged(index, index, {role});
        return true;
    case SdrGamutWideness:
        output.ptr->setSdrGamutWideness(value.toDouble());
        Q_EMIT dataChanged(index, index, {role});
        return true;
    case ColorProfileSource:
        output.ptr->setColorProfileSource(static_cast<KScreen::Output::ColorProfileSource>(value.toUInt()));
        Q_EMIT dataChanged(index, index, {role});
        return true;
    }
    return false;
}

QHash<int, QByteArray> OutputModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[EnabledRole] = "enabled";
    roles[InternalRole] = "internal";
    roles[PriorityRole] = "priority";
    roles[SizeRole] = "size";
    roles[PositionRole] = "position";
    roles[NormalizedPositionRole] = "normalizedPosition";
    roles[AutoRotateRole] = "autoRotate";
    roles[RotationRole] = "rotation";
    roles[ScaleRole] = "scale";
    roles[ResolutionIndexRole] = "resolutionIndex";
    roles[ResolutionsRole] = "resolutions";
    roles[ResolutionRole] = "resolution";
    roles[RefreshRateIndexRole] = "refreshRateIndex";
    roles[RefreshRatesRole] = "refreshRates";
    roles[ReplicationSourceModelRole] = "replicationSourceModel";
    roles[ReplicationSourceIndexRole] = "replicationSourceIndex";
    roles[ReplicasModelRole] = "replicasModel";
    roles[CapabilitiesRole] = "capabilities";
    roles[OverscanRole] = "overscan";
    roles[VrrPolicyRole] = "vrrPolicy";
    roles[RgbRangeRole] = "rgbRange";
    roles[InteractiveMoveRole] = "interactiveMove";
    roles[IccProfileRole] = "iccProfilePath";
    roles[HdrRole] = "hdr";
    roles[SdrBrightnessRole] = "sdrBrightness";
    roles[MaxBrightnessRole] = "peakBrightness";
    roles[SdrGamutWideness] = "sdrGamutWideness";
    roles[ColorProfileSource] = "colorProfileSource";
    return roles;
}

void OutputModel::add(const KScreen::OutputPtr &output)
{
    const int insertPos = m_outputs.count();
    beginInsertRows(QModelIndex(), insertPos, insertPos);

    int i = 0;
    while (i < m_outputs.size()) {
        const QPoint pos = m_outputs[i].ptr->pos();
        if (output->pos().x() < pos.x()) {
            break;
        }
        if (output->pos().x() == pos.x() && output->pos().y() < pos.y()) {
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

    endInsertRows();

    connect(output.data(), &KScreen::Output::modesChanged, this, [this, output]() {
        rolesChanged(output->id(), {ResolutionsRole, ResolutionIndexRole, ResolutionRole, SizeRole});
        Q_EMIT sizeChanged();
    });

    // Update replications.
    for (int j = 0; j < m_outputs.size(); j++) {
        if (i == j) {
            continue;
        }
        QModelIndex index = createIndex(j, 0);
        // Calling this directly ignores possible optimization when the
        // refresh rate hasn't changed in fact. But that's ok.
        Q_EMIT dataChanged(index, index, {ReplicationSourceModelRole, ReplicationSourceIndexRole});
    }
}

void OutputModel::remove(int outputId)
{
    auto it = std::find_if(m_outputs.begin(), m_outputs.end(), [outputId](const Output &output) {
        return output.ptr->id() == outputId;
    });
    if (it != m_outputs.end()) {
        const int index = it - m_outputs.begin();
        beginRemoveRows(QModelIndex(), index, index);
        m_outputs.erase(it);
        endRemoveRows();
    }
}

void OutputModel::resetPosition(Output &output)
{
    if (!output.posReset.has_value()) {
        // KCM was closed in between.
        for (const Output &out : std::as_const(m_outputs)) {
            if (out.ptr->id() == output.ptr->id()) {
                continue;
            }
            const auto geometry = out.ptr->geometry();
            if (geometry.x() + geometry.width() > output.ptr->pos().x()) {
                output.ptr->setPos(QPoint(geometry.x() + geometry.width(), geometry.top()));
            }
        }
    } else {
        QPoint reset = output.posReset.value();
        output.posReset.reset();
        QPoint shift = QPoint(0, 0);

        if (reset.x() < 0) {
            shift.setX(-reset.x());
            reset.setX(0);
        }
        if (reset.y() < 0) {
            shift.setY(-reset.y());
            reset.setY(0);
        }

        for (Output &out : m_outputs) {
            if (out.ptr->id() == output.ptr->id()) {
                continue;
            }
            if (positionable(out)) {
                out.ptr->setPos(out.ptr->pos() + shift);
            }
        }
        output.ptr->setPos(reset);
    }

    // TODO: this function is called when positioning programatically,
    //   it may make sense to run the final positions through the snapping logic
    //   to make sure the results are consistent with manual snapping
}

QPoint OutputModel::mostTopLeftLocationOfPositionableOutputOptionallyIgnoringOneOfThem(std::optional<KScreen::OutputPtr> ignored) const
{
    auto foldTopLeft = [this, ignored](std::optional<QPoint> a, const Output &out) {
        if (!positionable(out) || (ignored.has_value() && out.ptr->id() == ignored.value()->id())) {
            return a;
        }
        if (a.has_value()) {
            return std::optional(QPoint(std::min(a.value().x(), out.pos.x()), std::min(a.value().y(), out.pos.y())));
        } else {
            return std::optional(out.pos);
        }
    };
    return std::accumulate(m_outputs.constBegin(), m_outputs.constEnd(), std::optional<QPoint>(), foldTopLeft).value_or(QPoint(0, 0));
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
    } else {
        // assuming it was already properly normalized, so current topleft (without disabling) is (0,0)
        const QPoint topLeft = mostTopLeftLocationOfPositionableOutputOptionallyIgnoringOneOfThem(std::optional(output.ptr));

        QPoint reset = output.ptr->pos();
        if (topLeft.x() > 0) {
            reset.setX(-topLeft.x());
        }
        if (topLeft.y() > 0) {
            reset.setY(-topLeft.y());
        }

        output.posReset = std::optional(reset);
    }

    reposition();

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

    const float oldRate = output.ptr->currentMode() ? output.ptr->currentMode()->refreshRate() : -1;
    const auto modes = output.ptr->modes();

    auto modeIt = std::find_if(modes.begin(), modes.end(), [size, oldRate](const KScreen::ModePtr &mode) {
        // TODO: we don't want to compare against old refresh rate if
        //       refresh rate selection is auto.
        return mode->size() == size && refreshRateCompare(mode->refreshRate(), oldRate);
    });

    if (modeIt == modes.end()) {
        // New resolution does not support previous refresh rate.
        // Get the highest one instead.
        float bestRefreshRate = 0;
        auto it = modes.begin();
        while (it != modes.end()) {
            if ((*it)->size() == size && (*it)->refreshRate() > bestRefreshRate) {
                bestRefreshRate = (*it)->refreshRate();
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
    const auto oldSize = output.ptr->explicitLogicalSizeInt();
    output.ptr->setCurrentModeId(id);
    output.ptr->setSize(output.ptr->currentMode()->size());

    const auto newSize = m_config->config()->logicalSizeForOutputInt(*output.ptr);
    output.ptr->setExplicitLogicalSize(newSize);

    maintainSnapping(output, oldSize, newSize);

    QModelIndex index = createIndex(outputIndex, 0);
    // Calling this directly ignores possible optimization when the
    // refresh rate hasn't changed in fact. But that's ok.
    Q_EMIT dataChanged(index, index, {ResolutionIndexRole, ResolutionRole, SizeRole, RefreshRatesRole, RefreshRateIndexRole});
    Q_EMIT sizeChanged();
    return true;
}

bool OutputModel::setRefreshRate(int outputIndex, int refIndex)
{
    const Output &output = m_outputs[outputIndex];
    const auto rates = refreshRates(output.ptr);
    if (refIndex < 0 || refIndex >= rates.size() || !output.ptr->isEnabled()) {
        return false;
    }
    const float refreshRate = rates[refIndex];

    const auto modes = output.ptr->modes();
    const auto oldMode = output.ptr->currentMode();

    auto modeIt = std::find_if(modes.begin(), modes.end(), [oldMode, refreshRate](const KScreen::ModePtr &mode) {
        // TODO: we don't want to compare against old refresh rate if
        //       refresh rate selection is auto.
        return mode->size() == oldMode->size() && refreshRateCompare(mode->refreshRate(), refreshRate);
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

bool OutputModel::setRotation(int outputIndex, KScreen::Output::Rotation rotation)
{
    const Output &output = m_outputs[outputIndex];

    if (rotation != KScreen::Output::None && rotation != KScreen::Output::Left && rotation != KScreen::Output::Inverted && rotation != KScreen::Output::Right) {
        return false;
    }
    if (output.ptr->rotation() == rotation) {
        return false;
    }
    const auto oldSize = output.ptr->explicitLogicalSizeInt();
    output.ptr->setRotation(rotation);

    const auto newSize = m_config->config()->logicalSizeForOutputInt(*output.ptr);
    output.ptr->setExplicitLogicalSize(newSize);

    maintainSnapping(output, oldSize, newSize);

    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(index, index, {RotationRole, SizeRole});
    Q_EMIT sizeChanged();
    return true;
}

int OutputModel::resolutionIndex(const KScreen::OutputPtr &output) const
{
    const QSize currentResolution = output->enforcedModeSize();

    if (!currentResolution.isValid()) {
        return 0;
    }

    const auto sizes = resolutions(output);

    const auto it = std::find_if(sizes.begin(), sizes.end(), [currentResolution](const QSize &size) {
        return size == currentResolution;
    });
    if (it == sizes.end()) {
        return -1;
    }
    return it - sizes.begin();
}

QSize OutputModel::resolution(const KScreen::OutputPtr &output) const
{
    const QSize currentResolution = output->enforcedModeSize();

    if (!currentResolution.isValid()) {
        return QSize();
    }

    return currentResolution;
}

int OutputModel::refreshRateIndex(const KScreen::OutputPtr &output) const
{
    if (!output->currentMode()) {
        return 0;
    }
    const auto rates = refreshRates(output);
    const float currentRate = output->currentMode()->refreshRate();

    const auto it = std::find_if(rates.begin(), rates.end(), [currentRate](float rate) {
        return refreshRateCompare(rate, currentRate);
    });
    if (it == rates.end()) {
        return 0;
    }
    return it - rates.begin();
}

static int greatestCommonDivisor(int a, int b)
{
    if (b == 0) {
        return a;
    }
    return greatestCommonDivisor(b, a % b);
}

QVariantList OutputModel::resolutionsStrings(const KScreen::OutputPtr &output) const
{
    QVariantList ret;
    const auto resolutionList = resolutions(output);
    for (const QSize &size : resolutionList) {
        if (size.isEmpty()) {
            const QString text = i18nc("Width x height",
                                       "%1x%2",
                                       // Explicitly not have it add thousand-separators.
                                       QString::number(size.width()),
                                       QString::number(size.height()));
            ret << text;
        } else {
            int divisor = greatestCommonDivisor(size.width(), size.height());

            if (size.height() / divisor == 5 || size.height() / divisor == 8) { // Prefer "16:10" over "8:5"
                divisor /= 2;
            } else if (size.height() / divisor == 27 || size.height() / divisor == 64) { // Prefer "21:9" over "64:27"
                divisor *= 3;
            } else if (size.height() / divisor == 18 || size.height() / divisor == 43) { // Prefer "21:9" over "43:18"
                divisor *= 2;
            } else if (size.height() / divisor == 384 || size.height() / divisor == 683) { // Prefer "16:9" over "683:384"
                divisor *= 41;
            }

            const QString text = i18nc("Width x height (aspect ratio)",
                                       "%1x%2 (%3:%4)",
                                       // Explicitly not have it add thousand-separators.
                                       QString::number(size.width()),
                                       QString::number(size.height()),
                                       size.width() / divisor,
                                       size.height() / divisor);

            ret << text;
        }
    }
    return ret;
}

QList<QSize> OutputModel::resolutions(const KScreen::OutputPtr &output) const
{
    QList<QSize> hits;

    const auto modes = output->modes();
    for (const auto &mode : modes) {
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

QList<float> OutputModel::refreshRates(const KScreen::OutputPtr &output) const
{
    QList<float> hits;

    QSize baseSize;
    if (output->currentMode()) {
        baseSize = output->currentMode()->size();
    } else if (output->preferredMode()) {
        baseSize = output->preferredMode()->size();
    }
    if (!baseSize.isValid()) {
        return hits;
    }

    const auto modes = output->modes();
    for (const auto &mode : modes) {
        if (mode->size() != baseSize) {
            continue;
        }
        const float rate = mode->refreshRate();
        if (std::find_if(hits.begin(),
                         hits.end(),
                         [rate](float r) {
                             return refreshRateCompare(r, rate);
                         })
            != hits.end()) {
            continue;
        }
        hits << rate;
    }
    std::stable_sort(hits.begin(), hits.end(), std::greater<>());
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
    QStringList ret = {i18n("None")};

    for (const auto &out : m_outputs) {
        if (out.ptr->id() != output->id()) {
            const int outSourceId = replicationSourceId(out);
            if (outSourceId == output->id()) {
                // 'output' is already source for replication, can't be replica itself
                return {i18n("Replicated by other output")};
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

static KScreen::ModePtr getBestMode(const KScreen::OutputPtr &output, const KScreen::OutputPtr &source)
{
    auto calculateAspectRatio = [](const auto &output, const auto &mode) {
        const qreal ratio = mode->size().width() / qreal(mode->size().height());
        const qreal ratioTransposed = mode->size().height() / qreal(mode->size().width());
        switch (output->rotation()) {
        case KScreen::Output::Left:
        case KScreen::Output::Right:
            return ratioTransposed;
        default:
            return ratio;
        }
    };
    const qreal sourceRatio = calculateAspectRatio(source, source->currentMode());
    // 1.1: Find modes with the same aspect ratio as the source output; if none, don't change the mode
    std::vector<KScreen::ModePtr> availableModes(output->modes().cbegin(), output->modes().cend());
    std::erase_if(availableModes, [&calculateAspectRatio, &output, sourceRatio](const auto &mode) {
        return !qFuzzyCompare(calculateAspectRatio(output, mode), sourceRatio);
    });
    if (availableModes.empty()) {
        return output->currentMode();
    }

    // 1.2: Use the smallest mode at least as large as the source output; if none, use the largest mode
    std::sort(availableModes.begin(), availableModes.end(), [&output](const auto &a, const auto &b) {
        return a->size().width() < b->size().width();
    });
    auto getRotatedSize = [](const auto &output, const auto &mode) {
        if (output->rotation() == KScreen::Output::Left || output->rotation() == KScreen::Output::Right) {
            return mode->size().transposed();
        }
        return mode->size();
    };
    const auto sourceSize = getRotatedSize(source, source->currentMode());
    const auto it = std::find_if(availableModes.begin(), availableModes.end(), [sourceSize](const auto &mode) {
        return mode->size().width() >= sourceSize.width();
    });
    if (it != availableModes.end()) {
        return *it;
    } else {
        return availableModes.back();
    }
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

        // In replicating outputs, we don't change the source
        // Step 1: set the destination mode, if possible
        const auto bestMode = getBestMode(output.ptr, source);
        if (!bestMode) {
            return false;
        }
        // Step 2: reposition ans scale destination output to be centered inside the source output
        auto sourceSize = source->currentMode()->size();
        auto destinationSize = bestMode->size();
        if (source->rotation() == KScreen::Output::Left || source->rotation() == KScreen::Output::Right) {
            sourceSize = sourceSize.transposed();
        }
        if (output.ptr->rotation() == KScreen::Output::Left || output.ptr->rotation() == KScreen::Output::Right) {
            destinationSize = destinationSize.transposed();
        }
        qreal scale = source->scale() * std::max(destinationSize.width() / qreal(sourceSize.width()), destinationSize.height() / qreal(sourceSize.height()));
        // round up to integer multiples of 1/120 to avoid issues with triggering hidden panels from the edge
        scale = std::ceil(scale * 120.0) / 120.0;
        const QPoint relPos((sourceSize.width() / source->scale() - destinationSize.width() / scale) / 2,
                            (sourceSize.height() / source->scale() - destinationSize.height() / scale) / 2);

        output.ptr->setCurrentModeId(bestMode->id());
        output.ptr->setScale(scale);
        m_config->setReplicationSource(output.ptr, source);
        output.posReset = std::optional(output.ptr->pos());
        output.ptr->setPos(source->pos() + relPos);
    }

    reposition();

    QModelIndex index = createIndex(outputIndex, 0);
    Q_EMIT dataChanged(
        index,
        index,
        {ReplicationSourceIndexRole, SizeRole, NormalizedPositionRole, ScaleRole, RefreshRatesRole, RefreshRateIndexRole, ResolutionRole, ResolutionIndexRole});

    if (oldSourceId != 0) {
        auto it = std::find_if(m_outputs.begin(), m_outputs.end(), [oldSourceId](const Output &out) {
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

void OutputModel::rolesChanged(int outputId, const QList<int> &roles)
{
    const auto index = indexForOutputId(outputId);
    if (index.isValid()) {
        Q_EMIT dataChanged(index, index, roles);
    }
}

QModelIndex OutputModel::indexForOutputId(int outputId) const
{
    for (int i = 0; i < m_outputs.size(); i++) {
        const Output &output = m_outputs[i];
        if (output.ptr->id() == outputId) {
            return createIndex(i, 0);
        }
    }
    return QModelIndex();
}

bool OutputModel::positionable(const Output &output) const
{
    return output.ptr->isPositionable();
}

bool OutputModel::isMoving() const
{
    return std::any_of(m_outputs.cbegin(), m_outputs.cend(), std::mem_fn(&Output::moving));
}

void OutputModel::reposition()
{
    int x = 0;
    int y = 0;

    // Find first valid output.
    for (const auto &out : std::as_const(m_outputs)) {
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

void OutputModel::updatePositions()
{
    const QPoint delta = mostTopLeftLocationOfPositionableOutputOptionallyIgnoringOneOfThem();
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
    return mostTopLeftLocationOfPositionableOutputOptionallyIgnoringOneOfThem().manhattanLength() < 5;
}

const int s_snapArea = 80;

bool isVerticalClose(const QRect &rect1, const QRect &rect2)
{
    if (rect2.top() - (rect1.y() + rect1.height()) > s_snapArea) {
        return false;
    }
    if (rect1.top() - (rect2.y() + rect2.height()) > s_snapArea) {
        return false;
    }
    return true;
}

bool snapToRight(const QRect &target, const QSize &size, QPoint &dest)
{
    if (qAbs(target.x() + target.width() - dest.x()) < s_snapArea) {
        // In snap zone for left to right snap.
        dest.setX(target.x() + target.width());
        return true;
    }
    if (qAbs(target.x() + target.width() - (dest.x() + size.width())) < s_snapArea) {
        // In snap zone for right to right snap.
        dest.setX(target.x() + target.width() - size.width());
        return true;
    }
    return false;
}

bool snapToLeft(const QRect &target, const QSize &size, QPoint &dest)
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

bool snapHorizontal(const QRect &target, const QSize &size, QPoint &dest)
{
    if (snapToRight(target, size, dest)) {
        return true;
    }
    if (snapToLeft(target, size, dest)) {
        return true;
    }
    return false;
}

bool snapToMiddle(const QRect &target, const QSize &size, QPoint &dest)
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

bool snapToTop(const QRect &target, const QSize &size, QPoint &dest)
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

bool snapToBottom(const QRect &target, const QSize &size, QPoint &dest)
{
    if (qAbs(target.y() + target.height() - dest.y()) < s_snapArea) {
        // In snap zone for top to bottom snap.
        dest.setY(target.y() + target.height());
        return true;
    }
    if (qAbs(target.y() + target.height() - (dest.y() + size.height())) < s_snapArea) {
        // In snap zone for bottom to bottom snap.
        dest.setY(target.y() + target.height() - size.height());
        return true;
    }
    return false;
}

bool snapVertical(const QRect &target, const QSize &size, QPoint &dest)
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
    const QRect outputRect(dest, size);

    QList<std::reference_wrapper<const Output>> positionableOutputs;
    positionableOutputs.reserve(m_outputs.size());
    std::copy_if(m_outputs.cbegin(), m_outputs.cend(), std::back_inserter(positionableOutputs), [](const Output &output) {
        return output.ptr->isPositionable();
    });

    // Special case for two outputs, we want to make sure they always touch;
    if (positionableOutputs.size() == 2) {
        const Output &other = positionableOutputs.at(0).get().ptr->id() == output.ptr->id() ? positionableOutputs.at(1) : positionableOutputs.at(0);
        const QRect target(other.pos, other.ptr->geometry().size());
        const bool xOverlap = dest.x() <= target.x() + target.width() && target.x() <= dest.x() + size.width();
        const bool yOverlap = dest.y() <= target.y() + target.height() && target.y() <= dest.y() + size.height();
        // Special special case, snap to center if centers are close
        if (std::abs((outputRect.center() - target.center()).manhattanLength()) < s_snapArea * 2) {
            dest = target.center() - (outputRect.center() - outputRect.topLeft());
            return;
        }
        if (xOverlap) {
            const int topDist = std::abs(dest.y() - target.y() - target.height());
            const int bottomDist = std::abs(outputRect.y() + outputRect.height() - target.y());
            if (topDist < bottomDist) {
                dest.setY(target.y() + target.height());
            } else {
                dest.setY(target.y() - size.height());
            }
            // Secondary snap to align the other edges if close - right to right, left to left
            snapHorizontal(target, size, dest);
            return;
        }
        if (yOverlap) {
            const int leftDist = std::abs(dest.x() - target.x() - target.width());
            const int rightDiff = std::abs(outputRect.x() + outputRect.width() - target.x());
            if (leftDist < rightDiff) {
                dest.setX(target.x() + target.width());
            } else {
                dest.setX(target.x() - size.width());
            }
            // Secondary snap to align the other edges if close - top to top, bottom to bottom, center to center
            snapVertical(target, size, dest);
            return;
        }
        // No overlap at all can happen at a corner, do not let the output move away
        dest = output.pos;
        return;
    }

    for (const Output &out : std::as_const(positionableOutputs)) {
        if (out.ptr->id() == output.ptr->id()) {
            // Can not snap to itself.
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

void OutputModel::maintainSnapping(const OutputModel::Output &changedOutput, const QSize &oldSize, const QSize &newSize)
{
    const auto changedCenter = QRect(changedOutput.ptr->pos(), oldSize).center();

    const auto dSize = newSize - oldSize;
    const auto delta = QPoint(dSize.width(), dSize.height());

    auto updated = false;
    for (auto &output : m_outputs) {
        if (output.ptr->id() == changedOutput.ptr->id()) {
            continue;
        }

        const auto pos = output.ptr->pos();
        const auto isXTranslate = pos.x() >= changedCenter.x();
        const auto isYTranslate = pos.y() >= changedCenter.y();
        const auto translation = QPoint(isXTranslate ? delta.x() : 0, isYTranslate ? delta.y() : 0);
        if (translation.isNull()) {
            continue;
        }

        output.pos = pos + translation;
        updated = true;
    }

    if (updated) {
        updatePositions();
    }
}

#include "moc_output_model.cpp"
