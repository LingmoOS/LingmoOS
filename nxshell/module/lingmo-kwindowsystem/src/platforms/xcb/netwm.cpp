/*

  Copyright (c) 2000 Troll Tech AS
  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

*/

//#define NETWMDEBUG
#include "netwm.h"

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "netwm_p.h"
#include "atoms_p.h"

#include <config-kwindowsystem.h>
#if KWINDOWSYSTEM_HAVE_X11 //FIXME

#include <qx11info_x11.h>

#include <kwindowsystem.h>
#include <kxutils_p.h>

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// This struct is defined here to avoid a dependency on xcb-icccm
struct kde_wm_hints {
    uint32_t      flags;
    uint32_t      input;
    int32_t       initial_state;
    xcb_pixmap_t  icon_pixmap;
    xcb_window_t  icon_window;
    int32_t       icon_x;
    int32_t       icon_y;
    xcb_pixmap_t  icon_mask;
    xcb_window_t  window_group;
};

typedef QHash< xcb_connection_t*, QSharedDataPointer<Atoms> > AtomHash;
Q_GLOBAL_STATIC(AtomHash, s_gAtomsHash)

static QSharedDataPointer<Atoms> atomsForConnection(xcb_connection_t *c)
{
    auto it = s_gAtomsHash->constFind(c);
    if (it == s_gAtomsHash->constEnd()) {
        QSharedDataPointer<Atoms> atom(new Atoms(c));
        s_gAtomsHash->insert(c, atom);
        return atom;
    }
    return it.value();
}

Atoms::Atoms(xcb_connection_t *c)
    : QSharedData()
    , m_connection(c)
{
    for (int i = 0; i < KwsAtomCount; ++i) {
        m_atoms[i] = XCB_ATOM_NONE;
    }
    init();
}

static const uint32_t netwm_sendevent_mask =
    (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY);

const long MAX_PROP_SIZE = 100000;

static char *nstrdup(const char *s1)
{
    if (! s1) {
        return (char *) nullptr;
    }

    int l = strlen(s1) + 1;
    char *s2 = new char[l];
    strncpy(s2, s1, l);
    return s2;
}

static char *nstrndup(const char *s1, int l)
{
    if (! s1 || l == 0) {
        return (char *) nullptr;
    }

    char *s2 = new char[l + 1];
    strncpy(s2, s1, l);
    s2[l] = '\0';
    return s2;
}

static xcb_window_t *nwindup(const xcb_window_t *w1, int n)
{
    if (! w1 || n == 0) {
        return (xcb_window_t *) nullptr;
    }

    xcb_window_t *w2 = new xcb_window_t[n];
    while (n--) {
        w2[n] = w1[n];
    }
    return w2;
}

static void refdec_nri(NETRootInfoPrivate *p)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NET: decrementing NETRootInfoPrivate::ref (%d)\n", p->ref - 1);
#endif

    if (! --p->ref) {
#ifdef NETWMDEBUG
        fprintf(stderr, "NET: \tno more references, deleting\n");
#endif

        delete [] p->name;
        delete [] p->stacking;
        delete [] p->clients;
        delete [] p->virtual_roots;
        delete [] p->temp_buf;

        int i;
        for (i = 0; i < p->desktop_names.size(); i++) {
            delete [] p->desktop_names[i];
        }
    }
}

static void refdec_nwi(NETWinInfoPrivate *p)
{

#ifdef    NETWMDEBUG
    fprintf(stderr, "NET: decrementing NETWinInfoPrivate::ref (%d)\n", p->ref - 1);
#endif

    if (! --p->ref) {

#ifdef    NETWMDEBUG
        fprintf(stderr, "NET: \tno more references, deleting\n");
#endif

        delete [] p->name;
        delete [] p->visible_name;
        delete [] p->window_role;
        delete [] p->icon_name;
        delete [] p->visible_icon_name;
        delete [] p->startup_id;
        delete [] p->class_class;
        delete [] p->class_name;
        delete [] p->activities;
        delete [] p->client_machine;
        delete [] p->desktop_file;

        int i;
        for (i = 0; i < p->icons.size(); i++) {
            delete [] p->icons[i].data;
        }
        delete [] p->icon_sizes;
    }
}

template <typename T>
T get_value_reply(xcb_connection_t *c, const xcb_get_property_cookie_t cookie, xcb_atom_t type, T def, bool *success = nullptr)
{
    T value = def;

    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);

    if (success) {
        *success = false;
    }

    if (reply) {
        if (reply->type == type && reply->value_len == 1 && reply->format == sizeof(T) * 8) {
            value = *reinterpret_cast<T *>(xcb_get_property_value(reply));

            if (success) {
                *success = true;
            }
        }

        free(reply);
    }

    return value;
}

template <typename T>
QVector<T> get_array_reply(xcb_connection_t *c, const xcb_get_property_cookie_t cookie, xcb_atom_t type)
{
    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
    if (!reply) {
        return QVector<T>();
    }

    QVector<T> vector;

    if (reply->type == type && reply->value_len > 0 && reply->format == sizeof(T) * 8) {
        T *data = reinterpret_cast<T *>(xcb_get_property_value(reply));

        vector.resize(reply->value_len);
        memcpy((void *)&vector.first(), (void *)data, reply->value_len * sizeof(T));
    }

    free(reply);
    return vector;
}

static QByteArray get_string_reply(xcb_connection_t *c,
                                   const xcb_get_property_cookie_t cookie,
                                   xcb_atom_t type)
{
    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
    if (!reply) {
        return QByteArray();
    }

    QByteArray value;

    if (reply->type == type && reply->format == 8 && reply->value_len > 0) {
        const char *data = (const char *) xcb_get_property_value(reply);
        int len = reply->value_len;

        if (data) {
            value = QByteArray(data, data[len - 1] ? len : len - 1);
        }
    }

    free(reply);
    return value;
}

static QList<QByteArray> get_stringlist_reply(xcb_connection_t *c,
        const xcb_get_property_cookie_t cookie,
        xcb_atom_t type)
{
    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);
    if (!reply) {
        return QList<QByteArray>();
    }

    QList<QByteArray> list;

    if (reply->type == type && reply->format == 8 && reply->value_len > 0) {
        const char *data = (const char *) xcb_get_property_value(reply);
        int len = reply->value_len;

        if (data) {
            const QByteArray ba = QByteArray(data, data[len - 1] ? len : len - 1);
            list = ba.split('\0');
        }
    }

    free(reply);
    return list;
}

#ifdef NETWMDEBUG
static QByteArray get_atom_name(xcb_connection_t *c, xcb_atom_t atom)
{
    const xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(c, atom);

    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(c, cookie, 0);
    if (!reply) {
        return QByteArray();
    }

    QByteArray ba(xcb_get_atom_name_name(reply));
    free(reply);

    return ba;
}
#endif

void Atoms::init()
{
#define ENUM_CREATE_CHAR_ARRAY 1
#include "atoms_p.h" // creates const char* array "KwsAtomStrings"
    // Send the intern atom requests
    xcb_intern_atom_cookie_t cookies[KwsAtomCount];
    for (int i = 0; i < KwsAtomCount; ++i) {
        cookies[i] = xcb_intern_atom(m_connection, false, strlen(KwsAtomStrings[i]), KwsAtomStrings[i]);
    }

    // Get the replies
    for (int i = 0; i < KwsAtomCount; ++i) {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(m_connection, cookies[i], nullptr);
        if (!reply) {
            continue;
        }

        m_atoms[i] = reply->atom;
        free(reply);
    }
}

static void readIcon(xcb_connection_t *c, const xcb_get_property_cookie_t cookie,
                     NETRArray<NETIcon> &icons, int &icon_count)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NET: readIcon\n");
#endif

    // reset
    for (int i = 0; i < icons.size(); i++) {
        delete [] icons[i].data;
    }

    icons.reset();
    icon_count = 0;

    xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);

    if (!reply || reply->value_len < 3 || reply->format != 32 || reply->type != XCB_ATOM_CARDINAL) {
        if (reply) {
            free(reply);
        }

        return;
    }

    uint32_t *data = (uint32_t *) xcb_get_property_value(reply);

    for (unsigned int i = 0, j = 0; j < reply->value_len - 2; i++) {
        uint32_t width  = data[j++];
        uint32_t height = data[j++];
        uint32_t size   = width * height * sizeof(uint32_t);
        if (j + width * height > reply->value_len) {
            fprintf(stderr, "Ill-encoded icon data; proposed size leads to out of bounds access. Skipping. (%d x %d)\n", width, height);
            break;
        }
        if (width > 1024 || height > 1024) {
            fprintf(stderr, "Warning: found huge icon. The icon data may be ill-encoded. (%d x %d)\n", width, height);
            // do not break nor continue - the data may likely be junk, but causes no harm (yet) and might actually be just a huge icon, eg. when the icon system is abused to transfer wallpapers or such.
        }

        icons[i].size.width  = width;
        icons[i].size.height = height;
        icons[i].data = new unsigned char[size];

        memcpy((void *)icons[i].data, (const void *)&data[j], size);

        j += width * height;
        icon_count++;
    }

    free(reply);

#ifdef NETWMDEBUG
    fprintf(stderr, "NET: readIcon got %d icons\n", icon_count);
#endif
}

static void send_client_message(xcb_connection_t *c, uint32_t mask,
                                xcb_window_t destination, xcb_window_t window,
                                xcb_atom_t message, const uint32_t data[])
{
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = window;
    event.type = message;

    for (int i = 0; i < 5; i++) {
        event.data.data32[i] = data[i];
    }

    xcb_send_event(c, false, destination, mask, (const char *) &event);
}

template <class Z>
NETRArray<Z>::NETRArray()
    : sz(0),  capacity(2)
{
    d = (Z *) calloc(capacity, sizeof(Z)); // allocate 2 elts and set to zero
}

template <class Z>
NETRArray<Z>::~NETRArray()
{
    free(d);
}

template <class Z>
void NETRArray<Z>::reset()
{
    sz = 0;
    capacity = 2;
    d = (Z *) realloc(d, sizeof(Z) * capacity);
    memset((void *) d, 0, sizeof(Z)*capacity);
}

template <class Z>
Z &NETRArray<Z>::operator[](int index)
{
    if (index >= capacity) {
        // allocate space for the new data
        // open table has amortized O(1) access time
        // when N elements appended consecutively -- exa
        int newcapacity = 2 * capacity > index + 1 ? 2 * capacity : index + 1; // max
        // copy into new larger memory block using realloc
        d = (Z *) realloc(d, sizeof(Z) * newcapacity);
        memset((void *) &d[capacity], 0, sizeof(Z) * (newcapacity - capacity));
        capacity = newcapacity;
    }
    if (index >= sz) {          // at this point capacity>index
        sz = index + 1;
    }

    return d[index];
}

/*
 The viewport<->desktop matching is a bit backwards, since NET* classes are the base
 (and were originally even created with the intention of being the reference WM spec
 implementation) and KWindowSystem builds on top of it. However it's simpler to add watching
 whether the WM uses viewport is simpler to KWindowSystem and not having this mapping
 in NET* classes could result in some code using it directly and not supporting viewport.
 So NET* classes check if mapping is needed and if yes they forward to KWindowSystem,
 which will forward again back to NET* classes, but to viewport calls instead of desktop calls.
*/

// Construct a new NETRootInfo object.

NETRootInfo::NETRootInfo(xcb_connection_t *connection, xcb_window_t supportWindow, const char *wmName,
                         NET::Properties properties, NET::WindowTypes windowTypes, NET::States states,
                         NET::Properties2 properties2, NET::Actions actions,
                         int screen, bool doActivate)
{

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using window manager constructor\n");
#endif

    p = new NETRootInfoPrivate;
    p->ref = 1;
    p->atoms = atomsForConnection(connection);

    p->name = nstrdup(wmName);

    p->conn = connection;

    p->temp_buf = nullptr;
    p->temp_buf_size = 0;

    const xcb_setup_t *setup = xcb_get_setup(p->conn);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);

    if (screen != -1 && screen < setup->roots_len) {
        for (int i = 0; i < screen; i++) {
            xcb_screen_next(&it);
        }
    }

    p->root = it.data->root;
    p->supportwindow = supportWindow;
    p->number_of_desktops = p->current_desktop = 0;
    p->active = XCB_WINDOW_NONE;
    p->clients = p->stacking = p->virtual_roots = (xcb_window_t *) nullptr;
    p->clients_count = p->stacking_count = p->virtual_roots_count = 0;
    p->showing_desktop = false;
    p->desktop_layout_orientation = OrientationHorizontal;
    p->desktop_layout_corner = DesktopLayoutCornerTopLeft;
    p->desktop_layout_columns = p->desktop_layout_rows = 0;
    setDefaultProperties();
    p->properties = properties;
    p->properties2 = properties2;
    p->windowTypes = windowTypes;
    p->states = states;
    p->actions = actions;
    // force support for Supported and SupportingWMCheck for window managers
    p->properties |= (Supported | SupportingWMCheck);
    p->clientProperties = DesktopNames // the only thing that can be changed by clients
                          | WMPing; // or they can reply to this
    p->clientProperties2 = WM2DesktopLayout;

    p->role = WindowManager;

    if (doActivate) {
        activate();
    }
}

NETRootInfo::NETRootInfo(xcb_connection_t *connection, NET::Properties properties, NET::Properties2 properties2,
                         int screen, bool doActivate)
{

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using Client constructor\n");
#endif

    p = new NETRootInfoPrivate;
    p->ref = 1;
    p->atoms = atomsForConnection(connection);

    p->name = nullptr;

    p->conn = connection;

    p->temp_buf = nullptr;
    p->temp_buf_size = 0;

    const xcb_setup_t *setup = xcb_get_setup(p->conn);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);

    if (screen != -1 && screen < setup->roots_len) {
        for (int i = 0; i < screen; i++) {
            xcb_screen_next(&it);
        }
    }

    p->root = it.data->root;
    p->rootSize.width = it.data->width_in_pixels;
    p->rootSize.height = it.data->height_in_pixels;

    p->supportwindow = XCB_WINDOW_NONE;
    p->number_of_desktops = p->current_desktop = 0;
    p->active = XCB_WINDOW_NONE;
    p->clients = p->stacking = p->virtual_roots = (xcb_window_t *) nullptr;
    p->clients_count = p->stacking_count = p->virtual_roots_count = 0;
    p->showing_desktop = false;
    p->desktop_layout_orientation = OrientationHorizontal;
    p->desktop_layout_corner = DesktopLayoutCornerTopLeft;
    p->desktop_layout_columns = p->desktop_layout_rows = 0;
    setDefaultProperties();
    p->clientProperties = properties;
    p->clientProperties2 = properties2;
    p->properties = NET::Properties();
    p->properties2 = NET::Properties2();
    p->windowTypes = NET::WindowTypes();
    p->states = NET::States();
    p->actions = NET::Actions();

    p->role = Client;

    if (doActivate) {
        activate();
    }
}



// Copy an existing NETRootInfo object.

NETRootInfo::NETRootInfo(const NETRootInfo &rootinfo)
{

#ifdef    NETWMDEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using copy constructor\n");
#endif

    p = rootinfo.p;

    p->ref++;
}

// Be gone with our NETRootInfo.

NETRootInfo::~NETRootInfo()
{
    refdec_nri(p);

    if (! p->ref) {
        delete p;
    }
}

void NETRootInfo::setDefaultProperties()
{
    p->properties = Supported | SupportingWMCheck;
    p->windowTypes = NormalMask | DesktopMask | DockMask
                                    | ToolbarMask | MenuMask | DialogMask;
    p->states = Modal | Sticky | MaxVert | MaxHoriz | Shaded
                              | SkipTaskbar | StaysOnTop;
    p->properties2 = NET::Properties2();
    p->actions = NET::Actions();
    p->clientProperties = NET::Properties();
    p->clientProperties2 = NET::Properties2();
}

void NETRootInfo::activate()
{
    if (p->role == WindowManager) {

#ifdef    NETWMDEBUG
        fprintf(stderr,
                "NETRootInfo::activate: setting supported properties on root\n");
#endif

        setSupported();
        update(p->clientProperties, p->clientProperties2);
    } else {

#ifdef    NETWMDEBUG
        fprintf(stderr, "NETRootInfo::activate: updating client information\n");
#endif

        update(p->clientProperties, p->clientProperties2);
    }
}

void NETRootInfo::setClientList(const xcb_window_t *windows, unsigned int count)
{
    if (p->role != WindowManager) {
        return;
    }

    p->clients_count = count;

    delete [] p->clients;
    p->clients = nwindup(windows, count);

#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setClientList: setting list with %ld windows\n",
            p->clients_count);
#endif

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_CLIENT_LIST),
                        XCB_ATOM_WINDOW, 32, p->clients_count,
                        (const void *) windows);
}

void NETRootInfo::setClientListStacking(const xcb_window_t *windows, unsigned int count)
{
    if (p->role != WindowManager) {
        return;
    }

    p->stacking_count = count;
    delete [] p->stacking;
    p->stacking = nwindup(windows, count);

#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::setClientListStacking: setting list with %ld windows\n",
            p->clients_count);
#endif

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_CLIENT_LIST_STACKING),
                        XCB_ATOM_WINDOW, 32, p->stacking_count,
                        (const void *) windows);
}

void NETRootInfo::setNumberOfDesktops(int numberOfDesktops)
{
#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::setNumberOfDesktops: setting desktop count to %d (%s)\n",
            numberOfDesktops, (p->role == WindowManager) ? "WM" : "Client");
#endif

    if (p->role == WindowManager) {
        p->number_of_desktops = numberOfDesktops;
        const uint32_t d = numberOfDesktops;
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_NUMBER_OF_DESKTOPS),
                            XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
    } else {
        const uint32_t data[5] = {
            uint32_t(numberOfDesktops), 0, 0, 0, 0
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root,
                            p->root, p->atom(_NET_NUMBER_OF_DESKTOPS), data);
    }
}

void NETRootInfo::setCurrentDesktop(int desktop, bool ignore_viewport)
{
#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::setCurrentDesktop: setting current desktop = %d (%s)\n",
            desktop, (p->role == WindowManager) ? "WM" : "Client");
#endif

    if (p->role == WindowManager) {
        p->current_desktop = desktop;
        uint32_t d = p->current_desktop - 1;
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_CURRENT_DESKTOP),
                            XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
    } else {

        if (!ignore_viewport && KWindowSystem::mapViewport()) {
            KWindowSystem::setCurrentDesktop(desktop);
            return;
        }

        const uint32_t data[5] = {
            uint32_t(desktop - 1), 0, 0, 0, 0
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root,
                            p->root, p->atom(_NET_CURRENT_DESKTOP), data);
    }
}

void NETRootInfo::setDesktopName(int desktop, const char *desktopName)
{
    // Allow setting desktop names even for non-existent desktops, see the spec, sect.3.7.
    if (desktop < 1) {
        return;
    }

    delete [] p->desktop_names[desktop - 1];
    p->desktop_names[desktop - 1] = nstrdup(desktopName);

    unsigned int i, proplen,
             num = ((p->number_of_desktops > p->desktop_names.size()) ?
                    p->number_of_desktops : p->desktop_names.size());
    for (i = 0, proplen = 0; i < num; i++) {
        proplen += (p->desktop_names[i] != nullptr ? strlen(p->desktop_names[i]) + 1 : 1);
    }

    char *prop = new char[proplen], *propp = prop;

    for (i = 0; i < num; i++)
        if (p->desktop_names[i]) {
            strcpy(propp, p->desktop_names[i]);
            propp += strlen(p->desktop_names[i]) + 1;
        } else {
            *propp++ = '\0';
        }

#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::setDesktopName(%d, '%s')\n"
            "NETRootInfo::setDesktopName: total property length = %d",
            desktop, desktopName, proplen);
#endif

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_DESKTOP_NAMES),
                        p->atom(UTF8_STRING), 8, proplen, (const void *) prop);

    delete [] prop;
}

void NETRootInfo::setDesktopGeometry(const NETSize &geometry)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setDesktopGeometry( -- , { %d, %d }) (%s)\n",
            geometry.width, geometry.height, (p->role == WindowManager) ? "WM" : "Client");
#endif

    if (p->role == WindowManager) {
        p->geometry = geometry;

        uint32_t data[2];
        data[0] = p->geometry.width;
        data[1] = p->geometry.height;

        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_DESKTOP_GEOMETRY),
                            XCB_ATOM_CARDINAL, 32, 2, (const void *) data);
    } else {
        uint32_t data[5] = {
            uint32_t(geometry.width), uint32_t(geometry.height), 0, 0, 0
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root,
                            p->root, p->atom(_NET_DESKTOP_GEOMETRY), data);
    }
}

void NETRootInfo::setDesktopViewport(int desktop, const NETPoint &viewport)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setDesktopViewport(%d, { %d, %d }) (%s)\n",
            desktop, viewport.x, viewport.y, (p->role == WindowManager) ? "WM" : "Client");
#endif

    if (desktop < 1) {
        return;
    }

    if (p->role == WindowManager) {
        p->viewport[desktop - 1] = viewport;

        int d, i, l;
        l = p->number_of_desktops * 2;
        uint32_t *data = new uint32_t[l];
        for (d = 0, i = 0; d < p->number_of_desktops; d++) {
            data[i++] = p->viewport[d].x;
            data[i++] = p->viewport[d].y;
        }

        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_DESKTOP_VIEWPORT),
                            XCB_ATOM_CARDINAL, 32, l, (const void *) data);

        delete [] data;
    } else {
        const uint32_t data[5] = {
            uint32_t(viewport.x), uint32_t(viewport.y), 0, 0, 0
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root,
                            p->root, p->atom(_NET_DESKTOP_VIEWPORT), data);
    }
}

void NETRootInfo::setSupported()
{
    if (p->role != WindowManager) {
#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::setSupported - role != WindowManager\n");
#endif

        return;
    }

    xcb_atom_t atoms[KwsAtomCount];
    int pnum = 2;

    // Root window properties/messages
    atoms[0] = p->atom(_NET_SUPPORTED);
    atoms[1] = p->atom(_NET_SUPPORTING_WM_CHECK);

    if (p->properties & ClientList) {
        atoms[pnum++] = p->atom(_NET_CLIENT_LIST);
    }

    if (p->properties & ClientListStacking) {
        atoms[pnum++] = p->atom(_NET_CLIENT_LIST_STACKING);
    }

    if (p->properties & NumberOfDesktops) {
        atoms[pnum++] = p->atom(_NET_NUMBER_OF_DESKTOPS);
    }

    if (p->properties & DesktopGeometry) {
        atoms[pnum++] = p->atom(_NET_DESKTOP_GEOMETRY);
    }

    if (p->properties & DesktopViewport) {
        atoms[pnum++] = p->atom(_NET_DESKTOP_VIEWPORT);
    }

    if (p->properties & CurrentDesktop) {
        atoms[pnum++] = p->atom(_NET_CURRENT_DESKTOP);
    }

    if (p->properties & DesktopNames) {
        atoms[pnum++] = p->atom(_NET_DESKTOP_NAMES);
    }

    if (p->properties & ActiveWindow) {
        atoms[pnum++] = p->atom(_NET_ACTIVE_WINDOW);
    }

    if (p->properties & WorkArea) {
        atoms[pnum++] = p->atom(_NET_WORKAREA);
    }

    if (p->properties & VirtualRoots) {
        atoms[pnum++] = p->atom(_NET_VIRTUAL_ROOTS);
    }

    if (p->properties2 & WM2DesktopLayout) {
        atoms[pnum++] = p->atom(_NET_DESKTOP_LAYOUT);
    }

    if (p->properties & CloseWindow) {
        atoms[pnum++] = p->atom(_NET_CLOSE_WINDOW);
    }

    if (p->properties2 & WM2RestackWindow) {
        atoms[pnum++] = p->atom(_NET_RESTACK_WINDOW);
    }

    if (p->properties2 & WM2ShowingDesktop) {
        atoms[pnum++] = p->atom(_NET_SHOWING_DESKTOP);
    }

    // Application window properties/messages
    if (p->properties & WMMoveResize) {
        atoms[pnum++] = p->atom(_NET_WM_MOVERESIZE);
    }

    if (p->properties2 & WM2MoveResizeWindow) {
        atoms[pnum++] = p->atom(_NET_MOVERESIZE_WINDOW);
    }

    if (p->properties & WMName) {
        atoms[pnum++] = p->atom(_NET_WM_NAME);
    }

    if (p->properties & WMVisibleName) {
        atoms[pnum++] = p->atom(_NET_WM_VISIBLE_NAME);
    }

    if (p->properties & WMIconName) {
        atoms[pnum++] = p->atom(_NET_WM_ICON_NAME);
    }

    if (p->properties & WMVisibleIconName) {
        atoms[pnum++] = p->atom(_NET_WM_VISIBLE_ICON_NAME);
    }

    if (p->properties & WMDesktop) {
        atoms[pnum++] = p->atom(_NET_WM_DESKTOP);
    }

    if (p->properties & WMWindowType) {
        atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE);

        // Application window types
        if (p->windowTypes & NormalMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_NORMAL);
        }
        if (p->windowTypes & DesktopMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_DESKTOP);
        }
        if (p->windowTypes & DockMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_DOCK);
        }
        if (p->windowTypes & ToolbarMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_TOOLBAR);
        }
        if (p->windowTypes & MenuMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_MENU);
        }
        if (p->windowTypes & DialogMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_DIALOG);
        }
        if (p->windowTypes & UtilityMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_UTILITY);
        }
        if (p->windowTypes & SplashMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_SPLASH);
        }
        if (p->windowTypes & DropdownMenuMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU);
        }
        if (p->windowTypes & PopupMenuMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_POPUP_MENU);
        }
        if (p->windowTypes & TooltipMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_TOOLTIP);
        }
        if (p->windowTypes & NotificationMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_NOTIFICATION);
        }
        if (p->windowTypes & ComboBoxMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_COMBOBOX);
        }
        if (p->windowTypes & DNDIconMask) {
            atoms[pnum++] = p->atom(_NET_WM_WINDOW_TYPE_DND);
        }
        // KDE extensions
        if (p->windowTypes & OverrideMask) {
            atoms[pnum++] = p->atom(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE);
        }
        if (p->windowTypes & TopMenuMask) {
            atoms[pnum++] = p->atom(_KDE_NET_WM_WINDOW_TYPE_TOPMENU);
        }
        if (p->windowTypes & OnScreenDisplayMask) {
            atoms[pnum++] = p->atom(_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY);
        }
    }

    if (p->properties & WMState) {
        atoms[pnum++] = p->atom(_NET_WM_STATE);

        // Application window states
        if (p->states & Modal) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_MODAL);
        }
        if (p->states & Sticky) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_STICKY);
        }
        if (p->states & MaxVert) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_MAXIMIZED_VERT);
        }
        if (p->states & MaxHoriz) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_MAXIMIZED_HORZ);
        }
        if (p->states & Shaded) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_SHADED);
        }
        if (p->states & SkipTaskbar) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_SKIP_TASKBAR);
        }
        if (p->states & SkipPager) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_SKIP_PAGER);
        }
        if (p->states & SkipSwitcher) {
            atoms[pnum++] = p->atom(_KDE_NET_WM_STATE_SKIP_SWITCHER);
        }
        if (p->states & Hidden) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_HIDDEN);
        }
        if (p->states & FullScreen) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_FULLSCREEN);
        }
        if (p->states & KeepAbove) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_ABOVE);
        }
        if (p->states & KeepBelow) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_BELOW);
        }
        if (p->states & DemandsAttention) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_DEMANDS_ATTENTION);
        }

        if (p->states & StaysOnTop) {
            atoms[pnum++] = p->atom(_NET_WM_STATE_STAYS_ON_TOP);
        }
    }

    if (p->properties & WMStrut) {
        atoms[pnum++] = p->atom(_NET_WM_STRUT);
    }

    if (p->properties2 & WM2ExtendedStrut) {
        atoms[pnum++] = p->atom(_NET_WM_STRUT_PARTIAL);
    }

    if (p->properties & WMIconGeometry) {
        atoms[pnum++] = p->atom(_NET_WM_ICON_GEOMETRY);
    }

    if (p->properties & WMIcon) {
        atoms[pnum++] = p->atom(_NET_WM_ICON);
    }

    if (p->properties & WMPid) {
        atoms[pnum++] = p->atom(_NET_WM_PID);
    }

    if (p->properties & WMHandledIcons) {
        atoms[pnum++] = p->atom(_NET_WM_HANDLED_ICONS);
    }

    if (p->properties & WMPing) {
        atoms[pnum++] = p->atom(_NET_WM_PING);
    }

    if (p->properties2 & WM2UserTime) {
        atoms[pnum++] = p->atom(_NET_WM_USER_TIME);
    }

    if (p->properties2 & WM2StartupId) {
        atoms[pnum++] = p->atom(_NET_STARTUP_ID);
    }

    if (p->properties2 & WM2Opacity) {
        atoms[pnum++] = p->atom(_NET_WM_WINDOW_OPACITY);
    }

    if (p->properties2 & WM2FullscreenMonitors) {
        atoms[pnum++] = p->atom(_NET_WM_FULLSCREEN_MONITORS);
    }

    if (p->properties2 & WM2AllowedActions) {
        atoms[pnum++] = p->atom(_NET_WM_ALLOWED_ACTIONS);

        // Actions
        if (p->actions & ActionMove) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_MOVE);
        }
        if (p->actions & ActionResize) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_RESIZE);
        }
        if (p->actions & ActionMinimize) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_MINIMIZE);
        }
        if (p->actions & ActionShade) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_SHADE);
        }
        if (p->actions & ActionStick) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_STICK);
        }
        if (p->actions & ActionMaxVert) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_MAXIMIZE_VERT);
        }
        if (p->actions & ActionMaxHoriz) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_MAXIMIZE_HORZ);
        }
        if (p->actions & ActionFullScreen) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_FULLSCREEN);
        }
        if (p->actions & ActionChangeDesktop) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_CHANGE_DESKTOP);
        }
        if (p->actions & ActionClose) {
            atoms[pnum++] = p->atom(_NET_WM_ACTION_CLOSE);
        }
    }

    if (p->properties & WMFrameExtents) {
        atoms[pnum++] = p->atom(_NET_FRAME_EXTENTS);
        atoms[pnum++] = p->atom(_KDE_NET_WM_FRAME_STRUT);
    }

    if (p->properties2 & WM2FrameOverlap) {
        atoms[pnum++] = p->atom(_NET_WM_FRAME_OVERLAP);
    }

    if (p->properties2 & WM2KDETemporaryRules) {
        atoms[pnum++] = p->atom(_KDE_NET_WM_TEMPORARY_RULES);
    }
    if (p->properties2 & WM2FullPlacement) {
        atoms[pnum++] = p->atom(_NET_WM_FULL_PLACEMENT);
    }

    if (p->properties2 & WM2Activities) {
        atoms[pnum++] = p->atom(_KDE_NET_WM_ACTIVITIES);
    }

    if (p->properties2 & WM2BlockCompositing) {
        atoms[pnum++] = p->atom(_KDE_NET_WM_BLOCK_COMPOSITING);
        atoms[pnum++] = p->atom(_NET_WM_BYPASS_COMPOSITOR);
    }

    if (p->properties2 & WM2KDEShadow) {
        atoms[pnum++] = p->atom(_KDE_NET_WM_SHADOW);
    }

    if (p->properties2 & WM2OpaqueRegion) {
        atoms[pnum++] = p->atom(_NET_WM_OPAQUE_REGION);
    }

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_SUPPORTED),
                        XCB_ATOM_ATOM, 32, pnum, (const void *) atoms);

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_SUPPORTING_WM_CHECK),
                        XCB_ATOM_WINDOW, 32, 1, (const void *) & (p->supportwindow));

#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::setSupported: _NET_SUPPORTING_WM_CHECK = 0x%lx on 0x%lx\n"
            "                         : _NET_WM_NAME = '%s' on 0x%lx\n",
            p->supportwindow, p->supportwindow, p->name, p->supportwindow);
#endif

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->supportwindow,
                        p->atom(_NET_SUPPORTING_WM_CHECK), XCB_ATOM_WINDOW, 32,
                        1, (const void *) & (p->supportwindow));

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->supportwindow,
                        p->atom(_NET_WM_NAME), p->atom(UTF8_STRING), 8, strlen(p->name),
                        (const void *) p->name);
}

void NETRootInfo::updateSupportedProperties(xcb_atom_t atom)
{
    if (atom == p->atom(_NET_SUPPORTED)) {
        p->properties |= Supported;
    }

    else if (atom == p->atom(_NET_SUPPORTING_WM_CHECK)) {
        p->properties |= SupportingWMCheck;
    }

    else if (atom == p->atom(_NET_CLIENT_LIST)) {
        p->properties |= ClientList;
    }

    else if (atom == p->atom(_NET_CLIENT_LIST_STACKING)) {
        p->properties |= ClientListStacking;
    }

    else if (atom == p->atom(_NET_NUMBER_OF_DESKTOPS)) {
        p->properties |= NumberOfDesktops;
    }

    else if (atom == p->atom(_NET_DESKTOP_GEOMETRY)) {
        p->properties |= DesktopGeometry;
    }

    else if (atom == p->atom(_NET_DESKTOP_VIEWPORT)) {
        p->properties |= DesktopViewport;
    }

    else if (atom == p->atom(_NET_CURRENT_DESKTOP)) {
        p->properties |= CurrentDesktop;
    }

    else if (atom == p->atom(_NET_DESKTOP_NAMES)) {
        p->properties |= DesktopNames;
    }

    else if (atom == p->atom(_NET_ACTIVE_WINDOW)) {
        p->properties |= ActiveWindow;
    }

    else if (atom == p->atom(_NET_WORKAREA)) {
        p->properties |= WorkArea;
    }

    else if (atom == p->atom(_NET_VIRTUAL_ROOTS)) {
        p->properties |= VirtualRoots;
    }

    else if (atom == p->atom(_NET_DESKTOP_LAYOUT)) {
        p->properties2 |= WM2DesktopLayout;
    }

    else if (atom == p->atom(_NET_CLOSE_WINDOW)) {
        p->properties |= CloseWindow;
    }

    else if (atom == p->atom(_NET_RESTACK_WINDOW)) {
        p->properties2 |= WM2RestackWindow;
    }

    else if (atom == p->atom(_NET_SHOWING_DESKTOP)) {
        p->properties2 |= WM2ShowingDesktop;
    }

    // Application window properties/messages
    else if (atom == p->atom(_NET_WM_MOVERESIZE)) {
        p->properties |= WMMoveResize;
    }

    else if (atom == p->atom(_NET_MOVERESIZE_WINDOW)) {
        p->properties2 |= WM2MoveResizeWindow;
    }

    else if (atom == p->atom(_NET_WM_NAME)) {
        p->properties |= WMName;
    }

    else if (atom == p->atom(_NET_WM_VISIBLE_NAME)) {
        p->properties |= WMVisibleName;
    }

    else if (atom == p->atom(_NET_WM_ICON_NAME)) {
        p->properties |= WMIconName;
    }

    else if (atom == p->atom(_NET_WM_VISIBLE_ICON_NAME)) {
        p->properties |= WMVisibleIconName;
    }

    else if (atom == p->atom(_NET_WM_DESKTOP)) {
        p->properties |= WMDesktop;
    }

    else if (atom == p->atom(_NET_WM_WINDOW_TYPE)) {
        p->properties |= WMWindowType;
    }

    // Application window types
    else if (atom == p->atom(_NET_WM_WINDOW_TYPE_NORMAL)) {
        p->windowTypes |= NormalMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_DESKTOP)) {
        p->windowTypes |= DesktopMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_DOCK)) {
        p->windowTypes |= DockMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_TOOLBAR)) {
        p->windowTypes |= ToolbarMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_MENU)) {
        p->windowTypes |= MenuMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_DIALOG)) {
        p->windowTypes |= DialogMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_UTILITY)) {
        p->windowTypes |= UtilityMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_SPLASH)) {
        p->windowTypes |= SplashMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU)) {
        p->windowTypes |= DropdownMenuMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_POPUP_MENU)) {
        p->windowTypes |= PopupMenuMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_TOOLTIP)) {
        p->windowTypes |= TooltipMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_NOTIFICATION)) {
        p->windowTypes |= NotificationMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_COMBOBOX)) {
        p->windowTypes |= ComboBoxMask;
    } else if (atom == p->atom(_NET_WM_WINDOW_TYPE_DND)) {
        p->windowTypes |= DNDIconMask;
    }
    // KDE extensions
    else if (atom == p->atom(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE)) {
        p->windowTypes |= OverrideMask;
    } else if (atom == p->atom(_KDE_NET_WM_WINDOW_TYPE_TOPMENU)) {
        p->windowTypes |= TopMenuMask;
    } else if (atom == p->atom(_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY)) {
        p->windowTypes |= OnScreenDisplayMask;
    }

    else if (atom == p->atom(_NET_WM_STATE)) {
        p->properties |= WMState;
    }

    // Application window states
    else if (atom == p->atom(_NET_WM_STATE_MODAL)) {
        p->states |= Modal;
    } else if (atom == p->atom(_NET_WM_STATE_STICKY)) {
        p->states |= Sticky;
    } else if (atom == p->atom(_NET_WM_STATE_MAXIMIZED_VERT)) {
        p->states |= MaxVert;
    } else if (atom == p->atom(_NET_WM_STATE_MAXIMIZED_HORZ)) {
        p->states |= MaxHoriz;
    } else if (atom == p->atom(_NET_WM_STATE_SHADED)) {
        p->states |= Shaded;
    } else if (atom == p->atom(_NET_WM_STATE_SKIP_TASKBAR)) {
        p->states |= SkipTaskbar;
    } else if (atom == p->atom(_NET_WM_STATE_SKIP_PAGER)) {
        p->states |= SkipPager;
    } else if (atom == p->atom(_KDE_NET_WM_STATE_SKIP_SWITCHER)) {
        p->states |= SkipSwitcher;
    } else if (atom == p->atom(_NET_WM_STATE_HIDDEN)) {
        p->states |= Hidden;
    } else if (atom == p->atom(_NET_WM_STATE_FULLSCREEN)) {
        p->states |= FullScreen;
    } else if (atom == p->atom(_NET_WM_STATE_ABOVE)) {
        p->states |= KeepAbove;
    } else if (atom == p->atom(_NET_WM_STATE_BELOW)) {
        p->states |= KeepBelow;
    } else if (atom == p->atom(_NET_WM_STATE_DEMANDS_ATTENTION)) {
        p->states |= DemandsAttention;
    }

    else if (atom == p->atom(_NET_WM_STATE_STAYS_ON_TOP)) {
        p->states |= StaysOnTop;
    }

    else if (atom == p->atom(_NET_WM_STRUT)) {
        p->properties |= WMStrut;
    }

    else if (atom == p->atom(_NET_WM_STRUT_PARTIAL)) {
        p->properties2 |= WM2ExtendedStrut;
    }

    else if (atom == p->atom(_NET_WM_ICON_GEOMETRY)) {
        p->properties |= WMIconGeometry;
    }

    else if (atom == p->atom(_NET_WM_ICON)) {
        p->properties |= WMIcon;
    }

    else if (atom == p->atom(_NET_WM_PID)) {
        p->properties |= WMPid;
    }

    else if (atom == p->atom(_NET_WM_HANDLED_ICONS)) {
        p->properties |= WMHandledIcons;
    }

    else if (atom == p->atom(_NET_WM_PING)) {
        p->properties |= WMPing;
    }

    else if (atom == p->atom(_NET_WM_USER_TIME)) {
        p->properties2 |= WM2UserTime;
    }

    else if (atom == p->atom(_NET_STARTUP_ID)) {
        p->properties2 |= WM2StartupId;
    }

    else if (atom == p->atom(_NET_WM_WINDOW_OPACITY)) {
        p->properties2 |= WM2Opacity;
    }

    else if (atom == p->atom(_NET_WM_FULLSCREEN_MONITORS)) {
        p->properties2 |= WM2FullscreenMonitors;
    }

    else if (atom == p->atom(_NET_WM_ALLOWED_ACTIONS)) {
        p->properties2 |= WM2AllowedActions;
    }

    // Actions
    else if (atom == p->atom(_NET_WM_ACTION_MOVE)) {
        p->actions |= ActionMove;
    } else if (atom == p->atom(_NET_WM_ACTION_RESIZE)) {
        p->actions |= ActionResize;
    } else if (atom == p->atom(_NET_WM_ACTION_MINIMIZE)) {
        p->actions |= ActionMinimize;
    } else if (atom == p->atom(_NET_WM_ACTION_SHADE)) {
        p->actions |= ActionShade;
    } else if (atom == p->atom(_NET_WM_ACTION_STICK)) {
        p->actions |= ActionStick;
    } else if (atom == p->atom(_NET_WM_ACTION_MAXIMIZE_VERT)) {
        p->actions |= ActionMaxVert;
    } else if (atom == p->atom(_NET_WM_ACTION_MAXIMIZE_HORZ)) {
        p->actions |= ActionMaxHoriz;
    } else if (atom == p->atom(_NET_WM_ACTION_FULLSCREEN)) {
        p->actions |= ActionFullScreen;
    } else if (atom == p->atom(_NET_WM_ACTION_CHANGE_DESKTOP)) {
        p->actions |= ActionChangeDesktop;
    } else if (atom == p->atom(_NET_WM_ACTION_CLOSE)) {
        p->actions |= ActionClose;
    }

    else if (atom == p->atom(_NET_FRAME_EXTENTS)) {
        p->properties |= WMFrameExtents;
    } else if (atom == p->atom(_KDE_NET_WM_FRAME_STRUT)) {
        p->properties |= WMFrameExtents;
    } else if (atom == p->atom(_NET_WM_FRAME_OVERLAP)) {
        p->properties2 |= WM2FrameOverlap;
    }

    else if (atom == p->atom(_KDE_NET_WM_TEMPORARY_RULES)) {
        p->properties2 |= WM2KDETemporaryRules;
    } else if (atom == p->atom(_NET_WM_FULL_PLACEMENT)) {
        p->properties2 |= WM2FullPlacement;
    }

    else if (atom == p->atom(_KDE_NET_WM_ACTIVITIES)) {
        p->properties2 |= WM2Activities;
    }

    else if (atom == p->atom(_KDE_NET_WM_BLOCK_COMPOSITING) ||
             atom == p->atom(_NET_WM_BYPASS_COMPOSITOR)) {
        p->properties2 |= WM2BlockCompositing;
    }

    else if (atom == p->atom(_KDE_NET_WM_SHADOW)) {
        p->properties2 |= WM2KDEShadow;
    }

    else if (atom == p->atom(_NET_WM_OPAQUE_REGION)) {
        p->properties2 |= WM2OpaqueRegion;
    }
}

void NETRootInfo::setActiveWindow(xcb_window_t window)
{
    setActiveWindow(window, FromUnknown, QX11Info::appUserTime(), XCB_WINDOW_NONE);
}

void NETRootInfo::setActiveWindow(xcb_window_t window, NET::RequestSource src,
                                  xcb_timestamp_t timestamp, xcb_window_t active_window)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setActiveWindow(0x%lx) (%s)\n",
            window, (p->role == WindowManager) ? "WM" : "Client");
#endif

    if (p->role == WindowManager) {
        p->active = window;

        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_ACTIVE_WINDOW),
                            XCB_ATOM_WINDOW, 32, 1, (const void *) & (p->active));
    } else {
        const uint32_t data[5] = {
            src, timestamp, active_window, 0, 0
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root,
                            window, p->atom(_NET_ACTIVE_WINDOW), data);
    }
}

void NETRootInfo::setWorkArea(int desktop, const NETRect &workarea)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setWorkArea(%d, { %d, %d, %d, %d }) (%s)\n",
            desktop, workarea.pos.x, workarea.pos.y, workarea.size.width, workarea.size.height,
            (p->role == WindowManager) ? "WM" : "Client");
#endif

    if (p->role != WindowManager || desktop < 1) {
        return;
    }

    p->workarea[desktop - 1] = workarea;

    uint32_t *wa = new uint32_t[p->number_of_desktops * 4];
    int i, o;
    for (i = 0, o = 0; i < p->number_of_desktops; i++) {
        wa[o++] = p->workarea[i].pos.x;
        wa[o++] = p->workarea[i].pos.y;
        wa[o++] = p->workarea[i].size.width;
        wa[o++] = p->workarea[i].size.height;
    }

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_WORKAREA),
                        XCB_ATOM_CARDINAL, 32, p->number_of_desktops * 4,
                        (const void *) wa);

    delete [] wa;
}

void NETRootInfo::setVirtualRoots(const xcb_window_t *windows, unsigned int count)
{
    if (p->role != WindowManager) {
        return;
    }

    p->virtual_roots_count = count;
    delete[] p->virtual_roots;
    p->virtual_roots = nwindup(windows, count);

#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setVirtualRoots: setting list with %ld windows\n",
            p->virtual_roots_count);
#endif

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_VIRTUAL_ROOTS),
                        XCB_ATOM_WINDOW, 32, p->virtual_roots_count,
                        (const void *) windows);
}

void NETRootInfo::setDesktopLayout(NET::Orientation orientation, int columns, int rows,
                                   NET::DesktopLayoutCorner corner)
{
    p->desktop_layout_orientation = orientation;
    p->desktop_layout_columns = columns;
    p->desktop_layout_rows = rows;
    p->desktop_layout_corner = corner;

#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setDesktopLayout: %d %d %d %d\n",
            orientation, columns, rows, corner);
#endif

    uint32_t data[4];
    data[0] = orientation;
    data[1] = columns;
    data[2] = rows;
    data[3] = corner;

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_DESKTOP_LAYOUT),
                        XCB_ATOM_CARDINAL, 32, 4, (const void *) data);
}

void NETRootInfo::setShowingDesktop(bool showing)
{
    if (p->role == WindowManager) {
        uint32_t d = p->showing_desktop = showing;
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->root, p->atom(_NET_SHOWING_DESKTOP),
                            XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
    } else {

        uint32_t data[5] = {
            uint32_t(showing ? 1 : 0), 0, 0, 0, 0
        };
        send_client_message(p->conn, netwm_sendevent_mask, p->root, p->root, p->atom(_NET_SHOWING_DESKTOP), data);
    }
}

bool NETRootInfo::showingDesktop() const
{
    return p->showing_desktop;
}

void NETRootInfo::closeWindowRequest(xcb_window_t window)
{
#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::closeWindowRequest: requesting close for 0x%lx\n",
            window);
#endif

    const uint32_t data[5] = { 0, 0, 0, 0, 0 };
    send_client_message(p->conn, netwm_sendevent_mask, p->root, window, p->atom(_NET_CLOSE_WINDOW), data);
}

void NETRootInfo::moveResizeRequest(xcb_window_t window, int x_root, int y_root,
                                    Direction direction)
{
#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::moveResizeRequest: requesting resize/move for 0x%lx (%d, %d, %d)\n",
            window, x_root, y_root, direction);
#endif

    const uint32_t data[5] = {
        uint32_t(x_root), uint32_t(y_root), uint32_t(direction), 0, 0
    };

    send_client_message(p->conn, netwm_sendevent_mask, p->root, window, p->atom(_NET_WM_MOVERESIZE), data);
}

void NETRootInfo::moveResizeWindowRequest(xcb_window_t window, int flags, int x, int y, int width, int height)
{
#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::moveResizeWindowRequest: resizing/moving 0x%lx (%d, %d, %d, %d, %d)\n",
            window, flags, x, y, width, height);
#endif

    const uint32_t data[5] = {
        uint32_t(flags), uint32_t(x), uint32_t(y), uint32_t(width), uint32_t(height)
    };

    send_client_message(p->conn, netwm_sendevent_mask, p->root, window, p->atom(_NET_MOVERESIZE_WINDOW), data);
}

void NETRootInfo::restackRequest(xcb_window_t window, RequestSource src, xcb_window_t above, int detail, xcb_timestamp_t timestamp)
{
#ifdef NETWMDEBUG
    fprintf(stderr,
            "NETRootInfo::restackRequest: requesting restack for 0x%lx (%lx, %d)\n",
            window, above, detail);
#endif

    const uint32_t data[5] = {
        uint32_t(src), uint32_t(above), uint32_t(detail), uint32_t(timestamp), 0
    };

    send_client_message(p->conn, netwm_sendevent_mask, p->root, window, p->atom(_NET_RESTACK_WINDOW), data);
}

void NETRootInfo::sendPing(xcb_window_t window, xcb_timestamp_t timestamp)
{
    if (p->role != WindowManager) {
        return;
    }

#ifdef NETWMDEBUG
    fprintf(stderr, "NETRootInfo::setPing: window 0x%lx, timestamp %lu\n",
            window, timestamp);
#endif

    const uint32_t data[5] = {
        p->atom(_NET_WM_PING), timestamp, window, 0, 0
    };

    send_client_message(p->conn, 0, window, window, p->atom(WM_PROTOCOLS), data);
}

// assignment operator

const NETRootInfo &NETRootInfo::operator=(const NETRootInfo &rootinfo)
{

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETRootInfo::operator=()\n");
#endif

    if (p != rootinfo.p) {
        refdec_nri(p);

        if (! p->ref) {
            delete p;
        }
    }

    p = rootinfo.p;
    p->ref++;

    return *this;
}

NET::Properties NETRootInfo::event(xcb_generic_event_t *ev)
{
    NET::Properties props;
    event(ev, &props);
    return props;
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
void NETRootInfo::event(xcb_generic_event_t *ev, unsigned long *properties, int properties_size)
{
    unsigned long props[ PROPERTIES_SIZE ] = { 0, 0, 0, 0, 0 };
    assert(PROPERTIES_SIZE == 5);   // add elements above
    NET::Properties p;
    NET::Properties2 p2;
    event(ev, &p, &p2);
    props[ PROTOCOLS ]  = p;
    props[ PROTOCOLS2 ] = p2;

    if (properties_size > PROPERTIES_SIZE) {
        properties_size = PROPERTIES_SIZE;
    }
    for (int i = 0;
            i < properties_size;
            ++i) {
        properties[ i ] = props[ i ];
    }
}
#endif

void NETRootInfo::event(xcb_generic_event_t *event, NET::Properties *properties, NET::Properties2 *properties2)
{
    NET::Properties dirty;
    NET::Properties2 dirty2;
    bool do_update = false;
    const uint8_t eventType = event->response_type & ~0x80;

    // the window manager will be interested in client messages... no other
    // client should get these messages
    if (p->role == WindowManager && eventType == XCB_CLIENT_MESSAGE &&
            reinterpret_cast<xcb_client_message_event_t *>(event)->format == 32) {
        xcb_client_message_event_t *message = reinterpret_cast<xcb_client_message_event_t *>(event);
#ifdef    NETWMDEBUG
        fprintf(stderr, "NETRootInfo::event: handling ClientMessage event\n");
#endif

        if (message->type == p->atom(_NET_NUMBER_OF_DESKTOPS)) {
            dirty = NumberOfDesktops;

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: changeNumberOfDesktops(%ld)\n",
                    message->data.data32[0]);
#endif

            changeNumberOfDesktops(message->data.data32[0]);
        } else if (message->type == p->atom(_NET_DESKTOP_GEOMETRY)) {
            dirty = DesktopGeometry;

            NETSize sz;
            sz.width = message->data.data32[0];
            sz.height = message->data.data32[1];

#ifdef    NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: changeDesktopGeometry( -- , { %d, %d })\n",
                    sz.width, sz.height);
#endif

            changeDesktopGeometry(~0, sz);
        } else if (message->type == p->atom(_NET_DESKTOP_VIEWPORT)) {
            dirty = DesktopViewport;

            NETPoint pt;
            pt.x = message->data.data32[0];
            pt.y = message->data.data32[1];

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: changeDesktopViewport(%d, { %d, %d })\n",
                    p->current_desktop, pt.x, pt.y);
#endif

            changeDesktopViewport(p->current_desktop, pt);
        } else if (message->type == p->atom(_NET_CURRENT_DESKTOP)) {
            dirty = CurrentDesktop;

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: changeCurrentDesktop(%ld)\n",
                    message->data.data32[0] + 1);
#endif

            changeCurrentDesktop(message->data.data32[0] + 1);
        } else if (message->type == p->atom(_NET_ACTIVE_WINDOW)) {
            dirty = ActiveWindow;

#ifdef    NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: changeActiveWindow(0x%lx)\n",
                    message->window);
#endif

            RequestSource src = FromUnknown;
            xcb_timestamp_t timestamp = XCB_TIME_CURRENT_TIME;
            xcb_window_t active_window = XCB_WINDOW_NONE;
            // make sure there aren't unknown values
            if (message->data.data32[0] >= FromUnknown
                    && message->data.data32[0] <= FromTool) {
                src = static_cast< RequestSource >(message->data.data32[0]);
                timestamp = message->data.data32[1];
                active_window = message->data.data32[2];
            }
            changeActiveWindow(message->window, src, timestamp, active_window);
        } else if (message->type == p->atom(_NET_WM_MOVERESIZE)) {

#ifdef    NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: moveResize(%ld, %ld, %ld, %ld)\n",
                    message->window,
                    message->data.data32[0],
                    message->data.data32[1],
                    message->data.data32[2]
                   );
#endif

            moveResize(message->window,
                       message->data.data32[0],
                       message->data.data32[1],
                       message->data.data32[2]);
        } else if (message->type == p->atom(_NET_MOVERESIZE_WINDOW)) {

#ifdef    NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: moveResizeWindow(%ld, %ld, %ld, %ld, %ld, %ld)\n",
                    message->window,
                    message->data.data32[0],
                    message->data.data32[1],
                    message->data.data32[2],
                    message->data.data32[3],
                    message->data.data32[4]
                   );
#endif

            moveResizeWindow(message->window,
                             message->data.data32[0],
                             message->data.data32[1],
                             message->data.data32[2],
                             message->data.data32[3],
                             message->data.data32[4]);
        } else if (message->type == p->atom(_NET_CLOSE_WINDOW)) {

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: closeWindow(0x%lx)\n",
                    message->window);
#endif

            closeWindow(message->window);
        } else if (message->type == p->atom(_NET_RESTACK_WINDOW)) {

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: restackWindow(0x%lx)\n",
                    message->window);
#endif

            RequestSource src = FromUnknown;
            xcb_timestamp_t timestamp = XCB_TIME_CURRENT_TIME;
            // make sure there aren't unknown values
            if (message->data.data32[0] >= FromUnknown
                    && message->data.data32[0] <= FromTool) {
                src = static_cast< RequestSource >(message->data.data32[0]);
                timestamp = message->data.data32[3];
            }
            restackWindow(message->window, src,
                          message->data.data32[1], message->data.data32[2], timestamp);
        } else if (message->type == p->atom(WM_PROTOCOLS)
                   && (xcb_atom_t)message->data.data32[ 0 ] == p->atom(_NET_WM_PING)) {
            dirty = WMPing;

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: gotPing(0x%lx,%lu)\n",
                    message->window, message->data.data32[1]);
#endif
            gotPing(message->data.data32[2], message->data.data32[1]);
        } else if (message->type == p->atom(_NET_SHOWING_DESKTOP)) {
            dirty2 = WM2ShowingDesktop;

#ifdef   NETWMDEBUG
            fprintf(stderr, "NETRootInfo::event: changeShowingDesktop(%ld)\n",
                    message->data.data32[0]);
#endif

            changeShowingDesktop(message->data.data32[0]);
        }
    }

    if (eventType == XCB_PROPERTY_NOTIFY) {

#ifdef    NETWMDEBUG
        fprintf(stderr, "NETRootInfo::event: handling PropertyNotify event\n");
#endif

        xcb_property_notify_event_t *pe = reinterpret_cast<xcb_property_notify_event_t *>(event);
        if (pe->atom == p->atom(_NET_CLIENT_LIST)) {
            dirty |= ClientList;
        } else if (pe->atom == p->atom(_NET_CLIENT_LIST_STACKING)) {
            dirty |= ClientListStacking;
        } else if (pe->atom == p->atom(_NET_DESKTOP_NAMES)) {
            dirty |= DesktopNames;
        } else if (pe->atom == p->atom(_NET_WORKAREA)) {
            dirty |= WorkArea;
        } else if (pe->atom == p->atom(_NET_NUMBER_OF_DESKTOPS)) {
            dirty |= NumberOfDesktops;
        } else if (pe->atom == p->atom(_NET_DESKTOP_GEOMETRY)) {
            dirty |= DesktopGeometry;
        } else if (pe->atom == p->atom(_NET_DESKTOP_VIEWPORT)) {
            dirty |= DesktopViewport;
        } else if (pe->atom == p->atom(_NET_CURRENT_DESKTOP)) {
            dirty |= CurrentDesktop;
        } else if (pe->atom == p->atom(_NET_ACTIVE_WINDOW)) {
            dirty |= ActiveWindow;
        } else if (pe->atom == p->atom(_NET_SHOWING_DESKTOP)) {
            dirty2 |= WM2ShowingDesktop;
        } else if (pe->atom == p->atom(_NET_SUPPORTED)) {
            dirty |= Supported;    // update here?
        } else if (pe->atom == p->atom(_NET_SUPPORTING_WM_CHECK)) {
            dirty |= SupportingWMCheck;
        } else if (pe->atom == p->atom(_NET_VIRTUAL_ROOTS)) {
            dirty |= VirtualRoots;
        } else if (pe->atom == p->atom(_NET_DESKTOP_LAYOUT)) {
            dirty2 |= WM2DesktopLayout;
        }

        do_update = true;
    }

    if (do_update) {
        update(dirty, dirty2);
    }

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETRootInfo::event: handled events, returning dirty = 0x%lx, 0x%lx\n",
            dirty, dirty2);
#endif

    if (properties) {
        *properties = dirty;
    }
    if (properties2) {
        *properties2 = dirty2;
    }
}

// private functions to update the data we keep

void NETRootInfo::update(NET::Properties properties, NET::Properties2 properties2)
{
    NET::Properties dirty   = properties  & p->clientProperties;
    NET::Properties2 dirty2 = properties2 & p->clientProperties2;

    xcb_get_property_cookie_t cookies[255];
    xcb_get_property_cookie_t wm_name_cookie;
    int c = 0;

    // Send the property requests
    if (dirty & Supported) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_SUPPORTED), XCB_ATOM_ATOM, 0, MAX_PROP_SIZE);
    }

    if (dirty & ClientList) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_CLIENT_LIST), XCB_ATOM_WINDOW, 0, MAX_PROP_SIZE);
    }

    if (dirty & ClientListStacking) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_CLIENT_LIST_STACKING), XCB_ATOM_WINDOW, 0, MAX_PROP_SIZE);
    }

    if (dirty & NumberOfDesktops) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_NUMBER_OF_DESKTOPS), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty & DesktopGeometry) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_DESKTOP_GEOMETRY), XCB_ATOM_CARDINAL, 0, 2);
    }

    if (dirty & DesktopViewport) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_DESKTOP_VIEWPORT), XCB_ATOM_CARDINAL, 0, MAX_PROP_SIZE);
    }

    if (dirty & CurrentDesktop) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_CURRENT_DESKTOP), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty & DesktopNames) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_DESKTOP_NAMES), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty & ActiveWindow) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_ACTIVE_WINDOW), XCB_ATOM_WINDOW, 0, 1);
    }

    if (dirty & WorkArea) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_WORKAREA), XCB_ATOM_CARDINAL, 0, MAX_PROP_SIZE);
    }

    if (dirty & SupportingWMCheck) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_SUPPORTING_WM_CHECK), XCB_ATOM_WINDOW, 0, 1);
    }

    if (dirty & VirtualRoots) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_VIRTUAL_ROOTS), XCB_ATOM_WINDOW, 0, 1);
    }

    if (dirty2 & WM2DesktopLayout) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_DESKTOP_LAYOUT), XCB_ATOM_CARDINAL, 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2ShowingDesktop) {
        cookies[c++] = xcb_get_property(p->conn, false, p->root, p->atom(_NET_SHOWING_DESKTOP), XCB_ATOM_CARDINAL, 0, 1);
    }

    // Get the replies
    c = 0;

    if (dirty & Supported) {
        // Only in Client mode
        p->properties = NET::Properties();
        p->properties2 = NET::Properties2();
        p->windowTypes = NET::WindowTypes();
        p->states = NET::States();
        p->actions = NET::Actions();

        const QVector<xcb_atom_t> atoms = get_array_reply<xcb_atom_t>(p->conn, cookies[c++], XCB_ATOM_ATOM);
        Q_FOREACH (const xcb_atom_t atom, atoms) {
            updateSupportedProperties(atom);
        }
    }

    if (dirty & ClientList) {
        QList<xcb_window_t> clientsToRemove;
        QList<xcb_window_t> clientsToAdd;

        QVector<xcb_window_t> clients = get_array_reply<xcb_window_t>(p->conn, cookies[c++], XCB_ATOM_WINDOW);
        qSort(clients);

        if (p->clients) {
            if (p->role == Client) {
                int new_index = 0, old_index = 0;
                int old_count = p->clients_count;
                int new_count = clients.count();

                while (old_index < old_count || new_index < new_count) {
                    if (old_index == old_count) {
                        clientsToAdd.append(clients[new_index++]);
                    } else if (new_index == new_count) {
                        clientsToRemove.append(p->clients[old_index++]);
                    } else {
                        if (p->clients[old_index] < clients[new_index]) {
                            clientsToRemove.append(p->clients[old_index++]);
                        } else if (clients[new_index] <
                                   p->clients[old_index]) {
                            clientsToAdd.append(clients[new_index++]);
                        } else {
                            new_index++;
                            old_index++;
                        }
                    }
                }
            }

            delete [] p->clients;
            p->clients = nullptr;
        } else {
#ifdef NETWMDEBUG
            fprintf(stderr, "NETRootInfo::update: client list null, creating\n");
#endif

            for (int i = 0; i < clients.count(); i++) {
                clientsToAdd.append(clients[i]);
            }
        }

        if (clients.count() > 0) {
            p->clients_count = clients.count();
            p->clients = new xcb_window_t[clients.count()];
            for (int i = 0; i < clients.count(); i++) {
                p->clients[i] = clients.at(i);
            }
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: client list updated (%ld clients)\n",
                p->clients_count);
#endif

        for (int i = 0; i < clientsToRemove.size(); ++i) {
            removeClient(clientsToRemove.at(i));
        }

        for (int i = 0; i < clientsToAdd.size(); ++i) {
            addClient(clientsToAdd.at(i));
        }
    }

    if (dirty & ClientListStacking) {
        p->stacking_count = 0;

        delete[] p->stacking;
        p->stacking = nullptr;

        const QVector<xcb_window_t> wins = get_array_reply<xcb_window_t>(p->conn, cookies[c++], XCB_ATOM_WINDOW);

        if (wins.count() > 0) {
            p->stacking_count = wins.count();
            p->stacking = new xcb_window_t[wins.count()];
            for (int i = 0; i < wins.count(); i++) {
                p->stacking[i] = wins.at(i);
            }
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: client stacking updated (%ld clients)\n",
                p->stacking_count);
#endif
    }

    if (dirty & NumberOfDesktops) {
        p->number_of_desktops = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0);

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: number of desktops = %d\n",
                p->number_of_desktops);
#endif
    }

    if (dirty & DesktopGeometry) {
        p->geometry = p->rootSize;

        const QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == 2) {
            p->geometry.width  = data.at(0);
            p->geometry.height = data.at(1);
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: desktop geometry updated\n");
#endif
    }

    if (dirty & DesktopViewport) {
        for (int i = 0; i < p->viewport.size(); i++) {
            p->viewport[i].x = p->viewport[i].y = 0;
        }

        const QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);

        if (data.count() >= 2) {
            int n = data.count() / 2;
            for (int d = 0, i = 0; d < n; d++) {
                p->viewport[d].x = data[i++];
                p->viewport[d].y = data[i++];
            }

#ifdef NETWMDEBUG
            fprintf(stderr,
                    "NETRootInfo::update: desktop viewport array updated (%d entries)\n",
                    p->viewport.size());

            if (data.count() % 2 != 0) {
                fprintf(stderr,
                        "NETRootInfo::update(): desktop viewport array "
                        "size not a multiple of 2\n");
            }
#endif
        }
    }

    if (dirty & CurrentDesktop) {
        p->current_desktop = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0) + 1;

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: current desktop = %d\n",
                p->current_desktop);
#endif
    }

    if (dirty & DesktopNames) {
        for (int i = 0; i < p->desktop_names.size(); ++i) {
            delete[] p->desktop_names[i];
        }

        p->desktop_names.reset();

        const QList<QByteArray> names = get_stringlist_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        for (int i = 0; i < names.count(); i++) {
            p->desktop_names[i] = nstrndup(names[i].constData(), names[i].length());
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: desktop names array updated (%d entries)\n",
                p->desktop_names.size());
#endif
    }

    if (dirty & ActiveWindow) {
        p->active = get_value_reply<xcb_window_t>(p->conn, cookies[c++], XCB_ATOM_WINDOW, 0);

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: active window = 0x%lx\n", p->active);
#endif
    }

    if (dirty & WorkArea) {
        p->workarea.reset();

        const QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == p->number_of_desktops * 4) {
            for (int i = 0, j = 0; i < p->number_of_desktops; i++) {
                p->workarea[i].pos.x       = data[j++];
                p->workarea[i].pos.y       = data[j++];
                p->workarea[i].size.width  = data[j++];
                p->workarea[i].size.height = data[j++];
            }
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: work area array updated (%d entries)\n",
                p->workarea.size());
#endif
    }

    if (dirty & SupportingWMCheck) {
        delete[] p->name;
        p->name = nullptr;

        p->supportwindow = get_value_reply<xcb_window_t>(p->conn, cookies[c++], XCB_ATOM_WINDOW, 0);

        // We'll get the reply for this request at the bottom of this function,
        // after we've processing the other pending replies
        if (p->supportwindow)
            wm_name_cookie = xcb_get_property(p->conn, false, p->supportwindow, p->atom(_NET_WM_NAME),
                                              p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty & VirtualRoots) {
        p->virtual_roots_count = 0;

        delete[] p->virtual_roots;
        p->virtual_roots = nullptr;

        const QVector<xcb_window_t> wins = get_array_reply<xcb_window_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);

        if (wins.count() > 0) {
            p->virtual_roots_count = wins.count();
            p->virtual_roots = new xcb_window_t[wins.count()];
            for (int i = 0; i < wins.count(); i++) {
                p->virtual_roots[i] = wins.at(i);
            }
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::updated: virtual roots updated (%ld windows)\n",
                p->virtual_roots_count);
#endif
    }

    if (dirty2 & WM2DesktopLayout) {
        p->desktop_layout_orientation = OrientationHorizontal;
        p->desktop_layout_corner = DesktopLayoutCornerTopLeft;
        p->desktop_layout_columns = p->desktop_layout_rows = 0;

        const QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);

        if (data.count() >= 4 && data[3] <= 3) {
            p->desktop_layout_corner = (NET::DesktopLayoutCorner)data[3];
        }

        if (data.count() >= 3) {
            if (data[0] <= 1) {
                p->desktop_layout_orientation = (NET::Orientation)data[0];
            }

            p->desktop_layout_columns = data[1];
            p->desktop_layout_rows = data[2];
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::updated: desktop layout updated (%d %d %d %d)\n",
                p->desktop_layout_orientation, p->desktop_layout_columns,
                p->desktop_layout_rows, p->desktop_layout_corner);
#endif
    }

    if (dirty2 & WM2ShowingDesktop) {
        const uint32_t val = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0);
        p->showing_desktop = bool(val);

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: showing desktop = %d\n",
                p->showing_desktop);
#endif
    }

    if ((dirty & SupportingWMCheck) && p->supportwindow) {
        const QByteArray ba = get_string_reply(p->conn, wm_name_cookie, p->atom(UTF8_STRING));
        if (ba.length() > 0) {
            p->name = nstrndup((const char *) ba.constData(), ba.length());
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETRootInfo::update: supporting window manager = '%s'\n", p->name);
#endif
    }
}

xcb_connection_t *NETRootInfo::xcbConnection() const
{
    return p->conn;
}

xcb_window_t NETRootInfo::rootWindow() const
{
    return p->root;
}

xcb_window_t NETRootInfo::supportWindow() const
{
    return p->supportwindow;
}

const char *NETRootInfo::wmName() const
{
    return p->name;
}

NET::Properties NETRootInfo::supportedProperties() const
{
    return p->properties;
}

NET::Properties2 NETRootInfo::supportedProperties2() const
{
    return p->properties2;
}

NET::States NETRootInfo::supportedStates() const
{
    return p->states;
}

NET::WindowTypes NETRootInfo::supportedWindowTypes() const
{
    return p->windowTypes;
}

NET::Actions NETRootInfo::supportedActions() const
{
    return p->actions;
}

NET::Properties NETRootInfo::passedProperties() const
{
    return p->role == WindowManager
           ? p->properties
           : p->clientProperties;
}

NET::Properties2 NETRootInfo::passedProperties2() const
{
    return p->role == WindowManager
           ? p->properties2
           : p->clientProperties2;
}

NET::States NETRootInfo::passedStates() const
{
    return p->role == WindowManager
           ? p->states
           : NET::States();
}

NET::WindowTypes NETRootInfo::passedWindowTypes() const
{
    return p->role == WindowManager
           ? p->windowTypes
           : NET::WindowTypes();
}

NET::Actions NETRootInfo::passedActions() const
{
    return p->role == WindowManager
           ? p->actions
           : NET::Actions();
}

void NETRootInfo::setSupported(NET::Property property, bool on)
{
    if (p->role != WindowManager) {
        return;
    }

    if (on && !isSupported(property)) {
        p->properties |= property;
        setSupported();
    } else if (!on && isSupported(property)) {
        p->properties &= ~property;
        setSupported();
    }
}

void NETRootInfo::setSupported(NET::Property2 property, bool on)
{
    if (p->role != WindowManager) {
        return;
    }

    if (on && !isSupported(property)) {
        p->properties2 |= property;
        setSupported();
    } else if (!on && isSupported(property)) {
        p->properties2 &= ~property;
        setSupported();
    }
}

void NETRootInfo::setSupported(NET::WindowTypeMask property, bool on)
{
    if (p->role != WindowManager) {
        return;
    }

    if (on && !isSupported(property)) {
        p->windowTypes |= property;
        setSupported();
    } else if (!on && isSupported(property)) {
        p->windowTypes &= ~property;
        setSupported();
    }
}

void NETRootInfo::setSupported(NET::State property, bool on)
{
    if (p->role != WindowManager) {
        return;
    }

    if (on && !isSupported(property)) {
        p->states |= property;
        setSupported();
    } else if (!on && isSupported(property)) {
        p->states &= ~property;
        setSupported();
    }
}

void NETRootInfo::setSupported(NET::Action property, bool on)
{
    if (p->role != WindowManager) {
        return;
    }

    if (on && !isSupported(property)) {
        p->actions |= property;
        setSupported();
    } else if (!on && isSupported(property)) {
        p->actions &= ~property;
        setSupported();
    }
}

bool NETRootInfo::isSupported(NET::Property property) const
{
    return p->properties & property;
}

bool NETRootInfo::isSupported(NET::Property2 property) const
{
    return p->properties2 & property;
}

bool NETRootInfo::isSupported(NET::WindowTypeMask type) const
{
    return p->windowTypes & type;
}

bool NETRootInfo::isSupported(NET::State state) const
{
    return p->states & state;
}

bool NETRootInfo::isSupported(NET::Action action) const
{
    return p->actions & action;
}

const xcb_window_t *NETRootInfo::clientList() const
{
    return p->clients;
}

int NETRootInfo::clientListCount() const
{
    return p->clients_count;
}

const xcb_window_t *NETRootInfo::clientListStacking() const
{
    return p->stacking;
}

int NETRootInfo::clientListStackingCount() const
{
    return p->stacking_count;
}

NETSize NETRootInfo::desktopGeometry() const
{
    return p->geometry.width != 0 ? p->geometry : p->rootSize;
}

NETPoint NETRootInfo::desktopViewport(int desktop) const
{
    if (desktop < 1) {
        NETPoint pt; // set to (0,0)
        return pt;
    }

    return p->viewport[desktop - 1];
}

NETRect NETRootInfo::workArea(int desktop) const
{
    if (desktop < 1) {
        NETRect rt;
        return rt;
    }

    return p->workarea[desktop - 1];
}

const char *NETRootInfo::desktopName(int desktop) const
{
    if (desktop < 1) {
        return nullptr;
    }

    return p->desktop_names[desktop - 1];
}

const xcb_window_t *NETRootInfo::virtualRoots() const
{
    return p->virtual_roots;
}

int NETRootInfo::virtualRootsCount() const
{
    return p->virtual_roots_count;
}

NET::Orientation NETRootInfo::desktopLayoutOrientation() const
{
    return p->desktop_layout_orientation;
}

QSize NETRootInfo::desktopLayoutColumnsRows() const
{
    return QSize(p->desktop_layout_columns, p->desktop_layout_rows);
}

NET::DesktopLayoutCorner NETRootInfo::desktopLayoutCorner() const
{
    return p->desktop_layout_corner;
}

int NETRootInfo::numberOfDesktops(bool ignore_viewport) const
{
    if (!ignore_viewport && KWindowSystem::mapViewport()) {
        return KWindowSystem::numberOfDesktops();
    }
    return p->number_of_desktops == 0 ? 1 : p->number_of_desktops;
}

int NETRootInfo::currentDesktop(bool ignore_viewport) const
{
    if (!ignore_viewport && KWindowSystem::mapViewport()) {
        return KWindowSystem::currentDesktop();
    }
    return p->current_desktop == 0 ? 1 : p->current_desktop;
}

xcb_window_t NETRootInfo::activeWindow() const
{
    return p->active;
}

// NETWinInfo stuffs

const int NETWinInfo::OnAllDesktops = NET::OnAllDesktops;

NETWinInfo::NETWinInfo(xcb_connection_t *connection, xcb_window_t window, xcb_window_t rootWindow,
                       NET::Properties properties, NET::Properties2 properties2,
                       Role role)
{

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETWinInfo::NETWinInfo: constructing object with role '%s'\n",
            (role == WindowManager) ? "WindowManager" : "Client");
#endif

    p = new NETWinInfoPrivate;
    p->ref = 1;
    p->atoms = atomsForConnection(connection);

    p->conn = connection;
    p->window = window;
    p->root = rootWindow;
    p->mapping_state = Withdrawn;
    p->mapping_state_dirty = true;
    p->state = NET::States();
    p->types[ 0 ] = Unknown;
    p->name = (char *) nullptr;
    p->visible_name = (char *) nullptr;
    p->icon_name = (char *) nullptr;
    p->visible_icon_name = (char *) nullptr;
    p->desktop = p->pid = 0;
    p->handled_icons = false;
    p->user_time = -1U;
    p->startup_id = nullptr;
    p->transient_for = XCB_NONE;
    p->opacity = 0xffffffffU;
    p->window_group = XCB_NONE;
    p->icon_pixmap = XCB_PIXMAP_NONE;
    p->icon_mask = XCB_PIXMAP_NONE;
    p->allowed_actions = NET::Actions();
    p->has_net_support = false;
    p->class_class = (char *) nullptr;
    p->class_name = (char *) nullptr;
    p->window_role = (char *) nullptr;
    p->client_machine = (char *) nullptr;
    p->icon_sizes = nullptr;
    p->activities = (char *) nullptr;
    p->desktop_file = nullptr;
    p->blockCompositing = false;
    p->urgency = false;
    p->input = true;
    p->initialMappingState = NET::Withdrawn;
    p->protocols = NET::NoProtocol;

    // p->strut.left = p->strut.right = p->strut.top = p->strut.bottom = 0;
    // p->frame_strut.left = p->frame_strut.right = p->frame_strut.top =
    // p->frame_strut.bottom = 0;

    p->properties = properties;
    p->properties2 = properties2;

    p->icon_count = 0;

    p->role = role;

    update(p->properties, p->properties2);
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
NETWinInfo::NETWinInfo(xcb_connection_t *connection, xcb_window_t window, xcb_window_t rootWindow,
                       NET::Properties properties, Role role)
{

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETWinInfo::NETWinInfo: constructing object with role '%s'\n",
            (role == WindowManager) ? "WindowManager" : "Client");
#endif

    p = new NETWinInfoPrivate;
    p->ref = 1;
    p->atoms = atomsForConnection(connection);

    p->conn = connection;
    p->window = window;
    p->root = rootWindow;
    p->mapping_state = Withdrawn;
    p->mapping_state_dirty = true;
    p->state = NET::States();
    p->types[ 0 ] = Unknown;
    p->name = (char *) nullptr;
    p->visible_name = (char *) nullptr;
    p->icon_name = (char *) nullptr;
    p->visible_icon_name = (char *) nullptr;
    p->desktop = p->pid = 0;
    p->handled_icons = false;
    p->user_time = -1U;
    p->startup_id = nullptr;
    p->transient_for = XCB_NONE;
    p->opacity = 0xffffffffU;
    p->window_group = XCB_NONE;
    p->icon_pixmap = XCB_PIXMAP_NONE;
    p->icon_mask = XCB_PIXMAP_NONE;
    p->allowed_actions = NET::Actions();
    p->has_net_support = false;
    p->class_class = (char *) nullptr;
    p->class_name = (char *) nullptr;
    p->window_role = (char *) nullptr;
    p->client_machine = (char *) nullptr;
    p->icon_sizes = nullptr;
    p->activities = (char *) nullptr;
    p->desktop_file = nullptr;
    p->blockCompositing = false;
    p->urgency = false;
    p->input = true;
    p->initialMappingState = NET::Withdrawn;
    p->protocols = NET::NoProtocol;

    // p->strut.left = p->strut.right = p->strut.top = p->strut.bottom = 0;
    // p->frame_strut.left = p->frame_strut.right = p->frame_strut.top =
    // p->frame_strut.bottom = 0;

    p->properties = properties;
    p->properties2 = NET::Properties2();

    p->icon_count = 0;

    p->role = role;

    update(p->properties);
}
#endif

NETWinInfo::NETWinInfo(const NETWinInfo &wininfo)
{
    p = wininfo.p;
    p->ref++;
}

NETWinInfo::~NETWinInfo()
{
    refdec_nwi(p);

    if (! p->ref) {
        delete p;
    }
}

// assignment operator

const NETWinInfo &NETWinInfo::operator=(const NETWinInfo &wininfo)
{

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETWinInfo::operator=()\n");
#endif

    if (p != wininfo.p) {
        refdec_nwi(p);

        if (! p->ref) {
            delete p;
        }
    }

    p = wininfo.p;
    p->ref++;

    return *this;
}

void NETWinInfo::setIcon(NETIcon icon, bool replace)
{
    setIconInternal(p->icons, p->icon_count, p->atom(_NET_WM_ICON), icon, replace);
}

void NETWinInfo::setIconInternal(NETRArray<NETIcon> &icons, int &icon_count, xcb_atom_t property, NETIcon icon, bool replace)
{
    if (p->role != Client) {
        return;
    }

    if (replace) {
        for (int i = 0; i < icons.size(); i++) {
            delete [] icons[i].data;

            icons[i].data = nullptr;
            icons[i].size.width = 0;
            icons[i].size.height = 0;
        }

        icon_count = 0;
    }

    // assign icon
    icons[icon_count] = icon;
    icon_count++;

    // do a deep copy, we want to own the data
    NETIcon &ni = icons[icon_count - 1];
    int sz = ni.size.width * ni.size.height;
    uint32_t *d = new uint32_t[sz];
    ni.data = (unsigned char *) d;
    memcpy(d, icon.data, sz * sizeof(uint32_t));

    // compute property length
    int proplen = 0;
    for (int i = 0; i < icon_count; i++) {
        proplen += 2 + (icons[i].size.width *
                        icons[i].size.height);
    }

    uint32_t *prop = new uint32_t[proplen], *pprop = prop;
    for (int i = 0; i < icon_count; i++) {
        // copy size into property
        *pprop++ = icons[i].size.width;
        *pprop++ = icons[i].size.height;

        // copy data into property
        sz = (icons[i].size.width * icons[i].size.height);
        uint32_t *d32 = (uint32_t *) icons[i].data;
        for (int j = 0; j < sz; j++) {
            *pprop++ = *d32++;
        }
    }

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, property,
                        XCB_ATOM_CARDINAL, 32, proplen, (const void *) prop);

    delete [] prop;
    delete [] p->icon_sizes;
    p->icon_sizes = nullptr;
}

void NETWinInfo::setIconGeometry(NETRect geometry)
{
    if (p->role != Client) {
        return;
    }

    p->icon_geom = geometry;

    if (geometry.size.width == 0) { // Empty
        xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_ICON_GEOMETRY));
    } else {
        uint32_t data[4];
        data[0] = geometry.pos.x;
        data[1] = geometry.pos.y;
        data[2] = geometry.size.width;
        data[3] = geometry.size.height;

        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_ICON_GEOMETRY),
                            XCB_ATOM_CARDINAL, 32, 4, (const void *) data);
    }
}

void NETWinInfo::setExtendedStrut(const NETExtendedStrut &extended_strut)
{
    if (p->role != Client) {
        return;
    }

    p->extended_strut = extended_strut;

    uint32_t data[12];
    data[0] = extended_strut.left_width;
    data[1] = extended_strut.right_width;
    data[2] = extended_strut.top_width;
    data[3] = extended_strut.bottom_width;
    data[4] = extended_strut.left_start;
    data[5] = extended_strut.left_end;
    data[6] = extended_strut.right_start;
    data[7] = extended_strut.right_end;
    data[8] = extended_strut.top_start;
    data[9] = extended_strut.top_end;
    data[10] = extended_strut.bottom_start;
    data[11] = extended_strut.bottom_end;

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_STRUT_PARTIAL),
                        XCB_ATOM_CARDINAL, 32, 12, (const void *) data);
}

void NETWinInfo::setStrut(NETStrut strut)
{
    if (p->role != Client) {
        return;
    }

    p->strut = strut;

    uint32_t data[4];
    data[0] = strut.left;
    data[1] = strut.right;
    data[2] = strut.top;
    data[3] = strut.bottom;

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_STRUT),
                        XCB_ATOM_CARDINAL, 32, 4, (const void *) data);
}

void NETWinInfo::setFullscreenMonitors(NETFullscreenMonitors topology)
{
    if (p->role == Client) {
        const uint32_t data[5] = {
            uint32_t(topology.top), uint32_t(topology.bottom), uint32_t(topology.left), uint32_t(topology.right), 1
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root, p->window, p->atom(_NET_WM_FULLSCREEN_MONITORS), data);
    } else {
        p->fullscreen_monitors = topology;

        uint32_t data[4];
        data[0] = topology.top;
        data[1] = topology.bottom;
        data[2] = topology.left;
        data[3] = topology.right;

        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_FULLSCREEN_MONITORS),
                            XCB_ATOM_CARDINAL, 32, 4, (const void *) data);
    }
}

void NETWinInfo::setState(NET::States state, NET::States mask)
{
    if (p->mapping_state_dirty) {
        updateWMState();
    }

    // setState() needs to know the current state, so read it even if not requested
    if ((p->properties & WMState) == 0) {
        p->properties |= WMState;

        update(WMState);

        p->properties &= ~WMState;
    }

    if (p->role == Client && p->mapping_state != Withdrawn) {

#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::setState (0x%lx, 0x%lx) (Client)\n",
                state, mask);
#endif // NETWMDEBUG

        xcb_client_message_event_t event;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.sequence = 0;
        event.window = p->window;
        event.type = p->atom(_NET_WM_STATE);
        event.data.data32[3] = 0;
        event.data.data32[4] = 0;

        if ((mask & Modal) && ((p->state & Modal) != (state & Modal))) {
            event.data.data32[0] = (state & Modal) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_MODAL);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & Sticky) && ((p->state & Sticky) != (state & Sticky))) {
            event.data.data32[0] = (state & Sticky) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_STICKY);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & Max) && (((p->state & mask) & Max) != (state & Max))) {

            NET::States wishstate = (p->state & ~mask) | (state & mask);
            if (((wishstate & MaxHoriz) != (p->state & MaxHoriz))
                    && ((wishstate & MaxVert) != (p->state & MaxVert))) {
                if ((wishstate & Max) == Max) {
                    event.data.data32[0] = 1;
                    event.data.data32[1] = p->atom(_NET_WM_STATE_MAXIMIZED_HORZ);
                    event.data.data32[2] = p->atom(_NET_WM_STATE_MAXIMIZED_VERT);
                    xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
                } else if ((wishstate & Max) == 0) {
                    event.data.data32[0] = 0;
                    event.data.data32[1] = p->atom(_NET_WM_STATE_MAXIMIZED_HORZ);
                    event.data.data32[2] = p->atom(_NET_WM_STATE_MAXIMIZED_VERT);
                    xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
                } else {
                    event.data.data32[0] = (wishstate & MaxHoriz) ? 1 : 0;
                    event.data.data32[1] = p->atom(_NET_WM_STATE_MAXIMIZED_HORZ);
                    event.data.data32[2] = 0;
                    xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);

                    event.data.data32[0] = (wishstate & MaxVert) ? 1 : 0;
                    event.data.data32[1] = p->atom(_NET_WM_STATE_MAXIMIZED_VERT);
                    event.data.data32[2] = 0;
                    xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
                }
            } else if ((wishstate & MaxVert) != (p->state & MaxVert)) {
                event.data.data32[0] = (wishstate & MaxVert) ? 1 : 0;
                event.data.data32[1] = p->atom(_NET_WM_STATE_MAXIMIZED_VERT);
                event.data.data32[2] = 0;

                xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
            } else if ((wishstate & MaxHoriz) != (p->state & MaxHoriz)) {
                event.data.data32[0] = (wishstate & MaxHoriz) ? 1 : 0;
                event.data.data32[1] = p->atom(_NET_WM_STATE_MAXIMIZED_HORZ);
                event.data.data32[2] = 0;

                xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
            }
        }

        if ((mask & Shaded) && ((p->state & Shaded) != (state & Shaded))) {
            event.data.data32[0] = (state & Shaded) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_SHADED);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & SkipTaskbar) && ((p->state & SkipTaskbar) != (state & SkipTaskbar))) {
            event.data.data32[0] = (state & SkipTaskbar) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_SKIP_TASKBAR);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & SkipPager) && ((p->state & SkipPager) != (state & SkipPager))) {
            event.data.data32[0] = (state & SkipPager) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_SKIP_PAGER);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & SkipSwitcher) && ((p->state & SkipSwitcher) != (state & SkipSwitcher))) {
            event.data.data32[0] = (state & SkipSwitcher) ? 1 : 0;
            event.data.data32[1] = p->atom(_KDE_NET_WM_STATE_SKIP_SWITCHER);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & Hidden) && ((p->state & Hidden) != (state & Hidden))) {
            event.data.data32[0] = (state & Hidden) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_HIDDEN);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & FullScreen) && ((p->state & FullScreen) != (state & FullScreen))) {
            event.data.data32[0] = (state & FullScreen) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_FULLSCREEN);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & KeepAbove) && ((p->state & KeepAbove) != (state & KeepAbove))) {
            event.data.data32[0] = (state & KeepAbove) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_ABOVE);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & KeepBelow) && ((p->state & KeepBelow) != (state & KeepBelow))) {
            event.data.data32[0] = (state & KeepBelow) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_BELOW);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & StaysOnTop) && ((p->state & StaysOnTop) != (state & StaysOnTop))) {
            event.data.data32[0] = (state & StaysOnTop) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_STAYS_ON_TOP);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

        if ((mask & DemandsAttention) && ((p->state & DemandsAttention) != (state & DemandsAttention))) {
            event.data.data32[0] = (state & DemandsAttention) ? 1 : 0;
            event.data.data32[1] = p->atom(_NET_WM_STATE_DEMANDS_ATTENTION);
            event.data.data32[2] = 0l;

            xcb_send_event(p->conn, false, p->root, netwm_sendevent_mask, (const char *) &event);
        }

    } else {
        p->state &= ~mask;
        p->state |= state;

        uint32_t data[50];
        int count = 0;

        // Hints
        if (p->state & Modal) {
            data[count++] = p->atom(_NET_WM_STATE_MODAL);
        }
        if (p->state & MaxVert) {
            data[count++] = p->atom(_NET_WM_STATE_MAXIMIZED_VERT);
        }
        if (p->state & MaxHoriz) {
            data[count++] = p->atom(_NET_WM_STATE_MAXIMIZED_HORZ);
        }
        if (p->state & Shaded) {
            data[count++] = p->atom(_NET_WM_STATE_SHADED);
        }
        if (p->state & Hidden) {
            data[count++] = p->atom(_NET_WM_STATE_HIDDEN);
        }
        if (p->state & FullScreen) {
            data[count++] = p->atom(_NET_WM_STATE_FULLSCREEN);
        }
        if (p->state & DemandsAttention) {
            data[count++] = p->atom(_NET_WM_STATE_DEMANDS_ATTENTION);
        }

        // Policy
        if (p->state & KeepAbove) {
            data[count++] = p->atom(_NET_WM_STATE_ABOVE);
        }
        if (p->state & KeepBelow) {
            data[count++] = p->atom(_NET_WM_STATE_BELOW);
        }
        if (p->state & StaysOnTop) {
            data[count++] = p->atom(_NET_WM_STATE_STAYS_ON_TOP);
        }
        if (p->state & Sticky) {
            data[count++] = p->atom(_NET_WM_STATE_STICKY);
        }
        if (p->state & SkipTaskbar) {
            data[count++] = p->atom(_NET_WM_STATE_SKIP_TASKBAR);
        }
        if (p->state & SkipPager) {
            data[count++] = p->atom(_NET_WM_STATE_SKIP_PAGER);
        }
        if (p->state & SkipSwitcher) {
            data[count++] = p->atom(_KDE_NET_WM_STATE_SKIP_SWITCHER);
        }

#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::setState: setting state property (%d)\n", count);
        for (int i = 0; i < count; i++) {
            const QByteArray ba = get_atom_name(p->conn, data[i]);
            fprintf(stderr, "NETWinInfo::setState:   state %ld '%s'\n",
                    data[i], ba.constData());
        }
#endif

        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_STATE),
                            XCB_ATOM_ATOM, 32, count, (const void *) data);
    }
}

void NETWinInfo::setWindowType(WindowType type)
{
    if (p->role != Client) {
        return;
    }

    int len;
    uint32_t data[2];

    switch (type) {
    case Override:
        // spec extension: override window type.  we must comply with the spec
        // and provide a fall back (normal seems best)
        data[0] = p->atom(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_NORMAL);
        len = 2;
        break;

    case  Dialog:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_DIALOG);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case Menu:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_MENU);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case TopMenu:
        // spec extension: override window type.  we must comply with the spec
        // and provide a fall back (dock seems best)
        data[0] = p->atom(_KDE_NET_WM_WINDOW_TYPE_TOPMENU);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_DOCK);
        len = 2;
        break;

    case Toolbar:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_TOOLBAR);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case Dock:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_DOCK);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case Desktop:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_DESKTOP);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case Utility:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_UTILITY);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_DIALOG); // fallback for old netwm version
        len = 2;
        break;

    case Splash:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_SPLASH);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_DOCK); // fallback (dock seems best)
        len = 2;
        break;

    case DropdownMenu:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_MENU); // fallback (tearoff seems to be the best)
        len = 1;
        break;

    case PopupMenu:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_POPUP_MENU);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_MENU); // fallback (tearoff seems to be the best)
        len = 1;
        break;

    case Tooltip:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_TOOLTIP);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case Notification:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_NOTIFICATION);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_UTILITY); // fallback (utility seems to be the best)
        len = 1;
        break;

    case ComboBox:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_COMBOBOX);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case DNDIcon:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_DND);
        data[1] = XCB_NONE;
        len = 1;
        break;

    case OnScreenDisplay:
        data[0] = p->atom(_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY);
        data[1] = p->atom(_NET_WM_WINDOW_TYPE_NOTIFICATION);
        len = 1;
        break;

    default:
    case Normal:
        data[0] = p->atom(_NET_WM_WINDOW_TYPE_NORMAL);
        data[1] = XCB_NONE;
        len = 1;
        break;
    }

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_WINDOW_TYPE),
                        XCB_ATOM_ATOM, 32, len, (const void *) &data);
}

void NETWinInfo::setName(const char *name)
{
    if (p->role != Client) {
        return;
    }

    delete [] p->name;
    p->name = nstrdup(name);

    if (p->name[0] != '\0')
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_NAME),
                            p->atom(UTF8_STRING), 8, strlen(p->name), (const void *) p->name);
    else {
        xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_NAME));
    }
}

void NETWinInfo::setVisibleName(const char *visibleName)
{
    if (p->role != WindowManager) {
        return;
    }

    delete [] p->visible_name;
    p->visible_name = nstrdup(visibleName);

    if (p->visible_name[0] != '\0')
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_VISIBLE_NAME),
                            p->atom(UTF8_STRING), 8, strlen(p->visible_name),
                            (const void *) p->visible_name);
    else {
        xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_VISIBLE_NAME));
    }
}

void NETWinInfo::setIconName(const char *iconName)
{
    if (p->role != Client) {
        return;
    }

    delete [] p->icon_name;
    p->icon_name = nstrdup(iconName);

    if (p->icon_name[0] != '\0')
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_ICON_NAME),
                            p->atom(UTF8_STRING), 8, strlen(p->icon_name),
                            (const void *) p->icon_name);
    else {
        xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_ICON_NAME));
    }
}

void NETWinInfo::setVisibleIconName(const char *visibleIconName)
{
    if (p->role != WindowManager) {
        return;
    }

    delete [] p->visible_icon_name;
    p->visible_icon_name = nstrdup(visibleIconName);

    if (p->visible_icon_name[0] != '\0')
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_VISIBLE_ICON_NAME),
                            p->atom(UTF8_STRING), 8, strlen(p->visible_icon_name),
                            (const void *) p->visible_icon_name);
    else {
        xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_VISIBLE_ICON_NAME));
    }
}

void NETWinInfo::setDesktop(int desktop, bool ignore_viewport)
{
    if (p->mapping_state_dirty) {
        updateWMState();
    }

    if (p->role == Client && p->mapping_state != Withdrawn) {
        // We only send a ClientMessage if we are 1) a client and 2) managed

        if (desktop == 0) {
            return;    // We can't do that while being managed
        }

        if (!ignore_viewport && KWindowSystem::mapViewport()) {
            KWindowSystem::setOnDesktop(p->window, desktop);
            return;
        }

        const uint32_t data[5] = {
            desktop == OnAllDesktops ? 0xffffffff : desktop - 1, 0, 0, 0, 0
        };

        send_client_message(p->conn, netwm_sendevent_mask, p->root, p->window, p->atom(_NET_WM_DESKTOP), data);
    } else {
        // Otherwise we just set or remove the property directly
        p->desktop = desktop;

        if (desktop == 0) {
            xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_DESKTOP));
        } else {
            uint32_t d = (desktop == OnAllDesktops ? 0xffffffff : desktop - 1);
            xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_DESKTOP),
                                XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
        }
    }
}

void NETWinInfo::setPid(int pid)
{
    if (p->role != Client) {
        return;
    }

    p->pid = pid;
    uint32_t d = pid;
    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_PID),
                        XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
}

void NETWinInfo::setHandledIcons(bool handled)
{
    if (p->role != Client) {
        return;
    }

    p->handled_icons = handled;
    uint32_t d = handled;
    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_HANDLED_ICONS),
                        XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
}

void NETWinInfo::setStartupId(const char *id)
{
    if (p->role != Client) {
        return;
    }

    delete[] p->startup_id;
    p->startup_id = nstrdup(id);

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_STARTUP_ID),
                        p->atom(UTF8_STRING), 8, strlen(p->startup_id),
                        (const void *) p->startup_id);
}

void NETWinInfo::setOpacity(unsigned long opacity)
{
//    if (p->role != Client) return;

    p->opacity = opacity;
    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_WINDOW_OPACITY),
                        XCB_ATOM_CARDINAL, 32, 1, (const void *) &p->opacity);
}

void NETWinInfo::setAllowedActions(NET::Actions actions)
{
    if (p->role != WindowManager) {
        return;
    }

    uint32_t data[50];
    int count = 0;

    p->allowed_actions = actions;
    if (p->allowed_actions & ActionMove) {
        data[count++] = p->atom(_NET_WM_ACTION_MOVE);
    }
    if (p->allowed_actions & ActionResize) {
        data[count++] = p->atom(_NET_WM_ACTION_RESIZE);
    }
    if (p->allowed_actions & ActionMinimize) {
        data[count++] = p->atom(_NET_WM_ACTION_MINIMIZE);
    }
    if (p->allowed_actions & ActionShade) {
        data[count++] = p->atom(_NET_WM_ACTION_SHADE);
    }
    if (p->allowed_actions & ActionStick) {
        data[count++] = p->atom(_NET_WM_ACTION_STICK);
    }
    if (p->allowed_actions & ActionMaxVert) {
        data[count++] = p->atom(_NET_WM_ACTION_MAXIMIZE_VERT);
    }
    if (p->allowed_actions & ActionMaxHoriz) {
        data[count++] = p->atom(_NET_WM_ACTION_MAXIMIZE_HORZ);
    }
    if (p->allowed_actions & ActionFullScreen) {
        data[count++] = p->atom(_NET_WM_ACTION_FULLSCREEN);
    }
    if (p->allowed_actions & ActionChangeDesktop) {
        data[count++] = p->atom(_NET_WM_ACTION_CHANGE_DESKTOP);
    }
    if (p->allowed_actions & ActionClose) {
        data[count++] = p->atom(_NET_WM_ACTION_CLOSE);
    }

#ifdef NETWMDEBUG
    fprintf(stderr, "NETWinInfo::setAllowedActions: setting property (%d)\n", count);
    for (int i = 0; i < count; i++) {
        const QByteArray ba = get_atom_name(p->conn, data[i]);
        fprintf(stderr, "NETWinInfo::setAllowedActions:   action %ld '%s'\n",
                data[i], ba.constData());
    }
#endif

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_ALLOWED_ACTIONS),
                        XCB_ATOM_ATOM, 32, count, (const void *) data);
}

void NETWinInfo::setFrameExtents(NETStrut strut)
{
    if (p->role != WindowManager) {
        return;
    }

    p->frame_strut = strut;

    uint32_t d[4];
    d[0] = strut.left;
    d[1] = strut.right;
    d[2] = strut.top;
    d[3] = strut.bottom;

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_FRAME_EXTENTS),
                        XCB_ATOM_CARDINAL, 32, 4, (const void *) d);
    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_KDE_NET_WM_FRAME_STRUT),
                        XCB_ATOM_CARDINAL, 32, 4, (const void *) d);
}

NETStrut NETWinInfo::frameExtents() const
{
    return p->frame_strut;
}

void NETWinInfo::setFrameOverlap(NETStrut strut)
{
    if (strut.left != -1 || strut.top != -1 || strut.right != -1 || strut.bottom != -1) {
        strut.left   = qMax(0, strut.left);
        strut.top    = qMax(0, strut.top);
        strut.right  = qMax(0, strut.right);
        strut.bottom = qMax(0, strut.bottom);
    }

    p->frame_overlap = strut;

    uint32_t d[4];
    d[0] = strut.left;
    d[1] = strut.right;
    d[2] = strut.top;
    d[3] = strut.bottom;

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_FRAME_OVERLAP),
                        XCB_ATOM_CARDINAL, 32, 4, (const void *) d);
}

NETStrut NETWinInfo::frameOverlap() const
{
    return p->frame_overlap;
}

void NETWinInfo::kdeGeometry(NETRect &frame, NETRect &window)
{
    if (p->win_geom.size.width == 0 || p->win_geom.size.height == 0) {
        const xcb_get_geometry_cookie_t geometry_cookie
            = xcb_get_geometry(p->conn, p->window);

        const xcb_translate_coordinates_cookie_t translate_cookie
            = xcb_translate_coordinates(p->conn, p->window, p->root, 0, 0);

        xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(p->conn, geometry_cookie, nullptr);
        xcb_translate_coordinates_reply_t *translated
            = xcb_translate_coordinates_reply(p->conn, translate_cookie, nullptr);

        if (geometry && translated) {
            p->win_geom.pos.x = translated->dst_x;
            p->win_geom.pos.y = translated->dst_y;

            p->win_geom.size.width  = geometry->width;
            p->win_geom.size.height = geometry->height;
        }

        if (geometry) {
            free(geometry);
        }

        if (translated) {
            free(translated);
        }
    }

    // TODO try to work also without _NET_WM_FRAME_EXTENTS
    window = p->win_geom;

    frame.pos.x = window.pos.x - p->frame_strut.left;
    frame.pos.y = window.pos.y - p->frame_strut.top;
    frame.size.width = window.size.width + p->frame_strut.left + p->frame_strut.right;
    frame.size.height = window.size.height + p->frame_strut.top + p->frame_strut.bottom;
}

NETIcon NETWinInfo::icon(int width, int height) const
{
    return iconInternal(p->icons, p->icon_count, width, height);
}

const int *NETWinInfo::iconSizes() const
{
    if (p->icon_sizes == nullptr) {
        p->icon_sizes = new int[ p->icon_count * 2 + 2 ];
        for (int i = 0;
                i < p->icon_count;
                ++i) {
            p->icon_sizes[ i * 2 ] = p->icons[ i ].size.width;
            p->icon_sizes[ i * 2 + 1 ] = p->icons[ i ].size.height;
        }
        p->icon_sizes[ p->icon_count * 2 ] = 0; // terminator
        p->icon_sizes[ p->icon_count * 2 + 1 ] = 0;
    }
    return p->icon_sizes;
}

NETIcon NETWinInfo::iconInternal(NETRArray<NETIcon> &icons, int icon_count, int width, int height) const
{
    NETIcon result;

    if (!icon_count) {
        result.size.width = 0;
        result.size.height = 0;
        result.data = nullptr;
        return result;
    }

    // find the largest icon
    result = icons[0];
    for (int i = 1; i < icons.size(); i++) {
        if (icons[i].size.width >= result.size.width &&
                icons[i].size.height >= result.size.height) {
            result = icons[i];
        }
    }

    // return the largest icon if w and h are -1
    if (width == -1 && height == -1) {
        return result;
    }

    // find the icon that's closest in size to w x h...
    for (int i = 0; i < icons.size(); i++) {
        if ((icons[i].size.width >= width &&
                icons[i].size.width < result.size.width) &&
                (icons[i].size.height >= height &&
                 icons[i].size.height < result.size.height)) {
            result = icons[i];
        }
    }

    return result;
}

void NETWinInfo::setUserTime(xcb_timestamp_t time)
{
    if (p->role != Client) {
        return;
    }

    p->user_time = time;
    uint32_t d = time;

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_USER_TIME),
                        XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
}

NET::Properties NETWinInfo::event(xcb_generic_event_t *ev)
{
    NET::Properties properties;
    event(ev, &properties);
    return properties;
}

void NETWinInfo::event(xcb_generic_event_t *event, NET::Properties *properties, NET::Properties2 *properties2)
{
    NET::Properties dirty;
    NET::Properties2 dirty2;
    bool do_update = false;
    const uint8_t eventType = event->response_type & ~0x80;

    if (p->role == WindowManager && eventType == XCB_CLIENT_MESSAGE &&
            reinterpret_cast<xcb_client_message_event_t *>(event)->format == 32) {
        xcb_client_message_event_t *message = reinterpret_cast<xcb_client_message_event_t *>(event);
#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::event: handling ClientMessage event\n");
#endif // NETWMDEBUG

        if (message->type == p->atom(_NET_WM_STATE)) {
            dirty = WMState;

            // we need to generate a change mask

#ifdef NETWMDEBUG
            fprintf(stderr,
                    "NETWinInfo::event: state client message, getting new state/mask\n");
#endif

            int i;
            NET::States state = NET::States(), mask = NET::States();

            for (i = 1; i < 3; i++) {
#ifdef NETWMDEBUG
                const QByteArray ba = get_atom_name(p->conn, (xcb_atom_t) message->data.data32[i]);
                fprintf(stderr, "NETWinInfo::event:  message %ld '%s'\n",
                        message->data.data32[i], ba.constData());
#endif

                if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_MODAL)) {
                    mask |= Modal;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_STICKY)) {
                    mask |= Sticky;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_MAXIMIZED_VERT)) {
                    mask |= MaxVert;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_MAXIMIZED_HORZ)) {
                    mask |= MaxHoriz;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_SHADED)) {
                    mask |= Shaded;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_SKIP_TASKBAR)) {
                    mask |= SkipTaskbar;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_SKIP_PAGER)) {
                    mask |= SkipPager;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_KDE_NET_WM_STATE_SKIP_SWITCHER)) {
                    mask |= SkipSwitcher;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_HIDDEN)) {
                    mask |= Hidden;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_FULLSCREEN)) {
                    mask |= FullScreen;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_ABOVE)) {
                    mask |= KeepAbove;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_BELOW)) {
                    mask |= KeepBelow;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_DEMANDS_ATTENTION)) {
                    mask |= DemandsAttention;
                } else if ((xcb_atom_t) message->data.data32[i] == p->atom(_NET_WM_STATE_STAYS_ON_TOP)) {
                    mask |= StaysOnTop;
                }
            }

            // when removing, we just leave newstate == 0
            switch (message->data.data32[0]) {
            case 1: // set
                // to set... the change state should be the same as the mask
                state = mask;
                break;

            case 2: // toggle
                // to toggle, we need to xor the current state with the new state
                state = (p->state & mask) ^ mask;
                break;

            default:
                // to clear state, the new state should stay zero
                ;
            }

#ifdef NETWMDEBUG
            fprintf(stderr, "NETWinInfo::event: calling changeState(%lx, %lx)\n",
                    state, mask);
#endif

            changeState(state, mask);
        } else if (message->type == p->atom(_NET_WM_DESKTOP)) {
            dirty = WMDesktop;

            if (message->data.data32[0] == OnAllDesktops) {
                changeDesktop(OnAllDesktops);
            } else {
                changeDesktop(message->data.data32[0] + 1);
            }
        } else if (message->type == p->atom(_NET_WM_FULLSCREEN_MONITORS)) {
            dirty2 = WM2FullscreenMonitors;

            NETFullscreenMonitors topology;
            topology.top =  message->data.data32[0];
            topology.bottom =  message->data.data32[1];
            topology.left =  message->data.data32[2];
            topology.right =  message->data.data32[3];

#ifdef    NETWMDEBUG
            fprintf(stderr, "NETWinInfo2::event: calling changeFullscreenMonitors"
                    "(%ld, %ld, %ld, %ld, %ld)\n",
                    message->window,
                    message->data.data32[0],
                    message->data.data32[1],
                    message->data.data32[2],
                    message->data.data32[3]
                   );
#endif
            changeFullscreenMonitors(topology);
        }
    }

    if (eventType == XCB_PROPERTY_NOTIFY) {

#ifdef    NETWMDEBUG
        fprintf(stderr, "NETWinInfo::event: handling PropertyNotify event\n");
#endif

        xcb_property_notify_event_t *pe = reinterpret_cast<xcb_property_notify_event_t *>(event);

        if (pe->atom == p->atom(_NET_WM_NAME)) {
            dirty |= WMName;
        } else if (pe->atom == p->atom(_NET_WM_VISIBLE_NAME)) {
            dirty |= WMVisibleName;
        } else if (pe->atom == p->atom(_NET_WM_DESKTOP)) {
            dirty |= WMDesktop;
        } else if (pe->atom == p->atom(_NET_WM_WINDOW_TYPE)) {
            dirty |= WMWindowType;
        } else if (pe->atom == p->atom(_NET_WM_STATE)) {
            dirty |= WMState;
        } else if (pe->atom == p->atom(_NET_WM_STRUT)) {
            dirty |= WMStrut;
        } else if (pe->atom == p->atom(_NET_WM_STRUT_PARTIAL)) {
            dirty2 |= WM2ExtendedStrut;
        } else if (pe->atom == p->atom(_NET_WM_ICON_GEOMETRY)) {
            dirty |= WMIconGeometry;
        } else if (pe->atom == p->atom(_NET_WM_ICON)) {
            dirty |= WMIcon;
        } else if (pe->atom == p->atom(_NET_WM_PID)) {
            dirty |= WMPid;
        } else if (pe->atom == p->atom(_NET_WM_HANDLED_ICONS)) {
            dirty |= WMHandledIcons;
        } else if (pe->atom == p->atom(_NET_STARTUP_ID)) {
            dirty2 |= WM2StartupId;
        } else if (pe->atom == p->atom(_NET_WM_WINDOW_OPACITY)) {
            dirty2 |= WM2Opacity;
        } else if (pe->atom == p->atom(_NET_WM_ALLOWED_ACTIONS)) {
            dirty2 |= WM2AllowedActions;
        } else if (pe->atom == p->atom(WM_STATE)) {
            dirty |= XAWMState;
        } else if (pe->atom == p->atom(_NET_FRAME_EXTENTS)) {
            dirty |= WMFrameExtents;
        } else if (pe->atom == p->atom(_KDE_NET_WM_FRAME_STRUT)) {
            dirty |= WMFrameExtents;
        } else if (pe->atom == p->atom(_NET_WM_FRAME_OVERLAP)) {
            dirty2 |= WM2FrameOverlap;
        } else if (pe->atom == p->atom(_NET_WM_ICON_NAME)) {
            dirty |= WMIconName;
        } else if (pe->atom == p->atom(_NET_WM_VISIBLE_ICON_NAME)) {
            dirty |= WMVisibleIconName;
        } else if (pe->atom == p->atom(_NET_WM_USER_TIME)) {
            dirty2 |= WM2UserTime;
        } else if (pe->atom == XCB_ATOM_WM_HINTS) {
            dirty2 |= WM2GroupLeader;
            dirty2 |= WM2Urgency;
            dirty2 |= WM2Input;
            dirty2 |= WM2InitialMappingState;
            dirty2 |= WM2IconPixmap;
        } else if (pe->atom == XCB_ATOM_WM_TRANSIENT_FOR) {
            dirty2 |= WM2TransientFor;
        } else if (pe->atom == XCB_ATOM_WM_CLASS) {
            dirty2 |= WM2WindowClass;
        } else if (pe->atom == p->atom(WM_WINDOW_ROLE)) {
            dirty2 |= WM2WindowRole;
        } else if (pe->atom == XCB_ATOM_WM_CLIENT_MACHINE) {
            dirty2 |= WM2ClientMachine;
        } else if (pe->atom == p->atom(_KDE_NET_WM_ACTIVITIES)) {
            dirty2 |= WM2Activities;
        } else if (pe->atom == p->atom(_KDE_NET_WM_BLOCK_COMPOSITING) ||
                   pe->atom == p->atom(_NET_WM_BYPASS_COMPOSITOR)) {
            dirty2 |= WM2BlockCompositing;
        } else if (pe->atom == p->atom(_KDE_NET_WM_SHADOW)) {
            dirty2 |= WM2KDEShadow;
        } else if (pe->atom == p->atom(WM_PROTOCOLS)) {
            dirty2 |= WM2Protocols;
        } else if (pe->atom == p->atom(_NET_WM_OPAQUE_REGION)) {
            dirty2 |= WM2OpaqueRegion;
        } else if (pe->atom == p->atom(_KDE_NET_WM_DESKTOP_FILE)) {
            dirty2 = WM2DesktopFileName;
        } else if (pe->atom == p->atom(_NET_WM_FULLSCREEN_MONITORS)) {
            dirty2 = WM2FullscreenMonitors;
        }

        do_update = true;
    } else if (eventType == XCB_CONFIGURE_NOTIFY) {

#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::event: handling ConfigureNotify event\n");
#endif

        dirty |= WMGeometry;

        // update window geometry
        xcb_configure_notify_event_t *configure = reinterpret_cast<xcb_configure_notify_event_t *>(event);
        p->win_geom.pos.x = configure->x;
        p->win_geom.pos.y = configure->y;
        p->win_geom.size.width = configure->width;
        p->win_geom.size.height = configure->height;
    }

    if (do_update) {
        update(dirty, dirty2);
    }

    if (properties) {
        *properties = dirty;
    }
    if (properties2) {
        *properties2 = dirty2;
    }
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
void NETWinInfo::event(xcb_generic_event_t *ev, unsigned long *properties, int properties_size)
{
    NET::Properties p;
    NET::Properties2 p2;
    event(ev, &p, &p2);
    unsigned long props[ PROPERTIES_SIZE ] = { p, p2 };
    assert(PROPERTIES_SIZE == 2);   // add elements above

    if (properties_size > PROPERTIES_SIZE) {
        properties_size = PROPERTIES_SIZE;
    }
    for (int i = 0;
            i < properties_size;
            ++i) {
        properties[ i ] = props[ i ];
    }
}
#endif

void NETWinInfo::updateWMState()
{
    update(XAWMState);
}

void NETWinInfo::update(NET::Properties dirtyProperties, NET::Properties2 dirtyProperties2)
{
    Properties dirty = dirtyProperties & p->properties;
    Properties2 dirty2 = dirtyProperties2 & p->properties2;

    // We *always* want to update WM_STATE if set in dirty_props
    if (dirtyProperties & XAWMState) {
        dirty |= XAWMState;
    }

    xcb_get_property_cookie_t cookies[255];
    int c = 0;

    if (dirty & XAWMState) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(WM_STATE), p->atom(WM_STATE), 0, 1);
    }

    if (dirty & WMState) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_STATE), XCB_ATOM_ATOM, 0, 2048);
    }

    if (dirty & WMDesktop) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_DESKTOP), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty & WMName) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_NAME), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty & WMVisibleName) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_VISIBLE_NAME), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty & WMIconName) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_ICON_NAME), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty & WMVisibleIconName) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_VISIBLE_ICON_NAME), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty & WMWindowType) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_WINDOW_TYPE), XCB_ATOM_ATOM, 0, 2048);
    }

    if (dirty & WMStrut) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_STRUT), XCB_ATOM_CARDINAL, 0, 4);
    }

    if (dirty2 & WM2ExtendedStrut) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_STRUT_PARTIAL), XCB_ATOM_CARDINAL, 0, 12);
    }

    if (dirty2 & WM2FullscreenMonitors) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_FULLSCREEN_MONITORS), XCB_ATOM_CARDINAL, 0, 4);
    }

    if (dirty & WMIconGeometry) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_ICON_GEOMETRY), XCB_ATOM_CARDINAL, 0, 4);
    }

    if (dirty & WMIcon) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_ICON), XCB_ATOM_CARDINAL, 0, 0xffffffff);
    }

    if (dirty & WMFrameExtents) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_FRAME_EXTENTS), XCB_ATOM_CARDINAL, 0, 4);
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_KDE_NET_WM_FRAME_STRUT), XCB_ATOM_CARDINAL, 0, 4);
    }

    if (dirty2 & WM2FrameOverlap) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_FRAME_OVERLAP), XCB_ATOM_CARDINAL, 0, 4);
    }

    if (dirty2 & WM2Activities) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_KDE_NET_WM_ACTIVITIES), XCB_ATOM_STRING, 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2BlockCompositing) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_KDE_NET_WM_BLOCK_COMPOSITING), XCB_ATOM_CARDINAL, 0, 1);
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_BYPASS_COMPOSITOR), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty & WMPid) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_PID), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty2 & WM2StartupId) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_STARTUP_ID), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2Opacity) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_WINDOW_OPACITY), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty2 & WM2AllowedActions) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_ALLOWED_ACTIONS), XCB_ATOM_ATOM, 0, 2048);
    }

    if (dirty2 & WM2UserTime) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_USER_TIME), XCB_ATOM_CARDINAL, 0, 1);
    }

    if (dirty2 & WM2TransientFor) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
    }

    if (dirty2 & (WM2GroupLeader | WM2Urgency | WM2Input | WM2InitialMappingState | WM2IconPixmap)) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 0, 9);
    }

    if (dirty2 & WM2WindowClass) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2WindowRole) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(WM_WINDOW_ROLE), XCB_ATOM_STRING, 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2ClientMachine) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, XCB_ATOM_WM_CLIENT_MACHINE, XCB_ATOM_STRING, 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2Protocols) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(WM_PROTOCOLS), XCB_ATOM_ATOM, 0, 2048);
    }

    if (dirty2 & WM2OpaqueRegion) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_NET_WM_OPAQUE_REGION), XCB_ATOM_CARDINAL, 0, MAX_PROP_SIZE);
    }

    if (dirty2 & WM2DesktopFileName) {
        cookies[c++] = xcb_get_property(p->conn, false, p->window, p->atom(_KDE_NET_WM_DESKTOP_FILE), p->atom(UTF8_STRING), 0, MAX_PROP_SIZE);
    }

    c = 0;

    if (dirty & XAWMState) {
        p->mapping_state = Withdrawn;

        bool success;
        uint32_t state = get_value_reply<uint32_t>(p->conn, cookies[c++], p->atom(WM_STATE), 0, &success);

        if (success) {
            switch (state) {
            case 3: // IconicState
                p->mapping_state = Iconic;
                break;

            case 1: // NormalState
                p->mapping_state = Visible;
                break;

            case 0: // WithdrawnState
            default:
                p->mapping_state = Withdrawn;
                break;
            }

            p->mapping_state_dirty = false;
        }
    }

    if (dirty & WMState) {
        p->state = NET::States();
        const QVector<xcb_atom_t> states = get_array_reply<xcb_atom_t>(p->conn, cookies[c++], XCB_ATOM_ATOM);

#ifdef NETWMDEBUG
        fprintf(stderr, "NETWinInfo::update: updating window state (%ld)\n", states.count());
#endif

        Q_FOREACH (const xcb_atom_t state, states) {
#ifdef NETWMDEBUG
            const QByteArray ba = get_atom_name(p->conn, state);
            fprintf(stderr, "NETWinInfo::update:   adding window state %ld '%s'\n",
                    state, ba.constData());
#endif
            if (state == p->atom(_NET_WM_STATE_MODAL)) {
                p->state |= Modal;
            }

            else if (state == p->atom(_NET_WM_STATE_STICKY)) {
                p->state |= Sticky;
            }

            else if (state == p->atom(_NET_WM_STATE_MAXIMIZED_VERT)) {
                p->state |= MaxVert;
            }

            else if (state == p->atom(_NET_WM_STATE_MAXIMIZED_HORZ)) {
                p->state |= MaxHoriz;
            }

            else if (state == p->atom(_NET_WM_STATE_SHADED)) {
                p->state |= Shaded;
            }

            else if (state == p->atom(_NET_WM_STATE_SKIP_TASKBAR)) {
                p->state |= SkipTaskbar;
            }

            else if (state == p->atom(_NET_WM_STATE_SKIP_PAGER)) {
                p->state |= SkipPager;
            }

            else if (state == p->atom(_KDE_NET_WM_STATE_SKIP_SWITCHER)) {
                p->state |= SkipSwitcher;
            }

            else if (state == p->atom(_NET_WM_STATE_HIDDEN)) {
                p->state |= Hidden;
            }

            else if (state == p->atom(_NET_WM_STATE_FULLSCREEN)) {
                p->state |= FullScreen;
            }

            else if (state == p->atom(_NET_WM_STATE_ABOVE)) {
                p->state |= KeepAbove;
            }

            else if (state == p->atom(_NET_WM_STATE_BELOW)) {
                p->state |= KeepBelow;
            }

            else if (state == p->atom(_NET_WM_STATE_DEMANDS_ATTENTION)) {
                p->state |= DemandsAttention;
            }

            else if (state == p->atom(_NET_WM_STATE_STAYS_ON_TOP)) {
                p->state |= StaysOnTop;
            }
        }
    }

    if (dirty & WMDesktop) {
        p->desktop = 0;

        bool success;
        uint32_t desktop = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0, &success);

        if (success) {
            if (desktop != 0xffffffff) {
                p->desktop = desktop + 1;
            } else {
                p->desktop = OnAllDesktops;
            }
        }
    }

    if (dirty & WMName) {
        delete[] p->name;
        p->name = nullptr;

        const QByteArray str = get_string_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        if (str.length() > 0) {
            p->name = nstrndup(str.constData(), str.length());
        }
    }

    if (dirty & WMVisibleName) {
        delete[] p->visible_name;
        p->visible_name = nullptr;

        const QByteArray str = get_string_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        if (str.length() > 0) {
            p->visible_name = nstrndup(str.constData(), str.length());
        }
    }

    if (dirty & WMIconName) {
        delete[] p->icon_name;
        p->icon_name = nullptr;

        const QByteArray str = get_string_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        if (str.length() > 0) {
            p->icon_name = nstrndup(str.constData(), str.length());
        }
    }

    if (dirty & WMVisibleIconName) {
        delete[] p->visible_icon_name;
        p->visible_icon_name = nullptr;

        const QByteArray str = get_string_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        if (str.length() > 0) {
            p->visible_icon_name = nstrndup(str.constData(), str.length());
        }
    }

    if (dirty & WMWindowType) {
        p->types.reset();
        p->types[0] = Unknown;
        p->has_net_support = false;

        const QVector<xcb_atom_t> types = get_array_reply<xcb_atom_t>(p->conn, cookies[c++], XCB_ATOM_ATOM);

        if (types.count() > 0) {
#ifdef NETWMDEBUG
            fprintf(stderr, "NETWinInfo::update: getting window type (%ld)\n", types.count());
#endif
            p->has_net_support = true;
            int pos = 0;

            Q_FOREACH (const xcb_atom_t type, types) {
#ifdef NETWMDEBUG
                const QByteArray name = get_atom_name(p->conn, type);
                fprintf(stderr,  "NETWinInfo::update:   examining window type %ld %s\n",
                        type, name.constData());
#endif
                if (type == p->atom(_NET_WM_WINDOW_TYPE_NORMAL)) {
                    p->types[pos++] = Normal;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_DESKTOP)) {
                    p->types[pos++] = Desktop;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_DOCK)) {
                    p->types[pos++] = Dock;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_TOOLBAR)) {
                    p->types[pos++] = Toolbar;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_MENU)) {
                    p->types[pos++] = Menu;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_DIALOG)) {
                    p->types[pos++] = Dialog;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_UTILITY)) {
                    p->types[ pos++ ] = Utility;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_SPLASH)) {
                    p->types[pos++] = Splash;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU)) {
                    p->types[pos++] = DropdownMenu;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_POPUP_MENU)) {
                    p->types[pos++] = PopupMenu;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_TOOLTIP)) {
                    p->types[pos++] = Tooltip;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_NOTIFICATION)) {
                    p->types[pos++] = Notification;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_COMBOBOX)) {
                    p->types[pos++] = ComboBox;
                }

                else if (type == p->atom(_NET_WM_WINDOW_TYPE_DND)) {
                    p->types[pos++] = DNDIcon;
                }

                else if (type == p->atom(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE)) {
                    p->types[pos++] = Override;
                }

                else if (type == p->atom(_KDE_NET_WM_WINDOW_TYPE_TOPMENU)) {
                    p->types[pos++] = TopMenu;
                }

                else if (type == p->atom(_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY)) {
                    p->types[pos++] = OnScreenDisplay;
                }
            }
        }
    }

    if (dirty & WMStrut) {
        p->strut = NETStrut();

        QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == 4) {
            p->strut.left   = data[0];
            p->strut.right  = data[1];
            p->strut.top    = data[2];
            p->strut.bottom = data[3];
        }
    }

    if (dirty2 & WM2ExtendedStrut) {
        p->extended_strut = NETExtendedStrut();

        QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == 12) {
            p->extended_strut.left_width   = data[0];
            p->extended_strut.right_width  = data[1];
            p->extended_strut.top_width    = data[2];
            p->extended_strut.bottom_width = data[3];
            p->extended_strut.left_start   = data[4];
            p->extended_strut.left_end     = data[5];
            p->extended_strut.right_start  = data[6];
            p->extended_strut.right_end    = data[7];
            p->extended_strut.top_start    = data[8];
            p->extended_strut.top_end      = data[9];
            p->extended_strut.bottom_start = data[10];
            p->extended_strut.bottom_end   = data[11];
        }
    }

    if (dirty2 & WM2FullscreenMonitors) {
        p->fullscreen_monitors = NETFullscreenMonitors();

        QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == 4) {
            p->fullscreen_monitors.top    = data[0];
            p->fullscreen_monitors.bottom = data[1];
            p->fullscreen_monitors.left   = data[2];
            p->fullscreen_monitors.right  = data[3];
        }
    }

    if (dirty & WMIconGeometry) {
        p->icon_geom = NETRect();

        QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == 4) {
            p->icon_geom.pos.x       = data[0];
            p->icon_geom.pos.y       = data[1];
            p->icon_geom.size.width  = data[2];
            p->icon_geom.size.height = data[3];
        }
    }

    if (dirty & WMIcon) {
        readIcon(p->conn, cookies[c++], p->icons, p->icon_count);
        delete[] p->icon_sizes;
        p->icon_sizes = nullptr;
    }

    if (dirty & WMFrameExtents) {
        p->frame_strut = NETStrut();

        QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);

        if (data.count() == 0) {
            data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        } else {
            xcb_discard_reply(p->conn, cookies[c++].sequence);
        }

        if (data.count() == 4) {
            p->frame_strut.left   = data[0];
            p->frame_strut.right  = data[1];
            p->frame_strut.top    = data[2];
            p->frame_strut.bottom = data[3];
        }
    }

    if (dirty2 & WM2FrameOverlap) {
        p->frame_overlap = NETStrut();

        QVector<uint32_t> data = get_array_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        if (data.count() == 4) {
            p->frame_overlap.left   = data[0];
            p->frame_overlap.right  = data[1];
            p->frame_overlap.top    = data[2];
            p->frame_overlap.bottom = data[3];
        }
    }

    if (dirty2 & WM2Activities) {
        delete[] p->activities;
        p->activities = nullptr;

        const QByteArray activities = get_string_reply(p->conn, cookies[c++], XCB_ATOM_STRING);
        if (activities.length() > 0) {
            p->activities = nstrndup(activities.constData(), activities.length());
        }
    }

    if (dirty2 & WM2BlockCompositing) {
        bool success;
        p->blockCompositing = false;

        // _KDE_NET_WM_BLOCK_COMPOSITING
        uint32_t data = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0, &success);
        if (success)
            p->blockCompositing = bool(data);

        // _NET_WM_BYPASS_COMPOSITOR
        data = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0, &success);
        if (success) {
            switch (data) {
                case 1: p->blockCompositing = true; break;
                case 2: p->blockCompositing = false; break;
                default: break; // yes, the standard /is/ that stupid.
            }
        }
    }

    if (dirty & WMPid) {
        p->pid = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0);
    }

    if (dirty2 & WM2StartupId) {
        delete[] p->startup_id;
        p->startup_id = nullptr;

        const QByteArray id = get_string_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        if (id.length() > 0) {
            p->startup_id = nstrndup(id.constData(), id.length());
        }
    }

    if (dirty2 & WM2Opacity) {
        p->opacity = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0xffffffff);
    }

    if (dirty2 & WM2AllowedActions) {
        p->allowed_actions = NET::Actions();

        const QVector<xcb_atom_t> actions = get_array_reply<xcb_atom_t>(p->conn, cookies[c++], XCB_ATOM_ATOM);
        if (actions.count() > 0) {
#ifdef NETWMDEBUG
            fprintf(stderr, "NETWinInfo::update: updating allowed actions (%ld)\n", actions.count());
#endif

            Q_FOREACH (const xcb_atom_t action, actions) {
#ifdef NETWMDEBUG
                const QByteArray name = get_atom_name(p->conn, action);
                fprintf(stderr,
                        "NETWinInfo::update:   adding allowed action %ld '%s'\n",
                        action, name.constData());
#endif
                if (action == p->atom(_NET_WM_ACTION_MOVE)) {
                    p->allowed_actions |= ActionMove;
                }

                else if (action == p->atom(_NET_WM_ACTION_RESIZE)) {
                    p->allowed_actions |= ActionResize;
                }

                else if (action == p->atom(_NET_WM_ACTION_MINIMIZE)) {
                    p->allowed_actions |= ActionMinimize;
                }

                else if (action == p->atom(_NET_WM_ACTION_SHADE)) {
                    p->allowed_actions |= ActionShade;
                }

                else if (action == p->atom(_NET_WM_ACTION_STICK)) {
                    p->allowed_actions |= ActionStick;
                }

                else if (action == p->atom(_NET_WM_ACTION_MAXIMIZE_VERT)) {
                    p->allowed_actions |= ActionMaxVert;
                }

                else if (action == p->atom(_NET_WM_ACTION_MAXIMIZE_HORZ)) {
                    p->allowed_actions |= ActionMaxHoriz;
                }

                else if (action == p->atom(_NET_WM_ACTION_FULLSCREEN)) {
                    p->allowed_actions |= ActionFullScreen;
                }

                else if (action == p->atom(_NET_WM_ACTION_CHANGE_DESKTOP)) {
                    p->allowed_actions |= ActionChangeDesktop;
                }

                else if (action == p->atom(_NET_WM_ACTION_CLOSE)) {
                    p->allowed_actions |= ActionClose;
                }
            }
        }
    }

    if (dirty2 & WM2UserTime) {
        p->user_time = -1U;

        bool success;
        uint32_t value = get_value_reply<uint32_t>(p->conn, cookies[c++], XCB_ATOM_CARDINAL, 0, &success);

        if (success) {
            p->user_time = value;
        }
    }

    if (dirty2 & WM2TransientFor) {
        p->transient_for = get_value_reply<xcb_window_t>(p->conn, cookies[c++], XCB_ATOM_WINDOW, 0);
    }

    if (dirty2 & (WM2GroupLeader | WM2Urgency | WM2Input | WM2InitialMappingState | WM2IconPixmap)) {
        xcb_get_property_reply_t *reply = xcb_get_property_reply(p->conn, cookies[c++], nullptr);

        if (reply && reply->format == 32 && reply->value_len == 9 && reply->type == XCB_ATOM_WM_HINTS) {
            kde_wm_hints *hints = reinterpret_cast<kde_wm_hints *>(xcb_get_property_value(reply));

            if (hints->flags & (1 << 0)/*Input*/) {
                p->input = hints->input;
            }
            if (hints->flags & (1 << 1)/*StateHint*/) {
                switch (hints->initial_state) {
                case 3: // IconicState
                    p->initialMappingState = Iconic;
                    break;

                case 1: // NormalState
                    p->initialMappingState = Visible;
                    break;

                case 0: // WithdrawnState
                default:
                    p->initialMappingState = Withdrawn;
                    break;
                }
            }
            if (hints->flags & (1 << 2)/*IconPixmapHint*/) {
                p->icon_pixmap = hints->icon_pixmap;
            }
            if (hints->flags & (1 << 5)/*IconMaskHint*/) {
                p->icon_mask = hints->icon_mask;
            }
            if (hints->flags & (1 << 6)/*WindowGroupHint*/) {
                p->window_group = hints->window_group;
            }
            p->urgency = (hints->flags & (1 << 8)/*UrgencyHint*/);
        }

        if (reply) {
            free(reply);
        }
    }

    if (dirty2 & WM2WindowClass) {
        delete[] p->class_name;
        delete[] p->class_class;
        p->class_name = nullptr;
        p->class_class = nullptr;

        const QList<QByteArray> list = get_stringlist_reply(p->conn, cookies[c++], XCB_ATOM_STRING);
        if (list.count() == 2) {
            p->class_name  = nstrdup(list.at(0).constData());
            p->class_class = nstrdup(list.at(1).constData());
        }
    }

    if (dirty2 & WM2WindowRole) {
        delete[] p->window_role;
        p->window_role = nullptr;

        const QByteArray role = get_string_reply(p->conn, cookies[c++], XCB_ATOM_STRING);
        if (role.length() > 0) {
            p->window_role = nstrndup(role.constData(), role.length());
        }
    }

    if (dirty2 & WM2ClientMachine) {
        delete[] p->client_machine;
        p->client_machine = nullptr;

        const QByteArray value = get_string_reply(p->conn, cookies[c++], XCB_ATOM_STRING);
        if (value.length() > 0) {
            p->client_machine = nstrndup(value.constData(), value.length());
        }
    }

    if (dirty2 & WM2Protocols) {
        const QVector<xcb_atom_t> protocols = get_array_reply<xcb_atom_t>(p->conn, cookies[c++], XCB_ATOM_ATOM);
        p->protocols = NET::NoProtocol;
        for (auto it = protocols.begin(); it != protocols.end(); ++it) {
            if ((*it) == p->atom(WM_TAKE_FOCUS)) {
                p->protocols |= TakeFocusProtocol;
            } else if ((*it) == p->atom(WM_DELETE_WINDOW)) {
                p->protocols |= DeleteWindowProtocol;
            } else if ((*it) == p->atom(_NET_WM_PING)) {
                p->protocols |= PingProtocol;
            } else if ((*it) == p->atom(_NET_WM_SYNC_REQUEST)) {
                p->protocols |= SyncRequestProtocol;
            } else if ((*it) == p->atom(_NET_WM_CONTEXT_HELP)) {
                p->protocols |= ContextHelpProtocol;
            }
        }
    }

    if (dirty2 & WM2OpaqueRegion) {
        const QVector<qint32> values = get_array_reply<qint32>(p->conn, cookies[c++], XCB_ATOM_CARDINAL);
        p->opaqueRegion.clear();
        p->opaqueRegion.reserve(values.count() / 4);
        for (int i = 0; i < values.count() - 3; i += 4) {
            NETRect rect;
            rect.pos.x = values.at(i);
            rect.pos.y = values.at(i + 1);
            rect.size.width  = values.at(i + 2);
            rect.size.height = values.at(i + 3);
            p->opaqueRegion.push_back(rect);
        }
    }

    if (dirty2 & WM2DesktopFileName) {
        delete[] p->desktop_file;
        p->desktop_file = nullptr;

        const QByteArray id = get_string_reply(p->conn, cookies[c++], p->atom(UTF8_STRING));
        if (id.length() > 0) {
            p->desktop_file = nstrndup(id.constData(), id.length());
        }
    }
}

NETRect NETWinInfo::iconGeometry() const
{
    return p->icon_geom;
}

NET::States NETWinInfo::state() const
{
    return p->state;
}

NETStrut NETWinInfo::strut() const
{
    return p->strut;
}

NETExtendedStrut NETWinInfo::extendedStrut() const
{
    return p->extended_strut;
}

NETFullscreenMonitors NETWinInfo::fullscreenMonitors() const
{
    return p->fullscreen_monitors;
}

bool NET::typeMatchesMask(WindowType type, WindowTypes mask)
{
    switch (type) {
#define CHECK_TYPE_MASK( type ) \
case type: \
    if( mask & type##Mask ) \
        return true; \
    break;
        CHECK_TYPE_MASK(Normal)
        CHECK_TYPE_MASK(Desktop)
        CHECK_TYPE_MASK(Dock)
        CHECK_TYPE_MASK(Toolbar)
        CHECK_TYPE_MASK(Menu)
        CHECK_TYPE_MASK(Dialog)
        CHECK_TYPE_MASK(Override)
        CHECK_TYPE_MASK(TopMenu)
        CHECK_TYPE_MASK(Utility)
        CHECK_TYPE_MASK(Splash)
        CHECK_TYPE_MASK(DropdownMenu)
        CHECK_TYPE_MASK(PopupMenu)
        CHECK_TYPE_MASK(Tooltip)
        CHECK_TYPE_MASK(Notification)
        CHECK_TYPE_MASK(ComboBox)
        CHECK_TYPE_MASK(DNDIcon)
        CHECK_TYPE_MASK(OnScreenDisplay)
#undef CHECK_TYPE_MASK
    default:
        break;
    }
    return false;
}

NET::WindowType NETWinInfo::windowType(WindowTypes supported_types) const
{
    for (int i = 0;
            i < p->types.size();
            ++i) {
        // return the type only if the application supports it
        if (typeMatchesMask(p->types[ i ], supported_types)) {
            return p->types[ i ];
        }
    }
    return Unknown;
}

bool NETWinInfo::hasWindowType() const
{
    return p->types.size() > 0;
}

const char *NETWinInfo::name() const
{
    return p->name;
}

const char *NETWinInfo::visibleName() const
{
    return p->visible_name;
}

const char *NETWinInfo::iconName() const
{
    return p->icon_name;
}

const char *NETWinInfo::visibleIconName() const
{
    return p->visible_icon_name;
}

int NETWinInfo::desktop(bool ignore_viewport) const
{
    if (!ignore_viewport && KWindowSystem::mapViewport()) {
        const KWindowInfo info(p->window, NET::WMDesktop);
        return info.desktop();
    }
    return p->desktop;
}

int NETWinInfo::pid() const
{
    return p->pid;
}

xcb_timestamp_t NETWinInfo::userTime() const
{
    return p->user_time;
}

const char *NETWinInfo::startupId() const
{
    return p->startup_id;
}

unsigned long NETWinInfo::opacity() const
{
    return p->opacity;
}

NET::Actions NETWinInfo::allowedActions() const
{
    return p->allowed_actions;
}

bool NETWinInfo::hasNETSupport() const
{
    return p->has_net_support;
}

xcb_window_t NETWinInfo::transientFor() const
{
    return p->transient_for;
}

xcb_window_t NETWinInfo::groupLeader() const
{
    return p->window_group;
}

bool NETWinInfo::urgency() const
{
    return p->urgency;
}

bool NETWinInfo::input() const
{
    return p->input;
}

NET::MappingState NETWinInfo::initialMappingState() const
{
    return p->initialMappingState;
}

xcb_pixmap_t NETWinInfo::icccmIconPixmap() const
{
    return p->icon_pixmap;
}

xcb_pixmap_t NETWinInfo::icccmIconPixmapMask() const
{
    return p->icon_mask;
}

const char *NETWinInfo::windowClassClass() const
{
    return p->class_class;
}

const char *NETWinInfo::windowClassName() const
{
    return p->class_name;
}

const char *NETWinInfo::windowRole() const
{
    return p->window_role;
}

const char *NETWinInfo::clientMachine() const
{
    return p->client_machine;
}

const char *NETWinInfo::activities() const
{
    return p->activities;
}

void NETWinInfo::setActivities(const char *activities)
{
    delete [] p->activities;

    if (activities == (char *) nullptr || activities[0] == '\0') {
        // on all activities
        static const char nulluuid[] = KDE_ALL_ACTIVITIES_UUID;

        p->activities = nstrdup(nulluuid);

    } else {
        p->activities = nstrdup(activities);

    }

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_KDE_NET_WM_ACTIVITIES),
                        XCB_ATOM_STRING, 8, strlen(p->activities), p->activities);
}

void NETWinInfo::setBlockingCompositing(bool active)
{
    if (p->role != Client) {
        return;
    }

    p->blockCompositing = active;
    if (active) {
        uint32_t d = 1;
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_KDE_NET_WM_BLOCK_COMPOSITING),
                            XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
        xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_NET_WM_BYPASS_COMPOSITOR),
                            XCB_ATOM_CARDINAL, 32, 1, (const void *) &d);
    } else {
        xcb_delete_property(p->conn, p->window, p->atom(_KDE_NET_WM_BLOCK_COMPOSITING));
        xcb_delete_property(p->conn, p->window, p->atom(_NET_WM_BYPASS_COMPOSITOR));
    }
}

bool NETWinInfo::isBlockingCompositing() const
{
    return p->blockCompositing;
}

bool NETWinInfo::handledIcons() const
{
    return p->handled_icons;
}

NET::Properties NETWinInfo::passedProperties() const
{
    return p->properties;
}

NET::Properties2 NETWinInfo::passedProperties2() const
{
    return p->properties2;
}

NET::MappingState NETWinInfo::mappingState() const
{
    return p->mapping_state;
}

NET::Protocols NETWinInfo::protocols() const
{
    return p->protocols;
}

bool NETWinInfo::supportsProtocol(NET::Protocol protocol) const
{
    return p->protocols.testFlag(protocol);
}

std::vector< NETRect > NETWinInfo::opaqueRegion() const
{
    return p->opaqueRegion;
}

xcb_connection_t *NETWinInfo::xcbConnection() const
{
    return p->conn;
}

void NETWinInfo::setDesktopFileName(const char *name)
{
    if (p->role != Client) {
        return;
    }

    delete[] p->desktop_file;
    p->desktop_file = nstrdup(name);

    xcb_change_property(p->conn, XCB_PROP_MODE_REPLACE, p->window, p->atom(_KDE_NET_WM_DESKTOP_FILE),
                        p->atom(UTF8_STRING), 8, strlen(p->desktop_file),
                        (const void *) p->desktop_file);
}

const char *NETWinInfo::desktopFileName() const
{
    return p->desktop_file;
}

void NETRootInfo::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}

void NETWinInfo::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}

int NET::timestampCompare(unsigned long time1, unsigned long time2)
{
    return KXUtils::timestampCompare(time1, time2);
}

int NET::timestampDiff(unsigned long time1, unsigned long time2)
{
    return KXUtils::timestampDiff(time1, time2);
}

#endif
