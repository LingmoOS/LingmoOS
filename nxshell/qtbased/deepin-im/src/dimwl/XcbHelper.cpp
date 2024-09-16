#include "XcbHelper.h"

#include <memory>
#include <stdexcept>

#include <limits.h>
#include <string.h>

XcbHelper::XcbHelper() { }

XcbHelper::~XcbHelper() = default;

void XcbHelper::flush()
{
    xcb_flush(conn_);
}

xcb_atom_t XcbHelper::getAtom(const std::string &atomName)
{
    auto iter = atoms_.find(atomName);
    if (iter != atoms_.end()) {
        return iter->second;
    }

    auto reply = XCB_REPLY(xcb_intern_atom, conn_, 0, atomName.length(), atomName.c_str());
    if (!reply) {
        throw std::runtime_error("xcb_intern_atom failed");
        return 0;
    }

    auto atom = reply->atom;
    atoms_.emplace(atomName, atom);
    return atom;
}

std::vector<char> XcbHelper::getProperty(xcb_window_t window,
                                         const std::string &property,
                                         uint32_t size)
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

std::vector<char> XcbHelper::getProperty(xcb_window_t window, const std::string &property)
{
    // Don't read anything, just get the size of the property data
    auto reply = XCB_REPLY(xcb_get_property,
                           conn_,
                           false,
                           window,
                           getAtom(property),
                           XCB_GET_PROPERTY_TYPE_ANY,
                           0,
                           0);
    if (!reply || reply->type == XCB_NONE) {
        // qWarning() << "no reply:" << windowToString(window) << property.c_str();
        return {};
    }

    uint32_t offset = 0;
    auto bytesLeft = reply->bytes_after;
    std::vector<char> buff(bytesLeft);

    while (bytesLeft > 0) {
        std::tie(offset, bytesLeft) = getPropertyAux(buff, window, property, offset, UINT_MAX);
    }

    return buff;
}

std::tuple<uint32_t, uint32_t> XcbHelper::getPropertyAux(std::vector<char> &buff,
                                                         xcb_window_t window,
                                                         const std::string &property,
                                                         uint32_t offset,
                                                         uint32_t size)
{
    auto xcbOffset = offset * 4;

    auto reply = XCB_REPLY(xcb_get_property,
                           conn_,
                           false,
                           window,
                           getAtom(property),
                           XCB_GET_PROPERTY_TYPE_ANY,
                           xcbOffset,
                           size / 4);
    if (!reply || reply->type == XCB_NONE) {
        // qWarning() << "no reply:" << windowToString(window) << property.c_str();
        return { 0, 0 };
    }

    auto bytesLeft = reply->bytes_after;
    char *data = static_cast<char *>(xcb_get_property_value(reply.get()));
    int length = xcb_get_property_value_length(reply.get());

    if ((offset + length) > buff.size()) {
        // qWarning("buffer overflow");
        length = buff.size() - offset;
        // escape loop
        bytesLeft = 0;
    }

    memcpy(buff.data() + offset, data, length);
    offset += length;

    return { offset, bytesLeft };
}

void XcbHelper::setPropertyAtom(xcb_window_t window,
                                const std::string &property,
                                const std::string &value)
{
    xcb_atom_t propertyAtom = getAtom(property);
    xcb_atom_t valueAtom = getAtom(value);
    xcb_atom_t values[] = { valueAtom };
    xcb_change_property(conn_,
                        XCB_PROP_MODE_APPEND,
                        window,
                        propertyAtom,
                        XCB_ATOM_ATOM,
                        32,
                        1,
                        values);
}

void XcbHelper::setPropertyAtoms(xcb_window_t window,
                                 const std::string &property,
                                 const std::vector<std::string> &values)
{
    xcb_atom_t propertyAtom = getAtom(property);
    std::vector<xcb_atom_t> valueAtoms;
    valueAtoms.reserve(values.size());
    for (auto &value : values) {
        valueAtoms.emplace_back(getAtom(value));
    }
    xcb_change_property(conn_,
                        XCB_PROP_MODE_APPEND,
                        window,
                        propertyAtom,
                        XCB_ATOM_ATOM,
                        32,
                        valueAtoms.size(),
                        valueAtoms.data());
}

void XcbHelper::deleteProperty(xcb_window_t window, const std::string &property)
{
    xcb_atom_t propertyAtom = getAtom(property);
    xcb_delete_property(conn_, window, propertyAtom);
}

void XcbHelper::changeWindowAttributes(xcb_window_t window,
                                       uint32_t valueMask,
                                       const uint32_t *valueList)
{
    xcb_change_window_attributes(conn_, window, valueMask, valueList);
}

void XcbHelper::mapWindow(xcb_window_t window)
{
    xcb_map_window(conn_, window);
}

void XcbHelper::unmapWindow(xcb_window_t window)
{
    xcb_unmap_window(conn_, window);
}

void XcbHelper::auxConfigureWindow(xcb_window_t window,
                                   uint16_t mask,
                                   const xcb_params_configure_window_t *params)
{
    xcb_aux_configure_window(conn_, window, mask, params);
}
