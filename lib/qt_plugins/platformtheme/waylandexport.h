#ifndef WAYLANDEXPORT_H
#define WAYLANDEXPORT_H

#include <QString>

class QWindow;

// One xdg-foreign export of an application window. The portal passes the handle
// to the dialog process, which parents itself onto the window. Exporting must
// outlive the dialog: destroying the export invalidates the relationship.
class WaylandExport
{
public:
    WaylandExport() = default;
    ~WaylandExport();

    WaylandExport(const WaylandExport &) = delete;
    WaylandExport &operator=(const WaylandExport &) = delete;

    // Returns the portal's parent_window identifier, or an empty string when
    // the window cannot be exported. Each call replaces the export held: a
    // window hidden since has a new surface, which the old export does not name.
    QString exportWindow(QWindow *window);

private:
    void release();

    struct wl_display *m_display = nullptr;
    struct zxdg_exported_v2 *m_exported = nullptr;
    // The listener writes here; it outlives the roundtrip that fills it.
    QString m_handle;
};

#endif // WAYLANDEXPORT_H
