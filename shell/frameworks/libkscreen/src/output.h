/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"
#include "types.h"

#include <QDebug>
#include <QMetaType>
#include <QObject>
#include <QPoint>
#include <QSize>
#include <QStringList>
#include <optional>

namespace KScreen
{
class Edid;
class Mode;

class KSCREEN_EXPORT Output : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
public:
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY outputChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY outputChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY outputChanged)
    Q_PROPERTY(ModeList modes READ modes NOTIFY modesChanged)
    Q_PROPERTY(QPoint pos READ pos WRITE setPos NOTIFY posChanged)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(Rotation rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QString currentModeId READ currentModeId WRITE setCurrentModeId NOTIFY currentModeIdChanged)
    Q_PROPERTY(QString preferredModeId READ preferredModeId CONSTANT)
    Q_PROPERTY(bool connected READ isConnected WRITE setConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY isEnabledChanged)
    Q_PROPERTY(bool primary READ isPrimary WRITE setPrimary NOTIFY priorityChanged)
    Q_PROPERTY(uint32_t priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(QList<int> clones READ clones WRITE setClones NOTIFY clonesChanged)
    Q_PROPERTY(int replicationSource READ replicationSource WRITE setReplicationSource NOTIFY replicationSourceChanged)
    Q_PROPERTY(KScreen::Edid *edid READ edid CONSTANT)
    Q_PROPERTY(QSize sizeMm READ sizeMm CONSTANT)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(bool followPreferredMode READ followPreferredMode WRITE setFollowPreferredMode NOTIFY followPreferredModeChanged)
    Q_PROPERTY(QSizeF explicitLogicalSize READ explicitLogicalSize WRITE setExplicitLogicalSize NOTIFY explicitLogicalSizeChanged)
    Q_PROPERTY(Capabilities capabilities READ capabilities NOTIFY capabilitiesChanged)
    Q_PROPERTY(uint32_t overscan READ overscan WRITE setOverscan NOTIFY overscanChanged)
    Q_PROPERTY(VrrPolicy vrrPolicy READ vrrPolicy WRITE setVrrPolicy NOTIFY vrrPolicyChanged)
    Q_PROPERTY(RgbRange rgbRange READ rgbRange WRITE setRgbRange NOTIFY rgbRangeChanged)
    Q_PROPERTY(bool hdrEnabled READ isHdrEnabled WRITE setHdrEnabled NOTIFY hdrEnabledChanged)
    Q_PROPERTY(uint32_t sdrBrightness READ sdrBrightness WRITE setSdrBrightness NOTIFY sdrBrightnessChanged)
    Q_PROPERTY(bool wcgEnabled READ isWcgEnabled WRITE setWcgEnabled NOTIFY wcgEnabledChanged)
    Q_PROPERTY(AutoRotatePolicy autoRotatePolicy READ autoRotatePolicy WRITE setAutoRotatePolicy NOTIFY autoRotatePolicyChanged)
    Q_PROPERTY(QString iccProfilePath READ iccProfilePath WRITE setIccProfilePath NOTIFY iccProfilePathChanged)
    Q_PROPERTY(ColorProfileSource colorProfileSource READ colorProfileSource WRITE setColorProfileSource NOTIFY colorProfileSourceChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)

    enum Type {
        Unknown,
        VGA,
        DVI,
        DVII,
        DVIA,
        DVID,
        HDMI,
        Panel,
        TV,
        TVComposite,
        TVSVideo,
        TVComponent,
        TVSCART,
        TVC4,
        DisplayPort,
    };
    Q_ENUM(Type)

    enum Rotation {
        None = 1 << 0,
        Left = 1 << 1,
        Inverted = 1 << 2,
        Right = 1 << 3,
        Flipped = 1 << 4,
        Flipped90 = 1 << 5,
        Flipped180 = 1 << 6,
        Flipped270 = 1 << 7,
    };
    Q_ENUM(Rotation)

    enum class Capability {
        Overscan = 1 << 0,
        Vrr = 1 << 1,
        RgbRange = 1 << 2,
        HighDynamicRange = 1 << 3,
        WideColorGamut = 1 << 4,
        AutoRotation = 1 << 5,
        IccProfile = 1 << 6,
        BrightnessControl = 1 << 7,
    };
    Q_ENUM(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    enum class VrrPolicy {
        Never = 0,
        Always = 1,
        Automatic = 2,
    };
    Q_ENUM(VrrPolicy)

    enum class RgbRange {
        Automatic = 0,
        Full = 1,
        Limited = 2,
    };
    Q_ENUM(RgbRange)

    enum class AutoRotatePolicy {
        Never = 0,
        InTabletMode = 1,
        Always = 2,
    };
    Q_ENUM(AutoRotatePolicy)

    enum class ColorProfileSource {
        sRGB = 0,
        ICC = 1,
        EDID = 2,
    };
    Q_ENUM(ColorProfileSource)

    explicit Output();
    ~Output() override;

    OutputPtr clone() const;

    int id() const;
    void setId(int id);

    QString name() const;
    void setName(const QString &name);

    /**
     * Returns an identifying hash for this output.
     *
     * The hash is calculated either via the edid hash or if no
     * edid is available by the output name.
     *
     * @return identifying hash of this output
     * @since 5.15
     * @deprecated
     * @see hashMd5
     */
    QString hash() const;

    /**
     * Returns an identifying hex encoded MD5-hash for this output.
     *
     * The hash is calculated either via the edid hash or if no
     * edid is available by the output name, which is hashed as well.
     *
     * @return identifying hash of this output
     * @since 5.17
     */
    QString hashMd5() const;

    Type type() const;
    QString typeName() const;
    void setType(Type type);

    QString icon() const;
    void setIcon(const QString &icon);

    Q_INVOKABLE ModePtr mode(const QString &id) const;
    ModeList modes() const;
    void setModes(const ModeList &modes);

    QString currentModeId() const;
    void setCurrentModeId(const QString &mode);
    Q_INVOKABLE ModePtr currentMode() const;

    void setPreferredModes(const QStringList &modes);
    QStringList preferredModes() const;
    /**
     * Returns the preferred mode with higher resolution and refresh
     */
    Q_INVOKABLE QString preferredModeId() const;
    /**
     * Returns KScreen::Mode associated with preferredModeId()
     */
    Q_INVOKABLE ModePtr preferredMode() const;

    QPoint pos() const;
    void setPos(const QPoint &pos);

    /***
     * Returns actual size being rendered in the output
     *
     * The returned valued is after transformations have been applied to
     * the resolution of the current mode.
     *
     * For example if currentMode is 1280x800 but it is a vertical screen
     * the returned size will be 800x1280.
     *
     * If that same resolution (1280x800) is transformed and scale x2, the
     * value returned will be 2560x1600.
     *
     * This property reflects the currently active output configuration and
     * is not affected by current mode or orientation change made by user
     * until the config is applied.
     *
     * @since 5.4
     */
    QSize size() const;
    void setSize(const QSize &size);

    /**
     * Returns either current mode size or if not available preferred one or if also not
     * available the first one in the ModeList.
     *
     * @return mode size
     */
    QSize enforcedModeSize() const;

    Rotation rotation() const;
    void setRotation(Rotation rotation);
    /**
     * A comfortable function that returns true when output is not rotated
     * or is rotated upside down.
     */
    Q_INVOKABLE inline bool isHorizontal() const
    {
        return rotation() == Output::None || rotation() == Output::Inverted || rotation() == Output::Flipped || rotation() == Output::Flipped180;
    }

    bool isConnected() const;
    void setConnected(bool connected);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isPrimary() const;
    void setPrimary(bool primary);

    uint32_t priority() const;
    void setPriority(uint32_t priority);

    /**
     * @brief Immutable clones because of hardware restrictions
     *
     * Clones are set symmetrically on all outputs. The list contains ids
     * for all other outputs being clones of this output.
     *
     * @return List of output ids being clones of each other.
     */
    QList<int> clones() const;
    /**
     * @brief Set the clones list.
     *
     * When this output is part of a configuration this call is followed by
     * similar calls on other outputs making the lists in all outputs
     * symmetric.
     * @param outputlist
     */
    void setClones(const QList<int> &outputlist);

    /**
     * @brief Provides the source for an ongoing replication
     *
     * If the returned output id is non-null this output is a replica of the
     * returned output. If null is returned the output is no replica of any
     * other output.
     *
     * @return Replication source output id of this output
     */
    int replicationSource() const;
    /**
     * @brief Set the replication source.
     * @param source
     */
    void setReplicationSource(int source);

    void setEdid(const QByteArray &rawData);

    /**
     * edid returns the output's EDID information if available.
     *
     * The output maintains ownership of the returned Edid, so the caller should not delete it.
     * Note that the edid is only valid as long as the output is alive.
     */
    Edid *edid() const;

    /**
     * Returns the physical size of the screen in milimeters.
     *
     * @note Some broken GPUs or monitors return the size in centimeters instead
     * of millimeters. KScreen at the moment is not sanitizing the values.
     */
    QSize sizeMm() const;
    void setSizeMm(const QSize &size);

    /**
     * Returns if the output needs to be taken account for in the overall compositor/screen
     * space and if it should be depicted on its own in a graphical view for repositioning.
     *
     * @return true if the output is positionable in compositor/screen space.
     *
     * @since 5.17
     */
    bool isPositionable() const;

    /**
     * Returns a rectangle containing the currently set output position and
     * size.
     *
     * The geometry also reflects current orientation (i.e. if current mode
     * is 1920x1080 and orientation is @p KScreen::Output::Left, then the
     * size of the returned rectangle will be 1080x1920.
     *
     * This property contains the current settings stored in the particular
     * Output object, so it is updated even when user changes current mode
     * or orientation without applying the whole config/
     */
    QRect geometry() const;

    /**
     * returns the scaling factor to use for this output
     *
     * @since 5.9
     */
    qreal scale() const;

    /**
     * Set the scaling factor for this output.
     *
     * @arg factor Scale factor to use for this output, the backend may or may not
     * be able to deal with non-integer values, in that case, the factor gets rounded.
     *
     * @since 5.9
     */
    void setScale(qreal factor);

    /**
     * The logical size is the output's representation internal to the display server and its
     * overall screen geometry.
     *
     * returns the explicitly set logical size of this output, is an invalid size if not set
     *
     * @since 5.18
     */
    QSizeF explicitLogicalSize() const;

    /**
     * The logical size is the output's representation internal to the display
     * server and its overall screen geometry.
     *
     * returns the logical size of this output, rounded up to the next integer;
     * is an invalid size if not set
     *
     * @since 6.0
     */
    QSize explicitLogicalSizeInt() const;

    /**
     * Specifies explicitly the logical size of this output and by that overrides any other
     * logical size calculation through mode and scale. To enable this instead again call this
     * function with an invalid size as argument.
     *
     * @param size of this output in logical space
     *
     * @since 5.24
     */
    void setExplicitLogicalSize(const QSizeF &size);

    /**
     * @returns whether the mode should be changed to the new preferred mode
     * once it changes
     *
     * @since 5.15
     */
    bool followPreferredMode() const;

    /**
     * Set whether the preferred mode should be followed through @arg follow
     *
     * @since 5.15
     */
    void setFollowPreferredMode(bool follow);

    /**
     * @returns the capabilities of this output
     * @since 5.22
     */
    Capabilities capabilities() const;

    /**
     * sets the capabilities of this output
     * @since 5.22
     */
    void setCapabilities(Capabilities capabilities);

    /**
     * @returns the overscan value of this output in %
     * @since 5.22
     */
    uint32_t overscan() const;

    /**
     * Set the overscan for this output
     * @param overscan the overscan value in %
     * @since 5.22
     */
    void setOverscan(uint32_t overscan);

    /**
     * @returns when variable refresh rate should be used on this output
     *
     * @since 5.22
     */
    VrrPolicy vrrPolicy() const;

    /**
     * Set when variable refresh rate should be used on this output
     *
     * @since 5.22
     */
    void setVrrPolicy(VrrPolicy policy);

    /**
     * @returns which rgb range the output is using
     * @since 5.23
     */
    RgbRange rgbRange() const;

    /**
     * Set which rgb range the output should use
     * @since 5.23
     */
    void setRgbRange(RgbRange rgbRange);

    /**
     * @returns if high dynamic range is enabled
     * @since 6.0
     */
    bool isHdrEnabled() const;

    /**
     * Set if high dynamic range should be enabled
     * @since 6.0
     */
    void setHdrEnabled(bool enable);

    /**
     * @returns the brightness for SDR content while the output is in HDR mode
     * @since 6.0
     */
    uint32_t sdrBrightness() const;

    /**
     * Set the brightness for SDR content while the output is in HDR mode
     * @since 6.0
     */
    void setSdrBrightness(uint32_t brightness);

    /**
     * @returns if the use of wide color gamut is enabled
     * @since 6.0
     */
    bool isWcgEnabled() const;

    /**
     * Set if a wide color gamut should be used
     * @since 6.0
     */
    void setWcgEnabled(bool enable);

    /**
     * @returns the policy for auto rotation
     * @since 6.0
     */
    AutoRotatePolicy autoRotatePolicy() const;

    /**
     * Set the policy for auto rotation
     * @since 6.0
     */
    void setAutoRotatePolicy(AutoRotatePolicy policy);

    /**
     * @since 6.0
     */
    QString iccProfilePath() const;
    /**
     * @since 6.0
     */
    void setIccProfilePath(const QString &path);

    double sdrGamutWideness() const;
    void setSdrGamutWideness(double value);

    double maxPeakBrightness() const;
    void setMaxPeakBrightness(double value);

    double maxAverageBrightness() const;
    void setMaxAverageBrightness(double value);

    double minBrightness() const;
    void setMinBrightness(double value);

    std::optional<double> maxPeakBrightnessOverride() const;
    void setMaxPeakBrightnessOverride(std::optional<double> value);

    std::optional<double> maxAverageBrightnessOverride() const;
    void setMaxAverageBrightnessOverride(std::optional<double> value);

    std::optional<double> minBrightnessOverride() const;
    void setMinBrightnessOverride(std::optional<double> value);

    ColorProfileSource colorProfileSource() const;
    void setColorProfileSource(ColorProfileSource source);

    double brightness() const;
    void setBrightness(double brightness);

    void apply(const OutputPtr &other);

Q_SIGNALS:
    void outputChanged();
    void posChanged();
    void sizeChanged();
    void currentModeIdChanged();
    void rotationChanged();
    void isConnectedChanged();
    void isEnabledChanged();
    void priorityChanged();
    void clonesChanged();
    void replicationSourceChanged();
    void scaleChanged();
    void explicitLogicalSizeChanged();
    void followPreferredModeChanged(bool followPreferredMode);
    void capabilitiesChanged();
    void overscanChanged();
    void vrrPolicyChanged();
    void rgbRangeChanged();
    void hdrEnabledChanged();
    void sdrBrightnessChanged();
    void wcgEnabledChanged();
    void autoRotatePolicyChanged();
    void iccProfilePathChanged();
    void sdrGamutWidenessChanged();
    void maxPeakBrightnessChanged();
    void maxAverageBrightnessChanged();
    void minBrightnessChanged();
    void maxPeakBrightnessOverrideChanged();
    void maxAverageBrightnessOverrideChanged();
    void minBrightnessOverrideChanged();
    void colorProfileSourceChanged();
    void brightnessChanged();

    /** The mode list changed.
     *
     * This may happen when a mode is added or changed.
     *
     * @since 5.8.3
     */
    void modesChanged();

private:
    Q_DISABLE_COPY(Output)

    class Private;
    Private *const d;

    explicit Output(Private *dd);
};

} // KScreen namespace

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::OutputPtr &output);
