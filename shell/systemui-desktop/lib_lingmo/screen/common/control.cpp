/********************************************************************
Copyright 2019 Roman Gilg <subdiff@gmail.com>

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
#include "control.h"
#include "globals.h"

#include <KDirWatch>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStringBuilder>

#include <kscreen/config.h>
#include <kscreen/output.h>

QString Control::s_dirName = QStringLiteral("control/");

Control::Control(QObject *parent)
    : QObject(parent)
{
}

void Control::activateWatcher()
{
    if (m_watcher) {
        return;
    }
    m_watcher = new KDirWatch(this);
    m_watcher->addFile(filePath());
    connect(m_watcher, &KDirWatch::dirty, this, [this]() {
        readFile();
        Q_EMIT changed();
    });
}

KDirWatch *Control::watcher() const
{
    return m_watcher;
}

bool Control::writeFile()
{
    const QString path = filePath();
    const auto infoMap = constInfo();

    if (infoMap.isEmpty()) {
        // Nothing to write. Default control. Remove file if it exists.
        QFile::remove(path);
        return true;
    }
    if (!QDir().mkpath(dirPath())) {
        return false;
    }

    // write updated data to file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument::fromVariant(infoMap).toJson());
    return true;
}

QString Control::dirPath() const
{
    return Globals::dirPath() % s_dirName;
}

void Control::readFile()
{
    QFile file(filePath());
    if (file.open(QIODevice::ReadOnly)) {
        // This might not be reached, bus this is ok. The control file will
        // eventually be created on first write later on.
        QJsonDocument parser;
        m_info = parser.fromJson(file.readAll()).toVariant().toMap();
    }
}

QString Control::filePathFromHash(const QString &hash) const
{
    return dirPath() % hash;
}

QVariantMap &Control::info()
{
    return m_info;
}

const QVariantMap &Control::constInfo() const
{
    return m_info;
}

Control::OutputRetention Control::convertVariantToOutputRetention(QVariant variant)
{
    if (variant.canConvert<int>()) {
        const auto retention = variant.toInt();
        if (retention == (int)OutputRetention::Global) {
            return OutputRetention::Global;
        }
        if (retention == (int)OutputRetention::Individual) {
            return OutputRetention::Individual;
        }
    }
    return OutputRetention::Undefined;
}

ControlConfig::ControlConfig(KScreen::ConfigPtr config, QObject *parent)
    : Control(parent)
    , m_config(config)
{
    //    qDebug() << "Looking for control file:" << config->connectedOutputsHash();
    readFile();

    // TODO: use a file watcher in case of changes to the control file while
    //       object exists?

    // As global outputs are indexed by a hash of their edid, which is not unique,
    // to be able to tell apart multiple identical outputs, these need special treatment
    QStringList allIds;
    const auto outputs = config->outputs();
    allIds.reserve(outputs.count());
    for (const KScreen::OutputPtr &output : outputs) {
        const auto outputId = output->hashMd5();
        if (allIds.contains(outputId) && !m_duplicateOutputIds.contains(outputId)) {
            m_duplicateOutputIds << outputId;
        }
        allIds << outputId;
    }

    for (auto output : outputs) {
        m_outputsControls << new ControlOutput(output, this);
    }

    // TODO: this is same in Output::readInOutputs of the daemon. Combine?

    // TODO: connect to outputs added/removed signals and reevaluate duplicate ids
    //       in case of such a change while object exists?
}

void ControlConfig::activateWatcher()
{
    if (watcher()) {
        // Watcher was already activated.
        return;
    }
    for (auto *output : m_outputsControls) {
        output->activateWatcher();
        connect(output, &ControlOutput::changed, this, &ControlConfig::changed);
    }
}

QString ControlConfig::dirPath() const
{
    return Control::dirPath() % QStringLiteral("configs/");
}

QString ControlConfig::filePath() const
{
    if (!m_config) {
        return QString();
    }
    return filePathFromHash(m_config->connectedOutputsHash());
}

bool ControlConfig::writeFile()
{
    bool success = true;
    for (auto *outputControl : m_outputsControls) {
        if (getOutputRetention(outputControl->id(), outputControl->name()) == OutputRetention::Individual) {
            continue;
        }
        success &= outputControl->writeFile();
    }
    return success && Control::writeFile();
}

bool ControlConfig::infoIsOutput(const QVariantMap &info, const QString &outputId, const QString &outputName) const
{
    const QString outputIdInfo = info[QStringLiteral("id")].toString();
    if (outputIdInfo.isEmpty()) {
        return false;
    }
    if (outputId != outputIdInfo) {
        return false;
    }

    if (!outputName.isEmpty() && m_duplicateOutputIds.contains(outputId)) {
        // We may have identical outputs connected, these will have the same id in the config
        // in order to find the right one, also check the output's name (usually the connector)
        const auto metadata = info[QStringLiteral("metadata")].toMap();
        const auto outputNameInfo = metadata[QStringLiteral("name")].toString();
        if (outputName != outputNameInfo) {
            // was a duplicate id, but info not for this output
            return false;
        }
    }
    return true;
}

Control::OutputRetention ControlConfig::getOutputRetention(const KScreen::OutputPtr &output) const
{
    return getOutputRetention(output->hashMd5(), output->name());
}

Control::OutputRetention ControlConfig::getOutputRetention(const QString &outputId, const QString &outputName) const
{
    const QVariantList outputsInfo = getOutputs();
    for (const auto &variantInfo : outputsInfo) {
        const QVariantMap info = variantInfo.toMap();
        if (!infoIsOutput(info, outputId, outputName)) {
            continue;
        }
        return convertVariantToOutputRetention(info[QStringLiteral("retention")]);
    }
    // info for output not found
    return OutputRetention::Undefined;
}

static QVariantMap metadata(const QString &outputName)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = outputName;
    return metadata;
}

QVariantMap createOutputInfo(const QString &outputId, const QString &outputName)
{
    QVariantMap outputInfo;
    outputInfo[QStringLiteral("id")] = outputId;
    outputInfo[QStringLiteral("metadata")] = metadata(outputName);
    return outputInfo;
}

void ControlConfig::setOutputRetention(const KScreen::OutputPtr &output, OutputRetention value)
{
    setOutputRetention(output->hashMd5(), output->name(), value);
}

void ControlConfig::setOutputRetention(const QString &outputId, const QString &outputName, OutputRetention value)
{
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[QStringLiteral("retention")] = (int)value;
        *it = outputInfo;
        setOutputs(outputsInfo);
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[QStringLiteral("retention")] = (int)value;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
}

qreal ControlConfig::getScale(const KScreen::OutputPtr &output) const
{
    return getScale(output->hashMd5(), output->name());
}

qreal ControlConfig::getScale(const QString &outputId, const QString &outputName) const
{
    const auto retention = getOutputRetention(outputId, outputName);
    if (retention == OutputRetention::Individual) {
        const QVariantList outputsInfo = getOutputs();
        for (const auto &variantInfo : outputsInfo) {
            const QVariantMap info = variantInfo.toMap();
            if (!infoIsOutput(info, outputId, outputName)) {
                continue;
            }
            const auto val = info[QStringLiteral("scale")];
            return val.canConvert<qreal>() ? val.toReal() : -1;
        }
    }
    // Retention is global or info for output not in config control file.
    if (auto *outputControl = getOutputControl(outputId, outputName)) {
        return outputControl->getScale();
    }

    // Info for output not found.
    return -1;
}

void ControlConfig::setScale(const KScreen::OutputPtr &output, qreal value)
{
    setScale(output->hashMd5(), output->name(), value);
}

// TODO: combine methods (templated functions)
void ControlConfig::setScale(const QString &outputId, const QString &outputName, qreal value)
{
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();

    auto setOutputScale = [&outputId, &outputName, value, this]() {
        if (auto *control = getOutputControl(outputId, outputName)) {
            control->setScale(value);
        }
    };

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[QStringLiteral("scale")] = value;
        *it = outputInfo;
        setOutputs(outputsInfo);
        setOutputScale();
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[QStringLiteral("scale")] = value;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
    setOutputScale();
}

bool ControlConfig::getAutoRotate(const KScreen::OutputPtr &output) const
{
    return getAutoRotate(output->hashMd5(), output->name());
}

bool ControlConfig::getAutoRotate(const QString &outputId, const QString &outputName) const
{
    const auto retention = getOutputRetention(outputId, outputName);
    if (retention == OutputRetention::Individual) {
        const QVariantList outputsInfo = getOutputs();
        for (const auto &variantInfo : outputsInfo) {
            const QVariantMap info = variantInfo.toMap();
            if (!infoIsOutput(info, outputId, outputName)) {
                continue;
            }
            const auto val = info[QStringLiteral("autorotate")];
            return !val.canConvert<bool>() || val.toBool();
        }
    }
    // Retention is global or info for output not in config control file.
    if (auto *outputControl = getOutputControl(outputId, outputName)) {
        return outputControl->getAutoRotate();
    }

    // Info for output not found.
    return true;
}

void ControlConfig::setAutoRotate(const KScreen::OutputPtr &output, bool value)
{
    setAutoRotate(output->hashMd5(), output->name(), value);
}

// TODO: combine methods (templated functions)
void ControlConfig::setAutoRotate(const QString &outputId, const QString &outputName, bool value)
{
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();

    auto setOutputAutoRotate = [&outputId, &outputName, value, this]() {
        if (auto *control = getOutputControl(outputId, outputName)) {
            control->setAutoRotate(value);
        }
    };

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[QStringLiteral("autorotate")] = value;
        *it = outputInfo;
        setOutputs(outputsInfo);
        setOutputAutoRotate();
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[QStringLiteral("autorotate")] = value;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
    setOutputAutoRotate();
}

bool ControlConfig::getAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const
{
    return getAutoRotateOnlyInTabletMode(output->hashMd5(), output->name());
}

bool ControlConfig::getAutoRotateOnlyInTabletMode(const QString &outputId, const QString &outputName) const
{
    const auto retention = getOutputRetention(outputId, outputName);
    if (retention == OutputRetention::Individual) {
        const QVariantList outputsInfo = getOutputs();
        for (const auto &variantInfo : outputsInfo) {
            const QVariantMap info = variantInfo.toMap();
            if (!infoIsOutput(info, outputId, outputName)) {
                continue;
            }
            const auto val = info[QStringLiteral("autorotate-tablet-only")];
            return !val.canConvert<bool>() || val.toBool();
        }
    }
    // Retention is global or info for output not in config control file.
    if (auto *outputControl = getOutputControl(outputId, outputName)) {
        return outputControl->getAutoRotateOnlyInTabletMode();
    }

    // Info for output not found.
    return true;
}

void ControlConfig::setAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output, bool value)
{
    setAutoRotateOnlyInTabletMode(output->hashMd5(), output->name(), value);
}

// TODO: combine methods (templated functions)
void ControlConfig::setAutoRotateOnlyInTabletMode(const QString &outputId, const QString &outputName, bool value)
{
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();

    auto setOutputAutoRotateOnlyInTabletMode = [&outputId, &outputName, value, this]() {
        if (auto *control = getOutputControl(outputId, outputName)) {
            control->setAutoRotateOnlyInTabletMode(value);
        }
    };

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[QStringLiteral("autorotate-tablet-only")] = value;
        *it = outputInfo;
        setOutputs(outputsInfo);
        setOutputAutoRotateOnlyInTabletMode();
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[QStringLiteral("autorotate-tablet-only")] = value;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
    setOutputAutoRotateOnlyInTabletMode();
}

KScreen::OutputPtr ControlConfig::getReplicationSource(const KScreen::OutputPtr &output) const
{
    return getReplicationSource(output->hashMd5(), output->name());
}

KScreen::OutputPtr ControlConfig::getReplicationSource(const QString &outputId, const QString &outputName) const
{
    const QVariantList outputsInfo = getOutputs();
    for (const auto &variantInfo : outputsInfo) {
        const QVariantMap info = variantInfo.toMap();
        if (!infoIsOutput(info, outputId, outputName)) {
            continue;
        }
        const QString sourceHash = info[QStringLiteral("replicate-hash")].toString();
        const QString sourceName = info[QStringLiteral("replicate-name")].toString();

        if (sourceHash.isEmpty() && sourceName.isEmpty()) {
            // Common case when the replication source has been unset.
            return nullptr;
        }

        for (const auto &output : m_config->outputs()) {
            if (output->hashMd5() == sourceHash && output->name() == sourceName) {
                return output;
            }
        }
        // No match.
        return nullptr;
    }
    // Info for output not found.
    return nullptr;
}

void ControlConfig::setReplicationSource(const KScreen::OutputPtr &output, const KScreen::OutputPtr &source)
{
    setReplicationSource(output->hashMd5(), output->name(), source);
}

void ControlConfig::setReplicationSource(const QString &outputId, const QString &outputName, const KScreen::OutputPtr &source)
{
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();
    const QString sourceHash = source ? source->hashMd5() : QStringLiteral("");
    const QString sourceName = source ? source->name() : QStringLiteral("");

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[QStringLiteral("replicate-hash")] = sourceHash;
        outputInfo[QStringLiteral("replicate-name")] = sourceName;
        *it = outputInfo;
        setOutputs(outputsInfo);
        // TODO: shall we set this information also as new global value (like with auto-rotate)?
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[QStringLiteral("replicate-hash")] = sourceHash;
    outputInfo[QStringLiteral("replicate-name")] = sourceName;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
    // TODO: shall we set this information also as new global value (like with auto-rotate)?
}

QVariantList ControlConfig::getOutputs() const
{
    return constInfo()[QStringLiteral("outputs")].toList();
}

void ControlConfig::setOutputs(QVariantList outputsInfo)
{
    auto &infoMap = info();
    infoMap[QStringLiteral("outputs")] = outputsInfo;
}
ControlOutput *ControlConfig::getOutputControl(const QString &outputId, const QString &outputName) const
{
    for (auto *control : m_outputsControls) {
        if (control->id() == outputId && control->name() == outputName) {
            return control;
        }
    }
    return nullptr;
}

ControlOutput::ControlOutput(KScreen::OutputPtr output, QObject *parent)
    : Control(parent)
    , m_output(output)
{
    readFile();
}

QString ControlOutput::id() const
{
    return m_output->hashMd5();
}

QString ControlOutput::name() const
{
    return m_output->name();
}

QString ControlOutput::dirPath() const
{
    return Control::dirPath() % QStringLiteral("outputs/");
}

QString ControlOutput::filePath() const
{
    if (!m_output) {
        return QString();
    }
    return filePathFromHash(m_output->hashMd5());
}

qreal ControlOutput::getScale() const
{
    const auto val = constInfo()[QStringLiteral("scale")];
    return val.canConvert<qreal>() ? val.toReal() : -1;
}

void ControlOutput::setScale(qreal value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[QStringLiteral("scale")] = value;
}

bool ControlOutput::getAutoRotate() const
{
    const auto val = constInfo()[QStringLiteral("autorotate")];
    return !val.canConvert<bool>() || val.toBool();
}

void ControlOutput::setAutoRotate(bool value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[QStringLiteral("autorotate")] = value;
}

bool ControlOutput::getAutoRotateOnlyInTabletMode() const
{
    const auto val = constInfo()[QStringLiteral("autorotate-tablet-only")];
    return !val.canConvert<bool>() || val.toBool();
}

void ControlOutput::setAutoRotateOnlyInTabletMode(bool value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[QStringLiteral("autorotate-tablet-only")] = value;
}
