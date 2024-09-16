/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_OUTPUTCONFIGURATION_V2_H
#define KWAYLAND_CLIENT_OUTPUTCONFIGURATION_V2_H

#include <QObject>
#include <QPoint>
#include <QVector>

#include "outputdevice_v2.h"
#include <DWayland/Client/kwaylandclient_export.h>

struct kde_output_management_v2;
struct kde_output_configuration_v2;

namespace KWayland
{
namespace Client
{
class EventQueue;

/** @class OutputConfigurationV2
 *
 * OutputConfigurationV2 provides access to changing OutputDevices. The interface is async
 * and atomic. An OutputConfigurationV2 is created through OutputConfigurationV2::createConfiguration().
 *
 * The overall mechanism is to get a new OutputConfigurationV2 from the OutputConfigurationV2 global and
 * apply changes through the OutputConfigurationV2::set* calls. When all changes are set, the client
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
    KWayland::Client::OutputDeviceV2 *output = m_clientOutputs.first();

    // Create a new configuration object
    auto config = m_outputManagement.createConfiguration();

    // handle applied and failed signals
    connect(config, &OutputConfigurationV2::applied, []() {
        qDebug() << "Configuration applied!";
    });
    connect(config, &OutputConfigurationV2::failed, []() {
        qDebug() << "Configuration failed!";
    });

    // Change settings
    config->setMode(output, m_clientOutputs.first()->modes().last().id);
    config->setTransform(output, OutputDeviceV2::Transform::Normal);
    config->setPosition(output, QPoint(0, 1920));
    config->setScale(output, 2);

    // Now ask the compositor to apply the changes
    config->apply();
    // You may wait for the applied() or failed() signal here
   \endverbatim

 * @see OutputDeviceV2
 * @see OutputConfigurationV2
 * @see OutputConfigurationV2::createConfiguration()
 * @since 5.5
 */
class KWAYLANDCLIENT_EXPORT OutputConfigurationV2 : public QObject
{
    Q_OBJECT
public:
    ~OutputConfigurationV2() override;

    /**
     * Setup this OutputConfigurationV2 to manage the @p outputconfiguration.
     * When using OutputConfigurationV2::createOutputConfiguration there is no need to call this
     * method.
     * @param outputconfiguration the outputconfiguration object to set up.
     **/
    void setup(kde_output_configuration_v2 *outputconfiguration);
    /**
     * @returns @c true if managing a kde_output_configuration_v2.
     **/
    bool isValid() const;
    /**
     * Releases the kde_output_configuration_v2 interface.
     * After the interface has been released the OutputConfigurationV2 instance is no
     * longer valid and can be setup with another kde_output_configuration_v2 interface.
     **/
    void release();
    /**
     * Destroys the data held by this OutputConfigurationV2.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new kde_output_configuration_v2 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * OutputConfigurationV2 gets destroyed.
     *
     *
     * @see release
     **/
    void destroy();
    /**
     * Sets the @p queue to use for creating a OutputConfigurationV2.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a OutputConfigurationV2
     **/
    EventQueue *eventQueue();

    /**
     * Enable or disable an output. Enabled means it's used by the
     * compositor for rendering, Disabled means, that no wl_output
     * is connected to this, and the device is sitting there unused
     * by this compositor.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @param enable new Enablement state of this output device.
     */
    void setEnabled(OutputDeviceV2 *outputdevice, OutputDeviceV2::Enablement enable);

    /**
     * Set the mode of this output, identified by its mode id.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @param modeId the id of the mode.
     */
    void setMode(OutputDeviceV2 *outputdevice, const int modeId);
    /**
     * Set brightness for this output, for example rotated or flipped.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @param brightness the brightnessing factor for this output device.
     */
    void setBrightness(OutputDeviceV2 *outputdevice, const int brightness);
    /**
     * Set transformation for this output, for example rotated or flipped.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @param scale the scaling factor for this output device.
     */
    void setTransform(OutputDeviceV2 *outputdevice, KWayland::Client::OutputDeviceV2::Transform transform);

    /**
     * Position this output in the global space, relative to other outputs.
     * QPoint(0, 0) for top-left. The position is the top-left corner of this output.
     * There may not be gaps between outputs, they have to be positioned adjacent to
     * each other.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @param pos the OutputDeviceV2 global position relative to other outputs,
     *
     */
    void setPosition(OutputDeviceV2 *outputdevice, const QPoint &pos);

#if KWAYLANDCLIENT_ENABLE_DEPRECATED_SINCE(5, 50)
    /**
     * Scale rendering of this output.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param scale the scaling factor for this output device.
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @deprecated Since 5.50, use setScaleF(OutputDeviceV2 *, qreal)
     */
    KWAYLANDCLIENT_DEPRECATED_VERSION(5, 50, "Use OutputConfigurationV2::setScaleF(OutputDeviceV2 *, qreal)")
    void setScale(OutputDeviceV2 *outputdevice, qint32 scale);
#endif

    /**
     * Scale rendering of this output.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param scale the scaling factor for this output device.
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @since 5.50
     */
    void setScaleF(OutputDeviceV2 *outputdevice, qreal scale);

    /* Set color curves for this output. The respective color curve vector
     * lengths must equal the current ones in the OutputDeviceV2. The codomain
     * of the curves is always the full uint16 value range, such that any vector
     * is accepted as long as it has the right size.
     * The changes done in this call will be recorded in the
     * OutputDeviceV2 and only applied after apply() has been called.
     *
     * @param red color curve of red channel.
     * @param green color curve of green channel.
     * @param blue color curve of blue channel.
     * @param outputdevice the OutputDeviceV2 this change applies to.
     * @since 5.50
     */
    void setColorCurves(OutputDeviceV2 *outputdevice, QVector<quint16> red, QVector<quint16> green, QVector<quint16> blue);

    /**
     * Set overscan in % for this output.
     *
     * @since 5.82
     */
    void setOverscan(OutputDeviceV2 *outputdevice, uint32_t overscan);

    /**
     * Set the VRR policy for this output
     *
     * @since 5.82
     */
    void setVrrPolicy(OutputDeviceV2 *outputdevice, OutputDeviceV2::VrrPolicy policy);

    void setRgbRange(OutputDeviceV2 *outputdevice, uint32_t rgbRange);

    void setPrimaryOutput(OutputDeviceV2 *outputdevice);

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

    operator kde_output_configuration_v2 *();
    operator kde_output_configuration_v2 *() const;

Q_SIGNALS:
    /**
     * The server has applied all settings successfully. Pending changes in the
     * OutputDevices have been cleared, changed signals from the OutputDeviceV2 have
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
    friend class OutputManagementV2;
    explicit OutputConfigurationV2(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::OutputConfigurationV2 *)

#endif
