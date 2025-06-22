/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOOLBARHANDLER_H
#define KTOOLBARHANDLER_H

#include <QObject>

#include <kxmlguiclient.h>

class KXmlGuiWindow;

namespace KDEPrivate
{
class ToolBarHandler : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**
     * Creates a new tool bar handler for the supplied
     * @param mainWindow.
     */
    explicit ToolBarHandler(KXmlGuiWindow *mainWindow);

    /**
     * Creates a new tool bar handler for the supplied
     * @param mainWindow and with the supplied parent.
     */
    ToolBarHandler(KXmlGuiWindow *mainWindow, QObject *parent);

    /**
     * Destroys the tool bar handler.
     */
    ~ToolBarHandler() override;

    /**
     * Returns the action which is responsible for the tool bar menu.
     */
    QAction *toolBarMenuAction();

public Q_SLOTS:
    void setupActions();

private Q_SLOTS:
    void clientAdded(KXMLGUIClient *client);

private:
    class Private;
    Private *const d;
};

} // namespace KDEPrivate

#endif // KTOOLBARHANDLER_H
