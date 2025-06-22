/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TESTGUICLIENT_H
#define TESTGUICLIENT_H

#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include <QDebug>

// because setDOMDocument and setXML are protected
class TestGuiClient : public KXMLGUIClient
{
public:
    explicit TestGuiClient(const QByteArray &xml = QByteArray())
        : KXMLGUIClient()
    {
        if (!xml.isNull()) {
            setXML(QString::fromLatin1(xml));
        }
    }
    void setXMLFilePublic(const QString &file, bool merge = false, bool setXMLDoc = true)
    {
        setXMLFile(file, merge, setXMLDoc);
    }
    void setLocalXMLFilePublic(const QString &file)
    {
        setLocalXMLFile(file);
    }
    void createGUI(const QByteArray &xml, bool withUiStandards = false)
    {
        if (withUiStandards) {
            setXMLFile(KXMLGUIClient::standardsXmlFileLocation());
        }

        setXML(QString::fromLatin1(xml), true);
    }
    void createActions(const QStringList &actionNames)
    {
        KActionCollection *coll = actionCollection();
        for (const QString &actionName : actionNames) {
            coll->addAction(actionName)->setText(QStringLiteral("Action"));
        }
    }

    // Find a toolbar (created by this guiclient)
    KToolBar *toolBarByName(const QString &name)
    {
        // qDebug() << "containers:" << factory()->containers("ToolBar");
        QWidget *toolBarW = factory()->container(name, this);
        if (!toolBarW) {
            qWarning() << "No toolbar found with name" << name;
        }
        Q_ASSERT(toolBarW);
        KToolBar *toolBar = qobject_cast<KToolBar *>(toolBarW);
        Q_ASSERT(toolBar);
        return toolBar;
    }
};

#endif /* TESTGUICLIENT_H */
