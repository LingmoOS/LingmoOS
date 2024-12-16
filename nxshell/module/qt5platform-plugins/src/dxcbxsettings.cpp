// SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LGPL-3.0-only

#include "dxcbxsettings.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#if __has_include("dplatformintegration.h")
#define __D_HAS_DPLATFORMINTEGRATION__
#endif
#else
#if QT_HAS_INCLUDE("dplatformintegration.h")
#define __D_HAS_DPLATFORMINTEGRATION__
#endif
#endif

#ifdef __D_HAS_DPLATFORMINTEGRATION__
#include "dplatformintegration.h"
#include "qxcbconnection.h"
#define IN_DXCB_PLUGIN
#endif

#include <QtCore/QByteArray>
#include <QtCore/QtEndian>
#include <QVariant>
#include <QSet>
#include <QColor>

#include <vector>
#include <algorithm>
#include <memory>

static xcb_atom_t internAtom(xcb_connection_t *conn, const char *name)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, false, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

static QByteArray atomName(xcb_connection_t *conn, xcb_atom_t atom)
{
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(conn, atom);
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(conn, cookie, nullptr);

    if (!reply)
        return nullptr;

    return QByteArray(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
}

#define Q_XCB_REPLY_CONNECTION_ARG(connection, ...) connection

struct Q_DECL_HIDDEN DStdFreeDeleter {
    void operator()(void *p) const noexcept { return std::free(p); }
};

#ifndef Q_XCB_REPLY
#define Q_XCB_REPLY(call, ...) \
    std::unique_ptr<call##_reply_t, DStdFreeDeleter>( \
        call##_reply(Q_XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr) \
    )
#endif

#ifndef Q_XCB_REPLY_UNCHECKED
#define Q_XCB_REPLY_UNCHECKED(call, ...) \
    std::unique_ptr<call##_reply_t, DStdFreeDeleter>( \
        call##_reply(Q_XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call##_unchecked(__VA_ARGS__), nullptr) \
    )
#endif

DPP_BEGIN_NAMESPACE
/* Implementation of http://standards.freedesktop.org/xsettings-spec/xsettings-0.5.html */

enum XSettingsType {
    XSettingsTypeInteger = 0,
    XSettingsTypeString = 1,
    XSettingsTypeColor = 2
};

struct Q_DECL_HIDDEN DXcbXSettingsCallback
{
    DXcbXSettings::PropertyChangeFunc func;
    void *handle;
};

struct Q_DECL_HIDDEN DXcbXSettingsSignalCallback
{
    DXcbXSettings::SignalFunc func;
    void *handle;
};

class Q_DECL_HIDDEN DXcbXSettingsPropertyValue
{
public:
    DXcbXSettingsPropertyValue()
        : last_change_serial(-1)
    {}

    bool updateValue(xcb_connection_t *connection, const QByteArray &name, const QVariant &value, int last_change_serial)
    {
        if (last_change_serial <= this->last_change_serial)
            return false;
        this->value = value;
        this->last_change_serial = last_change_serial;
        for (const auto &callback : callback_links)
            callback.func(connection, name, value, callback.handle);

        return true;
    }

    void addCallback(DXcbXSettings::PropertyChangeFunc func, void *handle)
    {
        DXcbXSettingsCallback callback = { func, handle };
        callback_links.push_back(callback);
    }

    QVariant value;
    int last_change_serial = -1;
    std::vector<DXcbXSettingsCallback> callback_links;
};

class Q_DECL_HIDDEN DXcbConnectionGrabber
{
public:
    DXcbConnectionGrabber(xcb_connection_t *connection);
    ~DXcbConnectionGrabber();
    void release();
private:
    xcb_connection_t *m_connection;
};

DXcbConnectionGrabber::DXcbConnectionGrabber(xcb_connection_t *connection)
    :m_connection(connection)
{
    xcb_grab_server(m_connection);
}

DXcbConnectionGrabber::~DXcbConnectionGrabber()
{
    release();
}

void DXcbConnectionGrabber::release()
{
    if (m_connection) {
        xcb_ungrab_server(m_connection);
        // 必须保证xserver立即处理此请求, 因为xcb是异步请求的
        // 当前线程中可能还存在其它的xcb connection，如果在
        // xcb_ungrab_server调用之后，且server收到请求之前
        // 其它的connection请求并wait了xcb，将会导致当前进程阻塞
        // 此xcb_ungrab_server的调用就无法到达xserver，于是就形成了死锁。
        xcb_flush(m_connection);
        m_connection = 0;
    }
}

class Q_DECL_HIDDEN DXcbXSettingsPrivate
{
public:
    DXcbXSettingsPrivate(xcb_connection_t *connection, const QByteArray &property, DXcbXSettings *qq)
        : connection(connection)
        , initialized(false)
        , q_ptr(qq)
    {
        if (property.isEmpty()) {
            x_settings_atom = internAtom(connection, "_XSETTINGS_SETTINGS");
        } else {
            x_settings_atom = internAtom(connection, property);
        }

        if (!_xsettings_notify_atom) {
            _xsettings_notify_atom = internAtom(connection, "_XSETTINGS_SETTINGS_NOTIFY");
        }

        if (!_xsettings_signal_atom) {
            _xsettings_signal_atom = internAtom(connection,"_XSETTINGS_SETTINGS_SIGNAL");
        }

        // init xsettings owner
        if (!_xsettings_owner) {
            _xsettings_owner = DXcbXSettings::getOwner(connection, 0);

            if (_xsettings_owner) {
                const uint32_t event = XCB_CW_EVENT_MASK;
                const uint32_t event_mask[] = { XCB_EVENT_MASK_STRUCTURE_NOTIFY|XCB_EVENT_MASK_PROPERTY_CHANGE };
                xcb_change_window_attributes(connection, _xsettings_owner, event, event_mask);
            }
        }
    }

    QByteArray getSettings()
    {
        DXcbConnectionGrabber connectionGrabber(connection);
        Q_UNUSED(connectionGrabber)
        int offset = 0;
        QByteArray settings;
        while (1) {
            xcb_get_property_cookie_t cookie = xcb_get_property(connection,
                                                                false,
                                                                x_settings_window,
                                                                x_settings_atom,
                                                                internAtom(connection, "_XSETTINGS_SETTINGS"),
                                                                offset/4,
                                                                8192);

            xcb_generic_error_t *error = nullptr;
            auto reply = xcb_get_property_reply(connection, cookie, &error);

            enum ErrorCode {
                BadWindow = 3
            };

            // 在窗口无效时，应当认为此native settings未初始化完成
            if (error && error->error_code == ErrorCode::BadWindow) {
                initialized = false;
                return settings;
            }

            bool more = false;
            if (!reply)
                return settings;

            const auto property_value_length = xcb_get_property_value_length(reply);
            settings.append(static_cast<const char *>(xcb_get_property_value(reply)), property_value_length);
            offset += property_value_length;
            more = reply->bytes_after != 0;
            free(reply);

            if (!more)
                break;
        }

        return settings;
    }

    void setSettings(const QByteArray &data)
    {
        DXcbConnectionGrabber connectionGrabber(connection);
        Q_UNUSED(connectionGrabber)
        xcb_change_property(connection,
                            XCB_PROP_MODE_REPLACE,
                            x_settings_window,
                            x_settings_atom,
                            internAtom(connection, "_XSETTINGS_SETTINGS"),
                            8, data.size(), data.constData());

        xcb_window_t xsettings_owner = _xsettings_owner;

        // xsettings owner窗口，按照标准，其属性变化的事件就表示了通知
        if (x_settings_window == xsettings_owner)
            return;

        // 对于非标准的窗口的xsettings，其窗口属性变化不是任何程序中都能默认收到，因此应该使用client message通知属性变化
        // 且窗口的属性改变事件可能会被其它事件过滤器接收并处理(eg: KWin)
        // 因此改用client message通知窗口级别的xsettings属性变化
        // 此client message事件发送给xsettings owner，且在事件数据
        // 中携带属性变化窗口的window id。
        if (xsettings_owner) {
            xcb_client_message_event_t notify_event;
            memset(&notify_event, 0, sizeof(notify_event));

            notify_event.response_type = XCB_CLIENT_MESSAGE;
            notify_event.format = 32;
            notify_event.sequence = 0;
            notify_event.window = xsettings_owner;
            notify_event.type = _xsettings_notify_atom;
            notify_event.data.data32[0] = x_settings_window;
            notify_event.data.data32[1] = x_settings_atom;

            xcb_send_event(connection, false, xsettings_owner, XCB_EVENT_MASK_PROPERTY_CHANGE, (const char *)&notify_event);
        }
    }

    static int round_to_nearest_multiple_of_4(int value)
    {
        int remainder = value % 4;
        if (!remainder)
            return value;
        return value + 4 - remainder;
    }

    void populateSettings(const QByteArray &xSettings)
    {
        if (xSettings.length() < 12)
            return;
        char byteOrder = xSettings.at(0);
        if (byteOrder != XCB_IMAGE_ORDER_LSB_FIRST && byteOrder != XCB_IMAGE_ORDER_MSB_FIRST) {
            qWarning("ByteOrder byte %d not 0 or 1", byteOrder);
            return;
        }

#define ADJUST_BO(b, t, x) \
        ((b == XCB_IMAGE_ORDER_LSB_FIRST) ?                          \
         qFromLittleEndian<t>(x) : \
         qFromBigEndian<t>(x))
#define VALIDATE_LENGTH(x)    \
        if ((size_t)xSettings.length() < (offset + local_offset + 12 + x)) { \
            qWarning("Length %d runs past end of data", x); \
            return;                                                     \
        }

        serial = ADJUST_BO(byteOrder, qint32, xSettings.mid(4,4).constData());
        uint number_of_settings = ADJUST_BO(byteOrder, quint32, xSettings.mid(8,4).constData());
        const char *data = xSettings.constData() + 12;
        size_t offset = 0;
        // 记录所有设置项的名称
        QSet<QByteArray> keys;
        keys.reserve(number_of_settings);

        for (uint i = 0; i < number_of_settings; i++) {
            int local_offset = 0;
            VALIDATE_LENGTH(2);
            XSettingsType type = static_cast<XSettingsType>(*reinterpret_cast<const quint8 *>(data + offset));
            local_offset += 2;

            VALIDATE_LENGTH(2);
            quint16 name_len = ADJUST_BO(byteOrder, quint16, data + offset + local_offset);
            local_offset += 2;

            VALIDATE_LENGTH(name_len);
            QByteArray name(data + offset + local_offset, name_len);
            local_offset += round_to_nearest_multiple_of_4(name_len);

            VALIDATE_LENGTH(4);
            int last_change_serial = ADJUST_BO(byteOrder, qint32, data + offset + local_offset);
            Q_UNUSED(last_change_serial);
            local_offset += 4;

            QVariant value;
            if (type == XSettingsTypeString) {
                VALIDATE_LENGTH(4);
                int value_length = ADJUST_BO(byteOrder, qint32, data + offset + local_offset);
                local_offset+=4;
                VALIDATE_LENGTH(value_length);
                QByteArray value_string(data + offset + local_offset, value_length);
                value.setValue(value_string);
                local_offset += round_to_nearest_multiple_of_4(value_length);
            } else if (type == XSettingsTypeInteger) {
                VALIDATE_LENGTH(4);
                int value_length = ADJUST_BO(byteOrder, qint32, data + offset + local_offset);
                local_offset += 4;
                value.setValue(value_length);
            } else if (type == XSettingsTypeColor) {
                VALIDATE_LENGTH(2*4);
                quint16 red = ADJUST_BO(byteOrder, quint16, data + offset + local_offset);
                local_offset += 2;
                quint16 green = ADJUST_BO(byteOrder, quint16, data + offset + local_offset);
                local_offset += 2;
                quint16 blue = ADJUST_BO(byteOrder, quint16, data + offset + local_offset);
                local_offset += 2;
                quint16 alpha= ADJUST_BO(byteOrder, quint16, data + offset + local_offset);
                local_offset += 2;
                QColor color_value(red,green,blue,alpha);
                value.setValue(color_value);
            }
            offset += local_offset;

            updateValue(settings[name], name,value,last_change_serial);
            keys << name;
        }

        for (const QByteArray &key : settings.keys()) {
            if (!keys.contains(key)) {
                // 通知属性已经无效
                updateValue(settings[key], key, QVariant(), INT_MAX);
                // 移除已经被删除的属性
                settings.remove(key);
            }
        }
    }

    QByteArray depopulateSettings()
    {
        QByteArray xSettings;
        uint number_of_settings = settings.size();
        xSettings.reserve(12 + number_of_settings * 12);
        char byteOrder = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? XCB_IMAGE_ORDER_LSB_FIRST : XCB_IMAGE_ORDER_MSB_FIRST;

        xSettings.append(byteOrder); //byte-order
        xSettings.append(3, '\0'); //unused
        xSettings.append((char*)&serial, sizeof(serial)); //SERIAL
        xSettings.append((char*)&number_of_settings, sizeof(number_of_settings)); //N_SETTINGS
        uint number_of_settings_index = xSettings.size() - sizeof(number_of_settings);
        for (auto i = settings.constBegin(); i != settings.constEnd(); ++i) {
            const DXcbXSettingsPropertyValue &value = i.value();

            // 忽略无效的数据
            if (!value.value.isValid()) {
                --number_of_settings;
                continue;
            }

            char type = XSettingsTypeString;
            const QByteArray &key = i.key();
            quint16 key_size = key.size();

            switch (value.value.type()) {
            case QMetaType::QColor:
                type = XSettingsTypeColor;
                break;
            case QMetaType::Int:
            case QMetaType::Bool:
                type = XSettingsTypeInteger;
                break;
            default:
                break;
            }

            xSettings.append(type); //type
            xSettings.append('\0'); //unused
            xSettings.append((char*)&key_size, 2); //name-len
            xSettings.append(key.constData()); //name
            xSettings.append(3 - (key_size + 3) % 4, '\0'); //4字节对齐
            xSettings.append((char*)&value.last_change_serial, 4); //last-change-serial

            QByteArray value_data;

            if (type == XSettingsTypeInteger) {
                qint32 int_value = value.value.toInt();
                value_data.append((char*)&int_value, 4);
            } else if (type == XSettingsTypeColor) {
                const QColor &color = qvariant_cast<QColor>(value.value);
                quint16 red = color.red();
                quint16 green = color.green();
                quint16 blue = color.blue();
                quint16 alpha = color.alpha();

                value_data.append((char*)&red, 2);
                value_data.append((char*)&green, 2);
                value_data.append((char*)&blue, 2);
                value_data.append((char*)&alpha, 2);
            } else {
                const QByteArray &string_data = value.value.toByteArray();
                quint32 data_size = string_data.size();
                value_data.append((char*)&data_size, 4);
                value_data.append(string_data);
                value_data.append(3 - (string_data.size() + 3) % 4, '\0'); //4字节对齐
            }

            xSettings.append(value_data);
        }

        if (number_of_settings == 0) {
            return QByteArray();
        }
        memcpy(xSettings.data() + number_of_settings_index, &number_of_settings, sizeof(number_of_settings));

        return xSettings;
    }

    void init(xcb_window_t setting_window, DXcbXSettings *object)
    {
        x_settings_window = setting_window;
        mapped.insert(x_settings_window, object);
        initialized = true;

        populateSettings(getSettings());
    }

    bool updateValue(DXcbXSettingsPropertyValue &xvalue, const QByteArray &name, const QVariant &value, int last_change_serial)
    {
        if (xvalue.updateValue(connection, name, value, last_change_serial)) {
            for (const auto &callback : callback_links) {
                callback.func(connection, name, value, callback.handle);
            }

            q_ptr->handlePropertyChanged(name, value);

            return true;
        }

        return false;
    }

    DXcbXSettings *q_ptr = nullptr;
    xcb_connection_t *connection;
    xcb_window_t x_settings_window;
    // 保存xsetting值的窗口属性
    xcb_atom_t x_settings_atom;
    qint32 serial = -1;
    QHash<QByteArray, DXcbXSettingsPropertyValue> settings;
    std::vector<DXcbXSettingsCallback> callback_links;
    std::vector<DXcbXSettingsSignalCallback> signal_callback_links;
    bool initialized;

    static xcb_window_t _xsettings_owner;
    // 用于通知窗口native设置项发生改变
    static xcb_atom_t _xsettings_notify_atom;
    // 用于实现信号通知
    static xcb_atom_t _xsettings_signal_atom;
    static QMultiHash<xcb_window_t, DXcbXSettings*> mapped;
};

xcb_atom_t DXcbXSettingsPrivate::_xsettings_notify_atom = 0;
xcb_atom_t DXcbXSettingsPrivate::_xsettings_signal_atom = 0;
xcb_window_t DXcbXSettingsPrivate::_xsettings_owner = 0;
QMultiHash<xcb_window_t, DXcbXSettings*> DXcbXSettingsPrivate::mapped;

DXcbXSettings::DXcbXSettings(xcb_connection_t *connection, const QByteArray &property)
    : DXcbXSettings(connection, 0, property)
{

}

DXcbXSettings::DXcbXSettings(xcb_connection_t *connection, xcb_window_t setting_window, const QByteArray &property)
    : d_ptr(new DXcbXSettingsPrivate(connection, property, this))
{
    if (!setting_window) {
        setting_window = d_ptr->_xsettings_owner;
    }

    d_ptr->init(setting_window, this);
}

DXcbXSettings::DXcbXSettings(xcb_window_t setting_window, const QByteArray &property)
    : DXcbXSettings(nullptr, setting_window, property)
{

}

DXcbXSettings::~DXcbXSettings()
{
    DXcbXSettingsPrivate::mapped.remove(d_ptr->x_settings_window, this);
    delete d_ptr;
    d_ptr = 0;
}

xcb_window_t DXcbXSettings::getOwner(xcb_connection_t *conn, int screenNumber)
{
    struct XcbConnectionDeleter
    {
        static inline void cleanup(xcb_connection_t *conn)
        {
            xcb_disconnect(conn);
        }
    };

    QScopedPointer<xcb_connection_t, XcbConnectionDeleter> tmp_conn;

    if (!conn) {
        conn = xcb_connect(qgetenv("DISPLAY"), &screenNumber);

        if (!conn)
            return XCB_NONE;

        tmp_conn.reset(conn);
    }

    QByteArray settings_atom_for_screen("_XSETTINGS_S");
    settings_atom_for_screen.append(QByteArray::number(screenNumber));
    auto atom_reply = Q_XCB_REPLY(xcb_intern_atom,
                                  conn,
                                  true,
                                  settings_atom_for_screen.length(),
                                  settings_atom_for_screen.constData());
    if (!atom_reply)
        return XCB_NONE;

    xcb_atom_t selection_owner_atom = atom_reply->atom;

    auto selection_result = Q_XCB_REPLY(xcb_get_selection_owner,
                                        conn, selection_owner_atom);
    if (!selection_result)
        return XCB_NONE;

    return selection_result->owner;
}

bool DXcbXSettings::initialized() const
{
    Q_D(const DXcbXSettings);
    return d->initialized;
}

bool DXcbXSettings::isEmpty() const
{
    Q_D(const DXcbXSettings);
    return d->settings.isEmpty();
}

bool DXcbXSettings::contains(const QByteArray &property) const
{
    Q_D(const DXcbXSettings);
    return d->settings.contains(property);
}

bool DXcbXSettings::handlePropertyNotifyEvent(const xcb_property_notify_event_t *event)
{
    // 其它窗口的xsettings属性变化是通过client message通知的
    if (event->window != DXcbXSettingsPrivate::_xsettings_owner) {
        return false;
    }

    auto self_list = DXcbXSettingsPrivate::mapped.values(event->window);

    if (self_list.isEmpty())
        return false;

    for (DXcbXSettings *self : self_list) {
        // 另外需要判断此对象的设置是监听的哪个窗口属性
        if (event->atom == self->d_ptr->x_settings_atom)
            self->d_ptr->populateSettings(self->d_ptr->getSettings());
    }

    return true;
}

bool DXcbXSettings::handleClientMessageEvent(const xcb_client_message_event_t *event)
{
    if (event->format != 32)
        return false;

    if (event->type == DXcbXSettingsPrivate::_xsettings_notify_atom) {
        // data32[0]为属性变化的窗口
        auto self_list = DXcbXSettingsPrivate::mapped.values(event->data.data32[0]);

        if (self_list.isEmpty())
            return false;

        for (DXcbXSettings *self : self_list) {
            // data32[1]为变化的属性类型
            if (self->d_ptr->x_settings_atom == event->data.data32[1])
                self->d_ptr->populateSettings(self->d_ptr->getSettings());
        }

        return true;
    } else if ( event->type == DXcbXSettingsPrivate::_xsettings_signal_atom) {
        // data32[0]为属性变化的窗口
        xcb_window_t window = event->data.data32[0];
        auto self_list = window ? DXcbXSettingsPrivate::mapped.values(window) : DXcbXSettingsPrivate::mapped.values();

        if (self_list.isEmpty())
            return false;

        // data32[1]记录信号属于窗口的哪个属性, 为0表示任意属性
        xcb_atom_t type = event->data.data32[1];

        for (DXcbXSettings *self : self_list) {
            if (type && type != self->d_ptr->x_settings_atom)
                continue;

            // data32[2]为signal id
            xcb_atom_t signal = event->data.data32[2];
#ifdef IN_DXCB_PLUGIN
            const QByteArray signal_string(DPlatformIntegration::xcbConnection()->atomName(signal));
#else
            const QByteArray signal_string(atomName(self->d_ptr->connection, signal));
#endif

            for (DXcbXSettingsSignalCallback cb : self->d_ptr->signal_callback_links) {
                cb.func(self->d_ptr->connection, signal_string, event->data.data32[3], event->data.data32[4], cb.handle);
            }

            self->handleNotify(signal_string, event->data.data32[3], event->data.data32[4]);
        }

        return true;
    }

    return false;
}

void DXcbXSettings::clearSettings(xcb_window_t setting_window)
{
    if (DXcbXSettings *self = DXcbXSettingsPrivate::mapped.value(setting_window)) {
        xcb_delete_property(self->d_ptr->connection, setting_window, self->d_ptr->x_settings_atom);
    }
}

void DXcbXSettings::registerCallbackForProperty(const QByteArray &property, DXcbXSettings::PropertyChangeFunc func, void *handle)
{
    Q_D(DXcbXSettings);
    d->settings[property].addCallback(func,handle);
}

void DXcbXSettings::removeCallbackForHandle(const QByteArray &property, void *handle)
{
    Q_D(DXcbXSettings);
    auto &callbacks = d->settings[property].callback_links;

    auto isCallbackForHandle = [handle](const DXcbXSettingsCallback &cb) { return cb.handle == handle; };

    callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
                                   isCallbackForHandle),
                    callbacks.end());
}

void DXcbXSettings::removeCallbackForHandle(void *handle)
{
    Q_D(DXcbXSettings);
    for (QHash<QByteArray, DXcbXSettingsPropertyValue>::const_iterator it = d->settings.cbegin();
         it != d->settings.cend(); ++it) {
        removeCallbackForHandle(it.key(),handle);
    }

    auto isCallbackForHandle = [handle](const DXcbXSettingsCallback &cb) { return cb.handle == handle; };
    d->callback_links.erase(std::remove_if(d->callback_links.begin(), d->callback_links.end(), isCallbackForHandle));
}

void DXcbXSettings::registerSignalCallback(DXcbXSettings::SignalFunc func, void *handle)
{
    Q_D(DXcbXSettings);
    DXcbXSettingsSignalCallback callback = { func, handle };
    d->signal_callback_links.push_back(callback);
}

void DXcbXSettings::removeSignalCallback(void *handle)
{
    Q_D(DXcbXSettings);
    auto isCallbackForHandle = [handle](const DXcbXSettingsSignalCallback &cb) { return cb.handle == handle; };
    d->signal_callback_links.erase(std::remove_if(d->signal_callback_links.begin(), d->signal_callback_links.end(), isCallbackForHandle));
}

void DXcbXSettings::emitSignal(const QByteArray &signal, qint32 data1, qint32 data2)
{
    Q_D(const DXcbXSettings);
    emitSignal(d->connection, d->x_settings_window, d->x_settings_atom, signal, data1, data2);
}

void DXcbXSettings::emitSignal(xcb_connection_t *conn, xcb_window_t window, xcb_atom_t property,
                               const QByteArray &signal, qint32 data1, qint32 data2)
{
    xcb_window_t xsettings_owner = DXcbXSettingsPrivate::_xsettings_owner;

    if (!xsettings_owner) {
        return;
    }

    xcb_atom_t signal_atom = internAtom(conn, signal.constData());

    // 使用client message事件模拟信号
    xcb_client_message_event_t notify_event;

    notify_event.response_type = XCB_CLIENT_MESSAGE;
    notify_event.format = 32;
    notify_event.sequence = 0;
    notify_event.window = xsettings_owner;
    // 标记为信号通知
    notify_event.type = DXcbXSettingsPrivate::_xsettings_signal_atom;
    // 信号附属的窗口
    notify_event.data.data32[0] = window;
    // 信号附属窗口的属性
    notify_event.data.data32[1] = property;
    // 信号id
    notify_event.data.data32[2] = signal_atom;
    // 信号携带的数据
    notify_event.data.data32[3] = data1;
    // 信号携带的数据
    notify_event.data.data32[4] = data2;

    xcb_send_event(conn,
                   false,
                   xsettings_owner,
                   XCB_EVENT_MASK_PROPERTY_CHANGE,
                   (const char *)&notify_event);
}

QVariant DXcbXSettings::setting(const QByteArray &property) const
{
    Q_D(const DXcbXSettings);
    return d->settings.value(property).value;
}

void DXcbXSettings::setSetting(const QByteArray &property, const QVariant &value)
{
    Q_D(DXcbXSettings);

    DXcbXSettingsPropertyValue &xvalue = d->settings[property];

    if (xvalue.value == value)
        return;

    d->updateValue(xvalue, property, value, xvalue.last_change_serial + 1);

    // 移除无效的属性
    if (!value.isValid()) {
        d->settings.remove(property);
    }

    ++d->serial;
    // 更新属性
    d->setSettings(d->depopulateSettings());
}

QByteArrayList DXcbXSettings::settingKeys() const
{
    Q_D(const DXcbXSettings);
    return d->settings.keys();
}

void DXcbXSettings::registerCallback(DXcbXSettings::PropertyChangeFunc func, void *handle)
{
    Q_D(DXcbXSettings);
    DXcbXSettingsCallback callback = { func, handle };
    d->callback_links.push_back(callback);
}

DPP_END_NAMESPACE
