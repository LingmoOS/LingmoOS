#include "detaillisttable.h"
#include <QPainter>

#include <QHelpEvent>
#include <QAbstractItemView>
#include <QToolTip>
#include <QStyleOptionViewItem>
#include <QPainterPath>

const int space = 10;
const int margin = 10;
BaseDetailItemDelegate::BaseDetailItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

BaseDetailItemDelegate::~BaseDetailItemDelegate()
{
}

void BaseDetailItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    auto palette = option.palette;
    QBrush background = palette.color(QPalette::Active, QPalette::Base);
    if (!(index.row() & 1)) background = palette.color(QPalette::Active, QPalette::AlternateBase);

    painter->save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(option.widget->rect().adjusted(1, 1, -1, -1), 6, 6);
    painter->setClipPath(clipPath);

    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRect(option.rect);

    if (index.isValid()) {
        const auto &ltextpalette = index.data(Qt::TextColorRole).value<QColor>();
        painter->setPen(ltextpalette);

        QString ltext = index.data().toString();
        QString rtext = index.data(Qt::UserRole).toString();

        int rtmpW = painter->fontMetrics().width(rtext) + space;
        int ltmpW = painter->fontMetrics().width(ltext) + space;

        int leftWidth = qMin(qMax(option.rect.width() - rtmpW - 2 * margin, 68), ltmpW);
        int rightWidth = qMin(option.rect.width() - leftWidth - 2 * margin, rtmpW);

        ltext = painter->fontMetrics().elidedText(ltext, Qt::ElideRight, leftWidth);
        painter->drawText(option.rect.adjusted(margin, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, ltext);

        const auto &rtextpalette = QPalette();
        painter->setPen(rtextpalette.color(QPalette::Text));

        rtext = painter->fontMetrics().elidedText(rtext, Qt::ElideRight, rightWidth);
        painter->drawText(option.rect.adjusted(0, 0, -margin, 0), Qt::AlignRight | Qt::AlignVCenter, rtext);
    }
    painter->restore();
}

QSize BaseDetailItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    return QSize(option.rect.width(), 36);
}

bool BaseDetailItemDelegate::helpEvent(QHelpEvent *e, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!e || !view)
        return false;

    // only process tooltip events for now
    if (e->type() == QEvent::ToolTip) {
        QFontMetrics fm(option.font);
        QString ltext = index.data().toString();
        QString rtext = index.data(Qt::UserRole).toString();

        int rtmpW = fm.width(rtext) + space;
        int ltmpW = fm.width(ltext) + space;

        int leftWidth = qMin(qMax(option.rect.width() - rtmpW - 2 * margin, 68), ltmpW);
        int rightWidth = qMin(option.rect.width() - leftWidth - 2 * margin, rtmpW);

        QRect leftRect = option.rect;
        leftRect.setWidth(leftWidth);

        QRect rightRect = option.rect;
        rightRect.setWidth(rightWidth);

        if (leftRect.contains(e->pos()) && leftWidth < ltmpW) {
            QToolTip::showText(e->globalPos(), QString("<div>%1</div>").arg(ltext.toHtmlEscaped()), view);
            return true;
        }

        if (rightRect.contains(e->pos()) && rightWidth < rtmpW) {
            QToolTip::showText(e->globalPos(), QString("<div>%1</div>").arg(rtext.toHtmlEscaped()), view);
            return true;
        }

        if (!QStyledItemDelegate::helpEvent(e, view, option, index))
            QToolTip::hideText();

        return true;
    }

    return QStyledItemDelegate::helpEvent(e, view, option, index);
}


DetailListTable::DetailListTable(QAbstractTableModel *dataModel, QStyledItemDelegate *baseStyle, QWidget *parent)
    :m_model(dataModel),
     mBaseStyleDelegate(baseStyle),
     QTableView(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);

    this->horizontalHeader()->setStretchLastSection(true);

    this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//水平表格自动拉伸
    this->setShowGrid(false);//不显示表格线
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    this->setGridStyle(Qt::NoPen);
    this->setFrameShape(QFrame::NoFrame);

    this->setModel(m_model);
    if (mBaseStyleDelegate == nullptr) {
        this->setItemDelegate(new BaseDetailItemDelegate());
    }else {
        this->setItemDelegate(mBaseStyleDelegate);
    }


    this->setFixedHeight(260);

    auto *scroller = QScroller::scroller(viewport());
    auto prop = scroller->scrollerProperties();
    // turn off overshoot to fix performance issue
    prop.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    prop.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0);
    prop.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 1);
    // lock scroll direction to fix performance issue
    prop.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1);
    scroller->setScrollerProperties(prop);
    // enable touch gesture
    QScroller::grabGesture(viewport(), QScroller::TouchGesture);
}

DetailListTable::~DetailListTable()
{

}

void DetailListTable::onModelUpdate()
{
    this->viewport()->update();
}

void DetailListTable::paintEvent(QPaintEvent *event)
{
    QTableView::paintEvent(event);

    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::Antialiasing, true);
    const auto &palette = QPalette();
    QColor frameColor = palette.color(QPalette::Text);
    frameColor.setAlphaF(0.1);

    painter.setPen(QPen(frameColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(this->horizontalHeader()->sectionSize(0) - 1, 2, this->horizontalHeader()->sectionSize(0) - 1, this->viewport()->height() - 2);
//    painter.drawRoundedRect(this->rect().adjusted(1, 1, -1, -1), 6, 6);
}
