// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "TableWidget.h"
#include "PageDriverControl.h"
#include "MacroDefinition.h"
#include "logviewitemdelegate.h"
#include "logtreeview.h"
#include "DBusWakeupInterface.h"

// Dtk头文件
#include <DFontSizeManager>
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DHeaderView>
#include <DMenu>

// Qt库文件
#include <QPainter>
#include <QLoggingCategory>
#include <QHBoxLayout>
#include <QPainterPath>

#define WAKEUP_OPEN 3
#define WAKEUP_CLOSE 4

TableWidget::TableWidget(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new LogTreeView(this))
    , m_HLayout(nullptr)
    , mp_Enable(new QAction(tr("Disable"), this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export"), this))
    , mp_updateDriver(new QAction(tr("Update drivers"), this))
    , mp_removeDriver(new QAction(tr("Uninstall drivers"), this))
    , mp_WakeupMachine(new QAction(tr("Allow it to wake the computer"), this))
    , mp_Menu(new DMenu(this))
    , m_Enable(false)

{
    initWidget();

    // 连接信号和曹函数
    connect(mp_Table, &LogTreeView::clicked, this, &TableWidget::slotItemClicked);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &TableWidget::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &TableWidget::slotActionExport);
    connect(mp_Enable, &QAction::triggered, this, &TableWidget::slotActionEnable);
    connect(mp_updateDriver, &QAction::triggered, this, &TableWidget::slotActionUpdateDriver);
    connect(mp_removeDriver, &QAction::triggered, this, &TableWidget::slotActionRemoveDriver);
    connect(mp_WakeupMachine, &QAction::triggered, this, &TableWidget::slotWakeupMachine);
}

TableWidget::~TableWidget()
{
    if (mp_Table) {
        delete mp_Table;
        mp_Table = nullptr;
    }

    if (m_HLayout) {
        delete m_HLayout;
        m_HLayout = nullptr;
    }

    if (mp_Enable) {
        delete mp_Enable;
        mp_Enable = nullptr;
    }

    if (mp_Refresh) {
        delete mp_Refresh;
        mp_Refresh = nullptr;
    }

    if (mp_Export) {
        delete mp_Export;
        mp_Export = nullptr;
    }

    if (mp_Menu) {
        delete mp_Menu;
        mp_Menu = nullptr;
    }
}

void TableWidget::setHeaderLabels(const QStringList &lst)
{
    QStringList headers;
    for (int i = 0; i < lst.size(); i++) {
        if (i < lst.size() - 1) {
            headers.append(lst[i]);
        } else {
            m_Enable = lst[i] == "yes" ? true : false;
        }
    }

    if (mp_Table) {
        mp_Table->setHeaderLabels(headers);
    }
}

void TableWidget::setItem(int row, int column, DStandardItem *item)
{
    if (mp_Table) {
        mp_Table->setItem(row, column, item);
    }
}

void TableWidget::setColumnAverage()
{
    if (mp_Table) {
        mp_Table->setColumnAverage();
    }
}

void TableWidget::updateCurItemEnable(int row, bool enable)
{
    if (mp_Table)
        mp_Table->updateCurItemEnable(row, enable);
}

void TableWidget::clear()
{
    if (mp_Table) {
        mp_Table->clear();
    }
}

void TableWidget::setRowNum(int row)
{
    // 设置表格行数
    mp_Table->setRowNum(row);
}

void TableWidget::paintEvent(QPaintEvent *e)
{
    DWidget::paintEvent(e);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    int radius = 8;

//     获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 设置Widget固定高度,(+1)表示包含表头高度,(*2)表示上下边距，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
    this->setFixedHeight(TREE_ROW_HEIGHT * (mp_Table->RowNum() + 1) + HORSCROLL_WIDTH + WIDGET_MARGIN * 2);
    QRect rect  = this->rect();

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    int width = 1;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
}

void TableWidget::slotShowMenu(const QPoint &point)
{
    mp_Menu->clear();
    // 不管什么状态 导出、刷新、复制 都有
    mp_Refresh->setEnabled(true);
    mp_Export->setEnabled(true);
    mp_Enable->setEnabled(true);
    mp_updateDriver->setEnabled(true);
    mp_removeDriver->setEnabled(true);
    mp_WakeupMachine->setEnabled(true);
    mp_WakeupMachine->setCheckable(true);
    mp_WakeupMachine->setChecked(false);

    // 不可用状态：卸载和启用禁用置灰
    if (!mp_Table->currentRowAvailable()) {
        mp_Enable->setEnabled(false);
        mp_removeDriver->setEnabled(false);
    }
    // 禁用状态：更新卸载置灰
    if (mp_Table->currentRowEnable()) {
        mp_Enable->setText(tr("Disable"));
    } else {
        mp_updateDriver->setEnabled(false);
        mp_removeDriver->setEnabled(false);
        mp_Enable->setEnabled(true);
        mp_Enable->setText(tr("Enable"));
    }
    // 驱动界面打开状态： 驱动的更新卸载和设备的启用禁用置灰
    if (PageDriverControl::isRunning()) {
        mp_updateDriver->setEnabled(false);
        mp_removeDriver->setEnabled(false);
        mp_Enable->setEnabled(false);
    }


    int row = mp_Table->currentRow();
    if(row < 0)
        return;
    bool isInstalled = false;
    bool isPrinter = false;
    //主线程时使用时会阻塞执行
    emit signalCheckPrinterStatus(row, isPrinter, isInstalled);
    //dde-printer未安装
    if (isPrinter && !isInstalled) {
        mp_updateDriver->setEnabled(false);
    }

    // 添加按钮到菜单
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    QModelIndexList selected = mp_Table->selectionModel()->selectedRows();
    // 选中item状态下才有启用/禁用按钮
    if (m_Enable && selected.size() > 0) {
        mp_Menu->addAction(mp_Enable);
    }
    // 主板、内存、cpu等没有驱动，无需右键按钮
    // 选中item状态下才有卸载、更新按钮
    bool canUninstall = true , canEnable = true;
    QStandardItem* item = mp_Table->item(row,0);
    if(item){ // 获取该设备是否可以更新卸载驱动
        canUninstall = item->data(Qt::UserRole).toString()=="true" ? true : false;
        canEnable = item->data(Qt::UserRole+1).toString()=="true" ? true : false;
    }
    if(!canEnable){
        mp_Enable->setEnabled(false);
    }
    if (canUninstall && selected.size() > 0) {
        mp_Menu->addSeparator();
        mp_Menu->addAction(mp_updateDriver);
        mp_Menu->addAction(mp_removeDriver);
    }

    QVariant canWakeup = item->data(Qt::UserRole+2);
    if(canWakeup.isValid()){
        mp_Menu->addSeparator();

        QString str = canWakeup.toString();
        // 先判断是网卡唤醒还是键鼠唤醒
        if("true" != str && "false" != str){
            // 网络唤醒菜单处理处理
            int res = DBusWakeupInterface::getInstance()->isNetworkWakeup(str);
            if(WAKEUP_OPEN == res){
                mp_WakeupMachine->setChecked(true);
            }else if(WAKEUP_CLOSE == res){
                mp_WakeupMachine->setChecked(false);
            }else{
                mp_WakeupMachine->setEnabled(false);
            }
        }else{ // 简述右键菜单处理
            bool canWakeupBool = str == "true" ? true : false;
            if(canWakeupBool){
                QString wakeupPath = item->data(Qt::UserRole+3).toString();
                QFile file(wakeupPath);
                bool isWakeup = false;
                if(file.open(QIODevice::ReadOnly)){
                    QString info = file.readAll();
                    if(info.contains("disabled"))
                        isWakeup = false;
                    else
                        isWakeup =  true;
                }
                mp_WakeupMachine->setChecked(isWakeup);
            }else{
                mp_WakeupMachine->setEnabled(false);
            }
        }

        // 如果是禁用状态，则唤醒置灰
        if(!mp_Table->currentRowEnable())
            mp_WakeupMachine->setEnabled(false);
        mp_Menu->addAction(mp_WakeupMachine);
    }
    mp_Menu->exec(QCursor::pos());
}

void TableWidget::slotActionRefresh()
{
    emit refreshInfo();
}

void TableWidget::slotActionExport()
{
    emit exportInfo();
}

void TableWidget::slotActionEnable()
{
    if (!mp_Table) {
        return;
    }

    // enble device
    if (mp_Enable->text() == tr("Enable")) {
        emit enableDevice(mp_Table->currentRow(), true);
    } else {
        // unenable device
        emit enableDevice(mp_Table->currentRow(), false);
    }
}

void TableWidget::slotActionUpdateDriver()
{
    emit installDriver(mp_Table->currentRow());
}

void TableWidget::slotActionRemoveDriver()
{
    emit uninstallDriver(mp_Table->currentRow());
}

void TableWidget::slotWakeupMachine()
{
    emit wakeupMachine(mp_Table->currentRow(),mp_WakeupMachine->isChecked());
}

void TableWidget::slotItemClicked(const QModelIndex &index)
{
    // click table item
    int row = index.row();
    if (row >= 0) {
        emit itemClicked(row);
    }
}

void TableWidget::initWidget()
{
    // init widget layout
    m_HLayout = new QHBoxLayout(this);
    int margin = 2;
    m_HLayout->setContentsMargins(margin, margin, margin, margin);
    m_HLayout->addWidget(mp_Table);
    setLayout(m_HLayout);
}
