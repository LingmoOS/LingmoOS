// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datatreeview.h"
#include "treeheaderview.h"
#include "datamodel.h"
#include "uitools.h"

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>

#include <QMouseEvent>
#include <QPainter>
#include <QHeaderView>
#include <QMimeData>
#include <QDebug>
#include <QScrollBar>
#include <QPainterPath>

StyleTreeViewDelegate::StyleTreeViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

StyleTreeViewDelegate::~StyleTreeViewDelegate()
{

}

QSize StyleTreeViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        return QSize(option.rect.width(), TABLE_HEIGHT_NormalMode);
    } else {
        return QSize(option.rect.width(), TABLE_HEIGHT_CompactMode);
    }
#else
    return QSize(option.rect.width(), TABLE_HEIGHT_NormalMode);
#endif
}

void StyleTreeViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);
    //设置高亮文字色
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            forground.setColor(palette.color(cg, DPalette::HighlightedText));
        }
    }

    painter->setPen(forground);
    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QPainterPath path, clipPath;
    QRect textRect = rect;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        // 左间距
        rect.setX(rect.x() + margin);
    } break;
    case QStyleOptionViewItem::Middle: {
    } break;
    case QStyleOptionViewItem::End: {
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    case QStyleOptionViewItem::OnlyOne: {
        // 左间距
        rect.setX(rect.x() + margin);
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    default: {
        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }

    //绘制图标
    if (QStyleOptionViewItem::Beginning == opt.viewItemPosition &&
            index.data(Qt::DecorationRole).isValid()) {
        // icon size
        auto iconSize = style->pixelMetric(DStyle::PM_ListViewIconSize, &option);
        QRect iconRect = rect;
        iconRect.setX(rect.x() + margin);
        iconRect.setWidth(iconSize);
        // 缩放大小并绘制
        auto diff = (iconRect.height() - iconSize) / 2;
        opt.icon.paint(painter, iconRect.adjusted(0, diff, 0, -diff));
    }

    //绘制文字
    textRect = rect;
    if (0 == index.column()) {
        textRect.setX(textRect.x() + margin + 32 - 2);
    } else {
        textRect.setX(textRect.x() + margin - 2);
    }

    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

DataTreeView::DataTreeView(QWidget *parent)
    : DTreeView(parent)
{
    initUI();
    initConnections();
}

DataTreeView::~DataTreeView()
{

}

void DataTreeView::resetLevel()
{
    m_iLevel = 0;
    m_strCurrentPath = QDir::separator();
    setPreLblVisible(false);
}

void DataTreeView::initUI()
{
    setAttribute(Qt::WA_AcceptTouchEvents); // 设置接收触摸屏事件
    setObjectName("TableViewFile");
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setIconSize(QSize(24, 24));
    setViewportMargins(10, 10, 10, 0);
    setIndentation(0);
    setFrameShape(QFrame::NoFrame);     // 设置无边框
    resizeColumnWidth();
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFocusPolicy(Qt::StrongFocus);

    // 设置样式代理
    StyleTreeViewDelegate *pDelegate = new StyleTreeViewDelegate(this);
    setItemDelegate(pDelegate);

    // 设置表头
    m_pHeaderView = new TreeHeaderView(Qt::Horizontal, this);
    m_pHeaderView->setStretchLastSection(true);
    setHeader(m_pHeaderView);

    m_pModel = new DataModel(this);
    setModel(m_pModel);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);    // 设置自定义右键菜单
    setAcceptDrops(true);

    m_selectionModel = selectionModel();
}

void DataTreeView::initConnections()
{
    connect(m_pHeaderView->getpreLbl(), &PreviousLabel::doubleClickedSignal, this, &DataTreeView::slotPreClicked);
}

void DataTreeView::resizeColumnWidth()
{
    setColumnWidth(0, width() * 25 / 58);
    setColumnWidth(1, width() * 17 / 58);
    setColumnWidth(2, width() * 8 / 58);
    setColumnWidth(3, width() * 8 / 58);
}

TreeHeaderView *DataTreeView::getHeaderView() const
{
    return m_pHeaderView;
}

void DataTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
#endif
    }

    auto *style = dynamic_cast<DStyle *>(DApplication::style());

    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
    auto margin = style->pixelMetric(DStyle::PM_ContentsMargins, &options);
    //根据实际情况设置颜色，奇数行为灰色
    auto palette = options.palette;
    QBrush background;
    bool bVis = m_pHeaderView->getpreLbl()->isVisible();
    if (bVis ? (index.row() & 1) : !(index.row() & 1)) {
        if(DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            background = QColor(255, 255, 255, 12);
        } else {
            background = palette.color(cg, DPalette::AlternateBase);
        }
    } else {
        background = palette.color(cg, DPalette::Base);
    }
    if (options.state & DStyle::State_Enabled) {
        if (m_selectionModel->isSelected(index)) {
            background = palette.color(cg, DPalette::Highlight);
        }
    }

    // 绘制整行背景，高度-2以让高分屏非整数缩放比例下无被选中的蓝色细线，防止原来通过delegate绘制单元格交替颜色背景出现的高分屏非整数缩放比例下qrect精度问题导致的横向单元格间出现白色边框
    QPainterPath path;
    QRect rowRect { options.rect.x() - header()->offset(),
                    options.rect.y(),
                    header()->length() - header()->sectionPosition(0),
                    options.rect.height() - 2 };
    rowRect.setX(rowRect.x() + margin);
    rowRect.setWidth(rowRect.width() - margin);

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    QTreeView::drawRow(painter, options, index);
    // draw focus
    if (hasFocus() && currentIndex().row() == index.row() && (Qt::TabFocusReason == m_reson || Qt::BacktabFocusReason == m_reson)) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(options);
        o.state |= QStyle::State_KeyboardFocusChange | QStyle::State_HasFocus;
        o.rect = style->visualRect(layoutDirection(), viewport()->rect(), rowRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, painter);
    }

    painter->restore();
}

void DataTreeView::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    // qInfo() << m_reson << model()->rowCount() << currentIndex();
    if (Qt::BacktabFocusReason == m_reson || Qt::TabFocusReason == m_reson) { // 修复不能多选删除
        if (model()->rowCount() > 0) {
            if (currentIndex().isValid()) {
                m_selectionModel->select(currentIndex(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            } else {
                QModelIndex firstModelIndex = model()->index(0, 0);
                m_selectionModel->select(firstModelIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }

    DTreeView::focusInEvent(event);
}

void DataTreeView::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // 判断是否有url
    if (!mime->hasUrls()) {
        e->ignore();
    } else {
        // 判断是否是本地设备文件，过滤 手机 网络 ftp smb 等
        for (const auto &url : mime->urls()) {
            if (!UiTools::isLocalDeviceFile(url.toLocalFile())) {
                e->ignore();
                return;
            }
        }

        e->accept();
    }
}

void DataTreeView::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void DataTreeView::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (false == mime->hasUrls()) {
        e->ignore();
    }

    e->accept();

    // 判断本地文件
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        fileList << url.toLocalFile();
    }

    if (fileList.size() == 0) {
        return;
    }

    emit signalDragFiles(fileList);
}

void DataTreeView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    resizeColumnWidth();
}

bool DataTreeView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin: {

        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);

        if (!m_isPressed && touchEvent && touchEvent->device() && touchEvent->device()->type() == QTouchDevice::TouchScreen && touchEvent->touchPointStates() == Qt::TouchPointPressed) {

            QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
            //dell触摸屏幕只有一个touchpoint 但却能捕获到pinchevent缩放手势?
            if (points.count() == 1) {
                QTouchEvent::TouchPoint p = points.at(0);
                m_lastTouchBeginPos = p.pos();
                m_lastTouchBeginPos.setY(m_lastTouchBeginPos.y() - m_pHeaderView->height());
                m_lastTouchTime = QTime::currentTime();
                m_isPressed = true;

            }
        }
        break;

    }
    default:
        break;
    }
    return  DTreeView::event(event);
}

void DataTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    DTreeView::mouseReleaseEvent(event);
}

void DataTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        //最小距离为防误触和双向滑动时,只触发横向或者纵向的
        int touchmindistance = 2;
        //最大步进距离是因为原地点按马上放开,则会出现-35~-38的不合理位移,加上每次步进距离没有那么大,所以设置为30
        int touchMaxDistance = 30;
        event->accept();
        double horiDelta = event->pos().x() - m_lastTouchBeginPos.x();
        double vertDelta = event->pos().y() - m_lastTouchBeginPos.y();
        qInfo() << "event->pos()" << event->pos() << "m_pHeaderView->height()" << m_pHeaderView->height();
        if (qAbs(horiDelta) > touchmindistance && qAbs(horiDelta) < touchMaxDistance) {
            horizontalScrollBar()->setValue(static_cast<int>(horizontalScrollBar()->value() - horiDelta));
        }

        if (qAbs(vertDelta) > touchmindistance && !(qAbs(vertDelta) < m_pHeaderView->height() + 2 && qAbs(vertDelta) > m_pHeaderView->height() - 2 && m_lastTouchTime.msecsTo(QTime::currentTime()) < 100)) {
            double svalue = 1;
            if (vertDelta > 0) {
                //svalue = svalue;
            } else if (vertDelta < 0) {
                svalue = -svalue;
            } else {
                svalue = 0;
            }
            verticalScrollBar()->setValue(static_cast<int>(verticalScrollBar()->value() - vertDelta));
        }
        m_lastTouchBeginPos = event->pos();
        return;
    }
}

void DataTreeView::keyPressEvent(QKeyEvent *event)
{
    // 当前选中表格第一行，再点击Key_Up,选中返回上一层目录
    if (Qt::Key_Up == event->key()) {
        // 当前行为0或为空文件夹时，焦点放在返回上一层目录上
        if (this->currentIndex().row() == 0 || model()->rowCount() == 0) {
            if (m_pHeaderView->isVisiable()) {
                m_pHeaderView->setLabelFocus(true);
                m_selectionModel->clearSelection();
            }
        }
    }

    // 当前选中返回上一层目录，再点击Key_Down,选中表格第一行
    if (Qt::Key_Down == event->key()) {
        if (m_pHeaderView->isVisiable() && m_pHeaderView->isLabelFocus() && model()->rowCount() > 0) {
            m_pHeaderView->setLabelFocus(false);
            m_selectionModel->setCurrentIndex(m_pModel->index(-1, -1), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    }

    if (Qt::Key_Delete == event->key() && m_selectionModel->selectedRows().count() > 0) { //删除键
        slotDeleteFile();
    } else if (Qt::Key_M == event->key() && Qt::AltModifier == event->modifiers()
               && m_selectionModel->selectedRows().count() > 0) { //Alt+M组合键调用右键菜单
        int y =   36 * currentIndex().row() + 36 / 2; //获取选中行y坐标+行高/2,列表行高36
        int x = static_cast<int>(width() * 0.618); //比较合适的x坐标

        slotShowRightMenu(QPoint(x, y));
    } else if ((Qt::Key_Enter == event->key() || Qt::Key_Return == event->key())
               && m_selectionModel->selectedRows().count() > 0) { //回车键以默认方式打开文件(夹)
        handleDoubleClick(currentIndex());
    } else {
        DTreeView::keyPressEvent(event);
    }
}

void DataTreeView::setPreLblVisible(bool bVisible, const QString &strPath)
{
    m_pHeaderView->getpreLbl()->setPrePath(strPath);
    m_pHeaderView->setPreLblVisible(bVisible);
}
