/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguiversionhandler_p.h"

#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QMap>
#include <QStandardPaths>

struct DocStruct {
    QString file;
    QString data;
};

static QList<QDomElement> extractToolBars(const QDomDocument &doc)
{
    QList<QDomElement> toolbars;
    QDomElement parent = doc.documentElement();
    for (QDomElement e = parent.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName().compare(QStringLiteral("ToolBar"), Qt::CaseInsensitive) == 0) {
            toolbars.append(e);
        }
    }
    return toolbars;
}

static QStringList toolBarNames(const QList<QDomElement> &toolBars)
{
    QStringList names;
    names.reserve(toolBars.count());
    for (const QDomElement &e : toolBars) {
        names.append(e.attribute(QStringLiteral("name")));
    }
    return names;
}

static void removeToolBars(QDomDocument &doc, const QStringList &toolBarNames)
{
    QDomElement parent = doc.documentElement();
    const QList<QDomElement> toolBars = extractToolBars(doc);
    for (const QDomElement &e : toolBars) {
        if (toolBarNames.contains(e.attribute(QStringLiteral("name")))) {
            parent.removeChild(e);
        }
    }
}

static void insertToolBars(QDomDocument &doc, const QList<QDomElement> &toolBars)
{
    QDomElement parent = doc.documentElement();
    QDomElement menuBar = parent.namedItem(QStringLiteral("MenuBar")).toElement();
    QDomElement insertAfter = menuBar;
    if (menuBar.isNull()) {
        insertAfter = parent.firstChildElement(); // if null, insertAfter will do an append
    }
    for (const QDomElement &e : toolBars) {
        QDomNode result = parent.insertAfter(e, insertAfter);
        Q_ASSERT(!result.isNull());
    }
}

//

typedef QMap<QString, QMap<QString, QString>> ActionPropertiesMap;

static ActionPropertiesMap extractActionProperties(const QDomDocument &doc)
{
    ActionPropertiesMap properties;

    QDomElement actionPropElement = doc.documentElement().namedItem(QStringLiteral("ActionProperties")).toElement();

    if (actionPropElement.isNull()) {
        return properties;
    }

    QDomNode n = actionPropElement.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        n = n.nextSibling(); // Advance now so that we can safely delete e
        if (e.isNull()) {
            continue;
        }

        if (e.tagName().compare(QStringLiteral("action"), Qt::CaseInsensitive) != 0) {
            continue;
        }

        const QString actionName = e.attribute(QStringLiteral("name"));
        if (actionName.isEmpty()) {
            continue;
        }

        QMap<QString, QMap<QString, QString>>::Iterator propIt = properties.find(actionName);
        if (propIt == properties.end()) {
            propIt = properties.insert(actionName, QMap<QString, QString>());
        }

        const QDomNamedNodeMap attributes = e.attributes();
        const int attributeslength = attributes.length();

        for (int i = 0; i < attributeslength; ++i) {
            const QDomAttr attr = attributes.item(i).toAttr();

            if (attr.isNull()) {
                continue;
            }

            const QString name = attr.name();

            if (name == QLatin1String("name") || name.isEmpty()) {
                continue;
            }

            (*propIt)[name] = attr.value();
        }
    }

    return properties;
}

static void storeActionProperties(QDomDocument &doc, const ActionPropertiesMap &properties)
{
    QDomElement actionPropElement = doc.documentElement().namedItem(QStringLiteral("ActionProperties")).toElement();

    if (actionPropElement.isNull()) {
        actionPropElement = doc.createElement(QStringLiteral("ActionProperties"));
        doc.documentElement().appendChild(actionPropElement);
    }

    // Remove only those ActionProperties entries from the document, that are present
    // in the properties argument. In real life this means that local ActionProperties
    // takes precedence over global ones, if they exists (think local override of shortcuts).
    QDomNode actionNode = actionPropElement.firstChild();
    while (!actionNode.isNull()) {
        if (properties.contains(actionNode.toElement().attribute(QStringLiteral("name")))) {
            QDomNode nextNode = actionNode.nextSibling();
            actionPropElement.removeChild(actionNode);
            actionNode = nextNode;
        } else {
            actionNode = actionNode.nextSibling();
        }
    }

    ActionPropertiesMap::ConstIterator it = properties.begin();
    const ActionPropertiesMap::ConstIterator end = properties.end();
    for (; it != end; ++it) {
        QDomElement action = doc.createElement(QStringLiteral("Action"));
        action.setAttribute(QStringLiteral("name"), it.key());
        actionPropElement.appendChild(action);

        const QMap<QString, QString> attributes = (*it);
        QMap<QString, QString>::ConstIterator attrIt = attributes.begin();
        const QMap<QString, QString>::ConstIterator attrEnd = attributes.end();
        for (; attrIt != attrEnd; ++attrIt) {
            action.setAttribute(attrIt.key(), attrIt.value());
        }
    }
}

KXmlGuiVersionHandler::KXmlGuiVersionHandler(const QStringList &files)
{
    Q_ASSERT(!files.isEmpty());

    if (files.count() == 1) {
        // No need to parse version numbers if there's only one file anyway
        m_file = files.first();
        m_doc = KXMLGUIFactory::readConfigFile(m_file);
        return;
    }

    std::vector<DocStruct> allDocuments;
    allDocuments.reserve(files.size());

    for (const QString &file : files) {
        allDocuments.push_back({file, KXMLGUIFactory::readConfigFile(file)});
    }

    auto best = allDocuments.end();
    uint bestVersion = 0;

    auto docIt = allDocuments.begin();
    const auto docEnd = allDocuments.end();
    for (; docIt != docEnd; ++docIt) {
        const QString versionStr = KXMLGUIClient::findVersionNumber((*docIt).data);
        if (versionStr.isEmpty()) {
            // qCDebug(DEBUG_KXMLGUI) << "found no version in" << (*docIt).file;
            continue;
        }

        bool ok = false;
        uint version = versionStr.toUInt(&ok);
        if (!ok) {
            continue;
        }
        // qCDebug(DEBUG_KXMLGUI) << "found version" << version << "for" << (*docIt).file;

        if (version > bestVersion) {
            best = docIt;
            // qCDebug(DEBUG_KXMLGUI) << "best version is now " << version;
            bestVersion = version;
        }
    }

    if (best != docEnd) {
        if (best != allDocuments.begin()) {
            auto local = allDocuments.begin();

            if ((*local).file.startsWith(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))) {
                // load the local document and extract the action properties
                QDomDocument localDocument;
                localDocument.setContent((*local).data);

                const ActionPropertiesMap properties = extractActionProperties(localDocument);
                const QList<QDomElement> toolbars = extractToolBars(localDocument);

                // in case the document has a ActionProperties section
                // we must not delete it but copy over the global doc
                // to the local and insert the ActionProperties section

                // TODO: kedittoolbar should mark toolbars as modified so that
                // we don't keep old toolbars just because the user defined a shortcut

                if (!properties.isEmpty() || !toolbars.isEmpty()) {
                    // now load the global one with the higher version number
                    // into memory
                    QDomDocument document;
                    document.setContent((*best).data);
                    // and store the properties in there
                    storeActionProperties(document, properties);
                    if (!toolbars.isEmpty()) {
                        // remove application toolbars present in the user file
                        // (not others, that the app might have added since)
                        removeToolBars(document, toolBarNames(toolbars));
                        // add user toolbars
                        insertToolBars(document, toolbars);
                    }

                    (*local).data = document.toString();
                    // make sure we pick up the new local doc, when we return later
                    best = local;

                    // write out the new version of the local document
                    QFile f((*local).file);
                    if (f.open(QIODevice::WriteOnly)) {
                        const QByteArray utf8data = (*local).data.toUtf8();
                        f.write(utf8data.constData(), utf8data.length());
                        f.close();
                    }
                } else {
                    // Move away the outdated local file, to speed things up next time
                    const QString f = (*local).file;
                    const QString backup = f + QLatin1String(".backup");
                    QFile::rename(f, backup);
                }
            }
        }
        m_doc = (*best).data;
        m_file = (*best).file;
    } else {
        // qCDebug(DEBUG_KXMLGUI) << "returning first one...";
        const auto &[file, data] = allDocuments.at(0);
        m_file = file;
        m_doc = data;
    }
}
