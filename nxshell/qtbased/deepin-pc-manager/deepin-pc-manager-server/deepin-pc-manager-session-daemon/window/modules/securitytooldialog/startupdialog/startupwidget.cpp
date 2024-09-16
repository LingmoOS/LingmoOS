// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "startupwidget.h"

#include "../../deepin-pc-manager/src/window/modules/common/common.h"
#include "autostartmodel.h"

#include <qnamespace.h>

#include <DDialog>
#include <DFontSizeManager>
#include <DFrame>
#include <DHeaderView>
#include <DPinyin>
#include <DTitlebar>
#include <DTreeView>

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QDBusPendingReply>
#include <QFile>
#include <QFileInfo>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QModelIndex>
#include <QPainter>
#include <QPair>
#include <QPixmap>
#include <QPushButton>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

#define ALL_INTEVAL 0
#define LEFT_INTEVAL 10
#define PIX_INTEVAL 4            // 表格里图片距离表格边线 间隔
#define ACTION_FLAG_DISABLE 0    // 状态标志 - 不允许
#define ACTION_FLAG_ENABLE 1     // 状态标志 - 允许
#define TABLE_HEADER_SORT_HIDE 2 // 第3列掩藏
#define NAME_COL_NAME 697        // 第1列列宽
#define NAME_COL_ACTION 214      // 第2列列宽

#define Startup_Disable \
    ":/icons/deepin/builtin/icons/dcc_startup_disable_29px.svg" // 状态资源图片 - 不允许
#define Startup_Enable \
    ":/icons/deepin/builtin/icons/dcc_startup_enable_29px.svg" // 状态资源图片 - 允许

QString GetAppsSourceType(QString sPath)
{
    QFile fileInfo(sPath);
    QString source;
    if (QFileInfo(sPath).isSymLink()) {
        source = "Third-party";
    } else if (fileInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileInfo);
        in.setCodec("UTF-8");
        QString sAllInfo = in.readAll();
        int nIndex = sAllInfo.indexOf("Exec");
        QString sOptInfo = sAllInfo.mid(nIndex, 10);
        if (sOptInfo.contains("opt"))
            source = "Third-party";
        else
            source = "System";
    }
    return source;
}

StartupWidget::StartupWidget(AutoStartModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
    , m_detailInfo(nullptr)
    , m_table_view(nullptr)
    , m_item_model(nullptr)
    , m_delegate(nullptr)
    , m_tableWidget(nullptr)
    , m_rowCount(0)
    , m_nSaveSortColumn(1)
{
    this->setAccessibleName("rightWidget_startupWidget");
    initUI();

    // 后台自启动数据刷新信号连接
    connect(m_model, &AutoStartModel::itemStatusChanged, this, &StartupWidget::refreshData);
}

StartupWidget::~StartupWidget()
{
    Q_EMIT requstCloseInfo();

    m_item_model->clear();
    m_item_model->deleteLater();
    m_item_model = nullptr;
}

void StartupWidget::initUI()
{
    setBackgroundRole(QPalette::Window);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(LEFT_INTEVAL);
    layout->setContentsMargins(ALL_INTEVAL, ALL_INTEVAL, ALL_INTEVAL, ALL_INTEVAL);

    QLabel *title = new QLabel(tr("Startup Programs"), this);
    title->setAccessibleName("startupWidget_titleLable");
    QFont font1 = title->font();
    font1.setBold(true);
    font1.setPixelSize(16);
    title->setFont(font1);

    m_detailInfo = new QLabel(this);
    m_detailInfo->setAccessibleName("startupWidget_detalInfoLable");
    m_detailInfo->setFont(Utils::getFixFontSize(12));

    layout->addWidget(title);
    layout->addWidget(m_detailInfo);

    // table_view 属性
    m_table_view = new DTableView(this);
    m_table_view->setFont(Utils::getFixFontSize(14));
    m_table_view->setAccessibleName("tableFrame_tableView");
    m_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table_view->verticalHeader()->setHidden(true);
    m_table_view->setSortingEnabled(true);
    m_table_view->verticalHeader()->setDefaultSectionSize(48);
    m_table_view->setShowGrid(false);
    m_table_view->setFrameShape(QTableView::NoFrame);

    // 连接代理
    m_delegate = new MyItemDelegate(this, this);
    m_table_view->setItemDelegate(m_delegate);

    // 连接表格与模型
    m_item_model = new StartupModel();
    m_item_model->setParent(this);
    m_table_view->setModel(m_item_model);

    // 初始化表格设置
    // 设置表头
    QStandardItem *item_0 = new QStandardItem(tr("Name"));
    item_0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *item_1 = new QStandardItem(tr("Auto Startup"));
    item_1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *item_2 = new QStandardItem(tr("Action"));
    item_2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_item_model->setHorizontalHeaderItem(0, item_0);
    m_item_model->setHorizontalHeaderItem(1, item_1);
    m_item_model->setHorizontalHeaderItem(2, item_2);
    m_item_model->setHorizontalHeaderItem(3, new QStandardItem("Id"));
    m_item_model->setHorizontalHeaderItem(4, new QStandardItem("Icon"));
    m_item_model->setHorizontalHeaderItem(5, new QStandardItem("Path"));
    m_item_model->setHorizontalHeaderItem(6, new QStandardItem("RealName"));

    // 表格
    m_tableWidget = new DefenderTable(this);
    m_tableWidget->setAccessibleName("startupWidget_tableFrame");
    m_tableWidget->setHeadViewSortHide(TABLE_HEADER_SORT_HIDE);
    m_tableWidget->setTableAndModel(m_table_view, m_item_model);
    layout->addWidget(m_tableWidget);
    this->setLayout(layout);

    // 取系统所有应用
    getAppsData();
    m_table_view->horizontalHeader()->setHighlightSections(false);
    m_table_view->horizontalHeader()->setSortIndicator(1, Qt::DescendingOrder);
    m_table_view->horizontalHeader()->setStretchLastSection(true);
    m_table_view->horizontalHeader()->setVisible(true);
}

// 取系统所有应用
void StartupWidget::getAppsData()
{
    m_mapEnable.clear();
    m_mapDisable.clear();

    // 取所有应用数据信息
    bool succeed = m_model->updateAppsAutoStartStatus();
    if (succeed) {
        QString sEnableInfo = m_model->getAllAutoStartAppsInfoStr();
        QString sDisableInfo = m_model->getAllNonautoStartAppsInfoStr();

        auto formatData = [&](QString info, QMap<QString, QStringList> &map) {
            QStringList sList = info.split("|");
            for (int i = 0; i < sList.count(); ++i) {
                QStringList sDetailInfoList = sList.at(i).split(",");
                if (sDetailInfoList.count() >= 5) {
                    QList<QString> sListDetail;
                    sListDetail.append(sDetailInfoList.at(0));
                    sListDetail.append(sDetailInfoList.at(1));
                    sListDetail.append(sDetailInfoList.at(2));
                    sListDetail.append(sDetailInfoList.at(3));
                    map[sDetailInfoList.at(4)] = sListDetail;
                }
            }
        };

        formatData(sEnableInfo, m_mapEnable);
        formatData(sDisableInfo, m_mapDisable);

        m_appNamefromId.clear();
        // 先按状态降序 再按名称升序
        m_rowCount = 0;
        if (m_mapEnable.count() > 0)
            loadMap(m_mapEnable);

        if (m_mapDisable.count() > 0)
            loadMap(m_mapDisable);

        // 设置统计行数据
        showTotalInfo();
    }
}

// QMap容器 数据加载
void StartupWidget::loadMap(QMap<QString, QStringList> map)
{
    // 加载应用
    QMapIterator<QString, QStringList> i(map);
    while (i.hasNext()) {
        i.next();
        QStringList list = i.value();
        if (list.count() < 4)
            break;

        QString sName = i.key();
        QString sPath = list.at(0);
        QString sIcon = list.at(1);
        QString sId = list.at(2);
        QString sRealName = list.at(3);

        // 加载应用
        loadAppData(m_rowCount, sPath, sName, sIcon, sId, sRealName);
        m_rowCount++;
    }

    // 隐藏不需要显示的列
    m_table_view->setColumnHidden(1, true);
    m_table_view->setColumnHidden(3, true);
    m_table_view->setColumnHidden(4, true);
    m_table_view->setColumnHidden(5, true);
    m_table_view->setColumnHidden(6, true);

    // 设置前三列的宽度
    m_table_view->setColumnWidth(0, NAME_COL_NAME);
    m_table_view->setColumnWidth(2, NAME_COL_ACTION);
}

// 加载每个应用的详细信息
void StartupWidget::loadAppData(
    int nRow, QString sPath, QString sName, QString sIcon, QString sId, QString sRealName)
{
    // 判断该应用状态（是否是自启动）
    bool bStatus = m_model->isAppAutoStart(sPath);

    // 设置每一行数据
    m_item_model->setItem(nRow, 0, new QStandardItem(QString("%1").arg(sName)));
    m_item_model->setItem(
        nRow,
        1,
        new QStandardItem(QString("%1").arg(bStatus ? ACTION_FLAG_ENABLE : ACTION_FLAG_DISABLE)));
    m_item_model->setItem(nRow, 2, new QStandardItem("statusButton"));
    m_item_model->setItem(nRow, 3, new QStandardItem(QString("%1").arg(sId)));
    m_item_model->setItem(nRow, 4, new QStandardItem(QString("%1").arg(sIcon)));
    m_item_model->setItem(nRow, 5, new QStandardItem(QString("%1").arg(sPath)));
    m_item_model->setItem(nRow, 6, new QStandardItem(QString("%1").arg(sRealName)));

    m_appNamefromId[sId] = sRealName;

    // 字体样式
    QPalette pal;
    if (bStatus)
        pal.setColor(QPalette::ButtonText, Qt::red);
    // 添加操作按钮 文本、颜色、属性等 （操作与应用状态正好相反）
    QString sDisableText = (bStatus) ? tr("Enabled") : tr("Disabled");
    QPushButton *button = new QPushButton(this);
    button->setObjectName(sId);
    button->setText(QString("%1").arg(sDisableText));
    button->setPalette(pal);
    button->setProperty("status", bStatus ? ACTION_FLAG_DISABLE : ACTION_FLAG_ENABLE);
    button->setProperty("path", sPath);
    button->setProperty("id", sId);
    connect(button, &QPushButton::clicked, this, &StartupWidget::onTableBtnClicked);

    DFrame *buttonFrame = new DFrame(this);
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->setContentsMargins(6, 6, 20, 6);
    buttonLayout->addWidget(button);
    buttonFrame->setLayout(buttonLayout);
    buttonFrame->setFrameStyle(QFrame::NoFrame);
    m_table_view->setIndexWidget(m_item_model->index(nRow, 2), buttonFrame);
}

// 表格按钮点击槽
void StartupWidget::onTableBtnClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    int nStatus = button->property("status").toInt();
    QString sPath = button->property("path").toString();
    QString sID = button->property("id").toString();

    // 执行自启动开启/关闭操作
    bool isExeAutoStart = m_model->setAppAutoStart(nStatus, sPath);

    if (isExeAutoStart) {
        if (ACTION_FLAG_DISABLE == nStatus)
            changeItemStatus(false, sID);
        else
            changeItemStatus(true, sID);

        // 添加安全日志
        QString log = nStatus ? tr("Enabled auto startup for %1").arg(m_appNamefromId[sID])
                              : tr("Disabled auto startup for %1").arg(m_appNamefromId[sID]);
        m_model->addSecurityLog(SECURITY_LOG_TYPE_TOOL, log);
        showTotalInfo();
    }
}

// 按钮点击后改变相应行的显示数据和状态
void StartupWidget::changeItemStatus(bool isStartup, QString sFlagData)
{
    QModelIndex index = m_table_view->currentIndex();
    int rows = m_item_model->rowCount();
    int column = m_item_model->columnCount();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < column; ++j) {
            QString sData =
                m_item_model->data(m_item_model->index(i, j), Qt::DisplayRole).toString();
            if (j == 3 && sData == sFlagData) {
                QWidget *widget = m_table_view->indexWidget(m_item_model->index(i, 2));
                QPushButton *button = widget->findChild<QPushButton *>(sFlagData);

                // 字体样式
                QPalette pal;
                if (isStartup)
                    pal.setColor(QPalette::ButtonText, Qt::red);
                // 添加操作按钮 文本、颜色、属性等 （操作与应用状态正好相反）
                QString sDisableText = (isStartup) ? tr("Enabled") : tr("Disabled");
                button->setText(QString("%1").arg(sDisableText));
                button->setPalette(pal);
                button->setProperty("status", isStartup ? ACTION_FLAG_DISABLE : ACTION_FLAG_ENABLE);
            }
        }
    }

    // 设置前三列的宽度
    m_table_view->setColumnWidth(0, NAME_COL_NAME);
    m_table_view->setColumnWidth(2, NAME_COL_ACTION);

    m_table_view->selectRow(index.row());
}

// 外部调用刷新表格数据
void StartupWidget::refreshData(bool bAdd, QString sID)
{
    changeItemStatus(bAdd, sID);
    showTotalInfo();
}

// 设置统计行信息
void StartupWidget::showTotalInfo()
{
    int nAllCount = 0;
    int nEnableCount = 0;
    int nDisableCount = 0;

    bool isStarted = m_model->updateAppsAutoStartStatus();
    if (isStarted) {
        QString sEnableInfo = m_model->getAllAutoStartAppsInfoStr();
        QString sDisableInfo = m_model->getAllNonautoStartAppsInfoStr();

        if (!sEnableInfo.isEmpty())
            nEnableCount = sEnableInfo.split("|").count();

        if (!sDisableInfo.isEmpty())
            nDisableCount = sDisableInfo.split("|").count();
    }
    nAllCount = nEnableCount + nDisableCount;

    // 设置统计行数据
    m_detailInfo->setText(tr("Startup apps") + QString(": %1      ").arg(nEnableCount)
                          + tr("All apps") + QString(": %1").arg(nAllCount));
}

// 代理函数
MyItemDelegate::MyItemDelegate(StartupWidget *widget, QObject *parent)
    : QItemDelegate(parent)
{
    m_widget = widget;
}

// 重绘操作
void MyItemDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    // 第一列
    if (index.column() == 0) {
        // 图标路径
        QString sIconPath =
            index.model()->data(index.model()->index(index.row(), 4), Qt::DisplayRole).toString();
        // 读取该应用执行信息
        QString sPath =
            index.model()->data(index.model()->index(index.row(), 5), Qt::DisplayRole).toString();
        // 设置图片
        QRect rect = option.rect;

        int pix_x = rect.x() + PIX_INTEVAL;
        int pix_y = rect.y() + PIX_INTEVAL;
        int height = rect.height() - PIX_INTEVAL * 2;

        QFont font2;
        font2.setFamily("SourceHanSansSC-Normal"); // 字体
        font2.setPixelSize(12);                    // 文字像素大小
        QPen pen2;
        pen2.setColor(QColor("#52607f"));

        QString sStr = GetAppsSourceType(sPath).toStdString().c_str();
        QString sAppsSources;
        if (sStr == "Third-party") {
            sAppsSources = tr("Third-party");
        } else {
            sAppsSources = tr("System");
        }

        QFile file(sIconPath);
        if (file.open(QIODevice::ReadOnly) && sIconPath.contains("/")) {
            file.close();
            // 绘画图片坐标
            QPixmap pixmap = QPixmap(sIconPath);
            pixmap = m_widget->getPixmap(pixmap);
            painter->drawPixmap(pix_x + LEFT_INTEVAL, pix_y, height, height, pixmap);

        } else {
            // 绘画图片坐标
            QIcon icon = QIcon::fromTheme(sIconPath);
            QRect rectIcon(pix_x + LEFT_INTEVAL, pix_y, height, height);
            icon.paint(painter, rectIcon);
        }

        // 绘画文字的坐标
        painter->save();
        int textWidth = rect.width() - height - PIX_INTEVAL * 8;
        int x = pix_x + height + PIX_INTEVAL * 2;
        int y = pix_y + rect.height() / 4 + height / 8;
        int y1 = pix_y + rect.height() / 2 + height / 4 + PIX_INTEVAL;
        QString sName =
            index.model()->data(index.model()->index(index.row(), 6), Qt::DisplayRole).toString();
        QString sElidedName = painter->fontMetrics().elidedText(sName,
                                                                Qt::ElideRight,
                                                                textWidth,
                                                                Qt::TextShowMnemonic);
        painter->drawText(x + LEFT_INTEVAL, y, sElidedName);

        // 绘画应用来源
        QString sElidedSource = painter->fontMetrics().elidedText(sAppsSources,
                                                                  Qt::ElideRight,
                                                                  textWidth,
                                                                  Qt::TextShowMnemonic);
        painter->setFont(font2);
        painter->setPen(pen2);
        painter->drawText(x + LEFT_INTEVAL, y1, sElidedSource);
        painter->restore();
    }
    // 第二列
    else if (index.column() == 1) {
        // 设置图片
        int nStatus = index.model()->data(index, Qt::DisplayRole).toInt();
        QPixmap pixmap = (nStatus == 0) ? QPixmap(Startup_Disable) : QPixmap(Startup_Enable);

        pixmap = m_widget->getPixmap(pixmap);
        const QPixmap &star = pixmap;
        // 绘画图片和文字的坐标
        int height = star.height();
        QRect rect = option.rect;
        int x = rect.x() + PIX_INTEVAL * 2;
        int y = rect.y() + rect.height() / 2 - height / 2;

        painter->drawPixmap(x, y, star);

    } else {
        //        QItemDelegate::paint(painter, option, index);
        //        return;
    }
}

QPixmap StartupWidget::getPixmap(QPixmap pixmap)
{
    const qreal ratio = devicePixelRatioF();
    pixmap.scaled(pixmap.size() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}

StartupModel::StartupModel() { }

QVariant StartupModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
        return QStandardItemModel::data(index, role);
    } else if (role == Qt::BackgroundRole) {
        if (index.row() % 2 == 0) {
            return QColor(0, 0, 0, 8);
        } else if (index.row() % 5 == 1)
            return QColor("#FFFFFF");
        else
            return QVariant();
    }

    return QVariant();
}

StartupMainWindow::StartupMainWindow(QWidget *pParent)
    : DefSecurityToolsMnd(pParent)
    , m_pStartupWidget(nullptr)
{
    AutoStartModel *pAutoStartModel = new AutoStartModel(this);
    m_pStartupWidget = new StartupWidget(pAutoStartModel);
    addContent(m_pStartupWidget);
    setFixedSize(960, 640);
}

StartupMainWindow::~StartupMainWindow() { }

void StartupMainWindow::closeEvent(QCloseEvent *event)
{
    Q_EMIT sendWindowClose();
    event->accept();
}
