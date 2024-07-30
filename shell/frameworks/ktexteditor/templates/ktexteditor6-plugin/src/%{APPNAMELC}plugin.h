/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef %{APPNAMEUC}PLUGIN_H
#define %{APPNAMEUC}PLUGIN_H

// KF headers
#include <KTextEditor/Plugin>

class %{APPNAME}Plugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Default constructor, with arguments as expected by KPluginFactory
     */
    %{APPNAME}Plugin(QObject* parent, const QVariantList& args);

    ~%{APPNAME}Plugin() override;

public: // KTextEditor::Plugin API
    QObject* createView(KTextEditor::MainWindow* mainWindow) override;
};

#endif // %{APPNAMEUC}PLUGIN_H
