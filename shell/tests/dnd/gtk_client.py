import sys
import gi

gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, GLib, Gtk

window = Gtk.Window(title=sys.argv[1])
window.set_default_size(180, 120)
area = Gtk.EventBox()
area.add(Gtk.Label(label="Lingmo drag"))
window.add(area)
targets = [Gtk.TargetEntry.new("application/x-lingmo-dnd", 0, 0)]
actions = Gdk.DragAction.COPY | Gdk.DragAction.MOVE

if sys.argv[1].endswith("source"):
    area.drag_source_set(Gdk.ModifierType.BUTTON1_MASK, targets, actions)
    area.connect("drag-begin", lambda *_: print("STARTED", flush=True))

    def send(widget, context, selection, info, time):
        selection.set(selection.get_target(), 8, b"d" * 131072)

    def ended(widget, context):
        print("FINISHED", flush=True)

    area.connect("drag-data-get", send)
    area.connect("drag-end", ended)
else:
    area.drag_dest_set(Gtk.DestDefaults.ALL, targets, actions)

    def received(widget, context, x, y, selection, info, time):
        valid = bytes(selection.get_data()) == b"d" * 131072
        Gtk.drag_finish(context, valid, False, time)
        print("PASS" if valid else "FAIL", flush=True)

    area.connect("drag-data-received", received)

window.connect("destroy", Gtk.main_quit)
window.show_all()
GLib.timeout_add_seconds(15, Gtk.main_quit)
Gtk.main()
