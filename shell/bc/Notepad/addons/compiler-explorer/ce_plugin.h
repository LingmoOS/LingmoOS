/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef COMPILER_EXPLORER_PLUGIN_H
#define COMPILER_EXPLORER_PLUGIN_H

#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>

class CEPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Plugin constructor.
     */
    explicit CEPlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());

    ~CEPlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

/**
 * Plugin view to merge the actions into the UI
 */
class CEPluginView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * Construct plugin view
     * @param plugin our plugin
     * @param mainwindows the mainwindow for this view
     */
    explicit CEPluginView(CEPlugin *plugin, KTextEditor::MainWindow *mainwindow);

    /**
     * Our Destructor
     */
    ~CEPluginView() override;

private Q_SLOTS:
    void openANewTab();

private:
    /**
     * the main window we belong to
     */
    KTextEditor::MainWindow *m_mainWindow;
    class CEWidget *m_mainWidget;
};

#endif
