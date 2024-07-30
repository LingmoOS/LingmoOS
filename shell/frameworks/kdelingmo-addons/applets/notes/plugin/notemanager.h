/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef NOTEMANAGER_H
#define NOTEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QSharedPointer>

#include "abstractnoteloader.h"
class Note;

class NoteManager : public QObject
{
    Q_OBJECT

public:
    explicit NoteManager(QObject *parent = nullptr);

    /**
     * Loads the note for the ID given
     * Ownership is passed to the QML context
     */
    Q_INVOKABLE Note *loadNote(const QString &id);

    /**
     * Remove any resources associated with the note ID
     */
    Q_INVOKABLE void deleteNoteResources(const QString &id);

    // LATER QAbstractListModel* notesModel(); //list of all notes

private:
    // ref count backends so that we only have for all notes
    static QSharedPointer<AbstractNoteLoader> loadBackend();

    QSharedPointer<AbstractNoteLoader> m_backend;
    QWeakPointer<Note> m_lastNote;
};

#endif // NOTEMANAGER_H
