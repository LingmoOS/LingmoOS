#include "item.h"
#include "calculatefilesize.h"
#include <QApplication>
#include <QLabel>
#include <QPainterPath>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QtSvg/QSvgRenderer>
ItemTitlebar::ItemTitlebar(const QString &label1_, const QString &label2_,
                           const qreal &label1LeftMargin_, const qreal &label2LeftMargin_,
                           const QRectF &iconPosSize_, const qreal &iconRadius_, QWidget *parent)
    : QFrame(parent),
      label1(label1_),
      label2(label2_),
      label1LeftMargin(label1LeftMargin_),
      label2LeftMargin(label2LeftMargin_),
      iconPosSize(iconPosSize_),
      iconRadius(iconRadius_)
{
    selectAllButton = new SelectAllButton(this);
    selectAllButton->move(iconPosSize.x(), iconPosSize.y());

    QObject::connect(selectAllButton, &SelectAllButton::selectAll, this, &ItemTitlebar::selectAll);

    initUI();
}

ItemTitlebar::ItemTitlebar(QWidget *parent) : QFrame(parent)
{
    selectAllButton = new SelectAllButton(this);
    selectAllButton->move(iconPosSize.x(), iconPosSize.y());
    QObject::connect(selectAllButton, &SelectAllButton::selectAll, this, &ItemTitlebar::selectAll);

    initUI();
}

ItemTitlebar::~ItemTitlebar() { }

void ItemTitlebar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(0, 0, 0, 200), 0.3));
    QPainterPath bottomLine;
    bottomLine.moveTo(type ? 3 : 50, 33);
    bottomLine.lineTo(size().width(), 33);
    painter.drawPath(bottomLine);

    QPainterPath line1;
    line1.moveTo(label2LeftMargin, 3);
    line1.lineTo(label2LeftMargin, 30);
    painter.drawPath(line1);

    QPainterPath line2;
    line2.moveTo(type ? label1LeftMargin : label2LeftMargin + 139, 3);
    line2.lineTo(type ? label1LeftMargin : label2LeftMargin + 139, 30);
    painter.drawPath(line2);
}

void ItemTitlebar::updateSelectAllButState(ListSelectionState selectState)
{
    selectAllButton->changeState(selectState);
}

void ItemTitlebar::initUI()
{
    setProperty("class", "myqframe");
    setContentsMargins(0, 0, 0, 0);
    setStyleSheet(".QLabel{"
                  "color: rgba(65,77,104,1);"
                  "opacity: 1;"
                  "font-family: \"SourceHanSansSC-Medium\";"
                  "font-size: 14px;"
                  "font-weight: 500;"
                  "font-style: normal;"
                  "text-align: left;}"
                  ".myqframe{"
                  "border: none;"
                  "border-radius: 8px;"
                  "}");
    QLabel *title1 = new QLabel(label1, this);
    QLabel *title2 = new QLabel(label2, this);

    title1->setGeometry(label1LeftMargin + 10, 8, 177.3, 20);
    title2->setGeometry(label2LeftMargin + 12, 8, 61.3, 20);

    sortButton1 = new SortButton(this);
    sortButton1->move(label2LeftMargin - 25, iconPosSize.y());
    sortButton1->setVisible(false);

    sortButton2 = new SortButton(this);
    sortButton2->move(label2LeftMargin + 115, iconPosSize.y());
    sortButton2->setVisible(false);
}

SortButton *ItemTitlebar::getSortButton1() const
{
    return sortButton1;
}

SortButton *ItemTitlebar::getSortButton2() const
{
    return sortButton2;
}

void ItemTitlebar::setIconRadius(qreal newIconRadius)
{
    iconRadius = newIconRadius;
}

void ItemTitlebar::setType(bool value)
{
    type = value;
}

void ItemTitlebar::setIconPosSize(const QRectF &newIconPosSize)
{
    iconPosSize = newIconPosSize;
}

void ItemTitlebar::setLabel2LeftMargin(qreal newLabel2LeftMargin)
{
    label2LeftMargin = newLabel2LeftMargin;
}

void ItemTitlebar::setLabel2(const QString &newLabel2)
{
    label2 = newLabel2;
}

void ItemTitlebar::setLabel1LeftMargin(qreal newLabel1LeftMargin)
{
    label1LeftMargin = newLabel1LeftMargin;
}

void ItemTitlebar::setLabel1(const QString &newLabel1)
{
    label1 = newLabel1;
}

ItemDelegate::ItemDelegate() { }

ItemDelegate::ItemDelegate(const qreal &filenameTextLeftMargin_, const qreal &filenameTextMaxLen_,
                           const qreal &remarkTextLeftMargin_, const qreal &remarkTextMaxLen_,
                           const qreal &backgroundColorLeftMargin_, const QPoint &iconPos_,
                           const QPoint &checkBoxPos_)
    : remarkTextLeftMargin(remarkTextLeftMargin_),
      remarkTextMaxLen(remarkTextMaxLen_),
      filenameTextLeftMargin(filenameTextLeftMargin_),
      filenameTextMaxLen(filenameTextMaxLen_),
      backgroundColorLeftMargin(backgroundColorLeftMargin_),
      iconPos(iconPos_),
      checkBoxPos(checkBoxPos_)
{
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    // draw back
    paintBackground(painter, option, index);

    // draw icon
    paintIcon(painter, option, index);

    // draw text
    paintText(painter, option, index);

    // draw checkbox
    paintCheckbox(painter, option, index);
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(480, 36);
}

void ItemDelegate::paintIcon(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->save();

    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    painter->setRenderHint(QPainter::Antialiasing);
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (!icon.isNull()) {
        QPoint pos = option.rect.topLeft();
        pos += iconPos;

        QRect positon = QRect(pos, QSize(24, 24));
        icon.paint(painter, positon, Qt::AlignLeft | Qt::AlignVCenter);
    }
    painter->restore();
}

void ItemDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    painter->save();

    painter->setPen(Qt::NoPen);

    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    QColor evencolor = QColor(0, 0, 0, 20);
    QPoint topleft = option.rect.topLeft();
    QRect positon(backgroundColorLeftMargin, topleft.y(), 440, 36);
    if (index.row() % 2 != 0) {
        painter->setBrush(evencolor);
        painter->drawRoundedRect(positon, 10, 10);
    }

    painter->restore();
}

void ItemDelegate::paintText(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->save();

    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    QRect filenameTextPos = option.rect.adjusted(filenameTextLeftMargin, 0, 0, 0);

    QFont filenameTextFont;
    filenameTextFont.setPixelSize(14);
    QString filenameText = index.data(Qt::DisplayRole).toString();
    QFontMetrics filenameMetrics(filenameTextFont);
    filenameText = filenameMetrics.elidedText(filenameText, Qt::ElideRight, filenameTextMaxLen);
    painter->drawText(filenameTextPos, Qt::AlignLeft | Qt::AlignVCenter, filenameText);

    QRect remarkTextPos = option.rect.adjusted(remarkTextLeftMargin, 0, 0, 0);
    QFont remarkTextFont;
    remarkTextFont.setPixelSize(14);
    QString remarkText = index.data(Qt::ToolTipRole).toString();
    QFontMetrics remarkMetrics(remarkTextFont);
    remarkTextFont = remarkMetrics.elidedText(filenameText, Qt::ElideRight, remarkTextMaxLen);
    painter->drawText(remarkTextPos, Qt::AlignLeft | Qt::AlignVCenter, remarkText);

    painter->restore();
}

void ItemDelegate::paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    painter->save();

    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    painter->setRenderHint(QPainter::Antialiasing);
    QPoint pos = option.rect.topLeft() + checkBoxPos;
    QRect checkBoxRect = QRect(pos.x(), pos.y(), 18, 18);

    painter->setPen(QColor(0, 0, 0, 255));
    painter->drawRoundedRect(checkBoxRect, 4, 4);

    if (index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked) {
       QRect iconRect(checkBoxRect.left() + 3, checkBoxRect.top() + 3, 13, 11);
//        QSvgRenderer render(QString(":/icon/check_black.svg"));
//        render.render(painter, iconRect);
        QPixmap pixmap = QIcon(":/icon/check_black.svg").pixmap(13,11);
        painter->drawPixmap(iconRect,pixmap);
    }
    painter->restore();
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                               const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
        || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonDblClick)
        || (event->type() == QEvent::MouseButtonPress)) {
        //  QRect checkBoxRect = option.rect;
        QPoint pos = option.rect.topLeft() + checkBoxPos;
        QRect checkBoxRect = QRect(pos.x(), pos.y(), 18, 18);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton || !checkBoxRect.contains(mouseEvent->pos()))
            return false;

        // eat the double click events inside the check rect
        if ((event->type() == QEvent::MouseButtonPress)
            || (event->type() == QEvent::MouseButtonDblClick)) {
            return true;
        }

        if (checkBoxRect.contains(mouseEvent->pos())) {
            Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
            state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            return model->setData(index, state, Qt::CheckStateRole);
        }
    }
}

void ItemDelegate::setCheckBoxPos(QPoint newCheckBoxPos)
{
    checkBoxPos = newCheckBoxPos;
}

void ItemDelegate::setIconPos(QPoint newIconPos)
{
    iconPos = newIconPos;
}

void ItemDelegate::setBackgroundColorLeftMargin(qreal newBackgroundColorLeftMargin)
{
    backgroundColorLeftMargin = newBackgroundColorLeftMargin;
}

void ItemDelegate::setFilenameTextMaxLen(qreal newFilenameTextMaxLen)
{
    filenameTextMaxLen = newFilenameTextMaxLen;
}

void ItemDelegate::setFilenameTextLeftMargin(qreal newFilenameTextLeftMargin)
{
    filenameTextLeftMargin = newFilenameTextLeftMargin;
}

void ItemDelegate::setRemarkTextMaxLen(qreal newRemarkTextMaxLen)
{
    remarkTextMaxLen = newRemarkTextMaxLen;
}

void ItemDelegate::setRemarkTextLeftMargin(qreal newRemarkTextLeftMargin)
{
    remarkTextLeftMargin = newRemarkTextLeftMargin;
}

SaveItemDelegate::SaveItemDelegate() { }

SaveItemDelegate::~SaveItemDelegate() { }

void SaveItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{

    // draw back
    paintBackground(painter, option, index);

    // draw icon
    paintIcon(painter, option, index);

    // draw text
    paintText(painter, option, index);

    // draw checkbox
    // paintCheckbox(painter, option, index);
}

QSize SaveItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(440, 58);
}

void SaveItemDelegate::paintIcon(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    painter->save();
    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (!icon.isNull()) {
        QPoint pos = option.rect.topLeft();
        pos += iconPos;
        QRect positon = QRect(pos, QSize(32, 32));
        icon.paint(painter, positon, Qt::AlignLeft | Qt::AlignVCenter);
    }
    painter->restore();
}

void SaveItemDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    painter->save();
    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    QPoint topleft = option.rect.topLeft();
    QRect positon(topleft.x(), topleft.y(), 440, 48);

    if (index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked) {
        QPen pen;
        pen.setColor(QColor(0, 129, 255, 100));
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(QColor(0, 129, 255, 50));

        painter->drawRoundedRect(positon, 8, 8);

        QIcon icon(":/icon/select.svg");
        icon.paint(painter, QRect(topleft.x() + 408, topleft.y() + 13, 32, 32));
    } else {
        QColor evencolor(0, 0, 0, 12);

        painter->setPen(Qt::NoPen);

        painter->setBrush(evencolor);
        painter->drawRoundedRect(positon, 8, 8);
    }
    painter->restore();
}

void SaveItemDelegate::paintText(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    painter->save();
    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    QRect filenameTextPos = option.rect.adjusted(filenameTextLeftMargin, -5, 0, 0);

    QFont filenameTextFont;
    filenameTextFont.setPixelSize(12);
    QString filenameText = index.data(Qt::DisplayRole).toString();
    QFontMetrics filenameMetrics(filenameTextFont);
    filenameText = filenameMetrics.elidedText(filenameText, Qt::ElideRight, filenameTextMaxLen);
    painter->drawText(filenameTextPos, Qt::AlignLeft | Qt::AlignVCenter, filenameText);

    QRect remarkTextPos = option.rect.adjusted(remarkTextLeftMargin, -5, -remarkTextRightMargin, 0);
    QFont remarkTextFont;
    remarkTextFont.setPixelSize(12);
    QString remarkText = index.data(Qt::ToolTipRole).toString();
    QFontMetrics remarkMetrics(remarkTextFont);
    remarkTextFont = remarkMetrics.elidedText(filenameText, Qt::ElideRight, remarkTextMaxLen);
    painter->drawText(remarkTextPos, Qt::AlignRight | Qt::AlignVCenter, remarkText);

    painter->restore();
}

void SaveItemDelegate::paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    painter->save();
    if (index.data(Qt::BackgroundRole).toBool() == true) {
        painter->setOpacity(opacity);
    }
    QPoint pos = option.rect.topLeft() + checkBoxPos;
    QRect checkBoxRect = QRect(pos.x(), pos.y(), 18, 18);

    QItemDelegate::drawCheck(painter, option, checkBoxRect,
                             index.data(Qt::CheckStateRole).value<Qt::CheckState>());
    painter->restore();
}

bool SaveItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
        || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;

    // make sure that we have the right event type
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonDblClick)
        || (event->type() == QEvent::MouseButtonPress)) {

        QRect checkBoxRect = option.rect;

        QRect emptyRect;
        doLayout(option, &checkBoxRect, &emptyRect, &emptyRect, false);
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() != Qt::LeftButton || !checkBoxRect.contains(me->pos()))
            return false;

        // eat the double click events inside the check rect
        if ((event->type() == QEvent::MouseButtonPress)
            || (event->type() == QEvent::MouseButtonDblClick))
            return true;

    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space
            && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
    if (flags & Qt::ItemIsUserTristate)
        state = ((Qt::CheckState)((state + 1) % 3));
    else
        state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    return model->setData(index, state, Qt::CheckStateRole);
}

SidebarItemDelegate::SidebarItemDelegate() { }

SidebarItemDelegate::~SidebarItemDelegate() { }

void SidebarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    paintBackground(painter, option, index);
    paintText(painter, option, index);
    paintCheckbox(painter, option, index);
}

QSize SidebarItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(180, 36);
}

void SidebarItemDelegate::paintText(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    painter->save();
    QPen pen;
    if (index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked) {
        pen.setColor(Qt::white);
    } else {
        pen.setColor(Qt::black);
    }
    painter->setPen(pen);
    QRect filenameTextPos = option.rect.adjusted(37, -2, 0, 0);

    QFont filenameTextFont;
    filenameTextFont.setPixelSize(12);
    QString filenameText = index.data(Qt::DisplayRole).toString();
    QFontMetrics filenameMetrics(filenameTextFont);
    filenameText = filenameMetrics.elidedText(filenameText, Qt::ElideRight, 90);
    painter->drawText(filenameTextPos, Qt::AlignLeft | Qt::AlignVCenter, filenameText);

    QRect remarkTextPos = option.rect.adjusted(138, -2, 0, 0);
    QFont remarkTextFont;
    remarkTextFont.setPixelSize(12);
    QString remarkText = index.data(Qt::ToolTipRole).toString();
    QFontMetrics remarkMetrics(remarkTextFont);
    remarkText = remarkMetrics.elidedText(remarkText, Qt::ElideRight, 50);
    painter->drawText(remarkTextPos, Qt::AlignLeft | Qt::AlignVCenter, remarkText);

    painter->restore();
}
void SidebarItemDelegate::paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QColor color;
    QString icon;
    if (index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked) {
        color = QColor(255, 255, 255, 255);
        icon = ":/icon/check_white.svg";
    } else {
        color = QColor(0, 0, 0, 255);
        icon = ":/icon/check_black.svg";
    }

    painter->setPen(color);
    QPoint pos = option.rect.topLeft() + QPoint(9, 7);
    QRect checkBoxRect = QRect(pos.x(), pos.y(), 18, 18);
    painter->drawRoundedRect(checkBoxRect, 4, 4);

    if (index.data(Qt::StatusTipRole).value<int>() == 0) {
        QRect iconRect(checkBoxRect.left() + 3, checkBoxRect.top() + 3, 13, 11);
//        QSvgRenderer render(icon);
        QPixmap pixmap = QIcon(icon).pixmap(13,11);
        painter->drawPixmap(iconRect,pixmap);
    } else if (index.data(Qt::StatusTipRole).value<int>() == 1) {
        int y = checkBoxRect.top() + 9;
        int x1 = checkBoxRect.left() + 4;
        int x2 = checkBoxRect.left() + 14;
        painter->setPen(QPen(color, 2));
        painter->drawLine(x1, y, x2, y);
    }
    painter->restore();
}
void SidebarItemDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    painter->save();
    QRect positon(option.rect);
    painter->setPen(Qt::NoPen);
    if (index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked) {
        painter->setBrush(QColor(0, 129, 255, 255));

    } else {
        painter->setBrush(QColor(255, 255, 255));
    }

    painter->drawRoundedRect(positon, 8, 8);
    painter->restore();
}
bool SidebarItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
        || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonDblClick)
        || (event->type() == QEvent::MouseButtonPress)) {
        QRect checkBoxRect = option.rect;
        QPoint pos = option.rect.topLeft() + QPoint(9, 7);
        QRect stateCheckBoxRect = QRect(pos.x(), pos.y(), 18, 18);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton || !checkBoxRect.contains(mouseEvent->pos()))
            return false;

        // eat the double click events inside the check rect
        if ((event->type() == QEvent::MouseButtonPress)
            || (event->type() == QEvent::MouseButtonDblClick)) {
            return true;
        }

        if (stateCheckBoxRect.contains(mouseEvent->pos()))
            model->setData(index, true, Qt::WhatsThisRole);

        if (checkBoxRect.contains(mouseEvent->pos())) {
            model->setData(index, Qt::Checked, Qt::CheckStateRole);
            return true;
        }
    }

    return false;
}

SelectAllButton::SelectAllButton(QWidget *parent) : QFrame(parent)
{
    setFixedSize(20, 20);
}

SelectAllButton::~SelectAllButton() { }

void SelectAllButton::changeState(ListSelectionState state)
{
    if (curState == state)
        return;
    curState = state;
    update();
}

void SelectAllButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color;

    painter.setPen(color);
    painter.drawRoundedRect(iconPosSize, iconRadius, iconRadius);

    if (curState == ListSelectionState::selectall) {
        QRect iconRect(iconPosSize.left() + 3, iconPosSize.top() + 3, 13, 11);
        QPixmap pixmap = QIcon(":/icon/check_black.svg").pixmap(13,11);
        painter.drawPixmap(iconRect,pixmap);
    } else if (curState == ListSelectionState::selecthalf) {
        int y = iconPosSize.top() + 9;
        int x1 = iconPosSize.left() + 4;
        int x2 = iconPosSize.left() + 14;
        painter.setPen(QPen(QColor(0, 0, 0, 255), 2));
        painter.drawLine(x1, y, x2, y);
    }
}

void SelectAllButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        ListSelectionState state;
        if (curState == ListSelectionState::selectall) {
            state = ListSelectionState::unselected;
        } else {
            state = ListSelectionState::selectall;
        }
        changeState(state);
        emit selectAll();
    }
    return QFrame::mousePressEvent(event);
}

SelectListView::SelectListView(QFrame *parent) : QListView(parent)
{
    QStandardItemModel *model = new QStandardItemModel(this);
    setStyleSheet(".SelectListView{"
                  "border: none;"
                  "}");
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QAbstractItemView::NoSelection);
    setVerticalScrollMode(QListView::ScrollPerPixel);
    QObject::connect(model, &QStandardItemModel::itemChanged, this,
                     &SelectListView::updateCurSelectItem);

    proxyModel = new SortProxyModel(this);
    proxyModel->setSourceModel(model);
    setModel(proxyModel);
}

SelectListView::~SelectListView() { }

void SelectListView::setAllSize(int size)
{
    allSize = size;
}

QStandardItemModel *SelectListView::getModel()
{
    return qobject_cast<QStandardItemModel *>(proxyModel->sourceModel());
}

void SelectListView::setSortRole(bool flag)
{
    proxyModel->setFlag(flag);
}

void SelectListView::selectorDelAllItem()
{
    QStandardItemModel *model = getModel();
    Qt::CheckState state = Qt::Unchecked;
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex itemIndex = model->index(row, 0);
        if (itemIndex.data(Qt::BackgroundRole).toBool() == true)
            continue;
        if (itemIndex.data(Qt::CheckStateRole) == Qt::Unchecked) {
            state = Qt::Checked;
            break;
        }
    }
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex itemIndex = model->index(row, 0);
        if (itemIndex.data(Qt::BackgroundRole).toBool() == true)
            continue;
        model->setData(itemIndex, state, Qt::CheckStateRole);
    }
}

void SelectListView::updateCurSelectItem(QStandardItem *item)
{
    if (item->data(Qt::CheckStateRole) == Qt::Checked) {
        curSelectItemNum++;
    } else {
        curSelectItemNum--;
    }
    if (curSelectItemNum < 0) {
        curSelectItemNum = 0;
        return;
    }
    ListSelectionState state;

    int tempAllsize = allSize == -1 ? model()->rowCount() : allSize;

    if (curSelectItemNum == 0) {
        state = ListSelectionState::unselected;
    } else if (curSelectItemNum < tempAllsize) {
        state = ListSelectionState::selecthalf;
    } else {
        state = ListSelectionState::selectall;
    }
    if (state == curSelectState) {
        return;
    }
    curSelectState = state;
    emit currentSelectState(curSelectState);
}

void SelectListView::sortListview()
{
    proxyModel->setMode(ascendingOrder);
    if (ascendingOrder) {
        proxyModel->sort(0, Qt::AscendingOrder);
    } else {
        proxyModel->sort(0, Qt::DescendingOrder);
    }
    ascendingOrder = !ascendingOrder;
    // setModel(proxyModel);
}

SortButton::SortButton(QWidget *parent) : QPushButton(parent)
{
    setIcon(QIcon(":/icon/arrow_black.svg"));
    setStyleSheet(".SortButton { border: none; }");
    setIconSize(QSize(16, 16));
}

SortButton::~SortButton() { }

void SortButton::mousePressEvent(QMouseEvent *event)
{
    if (flag) {
        QMatrix matrix;
        matrix.rotate(180);
        setIcon(QPixmap(":/icon/arrow_black.svg").transformed(matrix, Qt::SmoothTransformation));
    } else {
        setIcon(QIcon(":/icon/arrow_black.svg"));
    }
    flag = !flag;
    emit sort();
    return QPushButton::mouseMoveEvent(event);
}

SortProxyModel::SortProxyModel(QObject *parent) : QSortFilterProxyModel(parent) { }

SortProxyModel::~SortProxyModel() { }

void SortProxyModel::setMode(bool Model)
{
    sortModel = Model;
}

void SortProxyModel::setFlag(bool flag)
{
    flagDisplayrole = flag;
}

bool SortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftCategory = sourceModel()->data(left, Qt::ToolTipPropertyRole);
    QVariant rightCategory = sourceModel()->data(right, Qt::ToolTipPropertyRole);

    if (leftCategory.type() == QVariant::Int && rightCategory.type() == QVariant::Int) {
        if (leftCategory.toInt() != rightCategory.toInt()) {
            if (sortModel)
                return leftCategory.toInt() < rightCategory.toInt();
            else
                return leftCategory.toInt() > rightCategory.toInt();
        }
    }
    if (flagDisplayrole) {
        QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
        QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);

        if (leftData.type() == QVariant::String && rightData.type() == QVariant::String) {
            QChar firstLetter1 = leftData.toString().at(0);
            QChar firstLetter2 = rightData.toString().at(0);
            return firstLetter1 < firstLetter2;
        }
    } else {
        QVariant leftData = sourceModel()->data(left, Qt::ToolTipRole);
        QVariant rightData = sourceModel()->data(right, Qt::ToolTipRole);

        if (leftData.type() == QVariant::String && rightData.type() == QVariant::String) {
            quint64 firstLetter1 = fromQstringToByte(leftData.toString());
            quint64 firstLetter2 = fromQstringToByte(rightData.toString());
            return firstLetter1 > firstLetter2;
        }
    }

    return false;
}
