/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "backendmanager_p.h"
#include "kscreen_debug.h"
#include "mode.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QRect>
#include <QStringList>

#include <algorithm>
#include <utility>

using namespace KScreen;

class Q_DECL_HIDDEN Config::Private : public QObject
{
    Q_OBJECT
public:
    Private(Config *parent)
        : QObject(parent)
        , valid(true)
        , supportedFeatures(Config::Feature::None)
        , tabletModeAvailable(false)
        , tabletModeEngaged(false)
        , q(parent)
    {
    }

    KScreen::OutputPtr findPrimaryOutput() const
    {
        auto iter = std::find_if(outputs.constBegin(), outputs.constEnd(), [](const KScreen::OutputPtr &output) -> bool {
            return output->isPrimary();
        });
        return iter == outputs.constEnd() ? KScreen::OutputPtr() : iter.value();
    }

    // output priorities may be inconsistent after this call
    OutputList::Iterator removeOutput(OutputList::Iterator iter)
    {
        if (iter == outputs.end()) {
            return iter;
        }

        const int outputId = iter.key();
        OutputPtr output = iter.value();

        iter = outputs.erase(iter);

        if (output) {
            output->disconnect(q);
            Q_EMIT q->outputRemoved(outputId);
        }

        return iter;
    }

    bool valid;
    ScreenPtr screen;
    OutputList outputs;
    Features supportedFeatures;
    bool tabletModeAvailable;
    bool tabletModeEngaged;

private:
    Config *q;
};

bool Config::canBeApplied(const ConfigPtr &config)
{
    return canBeApplied(config, ValidityFlag::None);
}

bool Config::canBeApplied(const ConfigPtr &config, ValidityFlags flags)
{
    if (!config) {
        qCDebug(KSCREEN) << "canBeApplied: Config not available, returning false";
        return false;
    }
    ConfigPtr currentConfig = BackendManager::instance()->config();
    if (!currentConfig) {
        qCDebug(KSCREEN) << "canBeApplied: Current config not available, returning false";
        return false;
    }

    QRect rect;
    OutputPtr currentOutput;
    const OutputList outputs = config->outputs();
    int enabledOutputsCount = 0;
    for (const OutputPtr &output : outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        ++enabledOutputsCount;

        currentOutput = currentConfig->output(output->id());
        // If there is no such output
        if (!currentOutput) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "does not exists";
            return false;
        }
        // If the output is not connected
        if (!currentOutput->isConnected()) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "is not connected";
            return false;
        }
        // if there is no currentMode
        if (output->currentModeId().isEmpty()) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "has no currentModeId";
            return false;
        }
        // If the mode is not found in the current output
        if (!currentOutput->mode(output->currentModeId())) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "has no mode:" << output->currentModeId();
            return false;
        }

        const ModePtr currentMode = output->currentMode();

        const QSize outputSize = currentMode->size();

        if (output->pos().x() < rect.x()) {
            rect.setX(output->pos().x());
        }

        if (output->pos().y() < rect.y()) {
            rect.setY(output->pos().y());
        }

        QPoint bottomRight;
        if (output->isHorizontal()) {
            bottomRight = QPoint(output->pos().x() + outputSize.width(), output->pos().y() + outputSize.height());
        } else {
            bottomRight = QPoint(output->pos().x() + outputSize.height(), output->pos().y() + outputSize.width());
        }

        if (bottomRight.x() > rect.width()) {
            rect.setWidth(bottomRight.x());
        }

        if (bottomRight.y() > rect.height()) {
            rect.setHeight(bottomRight.y());
        }
    }

    if (flags & ValidityFlag::RequireAtLeastOneEnabledScreen && enabledOutputsCount == 0) {
        qCDebug(KSCREEN) << "canBeAppled: There are no enabled screens, at least one required";
        return false;
    }

    const int maxEnabledOutputsCount = config->screen()->maxActiveOutputsCount();
    if (enabledOutputsCount > maxEnabledOutputsCount) {
        qCDebug(KSCREEN).nospace() << "canBeApplied: Too many active screens. Requested: " << enabledOutputsCount << ", Max: " << maxEnabledOutputsCount;
        return false;
    }

    if (rect.width() > config->screen()->maxSize().width()) {
        qCDebug(KSCREEN).nospace() << "canBeApplied: The configuration is too wide: " << rect.width() << ", Max: " << config->screen()->maxSize().width();
        return false;
    }
    if (rect.height() > config->screen()->maxSize().height()) {
        qCDebug(KSCREEN).nospace() << "canBeApplied: The configuration is too high: " << rect.height() << ", Max: " << config->screen()->maxSize().height();
        return false;
    }

    return true;
}

Config::Config()
    : QObject(nullptr)
    , d(new Private(this))
{
}

Config::~Config()
{
    delete d;
}

ConfigPtr Config::clone() const
{
    ConfigPtr newConfig(new Config());
    newConfig->d->screen = d->screen->clone();
    newConfig->setSupportedFeatures(supportedFeatures());
    newConfig->setTabletModeAvailable(tabletModeAvailable());
    newConfig->setTabletModeEngaged(tabletModeEngaged());
    for (const OutputPtr &ourOutput : std::as_const(d->outputs)) {
        newConfig->addOutput(ourOutput->clone());
    }
    return newConfig;
}

QString Config::connectedOutputsHash() const
{
    QStringList hashedOutputs;

    const auto outputs = connectedOutputs();
    hashedOutputs.reserve(outputs.count());
    for (const OutputPtr &output : outputs) {
        hashedOutputs << output->hash();
    }
    std::sort(hashedOutputs.begin(), hashedOutputs.end());
    const auto hash = QCryptographicHash::hash(hashedOutputs.join(QString()).toLatin1(), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
}

ScreenPtr Config::screen() const
{
    return d->screen;
}

void Config::setScreen(const ScreenPtr &screen)
{
    d->screen = screen;
}

OutputPtr Config::output(int outputId) const
{
    return d->outputs.value(outputId);
}

Config::Features Config::supportedFeatures() const
{
    return d->supportedFeatures;
}

void Config::setSupportedFeatures(const Config::Features &features)
{
    d->supportedFeatures = features;
}

bool Config::tabletModeAvailable() const
{
    return d->tabletModeAvailable;
}

void Config::setTabletModeAvailable(bool available)
{
    d->tabletModeAvailable = available;
}

bool Config::tabletModeEngaged() const
{
    return d->tabletModeEngaged;
}

void Config::setTabletModeEngaged(bool engaged)
{
    d->tabletModeEngaged = engaged;
}

OutputList Config::outputs() const
{
    return d->outputs;
}

OutputList Config::connectedOutputs() const
{
    OutputList outputs;
    for (const OutputPtr &output : std::as_const(d->outputs)) {
        if (!output->isConnected()) {
            continue;
        }
        outputs.insert(output->id(), output);
    }

    return outputs;
}

OutputPtr Config::primaryOutput() const
{
    return d->findPrimaryOutput();
}

void Config::setPrimaryOutput(const OutputPtr &newPrimary)
{
    setOutputPriority(newPrimary, 1);
}

void Config::addOutput(const OutputPtr &output)
{
    d->outputs.insert(output->id(), output);
    output->setExplicitLogicalSize(logicalSizeForOutput(*output));

    Q_EMIT outputAdded(output);
}

void Config::removeOutput(int outputId)
{
    d->removeOutput(d->outputs.find(outputId));
}

void Config::setOutputs(const OutputList &outputs)
{
    for (auto iter = d->outputs.begin(), end = d->outputs.end(); iter != end;) {
        iter = d->removeOutput(iter);
        end = d->outputs.end();
    }

    for (const OutputPtr &output : outputs) {
        addOutput(output);
    }

    adjustPriorities();
}

void Config::setOutputPriority(const OutputPtr &output, uint32_t priority)
{
    if (!d->outputs.contains(output->id()) || d->outputs[output->id()] != output) {
        qCDebug(KSCREEN) << "The output" << output << "does not belong to this config";
        return;
    }
    if (output->priority() == priority) {
        return;
    }
    output->setEnabled(priority != 0);
    output->setPriority(priority);
    adjustPriorities((priority != 0) ? std::optional(output) : std::nullopt);
}

void Config::setOutputPriorities(QMap<OutputPtr, uint32_t> &priorities)
{
    for (auto it = priorities.constBegin(); it != priorities.constEnd(); it++) {
        const OutputPtr &output = it.key();
        const uint32_t priority = it.value();

        if (!d->outputs.contains(output->id()) || d->outputs[output->id()] != output) {
            qCDebug(KSCREEN) << "The output" << output << "does not belong to this config";
            return;
        }
        output->setEnabled(priority != 0);
        output->setPriority(priority);
    }
    adjustPriorities();
}

static std::optional<OutputPtr> removeOptional(QList<OutputPtr> &haystack, std::optional<OutputPtr> &needle)
{
    if (!needle.has_value()) {
        return std::nullopt;
    }
    const OutputPtr &value = needle.value();
    const bool removed = haystack.removeOne(value);
    return removed ? needle : std::nullopt;
}

void Config::adjustPriorities(std::optional<OutputPtr> keep)
{
    // we need specifically tree-based QMap for this
    QMap<uint32_t, QList<OutputPtr>> multimap;
    uint32_t maxPriority = 0;
    bool found = false;

    for (const OutputPtr &output : d->outputs) {
        maxPriority = std::max(maxPriority, output->priority());
    }

    if (keep.has_value() && keep.value()->priority() == 0) {
        qCDebug(KSCREEN) << "The output to keep" << keep.value() << "has zero priority. Did you forget to set priority after enabling it?";
        keep.reset();
    }
    for (const OutputPtr &output : d->outputs) {
        if (keep.has_value() && keep.value() == output) {
            found = true;
        }
        if (!output->isEnabled()) {
            output->setPriority(0);
        } else {
            // XXX: we are currently not enforcing consistency after enabling an output.
            if (output->priority() == 0) {
                output->setPriority(maxPriority + 1);
            }
            QList<OutputPtr> &entry = multimap[output->priority()];
            entry.append(output);
        }
    }
    if (keep.has_value() && !found) {
        qCDebug(KSCREEN) << "The output to keep" << keep.value() << "is not in the list of outputs" << d->outputs;
        keep.reset();
    }

    uint32_t nextPriority = 1;
    for (QList<OutputPtr> &current_list : multimap) {
        std::optional<OutputPtr> currentKeep = removeOptional(current_list, keep);

        // deterministic sorting of identically-prioritized outputs.
        // ordering reversed, so that later we can use pop() operation instead of removing from the beginning.
        std::stable_sort(current_list.begin(), current_list.end(), [](const OutputPtr &lhs, const OutputPtr &rhs) -> bool {
            return rhs->name() < lhs->name();
        });

        while (currentKeep.has_value() || !current_list.isEmpty()) {
            OutputPtr nextOutput;
            if (currentKeep.has_value() && (currentKeep.value()->priority() <= nextPriority || current_list.isEmpty())) {
                nextOutput = currentKeep.value();
                currentKeep.reset();
            } else {
                Q_ASSERT(!current_list.isEmpty());
                nextOutput = current_list.takeLast();
            }
            nextOutput->setPriority(nextPriority);
            nextPriority += 1;
        }
    }

    Q_EMIT prioritiesChanged();
}

bool Config::isValid() const
{
    return d->valid;
}

void Config::setValid(bool valid)
{
    d->valid = valid;
}

void Config::apply(const ConfigPtr &other)
{
    d->screen->apply(other->screen());

    setTabletModeAvailable(other->tabletModeAvailable());
    setTabletModeEngaged(other->tabletModeEngaged());

    // Remove removed outputs
    for (auto it = d->outputs.begin(); it != d->outputs.end();) {
        if (!other->d->outputs.contains((*it)->id())) {
            it = d->removeOutput(it);
        } else {
            ++it;
        }
    }

    for (const OutputPtr &otherOutput : std::as_const(other->d->outputs)) {
        // Add new outputs
        if (!d->outputs.contains(otherOutput->id())) {
            addOutput(otherOutput->clone());
        } else {
            // Update existing outputs
            d->outputs[otherOutput->id()]->apply(otherOutput);
            d->outputs[otherOutput->id()]->setExplicitLogicalSize(logicalSizeForOutput(*d->outputs[otherOutput->id()]));
        }
    }

    // Update validity
    setValid(other->isValid());

    Q_EMIT prioritiesChanged();
}

QRect Config::outputGeometryForOutput(const KScreen::Output &output) const
{
    QSize size = logicalSizeForOutputInt(output);
    if (!size.isValid()) {
        return QRect();
    }

    return QRect(output.pos(), size);
}

QSizeF Config::logicalSizeForOutput(const KScreen::Output &output) const
{
    QSizeF size = output.enforcedModeSize();
    if (!size.isValid()) {
        return QSizeF();
    }
    // ignore scale where scaling is not per-output
    if (supportedFeatures().testFlag(Feature::PerOutputScaling)) {
        size = size / output.scale();
    }

    // We can't use output.size(), because it does not reflect the actual rotation() set by caller.
    // It is only updated when we get update from KScreen, but not when user changes mode or
    // rotation manually.

    if (!output.isHorizontal()) {
        size = size.transposed();
    }
    return size;
}

QSize Config::logicalSizeForOutputInt(const KScreen::Output &output) const
{
    const QSizeF sizeF = logicalSizeForOutput(output);
    return QSize(std::ceil(sizeF.width()), std::ceil(sizeF.height()));
}

QDebug operator<<(QDebug dbg, const KScreen::ConfigPtr &config)
{
    if (config) {
        dbg << "KScreen::Config(";
        const auto outputs = config->outputs();
        for (const auto &output : outputs) {
            if (output->isConnected()) {
                dbg << Qt::endl << output;
            }
        }
        dbg << ")";
    } else {
        dbg << "KScreen::Config(NULL)";
    }
    return dbg;
}

#include "config.moc"

#include "moc_config.cpp"
