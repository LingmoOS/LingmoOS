// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLE_H
#define TABLE_H
#include "docx_global.h"
#include "shared.h"
#include "./oxml/oxmltable.h"
#include "length.h"

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>

namespace Docx {
class DocumentPart;
class Paragraph;
class Column;
class Row;
class Cell;
class Columns;

class DOCX_EXPORT Table
{
public:

    Table(DocumentPart *part, const QDomElement &element);    
    Cell* cell(int rowIndex, int colIndex);
    Row* addRow();    
    Column* addColumn();
    QList<Cell*> rowCells(int rowIndex);
    QList<Row *> rows();    
    void setStyle(const QString &style);
    void setAlignment(WD_TABLE_ALIGNMENT alignment);
    virtual ~Table();

    void loadExistRowElement();

private:
    QList<Row*> m_rows;
    DocumentPart *m_part = nullptr;
    QDomDocument *m_dom = nullptr;        
    CT_Tbl *m_ctTbl = nullptr;
    friend class Row;
    friend class CT_Tbl;
};

class DOCX_EXPORT Cell
{
public:
    Cell(const QDomElement &element, Row *row);
    Paragraph *addParagraph(const QString &text = QString(), const QString &style = QString());
    void addText(const QString &text);
    Table *addTable(int rows, int cols, const QString &style = QString::fromLatin1("TableGrid"));
    Cell *merge(Cell *other);
    int cellIndex();
    int rowIndex();
    Table *table();
    virtual ~Cell();

private:
    QDomDocument *m_dom;
    DocumentPart *m_part;    
    QList<Paragraph *> m_paras;
    Paragraph *m_currentpara;
    Row *m_row;    
    QSharedPointer<CT_Tc> m_tc;
    friend class CT_Tc;
    friend class Row;
};

class DOCX_EXPORT Columns
{
public:
    Columns();    
    virtual ~Columns();

private:

};

class DOCX_EXPORT Column
{
public:
    Column(const QDomElement &tlGrid, int gridIndex, Table *table);
    Length width() const;
    void setWidth();
    virtual ~Column();

private:
    QDomElement m_grid;
    Table *m_table;
    int m_index;
};


class DOCX_EXPORT Row
{
public:
    Row(const QDomElement &element, Table *table);
    void loadExistElement();
    Table *table() const;
    QList<Cell *> cells() const;
    Table *table();
    int rowIndex();
    virtual ~Row();

private:
    void addTc();

private:
    QList<Cell *> m_cells;
    QDomElement m_ele;
    Table *m_table;
    DocumentPart *m_part;
    QDomDocument *m_dom;
    friend class Cell;
    friend class Table;

};
}
#endif // TABLE_H
