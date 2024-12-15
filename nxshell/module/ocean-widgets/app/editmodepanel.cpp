// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editmodepanel.h"
#include "pluginspec.h"
#include "widgethandler.h"
#include "instancemodel.h"
#include "instanceproxy.h"
#include "widgetmanager.h"
#include "utils.h"
#include "button.h"

#include <QScrollArea>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMoveEvent>
#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include <QCoreApplication>

#include <DIconButton>
#include <DFontManager>
#include <DFontSizeManager>

DGUI_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
EditModePanelCell::EditModePanelCell(Instance *instance, QWidget *parent)
    : InstancePanelCell(instance, parent)
{
}

void EditModePanelCell::init(const QString &title)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(UI::defaultMargins);
    auto topLayout = new  QHBoxLayout();
    auto topMargin = UI::defaultMargins;
    topMargin.setLeft(UI::Edit::titleLeftMargin);
    topLayout->setContentsMargins(topMargin);
    auto topTitle = new QLabel(title);
    topTitle->setObjectName("edit-title-label");
    {
        QFont font;
        font.setBold(true);
        topTitle->setFont(DFontSizeManager::instance()->t8(font));
    }
    if (isCustom()) {
        //TODO hide or show, there is lacking of communication between plugin with frame.
    }
    topTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topLayout->addWidget(topTitle);
    topLayout->addStretch(1);

    if (!isFixted()) {
        auto action = new DIconButton(DStyle::SP_DeleteButton, this);
        action->setObjectName("delete-button");
        action->setFlat(true);
        action->setIconSize(UI::Edit::DeleteIconSize);
        topLayout->addWidget(action);
        connect(action, &DIconButton::clicked, this, [this](){
            Q_EMIT removeWidget(m_instance->handler()->id());
        });
        m_action = action;
    }
    layout->addLayout(topLayout);
}

void EditModePanelCell::setView()
{
    layout()->addWidget(view());
    // TODO it's exist the `spacing` if only hide the view, it maybe DFlowLayout's bug.
    setVisible(!isCustom());
}

QList<QWidget *> EditModePanelCell::focusWidgetList() const
{
    QList<QWidget *> results;
    if (m_action)
        results << m_action;

    return results;
}

EditModePanel::EditModePanel(WidgetManager *manager, QWidget *parent)
    : InstancePanel(manager, parent)
{
    setContentsMargins(UI::Edit::leftMargin, UI::Edit::topMargin, UI::Edit::rightMargin, UI::Edit::bottomMargin);
}

void EditModePanel::init()
{
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(UI::defaultMargins);
    m_views->setFixedWidth(width());

    layout->addWidget(m_views);

    auto button = new TransparentButton();
    button->setObjectName("complete-button");
    button->setText(qApp->translate("EditModePanel","complete"));
    button->setFixedSize(UI::Edit::CompleteSize);
    connect(button, &QPushButton::clicked, this, &EditModePanel::editCompleted);
    layout->addSpacing(UI::Ins::button2FlowPanelTopMargin);
    layout->addWidget(button, 0, Qt::AlignHCenter);
    layout->addSpacing(UI::Ins::button2FlowPanelTopMargin);

    layout->addStretch();
}

InstancePanelCell *EditModePanel::createWidget(Instance *instance)
{
    auto cell = new EditModePanelCell(instance, m_views);
    auto plugin = m_manager->getPlugin(instance->handler()->pluginId());
    cell->init(plugin->title());
    connect(cell, &EditModePanelCell::removeWidget, m_model, &InstanceModel::removeInstance);
    return cell;
}

void EditModePanel::dragEnterEvent(QDragEnterEvent *event)
{
    InstancePanel::dragEnterEvent(event);
    if (event->isAccepted())
        return;

    if (event->mimeData()->hasFormat(EditModeMimeDataFormat)) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void EditModePanel::dragMoveEvent(QDragMoveEvent *event)
{
    InstancePanel::dragMoveEvent(event);

    if (event->isAccepted())
        return;

    if (event->mimeData()->hasFormat(EditModeMimeDataFormat)) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void EditModePanel::dropEvent(QDropEvent *event)
{
    InstancePanel::dropEvent(event);
    if (event->isAccepted())
        return;

    if (event->mimeData()->hasFormat(EditModeMimeDataFormat)) {
        QByteArray itemData = event->mimeData()->data(EditModeMimeDataFormat);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPoint hotSpot;
        QString pluginId;
        int ttype = IWidget::Small;
        dataStream >> pluginId >> ttype >> hotSpot;
        IWidget::Type type = static_cast<IWidget::Type>(ttype);

        auto posIndex = positionCell(event->pos(), WidgetHandlerImpl::size(type), hotSpot);

        if (!canDragDrop(posIndex)) {
            event->ignore();
            return;
        }

        m_model->addInstance(pluginId, type, posIndex);

        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}
WIDGETS_FRAME_END_NAMESPACE
