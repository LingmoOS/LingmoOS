/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "output.h"
#include "edid.h"
#include "kscreen_debug.h"
#include "mode.h"

#include <QCryptographicHash>
#include <QGuiApplication>
#include <QRect>
#include <QScopedPointer>

#include <cstdint>
#include <qobjectdefs.h>
#include <utility>

using namespace KScreen;

class Q_DECL_HIDDEN Output::Private
{
public:
    Private()
        : id(0)
        , type(Unknown)
        , rotation(None)
        , connected(false)
        , enabled(false)
        , priority(0)
        , replicationSource(0)
        , edid(nullptr)
        , scale(1.0)
        , explicitLogicalSize(QSizeF())
    {
    }

    Private(const Private &other)
        : id(other.id)
        , name(other.name)
        , type(other.type)
        , icon(other.icon)
        , pos(other.pos)
        , size(other.size)
        , rotation(other.rotation)
        , currentMode(other.currentMode)
        , preferredMode(other.preferredMode)
        , preferredModes(other.preferredModes)
        , connected(other.connected)
        , enabled(other.enabled)
        , priority(other.priority)
        , clones(other.clones)
        , replicationSource(other.replicationSource)
        , sizeMm(other.sizeMm)
        , scale(other.scale)
        , followPreferredMode(other.followPreferredMode)
        , explicitLogicalSize(other.explicitLogicalSize)
        , capabilities(other.capabilities)
        , overscan(other.overscan)
        , vrrPolicy(other.vrrPolicy)
        , rgbRange(other.rgbRange)
        , highDynamicRange(other.highDynamicRange)
        , sdrBrightness(other.sdrBrightness)
        , wideColorGamut(other.wideColorGamut)
        , autoRotatePolicy(other.autoRotatePolicy)
        , iccProfilePath(other.iccProfilePath)
        , sdrGamutWideness(other.sdrGamutWideness)
        , maxPeakBrightness(other.maxPeakBrightness)
        , maxAverageBrightness(other.maxAverageBrightness)
        , minBrightness(other.minBrightness)
        , maxPeakBrightnessOverride(other.maxPeakBrightnessOverride)
        , maxAverageBrightnessOverride(other.maxAverageBrightnessOverride)
        , minBrightnessOverride(other.minBrightnessOverride)
        , colorProfileSource(other.colorProfileSource)
        , brightness(other.brightness)
    {
        const auto otherModeList = other.modeList;
        for (const ModePtr &otherMode : otherModeList) {
            modeList.insert(otherMode->id(), otherMode->clone());
        }
        if (other.edid) {
            edid.reset(other.edid->clone());
        }
    }

    QString biggestMode(const ModeList &modes) const;
    bool compareModeList(const ModeList &before, const ModeList &after);

    // please keep them consistent with order of Q_PROPERTY declarations
    int id;
    QString name;
    Type type;
    QString icon;
    ModeList modeList;
    QPoint pos;
    QSize size;
    Rotation rotation;
    // next three don't exactly match properties by name, but keep them close to each other anyway
    QString currentMode;
    QString preferredMode;
    QStringList preferredModes;
    //
    bool connected;
    bool enabled;
    uint32_t priority;
    QList<int> clones;
    int replicationSource;
    QScopedPointer<Edid> edid;
    QSize sizeMm;
    qreal scale;
    bool followPreferredMode = false;
    QSizeF explicitLogicalSize;
    Capabilities capabilities;
    uint32_t overscan = 0;
    VrrPolicy vrrPolicy = VrrPolicy::Automatic;
    RgbRange rgbRange = RgbRange::Automatic;
    bool highDynamicRange = false;
    uint32_t sdrBrightness = 200;
    bool wideColorGamut = false;
    AutoRotatePolicy autoRotatePolicy = AutoRotatePolicy::InTabletMode;
    QString iccProfilePath;
    double sdrGamutWideness = 0;
    double maxPeakBrightness = 0;
    double maxAverageBrightness = 0;
    double minBrightness = 0;
    std::optional<double> maxPeakBrightnessOverride;
    std::optional<double> maxAverageBrightnessOverride;
    std::optional<double> minBrightnessOverride;
    ColorProfileSource colorProfileSource = ColorProfileSource::sRGB;
    double brightness = 1.0;
};

bool Output::Private::compareModeList(const ModeList &before, const ModeList &after)
{
    if (before.count() != after.count()) {
        return false;
    }

    for (auto itb = before.constBegin(); itb != before.constEnd(); ++itb) {
        auto ita = after.constFind(itb.key());
        if (ita == after.constEnd()) {
            return false;
        }
        const auto &mb = itb.value();
        const auto &ma = ita.value();
        if (mb->id() != ma->id()) {
            return false;
        }
        if (mb->size() != ma->size()) {
            return false;
        }
        if (!qFuzzyCompare(mb->refreshRate(), ma->refreshRate())) {
            return false;
        }
        if (mb->name() != ma->name()) {
            return false;
        }
    }
    // They're the same
    return true;
}

QString Output::Private::biggestMode(const ModeList &modes) const
{
    int area, total = 0;
    KScreen::ModePtr biggest;
    for (const KScreen::ModePtr &mode : modes) {
        area = mode->size().width() * mode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && mode->refreshRate() < biggest->refreshRate()) {
            continue;
        }
        if (area == total && mode->refreshRate() > biggest->refreshRate()) {
            biggest = mode;
            continue;
        }

        total = area;
        biggest = mode;
    }

    if (!biggest) {
        return QString();
    }

    return biggest->id();
}

Output::Output()
    : QObject(nullptr)
    , d(new Private())
{
}

Output::Output(Output::Private *dd)
    : QObject()
    , d(dd)
{
}

Output::~Output()
{
    delete d;
}

OutputPtr Output::clone() const
{
    return OutputPtr(new Output(new Private(*d)));
}

int Output::id() const
{
    return d->id;
}

void Output::setId(int id)
{
    if (d->id == id) {
        return;
    }
    d->id = id;
    Q_EMIT outputChanged();
}

QString Output::name() const
{
    return d->name;
}

void Output::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }
    d->name = name;
    Q_EMIT outputChanged();
}

// TODO KF6: remove this deprecated method
QString Output::hash() const
{
    if (edid() && edid()->isValid()) {
        return edid()->hash();
    }
    return name();
}

QString Output::hashMd5() const
{
    if (edid() && edid()->isValid()) {
        return edid()->hash();
    }
    const auto hash = QCryptographicHash::hash(name().toLatin1(), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
}

Output::Type Output::type() const
{
    return d->type;
}

void Output::setType(Type type)
{
    if (d->type == type) {
        return;
    }
    d->type = type;
    Q_EMIT outputChanged();
}

QString Output::typeName() const
{
    switch (d->type) {
    case Output::Unknown:
        return QStringLiteral("Unknown");
    case Output::Panel:
        return QStringLiteral("Panel (Laptop)");
    case Output::VGA:
        return QStringLiteral("VGA");
    case Output::DVI:
        return QStringLiteral("DVI");
    case Output::DVII:
        return QStringLiteral("DVI-I");
    case Output::DVIA:
        return QStringLiteral("DVI-A");
    case Output::DVID:
        return QStringLiteral("DVI-D");
    case Output::HDMI:
        return QStringLiteral("HDMI");
    case Output::TV:
        return QStringLiteral("TV");
    case Output::TVComposite:
        return QStringLiteral("TV-Composite");
    case Output::TVSVideo:
        return QStringLiteral("TV-SVideo");
    case Output::TVComponent:
        return QStringLiteral("TV-Component");
    case Output::TVSCART:
        return QStringLiteral("TV-SCART");
    case Output::TVC4:
        return QStringLiteral("TV-C4");
    case Output::DisplayPort:
        return QStringLiteral("DisplayPort");
    };
    return QStringLiteral("Invalid Type") + QString::number(d->type);
}

QString Output::icon() const
{
    return d->icon;
}

void Output::setIcon(const QString &icon)
{
    if (d->icon == icon) {
        return;
    }
    d->icon = icon;
    Q_EMIT outputChanged();
}

ModePtr Output::mode(const QString &id) const
{
    if (!d->modeList.contains(id)) {
        return ModePtr();
    }

    return d->modeList[id];
}

ModeList Output::modes() const
{
    return d->modeList;
}

void Output::setModes(const ModeList &modes)
{
    bool changed = !d->compareModeList(d->modeList, modes);
    d->modeList = modes;
    if (changed) {
        Q_EMIT modesChanged();
        Q_EMIT outputChanged();
    }
}

QString Output::currentModeId() const
{
    return d->currentMode;
}

void Output::setCurrentModeId(const QString &mode)
{
    if (d->currentMode == mode) {
        return;
    }
    d->currentMode = mode;
    Q_EMIT currentModeIdChanged();
}

ModePtr Output::currentMode() const
{
    return d->modeList.value(d->currentMode);
}

void Output::setPreferredModes(const QStringList &modes)
{
    d->preferredMode = QString();
    d->preferredModes = modes;
}

QStringList Output::preferredModes() const
{
    return d->preferredModes;
}

QString Output::preferredModeId() const
{
    if (!d->preferredMode.isEmpty()) {
        return d->preferredMode;
    }
    if (d->preferredModes.isEmpty()) {
        return d->biggestMode(modes());
    }

    int total = 0;
    KScreen::ModePtr biggest;
    KScreen::ModePtr candidateMode;
    for (const QString &modeId : std::as_const(d->preferredModes)) {
        candidateMode = mode(modeId);
        const int area = candidateMode->size().width() * candidateMode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && biggest && candidateMode->refreshRate() < biggest->refreshRate()) {
            continue;
        }
        if (area == total && biggest && candidateMode->refreshRate() > biggest->refreshRate()) {
            biggest = candidateMode;
            continue;
        }

        total = area;
        biggest = candidateMode;
    }

    Q_ASSERT_X(biggest, "preferredModeId", "biggest mode must exist");

    d->preferredMode = biggest->id();
    return d->preferredMode;
}

ModePtr Output::preferredMode() const
{
    return d->modeList.value(preferredModeId());
}

QPoint Output::pos() const
{
    return d->pos;
}

void Output::setPos(const QPoint &pos)
{
    if (d->pos == pos) {
        return;
    }
    d->pos = pos;
    Q_EMIT posChanged();
}

QSize Output::size() const
{
    return d->size;
}

void Output::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }
    d->size = size;
    Q_EMIT sizeChanged();
}

// TODO KF6: make the Rotation enum an enum class and align values with Wayland transformation property
Output::Rotation Output::rotation() const
{
    return d->rotation;
}

void Output::setRotation(Output::Rotation rotation)
{
    if (d->rotation == rotation) {
        return;
    }
    d->rotation = rotation;
    Q_EMIT rotationChanged();
}

qreal Output::scale() const
{
    return d->scale;
}

void Output::setScale(qreal factor)
{
    if (qFuzzyCompare(d->scale, factor)) {
        return;
    }
    d->scale = factor;
    Q_EMIT scaleChanged();
}

QSizeF Output::explicitLogicalSize() const
{
    return d->explicitLogicalSize;
}

QSize Output::explicitLogicalSizeInt() const
{
    const QSizeF sizeF = explicitLogicalSize();
    return QSize(std::ceil(sizeF.width()), std::ceil(sizeF.height()));
}

void Output::setExplicitLogicalSize(const QSizeF &size)
{
    if (qFuzzyCompare(d->explicitLogicalSize.width(), size.width()) && qFuzzyCompare(d->explicitLogicalSize.height(), size.height())) {
        return;
    }
    d->explicitLogicalSize = size;
    Q_EMIT explicitLogicalSizeChanged();
}

bool Output::isConnected() const
{
    return d->connected;
}

void Output::setConnected(bool connected)
{
    if (d->connected == connected) {
        return;
    }
    d->connected = connected;
    Q_EMIT isConnectedChanged();
}

bool Output::isEnabled() const
{
    return d->enabled;
}

void Output::setEnabled(bool enabled)
{
    if (d->enabled == enabled) {
        return;
    }
    d->enabled = enabled;
    Q_EMIT isEnabledChanged();
}

bool Output::isPrimary() const
{
    return d->enabled && (d->priority == 1);
}

void Output::setPrimary(bool primary)
{
    if (primary) {
        setPriority(1);
    } else {
        qCWarning(KSCREEN) << "Calling Output::setPrimary(false) is not supported. Port your code to Config::setPrimaryOutput";
    }
}

uint32_t Output::priority() const
{
    return d->priority;
}

void Output::setPriority(uint32_t priority)
{
    if (d->priority == priority) {
        return;
    }
    d->priority = priority;
    Q_EMIT priorityChanged();
}

QList<int> Output::clones() const
{
    return d->clones;
}

void Output::setClones(const QList<int> &outputlist)
{
    if (d->clones == outputlist) {
        return;
    }
    d->clones = outputlist;
    Q_EMIT clonesChanged();
}

int Output::replicationSource() const
{
    return d->replicationSource;
}

void Output::setReplicationSource(int source)
{
    if (d->replicationSource == source) {
        return;
    }
    d->replicationSource = source;
    Q_EMIT replicationSourceChanged();
}

void Output::setEdid(const QByteArray &rawData)
{
    Q_ASSERT(d->edid.isNull());
    d->edid.reset(new Edid(rawData));
}

Edid *Output::edid() const
{
    return d->edid.data();
}

QSize Output::sizeMm() const
{
    return d->sizeMm;
}

void Output::setSizeMm(const QSize &size)
{
    d->sizeMm = size;
}

bool KScreen::Output::followPreferredMode() const
{
    return d->followPreferredMode;
}

void KScreen::Output::setFollowPreferredMode(bool follow)
{
    if (follow == d->followPreferredMode) {
        return;
    }
    d->followPreferredMode = follow;
    Q_EMIT followPreferredModeChanged(follow);
}

bool Output::isPositionable() const
{
    return isConnected() && isEnabled() && !replicationSource();
}

QSize Output::enforcedModeSize() const
{
    if (const auto mode = currentMode()) {
        return mode->size();
    } else if (const auto mode = preferredMode()) {
        return mode->size();
    } else if (d->modeList.count() > 0) {
        return d->modeList.first()->size();
    }
    return QSize();
}

QRect Output::geometry() const
{
    QSize size = explicitLogicalSizeInt();
    if (!size.isValid()) {
        return QRect();
    }

    return QRect(d->pos, size);
}

Output::Capabilities Output::capabilities() const
{
    return d->capabilities;
}

void Output::setCapabilities(Capabilities capabilities)
{
    if (d->capabilities == capabilities) {
        return;
    }
    d->capabilities = capabilities;
    Q_EMIT capabilitiesChanged();
}

uint32_t Output::overscan() const
{
    return d->overscan;
}

void Output::setOverscan(uint32_t overscan)
{
    if (d->overscan == overscan) {
        return;
    }
    d->overscan = overscan;
    Q_EMIT overscanChanged();
}

Output::VrrPolicy Output::vrrPolicy() const
{
    return d->vrrPolicy;
}

void Output::setVrrPolicy(VrrPolicy policy)
{
    if (d->vrrPolicy == policy) {
        return;
    }
    d->vrrPolicy = policy;
    Q_EMIT vrrPolicyChanged();
}

Output::RgbRange Output::rgbRange() const
{
    return d->rgbRange;
}

void Output::setRgbRange(Output::RgbRange rgbRange)
{
    if (d->rgbRange == rgbRange) {
        return;
    }
    d->rgbRange = rgbRange;
    Q_EMIT rgbRangeChanged();
}

bool Output::isHdrEnabled() const
{
    return d->highDynamicRange;
}

void Output::setHdrEnabled(bool enable)
{
    if (d->highDynamicRange != enable) {
        d->highDynamicRange = enable;
        Q_EMIT hdrEnabledChanged();
    }
}

uint32_t Output::sdrBrightness() const
{
    return d->sdrBrightness;
}

void Output::setSdrBrightness(uint32_t brightness)
{
    if (d->sdrBrightness != brightness) {
        d->sdrBrightness = brightness;
        Q_EMIT sdrBrightnessChanged();
    }
}

bool Output::isWcgEnabled() const
{
    return d->wideColorGamut;
}

void Output::setWcgEnabled(bool enable)
{
    if (d->wideColorGamut != enable) {
        d->wideColorGamut = enable;
        Q_EMIT wcgEnabledChanged();
    }
}

Output::AutoRotatePolicy Output::autoRotatePolicy() const
{
    return d->autoRotatePolicy;
}

void Output::setAutoRotatePolicy(AutoRotatePolicy policy)
{
    if (d->autoRotatePolicy != policy) {
        d->autoRotatePolicy = policy;
        Q_EMIT autoRotatePolicyChanged();
    }
}

QString Output::iccProfilePath() const
{
    return d->iccProfilePath;
}

void Output::setIccProfilePath(const QString &path)
{
    if (d->iccProfilePath != path) {
        d->iccProfilePath = path;
        Q_EMIT iccProfilePathChanged();
    }
}

double Output::sdrGamutWideness() const
{
    return d->sdrGamutWideness;
}

void Output::setSdrGamutWideness(double value)
{
    if (d->sdrGamutWideness != value) {
        d->sdrGamutWideness = value;
        Q_EMIT sdrGamutWidenessChanged();
    }
}

double Output::maxPeakBrightness() const
{
    return d->maxPeakBrightness;
}

void Output::setMaxPeakBrightness(double value)
{
    if (d->maxPeakBrightness != value) {
        d->maxPeakBrightness = value;
        Q_EMIT maxPeakBrightnessChanged();
    }
}

double Output::maxAverageBrightness() const
{
    return d->maxAverageBrightness;
}

void Output::setMaxAverageBrightness(double value)
{
    if (d->maxAverageBrightness != value) {
        d->maxAverageBrightness = value;
        Q_EMIT maxAverageBrightnessChanged();
    }
}

double Output::minBrightness() const
{
    return d->minBrightness;
}

void Output::setMinBrightness(double value)
{
    if (d->minBrightness != value) {
        d->minBrightness = value;
        Q_EMIT minBrightnessChanged();
    }
}

std::optional<double> Output::maxPeakBrightnessOverride() const
{
    return d->maxPeakBrightnessOverride;
}

void Output::setMaxPeakBrightnessOverride(std::optional<double> value)
{
    if (d->maxPeakBrightnessOverride != value) {
        d->maxPeakBrightnessOverride = value;
        Q_EMIT maxPeakBrightnessOverrideChanged();
    }
}

std::optional<double> Output::maxAverageBrightnessOverride() const
{
    return d->maxAverageBrightnessOverride;
}

void Output::setMaxAverageBrightnessOverride(std::optional<double> value)
{
    if (d->maxAverageBrightnessOverride != value) {
        d->maxAverageBrightnessOverride = value;
        Q_EMIT maxAverageBrightnessOverrideChanged();
    }
}

std::optional<double> Output::minBrightnessOverride() const
{
    return d->minBrightnessOverride;
}

void Output::setMinBrightnessOverride(std::optional<double> value)
{
    if (d->minBrightnessOverride != value) {
        d->minBrightnessOverride = value;
        Q_EMIT minBrightnessOverrideChanged();
    }
}

Output::ColorProfileSource Output::colorProfileSource() const
{
    return d->colorProfileSource;
}

void Output::setColorProfileSource(ColorProfileSource source)
{
    if (d->colorProfileSource != source) {
        d->colorProfileSource = source;
        Q_EMIT colorProfileSourceChanged();
    }
}

double Output::brightness() const
{
    return d->brightness;
}

void Output::setBrightness(double brightness)
{
    if (d->brightness != brightness) {
        d->brightness = brightness;
        Q_EMIT brightnessChanged();
    }
}

void Output::apply(const OutputPtr &other)
{
    typedef void (KScreen::Output::*ChangeSignal)();
    QList<ChangeSignal> changes;

    // We block all signals, and emit them only after we have set up everything
    // This is necessary in order to prevent clients from accessing inconsistent
    // outputs from intermediate change signals
    const bool keepBlocked = blockSignals(true);
    if (d->name != other->d->name) {
        changes << &Output::outputChanged;
        setName(other->d->name);
    }
    if (d->type != other->d->type) {
        changes << &Output::outputChanged;
        setType(other->d->type);
    }
    if (d->icon != other->d->icon) {
        changes << &Output::outputChanged;
        setIcon(other->d->icon);
    }
    if (d->pos != other->d->pos) {
        changes << &Output::posChanged;
        setPos(other->pos());
    }
    if (d->rotation != other->d->rotation) {
        changes << &Output::rotationChanged;
        setRotation(other->d->rotation);
    }
    if (!qFuzzyCompare(d->scale, other->d->scale)) {
        changes << &Output::scaleChanged;
        setScale(other->d->scale);
    }
    if (d->currentMode != other->d->currentMode) {
        changes << &Output::currentModeIdChanged;
        setCurrentModeId(other->d->currentMode);
    }
    if (d->connected != other->d->connected) {
        changes << &Output::isConnectedChanged;
        setConnected(other->d->connected);
    }
    if (d->enabled != other->d->enabled) {
        changes << &Output::isEnabledChanged;
        setEnabled(other->d->enabled);
    }
    if (d->priority != other->d->priority) {
        changes << &Output::priorityChanged;
        setPriority(other->d->priority);
    }
    if (d->clones != other->d->clones) {
        changes << &Output::clonesChanged;
        setClones(other->d->clones);
    }
    if (d->replicationSource != other->d->replicationSource) {
        changes << &Output::replicationSourceChanged;
        setReplicationSource(other->d->replicationSource);
    }
    if (!d->compareModeList(d->modeList, other->d->modeList)) {
        changes << &Output::outputChanged;
        changes << &Output::modesChanged;
    }

    setPreferredModes(other->d->preferredModes);
    ModeList modes;
    for (const ModePtr &otherMode : other->modes()) {
        modes.insert(otherMode->id(), otherMode->clone());
    }
    setModes(modes);

    if (d->capabilities != other->d->capabilities) {
        changes << &Output::capabilitiesChanged;
        setCapabilities(other->d->capabilities);
    }
    if (d->vrrPolicy != other->d->vrrPolicy) {
        changes << &Output::vrrPolicyChanged;
        setVrrPolicy(other->d->vrrPolicy);
    }
    if (d->overscan != other->d->overscan) {
        changes << &Output::overscanChanged;
        setOverscan(other->d->overscan);
    }
    if (d->rgbRange != other->d->rgbRange) {
        changes << &Output::rgbRangeChanged;
        setRgbRange(other->d->rgbRange);
    }
    if (d->highDynamicRange != other->d->highDynamicRange) {
        changes << &Output::hdrEnabledChanged;
        setHdrEnabled(other->d->highDynamicRange);
    }
    if (d->sdrBrightness != other->d->sdrBrightness) {
        changes << &Output::sdrBrightnessChanged;
        setSdrBrightness(other->d->sdrBrightness);
    }
    if (d->wideColorGamut != other->d->wideColorGamut) {
        changes << &Output::wcgEnabledChanged;
        setWcgEnabled(other->d->wideColorGamut);
    }
    if (d->autoRotatePolicy != other->d->autoRotatePolicy) {
        changes << &Output::autoRotatePolicyChanged;
        setAutoRotatePolicy(other->d->autoRotatePolicy);
    }
    if (d->iccProfilePath != other->d->iccProfilePath) {
        changes << &Output::iccProfilePathChanged;
        setIccProfilePath(other->d->iccProfilePath);
    }
    if (d->sdrGamutWideness != other->d->sdrGamutWideness) {
        changes << &Output::sdrGamutWidenessChanged;
        setSdrGamutWideness(other->d->sdrGamutWideness);
    }
    if (d->maxPeakBrightness != other->d->maxPeakBrightness) {
        changes << &Output::maxPeakBrightnessChanged;
        setMaxPeakBrightness(other->d->maxPeakBrightness);
    }
    if (d->maxAverageBrightness != other->d->maxAverageBrightness) {
        changes << &Output::maxAverageBrightnessChanged;
        setMaxAverageBrightness(other->d->maxAverageBrightness);
    }
    if (d->minBrightness != other->d->minBrightness) {
        changes << &Output::minBrightnessChanged;
        setMinBrightness(other->d->minBrightness);
    }

    if (d->maxPeakBrightnessOverride != other->d->maxPeakBrightnessOverride) {
        changes << &Output::maxPeakBrightnessOverrideChanged;
        setMaxPeakBrightnessOverride(other->d->maxPeakBrightnessOverride);
    }
    if (d->maxAverageBrightnessOverride != other->d->maxAverageBrightnessOverride) {
        changes << &Output::maxAverageBrightnessOverrideChanged;
        setMaxAverageBrightnessOverride(other->d->maxAverageBrightnessOverride);
    }
    if (d->minBrightnessOverride != other->d->minBrightnessOverride) {
        changes << &Output::minBrightnessOverrideChanged;
        setMinBrightnessOverride(other->d->minBrightnessOverride);
    }
    if (d->colorProfileSource != other->d->colorProfileSource) {
        changes << &Output::colorProfileSourceChanged;
        setColorProfileSource(other->d->colorProfileSource);
    }
    if (d->brightness != other->d->brightness) {
        changes << &Output::brightnessChanged;
        setBrightness(other->d->brightness);
    }

    // Non-notifyable changes
    if (other->d->edid) {
        d->edid.reset(other->d->edid->clone());
    }

    blockSignals(keepBlocked);

    while (!changes.isEmpty()) {
        const ChangeSignal &sig = changes.first();
        Q_EMIT(this->*sig)();
        changes.removeAll(sig);
    }
}

QDebug operator<<(QDebug dbg, const KScreen::OutputPtr &output)
{
    QDebugStateSaver saver(dbg);
    if (!output) {
        dbg << "KScreen::Output(NULL)";
        return dbg;
    }

    // clang-format off
    dbg.nospace()
        << "KScreen::Output("
            << output->id() << ", "
            << output->name() << ", "
            << (output->isConnected() ? "connected " : "disconnected ")
            << (output->isEnabled() ? "enabled" : "disabled")
            << " priority " << output->priority()
            << ", pos: " << output->pos()
            << ", res: " << output->size()
            << ", modeId: " << output->currentModeId()
            << ", scale: " << output->scale()
            << ", clone: " << (output->clones().isEmpty() ? "no" : "yes")
            << ", rotation: " << output->rotation()
            << ", followPreferredMode: " << output->followPreferredMode()
        << ")";
    // clang-format on
    return dbg;
}

#include "moc_output.cpp"
