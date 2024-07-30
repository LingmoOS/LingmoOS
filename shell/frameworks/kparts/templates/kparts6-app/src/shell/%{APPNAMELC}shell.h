/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef %{APPNAMEUC}SHELL_H
#define %{APPNAMEUC}SHELL_H

// KF headers
#include <KParts/MainWindow>

namespace KParts {
class ReadWritePart;
}

/**
 * @short %{APPNAME} Shell
 */
class %{APPNAME}Shell : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    %{APPNAME}Shell();

    /**
     * Default Destructor
     */
    ~%{APPNAME}Shell() override;

    /**
     * Use this method to load whatever file/URL you have
     * @param url document to load
     */
    void loadDocument(const QUrl& url);

private Q_SLOTS:
    void fileNew();
    void fileOpen();

private:
    void setupActions();

private:
    KParts::ReadWritePart* m_part;
};

#endif // %{APPNAMEUC}_H
