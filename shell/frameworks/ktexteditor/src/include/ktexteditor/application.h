/*
    SPDX-FileCopyrightText: 2013 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_APPLICATION_H
#define KTEXTEDITOR_APPLICATION_H

#include <ktexteditor_export.h>

#include <QObject>

namespace KTextEditor
{
class Plugin;
class Document;
class MainWindow;

/**
 * \class Application application.h <KTextEditor/Application>
 *
 * This class allows the application that embeds the KTextEditor component to
 * allow it access to application wide information and interactions.
 *
 * For example the component can get the current active main window of the application.
 *
 * The application must pass a pointer to the Application object to the setApplication method of the
 * global editor instance and ensure that this object stays valid for the complete lifetime of the editor.
 *
 * It must not reimplement this class but construct an instance and pass a pointer to a QObject that
 * has the required slots to receive the requests.
 *
 * KTextEditor::Editor::instance()->application() will always return a non-nullptr object
 * to avoid the need for nullptr checks before calling the API.
 *
 * The same holds for activeMainWindow(), even if no main window is around, you will get a non-nullptr
 * interface object that allows to call the functions of the MainWindow without needs for a nullptr
 * check around it in the client code.
 *
 * @ref kte_plugin_hosting
 */
class KTEXTEDITOR_EXPORT Application : public QObject
{
    Q_OBJECT

public:
    /**
     * Construct an Application wrapper object.
     * The passed parent is both the parent of this QObject and the receiver of all interface
     * calls via invokeMethod.
     * @param parent object the calls are relayed to
     */
    Application(QObject *parent);

    /**
     * Virtual Destructor
     */
    ~Application() override;

    /**
     * Ask app to quit. The app might interact with the user and decide that
     * quitting is not possible and return false.
     *
     * \return true if the app could quit
     */
    bool quit();

    //
    // MainWindow related accessors
    //
public:
    /**
     * Get a list of all main windows.
     * @return all main windows, might be empty!
     */
    QList<KTextEditor::MainWindow *> mainWindows();

    /**
     * Accessor to the active main window.
     * \return a pointer to the active mainwindow, even if no main window is active you
     *         will get a non-nullptr dummy interface that allows you to call interface functions
     *         without the need for null checks
     */
    KTextEditor::MainWindow *activeMainWindow();

    //
    // Document related accessors
    //
public:
    /**
     * Get a list of all documents that are managed by the application.
     * This might contain less documents than the editor has in his documents () list.
     * @return all documents the application manages, might be empty!
     */
    QList<KTextEditor::Document *> documents();

    /**
     * Get the document with the URL \p url.
     * if multiple documents match the searched url, return the first found one...
     * \param url the document's URL
     * \return the document with the given \p url or nullptr, if none found
     */
    KTextEditor::Document *findUrl(const QUrl &url);

    /**
     * Open the document \p url with the given \p encoding.
     * if the url is empty, a new empty document will be created
     * \param url the document's url
     * \param encoding the preferred encoding. If encoding is QString() the
     *        encoding will be guessed or the default encoding will be used.
     * \return a pointer to the created document
     */
    KTextEditor::Document *openUrl(const QUrl &url, const QString &encoding = QString());

    /**
     * Close the given \p document. If the document is modified, user will be asked if he wants that.
     * \param document the document to be closed
     * \return \e true on success, otherwise \e false
     */
    bool closeDocument(KTextEditor::Document *document);

    /**
     * Close a list of documents. If any of them are modified, user will be asked if he wants that.
     * Use this, if you want to close multiple documents at once, as the application might
     * be able to group the "do you really want that" dialogs into one.
     * \param documents list of documents to be closed
     * \return \e true on success, otherwise \e false
     */
    bool closeDocuments(const QList<KTextEditor::Document *> &documents);

Q_SIGNALS:
    /**
     * This signal is emitted when the \p document was created.
     *
     * @param document document that was created
     */
    void documentCreated(KTextEditor::Document *document);

    /**
     * This signal is emitted before a \p document which should be closed is deleted
     * The document is still accessible and usable, but it will be deleted
     * after this signal was send.
     *
     * @param document document that will be deleted
     */
    void documentWillBeDeleted(KTextEditor::Document *document);

    /**
     * This signal is emitted when the \p document has been deleted.
     *
     * @warning Do not access the data referenced by the pointer, it is already invalid.
     * Use the pointer only to remove mappings in hash or maps
     *
     * @param document document that is deleted
     */
    void documentDeleted(KTextEditor::Document *document);

    //
    // Application plugin accessors
    //
public:
    /**
     * Get a plugin for the plugin with with identifier \p name.
     * \param name the plugin's name
     * \return pointer to the plugin if a plugin with \p name is loaded, otherwise nullptr
     */
    KTextEditor::Plugin *plugin(const QString &name);

    //
    // Signals related to application plugins
    //
Q_SIGNALS:
    /**
     * This signal is emitted when an Plugin was loaded.
     *
     * @param name name of plugin
     * @param plugin the new plugin
     */
    void pluginCreated(const QString &name, KTextEditor::Plugin *plugin);

    /**
     * This signal is emitted when an Plugin got deleted.
     *
     * @param name name of plugin
     * @param plugin the deleted plugin
     *
     * @warning Do not access the data referenced by the pointer, it is already invalid.
     * Use the pointer only to remove mappings in hash or maps
     */
    void pluginDeleted(const QString &name, KTextEditor::Plugin *plugin);

private:
    /**
     * Private d-pointer class is our best friend ;)
     */
    friend class ApplicationPrivate;

    /**
     * Private d-pointer
     */
    class ApplicationPrivate *const d;
};

} // namespace KTextEditor

#endif
