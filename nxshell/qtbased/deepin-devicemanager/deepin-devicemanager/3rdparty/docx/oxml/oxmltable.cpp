// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oxmltable.h"
#include "../table.h"
#include "DDLog.h"

#include <QDomElement>
#include <QLoggingCategory>

using namespace DDLog;

namespace Docx {

const QString strstyle       = QStringLiteral("w:tblPr");
const QString strtblGrid     = QStringLiteral("w:tblGrid");

CT_Tbl::CT_Tbl(Table *table, const QDomElement &ele)
    : m_table(table), m_tblEle(ele)
{
    m_dom = m_table->m_dom;

    // tblGrid
    QDomNodeList tblGrids = m_tblEle.elementsByTagName(strtblGrid);
    if (tblGrids.isEmpty()) {
        QDomElement tblGrid = m_dom->createElement(strtblGrid);
        m_tblGrid = new CT_TblGrid(m_dom, tblGrid);
        m_tblEle.appendChild(tblGrid);
    } else {
        m_tblGrid = new CT_TblGrid(m_dom, tblGrids.at(0).toElement());
    }

    // tblPr
    QDomNodeList tblPrEle = m_tblEle.elementsByTagName(strstyle);
    if (!tblPrEle.isEmpty()) {
        QDomElement styleEle = tblPrEle.at(0).toElement();
        m_style = new CT_TblPr(m_dom, styleEle);
    }



}

void CT_Tbl::setStyle(const QString &style)
{
    if (!m_style) {
        QDomElement styleEle = m_dom->createElement(strstyle);
        QDomElement tblBorders = m_dom->createElement("w:tblBorders");
        QDomElement top = m_dom->createElement("w:top");
        top.setAttribute("w:val", "single");
        top.setAttribute("w:color", "auto");
        top.setAttribute("w:sz", "4");
        top.setAttribute("w:space", "0");
        tblBorders.appendChild(top);

        QDomElement left = m_dom->createElement("w:left");

        left.setAttribute("w:val", "single");
        left.setAttribute("w:color", "auto");
        left.setAttribute("w:sz", "4");
        left.setAttribute("w:space", "0");
        tblBorders.appendChild(left);

        QDomElement bottom = m_dom->createElement("w:bottom");
        bottom.setAttribute("w:val", "single");
        bottom.setAttribute("w:color", "auto");
        bottom.setAttribute("w:sz", "4");
        bottom.setAttribute("w:space", "0");
        tblBorders.appendChild(bottom);

        QDomElement right = m_dom->createElement("w:right");
        right.setAttribute("w:val", "single");
        right.setAttribute("w:color", "auto");
        right.setAttribute("w:sz", "4");
        right.setAttribute("w:space", "0");
        tblBorders.appendChild(right);

        QDomElement insideH = m_dom->createElement("w:insideH");
        insideH.setAttribute("w:val", "single");
        insideH.setAttribute("w:color", "auto");
        insideH.setAttribute("w:sz", "4");
        insideH.setAttribute("w:space", "0");
        tblBorders.appendChild(insideH);

        QDomElement insideV = m_dom->createElement("w:insideV");
        insideV.setAttribute("w:val", "single");
        insideV.setAttribute("w:color", "auto");
        insideV.setAttribute("w:sz", "4");
        insideV.setAttribute("w:space", "0");
        tblBorders.appendChild(insideV);
        styleEle.appendChild(tblBorders);

        m_style = new CT_TblPr(m_dom, styleEle);
        QDomNode n = m_tblEle.firstChild();
        m_tblEle.insertBefore(styleEle, n);
    }
    m_style->setStyle(style);
}

void CT_Tbl::setAlignment(WD_TABLE_ALIGNMENT alignment)
{
    m_style->setAlignment(alignment);
}

CT_Tbl::~CT_Tbl()
{
    delete m_style;
}

CT_TblGrid::CT_TblGrid(QDomDocument *dom, const QDomElement &ele)
    :  m_element(ele), m_dom(dom)
{
    cols = m_element.childNodes().count();
}

QDomElement CT_TblGrid::addGridCol()
{
    QDomElement gridcol = m_dom->createElement(QStringLiteral("w:gridCol"));
    gridcol.setAttribute(QStringLiteral("w:w"), 1600);
    m_element.appendChild(gridcol);
    ++cols;
    return gridcol;
}

QDomElement CT_TblGrid::ele() const
{
    return m_element;
}

int CT_TblGrid::count() const
{
    return cols;
}

CT_TblGrid::~CT_TblGrid()
{

}

const QString strtblStyle = QStringLiteral("w:tblStyle");
const QString strJc = QStringLiteral("w:jc");

/*!
 * \brief 表格的样式
 * \param dom
 * \param ele
 */
CT_TblPr::CT_TblPr(QDomDocument *dom, const QDomElement &ele)
    : m_element(ele), m_dom(dom)
{
    initAlignsMap();
    loadExistStyle();
}

void CT_TblPr::loadExistStyle()
{
    QDomNodeList eles = m_element.elementsByTagName(strtblStyle);
    if (!eles.isEmpty())
        m_tblStyle = eles.at(0).toElement();

    eles = m_element.elementsByTagName(strJc);
    if (!eles.isEmpty())
        m_jcAlignment = eles.at(0).toElement();

}


/*!
 * \brief 初始化位置map
 */
void CT_TblPr::initAlignsMap()
{
    m_aligns.insert(WD_TABLE_ALIGNMENT::LEFT,           QStringLiteral("left"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::RIGHT,          QStringLiteral("right"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::CENTER,         QStringLiteral("center"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::BOTH,           QStringLiteral("both"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::DISTRIBUTE,     QStringLiteral("distribute"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::MEDIUMKASHIDA,  QStringLiteral("mediumKashida"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::HIGHKASHIDA,    QStringLiteral("highKashida"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::LOWKASHIDA,     QStringLiteral("lowKashida"));
    m_aligns.insert(WD_TABLE_ALIGNMENT::THAIDISTRIBUTE, QStringLiteral("thaiDistribute"));
}


void CT_TblPr::setStyle(const QString &style)
{
    checkStyleElement();
    m_tblStyle.setAttribute(QStringLiteral("w:val"), style);
}

void CT_TblPr::setAlignment(WD_TABLE_ALIGNMENT alignment)
{
    checkAlignment();
    m_jcAlignment.setAttribute(QStringLiteral("w:val"), m_aligns.value(alignment));
}

CT_TblPr::~CT_TblPr()
{

}

void CT_TblPr::checkStyleElement()
{
    if (m_tblStyle.isNull()) {
        m_tblStyle = m_dom->createElement(strtblStyle);
        m_element.appendChild(m_tblStyle);
    }
}

void CT_TblPr::checkAlignment()
{
    if (m_jcAlignment.isNull()) {
        m_jcAlignment = m_dom->createElement(strJc);
        m_element.appendChild(m_jcAlignment);
    }
}

// begin CT_Tc

const QString RESTART = QStringLiteral("restart");
const QString CONTINUE = QStringLiteral("continue");

CT_Tc::CT_Tc(Cell *cell, const QDomElement &ele)
    : m_cell(cell), m_ele(ele)
{
    loadExistStyle();
}

/*!
 * \brief 合并单元格
 * \param other
 */
CT_Tc *CT_Tc::merge(QSharedPointer<CT_Tc> other)
{
    int top, left, height, width;
    spanDimensions(other, top, left, height, width);
    Table *table = m_cell->table();
    Cell *cell = table->cell(top, left);
    CT_Tc *top_tc = cell->m_tc.data();
    top_tc->growTo(width, height);

    if (top_tc != this) {

        m_cell->m_tc->copyCt(top_tc);
    }

    top_tc->m_cell->addParagraph();
    return top_tc;
}

QDomElement CT_Tc::ele() const
{
    return m_ele;
}

QString CT_Tc::vMerge() const
{
    if (m_vMerge.isNull())
        return QString();
    return m_vMerge.attribute(QStringLiteral("w:val"));
}

void CT_Tc::loadExistStyle()
{
    QDomNodeList eles = m_ele.elementsByTagName(QStringLiteral("w:tcPr"));

    if (!eles.isEmpty()) {
        m_tcPr = eles.at(0).toElement();
        m_isLoad = true;

        // w:tcW
        eles = m_tcPr.elementsByTagName(QStringLiteral("w:tcW"));
        if (!eles.isEmpty())
            m_tcW = eles.at(0).toElement();

        // w:gridSpan
        eles = m_tcPr.elementsByTagName(QStringLiteral("w:gridSpan"));
        if (!eles.isEmpty())
            m_gridSpan = eles.at(0).toElement();

        // w:vMerge
        eles = m_tcPr.elementsByTagName(QStringLiteral("w:vMerge"));
        if (!eles.isEmpty())
            m_vMerge = eles.at(0).toElement();

    }
}

void CT_Tc::setvMerge(const QString &value)
{
    checktcPr();
    if (value.isEmpty())
        return;
    if (m_vMerge.isNull()) {
        m_vMerge = m_cell->m_dom->createElement(QStringLiteral("w:vMerge"));
        m_tcPr.appendChild(m_vMerge);
    }
    m_vMerge.setAttribute(QStringLiteral("w:val"), value);
}

int CT_Tc::gridSpan() const
{
    if (m_gridSpan.isNull())
        return 1;
    int span = m_gridSpan.attribute(QStringLiteral("w:val"), "1").toInt();
    return span;
}

void CT_Tc::setGridSpan(int span)
{
    checktcPr();
    if (m_gridSpan.isNull()) {
        m_gridSpan = m_cell->m_dom->createElement(QStringLiteral("w:gridSpan"));
        m_tcPr.appendChild(m_gridSpan);
    }
    m_gridSpan.setAttribute(QStringLiteral("w:val"), QString::number(span));

}

int CT_Tc::top() const
{
    if (m_vMerge.isNull() || vMerge() == RESTART) {
        return m_cell->rowIndex();
    }

    return tcAbove()->m_tc->top();
}

int CT_Tc::bottom() const
{
    if (!m_vMerge.isNull()) {

    }
    return m_cell->rowIndex() + 1;
}

/*!
 * \brief 列的index
 * \return
 */
int CT_Tc::left() const
{
    return m_cell->cellIndex();
}

int CT_Tc::right() const
{
    return left() + gridSpan();
}

CT_Tc::~CT_Tc()
{

}

/*!
 * \brief 测量需要合并的单元格大小
 * \param other
 * \param top
 * \param left
 * \param height
 * \param width
 */
void CT_Tc::spanDimensions(QSharedPointer<CT_Tc> other, int &top, int &left, int &height, int &width)
{
    raise_on_inverted_L(this, other.data());
    raise_on_tee_shaped(this, other.data());

    int vtop = qMin(this->top(), other->top());
    int vleft = qMin(this->left(), other->left());
    int vbottom = qMax(this->bottom(), other->bottom());
    int vright = qMax(this->right(), other->right());

    top = vtop;
    left = vleft;
    height = vbottom -  vtop;
    width = vright - vleft;
}

/*!
 * \brief 检查矩形是否合法
 * \param a
 * \param b
 */
void CT_Tc::raise_on_inverted_L(CT_Tc *a, CT_Tc *b)
{
    if (a->top() == b->top() && a->bottom() != b->bottom())
        throw InvalidSpanError(QStringLiteral("requested span not rectangular"));
    if (a->left() == b->left() && a->right() != b->right())
        throw InvalidSpanError(QStringLiteral("requested span not rectangular"));
}

/*!
 * \brief 检查矩形是否合法
 * \param a
 * \param b
 */
void CT_Tc::raise_on_tee_shaped(CT_Tc *a, CT_Tc *b)
{
    CT_Tc *top_most = a;
    CT_Tc *other = b;

    if (a->top() >= b->top()) {
        top_most = b;
        other = a;
    }
    if (top_most->top() < other->top() && top_most->bottom() > other->bottom())
        throw InvalidSpanError(QStringLiteral("requested span not rectangular"));

    CT_Tc *left_most = a;
    other = b;

    if (a->left() >= b->left()) {
        left_most = b;
        other = a;
    }

    if (left_most->left() < other->left() && left_most->right() > other->right())
        throw InvalidSpanError(QStringLiteral("requested span not rectangular"));
}

int CT_Tc::width() const
{
    if (m_tcW.isNull())
        return -1;
    return m_tcW.attribute(QStringLiteral("w:w"), QStringLiteral("-1")).toInt();
}

void CT_Tc::setWidth(int width)
{
    if (width < 0)
        return;
    checktcPr();
    if (m_tcW.isNull()) {
        m_tcW = m_cell->m_dom->createElement(QStringLiteral("w:tcW"));
        m_tcW.setAttribute(QStringLiteral("w:type"), QStringLiteral("dxa"));
        m_tcPr.appendChild(m_tcW);
    }
    m_tcW.setAttribute(QStringLiteral("w:w"), QString::number(width));
}

/*!
 * \brief 上边的相邻列
 * \return
 */
Cell *CT_Tc::tcAbove() const
{
    Row *row = trAbove();
    int index = m_cell->cellIndex();
    Cell *cell =  row->cells().at(index);

    CT_Tc *tc = cell->m_tc.data();

    if (tc->m_ele == m_ele)
        return tc->tcAbove();

    return cell;
}

/*!
 * \brief 下边的相邻列
 * \return
 */
Cell *CT_Tc::tcBelow() const
{
    Row *row = trBelow();
    //int rowIndex = row->rowIndex();
    if (row) {
        int index = m_cell->cellIndex();
        Cell *cell = row->cells().at(index);
        CT_Tc *tc = cell->m_tc.data();
        //rowIndex = tc->m_cell->rowIndex();
        if (tc->m_ele == m_ele)
            return tc->tcBelow();

        return cell;
    }

    return  nullptr;
}

/*!
 * \brief 上一行
 * \return
 */
Row *CT_Tc::trAbove() const
{
    int rowIndex = m_cell->rowIndex();
    if (rowIndex == 0)
        throw InvalidSpanError("no tr above topmost tr");
    Table *table = m_cell->table();
    QList<Row *> rows = table->rows();

    return rows.at(rowIndex - 1);
}

/*!
 * \brief 下一行
 * \return
 */
Row *CT_Tc::trBelow() const
{
    int rowIndex = m_cell->rowIndex();
    Table *table = m_cell->table();
    QList<Row *> rows = table->rows();
    ++rowIndex;
    if (rows.count() == rowIndex)
        return nullptr;
    return rows.at(rowIndex);
}

/*!
 * \brief 合并宽度为width和高度为height的一个矩形 单元格
 * \param width
 * \param height
 * \param top_tc
 */
void CT_Tc::growTo(int width, int height, CT_Tc *top_tc)
{
    // vMerge_val
    if (top_tc == nullptr)
        top_tc = this;

    spanToWidth(width, top_tc, vMergeVal(height, top_tc));

    if (height > 1) {
        Cell *belowCell = this->tcBelow();

        int belowRowIndex = belowCell->rowIndex();
        int currentRowIndex = this->m_cell->rowIndex();
        int calculateIndex = belowRowIndex - currentRowIndex - 1;
        if (calculateIndex > 0)
            height -= calculateIndex;
        belowCell->m_tc->growTo(width, height - 1, top_tc);

        belowCell->m_tc->copyCt(top_tc);
        //belowCell->m_tc = top_tc->m_cell->m_tc;
    }

}

/*!
 * \brief 单元格合并是开始节点还是中间合并结点的属性值
 * \param height
 * \param tc
 * \return
 */
QString CT_Tc::vMergeVal(int height, CT_Tc *tc)
{
    if (tc != this)
        return CONTINUE;
    if (height == 1)
        return QString();

    return RESTART;
}

/*!
 * \brief 横向合并单元格
 * \param width
 * \param top_tc
 * \param vmerge
 */
void CT_Tc::spanToWidth(int grid_width, CT_Tc *top_tc, const QString &vmerge)
{
    moveContentTo(top_tc);
    while (this->gridSpan() < grid_width) {
        this->swallowNextTc(grid_width, top_tc);
    }
    this->setvMerge(vmerge);
}

/*!
 * \brief 把自身的内容转移到 top_tc里
 * \param top_tc
 */
void CT_Tc::moveContentTo(CT_Tc *top_tc)
{
    if (top_tc == this)
        return;

    if (m_ele.childNodes().isEmpty())
        return;
    // remove element
    top_tc->removeTrailingEmptyP();

    // add content
    int i = 0;
    QDomNodeList nodes = m_ele.childNodes();
    int count = nodes.count() - 1;
    qCInfo(appLog) << "nodes count : " << count;
    QDomNode firstN = m_ele.firstChild();
    if (firstN.nodeName() == QStringLiteral("w:tcPr"))
        i = 1;
    for (; i <= count; count--)
        top_tc->m_ele.appendChild(nodes.at(count));

    // add back the required minimum single empty <w:p> element
    m_cell->addParagraph();
}

/*!
 * \brief 删除单元格最后的空段落
 */
void CT_Tc::removeTrailingEmptyP()
{
    QDomNode lastN = m_ele.lastChild();
    if (lastN.isNull()
            || lastN.nodeName() != QStringLiteral("w:p")
            || lastN.childNodes().count() > 0)
        return;

    m_ele.removeChild(lastN);
}

/*!
 * \brief 横向合并下一个单元格
 * \param grid_width
 * \param top_tc
 */
void CT_Tc::swallowNextTc(int grid_width, CT_Tc *top_tc)
{
    Cell *next_tc = this->nextTc();
    if (!next_tc)
        return;
    raise_on_invalid_swallow(grid_width, next_tc->m_tc.data());
    next_tc->m_tc->moveContentTo(top_tc);
    this->addWidthOf(next_tc->m_tc.data());
    this->setGridSpan(this->gridSpan() + next_tc->m_tc->gridSpan());
    QDomNode parent = next_tc->m_tc->m_ele.parentNode();
    parent.removeChild(next_tc->m_tc->m_ele);

    //next_tc->m_tc = this->m_cell->m_tc;
    next_tc->m_tc->copyCt(this);

}

void CT_Tc::addWidthOf(CT_Tc *other_tc)
{
    if (this->width() > 0 && other_tc->width() > 0)
        setWidth(this->width() + other_tc->width());
}

/*!
 * \brief 检查横向合并下一个单元格的正确性
 * \param grid_width
 * \param nextc
 */
void CT_Tc::raise_on_invalid_swallow(int grid_width, CT_Tc *nextc)
{
    if (!nextc)
        throw InvalidSpanError("not enough grid columns");
    if (this->gridSpan() + nextc->gridSpan() > grid_width)
        throw InvalidSpanError("span is not rectangular");
}

/*!
 * \brief 横向的下一下cell
 * \return
 */
Cell *CT_Tc::nextTc() const
{
    int currentIndex = m_cell->cellIndex();
    Row *row = m_cell->m_row;
    if ((currentIndex + 1) == row->cells().count())
        return nullptr;

    Cell *cell;
    for (int i = currentIndex + 1; i < row->cells().count(); i++) {
        cell = row->cells().at(i);

        if (cell->m_tc->m_ele != m_ele)
            return cell;
    }
    return nullptr;

}

void CT_Tc::checktcPr()
{
    if (m_tcPr.isNull()) {
        m_tcPr = m_cell->m_dom->createElement(QStringLiteral("w:tcPr"));
        QDomNode firstN = m_ele.firstChild();
        m_ele.insertBefore(m_tcPr, firstN);
    }
}

void CT_Tc::copyCt(CT_Tc *otherCell)
{
    m_ele = QDomElement(otherCell->m_ele);
    m_tcPr = QDomElement(otherCell->m_tcPr);
    m_vMerge = QDomElement(otherCell->m_vMerge);
    m_tcW = QDomElement(otherCell->m_tcW);
    m_gridSpan = QDomElement(otherCell->m_gridSpan);
}


}

