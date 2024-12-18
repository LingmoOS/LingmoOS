/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

// KWayland
#include "abstract_data_device.h"
#include "seat_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QVector>

#include <optional>

#include "qwayland-server-wayland.h"

namespace KWaylandServer
{
class AbstractDataSource;
class DataDeviceInterface;
class DataSourceInterface;
class DataControlDeviceV1Interface;
class TextInputV1Interface;
class TextInputV2Interface;
class TextInputV3Interface;
class PrimarySelectionDeviceV1Interface;
class DragAndDropIcon;
class OCEANSecurityInterface;

class VerifyState : public QObject
{
    Q_OBJECT
public:
    VerifyState(QObject *parent) : QObject(parent) {}
    uint32_t serial;
    AbstractDataSource *dataSource;
    AbstractDataDevice *dataDevice;
    SurfaceInterface *dragSurface = nullptr;
    int deviceType = AbstractDataDevice::DeviceType_Unknown;
};

class SeatInterfacePrivate : public QtWaylandServer::wl_seat
{
public:
    // exported for unit tests
    KWIN_EXPORT static SeatInterfacePrivate *get(SeatInterface *seat);
    SeatInterfacePrivate(SeatInterface *q, Display *display);

    void sendCapabilities();
    QVector<DataDeviceInterface *> dataDevicesForSurface(SurfaceInterface *surface) const;
    void registerPrimarySelectionDevice(PrimarySelectionDeviceV1Interface *primarySelectionDevice);
    void registerDataDevice(DataDeviceInterface *dataDevice);
    void registerDataControlDevice(DataControlDeviceV1Interface *dataDevice);
    void endDrag();
    void cancelDrag();

    bool verifySelection(AbstractDataDevice* dataDevice, AbstractDataSource *dataSource);
    uint32_t verifySelectionForX11(AbstractDataSource *source, int target);
    bool verifyDrag(SurfaceInterface *surface, AbstractDataDevice* dataDevice, AbstractDataSource *dataSource);
    void handleCopySecurityVerified(uint32_t serial, uint32_t permission);
    void handleDragSecurityVerified(uint32_t serial, uint32_t permission);
    void addSecurityInterface(OCEANSecurityInterface* security);

    SeatInterface *q;
    QPointer<Display> display;
    QString name;
    std::chrono::milliseconds timestamp = std::chrono::milliseconds::zero();
    quint32 capabilities = 0;
    std::unique_ptr<KeyboardInterface> keyboard;
    std::unique_ptr<PointerInterface> pointer;
    std::unique_ptr<TouchInterface> touch;
    QVector<DataDeviceInterface *> dataDevices;
    QVector<PrimarySelectionDeviceV1Interface *> primarySelectionDevices;
    QVector<DataControlDeviceV1Interface *> dataControlDevices;

    QPointer<TextInputV1Interface> textInputV1;
    // TextInput v2
    QPointer<TextInputV2Interface> textInputV2;
    QPointer<TextInputV3Interface> textInputV3;

    SurfaceInterface *focusedTextInputSurface = nullptr;
    QMetaObject::Connection focusedSurfaceDestroyConnection;

    // the last thing copied into the clipboard content
    AbstractDataSource *currentSelection = nullptr;
    AbstractDataSource *currentPrimarySelection = nullptr;
    AbstractDataSource *currentVerifySelection = nullptr;
    AbstractDataSource *primaryVerifySelection = nullptr;

    OCEANSecurityInterface* oceanSecurity = nullptr;

    QMap<uint32_t, VerifyState*> verifyingState;

    // Add for get the real pid of source
    // clipboard cache the source and clear the orgin pid
    pid_t lastSourcePid = -1;

    // Pointer related members
    struct Pointer
    {
        enum class State {
            Released,
            Pressed,
        };
        QHash<quint32, quint32> buttonSerials;
        QHash<quint32, State> buttonStates;
        QPointF pos;
        struct Focus
        {
            SurfaceInterface *surface = nullptr;
            QMetaObject::Connection destroyConnection;
            QPointF offset = QPointF();
            QMatrix4x4 transformation;
            quint32 serial = 0;
        };
        Focus focus;
    };
    Pointer globalPointer;
    void updatePointerButtonSerial(quint32 button, quint32 serial);
    void updatePointerButtonState(quint32 button, Pointer::State state);

    // Keyboard related members
    struct Keyboard
    {
        struct Focus
        {
            SurfaceInterface *surface = nullptr;
            QMetaObject::Connection destroyConnection;
            quint32 serial = 0;
            QVector<DataDeviceInterface *> selections;
            QVector<PrimarySelectionDeviceV1Interface *> primarySelections;
        };
        Focus focus;
    };
    Keyboard globalKeyboard;

    // Touch related members
    struct Touch
    {
        struct Focus
        {
            SurfaceInterface *surface = nullptr;
            QMetaObject::Connection destroyConnection;
            QPointF offset = QPointF();
            QPointF firstTouchPos;
            QMatrix4x4 transformation;
        };
        Focus focus;
        QMap<qint32, quint32> ids;
    };
    Touch globalTouch;

    struct Drag
    {
        enum class Mode {
            None,
            Pointer,
            Touch,
        };
        Mode mode = Mode::None;
        AbstractDataSource *source = nullptr;
        QPointer<SurfaceInterface> surface;
        SurfaceInterface *pendingSurface = nullptr;
        QPointer<AbstractDropHandler> target;
        QPointer<AbstractDropHandler> pendingTarget;
        QPointer<DragAndDropIcon> dragIcon;
        QMatrix4x4 transformation;
        QMatrix4x4 pendingTransformation;
        std::optional<quint32> dragImplicitGrabSerial;
        QMetaObject::Connection dragSourceDestroyConnection;
    };
    Drag drag;

protected:
    void seat_bind_resource(Resource *resource) override;
    void seat_get_pointer(Resource *resource, uint32_t id) override;
    void seat_get_keyboard(Resource *resource, uint32_t id) override;
    void seat_get_touch(Resource *resource, uint32_t id) override;
    void seat_release(Resource *resource) override;

private:
    void updateSelection(DataDeviceInterface *dataDevice);
    void updatePrimarySelection(PrimarySelectionDeviceV1Interface *primarySelectionDevice);
    bool skipVerify(pid_t pid);
};

} // namespace KWaylandServer
