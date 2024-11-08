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

#include <QDebug>

#include "noteModel.h"

NoteModel::NoteModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

NoteModel::~NoteModel()
{

}

QModelIndex NoteModel::addNote(NoteData* note)
{
    const int rowCnt = rowCount();
    //指定要插入的第一行和最后一行新行的行号，以及其父项的模型索引
    beginInsertRows(QModelIndex(), rowCnt, rowCnt);
    m_noteList << note;
    endInsertRows();

    return createIndex(rowCnt, 0);
}

QModelIndex NoteModel::insertNote(NoteData *note, int row)
{
    if(row >= rowCount()){
        return addNote(note);
    }else{
        beginInsertRows(QModelIndex(), row, row);
        m_noteList.insert(row, note);
        endInsertRows();
    }

    return createIndex(row,0);
}

NoteData* NoteModel::getNote(const QModelIndex& index)
{
    if(index.isValid()){
        //返回列表中索引位置index.row处的项, index.row 必须是列表中有效的索引位置
        return m_noteList.at(index.row());
    }else{
        return Q_NULLPTR;
    }
}

void NoteModel::addListNote(QList<NoteData *> noteList)
{
    int start = rowCount();
    int end = start + noteList.count()-1;
    beginInsertRows(QModelIndex(), start, end);
    m_noteList << noteList;
    endInsertRows();
}

NoteData* NoteModel::removeNote(const QModelIndex &noteIndex)
{
    int row = noteIndex.row();
    beginRemoveRows(QModelIndex(), row, row);
    NoteData* note = m_noteList.takeAt(row);
    endRemoveRows();

    return note;
}

bool NoteModel::moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild)
{
    if(sourceRow<0
            || sourceRow >= m_noteList.count()
            || destinationChild <0
            || destinationChild >= m_noteList.count()){

        return false;
    }
    //参数1 QAbstractListModel
    //参数2和参数3是要移动的行的第一行和最后一行,将参数2 参数3 所在行 移动到 destinationChild 之上
    //参数4 QAbstractListModel 索引对应于在其中这些行被移动的父
    beginMoveRows(sourceParent,sourceRow,sourceRow,destinationParent,destinationChild);
    //将 sourceRow item 移动到 destinationChild item位置, destinationChild item依次前移
    m_noteList.move(sourceRow,destinationChild);
    endMoveRows();

    return true;
}

void NoteModel::clearNotes()
{
    beginResetModel();
    m_noteList.clear();
    endResetModel();
}

//负责返回数据项对应于所述索引参数
QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_noteList.count())
        return QVariant();

    NoteData* note = m_noteList[index.row()];
    if(role == NoteID){
        return note->id();
    }else if(role == NoteFullTitle){
        return note->fullTitle();
    }else if(role == NoteCreationDateTime){
        return note->creationDateTime();
    }else if(role == NoteLastModificationDateTime){
        return note->lastModificationdateTime();
    }else if(role == NoteDeletionDateTime){
        return note->deletionDateTime();
    }else if(role == NoteContent){
        return note->content();
    }else if(role == NoteScrollbarPos){
        return note->scrollBarPosition();
    }else if(role == NoteColor){
        return note->notecolor();
    }else if (role == NoteMdContent){
        return note->mdContent();
    }

    return QVariant();
}

//provide a way for the delegate to set the data in the model
bool NoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    NoteData* note = m_noteList[index.row()];


    if(role == NoteID){
        note->setId(value.toInt());
    }else if(role == NoteFullTitle){
        note->setFullTitle(value.toString());
    }else if(role == NoteCreationDateTime){
        note->setCreationDateTime(value.toDateTime());
    }else if(role == NoteLastModificationDateTime){
        note->setLastModificationDateTime(value.toDateTime());
    }else if(role == NoteDeletionDateTime){
        note->setDeletionDateTime(value.toDateTime());
    }else if(role == NoteContent){
        note->setContent(value.toString());
    }else if(role == NoteScrollbarPos){
        note->setScrollBarPosition(value.toInt());
    }else if(role == NoteColor){
        note->setNoteColor(value.toInt());
    }else if(role == NoteMdContent){
        note->setMdContent(value.toString());
    }else{
        return false;
    }

    emit dataChanged(this->index(index.row()),
                     this->index(index.row()),
                     QVector<int>(1,role));

    return true;
}

Qt::ItemFlags NoteModel::flags(const QModelIndex &index) const
{
    //if (!index.isValid())
    //        return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    //return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

//保持模型中的行数与字符串列表中的字符串数相同
int NoteModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_noteList.count();
}

void NoteModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(order)
    qDebug() << "NoteModel::sort" << column;
    // 升序
    if(order == Qt::AscendingOrder){
        if(column == 0){            //创建时间排序
            std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
                return lhs->creationDateTime() > rhs->creationDateTime();
            });
        }else if (column == 1){     //修改时间排序
            std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
                return lhs->lastModificationdateTime() > rhs->lastModificationdateTime();
            });
        }else if (column == 2){     //名称排序
            std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
                return lhs->mdContent() > rhs->mdContent();
            });
        }

    }else if(order == Qt::DescendingOrder){
        if(column == 0){            //创建时间排序
            std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
                return lhs->creationDateTime() < rhs->creationDateTime();
            });
        }else if (column == 1){     //修改时间排序
            std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
                return lhs->lastModificationdateTime() < rhs->lastModificationdateTime();
            });
        }else if (column == 2){     //名称排序
            std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
                return lhs->mdContent() < rhs->mdContent();
            });
        }
    }
    emit dataChanged(index(0), index(rowCount()-1));
}

Qt::DropActions NoteModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
