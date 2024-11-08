/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QtSql/QSqlDatabase>

#include "noteData.h"

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = Q_NULLPTR);

private:

    void open(const QString& path, bool doCreate = false);
    void createTables();
    int  getLastRowID();
    bool forceLastRowIndexValue(const int indexValue);

    NoteData* getNote(QString id);
    bool isNoteExist(NoteData* note);

    QList<NoteData *> getAllNotes();
    bool addNote(NoteData* note);
    bool removeNote(NoteData* note);
    bool updateNote(NoteData* note);
    bool migrateNote(NoteData* note);
    bool migrateTrash(NoteData* note);

signals:
    void notesReceived(QList<NoteData*> noteList, int noteCounter);

public slots:
    bool permanantlyRemoveAllNotes();
    void onNotesListRequested();
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequested(NoteData* note);
    void onDeleteNoteRequested(NoteData* note);
    void onImportNotesRequested(QList<NoteData *> noteList);
    void onRestoreNotesRequested(QList<NoteData *> noteList);
    void onExportNotesRequested(QString fileName);
    void onMigrateNotesRequested(QList<NoteData *> noteList);
    void onMigrateTrashRequested(QList<NoteData *> noteList);
    void onForceLastRowIndexValueRequested(int index);
};

#endif // DBMANAGER_H
