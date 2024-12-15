// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DLabel>

#include "orderwidget.h"
#include "boxlayoutwidget.h"
#include "drawboard.h"
#include "pageview.h"
#include "pagescene.h"

const int BUTTON_MARGIN = 3;
const int MAX_HEIGHT = 36;
OrderWidget::OrderWidget(DrawBoard *drawBoard, QWidget *parent): AttributeWgt(EOrderProperty, parent), m_drawBoard(drawBoard)
{
    initUi();
    initConnect();
}

void OrderWidget::initUi()
{
    DLabel *l = new DLabel(tr("Order"), this);
    m_oneLayerUp = createIconButton("up_on_layer", tr("Raise layer"), "Raise_layer");
    m_oneLayerDown = createIconButton("down_one_layer", tr("Lower layer"), "Lower_layer");
    m_bringToFront = createIconButton("top_layer", tr("Layer to Top"), "Layer_to_Top");
    m_sendToback = createIconButton("botton_layer", tr("Layer to Bottom"), "Layer_to_Bottom");

    m_LeftAlign = createIconButton("left_alignment", tr("Align left"), "Align_left");
    m_HCenterAlign = createIconButton("horizontal_center_alignment", tr("Align center horizontally"), "Align_middle");
    m_RightAlign = createIconButton("right_alignment", tr("Align right"), "Align_right");
    m_TopAlign = createIconButton("top_alignment", tr("Align top"), "Align_top");
    m_VCenterAlign = createIconButton("center_alignmetn", tr("Align center vertically"), "Align_center");
    m_BottomAlign = createIconButton("bottom_alignment", tr("Align bottom"), "Align_bottom");

    m_HEqulSpaceAlign = createIconButton("vertical_ equally_ spaced_alignment", tr("Distribute horizontally"), "Distribute_horizontally");
    m_VEqulSpaceAlign = createIconButton("horizontal_equally_spaced_alignment", tr("Distribute vertically"), "Distribute_vertically");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(l);

    QHBoxLayout *hLayer = new QHBoxLayout;
    BoxLayoutWidget *layerWidget = new BoxLayoutWidget(this);
    layerWidget->setMaximumHeight(MAX_HEIGHT);
    layerWidget->setMargins(BUTTON_MARGIN);
    layerWidget->addWidget(m_oneLayerUp);
    layerWidget->addWidget(m_oneLayerDown);
    layerWidget->addWidget(m_bringToFront);
    layerWidget->addWidget(m_sendToback);
    BoxLayoutWidget *orderWidget = new BoxLayoutWidget(this);
    orderWidget->setMargins(BUTTON_MARGIN);
    orderWidget->setMaximumHeight(MAX_HEIGHT);
    orderWidget->addWidget(m_HEqulSpaceAlign);
    orderWidget->addWidget(m_VEqulSpaceAlign);
    hLayer->addWidget(layerWidget, 2);
    hLayer->addWidget(orderWidget, 1);
    BoxLayoutWidget *aligWidget = new BoxLayoutWidget(this);
    aligWidget->setMargins(BUTTON_MARGIN);
    aligWidget->setMaximumHeight(MAX_HEIGHT);
    aligWidget->addWidget(m_LeftAlign);
    aligWidget->addWidget(m_HCenterAlign);
    aligWidget->addWidget(m_RightAlign);
    aligWidget->addWidget(m_TopAlign);
    aligWidget->addWidget(m_VCenterAlign);
    aligWidget->addWidget(m_BottomAlign);

    mainLayout->addLayout(hLayer);
    mainLayout->addWidget(aligWidget);
    setLayout(mainLayout);

    addHSeparateLine();
}

void OrderWidget::initConnect()
{
    connect(m_drawBoard, qOverload<Page *>(&DrawBoard::currentPageChanged), this, [ = ](Page * p) {
        if (nullptr != p) {
            if (nullptr != m_currentScene) {
                disconnect(m_currentScene);
            }

            m_currentScene = p->scene();
            connect(m_currentScene, &PageScene::selectionChanged, this, [ = ] {
                setEnabled(m_currentScene->selectedItemCount() > 0);
                updateButtonsStatus();

            });
            updateButtonsStatus();
        }
    });

    connect(m_oneLayerUp, &DToolButton::clicked, this, [ = ] {
        auto scene = m_drawBoard->currentPage()->scene();
        if (scene == nullptr)
        {
            return;
        }
        scene->movePageItemsZValue(scene->selectedPageItems(), UpItemZ, 1, true);
        updateLayerButtonStatus();
    });

    connect(m_oneLayerDown, &DToolButton::clicked, this, [ = ] {
        auto scene = m_drawBoard->currentPage()->scene();
        if (scene == nullptr)
        {
            return;
        }
        scene->movePageItemsZValue(scene->selectedPageItems(), DownItemZ, 1, true);
        updateLayerButtonStatus();
    });

    connect(m_bringToFront, &DToolButton::clicked, this, [ = ] {
        auto scene = m_drawBoard->currentPage()->scene();
        if (scene == nullptr)
        {
            return;
        }
        scene->movePageItemsZValue(scene->selectedPageItems(), UpItemZ, -1, true);
        updateLayerButtonStatus();
    });

    connect(m_sendToback, &DToolButton::clicked, this, [ = ] {
        auto scene = m_drawBoard->currentPage()->scene();
        if (scene == nullptr)
        {
            return;
        }
        scene->movePageItemsZValue(scene->selectedPageItems(), DownItemZ, -1, true);
        updateLayerButtonStatus();
    });

    connect(m_LeftAlign, &DToolButton::clicked, this, [ = ] {
        doAlignment(Qt::AlignLeft);
    });

    connect(m_HCenterAlign, &DToolButton::clicked, this, [ = ] {
        doAlignment(Qt::AlignHCenter);
    });

    connect(m_RightAlign, &DToolButton::clicked, this, [ = ] {
        doAlignment(Qt::AlignRight);
    });

    connect(m_TopAlign, &DToolButton::clicked, this, [ = ] {
        doAlignment(Qt::AlignTop);
    });
    connect(m_VCenterAlign, &DToolButton::clicked, this, [ = ] {
        doAlignment(Qt::AlignVCenter);
    });

    connect(m_BottomAlign, &DToolButton::clicked, this, [ = ] {
        doAlignment(Qt::AlignBottom);
    });

    connect(m_HEqulSpaceAlign, &DToolButton::clicked, this, [ = ] {
        auto view = m_drawBoard->currentPage()->view();
        if (view == nullptr)
        {
            return;
        }
        view->itemsEqulSpaceAlignment(true);
        updateAlignButtonStatus();
    });

    connect(m_VEqulSpaceAlign, &DToolButton::clicked, this, [ = ] {
        auto view = m_drawBoard->currentPage()->view();
        if (view == nullptr)
        {
            return;
        }
        view->itemsEqulSpaceAlignment(false);
        updateAlignButtonStatus();
    });
}

DToolButton *OrderWidget::createIconButton(const QString &icon, const QString &toolTip, const QString &accesibleName)
{
    DToolButton *btn = new DToolButton(nullptr);
    btn->setIcon(QIcon::fromTheme(icon));
    btn->setToolTip(toolTip);
    setWgtAccesibleName(btn, accesibleName);
    btn->setMinimumSize(38, 30);
    btn->setIconSize(QSize(16, 16));
    btn->setFocusPolicy(Qt::NoFocus);
    return btn;
}

void OrderWidget::doAlignment(Qt::AlignmentFlag align)
{
    auto view = m_drawBoard->currentPage()->view();
    if (view == nullptr) {
        return;
    }
    view->alignmentSelectItmes(align);
    updateAlignButtonStatus();
}

void OrderWidget::updateAlignButtonStatus()
{
    int selectedCount = m_currentScene->selectedItemCount();
    bool acticonvistual = (selectedCount > 0);

    m_LeftAlign->setEnabled(acticonvistual);
    m_HCenterAlign->setEnabled(acticonvistual);
    m_RightAlign->setEnabled(acticonvistual);
    m_TopAlign->setEnabled(acticonvistual);
    m_VCenterAlign->setEnabled(acticonvistual);
    m_BottomAlign->setEnabled(acticonvistual);

    m_HEqulSpaceAlign->setEnabled(selectedCount >= 3);
    m_VEqulSpaceAlign->setEnabled(selectedCount >= 3);
}

void OrderWidget::updateLayerButtonStatus()
{
    auto view = m_drawBoard->currentPage()->view();
    if (view == nullptr) {
        return;
    }
    auto scene = view->pageScene();

    bool layerUp = PageScene::isItemsZMovable(scene->selectedPageItems(), UpItemZ);
    m_oneLayerUp->setEnabled(layerUp);
    m_bringToFront->setEnabled(layerUp);

    bool layerDown = PageScene::isItemsZMovable(scene->selectedPageItems(), DownItemZ);
    m_oneLayerDown->setEnabled(layerDown);
    m_sendToback->setEnabled(layerDown);
}

void OrderWidget::updateButtonsStatus()
{
    updateAlignButtonStatus();
    updateLayerButtonStatus();
}
