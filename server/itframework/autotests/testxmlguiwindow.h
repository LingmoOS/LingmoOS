/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TESTXMLGUIWINDOW_H
#define TESTXMLGUIWINDOW_H

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QResizeEvent>
#include <QTemporaryFile>
#include <QTest>

#include <kactioncollection.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>
#include <kxmlguiwindow.h>

class TestXmlGuiWindow : public KXmlGuiWindow
{
public:
    TestXmlGuiWindow(const QByteArray &xml, const char *localXmlFileName)
        : KXmlGuiWindow()
    {
        QVERIFY(m_userFile.open());
        m_userFile.write(xml);
        m_fileName = m_userFile.fileName(); // remember filename
        Q_ASSERT(!m_fileName.isEmpty());
        m_userFile.close(); // write to disk
        // just so that we can use kedittoolbar (because m_fileName is absolute)
        setLocalXMLFile(QString::fromLatin1(localXmlFileName));
    }
    void createGUI()
    {
        // This merges in ui_standards.rc, too.
        KXmlGuiWindow::createGUI(m_fileName);
    }
    void createGUIBad()
    {
        KXmlGuiWindow::createGUI(QStringLiteral("dontexist.rc"));
    }

    // Same as in KMainWindow_UnitTest
    void reallyResize(int width, int height)
    {
        const QSize oldSize = size();
        resize(width, height);
        // Send the pending resize event (resize() only sets Qt::WA_PendingResizeEvent)
        QResizeEvent e(size(), oldSize);
        QApplication::sendEvent(this, &e);
    }

    // KMainWindow::toolBar(name) creates it if not found, and we don't want that.
    // Also this way we test container() rather than just doing a findChild.
    KToolBar *toolBarByName(const QString &name)
    {
        KXMLGUIFactory *factory = guiFactory();
        // qDebug() << "containers:" << factory->containers("ToolBar");
        QWidget *toolBarW = factory->container(name, this);
        if (!toolBarW) {
            qWarning() << "No toolbar found with name" << name;
        }
        Q_ASSERT(toolBarW);
        KToolBar *toolBar = qobject_cast<KToolBar *>(toolBarW);
        Q_ASSERT(toolBar);
        return toolBar;
    }
    QMenu *menuByName(const QString &name)
    {
        KXMLGUIFactory *factory = guiFactory();
        QWidget *menuW = factory->container(name, this);
        Q_ASSERT(menuW);
        QMenu *menu = qobject_cast<QMenu *>(menuW);
        Q_ASSERT(menu);
        return menu;
    }

    void createActions(const QStringList &actionNames)
    {
        KActionCollection *coll = actionCollection();
        for (const QString &actionName : actionNames) {
            coll->addAction(actionName)->setText(QStringLiteral("Action"));
        }
    }

private:
    QTemporaryFile m_userFile;
    QString m_fileName;
};

#endif /* TESTXMLGUIWINDOW_H */
