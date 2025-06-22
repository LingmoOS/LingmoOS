/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOOLBAR_H
#define KTOOLBAR_H

#include <kxmlgui_export.h>

#include <QToolBar>
#include <memory>

class QDomElement;

class KConfigGroup;
class KConfig;
class KMainWindow;
class KXMLGUIClient;

/**
 * @class KToolBar ktoolbar.h KToolBar
 *
 * @short Floatable toolbar with auto resize.
 *
 * A KDE-style toolbar.
 *
 * KToolBar can be used as a standalone widget, but KMainWindow
 * provides easy factories and management of one or more toolbars.
 *
 * KToolBar uses a global config group to load toolbar settings on
 * construction. It will reread this config group on a
 * KApplication::appearanceChanged() signal.
 *
 * KToolBar respects Kiosk settings (see the KAuthorized namespace in the
 * KConfig framework).  In particular, system administrators can prevent users
 * from moving toolbars with the "movable_toolbars" action, and from showing or
 * hiding toolbars with the "options_show_toolbar" action.  For example, to
 * disable both, add the following the application or global configuration:
 * @verbatim
   [KDE Action Restrictions][$i]
   movable_toolbars=false
   options_show_toolbar=false
   @endverbatim
 *
 * If you can't depend on KXmlGui but still want to integrate better with KDE,
 * you can use QToolBar and:
 * - Set ToolButtonStyle to Qt::ToolButtonFollowStyle, this will make QToolBar
 *   use the settings for "Main Toolbar"
 * - Additionally set QToolBar::setProperty("otherToolbar", true) to use settings
 *   for "Other toolbars"; settings from "Other toolbars" will only work on widget
 *   styles derived from KStyle
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 * @author Stephan Kulow <coolo@kde.org>
 * @author Sven Radej <radej@kde.org>
 * @author Hamish Rodda <rodda@kde.org>
 */
class KXMLGUI_EXPORT KToolBar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if @p parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * @param parent      The standard toolbar parent (usually a KMainWindow)
     * @param isMainToolBar  True for the "main toolbar", false for other toolbars. Different settings apply.
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    explicit KToolBar(QWidget *parent, bool isMainToolBar = false, bool readConfig = true);
    // KDE5: remove. The one below is preferred so that all debug output from init() shows the right objectName already,
    // and so that isMainToolBar() and iconSizeDefault() return correct values during loading too.

    /**
     * Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if @p parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * @param objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position,
     *                    and so that KToolBar can find out if it's the main toolbar.
     * @param parent      The standard toolbar parent (usually a KMainWindow)
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    explicit KToolBar(const QString &objectName, QWidget *parent, bool readConfig = true);

    /**
     * Alternate constructor with additional arguments, e.g. to choose in which area
     * the toolbar should be auto-added. This is rarely used in KDE. When using XMLGUI
     * you can specify this as an xml attribute instead.
     *
     * @param objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position
     * @param parentWindow The window that should be the parent of this toolbar
     * @param area        The position of the toolbar. Usually Qt::TopToolBarArea.
     * @param newLine     If true, start a new line in the dock for this toolbar.
     * @param isMainToolBar  True for the "main toolbar", false for other toolbars. Different settings apply.
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    KToolBar(const QString &objectName,
             QMainWindow *parentWindow,
             Qt::ToolBarArea area,
             bool newLine = false,
             bool isMainToolBar = false,
             bool readConfig = true); // KDE5: remove, I don't think anyone is using this.

    /**
     * Destroys the toolbar.
     */
    ~KToolBar() override;

    /**
     * Returns the main window that this toolbar is docked with.
     */
    KMainWindow *mainWindow() const;

    /**
     * Convenience function to set icon size
     */
    void setIconDimensions(int size);

    /**
     * Returns the default size for this type of toolbar.
     *
     * @return the default size for this type of toolbar.
     */
    int iconSizeDefault() const; // KDE5: hide from public API. Doesn't make sense to export this, and it isn't used.

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * This allows you to enable or disable the context menu.
     *
     * @param enable If false, then the context menu will be disabled
     * @deprecated Since 5.0, use setContextMenuPolicy()
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use QWidget::setContextMenuPolicy(Qt::ContextMenuPolicy)")
    void setContextMenuEnabled(bool enable = true);
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Returns the context menu enabled flag
     * @return true if the context menu is disabled
     * @deprecated Since 5.0, use contextMenuPolicy()
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use QWidget::contextMenuPolicy()")
    bool contextMenuEnabled() const;
#endif

    /**
     * Save the toolbar settings to group @p cg.
     */
    void saveSettings(KConfigGroup &cg);

    /**
     * Read the toolbar settings from group @p cg
     * and apply them.
     */
    void applySettings(const KConfigGroup &cg);

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Sets the XML gui client.
     * @deprecated Since 5.0, use addXMLGUIClient()
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KToolBar::addXMLGUIClient(KXMLGUIClient *)")
    void setXMLGUIClient(KXMLGUIClient *client);
#endif

    /**
     * Adds an XML gui client that uses this toolbar
     * @since 4.8.1
     */
    void addXMLGUIClient(KXMLGUIClient *client);

    /**
     * Removes an XML gui client that uses this toolbar
     * @since 4.8.5
     */
    void removeXMLGUIClient(KXMLGUIClient *client);

    /**
     * Load state from an XML @param element, called by KXMLGUIBuilder.
     */
    void loadState(const QDomElement &element);

    /**
     * Save state into an XML @param element, called by KXMLGUIBuilder.
     */
    void saveState(QDomElement &element) const;

    /**
     * Reimplemented to support context menu activation on disabled tool buttons.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * Returns whether the toolbars are currently editable (drag & drop of actions).
     */
    static bool toolBarsEditable();

    /**
     * Enable or disable toolbar editing via drag & drop of actions.  This is
     * called by KEditToolBar and should generally be set to disabled whenever
     * KEditToolBar is not active.
     */
    static void setToolBarsEditable(bool editable);

    /**
     * Returns whether the toolbars are locked (i.e., moving of the toobars disallowed).
     */
    static bool toolBarsLocked();

    /**
     * Allows you to lock and unlock all toolbars (i.e., disallow/allow moving of the toobars).
     */
    static void setToolBarsLocked(bool locked);

    /**
     * Emits a D-Bus signal to tell all toolbars in all applications, that the user settings have
     * changed.
     * @since 5.0
     */
    static void emitToolbarStyleChanged();

protected Q_SLOTS:
    virtual void slotMovableChanged(bool movable);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void actionEvent(QActionEvent *) override;

    // Draggable toolbar configuration
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    friend class KToolBarPrivate;
    std::unique_ptr<class KToolBarPrivate> const d;

    Q_PRIVATE_SLOT(d, void slotAppearanceChanged())
    Q_PRIVATE_SLOT(d, void slotContextRight())
    Q_PRIVATE_SLOT(d, void slotContextTextRight())
};

#endif
