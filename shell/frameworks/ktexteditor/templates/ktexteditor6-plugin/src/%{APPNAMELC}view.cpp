/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "%{APPNAMELC}view.h"

#include "%{APPNAMELC}plugin.h"

// KF headers
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/MainWindow>

#include <KLocalizedString>


%{APPNAME}View::%{APPNAME}View(%{APPNAME}Plugin* plugin, KTextEditor::MainWindow* mainwindow)
    : QObject(mainwindow)
{
    Q_UNUSED(plugin);
}

%{APPNAME}View::~%{APPNAME}View()
{
}

#include "moc_%{APPNAMELC}view.cpp"
