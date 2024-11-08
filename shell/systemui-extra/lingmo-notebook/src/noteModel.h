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

#ifndef NOTEMODEL_H
#define NOTEMODEL_H

#include <QAbstractListModel>

#include "noteData.h"

class NoteModel : public QAbstractListModel
{

    friend class tst_NoteModel;

public:

    enum NoteRoles{
        NoteID = Qt::UserRole + 1,
        NoteFullTitle,
        NoteCreationDateTime,
        NoteLastModificationDateTime,
        NoteDeletionDateTime,
        NoteContent,
        NoteScrollbarPos,
        NoteColor,
        NoteMdContent,
    };

    explicit NoteModel(QObject *parent = Q_NULLPTR);
    ~NoteModel();

public:
    QModelIndex addNote(NoteData* note);
    QModelIndex insertNote(NoteData* note, int row);
    NoteData* getNote(const QModelIndex& index);
    void addListNote(QList<NoteData*> noteList);
    NoteData* removeNote(const QModelIndex& noteIndex);
    bool moveRow(const QModelIndex& sourceParent,
                 int sourceRow,
                 const QModelIndex& destinationParent,
                 int destinationChild);

    void clearNotes();

public:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    void sort(int column, Qt::SortOrder order) Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const override;

private:
    QList<NoteData *> m_noteList;

signals:
    void noteRemoved();
};

#endif // NOTEMODEL_H
