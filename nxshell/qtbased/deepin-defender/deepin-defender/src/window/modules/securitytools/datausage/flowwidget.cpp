// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "flowwidget.h"

#include <DPinyin>
#include <DStandardItem>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QMenu>

// 表格中每列索引
const int IIsbOnShowingCol = 6;
const int IPidCol = 7;
const int IIsbSysAppCol = 8;
const int IExecPathCol = 9;
const int IPkgNameCol = 10;
const int IAppNameCol = 11;
const int IIconCol = 12;
const int IDownloadsCol = 13;
const int IUploadsCol = 14;
const int ITotalsCol = 15;

#define COL_LEFT_ADD_SPACE 10 // 左侧增加10间隔，保持与表头标题一致

FlowWidget::FlowWidget(DataUsageModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
    , m_layout(new QVBoxLayout)
    , m_totalUsageLabel(nullptr)
    , m_timeRangeSelectBtn(nullptr)
    , m_flowtableview(new QTableView(this))
    , m_flowitemmodel(new QStandardItemModel(this))
    , m_flowTableWidget(nullptr)
    , m_flowitem(new FlowItemDelegate(this, this))
    , m_onDispTimeRangeType(TimeRangeType::None)
    , m_recedTimeRangeType(TimeRangeType::None)
    , m_waitSpinner(new DSpinner(this))
{
    this->setAccessibleName("dataUsageWidget_flowWidget");

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    QHBoxLayout *topHLayout = new QHBoxLayout;
    m_layout->addLayout(topHLayout);

    m_totalUsageLabel = new DTipLabel(tr("Received") + ": 0.00KB" + "  "
                                          + tr("Sent") + ": 0.00KB" + "  "
                                          + tr("Total") + ": 0.00KB",
                                      this);
    m_totalUsageLabel->setAccessibleName("flowWidget_totalUsageLable");
    topHLayout->addWidget(m_totalUsageLabel, 0, Qt::AlignLeft);

    topHLayout->addStretch(1);
    m_timeRangeSelectBtn = new DPushButton("time select", this);
    m_timeRangeSelectBtn->setAccessibleName("flowWidget_timeRangeSelectButton");
    m_timeRangeSelectBtn->setFlat(true);
    m_timeRangeSelectBtn->setMinimumWidth(100);
    m_timeRangeSelectBtn->setMinimumHeight(40);
    topHLayout->addWidget(m_timeRangeSelectBtn, 0, Qt::AlignRight);

    QMenu *timeRangeMenu = new QMenu(this);
    timeRangeMenu->setMinimumWidth(100);
    m_timeRangeSelectBtn->setMenu(timeRangeMenu);
    QAction *todayAction = new QAction(tr("Today"), this);
    todayAction->setObjectName("today");
    todayAction->setCheckable(true);
    timeRangeMenu->addAction(todayAction);
    QAction *yesterdayAction = new QAction(tr("Yesterday"), this);
    yesterdayAction->setObjectName("yesterday");
    yesterdayAction->setCheckable(true);
    timeRangeMenu->addAction(yesterdayAction);
    QAction *thisMonthAction = new QAction(tr("This month"), this);
    thisMonthAction->setObjectName("thisMonth");
    thisMonthAction->setCheckable(true);
    timeRangeMenu->addAction(thisMonthAction);
    QAction *lastMonthAction = new QAction(tr("Last month"), this);
    lastMonthAction->setObjectName("lastMonth");
    lastMonthAction->setCheckable(true);
    timeRangeMenu->addAction(lastMonthAction);

    // 表格
    m_flowTableWidget = new DefenderTable(this);
    m_flowTableWidget->setAccessibleName("flowWidget_flowTableWidget");
    m_flowTableWidget->setHeadViewSortHide(5);
    m_flowTableWidget->setTableAndModel(m_flowtableview, m_flowitemmodel);
    m_layout->addWidget(m_flowTableWidget, 1);
    m_flowtableview->setAccessibleName("flowTableWidget_flowTableView");

    // 等待界面
    m_waitSpinner->setFixedSize(30, 30);
    m_waitSpinner->setVisible(false);
    m_layout->addWidget(m_waitSpinner, 0, Qt::AlignCenter);

    // 信号连接
    connect(timeRangeMenu, &QMenu::triggered, this, [this](QAction *action) {
        //接受数据中，设置流量排名等待界面
        this->setWaitingPage(true);

        for (QAction *ttaction : m_timeRangeSelectBtn->menu()->actions()) {
            ttaction->setChecked(false);
        }
        m_timeRangeSelectBtn->setText(action->text());
        action->setChecked(true);
        if ("today" == action->objectName()) {
            m_onDispTimeRangeType = TimeRangeType::Today;
        } else if ("yesterday" == action->objectName()) {
            m_onDispTimeRangeType = TimeRangeType::Yesterday;
        } else if ("thisMonth" == action->objectName()) {
            m_onDispTimeRangeType = TimeRangeType::ThisMonth;
        } else if ("lastMonth" == action->objectName()) {
            m_onDispTimeRangeType = TimeRangeType::LastMonth;
        }
        asyncGetAllAppFlowListOnTime();
    });
    // 默认选择时间跨度为今天
    Q_EMIT timeRangeMenu->triggered(todayAction);

    SetFlowAttributes();

    // 定时刷新数据
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &FlowWidget::asyncGetAllAppFlowListOnTime);
    updateTimer->start(1000 * 1);

    // 连接接收数据槽
    connect(m_model, &DataUsageModel::sendAllAppFlowList, this, &FlowWidget::onRecAllAppFlowList);
}

FlowWidget::~FlowWidget()
{
    m_flowitemmodel->clear();
    m_flowitemmodel->deleteLater();
    m_flowitemmodel = nullptr;
}

void FlowWidget::onRecAllAppFlowList(const int timeRangeType, const DefenderProcInfoList &flowLst)
{
    m_recedTimeRangeType = TimeRangeType(timeRangeType);

    m_allAppFlowList = flowLst;
    updateTable();
}

//设置网速view属性
void FlowWidget::SetFlowAttributes()
{
    m_flowtableview->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_flowtableview->setSelectionMode(QAbstractItemView::NoSelection);
    m_flowtableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_flowtableview->setFrameShape(QTableView::NoFrame);
    //自动调整最后一列的宽度使它和表格的右边界对齐
    m_flowtableview->horizontalHeader()->setStretchLastSection(true);
    m_flowtableview->verticalHeader()->setHidden(true);
    m_flowtableview->setShowGrid(false);
    m_flowtableview->setSortingEnabled(true);
    m_flowtableview->verticalHeader()->setDefaultSectionSize(48);

    // 连接表格与模型
    m_flowtableview->setModel(m_flowitemmodel);
    //设置代理
    m_flowtableview->setItemDelegate(m_flowitem);
    // 初始化表格设置
    // 设置表头
    QStandardItem *standaritem0 = new QStandardItem(tr("No."));
    standaritem0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem1 = new QStandardItem(tr("Name"));
    standaritem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem2 = new QStandardItem(tr("Downloaded"));
    standaritem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem3 = new QStandardItem(tr("Uploaded"));
    standaritem3->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem4 = new QStandardItem(tr("Data Used"));
    standaritem4->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem5 = new QStandardItem(tr("Details"));
    standaritem5->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_flowitemmodel->setHorizontalHeaderItem(0, standaritem0);
    m_flowitemmodel->setHorizontalHeaderItem(1, standaritem1);
    m_flowitemmodel->setHorizontalHeaderItem(2, standaritem2);
    m_flowitemmodel->setHorizontalHeaderItem(3, standaritem3);
    m_flowitemmodel->setHorizontalHeaderItem(4, standaritem4);
    m_flowitemmodel->setHorizontalHeaderItem(5, standaritem5);

    m_flowitemmodel->setHorizontalHeaderItem(IIsbOnShowingCol, new QStandardItem(QString("IsbOnShowing")));
    m_flowitemmodel->setHorizontalHeaderItem(IPidCol, new QStandardItem(QString("Pid")));
    m_flowitemmodel->setHorizontalHeaderItem(IIsbSysAppCol, new QStandardItem(QString("IsbSysApp")));
    m_flowitemmodel->setHorizontalHeaderItem(IExecPathCol, new QStandardItem(QString("ExecName")));
    m_flowitemmodel->setHorizontalHeaderItem(IPkgNameCol, new QStandardItem(QString("PkgName")));
    m_flowitemmodel->setHorizontalHeaderItem(IAppNameCol, new QStandardItem(QString("AppName")));
    m_flowitemmodel->setHorizontalHeaderItem(IIconCol, new QStandardItem(QString("Icon")));
    m_flowitemmodel->setHorizontalHeaderItem(IDownloadsCol, new QStandardItem(QString("Downloads")));
    m_flowitemmodel->setHorizontalHeaderItem(IUploadsCol, new QStandardItem(QString("Uploads")));
    m_flowitemmodel->setHorizontalHeaderItem(ITotalsCol, new QStandardItem(QString("Totals")));

    m_flowtableview->setColumnHidden(IIsbOnShowingCol, true);
    m_flowtableview->setColumnHidden(IPidCol, true);
    m_flowtableview->setColumnHidden(IIsbSysAppCol, true);
    m_flowtableview->setColumnHidden(IExecPathCol, true);
    m_flowtableview->setColumnHidden(IPkgNameCol, true);
    m_flowtableview->setColumnHidden(IAppNameCol, true);
    m_flowtableview->setColumnHidden(IIconCol, true);
    m_flowtableview->setColumnHidden(IDownloadsCol, true);
    m_flowtableview->setColumnHidden(IUploadsCol, true);
    m_flowtableview->setColumnHidden(ITotalsCol, true);

    m_flowtableview->setColumnWidth(0, 65);
    m_flowtableview->setColumnWidth(1, 200);
    m_flowtableview->setColumnWidth(2, 125);
    m_flowtableview->setColumnWidth(3, 125);
    m_flowtableview->setColumnWidth(4, 125);
    m_flowtableview->setColumnWidth(5, 70);
    m_flowtableview->horizontalHeader()->setHighlightSections(false);

    m_flowtableview->sortByColumn(4, Qt::SortOrder::DescendingOrder);
}

void FlowWidget::updateTable()
{
    // 总流量
    double dTotalDownloads = 0.0;
    double dTotalUploads = 0.0;

    if (m_onDispTimeRangeType != m_recedTimeRangeType) {
        //设置鼠标等待界面
        m_waitSpinner->setVisible(true);
        m_waitSpinner->start();
        m_flowTableWidget->setVisible(false);
    } else {
        //恢复显示界面
        m_waitSpinner->setVisible(false);
        m_waitSpinner->stop();
        m_flowTableWidget->setVisible(true);
    }

    // 设置当前列表中所有行的 应用是否显示项 为 "false"
    for (int i = 0; i < m_flowitemmodel->rowCount(); i++) {
        m_flowitemmodel->setData(m_flowitemmodel->index(i, IIsbOnShowingCol), "false");
    }

    // 更新已存在进程数据 或 添加新进程数据
    int currentRowCount = m_flowitemmodel->rowCount();
    for (int i = 0; i < currentRowCount; i++) {
        // 获取当前行的进程名
        QString sExecPath = m_flowitemmodel->data(m_flowitemmodel->index(i, IExecPathCol), Qt::ItemDataRole::DisplayRole).toString();
        for (int j = 0; j < m_allAppFlowList.size(); j++) {
            if (sExecPath == m_allAppFlowList.at(j).sExecPath) {
                DefenderProcInfo procInfoTmp = m_allAppFlowList.at(j);

                // 设置列表中当前行的 应用是否在运行项 为 "true"
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IIsbOnShowingCol), "true");
                // 更新是否是系统应用
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IIsbSysAppCol), procInfoTmp.isbSysApp);
                // 更新
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IExecPathCol), procInfoTmp.sExecPath);
                // 更新
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IPkgNameCol), procInfoTmp.sPkgName);
                // 更新应用名
                // 如果能够获取到应用名，就取应用名；否则归为“其他应用”
                if (procInfoTmp.sAppName.isEmpty()) {
                    procInfoTmp.sAppName = tr("Other apps");
                }
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IAppNameCol), procInfoTmp.sAppName);
                // 更新当前行图标数据
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IIconCol), procInfoTmp.sThemeIcon);
                // 更新当前行流量数据
                // 如果流量小于0.01kb，则默认显示于0.01kb
                if (0.01 > procInfoTmp.dDownloads) {
                    procInfoTmp.dDownloads = 0.01;
                }
                if (0.01 > procInfoTmp.dUploads) {
                    procInfoTmp.dUploads = 0.01;
                }

                QString sDownloads = Utils::changeFlowValueUnit(procInfoTmp.dDownloads, 2);
                QString sUploads = Utils::changeFlowValueUnit(procInfoTmp.dUploads, 2);
                double dTotals = procInfoTmp.dDownloads + procInfoTmp.dUploads;
                QString sTotals = Utils::changeFlowValueUnit(dTotals, 2);
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IDownloadsCol), sDownloads);
                m_flowitemmodel->setData(m_flowitemmodel->index(i, IUploadsCol), sUploads);
                m_flowitemmodel->setData(m_flowitemmodel->index(i, ITotalsCol), sTotals);

                // ==========设置排序数据=========
                // 设置应用名称
                m_flowitemmodel->item(i, 1)->setData(DTK_CORE_NAMESPACE::Chinese2Pinyin(procInfoTmp.sAppName),
                                                     Qt::ItemDataRole::EditRole);
                // 原始单位为KB的流量大小，用于排序
                m_flowitemmodel->item(i, 2)->setData(QVariant(procInfoTmp.dDownloads), Qt::ItemDataRole::EditRole);
                m_flowitemmodel->item(i, 3)->setData(QVariant(procInfoTmp.dUploads), Qt::ItemDataRole::EditRole);
                m_flowitemmodel->item(i, 4)->setData(QVariant(dTotals), Qt::ItemDataRole::EditRole);
                // ==========设置显示数据-end=========

                // 找到已存在于表格中的进程项，提取数据后，从列表中移除该项
                m_allAppFlowList.removeAt(j);
                // 统计总流量
                dTotalDownloads += procInfoTmp.dDownloads;
                dTotalUploads += procInfoTmp.dUploads;
                break;
            }
        }
    }

    // 移除表格中没有运行的进程对应的行
    for (int i = 0; i < m_flowitemmodel->rowCount();) {
        if ("false" == m_flowitemmodel->index(i, IIsbOnShowingCol).data().toString()) {
            m_flowitemmodel->removeRow(i);
        } else {
            i++;
        }
    }

    // 添加新进程数据
    for (DefenderProcInfo procFlowInfo : m_allAppFlowList) {
        DStandardItem *snItem = new DStandardItem;
        DStandardItem *nameItem = new DStandardItem;
        DStandardItem *downloadsItem = new DStandardItem;
        DStandardItem *uploadsItem = new DStandardItem;
        DStandardItem *totalsItem = new DStandardItem;
        DStandardItem *detailsItem = new DStandardItem;

        QList<QStandardItem *> itemList;
        itemList << snItem << nameItem << downloadsItem << uploadsItem << totalsItem << detailsItem;
        m_flowitemmodel->insertRow(0, itemList);

        // ==========设置储存数据========
        // 设置进程号
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IPidCol), procFlowInfo.nPid);
        // 设置列表中当前行的 应用是否在运行项 为 "true"
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IIsbOnShowingCol), "true");
        // 更新是否是系统应用
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IIsbSysAppCol), procFlowInfo.isbSysApp);
        // 更新
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IExecPathCol), procFlowInfo.sExecPath);
        // 更新
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IPkgNameCol), procFlowInfo.sPkgName);
        // 更新应用名
        // 如果能够获取到应用名，就取应用名；否则归为“其他应用”
        if (procFlowInfo.sAppName.isEmpty()) {
            procFlowInfo.sAppName = tr("Other apps");
        }
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IAppNameCol), procFlowInfo.sAppName);
        // 更新当前行图标数据
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IIconCol), procFlowInfo.sThemeIcon);
        // 更新当前行流量数据
        QString sDownloads = Utils::changeFlowValueUnit(procFlowInfo.dDownloads, 2);
        QString sUploads = Utils::changeFlowValueUnit(procFlowInfo.dUploads, 2);
        double dTotals = procFlowInfo.dDownloads + procFlowInfo.dUploads;
        QString sTotals = Utils::changeFlowValueUnit(dTotals, 2);
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IDownloadsCol), sDownloads);
        m_flowitemmodel->setData(m_flowitemmodel->index(0, IUploadsCol), sUploads);
        m_flowitemmodel->setData(m_flowitemmodel->index(0, ITotalsCol), sTotals);
        // ==========设置储存数据-end========

        // ==========设置排序数据=========
        // 设置应用名称
        m_flowitemmodel->item(0, 1)->setData(DTK_CORE_NAMESPACE::Chinese2Pinyin(procFlowInfo.sAppName),
                                             Qt::ItemDataRole::EditRole);
        // 原始单位为KB的流量大小，用于排序
        m_flowitemmodel->item(0, 2)->setData(QVariant(procFlowInfo.dDownloads), Qt::ItemDataRole::EditRole);
        m_flowitemmodel->item(0, 3)->setData(QVariant(procFlowInfo.dUploads), Qt::ItemDataRole::EditRole);
        m_flowitemmodel->item(0, 4)->setData(QVariant(dTotals), Qt::ItemDataRole::EditRole);
        // ==========设置显示数据-end=========

        // 设置 联网管控 按钮
        QWidget *btnWgt = new QWidget(this);
        QHBoxLayout *btnLayout = new QHBoxLayout;
        btnLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);
        btnWgt->setLayout(btnLayout);

        DCommandLinkButton *detailBtn = new DCommandLinkButton(tr("Details"), this);
        detailBtn->setContentsMargins(0, 0, 0, 0);
        detailBtn->setDisabled(false);
        btnLayout->addWidget(detailBtn, 0, Qt::AlignLeft | Qt::AlignVCenter);
        m_flowtableview->setIndexWidget(m_flowitemmodel->index(0, 5), btnWgt);
        // 若不存在包名, 则设置详情按钮不可用
        if (procFlowInfo.sPkgName.isEmpty()) {
            detailBtn->setDisabled(true);
        }
        // 连接按钮信号
        connect(detailBtn, &DCommandLinkButton::clicked, this, [=] {
            showAppFlowDetailDialog(procFlowInfo);
        });

        // 统计总流量
        dTotalDownloads += procFlowInfo.dDownloads;
        dTotalUploads += procFlowInfo.dUploads;
    }

    // 根据当前排序状态排序
    Qt::SortOrder sortOrder = m_flowtableview->horizontalHeader()->sortIndicatorOrder();
    int sortCol = m_flowtableview->horizontalHeader()->sortIndicatorSection();
    m_flowtableview->sortByColumn(sortCol, sortOrder);

    // 更新总网速标签
    QString sTotalDownloads = Utils::changeFlowValueUnit(dTotalDownloads, 2);
    QString sTotalUploads = Utils::changeFlowValueUnit(dTotalUploads, 2);
    QString sTotals = Utils::changeFlowValueUnit(dTotalDownloads + dTotalUploads, 2);
    m_totalUsageLabel->setText(tr("Received") + ": " + sTotalDownloads + "  " + tr("Sent") + ": " + sTotalUploads + "  " + tr("Total") + ": " + sTotals);
}

void FlowWidget::showAppFlowDetailDialog(DefenderProcInfo procInfo)
{
    AppDataDetailDialog *appDataDetailDialog = new AppDataDetailDialog(procInfo.sAppName, procInfo.sPkgName, procInfo.sThemeIcon,
                                                                       m_model, this);
    appDataDetailDialog->setAttribute(Qt::WA_DeleteOnClose);
    appDataDetailDialog->show();
}

// 网速代理函数
FlowItemDelegate::FlowItemDelegate(FlowWidget *widget, QObject *parent)
    : QItemDelegate(parent)
{
    m_widget = widget;
}

// 网速重绘操作
void FlowItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 第1列
    if (index.column() == 0) {
        // 绘画文字的坐标
        QRect rect = option.rect;
        int x = rect.x() + 6;
        int y = rect.y() + rect.height() / 2 + 5;
        painter->drawText(x + COL_LEFT_ADD_SPACE, y, QString::number(index.row() + 1));
    } else if (index.column() == 1) {
        // 绘制图标
        QRect rect = option.rect;
        int pix_x = rect.x() + 4;
        int pix_y = rect.y() + 4;
        int height = rect.height() - 4 * 2;

        // 如果没有图标，则使用默认图标
        QString sThemeIcon = index.model()->index(index.row(), IIconCol).data(Qt::DisplayRole).toString();
        if (sThemeIcon.isEmpty()) {
            sThemeIcon = "dcc_application_executable";
        }
        painter->drawPixmap(pix_x, pix_y, height, height,
                            QIcon::fromTheme(sThemeIcon).pixmap(height, height));

        // 绘画应用名
        int textWidth = rect.width() - height - 4 * 8;
        int x = pix_x + height + 4 * 2;
        int y = pix_y + rect.height() / 4 + height / 8;
        int y1 = pix_y + rect.height() / 2 + height / 4 + 4;
        QString sName = index.model()->data(index.model()->index(index.row(), IAppNameCol), Qt::DisplayRole).toString();
        QString sElidedName = painter->fontMetrics().elidedText(sName, Qt::ElideRight, textWidth, Qt::TextShowMnemonic);
        painter->drawText(x + COL_LEFT_ADD_SPACE, y, sElidedName);

        // 绘画应用来源
        // 设置字体
        QFont sourceTypeFont;
        sourceTypeFont.setFamily("SourceHanSansSC-Normal"); //字体
        sourceTypeFont.setPixelSize(12); //文字像素大小
        QPen sourceTypePen;
        sourceTypePen.setColor(QColor("#52607f"));
        // 判断是否是系统应用
        bool isbSysApp = index.model()->data(index.model()->index(index.row(), IIsbSysAppCol), Qt::DisplayRole).toBool();
        QString sAppSourceType;
        if (isbSysApp)
            sAppSourceType = tr("System");
        else {
            sAppSourceType = tr("Third-party");
        }
        QString sElidedSource = painter->fontMetrics().elidedText(sAppSourceType, Qt::ElideRight, textWidth, Qt::TextShowMnemonic);
        painter->save();
        painter->setFont(sourceTypeFont);
        painter->setPen(sourceTypePen);
        painter->drawText(x + COL_LEFT_ADD_SPACE, y1, sElidedSource);
        // 还原绘画器
        painter->restore();
    } else if (index.column() == 2) {
        // 绘制下载流量
        QString download = index.model()->data(index.model()->index(index.row(), IDownloadsCol), Qt::DisplayRole).toString();

        // 绘画图片和文字的坐标
        QRect rect = option.rect;
        int x = rect.x() + COL_LEFT_ADD_SPACE;
        int y = rect.y() + rect.height() / 2 + 5;
        int downloadWidth = rect.width() - COL_LEFT_ADD_SPACE;
        QString sElidedDownload = painter->fontMetrics().elidedText(download, Qt::ElideRight, downloadWidth, Qt::TextShowMnemonic);
        painter->drawText(x, y, sElidedDownload);
    } else if (index.column() == 3) {
        // 绘制上传流量
        QString upload = index.model()->data(index.model()->index(index.row(), IUploadsCol), Qt::DisplayRole).toString();

        // 绘画文字的坐标
        QRect rect = option.rect;
        int x = rect.x() + COL_LEFT_ADD_SPACE;
        int y = rect.y() + rect.height() / 2 + 5;
        int uploadWidth = rect.width() - COL_LEFT_ADD_SPACE;
        QString sElidedUpload = painter->fontMetrics().elidedText(upload, Qt::ElideRight, uploadWidth, Qt::TextShowMnemonic);
        painter->drawText(x, y, sElidedUpload);
    } else if (index.column() == 4) {
        // 绘制总流量
        QString totals = index.model()->index(index.row(), ITotalsCol).data().toString();

        // 绘画文字的坐标
        QRect rect = option.rect;
        int x = rect.x() + COL_LEFT_ADD_SPACE;
        int y = rect.y() + rect.height() / 2 + 5;
        int totalsWidth = rect.width() - COL_LEFT_ADD_SPACE;
        QString sElidedTotals = painter->fontMetrics().elidedText(totals, Qt::ElideRight, totalsWidth, Qt::TextShowMnemonic);
        painter->drawText(x, y, sElidedTotals);
    }
}

void FlowWidget::setWaitingPage(bool isWaiting)
{
    if (isWaiting) {
        m_flowTableWidget->setVisible(false);
        m_waitSpinner->setVisible(true);
        m_waitSpinner->start();
    } else {
        m_flowTableWidget->setVisible(true);
        m_waitSpinner->setVisible(false);
        m_waitSpinner->stop();
    }
}

void FlowWidget::asyncGetAllAppFlowListOnTime()
{
    m_model->asyncGetAllAppFlowList(m_onDispTimeRangeType);
}
