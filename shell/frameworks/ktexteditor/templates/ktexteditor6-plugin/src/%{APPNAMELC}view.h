/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef %{APPNAMEUC}VIEW_H
#define %{APPNAMEUC}VIEW_H

// Qt headers
#include <QObject>

namespace KTextEditor {
class MainWindow;
}

class %{APPNAME}Plugin;

class %{APPNAME}View: public QObject
{
    Q_OBJECT

public:
    %{APPNAME}View(%{APPNAME}Plugin* plugin, KTextEditor::MainWindow *view);
    ~%{APPNAME}View() override;
};

#endif // %{APPNAMEUC}VIEW_H
