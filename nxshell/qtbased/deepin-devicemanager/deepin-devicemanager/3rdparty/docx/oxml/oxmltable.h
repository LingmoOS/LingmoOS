// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OXMLTABLE_H
#define OXMLTABLE_H

#include "../enums/enumtext.h"

#include <QDomElement>
#include <QMap>
#include <QSharedPointer>

namespace Docx {

class Table;
class Row;
class Cell;
class CT_TblPr;
class CT_TblGrid;
class CT_Tc;

/*!
 * \brief <w:tbl>
 */
class CT_Tbl
{
public:
    CT_Tbl(Table *table, const QDomElement &ele);
    void setStyle(const QString &style);
    void setAlignment(WD_TABLE_ALIGNMENT alignment);
    virtual ~CT_Tbl();

private:
    CT_TblPr *m_style = nullptr;
    CT_TblGrid *m_tblGrid = nullptr;
    QDomDocument *m_dom;
    Table *m_table;
    QDomElement m_tblEle;
    friend class Table;
};

/*!
 * \brief <w:tblPr> child of <w:tbl> tables style
 */
class CT_TblPr
{
public:
    CT_TblPr(QDomDocument *dom, const QDomElement &ele);
    void setStyle(const QString &style);
    void setAlignment(WD_TABLE_ALIGNMENT alignment);
    virtual ~CT_TblPr();

    void initAlignsMap();
    void loadExistStyle();
private:
    void checkStyleElement();
    void checkAlignment();

private:
    QDomElement m_element;
    //QDomElement m_element;
    QDomDocument *m_dom;    
    QDomElement m_tblStyle;
    QDomElement m_jcAlignment;
    QMap<WD_TABLE_ALIGNMENT, QString> m_aligns;
};

/*!
 * \brief <w:tblGrid>  child of <w:tbl>, holds <w:gridCol>
 */
class CT_TblGrid
{
public:
    CT_TblGrid(QDomDocument *dom, const QDomElement &ele);
    QDomElement addGridCol();
    QDomElement ele() const;
    int count() const;
    virtual ~CT_TblGrid();

private:
    QDomElement m_element;
    QDomDocument *m_dom;
    int cols = 0;
};

/*!
 * \brief <w:tc> table cell element
 */
class CT_Tc
{
public:    
    CT_Tc(Cell *cell, const QDomElement &ele);
    CT_Tc *merge(QSharedPointer<CT_Tc> other);
    QDomElement ele() const;
    QString vMerge() const;
    void loadExistStyle();
    void setvMerge(const QString &value);
    int gridSpan() const;
    void setGridSpan(int span);

    int top() const;
    int bottom() const;
    int left() const;
    int right() const;

    virtual ~CT_Tc();

private:
    void spanDimensions(QSharedPointer<CT_Tc> other, int &top, int &left, int &height, int &width);    
    void raise_on_inverted_L(CT_Tc *a, CT_Tc *b);
    void raise_on_tee_shaped(CT_Tc *a, CT_Tc *b);
    int width() const;
    void setWidth(int width);
    Cell *tcAbove() const;
    Cell *tcBelow() const;
    Row *trAbove() const;
    Row *trBelow() const;
    void growTo(int width, int height, CT_Tc *top_tc = nullptr);
    QString vMergeVal(int height, CT_Tc *tc);
    void spanToWidth(int grid_width, CT_Tc *top_tc, const QString &vmerge);    
    void moveContentTo(CT_Tc *top_tc);
    void removeTrailingEmptyP();    
    void swallowNextTc(int grid_width, CT_Tc *top_tc);
    void addWidthOf(CT_Tc *other_tc);
    void raise_on_invalid_swallow(int grid_width, CT_Tc *nextc);
    Cell *nextTc() const;    

private:
    void checktcPr();
    void copyCt(CT_Tc *otherCell);

private:
    Cell *m_cell = nullptr;
    QDomElement m_ele;
    QDomElement m_tcPr;
    QDomElement m_vMerge;
    QDomElement m_tcW;
    QDomElement m_gridSpan;
    bool m_isLoad = false;
    friend class Cell;
    friend class Row;
};


}

#endif // OXMLTABLE_H
