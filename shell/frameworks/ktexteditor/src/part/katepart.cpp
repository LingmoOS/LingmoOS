/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katedocument.h"

#include <KPluginFactory>

/**
 * wrapper factory to be sure nobody external deletes our kateglobal object
 * each instance will just increment the reference counter of our internal
 * super private global instance ;)
 */
class KateFactory : public KPluginFactory
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID KPluginFactory_iid FILE "katepart.json")

    Q_INTERFACES(KPluginFactory)

public:
    /**
     * This function is called when the factory asked to create an Object.
     *
     * You may reimplement it to provide a very flexible factory. This is especially useful to
     * provide generic factories for plugins implemented using a scripting language.
     *
     * \param iface The staticMetaObject::className() string identifying the plugin interface that
     * was requested. E.g. for KCModule plugins this string will be "KCModule".
     * \param parentWidget Only used if the requested plugin is a KPart.
     * \param parent The parent object for the plugin object.
     * \param args A plugin specific list of arbitrary arguments.
     */
    QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &) override
    {
        // iface == classname to construct
        const QByteArray classname(iface);

        // default to the kparts::* behavior of having one single widget() if the user don't requested a pure document
        const bool bWantSingleView = (classname != "KTextEditor::Document");

        // should we be readonly?
        const bool bWantReadOnly = (classname == "KParts::ReadOnlyPart");

        // construct right part variant
        KTextEditor::DocumentPrivate *part = new KTextEditor::DocumentPrivate(metaData(), bWantSingleView, bWantReadOnly, parentWidget, parent);
        part->setReadWrite(!bWantReadOnly);
        return part;
    }
};

#include "katepart.moc"
