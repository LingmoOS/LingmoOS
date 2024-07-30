/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef ABSTRACTNOTELOADER_H
#define ABSTRACTNOTELOADER_H
#include <QStringList>

class QString;
class Note;

class AbstractNoteLoader
{
public:
    explicit AbstractNoteLoader();
    virtual ~AbstractNoteLoader();

    virtual QStringList allNoteIds() = 0;
    virtual Note *loadNote(const QString &id) = 0;
    virtual void deleteNoteResources(const QString &id) = 0;

private:
};

#endif // ABSTRACTNOTEMANAGER_H
