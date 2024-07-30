/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef FILESYSTEMNOTELOADER_H
#define FILESYSTEMNOTELOADER_H

#include "abstractnoteloader.h"

#include <QDir>
#include <QObject>

class FileSystemNoteLoader : public AbstractNoteLoader
{
public:
    explicit FileSystemNoteLoader();
    QStringList allNoteIds() override;
    Note *loadNote(const QString &id) override;
    void deleteNoteResources(const QString &id) override;

private:
    QDir m_notesDir;
};

#endif // FILESYSTEMNOTEMANAGER_H
