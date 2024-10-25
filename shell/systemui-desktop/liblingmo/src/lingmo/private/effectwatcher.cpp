/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "effectwatcher_p.h"

#include <X11/Xlib.h>

namespace Lingmo
{
EffectWatcher::EffectWatcher(const QString &property, QObject *parent)
    : QObject(parent)
    , m_property(XCB_ATOM_NONE)
    , m_x11Interface(qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
{
    init(property);
}

void EffectWatcher::init(const QString &property)
{
    if (!m_x11Interface) {
        return;
    }
    QCoreApplication::instance()->installNativeEventFilter(this);

    xcb_connection_t *c = m_x11Interface->connection();
    const QByteArray propertyName = property.toLatin1();
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, propertyName.length(), propertyName.constData());
    xcb_get_window_attributes_cookie_t winAttrCookie = xcb_get_window_attributes_unchecked(c, DefaultRootWindow(m_x11Interface->display()));

    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom.isNull()) {
        m_property = atom->atom;
    }
    m_effectActive = isEffectActive();

    QScopedPointer<xcb_get_window_attributes_reply_t, QScopedPointerPodDeleter> attrs(xcb_get_window_attributes_reply(c, winAttrCookie, nullptr));
    if (!attrs.isNull()) {
        uint32_t events = attrs->your_event_mask | XCB_EVENT_MASK_PROPERTY_CHANGE;
        xcb_change_window_attributes(c, DefaultRootWindow(m_x11Interface->display()), XCB_CW_EVENT_MASK, &events);
    }
}

bool EffectWatcher::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);
    // A faster comparison than eventType != "xcb_generic_event_t"
    // given that eventType can only have the following values:
    // "xcb_generic_event_t", "windows_generic_MSG" and "mac_generic_NSEvent"
    // According to https://doc.qt.io/qt-5/qabstractnativeeventfilter.html
    if (eventType[0] != 'x') {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
    uint response_type = event->response_type & ~0x80;
    if (response_type != XCB_PROPERTY_NOTIFY || m_property == XCB_ATOM_NONE) {
        return false;
    }

    xcb_property_notify_event_t *prop_event = reinterpret_cast<xcb_property_notify_event_t *>(event);
    if (prop_event->atom == m_property) {
        bool nowEffectActive = isEffectActive();
        if (m_effectActive != nowEffectActive) {
            m_effectActive = nowEffectActive;
            Q_EMIT effectChanged(m_effectActive);
        }
    }
    return false;
}

bool EffectWatcher::isEffectActive() const
{
    if (m_property == XCB_ATOM_NONE || !m_x11Interface) {
        return false;
    }
    xcb_connection_t *c = m_x11Interface->connection();
    xcb_list_properties_cookie_t propsCookie = xcb_list_properties_unchecked(c, DefaultRootWindow(m_x11Interface->display()));
    QScopedPointer<xcb_list_properties_reply_t, QScopedPointerPodDeleter> props(xcb_list_properties_reply(c, propsCookie, nullptr));
    if (props.isNull()) {
        return false;
    }
    xcb_atom_t *atoms = xcb_list_properties_atoms(props.data());
    for (int i = 0; i < props->atoms_len; ++i) {
        if (atoms[i] == m_property) {
            return true;
        }
    }
    return false;
}

} // namespace Lingmo
