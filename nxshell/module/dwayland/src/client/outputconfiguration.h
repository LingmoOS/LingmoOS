/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_OUTPUTCONFIGURATION_H
#define KWAYLAND_CLIENT_OUTPUTCONFIGURATION_H

#include <QObject>
#include <QPoint>
#include <QVector>

#include "outputdevice.h"
#include <DWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_outputmanagement;
struct org_kde_kwin_outputconfiguration;

namespace KWayland
{
namespace Client
{
class EventQueue;

/** @class OutputConfiguration
 *
 * OutputConfiguration provides access to changing OutputDevices. The interface is async
 * and atomic. An OutputConfiguration is created through OutputManagement::createConfiguration().
 *
 * The overall mechanism is to get a new OutputConfiguration from the OutputManagement global and
 * apply changes through the OutputConfiguration::set* calls. When all changes are set, the client
 * calls apply, which asks the server to look at the changes and apply them. The server will then
 * signal back whether the changes have been applied successfully (@c applied()) or were rejected
 * or failed to apply (@c failed()).
 *
 * The current settings for outputdevices can be gotten from @c Registry::outputDevices(), these
 * are used in the set* calls to identify the output the setting applies to.
 *
 * These KWayland classes will not apply changes to the OutputDevices, this is the compositor's
 * task. As such, the configuration set through this interface can be seen as a hint what the
 * compositor should set up, but whether or not the compositor does it (based on hardware or
 * rendering policies, for example), is up to the compositor. The mode setting is passed on to
 * the DRM subsystem through the compositor. The compositor also saves this configuration and reads
 * it on startup, this interface is not involved in that process.
 *
 * @c apply() should only be called after changes to all output devices have been made, not after
 * each change. This allows to test the new configuration as a whole, and is a lot faster since
 * hardware changes can be tested in their new combination, they done in parallel.and rolled back
 * as a whole.
 *
 * \verbatim
    // We're just picking the first of our outputdevices
    KWayland::Client::OutputDevice *output = m_clientOutputs.first();

    // Create a new configuration object
    auto config = m_outputManagement.createConfiguration();

    // handle applied and failed signals
    connect(config, &OutputConfiguration::applied, []() {
        qDebug() << "Configuration applied!";
    });
    connect(config, &OutputConfiguration::failed, []() {
        qDebug() << "Configuration failed!";
    });

    // Change settings
    config->setMode(output, m_clientOutputs.first()->modes().last().id);
    config->setTransform(output, OutputDevice::Transform::Normal);
    config->setPosition(output, QPoint(0, 1920));
    config->setScale(output, 2);

    // Now ask the compositor to apply the changes
    config->apply();
    // You may wait for the applied() or failed() signal here
   \endverbatim

 * @see OutputDevice
 * @see OutputManagement
 * @see OutputManagement::createConfiguration()
 * @since 5.5
 */
class KWAYLANDCLIENT_EXPORT OutputConfiguration : public QObject
{
    Q_OBJECT
public:
    ~OutputConfiguration() override;

    /**
     * Setup this OutputConfiguration to manage the @p outputconfiguration.
     * When using OutputManagement::createOutputConfiguration there is no need to call this
     * method.
     * @param outputconfiguration the outputconfiguration object to set up.
     **/
    void setup(org_kde_kwin_outputconfiguration *outputconfiguration);
    /**
     * @returns @c true if managing a org_kde_kwin_outputconfiguration.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_outputconfiguration interface.
     * After the interface has been released the OutputConfiguration instance is no
     * longer valid and can be setup with another org_kde_kwin_outputconfiguration interface.
     **/
    void release();
    /**
     * Destroys the data held by this OutputConfiguration.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_kwin_outputconfiguration interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * OutputConfiguration gets destroyed.
     *
     *
     * @see release
     **/
    void destroy();
    /**
     * Sets the @p queue to use for creating a OutputConfiguration.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a OutputConfiguration
     **/
    EventQueue *eventQueue();

    /**
     * Enable or disable an output. Enabled means it's used by the
     * compositor for rendering, Disabled means, that no wl_output
     * is connected to this, and the device is sitting there unused
     * by this compositor.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDevice this change applies to.
     * @param enable new Enablement state of this output device.
     */
    void setEnabled(OutputDevice *outputdevice, OutputDevice::Enablement enable);

    /**
     * Set the mode of this output, identified by its mode id.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDevice this change applies to.
     * @param modeId the id of the mode.
     */
    void setMode(OutputDevice *outputdevice, const int modeId);
    /**
     * Set brightness for this output, for example rotated or flipped.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDevice this change applies to.
     * @param brightness the brightnessing factor for this output device.
     */
    void setBrightness(OutputDevice *outputdevice, const int brightness);
    /**
     * Set transformation for this output, for example rotated or flipped.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDevice this change applies to.
     * @param scale the scaling factor for this output device.
     */
    void setTransform(OutputDevice *outputdevice, KWayland::Client::OutputDevice::Transform transform);

    /**
     * Position this output in the global space, relative to other outputs.
     * QPoint(0, 0) for top-left. The position is the top-left corner of this output.
     * There may not be gaps between outputs, they have to be positioned adjacent to
     * each other.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDevice this change applies to.
     * @param pos the OutputDevice global position relative to other outputs,
     *
     */
    void setPosition(OutputDevice *outputdevice, const QPoint &pos);

#if KWAYLANDCLIENT_ENABLE_DEPRECATED_SINCE(5, 50)
    /**
     * Scale rendering of this output.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param scale the scaling factor for this output device.
     * @param outputdevice the OutputDevice this change applies to.
     * @deprecated Since 5.50, use setScaleF(OutputDevice *, qreal)
     */
    KWAYLANDCLIENT_DEPRECATED_VERSION(5, 50, "Use OutputConfiguration::setScaleF(OutputDevice *, qreal)")
    void setScale(OutputDevice *outputdevice, qint32 scale);
#endif

    /**
     * Scale rendering of this output.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param scale the scaling factor for this output device.
     * @param outputdevice the OutputDevice this change applies to.
     * @since 5.50
     */
    void setScaleF(OutputDevice *outputdevice, qreal scale);

    /* Set color curves for this output. The respective color curve vector
     * lengths must equal the current ones in the OutputDevice. The codomain
     * of the curves is always the full uint16 value range, such that any vector
     * is accepted as long as it has the right size.
     * The changes done in this call will be recorded in the
     * OutputDevice and only applied after apply() has been called.
     *
     * @param red color curve of red channel.
     * @param green color curve of green channel.
     * @param blue color curve of blue channel.
     * @param outputdevice the OutputDevice this change applies to.
     * @since 5.50
     */
    void setColorCurves(OutputDevice *outputdevice, QVector<quint16> red, QVector<quint16> green, QVector<quint16> blue);

    /**
     * Set overscan in % for this output.
     *
     * @since 5.82
     */
    void setOverscan(OutputDevice *outputdevice, uint32_t overscan);

    /**
     * Set the VRR policy for this output
     *
     * @since 5.82
     */
    void setVrrPolicy(OutputDevice *outputdevice, OutputDevice::VrrPolicy policy);

    /**
     * Ask the compositor to apply the changes.
     * This results in the compositor looking at all outputdevices and if they have
     * pending changes from the set* calls, these changes will be tested with the
     * hardware and applied if possible. The compositor will react to these changes
     * with the applied() or failed() signals. Note that mode setting may take a
     * while, so the interval between calling apply() and receiving the applied()
     * signal may be considerable, depending on the hardware.
     *
     * @see applied()
     * @see failed()
     */
    void apply();

    operator org_kde_kwin_outputconfiguration *();
    operator org_kde_kwin_outputconfiguration *() const;

Q_SIGNALS:
    /**
     * The server has applied all settings successfully. Pending changes in the
     * OutputDevices have been cleared, changed signals from the OutputDevice have
     * been emitted.
     */
    void applied();
    /**
     * The server has failed to apply the settings or rejected them. Pending changes
     * in the * OutputDevices have been cleared. No changes have been applied to the
     * OutputDevices.
     */
    void failed();

private:
    friend class OutputManagement;
    explicit OutputConfiguration(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::OutputConfiguration *)

#endif
