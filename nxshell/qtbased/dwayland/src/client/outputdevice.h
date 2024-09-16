/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_OUTPUTDEVICE_H
#define WAYLAND_OUTPUTDEVICE_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QVector>

#include <DWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_outputdevice;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{
class EventQueue;

/**
 * @short Wrapper for the org_kde_kwin_outputdevice interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_outputdevice interface.
 * Its main purpose is to hold the information about one OutputDevice.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an OutputDevice interface:
 * @code
 * OutputDevice *c = registry->createOutputDevice(name, version);
 * @endcode
 *
 * This creates the OutputDevice and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * OutputDevice *c = new OutputDevice;
 * c->setup(registry->bindOutputDevice(name, version));
 * @endcode
 *
 * The OutputDevice can be used as a drop-in replacement for any org_kde_kwin_outputdevice
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of OutputDevice are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the OutputDevice instance. By emitting changed
 * the OutputDevice indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT OutputDevice : public QObject
{
    Q_OBJECT
public:
    enum class SubPixel {
        Unknown,
        None,
        HorizontalRGB,
        HorizontalBGR,
        VerticalRGB,
        VerticalBGR,
    };
    Q_ENUM(SubPixel)
    enum class Transform {
        Normal,
        Rotated90,
        Rotated180,
        Rotated270,
        Flipped,
        Flipped90,
        Flipped180,
        Flipped270,
    };
    Q_ENUM(Transform)
    enum class Enablement {
        Disabled = 0,
        Enabled = 1,
    };
    Q_ENUM(Enablement)
    struct Mode {
        enum class Flag {
            None = 0,
            Current = 1 << 0,
            Preferred = 1 << 1,
        };
        Q_DECLARE_FLAGS(Flags, Flag)
        /**
         * The size of this Mode in pixel space.
         **/
        QSize size;
        /**
         * The refresh rate in mHz of this Mode.
         **/
        int refreshRate = 0;
        /**
         * The flags of this Mode, that is whether it's the
         * Current and/or Preferred Mode of the OutputDevice.
         **/
        Flags flags = Flag::None;
        /**
         * The OutputDevice to which this Mode belongs.
         **/
        QPointer<OutputDevice> output;
        /**
         * The id of this mode, unique per OutputDevice. This id can be used to call
         * OutputConfiguration->setMode();
         * @see OutputConfiguration::setMode
         **/
        int id;

        bool operator==(const Mode &m) const;
    };
    struct ColorCurves {
        QVector<quint16> red, green, blue;

        bool operator==(const ColorCurves &cc) const;
        bool operator!=(const ColorCurves &cc) const;
    };
    enum class Capability {
        Overscan = 0x1,
        Vrr = 0x2,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)
    enum class VrrPolicy {
        Never = 0,
        Always = 1,
        Automatic = 2
    };

    explicit OutputDevice(QObject *parent = nullptr);
    ~OutputDevice() override;

    /**
     * Setup this Compositor to manage the @p output.
     * When using Registry::createOutputDevice there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_outputdevice *output);

    /**
     * @returns @c true if managing a org_kde_kwin_outputdevice.
     **/
    bool isValid() const;
    operator org_kde_kwin_outputdevice *();
    operator org_kde_kwin_outputdevice *() const;
    org_kde_kwin_outputdevice *output();
    /**
     * Size in millimeters.
     **/
    QSize physicalSize() const;
    /**
     * Position within the global compositor space.
     **/
    QPoint globalPosition() const;
    /**
     * Textual description of the manufacturer.
     **/
    QString manufacturer() const;
    /**
     * Textual description of the model.
     **/
    QString model() const;
    /**
     * Textual representation of serial number.
     */
    QString serialNumber() const;
    /**
     * Textual representation of EISA identifier.
     */
    QString eisaId() const;
    /**
     * Size in the current mode.
     **/
    QSize pixelSize() const;
    /**
     * The geometry of this OutputDevice in pixels.
     * Convenient for QRect(globalPosition(), pixelSize()).
     * @see globalPosition
     * @see pixelSize
     **/
    QRect geometry() const;
    /**
     * Refresh rate in mHz of the current mode.
     **/
    int refreshRate() const;

#if KWAYLANDCLIENT_ENABLE_DEPRECATED_SINCE(5, 50)
    /**
     * Scaling factor of this output.
     *
     * A scale larger than 1 means that the compositor will automatically scale surface buffers
     * by this amount when rendering. This is used for very high resolution displays where
     * applications rendering at the native resolution would be too small to be legible.
     * @deprecated Since 5.50, use scaleF()
     **/
    KWAYLANDCLIENT_DEPRECATED_VERSION(5, 50, "Use OutputDevice::scaleF()")
    int scale() const;
#endif

    /**
     * Scaling factor of this output.
     *
     * A scale larger than 1 means that the compositor will automatically scale surface buffers
     * by this amount when rendering. This is used for very high resolution displays where
     * applications rendering at the native resolution would be too small to be legible.
     * @since 5.50
     **/
    qreal scaleF() const;
    /**
     * Subpixel orientation of this OutputDevice.
     **/
    SubPixel subPixel() const;
    /**
     * Transform that maps framebuffer to OutputDevice.
     *
     * The purpose is mainly to allow clients render accordingly and tell the compositor,
     * so that for fullscreen surfaces, the compositor will still be able to scan out
     * directly from client surfaces.
     **/
    Transform transform() const;
    /**
     * Color curves.
     * @since 5.50
     **/
    ColorCurves colorCurves() const;

    /**
     * @returns The Modes of this OutputDevice.
     **/
    QList<Mode> modes() const;

    KWayland::Client::OutputDevice::Mode currentMode() const;

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    /**
     * @returns The EDID information for this output.
     **/
    QByteArray edid() const;

    /**
     * @returns Whether this output is enabled or not.
     **/
    OutputDevice::Enablement enabled() const;

    /**
     * @returns A unique identifier for this outputdevice, determined by the server.
     **/
    QByteArray uuid() const;

    /**
     * @returns the capabilities of this outputdevice
     * @since 5.82
     **/
    Capabilities capabilities() const;

    /**
     * @returns what overscan value this outputdevice is using
     * @since 5.82
     **/
    uint32_t overscan() const;

    /**
     * @returns the compositors policy regarding vrr on this output
     * @since 5.82
     */
    VrrPolicy vrrPolicy() const;

    /**
     * Destroys the data hold by this OutputDevice.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail.
     *
     * This method is automatically invoked when the Registry which created this
     * Output gets destroyed.
     **/
    void destroy();

Q_SIGNALS:
    /**
     * Emitted when the output is fully initialized.
     **/
    void done();
    /**
     * Emitted whenever at least one of the data changed.
     **/
    void changed();
    /**
     * Emitted whenever the enabled property changes.
     **/
    void enabledChanged(OutputDevice::Enablement enabled);
    /**
     * Emitted whenever the id property changes.
     **/
    void uuidChanged(const QByteArray &uuid);
    /**
     * Emitted whenever a new Mode is added.
     * This normally only happens during the initial promoting of modes.
     * Afterwards only modeChanged should be emitted.
     * @param mode The newly added Mode.
     * @see modeChanged
     **/
    void modeAdded(const KWayland::Client::OutputDevice::Mode &mode);
    /**
     * Emitted whenever a Mode changes.
     * This normally means that the @c Mode::Flag::Current is added or removed.
     * @param mode The changed Mode
     **/
    void modeChanged(const KWayland::Client::OutputDevice::Mode &mode);
    /**
     * Emitted whenever the color curves changed.
     *
     * @since 5.TODO
     **/
    void colorCurvesChanged();
    /**
     * Emitted whenever the capabilities change
     * @param capabilities the new capabilities
     * @since 5.82
     **/
    void capabilitiesChanged(const Capabilities &capabilities);
    /**
     * Emitted whenever the overscan changes
     * @param overscan the new overscan value
     * @since 5.82
     **/
    void overscanChanged(uint32_t overscan);
    /**
     * Emitted whenever the variable refresh rate policy for this output changes
     *
     * @since 5.82
     **/
    void vrrPolicyChanged(VrrPolicy vrrPolicy);
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the OutputDevice got created by
     * Registry::createOutputDevice
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OutputDevice::Mode::Flags)

}
}

Q_DECLARE_METATYPE(KWayland::Client::OutputDevice::SubPixel)
Q_DECLARE_METATYPE(KWayland::Client::OutputDevice::Transform)
Q_DECLARE_METATYPE(KWayland::Client::OutputDevice::Enablement)
Q_DECLARE_METATYPE(KWayland::Client::OutputDevice::Mode)
Q_DECLARE_METATYPE(KWayland::Client::OutputDevice::ColorCurves)

#endif
