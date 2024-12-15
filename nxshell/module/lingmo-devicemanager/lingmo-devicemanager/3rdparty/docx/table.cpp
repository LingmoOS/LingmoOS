// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "table.h"
#include "text.h"
#include "./parts/documentpart.h"
#include "DDLog.h"

#include <QVector>
#include <QLoggingCategory>

using namespace DDLog;

namespace Docx {


const QString strtblRow      = QStringLiteral("w:tr");
const QString strtblCell     = QStringLiteral("w:tc");


Table::Table(DocumentPart *part, const QDomElement &element)
    : m_part(part)
{
    m_dom = part->m_dom;
    m_ctTbl = new CT_Tbl(this, element);
    loadExistRowElement();
}

/*!
 * \brief 如果有行则初始化
 */
void Table::loadExistRowElement()
{

    QDomNodeList reles = m_ctTbl->m_tblEle.childNodes();
    if (!reles.isEmpty()) {
        for (int i = 0; i < reles.count(); i++) {
            QDomElement rowEle = reles.at(i).toElement();
            if (rowEle.nodeName() != strtblRow)
                continue;
            Row *row = new Row(rowEle, this);
            m_rows.append(row);
            qCInfo(appLog) << m_rows.count();
        }
    }
}

Cell *Table::cell(int rowIndex, int colIndex)
{
    Row *row = m_rows.at(rowIndex);

    return row->cells().at(colIndex);
}

Table::~Table()
{
    delete m_ctTbl;
}

/*!
 * \brief 添加列
 * \return
 */
Column *Table::addColumn()
{
    QDomElement gridCol = m_ctTbl->m_tblGrid->addGridCol();
    for (Row *row : m_rows) {
        row->addTc();
    }
    return new Column(gridCol, m_ctTbl->m_tblGrid->count(), this);
}

/*!
 * \brief 添加行
 * \return
 */
Row *Table::addRow()
{
    QDomElement rowEle = m_dom->createElement(strtblRow);
    Row *row = new Row(rowEle, this);
    m_rows.append(row);
    for (int i = 0; i< m_ctTbl->m_tblGrid->count(); i++) {
        row->addTc();
    }
    m_ctTbl->m_tblEle.appendChild(rowEle);
    return row;
}

/*!
 * \brief 得到rowIndex的行内所有的cell
 *
 * index
 * \param rowIdx
 * \return
 */
QList<Cell *> Table::rowCells(int rowIndex)
{
    Row *row = m_rows.at(rowIndex);

    return row->cells();
}

QList<Row *> Table::rows()
{
    return m_rows;
}

/*!
 * \brief 设置样式
 * \param style
 */
void Table::setStyle(const QString &style)
{
    m_ctTbl->setStyle(style);
}

void Table::setAlignment(WD_TABLE_ALIGNMENT alignment)
{
    m_ctTbl->setAlignment(alignment);
}

Columns::Columns()
{

}

Columns::~Columns()
{

}

Column::Column(const QDomElement &tlGrid, int gridIndex, Table *table)
    : m_grid(tlGrid), m_table(table), m_index(gridIndex)
{

}

Length Column::width() const
{
    return Length();
}

void Column::setWidth()
{

}

Column::~Column()
{

}

Row::Row(const QDomElement &element, Table *table)
    : m_ele(element), m_table(table)
{
    m_dom = m_table->m_dom;
    m_part= m_table->m_part;

    loadExistElement();
}

/*!
 * \brief 初始化列
 */
void Row::loadExistElement()
{
    QDomNodeList eleList = m_ele.childNodes();
    if (eleList.isEmpty())
        return;

    for (int i = 0; i < eleList.count(); i++) {
        QDomElement celEle = eleList.at(i).toElement();
        if (celEle.nodeName() != strtblCell)
            continue;

        Cell *cell = new Cell(celEle, this);
        m_cells.append(cell);

        // 纵向
        QString strMerge = cell->m_tc->vMerge();
        if (!cell->m_tc->m_vMerge.isNull() && strMerge != QStringLiteral("restart")) {
            Row *uprow = m_table->m_rows.last();
            Cell *upcell = uprow->m_cells.at(m_cells.count() - 1);
            celEle = QDomElement(upcell->m_tc->ele());
            //
            m_cells.removeAll(cell);
            delete cell;
            cell = new Cell(celEle, this);
            m_cells.append(cell);
        }

        // 横向
        int spanCount = cell->m_tc->gridSpan();
        if (spanCount > 1)
            for (int i = 1; i < spanCount; i++) {
                Cell *hcell = new Cell(celEle, this);
                m_cells.append(hcell);
            }

    }
    //qCInfo(appLog) << "Current row's cell count" << m_cells.count();
}

void Row::addTc()
{
    QDomElement celEle = m_dom->createElement(strtblCell);
    Cell *cell = new Cell(celEle, this);
    m_cells.append(cell);

    QDomElement tcPr = m_dom->createElement("w:tcPr");
    QDomElement tcW = m_dom->createElement("w:tcW");
    tcW.setAttribute("w:w","2000");
    tcW.setAttribute("w:type","dxa");
    tcPr.appendChild(tcW);


    QDomElement tcBorders = m_dom->createElement("w:tcBorders");
    QDomElement tl2br = m_dom->createElement("w:tl2br");
    tl2br.setAttribute("w:val","nil");
    QDomElement tr2bl = m_dom->createElement("w:tr2bl");
    tr2bl.setAttribute("w:val","nil");

    tcBorders.appendChild(tl2br);
    tcBorders.appendChild(tr2bl);
    tcPr.appendChild(tcBorders);

    celEle.appendChild(tcPr);

    m_ele.appendChild(celEle);
}

Table *Row::table() const
{
    return m_table;
}

QList<Cell *> Row::cells() const
{
    return m_cells;
}

Table *Row::table()
{
    return m_table;
}

int Row::rowIndex()
{
    return m_table->m_rows.indexOf(this);
}

Row::~Row()
{

}

Cell::Cell(const QDomElement &element, Row *row)
    : m_row(row)
{
    m_dom = row->m_dom;
    m_part = row->m_part;
    m_tc = QSharedPointer<CT_Tc>(new CT_Tc(this, element));
    if (!m_tc->m_isLoad)
        addParagraph();
    else {
        QDomNode node = element.lastChild();
        if (!node.isNull() && node.nodeName() == QStringLiteral("w:p")) {
            QDomElement pEle = node.toElement();
            m_currentpara = new Paragraph(m_part, pEle);
            m_paras.append(m_currentpara);
        }
    }
}

Paragraph *Cell::addParagraph(const QString &text, const QString &style)
{    
    QDomElement pEle = m_dom->createElement(QStringLiteral("w:p"));

    m_currentpara = new Paragraph(m_part, pEle);

    if (!text.isEmpty())
        m_currentpara->addRun(text, style);

    m_tc->m_ele.appendChild(pEle);
    m_paras.append(m_currentpara);
    return m_currentpara;
}

void Cell::addText(const QString &text)
{
    m_currentpara->addRun(text);
}

Table *Cell::addTable(int rows, int cols, const QString &style)
{       
    QDomElement pEle = m_dom->createElement(QStringLiteral("w:tbl"));
    Table *table =  new Table(m_part, pEle);

    m_tc->m_ele.appendChild(pEle);

    for (int i = 0; i < cols; i++) {
        table->addColumn();
    }
    for (int i = 0; i < rows; i++) {
        table->addRow();
    }
    table->setStyle(style);
    addParagraph();
    return table;
}

Cell *Cell::merge(Cell *other)
{
    QSharedPointer<CT_Tc> tc = this->m_tc;
    QSharedPointer<CT_Tc> tc2 = other->m_tc;
    CT_Tc *tcReturn = tc->merge(tc2);
    return tcReturn->m_cell;
}

int Cell::cellIndex()
{
    return m_row->m_cells.indexOf(this);
}

int Cell::rowIndex()
{
    return m_row->rowIndex();
}

Table *Cell::table()
{
    return m_row->m_table;
}

Cell::~Cell()
{
    qDeleteAll(m_paras);
}

}
