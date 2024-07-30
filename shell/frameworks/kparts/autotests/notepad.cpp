/*
    SPDX-FileCopyrightText: 1999, 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "notepad.h"
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>

#include <QAction>
#include <QDebug>
#include <QFile>
#include <QTextEdit>
#include <QTextStream>

#include <KAboutData>
#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(NotepadPart, "notepad.json")

NotepadPart::NotepadPart(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData)
    : KParts::ReadWritePart(parent, metaData)
{
    m_edit = new QTextEdit(parentWidget);
    m_edit->setPlainText(QStringLiteral("NotepadPart's multiline edit"));
    setWidget(m_edit);

    QAction *searchReplace = new QAction(QStringLiteral("Search and replace"), this);
    actionCollection()->addAction(QStringLiteral("searchreplace"), searchReplace);

    setXMLFile(QStringLiteral("notepadpart.rc")); // will be found in the qrc resource

    setReadWrite(true);
}

NotepadPart::~NotepadPart()
{
}

void NotepadPart::setReadWrite(bool rw)
{
    m_edit->setReadOnly(!rw);
    if (rw) {
        connect(m_edit, &QTextEdit::textChanged, this, qOverload<>(&KParts::ReadWritePart::setModified));
    } else {
        disconnect(m_edit, &QTextEdit::textChanged, this, qOverload<>(&KParts::ReadWritePart::setModified));
    }

    ReadWritePart::setReadWrite(rw);
}

bool NotepadPart::openFile()
{
    // qDebug() << "NotepadPart: opening " << localFilePath();
    QFile f(localFilePath());
    QString s;
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream t(&f);
        // The default with Qt6 is UTF-8
        s = t.readAll();
        f.close();
    }
    m_edit->setPlainText(s);

    Q_EMIT setStatusBarText(url().toString());

    return true;
}

bool NotepadPart::saveFile()
{
    if (!isReadWrite()) {
        return false;
    }
    QFile f(localFilePath());
    if (f.open(QIODevice::WriteOnly)) {
        QTextStream t(&f);
        t << m_edit->toPlainText();
        f.close();
        return true;
    } else {
        return false;
    }
}

#include "moc_notepad.cpp"
#include "notepad.moc"
