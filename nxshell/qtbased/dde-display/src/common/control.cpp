// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "control.h"
#include "globals.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStringBuilder>

#include <kscreen/config.h>

// clang-format off
#define retentionString                 QStringLiteral("retention")
#define nameString                      QStringLiteral("name")
#define scaleString                     QStringLiteral("scale")
#define metadataString                  QStringLiteral("metadata")
#define idString                        QStringLiteral("id")
#define autorotateString                QStringLiteral("autorotate")
#define autorotateTabletOnlyString      QStringLiteral("autorotate-tablet-only")
#define replicateHashString             QStringLiteral("replicate-hash")
#define replicateNameString             QStringLiteral("replicate-name")
#define overscanString                  QStringLiteral("overscan")
#define vrrPolicyString                 QStringLiteral("vrrpolicy")
#define rgbRangeString                  QStringLiteral("rgbrange")
#define outputsString                   QStringLiteral("outputs")
// clang-format on

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

    //TODO
//    m_watcher = new KDirWatch(this);
//    m_watcher->addFile(filePath());
//    connect(m_watcher, &KDirWatch::dirty, this, [this]() {
//        readFile();
//        Q_EMIT changed();
//    });
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
        // TODO: error message
        return false;
    }

    // write updated data to file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        // TODO: logging category?
        //        qCWarning(KSCREEN_COMMON) << "Failed to open config control file for writing! " << file.errorString();
        return false;
    }
    file.write(QJsonDocument::fromVariant(infoMap).toJson());
    //    qCDebug(KSCREEN_COMMON) << "Control saved on: " << file.fileName();
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

    for (const auto &output : outputs) {
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
    for (auto *output : qAsConst(m_outputsControls)) {
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
    for (auto *outputControl : qAsConst(m_outputsControls)) {
        if (getOutputRetention(outputControl->id(), outputControl->name()) == OutputRetention::Individual) {
            continue;
        }
        success &= outputControl->writeFile();
    }
    return success && Control::writeFile();
}

bool ControlConfig::infoIsOutput(const QVariantMap &info, const QString &outputId, const QString &outputName) const
{
    const QString outputIdInfo = info[idString].toString();
    if (outputIdInfo.isEmpty()) {
        return false;
    }
    if (outputId != outputIdInfo) {
        return false;
    }

    if (!outputName.isEmpty() && m_duplicateOutputIds.contains(outputId)) {
        // We may have identical outputs connected, these will have the same id in the config
        // in order to find the right one, also check the output's name (usually the connector)
        const auto metadata = info[metadataString].toMap();
        const auto outputNameInfo = metadata[nameString].toString();
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
        return convertVariantToOutputRetention(info[retentionString]);
    }
    // info for output not found
    return OutputRetention::Undefined;
}

static QVariantMap metadata(const QString &outputName)
{
    QVariantMap metadata;
    metadata[nameString] = outputName;
    return metadata;
}

QVariantMap createOutputInfo(const QString &outputId, const QString &outputName)
{
    QVariantMap outputInfo;
    outputInfo[idString] = outputId;
    outputInfo[metadataString] = metadata(outputName);
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
        outputInfo[retentionString] = (int)value;
        *it = outputInfo;
        setOutputs(outputsInfo);
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[retentionString] = (int)value;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
}

template<typename T, typename F>
T ControlConfig::get(const KScreen::OutputPtr &output, const QString &name, F globalRetentionFunc, T defaultValue) const
{
    const auto &outputId = output->hashMd5();
    const auto &outputName = output->name();
    const auto retention = getOutputRetention(outputId, outputName);
    if (retention == OutputRetention::Individual) {
        const QVariantList outputsInfo = getOutputs();
        for (const auto &variantInfo : outputsInfo) {
            const QVariantMap info = variantInfo.toMap();
            if (!infoIsOutput(info, outputId, outputName)) {
                continue;
            }
            const auto val = info[name];
            if (val.template canConvert<T>()) {
                return val.template value<T>();
            } else {
                return defaultValue;
            }
        }
    }
    // Retention is global or info for output not in config control file.
    if (auto *outputControl = getOutputControl(outputId, outputName)) {
        return (outputControl->*globalRetentionFunc)();
    }

    // Info for output not found.
    return defaultValue;
}

template<typename T, typename F, typename V>
void ControlConfig::set(const KScreen::OutputPtr &output, const QString &name, F globalRetentionFunc, V value)
{
    const auto &outputId = output->hashMd5();
    const auto &outputName = output->name();
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[name] = static_cast<T>(value);
        *it = outputInfo;
        setOutputs(outputsInfo);
        if (auto *control = getOutputControl(outputId, outputName)) {
            (control->*globalRetentionFunc)(value);
        }
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(outputId, outputName);
    outputInfo[name] = static_cast<T>(value);

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
    if (auto *control = getOutputControl(outputId, outputName)) {
        (control->*globalRetentionFunc)(value);
    }
}

qreal ControlConfig::getScale(const KScreen::OutputPtr &output) const
{
    return get(output, scaleString, &ControlOutput::getScale, -1.0);
}

void ControlConfig::setScale(const KScreen::OutputPtr &output, qreal value)
{
    set<qreal>(output, scaleString, &ControlOutput::setScale, value);
}

bool ControlConfig::getAutoRotate(const KScreen::OutputPtr &output) const
{
    return get(output, autorotateString, &ControlOutput::getAutoRotate, true);
}

void ControlConfig::setAutoRotate(const KScreen::OutputPtr &output, bool value)
{
    set<bool>(output, autorotateString, &ControlOutput::setAutoRotate, value);
}

bool ControlConfig::getAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const
{
    return get(output, autorotateTabletOnlyString, &ControlOutput::getAutoRotateOnlyInTabletMode, true);
}

void ControlConfig::setAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output, bool value)
{
    set<bool>(output, autorotateTabletOnlyString, &ControlOutput::setAutoRotateOnlyInTabletMode, value);
}

KScreen::OutputPtr ControlConfig::getReplicationSource(const KScreen::OutputPtr &output) const
{
    const QVariantList outputsInfo = getOutputs();
    for (const auto &variantInfo : outputsInfo) {
        const QVariantMap info = variantInfo.toMap();
        if (!infoIsOutput(info, output->hashMd5(), output->name())) {
            continue;
        }
        const QString sourceHash = info[replicateHashString].toString();
        const QString sourceName = info[replicateNameString].toString();

        if (sourceHash.isEmpty() && sourceName.isEmpty()) {
            // Common case when the replication source has been unset.
            return nullptr;
        }

        const auto outputs = m_config->outputs();
        for (const auto &output : outputs) {
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
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();
    const QString sourceHash = source ? source->hashMd5() : QString();
    const QString sourceName = source ? source->name() : QString();

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, output->hashMd5(), output->name())) {
            continue;
        }
        outputInfo[replicateHashString] = sourceHash;
        outputInfo[replicateNameString] = sourceName;
        *it = outputInfo;
        setOutputs(outputsInfo);
        // TODO: shall we set this information also as new global value (like with auto-rotate)?
        return;
    }
    // no entry yet, create one
    auto outputInfo = createOutputInfo(output->hashMd5(), output->name());
    outputInfo[replicateHashString] = sourceHash;
    outputInfo[replicateNameString] = sourceName;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
    // TODO: shall we set this information also as new global value (like with auto-rotate)?
}

uint32_t ControlConfig::getOverscan(const KScreen::OutputPtr &output) const
{
    return get(output, overscanString, &ControlOutput::overscan, 0);
}

void ControlConfig::setOverscan(const KScreen::OutputPtr &output, const uint32_t value)
{
    set<uint32_t>(output, overscanString, &ControlOutput::setOverscan, value);
}

KScreen::Output::VrrPolicy ControlConfig::getVrrPolicy(const KScreen::OutputPtr &output) const
{
    return get(output, vrrPolicyString, &ControlOutput::vrrPolicy, KScreen::Output::VrrPolicy::Automatic);
}

void ControlConfig::setVrrPolicy(const KScreen::OutputPtr &output, const KScreen::Output::VrrPolicy value)
{
    set<uint32_t>(output, vrrPolicyString, &ControlOutput::setVrrPolicy, value);
}

KScreen::Output::RgbRange ControlConfig::getRgbRange(const KScreen::OutputPtr &output) const
{
    return get(output, rgbRangeString, &ControlOutput::rgbRange, KScreen::Output::RgbRange::Automatic);
}

void ControlConfig::setRgbRange(const KScreen::OutputPtr &output, const KScreen::Output::RgbRange value)
{
    set<uint32_t>(output, rgbRangeString, &ControlOutput::setRgbRange, value);
}

QVariantList ControlConfig::getOutputs() const
{
    return constInfo()[outputsString].toList();
}

void ControlConfig::setOutputs(QVariantList outputsInfo)
{
    auto &infoMap = info();
    infoMap[outputsString] = outputsInfo;
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
    const auto val = constInfo()[scaleString];
    return val.canConvert<qreal>() ? val.toReal() : -1;
}

void ControlOutput::setScale(qreal value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[scaleString] = value;
}

bool ControlOutput::getAutoRotate() const
{
    const auto val = constInfo()[autorotateString];
    return !val.canConvert<bool>() || val.toBool();
}

void ControlOutput::setAutoRotate(bool value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[autorotateString] = value;
}

bool ControlOutput::getAutoRotateOnlyInTabletMode() const
{
    const auto val = constInfo()[autorotateTabletOnlyString];
    return !val.canConvert<bool>() || val.toBool();
}

void ControlOutput::setAutoRotateOnlyInTabletMode(bool value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[autorotateTabletOnlyString] = value;
}

uint32_t ControlOutput::overscan() const
{
    const auto val = constInfo()[overscanString];
    if (val.canConvert<uint>()) {
        return val.toUInt();
    }
    return 0;
}

void ControlOutput::setOverscan(uint32_t value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[overscanString] = static_cast<uint>(value);
}

KScreen::Output::VrrPolicy ControlOutput::vrrPolicy() const
{
    const auto val = constInfo()[vrrPolicyString];
    if (val.canConvert<uint>()) {
        return static_cast<KScreen::Output::VrrPolicy>(val.toUInt());
    }
    return KScreen::Output::VrrPolicy::Automatic;
}

void ControlOutput::setVrrPolicy(KScreen::Output::VrrPolicy value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[vrrPolicyString] = static_cast<uint>(value);
}

KScreen::Output::RgbRange ControlOutput::rgbRange() const
{
    const auto val = constInfo()[rgbRangeString];
    if (val.canConvert<uint>()) {
        return static_cast<KScreen::Output::RgbRange>(val.toUInt());
    }
    return KScreen::Output::RgbRange::Automatic;
}

void ControlOutput::setRgbRange(KScreen::Output::RgbRange value)
{
    auto &infoMap = info();
    if (infoMap.isEmpty()) {
        infoMap = createOutputInfo(m_output->hashMd5(), m_output->name());
    }
    infoMap[rgbRangeString] = static_cast<uint>(value);
}
