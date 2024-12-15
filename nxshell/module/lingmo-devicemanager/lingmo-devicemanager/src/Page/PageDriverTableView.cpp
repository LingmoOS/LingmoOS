// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageDriverTableView.h"
#include "drivertableview.h"
#include "MacroDefinition.h"

#include <DApplicationHelper>
#include <DApplication>

#include <QPainter>
#include <QHBoxLayout>
#include <QPainterPath>

PageDriverTableView::PageDriverTableView(DWidget *parent)
    : DWidget(parent)
    , mp_View(new DriverTableView(this))
    , m_PreWidth(width())
{
    initWidgets();
    connect(mp_View, &DriverTableView::operatorClicked, this, &PageDriverTableView::operatorClicked);
    connect(mp_View, &DriverTableView::itemChecked, this, &PageDriverTableView::itemChecked);
}

void PageDriverTableView::setColumnWidth(int row, int column)
{
    mp_View->setColumnWidth(row, column);
}

void PageDriverTableView::appendRowItems(int column)
{
    mp_View->appendRowItems(column);
    this->setFixedHeight(height() + DRIVER_TABLE_ROW_HEIGHT);
}

void PageDriverTableView::setWidget(int row, int column, DWidget *widget)
{
    mp_View->setWidget(row, column, widget);
    mp_View->resizeColumn(column);
}

QAbstractItemModel *PageDriverTableView::model() const
{
    return mp_View->model();
}

void PageDriverTableView::initHeaderView(const QStringList &headerList, bool check)
{
    mp_View->initHeaderView(headerList, check);
}

void PageDriverTableView::setHeaderCbStatus(bool checked)
{
    mp_View->setHeaderCbStatus(checked);
}

void PageDriverTableView::setCheckedCBDisnable()
{
    mp_View->setCheckedCBDisable();
}

void PageDriverTableView::getCheckedDriverIndex(QList<int> &lstIndex)
{
    mp_View->getCheckedDriverIndex(lstIndex);
}

void PageDriverTableView::setItemStatus(int index, Status s)
{
    mp_View->setItemStatus(index, s);
}

void PageDriverTableView::setErrorMsg(int index, const QString &msg)
{
    mp_View->setErrorMsg(index, msg);
}

bool PageDriverTableView::hasItemDisabled()
{
    return mp_View->hasItemDisabled();
}

void PageDriverTableView::clear()
{
    this->setFixedHeight(DRIVER_TABLE_HEADER_HEIGHT + 5);
    mp_View->clear();
}

void PageDriverTableView::setItemOperationEnable(int index, bool enable)
{
    mp_View->setItemOperationEnable(index, enable);
}

void PageDriverTableView::removeItemAndWidget(int row, int column)
{
    mp_View->removeItemAndWidget(row, column);
}

void PageDriverTableView::setHeaderCbEnable(bool enable)
{
    mp_View->setHeaderCbEnable(enable);
}

void PageDriverTableView::paintEvent(QPaintEvent *e)
{
    DWidget::paintEvent(e);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    int radius = 8;

//     获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 设置Widget固定高度,(+1)表示包含表头高度,(*2)表示上下边距，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
    QRect rect  = QRect(this->rect().x(), this->rect().y(), this->rect().width(), this->rect().height()-2);

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    int width = 1;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
}

void PageDriverTableView::resizeEvent(QResizeEvent *e)
{
    if(m_PreWidth < 680){
        m_PreWidth = this->width();
        return DWidget::resizeEvent(e);
    }


    int detal = this->width() - m_PreWidth;
    m_PreWidth = this->width();

    // 宽度不变，不做处理
    if(0 == detal) {
        return DWidget::resizeEvent(e);
    }

    // 动态改变表格宽度
    resizeColumnWidth(detal);
    return DWidget::resizeEvent(e);
}

void PageDriverTableView::initWidgets()
{
    this->setFixedHeight(DRIVER_TABLE_HEADER_HEIGHT);
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mp_View);
    mainLayout->addStretch();
    this->setLayout(mainLayout);
}

void PageDriverTableView::resizeColumnWidth(int detal)
{
    bool newDriverTable = mp_View->columnWidth(5) == 0;
    if(newDriverTable){
        int columnZeroWidth = mp_View->columnWidth(0);
        if(detal > 0){ // 放大
            mp_View->setColumnWidth(0,columnZeroWidth + detal);
        }else{ // 缩小
            if(columnZeroWidth + detal > 120){
                mp_View->setColumnWidth(0,columnZeroWidth + detal);
            }
        }
    }else{
        int columnOneWidth = mp_View->columnWidth(1);
        int columnTwoWidth = mp_View->columnWidth(2);
        int columnFourWidth = mp_View->columnWidth(4);
        if(detal > 0){ // 放大
            // 第一步：先放大第一列
            int detalOne = 324 - columnOneWidth;
            if(detalOne - detal >= 0){ // 此时只需要放大第一列
                mp_View->setColumnWidth(1,columnOneWidth + detal);
                return;
            }else {
                mp_View->setColumnWidth(1,columnOneWidth + detalOne);
                detal = detal - detalOne;
            }

            // 第二步：放大第二列
            int detalTwo = 186 - columnTwoWidth;
            if(detalTwo - detal >= 0){ // 此时只需要放大第一列
                mp_View->setColumnWidth(2,columnTwoWidth + detal);
                return;
            }else {
                mp_View->setColumnWidth(2,columnTwoWidth + detalTwo);
                detal = detal - detalTwo;
            }

            // 第三步：放大第四列
            int detalFour = 150 - columnFourWidth;
            if(detalFour - detal >= 0){ // 此时只需要放大第一列
                mp_View->setColumnWidth(4,columnFourWidth + detal);
                return;
            }else {
                mp_View->setColumnWidth(4,columnFourWidth + detalFour);
                detal = detal - detalFour;
            }

            // 第五步：继续放大第一列
            columnOneWidth = mp_View->columnWidth(1);
            mp_View->setColumnWidth(1, columnOneWidth + detal);

        }else{ // 缩小
            // 第一步：先缩小第一列
            int detalOne = columnOneWidth - 120; // 计算第一列可缩小距离
            if(detalOne + detal >= 0){ // 此时只需要降低第一列
                mp_View->setColumnWidth(1,columnOneWidth + detal);
                return;
            }else { // 此时需要降低第二列，但是先把第一列降低了再说
                mp_View->setColumnWidth(1,columnOneWidth - detalOne);
                detal = detal + detalOne;
            }

            // 第二步：缩小第二列
            int detalTwo = columnTwoWidth - 120;
            if(detalTwo + detal >= 0){ // 此时只需要降低第一列
                mp_View->setColumnWidth(2,columnTwoWidth + detal);
                return;
            }else { // 此时需要降低第二列，但是先把第一列降低了再说
                mp_View->setColumnWidth(2,columnTwoWidth - detalTwo);
                detal = detal + detalTwo;
            }

            // 第三步：缩小第四列
            int detalFour = columnFourWidth - 120;
            if(detalFour + detal >= 0){ // 此时只需要降低第一列
                mp_View->setColumnWidth(4,columnFourWidth + detal);
            }else { // 此时需要降低第二列，但是先把第一列降低了再说
                mp_View->setColumnWidth(4,columnFourWidth - detalFour);
            }
        }
    }
}
