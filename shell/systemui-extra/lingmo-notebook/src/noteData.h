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

#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <QObject>
#include <QDateTime>

#include "editPage.h"

class NoteData : public QObject
{
    Q_OBJECT

    friend class tst_NoteData;

public:
    explicit NoteData(QObject *parent = Q_NULLPTR);

    int id() const;
    void setId(const int& id);

    int notecolor() const;
    void setNoteColor(const int &notecolor);

    QString fullTitle() const;
    void setFullTitle(const QString &fullTitle);

    QDateTime lastModificationdateTime() const;
    void setLastModificationDateTime(const QDateTime &lastModificationdateTime);

    QDateTime creationDateTime() const;
    void setCreationDateTime(const QDateTime& creationDateTime);

    QString content() const;
    void setContent(const QString &content);

    QString mdContent() const;
    void setMdContent(const QString &mdContent);

    bool isModified() const;
    void setModified(bool isModified);

    bool isSelected() const;
    void setSelected(bool isSelected);

    int scrollBarPosition() const;
    void setScrollBarPosition(int scrollBarPosition);

    QDateTime deletionDateTime() const;
    void setDeletionDateTime(const QDateTime& deletionDateTime);
    EditPage *m_note;

private:
    int m_id;
    int m_notecolor;
    QString m_fullTitle;
    QDateTime m_lastModificationDateTime;
    QDateTime m_creationDateTime;
    QDateTime m_deletionDateTime;
    QString m_content;
    QString m_mdContent;
    bool m_isModified;
    bool m_isSelected;
    int m_scrollBarPosition;

};

QDataStream &operator<<(QDataStream &stream, const NoteData* noteData);
QDataStream &operator>>(QDataStream &stream, NoteData *&noteData);

#endif // NOTEDATA_H
