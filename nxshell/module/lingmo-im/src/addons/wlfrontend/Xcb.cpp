// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Xcb.h"

#include <QDebug>
#include <QSocketNotifier>

#include <iomanip>

Xcb::Xcb()
    : QObject()
{
    xconn_.reset(xcb_connect(nullptr, &defaultScreenNbr_));

    if (int err = xcb_connection_has_error(xconn_.get())) {
        qWarning() << "xcb connect failed";
        return;
    }

    setup_ = xcb_get_setup(xconn_.get());
    screen_ = screenOfDisplay(defaultScreenNbr_);

    xcbFd_ = xcb_get_file_descriptor(xconn_.get());
    socketNotifier_ = new QSocketNotifier(xcbFd_, QSocketNotifier::Type::Read, this);
    connect(socketNotifier_, &QSocketNotifier::activated, this, &Xcb::onXCBEvent);
}

Xcb::~Xcb() { }

xcb_atom_t Xcb::getAtom(const std::string &atomName)
{
    auto iter = atoms_.find(atomName);
    if (iter != atoms_.end()) {
        return iter->second;
    }

    auto reply = XCB_REPLY(xcb_intern_atom, xconn_.get(), 0, atomName.length(), atomName.c_str());
    if (!reply) {
        throw std::runtime_error("xcb_intern_atom failed");
        return 0;
    }

    auto atom = reply->atom;
    atoms_.emplace(atomName, atom);
    return atom;
}

std::vector<char> Xcb::getProperty(xcb_window_t window, const std::string &property, uint32_t size)
{
    std::vector<char> buff(size);

    uint32_t offset = 0;
    auto bytesLeft = size;
    std::tie(offset, bytesLeft) = getPropertyAux(buff, window, property, 0, size);
    if (offset == 0 && bytesLeft == 0) {
        return {};
    }

    return buff;
}

std::vector<char> Xcb::getProperty(xcb_window_t window, const std::string &property)
{
    // Don't read anything, just get the size of the property data
    auto reply = XCB_REPLY(xcb_get_property,
                           xconn_.get(),
                           false,
                           window,
                           getAtom(property),
                           XCB_GET_PROPERTY_TYPE_ANY,
                           0,
                           0);
    if (!reply || reply->type == XCB_NONE) {
        qWarning() << "no reply:" << windowToString(window).c_str() << property.c_str();
        return {};
    }

    uint32_t offset = 0;
    auto bytesLeft = reply->bytes_after;
    qInfo() << "size: " << bytesLeft;
    std::vector<char> buff(bytesLeft);

    while (bytesLeft > 0) {
        std::tie(offset, bytesLeft) = getPropertyAux(buff, window, property, offset, UINT_MAX);
    }

    return buff;
}

xcb_screen_t *Xcb::screenOfDisplay(int screen)
{
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup_);
    for (; iter.rem; --screen, xcb_screen_next(&iter)) {
        if (screen == 0) {
            return iter.data;
        }
    }

    return nullptr;
}

std::string Xcb::windowToString(xcb_window_t window)
{
    if (window == 0) {
        return {};
    }

    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(sizeof(xcb_window_t) * 2) << std::hex
           << window;
    return stream.str();
}

xcb_window_t Xcb::stringToWindow(const std::string &string)
{
    bool ok = false;
    return std::stoul(string, nullptr, 16);
}

void Xcb::onXCBEvent(QSocketDescriptor socket, QSocketNotifier::Type activationEvent)
{
    std::unique_ptr<xcb_generic_event_t> event;
    while (event.reset(xcb_poll_for_event(xconn_.get())), event) {
        xcbEvent(event);
    }
}

std::tuple<uint32_t, uint32_t> Xcb::getPropertyAux(std::vector<char> &buff,
                                                   xcb_window_t window,
                                                   const std::string &property,
                                                   uint32_t offset,
                                                   uint32_t size)
{
    auto xcbOffset = offset * 4;

    auto reply = XCB_REPLY(xcb_get_property,
                           xconn_.get(),
                           false,
                           window,
                           getAtom(property),
                           XCB_GET_PROPERTY_TYPE_ANY,
                           xcbOffset,
                           size / 4);
    if (!reply || reply->type == XCB_NONE) {
        qWarning() << "no reply:" << windowToString(window).c_str() << property.c_str();
        return { 0, 0 };
    }

    auto bytesLeft = reply->bytes_after;
    char *data = static_cast<char *>(xcb_get_property_value(reply.get()));
    int length = xcb_get_property_value_length(reply.get());

    if ((offset + length) > buff.size()) {
        qWarning("buffer overflow");
        length = buff.size() - offset;
        // escape loop
        bytesLeft = 0;
    }

    memcpy(buff.data() + offset, data, length);
    offset += length;

    return { offset, bytesLeft };
}
