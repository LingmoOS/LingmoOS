/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "%{APPNAMELC}plugin.h"

#include "%{APPNAMELC}view.h"

// KF headers
#include <KTextEditor/MainWindow>

#include <KPluginFactory>
#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(%{APPNAME}Plugin, "%{APPNAMELC}.json")


%{APPNAME}Plugin::%{APPNAME}Plugin(QObject* parent, const QVariantList& /*args*/)
    : KTextEditor::Plugin(parent)
{
}

%{APPNAME}Plugin::~%{APPNAME}Plugin()
{
}

QObject* %{APPNAME}Plugin::createView(KTextEditor::MainWindow* mainwindow)
{
    return new %{APPNAME}View(this, mainwindow);
}


// needed for K_PLUGIN_CLASS_WITH_JSON
#include <%{APPNAMELC}plugin.moc>

#include "moc_%{APPNAMELC}plugin.cpp"
