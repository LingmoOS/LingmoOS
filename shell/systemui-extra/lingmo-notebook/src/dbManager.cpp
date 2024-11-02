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

#include <QtSql/QSqlQuery>
#include <QTimeZone>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QtConcurrent>

#include "dbManager.h"

/*!
 * \brief DBManager::DBManager
 * \param parent
 */
DBManager::DBManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<NoteData*> >("QList<NoteData*>");
}

/*!
 * \brief DBManager::open
 * \param path
 * \param doCreate
 */
void DBManager::open(const QString &path, bool doCreate)
{
    QSqlDatabase m_db;
    m_db = QSqlDatabase::addDatabase("QSQLITE","note");

    m_db.setDatabaseName(path);
    if(!m_db.open()){
        qDebug() << "Error: connection with database fail";
    }else{
        qDebug() << "Database: connection ok";
    }

    if(doCreate)
        createTables();
}

/*!
 * \brief DBManager::createTables
 */
void DBManager::createTables()
{
    QSqlQuery query(QSqlDatabase::database("note"));
    QString active = "CREATE TABLE active_notes ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "creation_date INTEGER NOT NULL DEFAULT (0),"
                     "modification_date INTEGER NOT NULL DEFAULT (0),"
                     "deletion_date INTEGER NOT NULL DEFAULT (0),"
                     "content TEXT, "
                     "full_title TEXT,"
                     "note_color INTEGER DEFAULT (0),"
                     "md_content TEXT);";

    query.exec(active);
    //CREATE UNIQUE INDEX 用于在表中创建唯一索引,这意味着两个行不能拥有相同的索引值
    // ASC: 升序  DESC: 降序
    //CREATE UNIQUE INDEX active_index on active_notes (id ASC);
    QString active_index = "CREATE UNIQUE INDEX active_index on active_notes (id ASC);";
    query.exec(active_index);

    QString deleted = "CREATE TABLE deleted_notes ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                      "creation_date INTEGER NOT NULL DEFAULT (0),"
                      "modification_date INTEGER NOT NULL DEFAULT (0),"
                      "deletion_date INTEGER NOT NULL DEFAULT (0),"
                      "content TEXT,"
                      "full_title TEXT,"
                      "note_color INTEGER DEFAULT (0),"
                      "md_content TEXT);";
    query.exec(deleted);
}

/*!
 * \brief DBManager::getLastRowID
 * \return
 */
int DBManager::getLastRowID()
{
    QSqlQuery query(QSqlDatabase::database("note"));
    //sqlite_sequence表也是SQLite的系统表,数据库被创建时，sqlite_sequence表会被自动创建
    //该表包括两列。第一列为name，用来存储表的名称。第二列为seq，用来保存表对应的RowID的最大值
    //当对应的表增加记录，该表会自动更新。当表删除，该表对应的记录也会自动删除。如果该值超过最大值，会引起SQL_FULL错误。
    query.exec("SELECT seq from SQLITE_SEQUENCE WHERE name='active_notes';");
    query.next();
    return query.value(0).toInt();
}

/*!
 * \brief DBManager::forceLastRowIndexValue
 * \param indexValue
 * \return
 */
bool DBManager::forceLastRowIndexValue(const int indexValue)
{
    QSqlQuery query(QSqlDatabase::database("note"));
    QString queryStr = QStringLiteral("UPDATE SQLITE_SEQUENCE "
                                      "SET seq=%1 "
                                      "WHERE name='active_notes';").arg(indexValue);
    query.exec(queryStr);
    //Returns the number of rows affected by the result's SQL statement
    return query.numRowsAffected() == 1;
}

/*!
 * \brief DBManager::getNote
 * \param id
 * \return
 */
NoteData* DBManager::getNote(QString id)
{
    QSqlQuery query(QSqlDatabase::database("note"));

    int parsedId = id.split('_')[1].toInt();
    QString queryStr = QStringLiteral("SELECT * FROM active_notes WHERE id = %1 LIMIT 1").arg(parsedId);
    query.exec(queryStr);
    //检索结果中的第一条记录（如果有），并将查询放在检索到的记录上
    if (query.first()) {
        NoteData* note = new NoteData(this->parent() == Q_NULLPTR ? Q_NULLPTR : this);
        int id =  query.value(0).toInt();
        qint64 epochDateTimeCreation = query.value(1).toLongLong();
        QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
        qint64 epochDateTimeModification= query.value(2).toLongLong();
        QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
        QString content = query.value(4).toString();
        QString fullTitle = query.value(5).toString();
        int notecolor = query.value(6).toInt();
        QString mdContent = query.value(7).toString();

        note->setId(id);
        note->setCreationDateTime(dateTimeCreation);
        note->setLastModificationDateTime(dateTimeModification);
        note->setContent(content);
        note->setFullTitle(fullTitle);
        note->setNoteColor(notecolor);
        note->setMdContent(mdContent);

        return note;
    }
    return Q_NULLPTR;
}

/*!
 * \brief DBManager::isNoteExist
 * \param note
 * \return
 */
bool DBManager::isNoteExist(NoteData* note)
{
    QSqlQuery query(QSqlDatabase::database("note"));

    int id = note->id();
    QString queryStr = QStringLiteral("SELECT EXISTS(SELECT 1 FROM active_notes WHERE id = %1 LIMIT 1 )")
            .arg(id);
    query.exec(queryStr);
    query.next();

    return query.value(0).toInt() == 1;
}

/*!
 * \brief DBManager::getAllNotes
 * \return
 */
QList<NoteData *> DBManager::getAllNotes()
{
    QList<NoteData *> noteList;

    QSqlQuery query(QSqlDatabase::database("note"));
    query.prepare("SELECT * FROM active_notes");
    bool status = query.exec();
    if(status){
        while(query.next()){
            NoteData* note = new NoteData(this);
            int id =  query.value(0).toInt();

            //Qt::UTC	1	世界标准时间，代替格林威治标准时间
            //Qt::LocalTime	0	与语言环境相关的时间（时区和夏时制）
            qint64 epochDateTimeCreation = query.value(1).toLongLong();

            //协调世界时间(Qt::UTC)，并转换为Qt::LocalTime
            //返回一个时间日期是毫秒级的datetime
            QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
            qint64 epochDateTimeModification= query.value(2).toLongLong();
            QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
            QString content = query.value(4).toString();
            QString fullTitle = query.value(5).toString();
            int notecolor = query.value(6).toInt();
            QString mdContent = query.value(7).toString();

            note->setId(id);
            note->setCreationDateTime(dateTimeCreation);
            note->setLastModificationDateTime(dateTimeModification);
            note->setContent(content);
            note->setFullTitle(fullTitle);
            note->setNoteColor(notecolor);
            note->setMdContent(mdContent);

            noteList.push_back(note);
        }
    }

    return noteList;
}

/*!
 * \brief DBManager::addNote
 * \param note
 * \return
 */
bool DBManager::addNote(NoteData* note)
{
    QSqlQuery query(QSqlDatabase::database("note"));
    QString emptyStr;

    qint64 epochTimeDateCreated = note->creationDateTime()
            .toMSecsSinceEpoch();
    QString content = note->content()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                              .replace("'","''")
                              .replace(QChar('\x0'), emptyStr);

    QString mdContent = note->mdContent()
                              .replace("'","''")
                              .replace(QChar('\x0'), emptyStr);

    int notecolor = note->notecolor();

    qint64 epochTimeDateLastModified = note->lastModificationdateTime().isNull() ? epochTimeDateCreated
                                                                                 : note->lastModificationdateTime().toMSecsSinceEpoch();

    QString queryStr = QString("INSERT INTO active_notes "
                               "(creation_date, modification_date, deletion_date, content, full_title, note_color, md_content) "
                               "VALUES (%1, %2, -1, '%3', '%4', '%5', '%6');")
                               .arg(epochTimeDateCreated)
                               .arg(epochTimeDateLastModified)
                               .arg(content)
                               .arg(fullTitle)
                               .arg(notecolor)
                               .arg(mdContent);

    query.exec(queryStr);
    //Returns the number of rows affected by the result's SQL statement
    return (query.numRowsAffected() == 1);
}

/*!
 * \brief DBManager::removeNote
 * \param note
 * \return
 */
bool DBManager::removeNote(NoteData* note)
{
    QSqlQuery query(QSqlDatabase::database("note"));
    QString emptyStr;

    int id = note->id();
    QString queryStr = QStringLiteral("DELETE FROM active_notes "
                                      "WHERE id=%1").arg(id);
    query.exec(queryStr);
    bool removed = (query.numRowsAffected() == 1);

    qint64 epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateDeleted = note->deletionDateTime().toMSecsSinceEpoch();
    QString content = note->content()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                              .replace("'","''")
                              .replace(QChar('\x0'), emptyStr);

    QString mdContent = note->mdContent()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);

    int notecolor = note->notecolor();

    queryStr = QString("INSERT INTO deleted_notes "
                       "VALUES (%1, %2, %3, %4, '%5', '%6', '%7', '%8');")
                       .arg(id)
                       .arg(epochTimeDateCreated)
                       .arg(epochTimeDateModified)
                       .arg(epochTimeDateDeleted)
                       .arg(content)
                       .arg(fullTitle)
                       .arg(notecolor)
                       .arg(mdContent);

    query.exec(queryStr);
    bool addedToTrashDB = (query.numRowsAffected() == 1);

    return (removed && addedToTrashDB);
}

/*!
 * \brief DBManager::permanantlyRemoveAllNotes
 * \return
 */
bool DBManager::permanantlyRemoveAllNotes()
{
    QSqlQuery query(QSqlDatabase::database("note"));
    return query.exec(QString("DELETE FROM active_notes"));
}

/*!
 * \brief DBManager::updateNote
 * \param note
 * \return
 */
bool DBManager::updateNote(NoteData* note)
{
    QSqlQuery query(QSqlDatabase::database("note"));
    QString emptyStr;

    int id = note->id();
    qDebug() << "updateNote  note id = " <<id;
    int notecolor = note->notecolor();

    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note->content().replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle().replace(QChar('\x0'), emptyStr);
    QString mdContent = note->mdContent().replace(QChar('\x0'), emptyStr);

    query.prepare(QStringLiteral("UPDATE active_notes SET md_content = :mdContent, note_color = :color,"
                                 "modification_date = :date, content = :content, full_title = :title WHERE id = :id"));
    query.bindValue(QStringLiteral(":mdContent"), mdContent);
    query.bindValue(QStringLiteral(":color"), notecolor);
    query.bindValue(QStringLiteral(":date"), epochTimeDateModified);
    query.bindValue(QStringLiteral(":content"), content);
    query.bindValue(QStringLiteral(":title"), fullTitle);
    query.bindValue(QStringLiteral(":id"), id);


    if (!query.exec()) {
        qWarning () << __func__ << ": " << query.lastError();
    }

    return (query.numRowsAffected() == 1);
}

/*!
 * \brief DBManager::migrateNote
 * \param note
 * \return
 */
bool DBManager::migrateNote(NoteData* note)
{
    QSqlQuery query(QSqlDatabase::database("note"));

    QString emptyStr;

    int id = note->id();
    int notecolor = note->notecolor();
    qint64 epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note->content()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                              .replace("'","''")
                              .replace(QChar('\x0'), emptyStr);
    QString mdContent = note->mdContent()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);

    QString queryStr = QString("INSERT INTO active_notes "
                               "VALUES (%1, %2, %3, -1, '%4', '%5', '%6', '%7');")
                               .arg(id)
                               .arg(epochTimeDateCreated)
                               .arg(epochTimeDateModified)
                               .arg(content)
                               .arg(fullTitle)
                               .arg(notecolor)
                               .arg(mdContent);

    query.exec(queryStr);
    return (query.numRowsAffected() == 1);
}

/*!
 * \brief DBManager::migrateTrash
 * \param note
 * \return
 */
bool DBManager::migrateTrash(NoteData* note)
{
    QSqlQuery query(QSqlDatabase::database("note"));
    QString emptyStr;

    int id = note->id();
    int notecolor = note->notecolor();
    qint64 epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateDeleted = note->deletionDateTime().toMSecsSinceEpoch();
    QString content = note->content()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                              .replace("'","''")
                              .replace(QChar('\x0'), emptyStr);
    QString mdContent = note->mdContent()
                            .replace("'","''")
                            .replace(QChar('\x0'), emptyStr);

    QString queryStr = QString("INSERT INTO deleted_notes "
                               "VALUES (%1, %2, %3, %4, '%5', '%6', '%7', '%8');")
                               .arg(id)
                               .arg(epochTimeDateCreated)
                               .arg(epochTimeDateModified)
                               .arg(epochTimeDateDeleted)
                               .arg(content)
                               .arg(fullTitle)
                               .arg(notecolor)
                               .arg(mdContent);

    query.exec(queryStr);
    return (query.numRowsAffected() == 1);
}

/*!
 * \brief DBManager::onNotesListRequested
 */
void DBManager::onNotesListRequested()
{
    int noteCounter;
    QList<NoteData *> noteList;

    noteCounter = getLastRowID();
    noteList    = getAllNotes();

    emit notesReceived(noteList, noteCounter);
}

/*!
 * \brief DBManager::onOpenDBManagerRequested
 * \param path
 * \param doCreate
 */
void DBManager::onOpenDBManagerRequested(QString path, bool doCreate)
{
    open(path, doCreate);
}

/*!
 * \brief DBManager::onCreateUpdateRequested
 * \param note
 */
void DBManager::onCreateUpdateRequested(NoteData* note)
{
    bool exists = isNoteExist(note);

    if(exists){
        qDebug() << "DBManager::updateNote";
        updateNote(note);
    }
    else{
        qDebug() << "DBManager::addNote";
        addNote(note);
    }
}

/*!
 * \brief DBManager::onDeleteNoteRequested
 * \param note
 */
void DBManager::onDeleteNoteRequested(NoteData* note)
{
    qDebug() << "receive requestDeleteNote :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    removeNote(note);
}

/*!
 * \brief DBManager::onImportNotesRequested
 * \param noteList
 */
void DBManager::onImportNotesRequested(QList<NoteData *> noteList) {
    QSqlDatabase::database().transaction();
    for(NoteData* note : noteList)
        addNote(note);
    QSqlDatabase::database().commit();
}

/*!
 * \brief DBManager::onRestoreNotesRequested
 * \param noteList
 */
void DBManager::onRestoreNotesRequested(QList<NoteData*> noteList) {
    this->permanantlyRemoveAllNotes();
    this->onImportNotesRequested(noteList);
}

/*!
 * \brief DBManager::onExportNotesRequested
 * \param fileName
 */
void DBManager::onExportNotesRequested(QString fileName)
{
    QList<NoteData *> noteList;
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    out.setVersion(QDataStream::Qt_5_6);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    out.setVersion(QDataStream::Qt_5_4);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    out.setVersion(QDataStream::Qt_5_2);
#endif
    noteList = getAllNotes();
    out << noteList;
    file.close();

    qDeleteAll(noteList);
    noteList.clear();
}

/*!
 * \brief DBManager::onMigrateNotesRequested
 * \param noteList
 */
void DBManager::onMigrateNotesRequested(QList<NoteData *> noteList)
{
    QSqlDatabase::database().transaction();
    for(NoteData* note : noteList)
        migrateNote(note);
    QSqlDatabase::database().commit();

    qDeleteAll(noteList);
    noteList.clear();
}

/*!
 * \brief DBManager::onMigrateTrashRequested
 * \param noteList
 */
void DBManager::onMigrateTrashRequested(QList<NoteData *> noteList)
{
    QSqlDatabase::database().transaction();
    for(NoteData* note : noteList)
        migrateTrash(note);
    QSqlDatabase::database().commit();

    qDeleteAll(noteList);
    noteList.clear();
}

/*!
 * \brief DBManager::onForceLastRowIndexValueRequested
 * \param index
 */
void DBManager::onForceLastRowIndexValueRequested(int index)
{
    forceLastRowIndexValue(index);
}
