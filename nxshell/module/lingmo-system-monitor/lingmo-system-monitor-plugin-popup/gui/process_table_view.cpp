// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_table_view.h"

#include "application.h"

#include "settings.h"
#include "gui/ui_common.h"
#include "common/perf.h"
#include "common/common.h"
#include "common/error_context.h"
#include "model/process_sort_filter_proxy_model.h"
#include "model/process_table_model.h"
#include "process/process_db.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DDialog>
#include <DErrorMessage>
#include <DFontSizeManager>
#include <DFrame>
#include <DLabel>
#include <DMenu>
#include <DPalette>
#include <DSlider>
#include <DTipLabel>
#include <DTitlebar>
#include <DToolTip>
#include <DWidget>
#include <DHeaderView>

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QKeyEvent>
#include <QShortcut>

// process table view backup setting key
const QByteArray header_version = "_1.0.0";

ProcessTableView::ProcessTableView(DWidget *parent)
    : BaseTableView(parent)
{
    // install event filter for table view to handle key events
    installEventFilter(this);

    // model & sort filter proxy model instance
    m_model = new ProcessTableModel(this);
    m_proxyModel = new ProcessSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    // setModel must be called before calling loadSettings();
    setModel(m_proxyModel);

    // initialize ui components & connections
    initUI();
    initConnections();

    // adjust search result tip label text color dynamically on theme type change
    onThemeTypeChanged();
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &ProcessTableView::onThemeTypeChanged);

    this->setAttribute(Qt::WA_TranslucentBackground, true);

    setHeaderHidden(true);
}

// destructor
ProcessTableView::~ProcessTableView()
{

}

void ProcessTableView::onThemeTypeChanged()
{
    auto palette = DApplicationHelper::instance()->applicationPalette();
    palette.setColor(DPalette::Text, palette.color(DPalette::PlaceholderText));
    m_notFoundLabel->setPalette(palette);

    palette.setColor(DPalette::Button, palette.color(DPalette::Base));
    header()->setPalette(palette);

    this->setAttribute(Qt::WA_TranslucentBackground, true);

}

// event filter
bool ProcessTableView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        //屏蔽鼠标按下的事件
        return true;
    }

    return BaseTableView::eventFilter(obj, event);
}

// filter process table based on searched text
void ProcessTableView::search(const QString &text)
{
    m_proxyModel->setSortFilterString(text);
    // adjust search result tip label's visibility & position if needed
    adjustInfoLabelVisibility();
}

// switch process table view display mode
void ProcessTableView::switchDisplayMode(FilterType type)
{
    m_proxyModel->setFilterType(type);
}

// initialize ui components
void ProcessTableView::initUI()
{
    setAccessibleName("ProcessTableView");

    // search result not found tip label instance
    m_notFoundLabel = new DLabel(DApplication::translate("Common.Search", "No search results"), this);
    DFontSizeManager::instance()->bind(m_notFoundLabel, DFontSizeManager::T1);
    // change text color
    auto palette = DApplicationHelper::instance()->palette(m_notFoundLabel);
    QColor labelColor = palette.color(DPalette::PlaceholderText);
    palette.setColor(DPalette::Text, labelColor);
    m_notFoundLabel->setPalette(palette);
    m_notFoundLabel->setVisible(false);

    // header view options
    // header section movable
    header()->setSectionsMovable(true);
    // header section clickable
    header()->setSectionsClickable(true);
    // header section resizable
    header()->setSectionResizeMode(DHeaderView::Interactive);
    // stretch last header section
    header()->setStretchLastSection(true);
    // show sort indicator on sort column
    header()->setSortIndicatorShown(true);
    // header section default alignment
    header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // header section context menu policy
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setAttribute(Qt::WA_TranslucentBackground, true);


    // table options
    setSortingEnabled(true);
    // no allowed selection
    setSelectionMode(QAbstractItemView::NoSelection);
    // can only select whole row
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // table view context menu policy
    setContextMenuPolicy(Qt::CustomContextMenu);

    // proc name
    setColumnWidth(ProcessTableModel::kProcessNameColumn, 200);
    setColumnHidden(ProcessTableModel::kProcessNameColumn, false);

    // cpu
    setColumnWidth(ProcessTableModel::kProcessCPUColumn, 80);
    setColumnHidden(ProcessTableModel::kProcessCPUColumn, false);

    // cpu
    setColumnWidth(ProcessTableModel::kProcessPIDColumn, 0);
    setColumnHidden(ProcessTableModel::kProcessPIDColumn, true);

    //sort
    sortByColumn(ProcessTableModel::kProcessCPUColumn, Qt::DescendingOrder);
}

// initialize connections
void ProcessTableView::initConnections()
{
    // on each model update, we restore settings, adjust search result tip lable's visibility & positon, select the same process item before update if any
    connect(m_model, &ProcessTableModel::modelUpdated, this, [&]() {
        adjustInfoLabelVisibility();
        if (m_selectedPID.isValid()) {
            for (int i = 0; i < m_proxyModel->rowCount(); i++) {
                if (m_proxyModel->data(m_proxyModel->index(i, ProcessTableModel::kProcessPIDColumn),
                                       Qt::UserRole) == m_selectedPID)
                    this->setCurrentIndex(m_proxyModel->index(i, 0));
            }
        }
    });

}

// resize event handler
void ProcessTableView::resizeEvent(QResizeEvent *event)
{
    // adjust search result tip label's visibility & position when resizing
    adjustInfoLabelVisibility();

    DTreeView::resizeEvent(event);
}

// show event handler
void ProcessTableView::showEvent(QShowEvent *)
{
    // hide search result not found on initial show
    if (m_notFoundLabel) {
        m_notFoundLabel->hide();
    }
}

// backup current selected item's pid when selection changed
void ProcessTableView::selectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected)
{
    // if no selection, do nothing
    if (selected.size() <= 0) {
        return;
    }

    m_selectedPID = selected.indexes().value(ProcessTableModel::kProcessPIDColumn).data();

    DTreeView::selectionChanged(selected, deselected);
}

// return hinted size for specified column, so column can be resized to a prefered width when double clicked
int ProcessTableView::sizeHintForColumn(int column) const
{
    int margin = 10;
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    DTreeView::sizeHintForColumn(column) + margin * 2);
}

// adjust search result tip label's visibility & position
void ProcessTableView::adjustInfoLabelVisibility()
{
    setUpdatesEnabled(false);

    // move label to center of the process table view
    if (m_notFoundLabel->isVisible())
        m_notFoundLabel->move(rect().center() - m_notFoundLabel->rect().center());
    setUpdatesEnabled(true);
}
