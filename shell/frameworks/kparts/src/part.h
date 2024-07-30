/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_PART_H
#define _KPARTS_PART_H

#include <KPluginMetaData>
#include <kparts/partbase.h>

class QWidget;
class QEvent;
class QPoint;

/**
 * The KParts namespace,
 */
namespace KParts
{
class PartManager;
class PartPrivate;
class PartActivateEvent;
class GUIActivateEvent;

/**
 * @class Part part.h <KParts/Part>
 *
 * @short Base class for parts.
 *
 * A "part" is a GUI component, featuring:
 * @li A widget embeddedable in any application.
 * @li GUI elements that will be merged in the "host" user interface
 * (menubars, toolbars... ).
 *
 * <b>About the widget:</b>\n
 *
 * Note that KParts::Part does not inherit QWidget.
 * This is due to the fact that the "visual representation"
 * will probably not be a mere QWidget, but an elaborate one.
 * That's why when implementing your KParts::Part (or derived)
 * you should call KParts::Part::setWidget() in your constructor.
 *
 * <b>About the GUI elements:</b>\n
 *
 * Those elements trigger actions, defined by the part ( action()).
 * The layout of the actions in the GUI is defined by an XML file ( setXMLFile()).
 *
 * See also ReadOnlyPart and ReadWritePart, which define the
 * framework for a "viewer" part and for an "editor"-like part.
 * Use Part directly only if your part doesn't fit into those.
 */
class KPARTS_EXPORT Part : public QObject, public PartBase
{
    Q_OBJECT

    KPARTS_DECLARE_PRIVATE(Part)

public:
    /**
     *  Constructor.
     *
     *  @param parent Parent object of the part.
     *  @param data KPluginMetaData associated with this part, see Part::metaData()
     */
    explicit Part(QObject *parent = nullptr, const KPluginMetaData &data = {});

    /**
     *  Destructor.
     */
    ~Part() override;

    /**
     * @return The widget defined by this part, set by setWidget().
     */
    virtual QWidget *widget();

    /**
     * @internal
     * Used by the part manager.
     */
    virtual void setManager(PartManager *manager);

    /**
     * Returns the part manager handling this part, if any (0L otherwise).
     */
    PartManager *manager() const;

    /**
     * By default, the widget is deleted by the part when the part is deleted.
     * The hosting application can call setAutoDeleteWidget(false) to
     * disable this behavior, given that the widget is usually deleted by
     * its parent widget anyway.
     * This is a method for the hosting application only, Part subclasses
     * should never call this.
     */
    void setAutoDeleteWidget(bool autoDeleteWidget);

    /**
     * By default, the part deletes itself when its widget is deleted.
     * The hosting application can call setAutoDeletePart(false) to
     * disable this behavior, to be able to delete the widget and then the part,
     * independently.
     * This is a method for the hosting application only, Part subclasses
     * should never call this.
     */
    void setAutoDeletePart(bool autoDeletePart);

    /**
     * Returns the part (this, or a child part) at the given global position.
     * This is called by the part manager to ask whether a part should be activated
     * when clicking somewhere. In most cases the default implementation is enough.
     * Reimplement this if your part has child parts in some areas (like in khtml or koffice)
     * @param widget the part widget being clicked - usually the same as widget(), except in koffice.
     * @param globalPos the mouse coordinates in global coordinates
     */
    virtual Part *hitTest(QWidget *widget, const QPoint &globalPos);

    /**
     * @since 5.77
     */
    KPluginMetaData metaData() const;

Q_SIGNALS:
    /**
     * Emitted by the part, to set the caption of the window(s)
     * hosting this part
     *
     * @note this signal has only an effect on the window title if window title
     *       handling is enabled @see KParts::MainWindow::setWindowTitleHandling
     */
    void setWindowCaption(const QString &caption);
    /**
     * Emitted by the part, to set a text in the statusbar of the window(s)
     * hosting this part
     */
    void setStatusBarText(const QString &text);

protected:
    /**
     * Set the main widget.
     *
     * Call this in the Part-inherited class constructor.
     */
    virtual void setWidget(QWidget *widget);

    /**
     * @internal
     */
    void customEvent(QEvent *event) override;

    /**
     * Convenience method which is called when the Part received a PartActivateEvent .
     * Reimplement this if you don't want to reimplement event and test for the event yourself
     * or even install an event filter.
     */
    virtual void partActivateEvent(PartActivateEvent *event);

    /**
     * Convenience method which is called when the Part received a
     * GUIActivateEvent .
     * Reimplement this if you don't want to reimplement event and
     * test for the event yourself or even install an event filter.
     */
    virtual void guiActivateEvent(GUIActivateEvent *event);

    /**
     * Convenience method for KXMLGUIFactory::container.
     * @return a container widget owned by the Part's GUI.
     */
    QWidget *hostContainer(const QString &containerName);

protected Q_SLOTS:
    /**
     * @internal
     */
    void slotWidgetDestroyed();

protected:
    KPARTS_NO_EXPORT Part(PartPrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(Part)
};

} // namespace

#endif
