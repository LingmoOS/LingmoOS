// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "xcbnativeeventfilter.h"
#include "utility.h"
#include "dplatformwindowhelper.h"
#include "dframewindow.h"

#define private public
#include "qxcbconnection.h"
#include "qxcbclipboard.h"
#undef private

#include "dplatformintegration.h"
#include "dxcbwmsupport.h"
#include "dxcbxsettings.h"

#include <xcb/xfixes.h>
#include <xcb/damage.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput2.h>

#include <cmath>
#include <QApplication>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#define CONNECTION DPlatformIntegration::instance()->connection()
#else
#define CONNECTION DPlatformIntegration::instance()->defaultConnection()
#endif
DPP_BEGIN_NAMESPACE

XcbNativeEventFilter::XcbNativeEventFilter(QXcbConnection *connection)
    : m_connection(connection)
    , lastXIEventDeviceInfo(0, XIDeviceInfos())
{
    // init damage first event value
    xcb_prefetch_extension_data(connection->xcb_connection(), &xcb_damage_id);
    const auto* reply = xcb_get_extension_data(connection->xcb_connection(), &xcb_damage_id);

    if (reply->present) {
      m_damageFirstEvent = reply->first_event;
      xcb_damage_query_version_unchecked(connection->xcb_connection(), XCB_DAMAGE_MAJOR_VERSION, XCB_DAMAGE_MINOR_VERSION);
    } else {
        m_damageFirstEvent = 0;
    }

    updateXIDeviceInfoMap();
}

QClipboard::Mode XcbNativeEventFilter::clipboardModeForAtom(xcb_atom_t a) const
{
    if (a == XCB_ATOM_PRIMARY)
        return QClipboard::Selection;
    if (a == m_connection->atom(QXcbAtom::D_QXCBATOM_WRAPPER(CLIPBOARD)))
        return QClipboard::Clipboard;
    // not supported enum value, used to detect errors
    return QClipboard::FindBuffer;
}

typedef struct qt_xcb_ge_event_t {
    uint8_t  response_type;
    uint8_t  extension;
    uint16_t sequence;
    uint32_t length;
    uint16_t event_type;
} qt_xcb_ge_event_t;

static inline bool isXIEvent(xcb_generic_event_t *event, int opCode)
{
    qt_xcb_ge_event_t *e = (qt_xcb_ge_event_t *)event;
    return e->extension == opCode;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool XcbNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool XcbNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    uint response_type = event->response_type & ~0x80;

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    if (Q_UNLIKELY(response_type == m_connection->xfixes_first_event + XCB_XFIXES_SELECTION_NOTIFY)) {
#else
    // cannot use isXFixesType because symbols from QXcbBasicConnection are not exported
    if (response_type == m_connection->m_xfixesFirstEvent + XCB_XFIXES_SELECTION_NOTIFY) {
#endif
        xcb_xfixes_selection_notify_event_t *xsn = (xcb_xfixes_selection_notify_event_t *)event;

        if (xsn->selection == DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_CM_S0))) {
            DXcbWMSupport::instance()->updateHasComposite();
        }

        QClipboard::Mode mode = clipboardModeForAtom(xsn->selection);
        if (mode > QClipboard::Selection)
            return false;

        // here we care only about the xfixes events that come from non Qt processes
        if (xsn->owner == XCB_NONE && xsn->subtype == XCB_XFIXES_SELECTION_EVENT_SET_SELECTION_OWNER) {
            QXcbClipboard *xcbClipboard = m_connection->m_clipboard;
            xcbClipboard->emitChanged(mode);
        }
    } else if (Q_UNLIKELY(response_type == m_damageFirstEvent + XCB_DAMAGE_NOTIFY)) {
        xcb_damage_notify_event_t *ev = (xcb_damage_notify_event_t*)event;

        QXcbWindow *window = m_connection->platformWindowFromId(ev->drawable);

        if (Q_LIKELY(window)) {
            DPlatformWindowHelper *helper = DPlatformWindowHelper::mapped.value(window);

            if (Q_LIKELY(helper)) {
                helper->m_frameWindow->updateFromContents(ev);
            }
        }
    } else {
        switch (response_type) {
        case XCB_PROPERTY_NOTIFY: {
            xcb_property_notify_event_t *pn = (xcb_property_notify_event_t *)event;

            DXcbXSettings::handlePropertyNotifyEvent(pn);

            if (pn->atom == DPlatformIntegration::xcbConnection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_MOTIF_WM_HINTS))) {
                emit DXcbWMSupport::instance()->windowMotifWMHintsChanged(pn->window);
            } else if (pn->atom == DXcbWMSupport::instance()->_deepin_wallpaper_shared_key) {
                    DXcbWMSupport::instance()->wallpaperSharedChanged();
            } else {
                if (pn->window != CONNECTION->rootWindow()) {
                    return false;
                }

                if (pn->atom == CONNECTION->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_SUPPORTED))) {
                    DXcbWMSupport::instance()->updateNetWMAtoms();
                } else if (pn->atom == CONNECTION->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_SUPPORTING_WM_CHECK))) {
                    DXcbWMSupport::instance()->updateWMName();
                } else if (pn->atom == DXcbWMSupport::instance()->_kde_net_wm_blur_rehind_region_atom) {
                    DXcbWMSupport::instance()->updateRootWindowProperties();
                } else if (pn->atom == Utility::internAtom("_NET_CLIENT_LIST_STACKING")) {
                    emit DXcbWMSupport::instance()->windowListChanged();
                } else if (pn->atom == Utility::internAtom("_NET_KDE_COMPOSITE_TOGGLING")) {
                    DXcbWMSupport::instance()->updateWMName();
                }
            }
            break;
        }
            // 修复Qt程序对触摸板的自然滚动开关后不能实时生效
            // 由于在收到xi的DeviceChanged事件后，Qt更新ScrollingDevice时没有更新verticalIncrement字段
            // 导致那些使用increment的正负值控制自然滚动开关的设备对Qt程序无法实时生效
            // 有些电脑上触摸板没有此问题，是因为他的系统环境中没有安装xserver-xorg-input-synaptics
#ifdef XCB_USE_XINPUT21
        case XCB_GE_GENERIC: {
            QXcbConnection *xcb_connect = DPlatformIntegration::xcbConnection();

            if (Q_LIKELY(xcb_connect->m_xi2Enabled) && isXIEvent(event, xcb_connect->m_xiOpCode)) {
                xXIGenericDeviceEvent *xiEvent = reinterpret_cast<xXIGenericDeviceEvent *>(event);

                {
                    xXIDeviceEvent *xiDEvent = reinterpret_cast<xXIDeviceEvent*>(event);
                    // NOTE(zccrs): 获取设备编号，至于为何会偏移4个字节，参考：
                    // void QXcbConnection::xi2PrepareXIGenericDeviceEvent(xcb_ge_event_t *event)
                    // xcb event structs contain stuff that wasn't on the wire, the full_sequence field
                    // adds an extra 4 bytes and generic events cookie data is on the wire right after the standard 32 bytes.
                    // Move this data back to have the same layout in memory as it was on the wire
                    // and allow casting, overwriting the full_sequence field.
                    uint16_t source_id = *(&xiDEvent->sourceid + 2);

                    auto device = xiDeviceInfoMap.find(source_id);

                    // find device
                    if (Q_LIKELY(device != xiDeviceInfoMap.constEnd())) {
                        lastXIEventDeviceInfo = qMakePair(xiDEvent->time, device.value());
                    }
                }

                if (Q_LIKELY(xiEvent->evtype != XI_DeviceChanged)) {
                    if (Q_UNLIKELY(xiEvent->evtype == XI_HierarchyChanged)) {
                        xXIHierarchyEvent *xiEvent = reinterpret_cast<xXIHierarchyEvent *>(event);
                        // We only care about hotplugged devices
                        if (!(xiEvent->flags & (XISlaveRemoved | XISlaveAdded)))
                            return false;

                        updateXIDeviceInfoMap();
                    }

                    return false;
                }

// XIQueryDevice 在某些情况(程序放置一夜)会有一定的概率卡住，因此先禁用此逻辑，但会导致鼠标自然滚动设置无法实时生效。目前猜测可能和窗口抓取鼠标/键盘有关，被抓取设备自动休眠后再唤醒时可能会触发此问题.
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0) && false
                xXIDeviceChangedEvent *xiDCEvent = reinterpret_cast<xXIDeviceChangedEvent *>(xiEvent);
                QHash<int, QXcbConnection::ScrollingDevice>::iterator device = xcb_connect->m_scrollingDevices.find(xiDCEvent->sourceid);

                int nrDevices = 0;
                XIDeviceInfo* xiDeviceInfo = XIQueryDevice(static_cast<Display *>(xcb_connect->xlib_display()), xiDCEvent->sourceid, &nrDevices);

                if (nrDevices <= 0) {
                    return false;
                }

                for (int c = 0; c < xiDeviceInfo->num_classes; ++c) {
                    if (xiDeviceInfo->classes[c]->type == XIScrollClass) {
                        XIScrollClassInfo *sci = reinterpret_cast<XIScrollClassInfo *>(xiDeviceInfo->classes[c]);

                        if (sci->scroll_type == XIScrollTypeVertical) {
//                            device->legacyOrientations = device->orientations;
//                            device->orientations |= Qt::Vertical;
//                            device->verticalIndex = sci->number;
                            device->verticalIncrement = std::signbit(sci->increment)
                                    ? -std::abs(device->verticalIncrement)
                                    : std::abs(device->verticalIncrement);
                        }
                        else if (sci->scroll_type == XIScrollTypeHorizontal) {
//                            device->legacyOrientations = device->orientations;
//                            device->orientations |= Qt::Horizontal;
//                            device->horizontalIndex = sci->number;
                            device->horizontalIncrement = std::signbit(sci->increment)
                                    ? -std::abs(device->horizontalIncrement)
                                    : std::abs(device->horizontalIncrement);
                        }
                    }
                }

                XIFreeDeviceInfo(xiDeviceInfo);
#endif
            }
            break;
        }
#endif
        case XCB_CLIENT_MESSAGE: {
            xcb_client_message_event_t *ev = reinterpret_cast<xcb_client_message_event_t*>(event);

            if (Q_UNLIKELY(DXcbXSettings::handleClientMessageEvent(ev))) {
                return true;
            }
            break;
        }
        default:
            // 过时的缩放方案, 在引入 DHighDpi 后已不再使用, 此处仅作为兼容性保障支持
            static const auto updateScaleLogcailDpi = qApp->property("_d_updateScaleLogcailDpi").toULongLong();
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
            if (Q_UNLIKELY(updateScaleLogcailDpi) && DPlatformIntegration::xcbConnection()->has_randr_extension
                    && response_type == DPlatformIntegration::xcbConnection()->xrandr_first_event + XCB_RANDR_NOTIFY) {
#else
            // cannot use isXRandrType because symbols from QXcbBasicConnection are not exported
            if (Q_UNLIKELY(updateScaleLogcailDpi) && DPlatformIntegration::xcbConnection()->hasXRender()
                    && response_type == DPlatformIntegration::xcbConnection()->m_xrandrFirstEvent + XCB_RANDR_NOTIFY) {
#endif
                xcb_randr_notify_event_t *e = reinterpret_cast<xcb_randr_notify_event_t *>(event);
                xcb_randr_output_change_t output = e->u.oc;

                if (e->subCode == XCB_RANDR_NOTIFY_OUTPUT_CHANGE) {
                    QXcbScreen *screen = DPlatformIntegration::xcbConnection()->findScreenForOutput(output.window, output.output);

                    if (!screen && output.connection == XCB_RANDR_CONNECTION_CONNECTED
                            && output.crtc != XCB_NONE && output.mode != XCB_NONE) {
                        DPlatformIntegration::xcbConnection()->updateScreens(e);
                        // 通知deepin platform插件重设缩放后的dpi值
                        reinterpret_cast<void(*)()>(updateScaleLogcailDpi)();

                        return true;
                    }
                }
            }
            break;
        }
    }

    return false;
}

DeviceType XcbNativeEventFilter::xiEventSource(const QInputEvent *event) const
{
    if (lastXIEventDeviceInfo.first == event->timestamp())
        return lastXIEventDeviceInfo.second.type;

    return UnknowDevice;
}

void XcbNativeEventFilter::updateXIDeviceInfoMap()
{
    xiDeviceInfoMap.clear();

    QXcbConnection *xcb_connect = DPlatformIntegration::xcbConnection();

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    Display *xDisplay = static_cast<Display *>(xcb_connect->m_xlib_display);
#else
    Display *xDisplay = reinterpret_cast<Display *>(xcb_connect->xlib_display());
#endif
    int deviceCount = 0;
    XIDeviceInfo *devices = XIQueryDevice(xDisplay, XIAllDevices, &deviceCount);

    for (int i = 0; i < deviceCount; ++i) {
        // Only non-master pointing devices are relevant here.
        if (devices[i].use != XISlavePointer)
            continue;

        int nprops;
        Atom *props = XIListProperties(xDisplay, devices[i].deviceid, &nprops);

        if (!nprops)
            return;

        char *name;

        for (int j = 0; j < nprops; ++j) {
            name = XGetAtomName(xDisplay, props[j]);

            if (QByteArrayLiteral("Synaptics Off") == name
                    || QByteArrayLiteral("libinput Tapping Enabled") == name) {
                xiDeviceInfoMap[devices[i].deviceid] = XIDeviceInfos(TouchapdDevice);
            } else if (QByteArrayLiteral("Button Labels") == name
                       || QByteArrayLiteral("libinput Button Scrolling Button") == name) {
                xiDeviceInfoMap[devices[i].deviceid] = XIDeviceInfos(MouseDevice);
            }

            XFree(name);
        }

        XFree(props);
    }

    // XIQueryDevice may return NULL..boom
    if (devices)
        XIFreeDeviceInfo(devices);
}

DPP_END_NAMESPACE
