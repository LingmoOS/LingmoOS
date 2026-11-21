// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <map>
#include <string>

#include <wayland-client.h>

#include <appmenu-client-protocol.h>
#include <gtk-shell-client-protocol.h>
#include <cursor-shape-v1-client-protocol.h>
#include <lingmo-shell-v1-client-protocol.h>
#include <lingmo-status-item-v1-client-protocol.h>
#include <lingmo-surface-blur-v1-client-protocol.h>
#include <lingmo-window-hints-v1-client-protocol.h>
#include <ext-image-capture-source-v1-client-protocol.h>
#include <ext-data-control-v1-client-protocol.h>
#include <ext-foreign-toplevel-list-v1-client-protocol.h>
#include <ext-idle-notify-v1-client-protocol.h>
#include <ext-image-copy-capture-v1-client-protocol.h>
#include <fractional-scale-v1-client-protocol.h>
#include <idle-inhibit-unstable-v1-client-protocol.h>
#include <input-method-unstable-v2-client-protocol.h>
#include <keyboard-shortcuts-inhibit-unstable-v1-client-protocol.h>
#include <pointer-constraints-unstable-v1-client-protocol.h>
#include <pointer-gestures-unstable-v1-client-protocol.h>
#include <primary-selection-unstable-v1-client-protocol.h>
#include <relative-pointer-unstable-v1-client-protocol.h>
#include <text-input-unstable-v1-client-protocol.h>
#include <text-input-unstable-v3-client-protocol.h>
#include <viewporter-client-protocol.h>
#include <virtual-keyboard-unstable-v1-client-protocol.h>
#include <wlr-data-control-unstable-v1-client-protocol.h>
#include <xdg-activation-v1-client-protocol.h>
#include <xdg-decoration-client-protocol.h>
#include <xdg-dialog-v1-client-protocol.h>
#include <xdg-foreign-unstable-v2-client-protocol.h>
#include <xdg-output-unstable-v1-client-protocol.h>
#include <xdg-shell-client-protocol.h>
#include <xdg-toplevel-drag-v1-client-protocol.h>

namespace Lingmo::Testing {

// Every interface a Lingmo client can meet in the registry. A global missing
// here fails protocol_binding_test's AdvertisedGlobalsAreKnown, and the
// protocol fuzzer cannot bind it.
inline const std::map<std::string, const wl_interface *> &knownInterfaces()
{
    static const std::map<std::string, const wl_interface *> interfaces = [] {
        std::map<std::string, const wl_interface *> map;
        for (const wl_interface *interface : {
                     &wl_compositor_interface,
                     &wl_subcompositor_interface,
                     &wl_shm_interface,
                     &wl_seat_interface,
                     &wl_output_interface,
                     &wl_data_device_manager_interface,
                     &ext_idle_notifier_v1_interface,
                     &zwp_idle_inhibit_manager_v1_interface,
                     &wp_cursor_shape_manager_v1_interface,
                     &zwp_pointer_constraints_v1_interface,
                     &zwp_relative_pointer_manager_v1_interface,
                     &zwp_pointer_gestures_v1_interface,
                     &zwp_keyboard_shortcuts_inhibit_manager_v1_interface,
                     &zwp_primary_selection_device_manager_v1_interface,
                     &xdg_activation_v1_interface,
                     &xdg_wm_base_interface,
                     &zxdg_decoration_manager_v1_interface,
                     &xdg_wm_dialog_v1_interface,
                     &zxdg_exporter_v2_interface,
                     &zxdg_importer_v2_interface,
                     &xdg_toplevel_drag_manager_v1_interface,
                     &lingmo_surface_blur_v1_interface,
                     &lingmo_window_hints_v1_interface,
                     &lingmo_status_item_manager_v1_interface,
                     &org_kde_kwin_appmenu_manager_interface,
                     &gtk_shell1_interface,
                     &wp_fractional_scale_manager_v1_interface,
                     &zxdg_output_manager_v1_interface,
                     &wp_viewporter_interface,
                     &zwp_text_input_manager_v1_interface,
                     &zwp_text_input_manager_v3_interface,
                     &zwp_input_method_manager_v2_interface,
                     &zwp_virtual_keyboard_manager_v1_interface,
                     &ext_data_control_manager_v1_interface,
                     &zwlr_data_control_manager_v1_interface,
                     &ext_output_image_capture_source_manager_v1_interface,
                     &ext_foreign_toplevel_image_capture_source_manager_v1_interface,
                     &ext_image_copy_capture_manager_v1_interface,
                     &ext_foreign_toplevel_list_v1_interface,
                     &lingmo_shell_v1_interface,
             })
            map.emplace(interface->name, interface);
        return map;
    }();
    return interfaces;
}

} // namespace Lingmo::Testing
