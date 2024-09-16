#ifndef XCBHELPER_H
#define XCBHELPER_H

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

#include <string>
#include <unordered_map>
#include <vector>

#define XCB_REPLY_CONNECTION_ARG(connection, ...) connection
#define XCB_REPLY(call, ...)         \
    std::unique_ptr<call##_reply_t>( \
        call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr))

/**
 * @brief Bit mask to find event type regardless of event source.
 *
 * Each event in the X11 protocol contains an 8-bit type code.
 * The most-significant bit in this code is set if the event was
 * generated from a SendEvent request. This mask can be used to
 * determine the type of event regardless of how the event was
 * generated. See the X11R6 protocol specification for details.
 */
#define XCB_EVENT_RESPONSE_TYPE_MASK (0x7f)
#define XCB_EVENT_RESPONSE_TYPE(e) (e->response_type & XCB_EVENT_RESPONSE_TYPE_MASK)
#define XCB_EVENT_SENT(e) (e->response_type & ~XCB_EVENT_RESPONSE_TYPE_MASK)

class XcbHelper
{
public:
    XcbHelper();
    ~XcbHelper();

    void setConnection(xcb_connection_t *conn) { conn_ = conn; }

    void flush();

    xcb_atom_t getAtom(const std::string &atom);
    std::vector<char> getProperty(xcb_window_t window, const std::string &property, uint32_t size);
    std::vector<char> getProperty(xcb_window_t window, const std::string &property);

    void setPropertyAtom(xcb_window_t window,
                         const std::string &property,
                         const std::string &value);
    void setPropertyAtoms(xcb_window_t window,
                          const std::string &property,
                          const std::vector<std::string> &values);
    void deleteProperty(xcb_window_t window, const std::string &property);
    void changeWindowAttributes(xcb_window_t window,
                                uint32_t valueMask,
                                const uint32_t *valueList);

    void mapWindow(xcb_window_t window);
    void unmapWindow(xcb_window_t window);
    void auxConfigureWindow(xcb_window_t window,
                            uint16_t mask,
                            const xcb_params_configure_window_t *params);

private:
    std::tuple<uint32_t, uint32_t> getPropertyAux(std::vector<char> &buff,
                                                  xcb_window_t window,
                                                  const std::string &property,
                                                  uint32_t offset,
                                                  uint32_t size);

private:
    xcb_connection_t *conn_ = nullptr;
    std::unordered_map<std::string, xcb_atom_t> atoms_;
};

#endif // !XCBHELPER_H
