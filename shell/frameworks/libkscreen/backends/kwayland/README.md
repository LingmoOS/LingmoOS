# Design of libkscreen's Wayland backend

This backend uses KWayland's OutputManagement protocol for listing and
configuring devices. This is described here.

## Listing outputs

KScreen's outputs are created from kde_output_device_v2 objects,
they copy the data into kscreen's Outputs, and update these objects. A list
of outputs is requested from the client Registry object.

## Configuring outputs

The backend asks the global OutputManagement interface for an OutputConfiguration
object, then sets the changes per outputdevice on this object, and asks the
compositor to apply() this configuration.

For this to work, the compositor should support the Wayland org_kde_kwin_outputdevice
and org_kde_kwin_outputmanagement protocols, for example through
KWayland::Server classes OutputDevice, OutputManagmenent and OuputConfiguration.

## General working

WaylandBackend creates a global static internal config, available through
WaylandBackend::internalConfig(). WaylandConfig binds to the wl_registry
callbacks and catches org_kde_kwin_outputdevice creation and destruction.
It passes org_kde_kwin_outputdevice creation and removal on to
WB::internalConfig() to handle its internal data representation as WaylandOutput.
WaylandOutput binds to org_kde_kwin_outputdevice's callback, and gets notified
of geometry and modes, including changes. WaylandOutput administrates the
internal representation of these objects, and invokes the global notifier,
which then runs the pointers it holds through the updateK* methods in
Wayland{Screen,Output,...}.

KScreen:{Screen,Output,Edid,Mode} objects are created from the internal
representation as requested (usually triggered by the creation of a
KScreen::Config object through KScreen::Config::current()). As with other
backends, the objects which are handed out to the lib's user are expected
to be deleted by the user, the backend only takes ownership of its internal
data representation objects.

## Note about scope of output ids

The ids of the outputdevices are internal to the wayland backend. The id is
generated in the wayland backend, and does not match kwin's output ids. Do
not try to read kwin's config from here.

                                                            <sebas@kde.org>

