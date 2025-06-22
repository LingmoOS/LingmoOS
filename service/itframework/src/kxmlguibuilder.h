/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kxmlguibuilder_h
#define kxmlguibuilder_h

#include <kxmlgui_export.h>
#include <memory>

#include <QStringList>

class KXMLGUIBuilderPrivate;
class KXMLGUIClient;

class QAction;
class QDomElement;
class QWidget;

/**
 * @class KXMLGUIBuilder kxmlguibuilder.h KXMLGUIBuilder
 *
 * Implements the creation of the GUI (menubar, menus and toolbars)
 * as requested by the GUI factory.
 *
 * The virtual methods are mostly for historical reasons, there isn't really
 * a need to derive from KXMLGUIBuilder anymore.
 */
class KXMLGUI_EXPORT KXMLGUIBuilder
{
public:
    explicit KXMLGUIBuilder(QWidget *widget);
    virtual ~KXMLGUIBuilder();

    /* @internal */
    KXMLGUIClient *builderClient() const;
    /* @internal */
    void setBuilderClient(KXMLGUIClient *client);
    /* @internal */
    QWidget *widget();

    virtual QStringList containerTags() const;

    /**
     * Creates a container (menubar/menu/toolbar/statusbar/separator/...)
     * from an element in the XML file
     *
     * @param parent The parent for the container
     * @param index The index where the container should be inserted
     *              into the parent container/widget
     * @param element The element from the DOM tree describing the
     *                container (use it to access container specified
     *                attributes or child elements)
     * @param action The action created for this container; used for e.g. passing to removeContainer.
     */
    virtual QWidget *createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction);

    /**
     * Removes the given (and previously via createContainer )
     * created container.
     *
     */
    virtual void removeContainer(QWidget *container, QWidget *parent, QDomElement &element, QAction *containerAction);

    virtual QStringList customTags() const;

    virtual QAction *createCustomElement(QWidget *parent, int index, const QDomElement &element);

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
    // KF6 TODO: REMOVE
    /// @internal
    /// @deprecated Since 5.0, do not use
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Do not use")
    virtual void removeCustomElement(QWidget *parent, QAction *action);
#endif

    virtual void finalizeGUI(KXMLGUIClient *client);

protected:
    virtual void virtual_hook(int id, void *data);

private:
    std::unique_ptr<KXMLGUIBuilderPrivate> const d;
};

#endif
