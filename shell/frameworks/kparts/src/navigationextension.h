/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPARTS_NAVIGATIONEXTENSION
#define KPARTS_NAVIGATIONEXTENSION

#include <kparts/openurlarguments.h>
#include <kparts/readonlypart.h>

#include <memory>

#include <QAction>
#include <qplatformdefs.h> //mode_t

template<class Key, class T>
class QMap;
template<typename T>
class QList;

class KFileItem;
class KFileItemList;
class QDataStream;
class QPoint;

namespace KParts
{
class NavigationExtensionPrivate;

/**
 * @class NavigationExtension navigationextension.h <KParts/NavigationExtension>
 *
 * @short An extension to  KParts::ReadOnlyPart, which allows a better integration of parts
 * with browsers (in particular Konqueror).
 *
 * Remember that ReadOnlyPart only has openUrl(QUrl) and a few arguments() but not much more.
 * For full-fledged browsing, we need much more than that, including
 * enabling/disabling of standard actions (print, copy, paste...),
 * allowing parts to save and restore their data into the back/forward history,
 * allowing parts to control the location bar URL, to requests URLs
 * to be opened by the hosting browser, etc.
 *
 * The part developer needs to define its own class derived from BrowserExtension,
 * to implement the virtual methods [and the standard-actions slots, see below].
 *
 * The way to associate the BrowserExtension with the part is to simply
 * create the BrowserExtension as a child of the part (in QObject's terms).
 * The hosting application will look for it automatically.
 *
 * Another aspect of the browser integration is that a set of standard
 * actions are provided by the browser, but implemented by the part
 * (for the actions it supports).
 *
 * The following standard actions are defined by the host of the view:
 *
 * [selection-dependent actions]
 * @li @p cut : Copy selected items to clipboard and store 'not cut' in clipboard.
 * @li @p copy : Copy selected items to clipboard and store 'cut' in clipboard.
 * @li @p paste : Paste clipboard into view URL.
 * @li @p pasteTo(const QUrl &) : Paste clipboard into given URL.
 * @li @p searchProvider : Lookup selected text at default search provider
 *
 * [normal actions]
 * @li None anymore.
 *
 *
 * The view defines a slot with the name of the action in order to implement the action.
 * The browser will detect the slot automatically and connect its action to it when
 * appropriate (i.e. when the view is active).
 *
 *
 * The selection-dependent actions are disabled by default and the view should
 * enable them when the selection changes, emitting enableAction().
 *
 * The normal actions do not depend on the selection.
 *
 * A special case is the configuration slots, not connected to any action directly.
 *
 * [configuration slot]
 * @li @p reparseConfiguration : Re-read configuration and apply it.
 * @li @p disableScrolling: no scrollbars
 */
class KPARTS_EXPORT NavigationExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool urlDropHandling READ isURLDropHandlingEnabled WRITE setURLDropHandlingEnabled)
public:
    /**
     * Constructor
     *
     * @param parent The KParts::ReadOnlyPart that this extension ... "extends" :)
     */
    explicit NavigationExtension(KParts::ReadOnlyPart *parent);

    ~NavigationExtension() override;

    /**
     * Set of flags passed via the popupMenu signal, to ask for some items in the popup menu.
     * @see PopupFlags
     */
    enum PopupFlag {
        DefaultPopupItems = 0x0000, /**< default value, no additional menu item */
        ShowBookmark = 0x0008, /**< show "add to bookmarks" (usually not done on the local filesystem) */
        ShowCreateDirectory = 0x0010, /**<  show "create directory" (usually only done on the background of the view, or
                                       *                      in hierarchical views like directory trees, where the new dir would be visible) */
        ShowTextSelectionItems = 0x0020, /**< set when selecting text, for a popup that only contains text-related items. */
        NoDeletion = 0x0040, /**< deletion, trashing and renaming not allowed (e.g. parent dir not writeable).
                              *            (this is only needed if the protocol itself supports deletion, unlike e.g. HTTP) */
        IsLink = 0x0080, /**< show "Bookmark This Link" and other link-related actions (linkactions merging group) */
        ShowUrlOperations = 0x0100, /**< show copy, paste, as well as cut if NoDeletion is not set. */
        ShowProperties = 0x200, /**< show "Properties" action (usually done by directory views) */
    };

    /**
     * Stores a combination of #PopupFlag values.
     */
    Q_DECLARE_FLAGS(PopupFlags, PopupFlag)

    /**
     * Returns the current x offset.
     *
     * For a scrollview, implement this using contentsX().
     */
    virtual int xOffset();
    /**
     * Returns the current y offset.
     *
     * For a scrollview, implement this using contentsY().
     */
    virtual int yOffset();

    /**
     * Used by the browser to save the current state of the view
     * (in order to restore it if going back in navigation).
     *
     * If you want to save additional properties, reimplement it
     * but don't forget to call the parent method (probably first).
     */
    virtual void saveState(QDataStream &stream);

    /**
     * Used by the browser to restore the view in the state
     * it was when we left it.
     *
     * If you saved additional properties, reimplement it
     * but don't forget to call the parent method (probably first).
     */
    virtual void restoreState(QDataStream &stream);

    /**
     * Returns whether url drop handling is enabled.
     * See setURLDropHandlingEnabled for more information about this
     * property.
     */
    bool isURLDropHandlingEnabled() const;

    /**
     * Enables or disables url drop handling. URL drop handling is a property
     * describing whether the hosting shell component is allowed to install an
     * event filter on the part's widget, to listen for URI drop events.
     * Set it to true if you are exporting a BrowserExtension implementation and
     * do not provide any special URI drop handling. If set to false you can be
     * sure to receive all those URI drop events unfiltered. Also note that the
     * implementation as of Konqueror installs the event filter only on the part's
     * widget itself, not on child widgets.
     */
    void setURLDropHandlingEnabled(bool enable);

    /**
     * @return the status (enabled/disabled) of an action.
     * When the enableAction signal is emitted, the browserextension
     * stores the status of the action internally, so that it's possible
     * to query later for the status of the action, using this method.
     */
    bool isActionEnabled(const char *name) const;

    /**
     * @return the text of an action, if it was set explicitly by the part.
     * When the setActionText signal is emitted, the browserextension
     * stores the text of the action internally, so that it's possible
     * to query later for the text of the action, using this method.
     */
    QString actionText(const char *name) const;

    typedef QMap<QByteArray, QByteArray> ActionSlotMap;

    /**
     * Returns a pointer to the static map containing the action names as keys and corresponding
     * SLOT()'ified method names as data entries.
     * The map is created if it doesn't exist yet.
     *
     * This is very useful for
     * the host component, when connecting the own signals with the
     * extension's slots.
     * Basically you iterate over the map, check if the extension implements
     * the slot and connect to the slot using the data value of your map
     * iterator.
     * Checking if the extension implements a certain slot can be done like this:
     *
     * \code
     *   extension->metaObject()->slotNames().contains( actionName + "()" )
     * \endcode
     *
     * (note that @p actionName is the iterator's key value if already
     *  iterating over the action slot map, returned by this method)
     *
     * Connecting to the slot can be done like this:
     *
     * \code
     *   connect( yourObject, SIGNAL( yourSignal() ),
     *            extension, mapIterator.data() )
     * \endcode
     *
     * (where "mapIterator" is your ActionSlotMap iterator)
     */
    static ActionSlotMap *actionSlotMap();

    /**
     * Queries @p obj for a child object which inherits from this
     * BrowserExtension class. Convenience method.
     */
    static NavigationExtension *childObject(QObject *obj);

    /**
     * Asks the hosting browser to perform a paste (using openUrlRequestDelayed())
     */
    void pasteRequest();

    /**
     * Associates a list of actions with a predefined name known by the host's popupmenu:
     * "editactions" for actions related text editing,
     * "linkactions" for actions related to hyperlinks,
     * "partactions" for any other actions provided by the part
     */
    typedef QMap<QString, QList<QAction *>> ActionGroupMap;

Q_SIGNALS:
    /**
     * Enables or disable a standard action held by the browser.
     *
     * See class documentation for the list of standard actions.
     */
    void enableAction(const char *name, bool enabled);

    /**
     * Change the text of a standard action held by the browser.
     * This can be used to change "Paste" into "Paste Image" for instance.
     *
     * See class documentation for the list of standard actions.
     */
    void setActionText(const char *name, const QString &text);

    /**
     * Asks the host (browser) to open @p url.
     * To set a reload, the x and y offsets, the service type etc., fill in the
     * appropriate fields in the @p args structure.
     * Hosts should not connect to this signal but to openUrlRequestDelayed().
     */
    void openUrlRequest(const QUrl &url, const KParts::OpenUrlArguments &arguments = KParts::OpenUrlArguments());

    /**
     * This signal is emitted when openUrlRequest() is called, after a 0-seconds timer.
     * This allows the caller to terminate what it's doing first, before (usually)
     * being destroyed. Parts should never use this signal, hosts should only connect
     * to this signal.
     */
    void openUrlRequestDelayed(const QUrl &url, const KParts::OpenUrlArguments &arguments);

    /**
     * Tells the hosting browser that the part opened a new URL (which can be
     * queried via KParts::Part::url().
     *
     * This helps the browser to update/create an entry in the history.
     * The part may @em not emit this signal together with openUrlRequest().
     * Emit openUrlRequest() if you want the browser to handle a URL the user
     * asked to open (from within your part/document). This signal however is
     * useful if you want to handle URLs all yourself internally, while still
     * telling the hosting browser about new opened URLs, in order to provide
     * a proper history functionality to the user.
     * An example of usage is a html rendering component which wants to emit
     * this signal when a child frame document changed its URL.
     * Conclusion: you probably want to use openUrlRequest() instead.
     */
    void openUrlNotify();

    /**
     * Updates the URL shown in the browser's location bar to @p url.
     */
    void setLocationBarUrl(const QString &url);

    /**
     * Sets the URL of an icon for the currently displayed page.
     */
    void setIconUrl(const QUrl &url);

    /**
     * Asks the hosting browser to open a new window for the given @p url
     * and return a reference to the content part.
     */
    void createNewWindow(const QUrl &url);

    /**
     * Since the part emits the jobid in the started() signal,
     * progress information is automatically displayed.
     *
     * However, if you don't use a KIO::Job in the part,
     * you can use loadingProgress() and speedProgress()
     * to display progress information.
     */
    void loadingProgress(int percent);
    /**
     * @see loadingProgress
     */
    void speedProgress(int bytesPerSecond);

    void infoMessage(const QString &);

    /**
     * Emit this to make the browser show a standard popup menu for the files @p items.
     *
     * @param global global coordinates where the popup should be shown
     * @param items list of file items which the popup applies to
     * @param args OpenUrlArguments, mostly for metadata here
     * @param flags enables/disables certain builtin actions in the popupmenu
     * @param actionGroups named groups of actions which should be inserted into the popup, see ActionGroupMap
     */
    void popupMenu(const QPoint &global,
                   const KFileItemList &items,
                   const KParts::OpenUrlArguments &arguments = KParts::OpenUrlArguments(),
                   KParts::NavigationExtension::PopupFlags flags = KParts::NavigationExtension::DefaultPopupItems,
                   const KParts::NavigationExtension::ActionGroupMap &actionGroups = ActionGroupMap());

    /**
     * Emit this to make the browser show a standard popup menu for the given @p url.
     *
     * Give as much information about this URL as possible,
     * like @p args.mimeType and the file type @p mode
     *
     * @param global global coordinates where the popup should be shown
     * @param url the URL this popup applies to
     * @param mode the file type of the url (S_IFREG, S_IFDIR...)
     * @param args OpenUrlArguments, set the mimetype of the URL using setMimeType()
     * @param flags enables/disables certain builtin actions in the popupmenu
     * @param actionGroups named groups of actions which should be inserted into the popup, see ActionGroupMap
     */
    void popupMenu(const QPoint &global,
                   const QUrl &url,
                   mode_t mode = static_cast<mode_t>(-1),
                   const KParts::OpenUrlArguments &arguments = KParts::OpenUrlArguments(),
                   KParts::NavigationExtension::PopupFlags flags = KParts::NavigationExtension::DefaultPopupItems,
                   const KParts::NavigationExtension::ActionGroupMap &actionGroups = ActionGroupMap());

    /**
     * Inform the hosting application about the current selection.
     * Used when a set of files/URLs is selected (with full information
     * about those URLs, including size, permissions etc.)
     */
    void selectionInfo(const KFileItemList &items);

    /**
     * Inform the hosting application that the user moved the mouse over an item.
     * Used when the mouse is on an URL.
     */
    void mouseOverInfo(const KFileItem &item);

    /**
     * Ask the hosting application to add a new HTML (aka Mozilla/Netscape)
     * SideBar entry.
     */
    void addWebSideBar(const QUrl &url, const QString &name);

    /**
     * Ask the hosting application to move the top level widget.
     */
    void moveTopLevelWidget(int x, int y);

    /**
     * Ask the hosting application to resize the top level widget.
     */
    void resizeTopLevelWidget(int w, int h);

    /**
     * Ask the hosting application to focus @p part.
     */
    void requestFocus(KParts::ReadOnlyPart *part);

    /**
     * Tell the host (browser) about security state of current page
     * enum PageSecurity { NotCrypted, Encrypted, Mixed };
     */
    void setPageSecurity(int);

    /**
     * Inform the host about items that have been removed.
     */
    void itemsRemoved(const KFileItemList &items);

private Q_SLOTS:
    KPARTS_NO_EXPORT void slotOpenUrlRequest(const QUrl &url, const KParts::OpenUrlArguments &arguments = KParts::OpenUrlArguments());

    KPARTS_NO_EXPORT void slotEmitOpenUrlRequestDelayed();
    KPARTS_NO_EXPORT void slotEnableAction(const char *, bool);
    KPARTS_NO_EXPORT void slotSetActionText(const char *, const QString &);

public:
    typedef QMap<QByteArray, int> ActionNumberMap;

private:
    std::unique_ptr<NavigationExtensionPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NavigationExtension::PopupFlags)

}

#endif
