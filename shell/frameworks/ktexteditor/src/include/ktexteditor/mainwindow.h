/*
    SPDX-FileCopyrightText: 2013 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_MAINWINDOW_H
#define KTEXTEDITOR_MAINWINDOW_H

#include <ktexteditor_export.h>

#include <QObject>
#include <QWidgetList>

class QEvent;
class QIcon;
class QUrl;
class QWidget;

class KXMLGUIFactory;

namespace KTextEditor
{
class Plugin;
class Document;
class View;

/**
 * \class MainWindow mainwindow.h <KTextEditor/MainWindow>
 *
 * This class allows the application that embeds the KTextEditor component to
 * allow it to access parts of its main window.
 *
 * For example the component can get a place to show view bar widgets (e.g. search&replace, goto line, ...).
 * This is useful to e.g. have one place inside the window to show such stuff even if the application allows
 * the user to have multiple split views available per window.
 *
 * The application must pass a pointer to the MainWindow object to the createView method on view creation
 * and ensure that this main window stays valid for the complete lifetime of the view.
 *
 * It must not reimplement this class but construct an instance and pass a pointer to a QObject that
 * has the required slots to receive the requests.
 *
 * @ref kte_plugin_hosting
 */
class KTEXTEDITOR_EXPORT MainWindow : public QObject
{
    Q_OBJECT

public:
    /**
     * Construct an MainWindow wrapper object.
     * The passed parent is both the parent of this QObject and the receiver of all interface
     * calls via invokeMethod.
     * @param parent object the calls are relayed to
     */
    MainWindow(QObject *parent);

    /**
     * Virtual Destructor
     */
    ~MainWindow() override;

    //
    // Accessors to some window properties and contents
    //
public:
    /**
     * Get the toplevel widget.
     * \return the real main window widget, nullptr if not available
     */
    QWidget *window();

    /**
     * Accessor to the XMLGUIFactory.
     * \return the mainwindow's KXMLGUIFactory, nullptr if not available
     */
    KXMLGUIFactory *guiFactory();

    //
    // Signals related to the main window
    //
Q_SIGNALS:
    /**
     * This signal is emitted for every unhandled ShortcutOverride in the window
     * @param e responsible event
     */
    void unhandledShortcutOverride(QEvent *e);

    //
    // View access and manipulation interface
    //
public:
    /**
     * Get a list of all views for this main window.
     *
     * It is beneficial if the list is sorted by most recently used,
     * as the library will e.g. try to use the most recent used url() by walking over this
     * list for save and other such things.
     *
     * @return all views, might be empty!
     */
    QList<KTextEditor::View *> views();

    /**
     * Access the active view.
     * \return active view, nullptr if not available
     */
    KTextEditor::View *activeView();

    /**
     * Activate the view with the corresponding \p document.
     * If none exist for this document, create one
     * \param document the document
     * \return activated view of this document,
     *         return nullptr if not possible
     */
    KTextEditor::View *activateView(KTextEditor::Document *document);

    /**
     * Open the document \p url with the given \p encoding.
     * \param url the document's url
     * \param encoding the preferred encoding. If encoding is QString() the
     *        encoding will be guessed or the default encoding will be used.
     * \return a pointer to the created view for the new document, if a document
     *         with this url is already existing, its view will be activated,
     *         return nullptr if not possible
     */
    KTextEditor::View *openUrl(const QUrl &url, const QString &encoding = QString());

    /**
     * Close selected view
     * \param view the view
     * \return true if view was closed
     */
    bool closeView(KTextEditor::View *view);

    /**
     * Split current view space according to \p orientation
     * \param orientation in which line split the view
     */
    void splitView(Qt::Orientation orientation);

    /**
     * Close the split view that contains the given view.
     * \param view the view.
     * \return true if the split view was closed.
     */
    bool closeSplitView(KTextEditor::View *view);

    /**
     * \returns \c true if the given views \p view1 and \p view2 share
     * the same split view, false otherwise.
     */
    bool viewsInSameSplitView(KTextEditor::View *view1, KTextEditor::View *view2);

    //
    // Signals related to view handling
    //
Q_SIGNALS:
    /**
     * This signal is emitted whenever the active view changes.
     * @param view new active view
     */
    void viewChanged(KTextEditor::View *view);

    /**
     * This signal is emitted whenever a new view is created
     * @param view view that was created
     */
    void viewCreated(KTextEditor::View *view);

    //
    // Interface to allow view bars to be constructed in a central place per window
    //
public:
    /**
     * Try to create a view bar for the given view.
     * @param view view for which we want an view bar
     * @return suitable widget that can host view bars widgets or nullptr
     */
    QWidget *createViewBar(KTextEditor::View *view);

    /**
     * Delete the view bar for the given view.
     * @param view view for which we want an view bar
     */
    void deleteViewBar(KTextEditor::View *view);

    /**
     * Add a widget to the view bar.
     * @param view view for which the view bar is used
     * @param bar bar widget, shall have the viewBarParent() as parent widget
     */
    void addWidgetToViewBar(KTextEditor::View *view, QWidget *bar);

    /**
     * Show the view bar for the given view
     * @param view view for which the view bar is used
     */
    void showViewBar(KTextEditor::View *view);

    /**
     * Hide the view bar for the given view
     * @param view view for which the view bar is used
     */
    void hideViewBar(KTextEditor::View *view);

    //
    // ToolView stuff, here all stuff belong which allows to
    // add/remove and manipulate the toolview of this main windows
    //
public:
    /**
     * Toolview position.
     * A toolview can only be at one side at a time.
     */
    enum ToolViewPosition {
        Left = 0, /**< Left side. */
        Right = 1, /**< Right side. */
        Top = 2, /**< Top side. */
        Bottom = 3 /**< Bottom side. */
    };

    /**
     * Create a new toolview with unique \p identifier at side \p pos
     * with \p icon and caption \p text. Use the returned widget to embedd
     * your widgets.
     * \param plugin which owns this tool view
     * \param identifier unique identifier for this toolview
     * \param pos position for the toolview, if we are in session restore,
     *        this is only a preference
     * \param icon icon to use in the sidebar for the toolview
     * \param text translated text (i18n()) to use in addition to icon
     * \return created toolview on success, otherwise NULL
     */
    QWidget *createToolView(KTextEditor::Plugin *plugin,
                            const QString &identifier,
                            KTextEditor::MainWindow::ToolViewPosition pos,
                            const QIcon &icon,
                            const QString &text);

    /**
     * Move the toolview \p widget to position \p pos.
     * \param widget the toolview to move, where the widget was constructed
     *        by createToolView().
     * \param pos new position to move widget to
     * \return \e true on success, otherwise \e false
     */
    bool moveToolView(QWidget *widget, KTextEditor::MainWindow::ToolViewPosition pos);

    /**
     * Show the toolview \p widget.
     * \param widget the toolview to show, where the widget was constructed
     *        by createToolView().
     * \return \e true on success, otherwise \e false
     * \todo add focus parameter: bool showToolView (QWidget *widget, bool giveFocus );
     */
    bool showToolView(QWidget *widget);

    /**
     * Hide the toolview \p widget.
     * \param widget the toolview to hide, where the widget was constructed
     *        by createToolView().
     * \return \e true on success, otherwise \e false
     */
    bool hideToolView(QWidget *widget);

    //
    // Application plugin accessors
    //
public:
    /**
     * Shows the @p plugin's config page. The @p page specifies which
     * config page will be shown, see KTextEditor::Plugin::configPages().
     *
     * \return \e true on success, otherwise \e false
     * \since 5.63
     */
    bool showPluginConfigPage(KTextEditor::Plugin *plugin, int page);

    /**
     * Get a plugin view for the plugin with with identifier \p name.
     * \param name the plugin's name
     * \return pointer to the plugin view if a plugin with \p name is loaded and has a view for this mainwindow,
     *         otherwise NULL
     */
    QObject *pluginView(const QString &name);

    //
    // Signals related to application plugins
    //
Q_SIGNALS:
    /**
     * This signal is emitted when the view of some Plugin is created for this main window.
     *
     * @param name name of plugin
     * @param pluginView the new plugin view
     */
    void pluginViewCreated(const QString &name, QObject *pluginView);

    /**
     * This signal is emitted when the view of some Plugin got deleted.
     *
     * @warning Do not access the data referenced by the pointer, it is already invalid.
     * Use the pointer only to remove mappings in hash or maps
     *
     * @param name name of plugin
     * @param pluginView the deleted plugin view
     */
    void pluginViewDeleted(const QString &name, QObject *pluginView);

    //
    // Custom widget handling
    //
public:
    /**
     * Add a widget to the main window.
     * This is useful to show non-KTextEditor::View widgets in the main window.
     * The host application should try to manage this like some KTextEditor::View (e.g. as a tab) and provide
     * the means to close it.
     * \param widget widget to add
     * \return success, if false, the plugin needs to take care to show the widget itself, otherwise
     *         the main window will take ownership of the widget
     * \since 5.98
     */
    bool addWidget(QWidget *widget);

    /**
     * \brief remove this \p widget from this mainwindow. The widget will be deleted afterwards
     * \param widget the widget to be removed
     * \return true on success
     * \since 6.0
     */
    bool removeWidget(QWidget *widget);

    /**
     * \brief returns the list of non-KTextEditor::View widgets in this main window.
     * \see addWidget
     * \since 6.0
     */
    QWidgetList widgets();

    /**
     * \brief returns the currently active widget. It can be a non-KTextEditor::View widget or a KTextEditor::View
     * \since 6.0
     */
    QWidget *activeWidget();

    /**
     * \brief activate @p widget. If the widget is not present in the window, it will be added to the window
     * \param widget the widget to activate
     * \since 6.0
     */
    void activateWidget(QWidget *widget);

    //
    // Signals related to widgets
    //
Q_SIGNALS:
    /**
     * \brief The @p widget was added to this window.
     * \param widget the widget that got added
     * \since 6.0
     */
    void widgetAdded(QWidget *widget);

    /**
     * \brief The @p widget was removed from this window.
     * \param widget the widget that got removed
     * \since 6.0
     */
    void widgetRemoved(QWidget *widget);

    //
    // Message output
    //
public:
    /**
     * Display a message to the user.
     * The host application might show this inside a dedicated output view.
     *
     * \param message incoming message we shall handle
     * \return true, if the host application was able to handle the message, else false
     * \since 5.98
     *
     * details of message format:
     *
     * message text, will be trimmed before output
     *
     *    message["text"] = i18n("your cool message")
     *
     * the text will be split in lines, all lines beside the first can be collapsed away
     *
     * message type, we support at the moment
     *
     *    message["type"] = "Error"
     *    message["type"] = "Warning"
     *    message["type"] = "Info"
     *    message["type"] = "Log"
     *
     * this is take from https://microsoft.github.io/language-server-protocol/specification#window_showMessage MessageType of LSP
     *
     * will lead to appropriate icons/... in the output view
     *
     * a message should have some category, like Git, LSP, ....
     *
     *    message["category"] = i18n(...)
     *
     * will be used to allow the user to filter for
     *
     * one can additionally provide a categoryIcon
     *
     *    message["categoryIcon"] = QIcon(...)
     *
     * the categoryIcon icon QVariant must contain a QIcon, nothing else!
     *
     * A string token can be passed to allow to replace messages already send out with new ones.
     * That is useful for e.g. progress output
     *
     *     message["token"] = "yourmessagetoken"
     *
     */
    bool showMessage(const QVariantMap &message);

private:
    /**
     * Private d-pointer class is our best friend ;)
     */
    friend class MainWindowPrivate;

    /**
     * Private d-pointer
     */
    class MainWindowPrivate *const d;
};

} // namespace KTextEditor

#endif
