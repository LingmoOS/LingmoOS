// SPDX-FileCopyrightText: 2016 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginitemdelegate.h"

#include <DStyleOption>
#include <DPalette>
#include <DToolTip>

#include <QDebug>
#include <QPainter>
#include <QVariant>
#include <QPalette>
#include <QIcon>
#include <QListView>

PluginItemDelegate::PluginItemDelegate(QAbstractItemView *parent)
    : QStyledItemDelegate(parent)
    , m_view(parent)
    , m_widgetHeight(36)
    , m_bottomSpacing(10)
    , m_beginningItemStyle(QStyleOptionViewItem::OnlyOne)
    , m_middleItemStyle(QStyleOptionViewItem::OnlyOne)
    , m_endItemStyle(QStyleOptionViewItem::OnlyOne)
{
}

void PluginItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DStyleOptionBackgroundGroup boption;
    boption.init(m_view);
    boption.QStyleOption::operator=(option);
    boption.state.setFlag(QStyle::State_Active);

    ItemSpacing itemSpacing = getItemSpacing(index);
    if (itemSpacing.top != 0 || itemSpacing.bottom != 0) {
        boption.rect.adjust(0, itemSpacing.top, 0, -itemSpacing.bottom);
    }

    QColor bgColor, textColor;
    if (m_view->currentIndex() == index) {
        if (option.rect.height() > 100) {
            textColor = boption.dpalette.brightText().color();
            bgColor = boption.dpalette.brightText().color();
            bgColor.setAlphaF(0.1);
        } else {
            textColor = boption.dpalette.highlightedText().color();
            bgColor = boption.dpalette.highlight().color();
        }
    } else {
        textColor = boption.dpalette.brightText().color();
        bgColor = boption.dpalette.brightText().color();
        bgColor.setAlphaF(0.05);
    }

    if (textColor.isValid()) {
        boption.palette.setBrush(QPalette::BrightText, textColor);
        boption.palette.setBrush(QPalette::Text, textColor);
        boption.palette.setBrush(QPalette::Highlight, textColor);
        QWidget *w = m_view->indexWidget(index);
        if (w) {
            w->setPalette(boption.palette);
        }
    }

    if (bgColor.isValid()) {
        boption.dpalette.setBrush(DPalette::ItemBackground, bgColor);
        boption.directions = Qt::Vertical;
        boption.position = DStyleOptionBackgroundGroup::ItemBackgroundPosition(itemSpacing.viewItemPosition);
        m_view->style()->drawPrimitive(static_cast<QStyle::PrimitiveElement>(DStyle::PE_ItemBackground), &boption, painter, option.widget);
    }
}

QWidget *PluginItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (!index.isValid()) {
        return nullptr;
    }

    QStandardItem* item = qobject_cast<QStandardItemModel*>(m_view->model())->itemFromIndex(index);
    PluginItemWidget *widget = nullptr;
    if (item) {
        widget = new PluginItemWidget(dynamic_cast<PluginItem *>(item), parent);
    }
    return widget;
}

QSize PluginItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    ItemSpacing itemSpacing = getItemSpacing(index);
    return QSize(m_view->sizeHint().width(), itemSpacing.top + itemSpacing.height + itemSpacing.bottom);
}

void PluginItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!editor)
        return;

    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    ItemSpacing itemSpacing = getItemSpacing(index);
    QRect geom = editor->geometry();
    // 设置 x 为 0，在 editor 中处理布局
    geom.adjust(-geom.x(), itemSpacing.top, 0, -itemSpacing.bottom);
    geom.setWidth(m_view->width());
    editor->setGeometry(geom);
}

void PluginItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &) const
{
    delete editor;
}

ItemSpacing PluginItemDelegate::getItemSpacing(const QModelIndex &index) const
{
    ItemSpacing spacing;
    spacing.top = 0;
    spacing.height = m_widgetHeight;
    int rowCount = m_view->model()->rowCount();
    spacing.bottom = index.row() == (rowCount - 1) ? 0 : m_bottomSpacing;
    // 显示样式
    if (rowCount == 1) {
        spacing.viewItemPosition = QStyleOptionViewItem::OnlyOne;
    } else if (index.row() == 0) {
        spacing.viewItemPosition = m_beginningItemStyle;
    } else if (index.row() == rowCount - 1) {
        spacing.viewItemPosition = m_endItemStyle;
    } else {
        spacing.viewItemPosition = m_middleItemStyle;
    }

    return spacing;
}

PluginItem::PluginItem(const QIcon &icon, const QString &name, const PluginItemState state)
    : QStandardItem()
    , m_icon(icon)
    , m_name(name)
    , m_state(state)
{
}

PluginItem::PluginItem()
    : QStandardItem()
    , m_icon(QIcon())
    , m_name(QString())
    , m_state(PluginItemState::NoState)
{
}

PluginItem::~PluginItem()
{
}

void PluginItem::updateIcon(const QIcon &icon)
{
    if (m_icon.cacheKey() != icon.cacheKey()) {
        m_icon = icon;
        Q_EMIT iconChanged(m_icon);
    }
}

void PluginItem::updateName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged(m_name);
    }
}

void PluginItem::updateState(const PluginItemState state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

PluginItemWidget::PluginItemWidget(PluginItem *item, QWidget *parent)
    : QWidget(parent)
    , m_item(item)
    , m_mainLayout(new QHBoxLayout(this))
    , m_iconBtn(nullptr)
    , m_nameLabel(nullptr)
    , m_connBtn(nullptr)
    , m_spinner(nullptr)
    , m_rightIconSpacerItem(new QSpacerItem(0, 0))
{
    if (!m_item) {
        QLabel *err = new QLabel(this);
        err->setText("Unknown Item");
        m_mainLayout->addWidget(err, 1);
        return;
    }

    setAccessibleName(item->name());

    m_iconBtn = new CommonIconButton(this);
    m_iconBtn->setFixedSize(16, 16);
    m_iconBtn->setIcon(item->icon());

    m_nameLabel = new DLabel(this);
    m_nameLabel->setText(item->name());
    DToolTip::setToolTipShowMode(m_nameLabel, DToolTip::ShowWhenElided);
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_nameLabel->setElideMode(Qt::ElideRight);

    m_connBtn = new CommonIconButton(this);
    m_connBtn->setIcon(QIcon::fromTheme("plugin_item_select"));
    m_connBtn->setHoverIcon(QIcon::fromTheme("plugin_item_disconnect"));
    m_connBtn->setFixedSize(16, 16);
    m_connBtn->setClickable(true);
    m_connBtn->hide();

    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(16, 16);
    m_spinner->hide();
    m_spinner->stop();

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_iconBtn, 0);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_nameLabel, 1);
    m_mainLayout->addStretch();
    m_mainLayout->addSpacerItem(m_rightIconSpacerItem);
    m_mainLayout->addWidget(m_connBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_mainLayout->addWidget(m_spinner, 0, Qt::AlignRight | Qt::AlignVCenter);
    updateState(item->state());

    if (parent)
        setForegroundRole(parent->foregroundRole());

    connect(m_item, &PluginItem::iconChanged, this, &PluginItemWidget::updateIcon);
    connect(m_item, &PluginItem::nameChanged, this, &PluginItemWidget::updateName);
    connect(m_item, &PluginItem::stateChanged, this, &PluginItemWidget::updateState);

    connect(m_connBtn, &CommonIconButton::clicked, m_item, &PluginItem::connectBtnClicked);
}

PluginItemWidget::~PluginItemWidget()
{
}

void PluginItemWidget::updateIcon(const QIcon &icon)
{
    m_iconBtn->setIcon(icon);
}

void PluginItemWidget::updateName(const QString &name)
{
    m_nameLabel->setText(name);
}

void PluginItemWidget::updateState(const PluginItemState state)
{
    m_rightIconSpacerItem->changeSize(10, 0);
    switch (state) {
    case PluginItemState::NoState:
        m_connBtn->setVisible(false);
        m_spinner->stop();
        m_spinner->setVisible(false);
        m_rightIconSpacerItem->changeSize(0, 0);
        break;
    case PluginItemState::Connecting:
        m_connBtn->setVisible(false);
        m_spinner->start();
        m_spinner->setVisible(true);
        break;
    case PluginItemState::Connected:
        m_connBtn->setVisible(true);
        m_connBtn->setClickable(true);
        m_connBtn->setHoverEnable(true);
        m_spinner->stop();
        m_spinner->setVisible(false);
        break;
    case PluginItemState::ConnectedOnlyPrompt:
        m_connBtn->setVisible(true);
        m_connBtn->setClickable(false);
        m_connBtn->setHoverEnable(false);
        m_spinner->stop();
        m_spinner->setVisible(false);
        break;
    default:
        m_connBtn->setVisible(false);
        m_spinner->stop();
        m_spinner->setVisible(false);
        m_rightIconSpacerItem->changeSize(0, 0);
        break;
    }

    m_mainLayout->invalidate();
}

bool PluginItemWidget::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::PaletteChange: {
        QLayout *layout = this->layout();
        for (int i = 0; i < layout->count(); i++) {
            QWidget *widget = layout->itemAt(i)->widget();
            if (widget) {
                widget->setPalette(palette());
            }
        }
    } break;
    default:
        break;
    }
    return QWidget::event(e);
}
