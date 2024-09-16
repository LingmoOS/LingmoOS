// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgetstore.h"
#include "pluginspec.h"
#include "widgethandler.h"
#include "widgetmanager.h"
#include "instanceproxy.h"
#include "utils.h"

#include <QScrollArea>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMoveEvent>
#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include <QStackedLayout>

#include <DIconButton>
#include <DAnchors>
#include <DFontSizeManager>
#include <DButtonBox>
#include <DClipEffectWidget>

WIDGETS_FRAME_BEGIN_NAMESPACE
WidgetStore::WidgetStore(WidgetManager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_views(new QWidget(this))
    , m_layout(new QVBoxLayout(m_views))
{
    m_layout->setSpacing(UI::Store::spacing);

    QPalette pt = palette();
    pt.setColor(QPalette::Window, Qt::transparent);
    setPalette(pt);
    setAutoFillBackground(true);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(UI::defaultMargins);
    layout->setSpacing(UI::Store::spacing);

    layout->addWidget(m_views);
    layout->addStretch();
}

void WidgetStore::init()
{
    m_views->setFixedWidth(width());
    m_views->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    load();
}

QScrollArea *WidgetStore::scrollView()
{
    if (!m_scrollView) {
        auto scrollArea = new QScrollArea();

        scrollArea->setObjectName(QString("WidgetStore"));
        scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QPalette pt = scrollArea->palette();
        pt.setColor(QPalette::Window, Qt::transparent);
        scrollArea->setPalette(pt);
        scrollArea->setWidget(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setAutoFillBackground(true);
        scrollArea->setFrameStyle(QFrame::NoFrame);
        m_scrollView = scrollArea;
    }

    return m_scrollView;
}

void WidgetStore::addPlugin(const PluginId &pluginId)
{
    auto pluginCell = addPluginCell(pluginId);
    m_layout->addWidget(pluginCell);
    m_pluginCells.insert(pluginId, pluginCell);
}

PluginCell *WidgetStore::addPluginCell(const PluginId &pluginId)
{
    auto plugin = m_manager->getPlugin(pluginId);
    auto pluginCell = new PluginCell(this);
    for (auto instance: m_manager->createWidgetStoreInstances(pluginId)) {
        auto view = instance->view();
        Q_ASSERT(view);
        auto cell = new WidgetStoreCell(instance->handler(), this);
        connect(cell, &WidgetStoreCell::addWidget, this, &WidgetStore::addWidget);
        cell->setView(view);
        pluginCell->addCell(cell);
    }
    pluginCell->setTitle(plugin->title());
    pluginCell->setDescription(plugin->description());
    const int selectedCell = 0;
    pluginCell->setChecked(selectedCell);
    return pluginCell;
}

void WidgetStore::removePlugin(const PluginId &pluginId)
{
    for (auto iter = m_pluginCells.begin(); iter != m_pluginCells.end();) {
        if (iter.key() == pluginId) {
            m_layout->removeWidget(iter.value());
            iter.value()->deleteLater();
            iter = m_pluginCells.erase(iter);
        } else {
            iter++;
        }
    }
}

void WidgetStore::load()
{
    const auto plugins = m_manager->plugins(IWidgetPlugin::Normal);
    for (auto plugin : plugins) {
        const auto pluginId = plugin->id();
        addPlugin(pluginId);
    }
    m_layout->addStretch();
}

PluginCell::PluginCell(QWidget *parent)
    : DBlurEffectWidget (parent)
{
    setBlurRectXRadius(UI::Store::pluginCellRadius);
    setBlurRectYRadius(UI::Store::pluginCellRadius);
    setMaskColor(UI::Store::cellBackgroundColor);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(UI::Store::leftMargin, UI::Store::topMargin, UI::Store::rightMargin, UI::Store::bottomMargin);
    layout->setAlignment(Qt::AlignHCenter);
    DFontSizeManager *fontManager =  DFontSizeManager::instance();
    m_title = new QLabel();
    m_title->setObjectName("store-title-label");
    fontManager->bind(m_title, DFontSizeManager::T5, QFont::Bold);
    m_title->setAlignment(Qt::AlignHCenter);
    layout->addWidget(m_title);
    m_description = new QLabel();
    m_description->setWordWrap(true);
    m_title->setObjectName("store-description-label");
    m_description->setAlignment(Qt::AlignHCenter);
    fontManager->bind(m_description, DFontSizeManager::T8);

    QWidget *views = new QWidget();
    auto viewsLayout = new QVBoxLayout(views);
    auto cellViews = new QWidget(views);

    m_layout = new QStackedLayout(cellViews);
    m_typeBox = new DButtonBox;
    m_typeBox->setFixedHeight(UI::Store::TypeBoxSize.height());
    viewsLayout->addWidget(m_typeBox, 0, Qt::AlignHCenter);
    viewsLayout->addWidget(cellViews, 0, Qt::AlignHCenter);
    connect(m_typeBox, &DButtonBox::buttonClicked, this, [this](QAbstractButton *btn) {
        const auto index = m_typeBox->id(btn);
        Q_ASSERT(index >= 0 && index < m_layout->count());

        m_layout->setCurrentIndex(index);
    });

    layout->addWidget(views, 0, Qt::AlignHCenter);
    layout->addWidget(m_description);
    layout->addStretch();
}

void PluginCell::setTitle(const QString &text)
{
    m_title->setText(text);
}

void PluginCell::setDescription(const QString &text)
{
    m_description->setText(text);
}

void PluginCell::addCell(WidgetStoreCell *cell)
{
    m_cells << cell;
    const auto text= WidgetHandlerImpl::get(cell->m_handler)->typeString();
    auto btn = new DButtonBoxButton(text, cell);
    btn->setText(text);
    QWidget::setTabOrder(btn, cell->action());
    btn->installEventFilter(this);
    QList<DButtonBoxButton *> tmpBtnList;
    for (auto item : m_typeBox->buttonList()) {
        tmpBtnList << qobject_cast<DButtonBoxButton *>(item);
    }
    tmpBtnList << btn;
    m_typeBox->setButtonList(tmpBtnList, true);
    // m_typeBox and m_cells are mapped by index.
    for (int i = 0; i < m_cells.count(); ++i) {
        m_typeBox->setId(tmpBtnList[i], i);
    }
    // put in a layout because of different cell's size.
    auto cellView = new QWidget();
    auto cellLayout = new QVBoxLayout(cellView);
    cellLayout->addWidget(cell, 0, Qt::AlignHCenter);
    m_layout->addWidget(cellView);
}

void PluginCell::setChecked(const int index, const bool checked)
{
    if(index < 0 || index >= m_typeBox->buttonList().count())
        return;

    m_typeBox->buttonList()[index]->click();
}

bool PluginCell::eventFilter(QObject *watched, QEvent *event)
{
    if (auto btn = qobject_cast<DButtonBoxButton *>(watched)) {
        switch (event->type()) {
        case QEvent::FocusIn: {
            const int index = m_typeBox->id(btn);
            Q_ASSERT(index >= 0 && index < m_cells.count());
            Q_EMIT m_cells[index]->enterChanged(true);
            break;
        }
        default:
            break;
        }
    }
    return DBlurEffectWidget::eventFilter(watched, event);
}

WidgetStoreCell::WidgetStoreCell(WidgetHandler *handler, QWidget *parent)
    : DragDropWidget(parent)
    , m_handler(handler)
{
    setFocusPolicy(Qt::NoFocus);
}

void WidgetStoreCell::setView(QWidget *view)
{
    m_view = view;
    m_view->setParent(this);
    m_view->setVisible(false);
    m_view->resize(m_handler->size());
    const auto &targetSize = WidgetHandlerImpl::size(m_handler->type(), false);

    m_viewPlaceholder = new PlaceholderWidget(m_view, this);
    m_viewPlaceholder->resize(targetSize);

    auto action = new DIconButton(DStyle::SP_AddButton);
    action->setObjectName("add-button");
    action->setParent(this);
    action->setIconSize(UI::Store::AddIconSize);
    action->setFlat(true);
    action->setVisible(false);
    connect(this, &WidgetStoreCell::enterChanged, action, &QWidget::setVisible);
    connect(action, &DIconButton::clicked, this, [this](){
        Q_EMIT addWidget(m_handler->pluginId(), m_handler->type());
    });
    m_action = action;
    m_action->installEventFilter(this);

    auto cellAnchors = new DAnchors<WidgetStoreCell>(this);
    auto actionAnchors = new DAnchors<DIconButton>(action);
    auto viewAnchors = new DAnchors<QWidget>(m_viewPlaceholder);
    actionAnchors->setTop(cellAnchors->top());
    actionAnchors->setRight(cellAnchors->right());
    viewAnchors->setBottom(cellAnchors->bottom());
    viewAnchors->setHorizontalCenter(cellAnchors->horizontalCenter());
    setFixedSize(targetSize + (UI::Store::AddIconSize / 2));
}

QWidget *WidgetStoreCell::action() const
{
    return m_action;
}

void WidgetStoreCell::startDrag(const QPoint &pos)
{
    QWidget *child = m_viewPlaceholder;
    if (!child)
        return;

    m_startDrag = mapToGlobal(child->pos());
    qDebug() << "startDrag:" << m_startDrag;

    QPoint hotSpot = pos - child->pos();
    QMimeData *mimeData = new QMimeData;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << m_handler->pluginId() << m_handler->type() << hotSpot;
    mimeData->setData(EditModeMimeDataFormat, itemData);

    QPixmap pixmap(child->grab());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(hotSpot);

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction, Qt::CopyAction);
    if (dropAction == Qt::IgnoreAction) {
        gobackDrag(pixmap, QCursor::pos());
    }
}

void WidgetStoreCell::enterEvent(QEvent *event)
{
    Q_EMIT enterChanged(true);
    QWidget::enterEvent(event);
}

void WidgetStoreCell::leaveEvent(QEvent *event)
{
    Q_EMIT enterChanged(false);
    QWidget::leaveEvent(event);
}

bool WidgetStoreCell::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_action) {
        switch (event->type()) {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            Q_EMIT enterChanged(event->type() == QEvent::FocusIn);
            break;
        default:
            break;
        }
    }
    return DragDropWidget::eventFilter(watched, event);
}

void WidgetStoreCell::timerEvent(QTimerEvent *event)
{
    do {
        if (event->timerId() != m_viewPlaceholderFresher.timerId())
            break;

        updateViewPlaceholder();
    } while (false);

    return DragDropWidget::timerEvent(event);
}

void WidgetStoreCell::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    updateViewPlaceholder();

    m_viewPlaceholderFresher.start(UI::Store::viewPlaceholderFresherTime, this);
}

void WidgetStoreCell::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    m_viewPlaceholderFresher.stop();
}

void WidgetStoreCell::updateViewPlaceholder()
{
    if (!m_viewPlaceholder)
        return;

    update();
}
WIDGETS_FRAME_END_NAMESPACE
