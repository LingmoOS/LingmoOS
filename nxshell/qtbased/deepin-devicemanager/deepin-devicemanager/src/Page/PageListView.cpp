// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageListView.h"
#include "DeviceListView.h"
#include "MacroDefinition.h"

// Dtk头文件
#include <DApplicationHelper>

// Qt库文件
#include <QHBoxLayout>
#include <QLoggingCategory>

PageListView::PageListView(DWidget *parent)
    : DWidget(parent)
    , mp_ListView(new DeviceListView(this))
    , mp_Refresh(new QAction(tr("Refresh"), this))
    , mp_Export(new QAction(tr("Export"), this))
    , mp_Menu(new QMenu(this))
    , m_CurType(tr("Overview"))
{
    //初始化界面
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(mp_ListView);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    this->setFixedWidth(152);
    // 初始化右键菜单
    mp_ListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mp_ListView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageListView::refreshActionTrigger);
    connect(mp_Export, &QAction::triggered, this, &PageListView::exportActionTrigger);

    // 连接item点击事件
    connect(mp_ListView, &DListView::clicked, this, &PageListView::slotListViewItemClicked);
}

PageListView::~PageListView()
{

}

void PageListView::updateListItems(const QList<QPair<QString, QString> > &lst)
{
    if (! mp_ListView) {
        return;
    }

    // 更新之前先清理
    mp_ListView->clearItem();

    // 更新 list
    foreach (auto it, lst) {
        mp_ListView->addItem(it.first, it.second);
    }

    // 更新之后恢复之前显示的设备
    mp_ListView->setCurItem(m_CurType);
    emit itemClicked(m_CurType);
}

QString PageListView::currentIndex()
{
    // 获取当前Index的UserRole
    return mp_ListView->currentIndex().data(Qt::UserRole).toString();
}

QString PageListView::currentType()
{
    // 获取当前设备类型
    return m_CurType;
}

void PageListView::clear()
{
    if (!mp_ListView) {
        return;
    }

    // 更新之前先清理
    mp_ListView->clearItem();
}

void PageListView::setCurType(QString type)
{
    m_CurType = type;
    mp_ListView->setCurItem(m_CurType);
}

void PageListView::paintEvent(QPaintEvent *event)
{
    // 让背景色适合主题颜色
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    pa.setBrush(DPalette::Background, pa.brush(DPalette::Base));
    DApplicationHelper::instance()->setPalette(this, pa);
    return DWidget::paintEvent(event);
}

void PageListView::slotShowMenu(const QPoint &point)
{
    // 右键菜单
    mp_Menu->clear();

    if (m_CurType == tr("Driver Install") || m_CurType == tr("Driver Backup") || m_CurType == tr("Driver Restore"))
        return;

    // 导出/刷新
    if (mp_ListView->indexAt(point).isValid()) {
        mp_Menu->addAction(mp_Export);
        mp_Menu->addAction(mp_Refresh);

        mp_Menu->exec(QCursor::pos());
    }
}

void PageListView::slotListViewItemClicked(const QModelIndex &index)
{
    // Item 点击事件
    QString concateStr = mp_ListView->getConcatenateStrings(index);
    if (!concateStr.isEmpty() && concateStr != QString("Separator")) {
        emit itemClicked(concateStr);
        m_CurType = concateStr;
    }
}
