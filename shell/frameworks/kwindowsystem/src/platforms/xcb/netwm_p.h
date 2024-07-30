/*
    SPDX-FileCopyrightText: 2000 Troll Tech AS
    SPDX-FileCopyrightText: 2003 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef netwm_p_h
#define netwm_p_h

#include <QSharedData>
#include <QSharedDataPointer>

#include "atoms_p.h"

class Atoms : public QSharedData
{
public:
    explicit Atoms(xcb_connection_t *c);

    xcb_atom_t atom(KwsAtom atom) const
    {
        return m_atoms[atom];
    }

private:
    void init();
    xcb_atom_t m_atoms[KwsAtomCount];
    xcb_connection_t *m_connection;
};

/**
   Resizable array class.

   This resizable array is used to simplify the implementation.  The existence of
   this class is to keep the implementation from depending on a separate
   framework/library.
   @internal
**/

template<class Z>
class NETRArray
{
public:
    /**
       Constructs an empty (size == 0) array.
    **/

    NETRArray();

    /**
       Resizable array destructor.
    **/

    ~NETRArray();

    /**
       The [] operator does the work.  If the index is larger than the current
       size of the array, it is resized.
     **/

    Z &operator[](int);

    /**
       Returns the size of the array.
     **/

    int size() const
    {
        return sz;
    }

    /**
       Resets the array (size == 0).
     **/
    void reset();

private:
    int sz;
    int capacity;
    Z *d;
};

/**
   Private data for the NETRootInfo class.
   @internal
**/

struct NETRootInfoPrivate {
    NET::Role role;

    // information about the X server
    xcb_connection_t *conn;
    NETSize rootSize;
    xcb_window_t root;
    xcb_window_t supportwindow;
    const char *name;

    uint32_t *temp_buf;
    size_t temp_buf_size;

    // data that changes (either by the window manager or by a client)
    // and requires updates
    NETRArray<NETPoint> viewport;
    NETRArray<NETRect> workarea;
    NETSize geometry;
    xcb_window_t active;
    xcb_window_t *clients, *stacking, *virtual_roots;
    NETRArray<const char *> desktop_names;
    int number_of_desktops;
    int current_desktop;

    unsigned long clients_count, stacking_count, virtual_roots_count;
    bool showing_desktop;
    NET::Orientation desktop_layout_orientation;
    NET::DesktopLayoutCorner desktop_layout_corner;
    int desktop_layout_columns, desktop_layout_rows;

    NET::Properties properties;
    NET::Properties2 properties2;
    NET::WindowTypes windowTypes;
    NET::States states;
    NET::Actions actions;
    NET::Properties clientProperties;
    NET::Properties2 clientProperties2;

    int ref;

    QSharedDataPointer<Atoms> atoms;
    xcb_atom_t atom(KwsAtom atom) const
    {
        return atoms->atom(atom);
    }
};

/**
   Private data for the NETWinInfo class.
   @internal
**/

struct NETWinInfoPrivate {
    NET::Role role;

    xcb_connection_t *conn;
    xcb_window_t window, root;
    NET::MappingState mapping_state;
    bool mapping_state_dirty;

    NETRArray<NETIcon> icons;
    int icon_count;
    int *icon_sizes; // for iconSizes() only

    NETRect icon_geom, win_geom;
    NET::States state;
    NETExtendedStrut extended_strut;
    NETStrut strut;
    NETStrut frame_strut; // strut?
    NETStrut frame_overlap;
    NETStrut gtk_frame_extents;
    NETRArray<NET::WindowType> types;
    char *name, *visible_name, *icon_name, *visible_icon_name;
    int desktop;
    int pid;
    bool handled_icons;
    xcb_timestamp_t user_time;
    char *startup_id;
    unsigned long opacity;
    xcb_window_t transient_for, window_group;
    xcb_pixmap_t icon_pixmap, icon_mask;
    NET::Actions allowed_actions;
    char *class_class, *class_name, *window_role, *client_machine, *desktop_file, *appmenu_object_path, *appmenu_service_name, *gtk_application_id;

    NET::Properties properties;
    NET::Properties2 properties2;
    NETFullscreenMonitors fullscreen_monitors;
    bool has_net_support;

    const char *activities;
    bool blockCompositing;
    bool urgency;
    bool input;
    NET::MappingState initialMappingState;
    NET::Protocols protocols;
    std::vector<NETRect> opaqueRegion;

    int ref;

    QSharedDataPointer<Atoms> atoms;
    xcb_atom_t atom(KwsAtom atom) const
    {
        return atoms->atom(atom);
    }
};

#endif // netwm_p_h
