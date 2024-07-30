/*
    SPDX-FileCopyrightText: 2001-2014 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2005-2014 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_PLUGIN_H
#define KTEXTEDITOR_PLUGIN_H

#include <QObject>

#include <ktexteditor_export.h>

namespace KTextEditor
{
class ConfigPage;
class MainWindow;

/**
 * \class Plugin plugin.h <KTextEditor/Plugin>
 *
 * \brief KTextEditor Plugin interface.
 *
 * Topics:
 *  - \ref plugin_intro
 *  - \ref plugin_config_pages
 *  - \ref plugin_sessions
 *
 * \section plugin_intro Introduction
 *
 * The Plugin class provides methods to create loadable plugins for the
 * KTextEditor framework. The Plugin class itself a function createView()
 * that is called for each MainWindow. In createView(), the plugin is
 * responsible to create tool views through MainWindow::createToolView(),
 * hook itself into the menus and toolbars through KXMLGuiClient, and attach
 * itself to View%s or Document%s.
 *
 * \section plugin_config_pages Plugin Config Pages
 *
 * If your plugin needs config pages, override the functions configPages()
 * and configPage() in your plugin as follows:
 * \code
 * class MyPlugin : public KTextEditor::Plugin
 * {
 *     Q_OBJECT
 * public:
 *     // ...
 *     int configPages() const override;
 *     ConfigPage *configPage(int number, QWidget *parent) override;
 * };
 * \endcode
 * The host application will call configPage() for each config page.
 *
 * \section plugin_sessions Session Management
 *
 * As an extension a Plugin can implement the SessionConfigInterface. This
 * interface provides functions to read and write session related settings.
 * If you have session dependent data additionally derive your Plugin from
 * this interface and implement the session related functions, for example:
 * \code
 * class MyPlugin : public KTextEditor::Plugin,
 *                  public KTextEditor::SessionConfigInterface
 * {
 *     Q_OBJECT
 *     Q_INTERFACES(KTextEditor::SessionConfigInterface)
 * public:
 *     // ...
 *     void readSessionConfig (const KConfigGroup& config) override;
 *     void writeSessionConfig (KConfigGroup& config) override;
 * };
 * \endcode
 *
 * \see KTextEditor::SessionConfigInterface, KTextEditor::ConfigPage,
 *      KTextEditor::MainWindow, \ref kte_plugin_hosting
 * \author Christoph Cullmann \<cullmann@kde.org\>
 */
class KTEXTEDITOR_EXPORT Plugin : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * Create a new application plugin.
     * \param parent parent object
     */
    Plugin(QObject *parent);

    /**
     * Virtual destructor.
     */
    ~Plugin() override;

    /**
     * Create a new View for this plugin for the given KTextEditor::MainWindow.
     * This may be called arbitrary often by the application to create as much
     * views as MainWindow%s exist. The application will take care to delete
     * a view whenever a MainWindow is closed, so you do not need to handle
     * deletion of the view yourself in the plugin.
     *
     * \note Session configuration: The host application will try to cast the
     *       returned QObject with \p qobject_cast into the SessionConfigInterface.
     *       This way, not only your Plugin, but also each Plugin view can have
     *       session related settings.
     *
     * \param mainWindow the MainWindow for which a view should be created
     * \return the new created view or NULL
     * @see SessionConfigInterface
     */
    virtual QObject *createView(KTextEditor::MainWindow *mainWindow) = 0;

    /**
     * Get the number of available config pages.
     * If a number < 1 is returned, it does not support config pages.
     * \return number of config pages, default implementation says 0
     * \see configPage()
     */
    virtual int configPages() const;

    /**
     * Get the config page with the \p number, config pages from 0 to
     * configPages()-1 are available if configPages() > 0.
     * \param number index of config page
     * \param parent parent widget for config page
     * \return created config page or NULL, if the number is out of bounds, default implementation returns NULL
     * \see configPages()
     */
    virtual ConfigPage *configPage(int number, QWidget *parent);

private:
    class PluginPrivate *const d;
};

}

#endif
