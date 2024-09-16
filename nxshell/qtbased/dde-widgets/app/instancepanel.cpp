// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "instancepanel.h"
#include "instancemodel.h"
#include "instanceproxy.h"
#include "widgethandler.h"
#include "widgetmanager.h"
#include "aboutdialog.h"
#include "utils.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMoveEvent>
#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QCoreApplication>

#include <DIconButton>

WIDGETS_FRAME_BEGIN_NAMESPACE
static const char* MoveMimeDataFormat = "application/dde-widgets-instancedata";

InstancePanelCell::InstancePanelCell(Instance *instance, QWidget *parent)
    : DragDropWidget(parent)
    , m_instance(instance)
{
    setFocusPolicy(Qt::NoFocus);
    show();
}

void InstancePanelCell::setInstance(Instance *instance)
{
    m_instance = instance;
}

QWidget *InstancePanelCell::view() const
{
    return m_instance->view();
}

InstanceId InstancePanelCell::id() const
{
    return m_instance->handler()->id();
}

bool InstancePanelCell::isFixted() const
{
    return WidgetHandlerImpl::get(m_instance->handler())->isFixted();
}

bool InstancePanelCell::isCustom() const
{
    return WidgetHandlerImpl::get(m_instance->handler())->isCustom();
}

QList<QWidget *> InstancePanelCell::focusWidgetList() const
{
    return {};
}

void InstancePanelCell::startDrag(const QPoint &pos)
{
    QWidget *child = view();
    if (!child)
        return;

    if (isFixted())
        return;

    m_startDrag = mapToGlobal(child->pos());

    QPoint hotSpot = pos - child->pos();

    QMimeData *mimeData = new QMimeData;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << m_instance->handler()->id() << hotSpot;
    mimeData->setData(MoveMimeDataFormat, itemData);

    QPixmap pixmap(child->grab());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(hotSpot);

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

    if (dropAction == Qt::IgnoreAction) {
        gobackDrag(pixmap, QCursor::pos());
    }
}

InstancePanel::InstancePanel(WidgetManager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_views(new QWidget())
    , m_layout(new DFlowLayout(m_views))
{
    setAcceptDrops(true);

    QPalette pt = palette();
    pt.setColor(QPalette::Window, Qt::transparent);
    setPalette(pt);
    setAutoFillBackground(true);
    m_layout->setContentsMargins(UI::defaultMargins);
    m_layout->setSpacing(UI::Ins::spacing);

    // TODO DFlowLayout seems to have the smallest size, it causes extra space even though add stretch.
    // and it's ok replaced `QVBoxLayout`, it maybe a `DFlowLayout` bug.
    connect(this, &InstancePanel::tabOrderChanged, this, &InstancePanel::updateTabOrder);
}

InstancePanel::~InstancePanel()
{
}

void InstancePanel::setModel(InstanceModel *model)
{
    m_model = model;
    connect(m_model, &InstanceModel::added, this, &InstancePanel::addWidget);
    connect(m_model, &InstanceModel::moved, this, &InstancePanel::moveWidget);
    connect(m_model, &InstanceModel::removed, this, &InstancePanel::removeWidget);
    connect(m_model, &InstanceModel::replaced, this, &InstancePanel::replaceWidget);

    for (int i = 0; i < m_model->count(); i++) {
        auto instance = m_model->getInstance(i);
        addWidgetImpl(instance->handler()->id(), i);
    }
    Q_EMIT tabOrderChanged();
}

void InstancePanel::setEnabledMode(bool mode)
{
    m_mode = mode;
    if (m_mode) {
        setView();
    }
    if (m_scrollView) {
        m_scrollView->setVisible(m_mode);
    } else {
        setVisible(m_mode);
    }
    tabOrderChanged();
}

QScrollArea *InstancePanel::scrollView()
{
    if (!m_scrollView) {
        auto scrollArea = new QScrollArea();
        QPalette pt = scrollArea->palette();
        pt.setColor(QPalette::Window, Qt::transparent);
        scrollArea->setPalette(pt);
        scrollArea->setAutoFillBackground(true);
        scrollArea->setWidget(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameStyle(QFrame::NoFrame);
        scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollView = scrollArea;
    }

    return m_scrollView;
}

void InstancePanel::addWidget(const InstanceId &key, InstancePos pos)
{
    addWidgetImpl(key, pos);

    auto newItem = dynamic_cast< AnimationWidgetItem *>(m_layout->itemAt(pos));
    Q_ASSERT(newItem);

    // ensureWidgetVisible when Widget is added.
    // we need delay to execute `ensureWidgetVisible` because of Animation.
    connect(newItem, &AnimationWidgetItem::moveFinished, this, [this, newItem]() {
        scrollView()->ensureWidgetVisible(newItem->widget());
        disconnect(newItem, &AnimationWidgetItem::moveFinished, this, nullptr);
    });
    tabOrderChanged();
}

void InstancePanel::addWidgetImpl(const InstanceId &key, InstancePos pos)
{
    auto instance = m_model->getInstance(key);
    Q_ASSERT(instance);

    auto view = instance->view();
    Q_ASSERT(view);
    auto cell = createWidget(instance);
    Q_ASSERT(cell);

    if (!cell->isCustom()) {
        view->setMaximumSize(instance->handler()->size());
    }

    if (isEnabledMode()) {
        cell->setView();
    }

    if (!WidgetHandlerImpl::get(instance->handler())->isCustom()) {
        cell->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(cell, &QWidget::customContextMenuRequested, this, [this, cell]{
            onMenuRequested(cell->id());
        });
    }

    auto newItem = new AnimationWidgetItem(cell);
    m_layout->insertItem(pos, newItem);
}

void InstancePanel::moveWidget(const InstancePos &source, InstancePos target)
{
    const InstancePos index = source;
    if (index < 0 || index >= m_layout->count()) {
        qWarning(dwLog()) << "not exist the cell" << source;
        return;
    }
    auto cell = m_layout->itemAt(index)->widget();
    Q_ASSERT(cell);

    m_layout->removeWidget(cell);
    m_layout->insertItem(target, new AnimationWidgetItem(cell));
    tabOrderChanged();
}

void InstancePanel::removeWidget(const InstanceId &id)
{
    const InstancePos index = m_model->instancePosition(id);

    auto cell = m_layout->itemAt(index)->widget();
    Q_ASSERT(cell);

    m_layout->removeWidget(cell);
    cell->deleteLater();
    tabOrderChanged();
}

void InstancePanel::replaceWidget(const InstanceId &id, InstancePos /*pos*/)
{
//    removeWidget(id);

//    addWidget(id, pos);

    const InstancePos index = m_model->instancePosition(id);
    auto cell = qobject_cast<InstancePanelCell *>(m_layout->itemAt(index)->widget());
    Q_ASSERT(cell);

    auto instance = m_model->getInstance(id);
    cell->setInstance(instance);
    if (!cell->isCustom()) {
        instance->view()->setMaximumSize(instance->handler()->size());
    }

    if (isEnabledMode()) {
        cell->setView();
    }
    tabOrderChanged();
}

void InstancePanel::setView()
{
    for (int i = 0; i < m_layout->count(); i++) {
        auto cell = qobject_cast<InstancePanelCell *>(m_layout->itemAt(i)->widget());
        Q_ASSERT(cell);

        cell->setView();
    }
}

int InstancePanel::positionCell(const QPoint &pos) const
{
    for (int i = 0; i < m_layout->count(); i++) {
        auto cell = m_layout->itemAt(i)->widget();
        if (cell->geometry().contains(pos)) {
            return i;
        }
    }
    return -1;
}

static qlonglong qrectArea(const QRect &rect)
{
    return rect.width() * rect.height();
}

// In cell > Intersected cell > Empty cell
int InstancePanel::positionCell(const QPoint &pos, const QSize &size, const QPoint &hotSpot) const
{
    const auto index = positionCell(pos);
    if (index >= 0)
        return index;

    const QRect boundingRect(pos - hotSpot, size);
    // max intersected area and it's index.
    QPair<InstancePos, qlonglong> maxIntersected{-1, 0};
    for (int i = 0; i < m_layout->count(); i++) {
        auto cell = m_layout->itemAt(i)->widget();
        if (!cell->geometry().intersects(boundingRect))
            continue;

        // get the largest intersected Rect as the target position.
        const auto intersectedArea = qrectArea(cell->geometry().intersected(boundingRect));
        if (maxIntersected.second < intersectedArea)
            maxIntersected = qMakePair(i, intersectedArea);
    }
    if (maxIntersected.first >= 0)
        return maxIntersected.first;

    for (int i = 0; i < m_layout->count(); i++) {
        auto cell = m_layout->itemAt(i)->widget();
        // pos is maybe in empty area, test pre cell whether contains the pos.
        if (cell->geometry().contains(pos.x() - size.width(), pos.y())) {
            return i + 1;
        }
    }

    return -1;
}

void InstancePanel::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(MoveMimeDataFormat)) {
        if (canDragDrop(event)) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
}

void InstancePanel::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(MoveMimeDataFormat)) {
        if (canDragDrop(event)) {
            event->acceptProposedAction();
            // it seems as `autoScroll` in QAbstractItemView roughly.
            const auto &pos = event->pos();
            scrollView()->ensureVisible(pos.x(), pos.y());
        } else {
            event->ignore();
        }
    }
}

void InstancePanel::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(MoveMimeDataFormat)) {
        QByteArray itemData = event->mimeData()->data(MoveMimeDataFormat);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPoint hotSpot;
        InstanceId id;
        dataStream >> id >> hotSpot;
        if (auto instance = m_model->getInstance(id)) {
            auto target = positionCell(event->pos(), instance->handler()->size(), hotSpot);
            if (canDragDrop(target)) {
                m_model->moveInstance(id, target);
                event->accept();
                return;
            }
        }
        event->ignore();
    }
}

bool InstancePanel::canDragDrop(const InstancePos pos) const
{
    if (pos >= 0 && pos < m_layout->count()) {
        if (auto cell = qobject_cast<InstancePanelCell *>(m_layout->itemAt(pos)->widget())) {
            return !cell->isFixted();
        }
    }
    return true;
}

bool InstancePanel::canDragDrop(QDropEvent *event) const
{
    QByteArray itemData = event->mimeData()->data(MoveMimeDataFormat);
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QPoint hotSpot;
    InstanceId id;
    dataStream >> id >> hotSpot;
    if (auto instance = m_model->getInstance(id)) {
        auto target = positionCell(event->pos(), instance->handler()->size(), hotSpot);
        return canDragDrop(target);
    }
    return true;
}

void InstancePanel::onMenuRequested(const InstanceId &id)
{
    auto instance = m_model->getInstance(id);
    Q_ASSERT(instance);

    QMenu *menu = new QMenu(this);
    do {
        const auto &types = m_model->pluginTypes(instance->handler()->pluginId());
        if (types.size() < 2)
            break;

        for (auto type : types) {
            QAction *action = menu->addAction(WidgetHandlerImpl::typeString(type));
            const bool currType = instance->handler()->type() == type;
            action->setCheckable(true);
            action->setChecked(currType);

            if (!currType) {
                connect(action, &QAction::triggered, this, [this, id, type](){
                    m_model->replaceInstance(id, type);
                });
            }
        }
        menu->addSeparator();
    } while (false);

    do {
        if (!instance->enableSettings())
            break;

        QAction *action = menu->addAction(qApp->translate("InstancePanel", "settings widget"));
        connect(action, &QAction::triggered, this, [this, instance](){
            instance->settings();
        });
    } while (false);

    do {
        if (WidgetHandlerImpl::get(instance->handler())->isFixted())
            break;

        QAction *action = menu->addAction(qApp->translate("InstancePanel", "remove widget"));
        connect(action, &QAction::triggered, this, [this, id](){
            m_model->removeInstance(id);
        });
    } while (false);

    do {
        menu->addSeparator();

        QAction *action = menu->addAction(qApp->translate("InstancePanel", "about widget"));
        connect(action, &QAction::triggered, this, [this, instance](){
            auto plugin = m_manager->getPlugin(instance->handler()->pluginId());
            if (!plugin)
                return;

            InstanceAboutDialog dialog(this);

            auto logo = plugin->logo();
            if(logo.isNull()) {
                QPixmap pixmap(instance->view()->grab());
                logo = QIcon(pixmap);
            }
            dialog.setLogo(logo);
            dialog.setTitle(plugin->title());
            dialog.setDescription(plugin->aboutDescription());
            dialog.setVersion(plugin->version());

            QString contributor = plugin->contributors().join(", ");
            if (contributor.isEmpty())
                contributor = qApp->translate("InstancePanel", "anonym");

            dialog.setContributor(contributor);
            dialog.move(instance->view()->mapToGlobal(instance->view()->geometry().bottomLeft()));

            dialog.exec();

        });
    } while (false);

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void InstancePanel::updateTabOrder()
{
    if (!isEnabledMode())
        return;

    QList<QWidget *> focusList;
    for (int i = 0; i < m_layout->count(); i++) {
        auto cell = qobject_cast<InstancePanelCell *>(m_layout->itemAt(i)->widget());
        Q_ASSERT(cell);
        focusList << cell->focusWidgetList();
    }

    for (int i = 1; i < focusList.count(); ++i) {
        QWidget::setTabOrder(focusList[i - 1], focusList[i]);
    }
}
WIDGETS_FRAME_END_NAMESPACE
