// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbconnectionwidget.h"
#include "widgets/settingsgroup.h"
#include "widgets/titlebuttonitem.h"
#include "widgets/defendertable.h"
#include "widgets/settingsheaderitem.h"
#include "widgets/usbinfolabel.h"
#include "widgets/usbinfodialog.h"

#include <DCommandLinkButton>
#include <DTipLabel>
#include <DTableView>
#include <DApplicationHelper>
#include <DSimpleListView>
#include <DDialog>
#include <DFontSizeManager>
#include <DButtonBox>
#include <DApplicationHelper>
#include <DSuggestButton>
#include <DPinyin>
#include <DWarningButton>

#include <QVBoxLayout>
#include <QApplication>
#include <QMenu>
#include <QStandardItemModel>
#include <QTableView>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QDateTime>

DCORE_USE_NAMESPACE

#define ALLOW_ALL_OBJ_NAME "allowAll" // 允许所有菜单项名称
#define ALLOW_PART_OBJ_NAME "allowPart" // 仅允许白名单菜单项名称
#define BLOCK_ALL_OBJ_NAME "blockAll" // 禁止所有菜单项名称

// 限制模式菜单检索
#define LIMIT_MODE_ALLOW_ALL_INDEX 0
#define LIMIT_MODE_PART_INDEX 1
#define LIMIT_MODE_BLOCK_ALL_INDEX 2

// 连接记录列表 各项列数
#define USB_LOG_ITEM_NAME_COL 0 // 连接记录列表 名称项列数
#define USB_LOG_ITEM_TIME_COL 1 // 连接记录列表 时间项列数
#define USB_LOG_ITEM_STATUS_COL 2 // 连接记录列表 连接状态项列数
#define USB_LOG_ITEM_BTN_COL 3 // 连接记录列表 按钮项列数
#define USB_LOG_ITEM_DATA_COL 4 // 连接记录列表 数据项列数

// 白名单列表 各项列数
#define USB_WHITELIST_ITEM_NAME_COL 0 // 白名单列表 名称项列数
#define USB_WHITELIST_ITEM_TIME_COL 1 // 白名单列表 时间项列数
#define USB_WHITELIST_ITEM_BTN_COL 2 // 白名单列表 按钮项列数
#define USB_WHITELIST_ITEM_DATA_COL 3 // 白名单列表 数据项列数

// 数据存储偏移
#define DATA_ROLE_NAME Qt::UserRole + 1 // 数据存储偏移 - 名称
#define DATA_ROLE_TIME Qt::UserRole + 2 // 数据存储偏移 - 时间
#define DATA_ROLE_STATUS Qt::UserRole + 3 // 数据存储偏移 - 连接状态
#define DATA_ROLE_SERIAL Qt::UserRole + 4 // 数据存储偏移 - 序列号
#define DATA_ROLE_VENDOR_ID Qt::UserRole + 5 // 数据存储偏移 - 制造商id
#define DATA_ROLE_PRODUCT_ID Qt::UserRole + 6 // 数据存储偏移 - 生产商id
#define DATA_ROLE_OP_STATUS Qt::UserRole + 7 // 数据存储偏移 - 操作（添加/移除）白名单状态

// 白名单操作状态
#define WHITELIST_OP_STATUS_ADD 1 // 添加
#define WHITELIST_OP_STATUS_REMOVE 0 // 移除

const QColor WhiteListRemoveColor = QColor(255, 87, 54, 255); // 白名单移除按钮颜色
const QColor WhiteListAddColor = QColor(0, 130, 250, 255); // 白名单添加按钮颜色

UsbConnectionWidget::UsbConnectionWidget(UsbConnectionWork *work, QWidget *parent)
    : QWidget(parent)
    , m_usbConnectionWork(work)
    , m_isbSaveRecord(false)
    , m_limitationModel(USB_LIMIT_MODEL_ALL)
    , m_usbLimitModeItem(nullptr)
    , m_usbLimitModeItemButtonLabel(nullptr)
    , m_connectionRecordTableWgt(nullptr)
    , m_whiteLstTableWgt(nullptr)
    , m_connectionRecordTable(nullptr)
    , m_connectionRecordTableModel(nullptr)
    , m_clearLogBtn(nullptr)
    , m_whiteLstTable(nullptr)
    , m_whiteLstTableModel(nullptr)
{
    this->setAccessibleName("rightWidget_usbConnectionWidget");
    //init data
    m_isbSaveRecord = m_usbConnectionWork->isbSaveRecord();
    m_limitationModel = m_usbConnectionWork->limitationModel();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignLeft);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    DLabel *titleLabel = new DLabel(tr("USB Connection"), this);
    titleLabel->setAccessibleName("usbConnectionWidget_titleLable");
    titleLabel->setContentsMargins(10, 0, 0, 0);
    QFont font = titleLabel->font();
    font.setBold(true);
    titleLabel->setFont(font);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T5);
    mainLayout->addWidget(titleLabel);

    m_usbLimitModeItem = new TitleButtonItem(this);
    m_usbLimitModeItem->setAccessibleName("usbConnectionWidget_usbLimitModelFrame");
    m_usbLimitModeItem->setAccessibleParentText("settingFrame");
    m_usbLimitModeItem->setParent(this);
    m_usbLimitModeItem->setTitle(tr("Connection limitation"));
    mainLayout->addWidget(m_usbLimitModeItem);

    //usbLimitModeItemButton
    m_usbLimitModeItem->button()->setMinimumWidth(150);
    QHBoxLayout *usbLimitModeItemButtonLayout = new QHBoxLayout;
    usbLimitModeItemButtonLayout->setContentsMargins(10, 0, 10, 0);
    m_usbLimitModeItem->button()->setLayout(usbLimitModeItemButtonLayout);
    m_usbLimitModeItemButtonLabel = new DLabel(this);
    m_usbLimitModeItemButtonLabel->setAccessibleName("settingButton_usbLimitModelLable");
    usbLimitModeItemButtonLayout->addWidget(m_usbLimitModeItemButtonLabel);
    m_usbLimitModeItemButtonLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QMenu *usbLimitModeMenu = new QMenu(this);
    m_usbLimitModeItem->button()->setMenu(usbLimitModeMenu);
    QAction *allowAllAction = new QAction(tr("Allow All"), this);
    allowAllAction->setObjectName(ALLOW_ALL_OBJ_NAME);
    allowAllAction->setCheckable(true);
    usbLimitModeMenu->addAction(allowAllAction);
    QAction *onlyWhitelistAction = new QAction(tr("Only Whitelist"), this);
    onlyWhitelistAction->setObjectName(ALLOW_PART_OBJ_NAME);
    onlyWhitelistAction->setCheckable(true);
    usbLimitModeMenu->addAction(onlyWhitelistAction);
    QAction *blockAllAction = new QAction(tr("Block All"), this);
    blockAllAction->setObjectName(BLOCK_ALL_OBJ_NAME);
    blockAllAction->setCheckable(true);
    usbLimitModeMenu->addAction(blockAllAction);

    // 初始化限制模式选项
    if (USB_LIMIT_MODEL_ALL == m_limitationModel) {
        allowAllAction->setChecked(true);
        m_usbLimitModeItemButtonLabel->setText(allowAllAction->text());
    } else if (USB_LIMIT_MODEL_PARTY == m_limitationModel) {
        onlyWhitelistAction->setChecked(true);
        m_usbLimitModeItemButtonLabel->setText(onlyWhitelistAction->text());
    } else if (USB_LIMIT_MODEL_ZERO == m_limitationModel) {
        blockAllAction->setChecked(true);
        m_usbLimitModeItemButtonLabel->setText(blockAllAction->text());
    }

    //connectionRecordBtn and whiteListBtn
    mainLayout->addSpacing(20);
    DButtonBox *tableSwitchBtn = new DButtonBox(this);
    tableSwitchBtn->setAccessibleName("usbConnectionWidget_tableSwitchButton");
    mainLayout->addWidget(tableSwitchBtn, 0, Qt::AlignHCenter);

    QList<DButtonBoxButton *> tableSwitchBtnLst;
    DButtonBoxButton *connectionRecordBtn = new DButtonBoxButton(tr("Connections"), this); //链接记录
    connectionRecordBtn->setAccessibleName("tableSwitchButton_connectionsButton");
    connectionRecordBtn->setFixedWidth(160);
    tableSwitchBtnLst.append(connectionRecordBtn);

    DButtonBoxButton *whiteListBtn = new DButtonBoxButton(tr("Whitelist"), this); //白名单
    whiteListBtn->setAccessibleName("tableSwitchButton_whiteListButton");
    whiteListBtn->setFixedWidth(160);
    tableSwitchBtnLst.append(whiteListBtn);

    tableSwitchBtn->setButtonList(tableSwitchBtnLst, true);
    connectionRecordBtn->setChecked(true);

    //添加两个表格模块
    mainLayout->addLayout(initTwoTable());
    m_connectionRecordTableWgt->hide();
    m_whiteLstTableWgt->hide();
    showConnectionRecordTableWgt();

    // 随数据处理对象中的操作按钮状态来设置界面中按钮状态
    enableAllWidgets(m_usbConnectionWork->getAllOperatingBtnStatus());

    //connect
    connect(m_usbConnectionWork, &UsbConnectionWork::sendAppendUsbConnectionLogInfo, this, &UsbConnectionWidget::onReceiveUsbInfo);
    // 通知去改变连接记录开关
    connect(m_usbConnectionWork, &UsbConnectionWork::notifyChangeIsbSaveRecord, this, &UsbConnectionWidget::changeIsbSaveRecord);
    // usb限制模式按钮信号
    connect(usbLimitModeMenu, &QMenu::triggered, this, &UsbConnectionWidget::selectLimitationModel);
    // 通知去改变usb限制模式
    connect(m_usbConnectionWork, &UsbConnectionWork::notifyChangeLimitationModel,
            this, &UsbConnectionWidget::updateUsbLimitModeItem);

    // 通知添加白名单
    connect(m_usbConnectionWork, &UsbConnectionWork::notifyAddUsbConnectionWhiteLst,
            this, &UsbConnectionWidget::addUsbConnectionWhiteLst);
    // 权限校验完成
    connect(m_usbConnectionWork, &UsbConnectionWork::checkAuthorizationFinished, this, [this] {
        this->enableAllWidgets(true);
    });

    // 连接清除记录按钮信号
    connect(m_clearLogBtn, &DCommandLinkButton::clicked, this, &UsbConnectionWidget::confirmClearUsbConnectionLog);
    // 通知去清除usb连接记录
    connect(m_usbConnectionWork, &UsbConnectionWork::notifyClearUsbConnectionLog,
            this, &UsbConnectionWidget::clearUsbConnectionLog);

    connect(connectionRecordBtn, &DButtonBoxButton::clicked, this, &UsbConnectionWidget::showConnectionRecordTableWgt);
    connect(whiteListBtn, &DButtonBoxButton::clicked, this, &UsbConnectionWidget::showWhiteLstTableWgt);
}

UsbConnectionWidget::~UsbConnectionWidget()
{
    m_connectionRecordTableModel->clear();
    m_connectionRecordTableModel->deleteLater();
    m_connectionRecordTableModel = nullptr;

    m_whiteLstTableModel->clear();
    m_whiteLstTableModel->deleteLater();
    m_whiteLstTableModel = nullptr;
}

void UsbConnectionWidget::onReceiveUsbInfo(UsbConnectionInfo info)
{
    //清除超过30天记录项
    cleanUsbConnectionLog();

    appendUsbConnectionLog(info);

    updateNameinWhiteLst(info.sName, info.sSerial, info.sVendorID, info.sProductID);

    //不显示 数据 栏
    m_connectionRecordTable->setColumnHidden(USB_LOG_ITEM_DATA_COL, true);
}

void UsbConnectionWidget::refreshUsbConnectionTable()
{
    for (int i = 0; i < m_connectionRecordTableModel->rowCount(); i++) {
        // 获取该列数据
        QStandardItem *dataItem = m_connectionRecordTableModel->item(i, USB_LOG_ITEM_DATA_COL);
        UsbConnectionInfo info = getInfoFromDataItem(dataItem);
        // 获取按钮
        QWidget *wgt = m_connectionRecordTable->indexWidget(m_connectionRecordTableModel->index(i, USB_LOG_ITEM_BTN_COL));
        DCommandLinkButton *linkBtn = wgt->findChild<DCommandLinkButton *>("linkBtn");
        if (m_usbConnectionWork->isUsbDeviceInWhiteList(info.sSerial, info.sVendorID, info.sProductID)) {
            linkBtn->setText(tr("Remove"));
            linkBtn->setToolTip(tr("Remove"));
            DPalette pa = DApplicationHelper::instance()->palette(linkBtn);
            pa.setColor(DPalette::ColorRole::Highlight, WhiteListRemoveColor);
            DApplicationHelper::instance()->setPalette(linkBtn, pa);
            // 设置操作状态
            dataItem->setData(WHITELIST_OP_STATUS_REMOVE, DATA_ROLE_OP_STATUS);
        } else {
            linkBtn->setText(tr("Add"));
            linkBtn->setToolTip(tr("Add"));
            DPalette pa = DApplicationHelper::instance()->palette(linkBtn);
            pa.setColor(DPalette::ColorRole::Highlight, WhiteListAddColor);
            DApplicationHelper::instance()->setPalette(linkBtn, pa);
            // 设置操作状态
            dataItem->setData(WHITELIST_OP_STATUS_ADD, DATA_ROLE_OP_STATUS);
        }
    }
}

void UsbConnectionWidget::removeItemFromWhiteLstTable(UsbConnectionInfo info)
{
    //如果id为空，提示添加失败
    if (info.sSerial.isEmpty() || info.sVendorID.isEmpty() || info.sProductID.isEmpty()) {
        qDebug() << "add to white list failed" << info.sSerial << info.sVendorID << info.sProductID;
        return;
    }

    // 清除白名单表格中该名单
    for (int i = 0; i < m_whiteLstTableModel->rowCount(); i++) {
        // 遍历白名单信息
        QStandardItem *dataItemTmp = m_whiteLstTableModel->item(i, USB_WHITELIST_ITEM_DATA_COL);
        UsbConnectionInfo infoTmp = getInfoFromDataItem(dataItemTmp);
        // 如果不在白名单中就移除该行
        if (info.sSerial == infoTmp.sSerial && info.sVendorID == infoTmp.sVendorID && info.sProductID == infoTmp.sProductID) {
            m_whiteLstTableModel->removeRow(i);
            break;
        }
    }
}

QHBoxLayout *UsbConnectionWidget::initTwoTable()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    m_connectionRecordTableWgt = initConnectionRecordTableWgt();
    m_connectionRecordTableWgt->setAccessibleName("usbConnectionWidget_connRecordWidget");
    mainLayout->addWidget(m_connectionRecordTableWgt);

    m_whiteLstTableWgt = initWhiteLstTableWgt();
    m_whiteLstTableWgt->setAccessibleName("usbConnectionWidget_whiteListWidget");
    mainLayout->addWidget(m_whiteLstTableWgt);

    return mainLayout;
}

QWidget *UsbConnectionWidget::initConnectionRecordTableWgt()
{
    QWidget *mainWgt = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainWgt->setLayout(mainLayout);

    m_connectionRecordTable = new DTableView(this);
    m_connectionRecordTable->setAccessibleName("detailTableFrame_tableView");
    // 设置代理
    m_connectionRecordTable->setItemDelegateForColumn(USB_LOG_ITEM_NAME_COL,
                                                      new UsbTableNameItemDelegate(UsbTableType::Record, m_connectionRecordTable));
    //设置表格行标题
    m_connectionRecordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_connectionRecordTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_connectionRecordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_connectionRecordTable->setFrameShape(QTableView::NoFrame);
    m_connectionRecordTable->setIconSize(QSize(25, 25));
    //自动调整最后一列的宽度使它和表格的右边界对齐
    m_connectionRecordTable->horizontalHeader()->setStretchLastSection(true);
    m_connectionRecordTable->verticalHeader()->setHidden(true);
    m_connectionRecordTable->setShowGrid(false);
    m_connectionRecordTable->setSortingEnabled(true);
    m_connectionRecordTable->verticalHeader()->setDefaultSectionSize(30);

    // 设置表头
    QStandardItem *standaritem0 = new QStandardItem(tr("Device"));
    standaritem0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem1 = new QStandardItem(tr("Time Connected"));
    standaritem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem2 = new QStandardItem(tr("Status"));
    standaritem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem3 = new QStandardItem(tr("Whitelist"));
    standaritem3->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_connectionRecordTableModel = new QStandardItemModel(this);
    m_connectionRecordTable->setModel(m_connectionRecordTableModel);
    m_connectionRecordTableModel->clear();
    m_connectionRecordTableModel->setHorizontalHeaderItem(USB_LOG_ITEM_NAME_COL, standaritem0);
    m_connectionRecordTableModel->setHorizontalHeaderItem(USB_LOG_ITEM_TIME_COL, standaritem1);
    m_connectionRecordTableModel->setHorizontalHeaderItem(USB_LOG_ITEM_STATUS_COL, standaritem2);
    m_connectionRecordTableModel->setHorizontalHeaderItem(USB_LOG_ITEM_BTN_COL, standaritem3);
    m_connectionRecordTableModel->horizontalHeaderItem(USB_LOG_ITEM_BTN_COL)->setSelectable(false);
    m_connectionRecordTableModel->setRowCount(0);

    //创建usb连接记录表格
    DefenderTable *detailTable = new DefenderTable;
    detailTable->setAccessibleName("connRecordWidget_detailTableFrame");
    detailTable->setHeadViewSortHide(USB_LOG_ITEM_BTN_COL);
    detailTable->setTableAndModel(m_connectionRecordTable, m_connectionRecordTableModel);
    detailTable->setRowHeight(30);

    mainLayout->addWidget(detailTable);

    //bottom line
    QHBoxLayout *bottomLayout = new QHBoxLayout;

    m_clearLogBtn = new DCommandLinkButton(tr("Clear all records"));
    m_clearLogBtn->setAccessibleName("connRecordWidget_clearButton");
    bottomLayout->addWidget(m_clearLogBtn);

    // 根据表格内容是否为空，设置清除记录按钮是否可用
    if (0 == m_connectionRecordTableModel->rowCount()) {
        m_clearLogBtn->setDisabled(true);
    }

    bottomLayout->addStretch(1);
    DTipLabel *logTableTip = new DTipLabel(tr("Show record in 30 days"));
    logTableTip->setAccessibleName("connRecordWidget_logTableTipLable");
    bottomLayout->addWidget(logTableTip);

    mainLayout->addLayout(bottomLayout);

    //设置usb连接记录表格内容
    QList<UsbConnectionInfo> usbConnectionInfoLst = m_usbConnectionWork->usbConnectionInfoLst();
    for (int i = 0; i < usbConnectionInfoLst.size(); i++) {
        UsbConnectionInfo info = usbConnectionInfoLst.at(i);
        appendUsbConnectionLog(info);
    }

    //不显示 数据 栏
    m_connectionRecordTable->setColumnHidden(USB_LOG_ITEM_DATA_COL, true);

    //设置各栏宽度
    m_connectionRecordTable->setColumnWidth(USB_LOG_ITEM_NAME_COL, 240);
    m_connectionRecordTable->setColumnWidth(USB_LOG_ITEM_TIME_COL, 190);
    m_connectionRecordTable->setColumnWidth(USB_LOG_ITEM_STATUS_COL, 150);

    //默认排序
    m_connectionRecordTable->sortByColumn(USB_LOG_ITEM_TIME_COL, Qt::SortOrder::DescendingOrder);

    return mainWgt;
}

QWidget *UsbConnectionWidget::initWhiteLstTableWgt()
{
    QWidget *mainWgt = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainWgt->setLayout(mainLayout);

    m_whiteLstTable = new DTableView(this);
    m_whiteLstTable->setAccessibleName("whiteListFrame_tableView");
    // 设置代理
    m_whiteLstTable->setItemDelegateForColumn(USB_WHITELIST_ITEM_NAME_COL,
                                              new UsbTableNameItemDelegate(UsbTableType::Whitelist, m_whiteLstTable));
    //设置表格行标题
    m_whiteLstTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_whiteLstTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_whiteLstTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_whiteLstTable->setFrameShape(QTableView::NoFrame);
    //自动调整最后一列的宽度使它和表格的右边界对齐
    m_whiteLstTable->horizontalHeader()->setStretchLastSection(true);
    m_whiteLstTable->verticalHeader()->setHidden(true);
    m_whiteLstTable->setShowGrid(false);
    m_whiteLstTable->setSortingEnabled(true);
    m_whiteLstTable->verticalHeader()->setDefaultSectionSize(35);

    // 设置表头
    QStandardItem *standaritem0 = new QStandardItem(tr("Device"));
    standaritem0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem1 = new QStandardItem(tr("Time Added"));
    standaritem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem2 = new QStandardItem(tr("Action"));
    standaritem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_whiteLstTableModel = new QStandardItemModel(this);
    m_whiteLstTable->setModel(m_whiteLstTableModel);
    m_whiteLstTableModel->clear();
    m_whiteLstTableModel->setHorizontalHeaderItem(USB_WHITELIST_ITEM_NAME_COL, standaritem0);
    m_whiteLstTableModel->setHorizontalHeaderItem(USB_WHITELIST_ITEM_TIME_COL, standaritem1);
    m_whiteLstTableModel->setHorizontalHeaderItem(USB_WHITELIST_ITEM_BTN_COL, standaritem2);
    m_whiteLstTableModel->setRowCount(0);

    //创建usb连接记录表格
    DefenderTable *detailTable = new DefenderTable(this);
    detailTable->setAccessibleName("whiteListWidget_whiteListFrame");
    detailTable->setHeadViewSortHide(USB_WHITELIST_ITEM_BTN_COL);
    detailTable->setTableAndModel(m_whiteLstTable, m_whiteLstTableModel);
    detailTable->setRowHeight(30);

    //设置usb连接记录表格内容
    QList<UsbConnectionInfo> usbConnectionWhiteLst = m_usbConnectionWork->usbConnectionWhiteLst();
    for (int i = 0; i < usbConnectionWhiteLst.size(); i++) {
        UsbConnectionInfo info = usbConnectionWhiteLst.at(i);
        appendUsbConnectionWhiteList(info);
    }

    //不显示 数据 栏
    m_whiteLstTable->setColumnHidden(USB_WHITELIST_ITEM_DATA_COL, true);

    //设置各栏宽度
    m_whiteLstTable->setColumnWidth(USB_WHITELIST_ITEM_NAME_COL, 300);
    m_whiteLstTable->setColumnWidth(USB_WHITELIST_ITEM_TIME_COL, 250);

    //默认排序
    m_whiteLstTable->sortByColumn(USB_WHITELIST_ITEM_TIME_COL, Qt::SortOrder::DescendingOrder);

    mainLayout->addWidget(detailTable);

    //white list table tip
    DTipLabel *whiteListTableTip = new DTipLabel(tr("You can add devices to the whitelist from the connection record"), this);
    whiteListTableTip->setAccessibleName("whiteListWidget_tipLable");
    whiteListTableTip->setWordWrap(true);
    whiteListTableTip->setAlignment(Qt::AlignmentFlag::AlignLeft);
    mainLayout->addWidget(whiteListTableTip);

    return mainWgt;
}

void UsbConnectionWidget::confirmChangeIsbSaveRecord()
{
    // 关闭时,需要授权
    if (m_usbConnectionWork->isbSaveRecord()) {
        // 授权弹窗前，禁用所有控件
        enableAllWidgets(false);
        // 请求确认是否改变连接记录开关
        m_usbConnectionWork->requestConfirmChangeIsbSaveRecord();
    } else {
        m_usbConnectionWork->changeIsbSaveRecord();
    }
    // 根据实际开关值刷新界面中开关状态
    changeIsbSaveRecord();
}

void UsbConnectionWidget::showConnectionRecordTableWgt()
{
    m_connectionRecordTableWgt->show(); //usb链接页面
    m_whiteLstTableWgt->hide();
}

void UsbConnectionWidget::showWhiteLstTableWgt()
{
    m_connectionRecordTableWgt->hide();
    m_whiteLstTableWgt->show();
}

void UsbConnectionWidget::appendUsbConnectionLog(UsbConnectionInfo info)
{
    // 数据项
    QStandardItem *dataItem = createDataItemFromInfo(info);

    QList<QStandardItem *> itemLst;
    QString deviceName = dataItem->data(DATA_ROLE_NAME).toString();
    QStandardItem *nameItem = new QStandardItem(deviceName);
    // 转换中文，使中文排序正常
    ushort firstUnicode = deviceName.front().unicode();
    if (0x4E00 <= firstUnicode) {
        QString pinyinStr = Chinese2Pinyin(deviceName).insert(0, "中");
        nameItem->setData(pinyinStr, Qt::ItemDataRole::EditRole);
    }
    QString timeAdded = dataItem->data(DATA_ROLE_TIME).toString();
    QStandardItem *timeItem = new QStandardItem(timeAdded);
    QString status = dataItem->data(DATA_ROLE_STATUS).toString();
    QStandardItem *statusItem = new QStandardItem;
    //设置限制状态图标
    if ("Connected" == status) {
        statusItem->setIcon(QIcon::fromTheme(USB_CONTROL_CONNECTED_ICON));
        statusItem->setText(tr("Connected"));
    } else {
        statusItem->setIcon(QIcon::fromTheme(USB_CONTROL_LIMITED_ICON));
        statusItem->setText(tr("Blocked"));
    }
    QStandardItem *btnItem = new QStandardItem; //白名单 操作栏

    itemLst << nameItem << timeItem << statusItem << btnItem << dataItem;

    m_connectionRecordTableModel->insertRow(0, itemLst);

    //按钮 - 添加白名单
    QWidget *btnWgt = new QWidget(this);
    QVBoxLayout *btnLayout = new QVBoxLayout;
    btnLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnWgt->setLayout(btnLayout);
    DCommandLinkButton *linkBtn = new DCommandLinkButton("", this);
    // 白名单操作状态
    int operationStatus = WHITELIST_OP_STATUS_REMOVE;
    // 判断设备是否在白名单中
    if (m_usbConnectionWork->isUsbDeviceInWhiteList(info.sSerial, info.sVendorID, info.sProductID)) {
        // 设置操作状态
        operationStatus = WHITELIST_OP_STATUS_REMOVE;
        // 设置按钮
        linkBtn->setText(tr("Remove"));
        linkBtn->setToolTip(tr("Remove"));
        linkBtn->setObjectName("linkBtn");

        DPalette pa = DApplicationHelper::instance()->palette(linkBtn);
        pa.setColor(DPalette::ColorRole::Highlight, WhiteListRemoveColor);
        DApplicationHelper::instance()->setPalette(linkBtn, pa);
    } else {
        // 设置操作状态
        operationStatus = WHITELIST_OP_STATUS_ADD;
        // 设置按钮
        linkBtn->setText(tr("Add"));
        linkBtn->setToolTip(tr("Add"));
        linkBtn->setObjectName("linkBtn");
        DPalette pa = DApplicationHelper::instance()->palette(linkBtn);
        pa.setColor(DPalette::ColorRole::Highlight, WhiteListAddColor);
        DApplicationHelper::instance()->setPalette(linkBtn, pa);
    }
    // 保存操作状态
    dataItem->setData(operationStatus, DATA_ROLE_OP_STATUS);
    // 将按钮添加到列表
    btnLayout->addWidget(linkBtn);
    m_connectionRecordTable->setIndexWidget(m_connectionRecordTableModel->index(0, USB_LOG_ITEM_BTN_COL), btnWgt);
    //connect
    connect(linkBtn, &DCommandLinkButton::clicked, this, [this, dataItem] {
        int dataPosX = dataItem->index().column();
        int dataPosY = dataItem->index().row();
        this->confirmChangeUsbConnectionWhiteLst(UsbTableType::Record, dataPosX, dataPosY);
    });

    //为使上面加入的一行内的控件对齐
    m_connectionRecordTableModel->insertRow(0, new QStandardItem);
    m_connectionRecordTableModel->removeRow(0);

    // 设置清除记录按钮可用
    m_clearLogBtn->setDisabled(false);
}

void UsbConnectionWidget::confirmClearUsbConnectionLog()
{
    DDialog *dialog = new DDialog(this);
    dialog->setAccessibleName("clearUsbConnDialog");
    dialog->setModal(true);
    // 设置关闭后删除
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
    dialog->setMessage(tr("Are you sure you want to clear all the records?"));
    dialog->setWordWrapMessage(true);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    QPushButton *cancelBtn = new QPushButton(dialog);
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setAccessibleName("clearUsbConnDialog_cancelButton");
    cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
    dialog->insertButton(0, cancelBtn, false);

    DWarningButton *exitBtn = new DWarningButton(dialog);
    exitBtn->setText(tr("Confirm"));
    exitBtn->setObjectName("confirmBtn");
    exitBtn->setAccessibleName("clearUsbConnDialog_confirmButton");
    exitBtn->setAttribute(Qt::WA_NoMousePropagation);
    dialog->insertButton(1, exitBtn, true);

    QLabel *messageLable = dialog->findChild<QLabel *>("MessageLabel");
    messageLable->setAccessibleName("clearUsbConnDialog_messageLable");

    connect(dialog, &DDialog::buttonClicked, this, [this](int index, QString) {
        switch (index) {
        case 0:
            break;
        case 1:
            // 授权弹窗前，禁用所有控件
            enableAllWidgets(false);
            // 请求确认清除usb连接记录
            m_usbConnectionWork->requestConfirmClearUsbConnectionLog();
            break;
        default:
            break;
        }
    });
    dialog->show();
}

void UsbConnectionWidget::clearUsbConnectionLog()
{
    //清除表格数据
    int count = m_connectionRecordTableModel->rowCount();
    for (int i = 0; i < count; i++) {
        m_connectionRecordTableModel->removeRow(0);
    }

    // 设置清除记录按钮不可用
    m_clearLogBtn->setDisabled(true);

    //同步到work
    m_usbConnectionWork->clearUsbConnectionLog();
    m_usbConnectionWork->clearSrvUsbConnectionLog();
}

void UsbConnectionWidget::appendUsbConnectionWhiteList(UsbConnectionInfo info)
{
    // 数据项
    QStandardItem *dataItem = createDataItemFromInfo(info);

    QList<QStandardItem *> itemLst;
    // 名称项
    QString deviceName = dataItem->data(DATA_ROLE_NAME).toString();
    QStandardItem *nameItem = new QStandardItem(deviceName);
    // 转换中文，使中文排序正常
    ushort firstUnicode = deviceName.front().unicode();
    if (0x4E00 <= firstUnicode) {
        QString pinyinStr = Chinese2Pinyin(deviceName).insert(0, "中");
        nameItem->setData(pinyinStr, Qt::ItemDataRole::EditRole);
    }
    // 时间项
    QString timeAdded = dataItem->data(DATA_ROLE_TIME).toString();
    QStandardItem *timeItem = new QStandardItem(timeAdded);
    // 按钮项
    QStandardItem *btnItem = new QStandardItem; //白名单 操作栏

    itemLst << nameItem << timeItem << btnItem << dataItem;

    m_whiteLstTableModel->insertRow(0, itemLst);

    //按钮 - “详情”&“移除”
    QWidget *btnWgt = new QWidget(this);
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnWgt->setLayout(btnLayout);

    DPushButton *detailBtn = new DPushButton;
    detailBtn->setToolTip(tr("Details"));
    detailBtn->setObjectName("details");
    detailBtn->setFlat(true);
    detailBtn->setIcon(QIcon::fromTheme(USB_CONTROL_DETAIL_ICON));
    detailBtn->setIconSize(QSize(17, 17));
    btnLayout->addWidget(detailBtn);

    btnLayout->addSpacing(20);
    DPushButton *removeBtn = new DPushButton;
    removeBtn->setToolTip(tr("Remove"));
    removeBtn->setObjectName("removeBtn");
    removeBtn->setFlat(true);
    removeBtn->setIcon(QIcon::fromTheme(USB_CONTROL_REMOVE_ICON));
    removeBtn->setIconSize(QSize(14, 19));
    btnLayout->addWidget(removeBtn);
    m_whiteLstTable->setIndexWidget(m_whiteLstTableModel->index(0, USB_WHITELIST_ITEM_BTN_COL), btnWgt);
    //connect
    connect(detailBtn, &DPushButton::clicked, this, [this, dataItem] {
        this->openUsbConnectionDetails(dataItem);
    });
    connect(removeBtn, &DPushButton::clicked, this, [this, dataItem] {
        // 从白名单列表 m_usbConnectionWhiteLst 中移除该项usb设备信息
        int dataPosX = dataItem->index().column();
        int dataPosY = dataItem->index().row();
        this->confirmChangeUsbConnectionWhiteLst(UsbTableType::Whitelist, dataPosX, dataPosY);
    });
}

void UsbConnectionWidget::updateUsbLimitModeItem()
{
    // 获取当前限制模式
    m_limitationModel = m_usbConnectionWork->limitationModel();

    int checkedActionIndex = LIMIT_MODE_ALLOW_ALL_INDEX;
    if (USB_LIMIT_MODEL_ALL == m_limitationModel) {
        checkedActionIndex = LIMIT_MODE_ALLOW_ALL_INDEX;
    } else if (USB_LIMIT_MODEL_PARTY == m_limitationModel) {
        checkedActionIndex = LIMIT_MODE_PART_INDEX;
    } else if (USB_LIMIT_MODEL_ZERO == m_limitationModel) {
        checkedActionIndex = LIMIT_MODE_BLOCK_ALL_INDEX;
    }
    for (QAction *ttaction : m_usbLimitModeItem->button()->menu()->actions()) {
        ttaction->setChecked(false);
    }
    QAction *checkedAction = m_usbLimitModeItem->button()->menu()->actions().at(checkedActionIndex);
    checkedAction->setChecked(true);
    m_usbLimitModeItemButtonLabel->setText(checkedAction->text());
}

void UsbConnectionWidget::cleanUsbConnectionLog()
{
    QDateTime limitTime = QDateTime::currentDateTime().addDays(-30);
    for (int i = 0; i < m_connectionRecordTableModel->rowCount(); i++) {
        QString datetime = m_connectionRecordTableModel->item(i, USB_LOG_ITEM_DATA_COL)->data(DATA_ROLE_TIME).toString();
        QDateTime itemTime = QDateTime::fromString(datetime, "yyyy-MM-dd hh:mm:ss");
        //清除超过30天记录项
        if (limitTime > itemTime) {
            m_connectionRecordTableModel->removeRow(i);
            i--;
        }
    }
}

void UsbConnectionWidget::updateNameinWhiteLst(QString sName, QString sSerial, QString sVendorID, QString sProductID)
{
    if (sName.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_whiteLstTableModel->rowCount(); i++) {
        // 遍历白名单信息
        QStandardItem *dataItemTmp = m_whiteLstTableModel->item(i, USB_WHITELIST_ITEM_DATA_COL);
        UsbConnectionInfo infoTmp = getInfoFromDataItem(dataItemTmp);

        // 如果不在白名单中就移除该行
        if (sSerial == infoTmp.sSerial && sVendorID == infoTmp.sVendorID && sProductID == infoTmp.sProductID) {
            m_whiteLstTableModel->item(i, USB_WHITELIST_ITEM_DATA_COL)->setData(sName, DATA_ROLE_NAME);
        }
    }
}

// 通过usb连接信息获得数据项
QStandardItem *UsbConnectionWidget::createDataItemFromInfo(UsbConnectionInfo info)
{
    // 数据项
    QStandardItem *dataItem = new QStandardItem;

    // 设备名
    QString deviceName;
    if (info.sName.isEmpty()) {
        deviceName = QString(tr("%1 Volume")).arg(info.sParentSize);
    } else {
        deviceName = info.sName;
    }
    dataItem->setData(deviceName, DATA_ROLE_NAME);

    // 添加时间
    dataItem->setData(info.sDatetime, DATA_ROLE_TIME);
    // 连接状态
    dataItem->setData(info.sStatus, DATA_ROLE_STATUS);
    // 序列号
    dataItem->setData(info.sSerial, DATA_ROLE_SERIAL);
    // 经销商id
    dataItem->setData(info.sVendorID, DATA_ROLE_VENDOR_ID);
    // 制造商id
    dataItem->setData(info.sProductID, DATA_ROLE_PRODUCT_ID);
    // 添加/移除白名单状态
    dataItem->setData(WHITELIST_OP_STATUS_REMOVE, DATA_ROLE_OP_STATUS);

    return dataItem;
}

// 通过数据项获得usb连接信息
UsbConnectionInfo UsbConnectionWidget::getInfoFromDataItem(QStandardItem *dataItem) const
{
    UsbConnectionInfo info;
    info.sName = dataItem->data(DATA_ROLE_NAME).toString();
    info.sDatetime = dataItem->data(DATA_ROLE_TIME).toString();
    info.sStatus = dataItem->data(DATA_ROLE_STATUS).toString();
    info.sSerial = dataItem->data(DATA_ROLE_SERIAL).toString();
    info.sVendorID = dataItem->data(DATA_ROLE_VENDOR_ID).toString();
    info.sProductID = dataItem->data(DATA_ROLE_PRODUCT_ID).toString();

    return info;
}

void UsbConnectionWidget::enableAllWidgets(bool enable)
{
    // 设置数据处理对象中的操作按钮状态
    m_usbConnectionWork->setAllOperatingBtnStatus(enable);
    // 设置界面
    m_usbLimitModeItem->setEnabled(enable);

    // 使能/禁用 连接记录列表中所有白名单操作按钮
    for (int i = 0; i < m_connectionRecordTableModel->rowCount(); i++) {
        // 获取按钮
        QWidget *wgt = m_connectionRecordTable->indexWidget(m_connectionRecordTableModel->index(i, USB_LOG_ITEM_BTN_COL));
        DCommandLinkButton *linkBtn = wgt->findChild<DCommandLinkButton *>("linkBtn");
        linkBtn->setEnabled(enable);
        qreal opacity;
        // 根据状态设置属性
        if (enable) {
            opacity = 1;
        } else {
            opacity = 0.5;
        }
        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
        opacityEffect->setOpacity(opacity);
        linkBtn->setGraphicsEffect(opacityEffect);
    }

    // 使能/禁用 白名单列表中所有白名单操作按钮
    for (int i = 0; i < m_whiteLstTableModel->rowCount(); i++) {
        // 获取按钮
        QWidget *wgt = m_whiteLstTable->indexWidget(m_whiteLstTableModel->index(i, USB_WHITELIST_ITEM_BTN_COL));
        DPushButton *removeBtn = wgt->findChild<DPushButton *>("removeBtn");
        removeBtn->setEnabled(enable);
    }

    if (!enable) {
        m_clearLogBtn->setEnabled(false);
    } else {
        m_clearLogBtn->setEnabled(true);
        // 根据表格中有无内容来设置日志清理按钮状态
        if (0 == m_connectionRecordTableModel->rowCount()) {
            m_clearLogBtn->setEnabled(false);
        }
    }

    update();
}

void UsbConnectionWidget::changeIsbSaveRecord()
{
    m_isbSaveRecord = m_usbConnectionWork->isbSaveRecord();
}

void UsbConnectionWidget::selectLimitationModel(QAction *action)
{
    action->setChecked(true);

    if (ALLOW_ALL_OBJ_NAME == action->objectName()) {
        // 如果再次选择相同项，则不做处理
        if (USB_LIMIT_MODEL_ALL == m_limitationModel)
            return;
        // 授权弹窗前，禁用所有控件
        enableAllWidgets(false);
        //身份验证
        m_usbConnectionWork->requestConfirmSelectLimitationModel(USB_LIMIT_MODEL_ALL);
    } else if (ALLOW_PART_OBJ_NAME == action->objectName()) {
        // 如果再次选择相同项，则不做处理
        if (USB_LIMIT_MODEL_PARTY == m_limitationModel) {
            return;
        } else if (USB_LIMIT_MODEL_ZERO == m_limitationModel) {
            // 授权弹窗前，禁用所有控件
            enableAllWidgets(false);
            // 身份验证
            m_usbConnectionWork->requestConfirmSelectLimitationModel(USB_LIMIT_MODEL_PARTY);
        } else {
            m_usbConnectionWork->selectLimitationModel(USB_LIMIT_MODEL_PARTY);
        }
    } else if (BLOCK_ALL_OBJ_NAME == action->objectName()) {
        if (USB_LIMIT_MODEL_ZERO == m_limitationModel)
            return;
        //用户确认
        DDialog *dialog = new DDialog(this);
        dialog->setAccessibleName("blockAllConnDialog");
        dialog->setModal(true);
        dialog->setOnButtonClickedClose(true);
        // 设置关闭后删除
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        // 设置左上角图标
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
        dialog->setMessage(tr("Are you sure you want to block the connection of all USB storage devices?"));
        dialog->setWordWrapMessage(true);

        // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
        // 通过insertButton的形式定制按钮,保持accessiableName的一致性
        QPushButton *cancelBtn = new QPushButton(dialog);
        cancelBtn->setText(tr("Cancel"));
        cancelBtn->setObjectName("cancelBtn");
        cancelBtn->setAccessibleName("blockAllConnDialog_cancelButton");
        cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
        dialog->insertButton(0, cancelBtn, false);

        DWarningButton *exitBtn = new DWarningButton(dialog);
        exitBtn->setText(tr("Confirm"));
        exitBtn->setObjectName("confirmBtn");
        exitBtn->setAccessibleName("blockAllConnDialog_confirmButton");
        exitBtn->setAttribute(Qt::WA_NoMousePropagation);
        dialog->insertButton(1, exitBtn, true);

        QLabel *messageLable = dialog->findChild<QLabel *>("MessageLabel");
        messageLable->setAccessibleName("blockAllConnDialog_messageLable");

        connect(dialog, &DDialog::buttonClicked, this, [this](int index, QString) {
            switch (index) {
            case 0:
                updateUsbLimitModeItem();
                break;
            case 1:
                // 状态修改，同步到work
                m_usbConnectionWork->selectLimitationModel(USB_LIMIT_MODEL_ZERO);
                break;
            default:
                updateUsbLimitModeItem();
                break;
            }
        });
        connect(dialog, &DDialog::closed, this, &UsbConnectionWidget::updateUsbLimitModeItem);
        dialog->show();
    }

    updateUsbLimitModeItem();
}

void UsbConnectionWidget::confirmChangeUsbConnectionWhiteLst(UsbTableType type, int dataPosX, int dataPosY)
{
    // 获取点击项的信息
    QStandardItem *dataItem = nullptr;
    switch (type) {
    case UsbTableType::Record:
        dataItem = m_connectionRecordTableModel->item(dataPosY, dataPosX);
        break;
    case UsbTableType::Whitelist:
        dataItem = m_whiteLstTableModel->item(dataPosY, dataPosX);
        break;
    }

    if (!dataItem)
        return;

    UsbConnectionInfo info = getInfoFromDataItem(dataItem);

    //如果id为空，提示添加失败
    if (info.sSerial.isEmpty() || info.sVendorID.isEmpty() || info.sProductID.isEmpty()) {
        DDialog *dialog = new DDialog(this);
        dialog->setAccessibleName("unableAddWhiteDialog");
        dialog->setOnButtonClickedClose(true);
        dialog->setModal(true);
        // 设置关闭后删除
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        // 设置弹窗左上角图标
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
        dialog->setMessage(tr("The hardware info of this device is incomplete, unable to add it to the whitelist"));
        dialog->setWordWrapMessage(true);

        // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
        // 通过insertButton的形式定制按钮,保持accessiableName的一致性
        QPushButton *cancelBtn = new QPushButton(dialog);
        cancelBtn->setText(tr("OK"));
        cancelBtn->setObjectName("okBtn");
        cancelBtn->setAccessibleName("unableAddWhiteDialog_okButton");
        cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
        dialog->insertButton(0, cancelBtn, false);

        QLabel *messageLable = dialog->findChild<QLabel *>("MessageLabel");
        messageLable->setAccessibleName("unableAddWhiteDialog_messageLable");

        dialog->show();
        qDebug() << "add to white list failed";
        return;
    }

    //用户确认
    int operationStatus = dataItem->data(DATA_ROLE_OP_STATUS).toInt();
    if (WHITELIST_OP_STATUS_REMOVE == operationStatus) {
        DDialog *dialog = new DDialog(this);
        dialog->setAccessibleName("sureRemoveListDialog");
        dialog->setModal(true);
        // 设置关闭后删除
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setMessage(tr("Are you sure you want to remove it from the whitelist?"));
        dialog->setWordWrapMessage(true);
        // 设置弹窗左上角图标
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));

        // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
        // 通过insertButton的形式定制按钮,保持accessiableName的一致性
        QPushButton *cancelBtn = new QPushButton(dialog);
        cancelBtn->setText(tr("Cancel"));
        cancelBtn->setObjectName("cancelBtn");
        cancelBtn->setAccessibleName("sureRemoveListDialog_cancelButton");
        cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
        dialog->insertButton(0, cancelBtn, false);

        DWarningButton *exitBtn = new DWarningButton(dialog);
        exitBtn->setText(tr("Remove"));
        exitBtn->setObjectName("removeBtn");
        exitBtn->setAccessibleName("sureRemoveListDialog_removeButton");
        exitBtn->setAttribute(Qt::WA_NoMousePropagation);
        dialog->insertButton(1, exitBtn, true);

        QLabel *messageLable = dialog->findChild<QLabel *>("MessageLabel");
        messageLable->setAccessibleName("sureRemoveListDialog_messageLable");

        connect(dialog, &DDialog::buttonClicked, this, [=](int index, QString) {
            switch (index) {
            case 0:
                break;
            case 1:
                this->changeUsbConnectionWhiteLst(operationStatus, info);
                break;
            default:
                break;
            }
        });
        dialog->show();
    } else { // 添加
        // 授权弹窗前，禁用所有控件
        enableAllWidgets(false);
        m_usbConnectionWork->requestConfirmAddUsbConnectionWhiteLst(info);
    }
}

void UsbConnectionWidget::changeUsbConnectionWhiteLst(int operationStatus, UsbConnectionInfo info)
{
    if (WHITELIST_OP_STATUS_ADD == operationStatus) {
        // 若白名单中已存在此项，则忽略
        if (m_usbConnectionWork->isUsbDeviceInWhiteList(info.sSerial, info.sVendorID, info.sProductID))
            return;
        // 白名单添加时间
        info.sDatetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        appendUsbConnectionWhiteList(info);
        m_usbConnectionWork->appendToUsbConnectionWhiteLst(info);
        // 不显示 数据 栏
        m_whiteLstTable->setColumnHidden(USB_WHITELIST_ITEM_DATA_COL, true);
    } else { // 移除
        m_usbConnectionWork->removeFromUsbConnectionWhiteLst(info);
        //从白名单表格显示窗口中删除该行
        removeItemFromWhiteLstTable(info);
    }
    //刷新usb连接记录表格中各行的按钮栏
    refreshUsbConnectionTable();
}

void UsbConnectionWidget::addUsbConnectionWhiteLst(const UsbConnectionInfo &info)
{
    DDialog *dialog = new DDialog(this);
    dialog->setAccessibleName("addListSuccDialog");
    // 设置弹窗左上角图标
    dialog->setIcon(QIcon::fromTheme(DIALOG_DEFENDER));
    dialog->setMessage(tr("Added to the whitelist successfully"));
    dialog->setWordWrapMessage(true);
    // 设置关闭后删除
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    QPushButton *cancelBtn = new QPushButton(dialog);
    cancelBtn->setText(tr("OK"));
    cancelBtn->setObjectName("okBtn");
    cancelBtn->setAccessibleName("addListSuccDialog_okButton");
    cancelBtn->setAttribute(Qt::WA_NoMousePropagation);
    dialog->insertButton(0, cancelBtn, false);

    QLabel *messageLable = dialog->findChild<QLabel *>("MessageLabel");
    messageLable->setAccessibleName("addListSuccDialog_messageLable");

    dialog->exec();
    changeUsbConnectionWhiteLst(WHITELIST_OP_STATUS_ADD, info);
}

void UsbConnectionWidget::openUsbConnectionDetails(QStandardItem *dateItem)
{
    // 通过数据项获得usb连接信息
    UsbConnectionInfo info = getInfoFromDataItem(dateItem);

    UsbInfoDialog *dialog = new UsbInfoDialog(this);
    dialog->setFixedWidth(380);
    dialog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *infoLayout = new QGridLayout;
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    infoLayout->setSpacing(0);
    QWidget *infoWgt = new QWidget(this);
    infoWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    infoWgt->setLayout(infoLayout);
    //添加信息窗口到对话框中
    dialog->mainLayout()->addWidget(infoWgt);

    DLabel *deviceNameTitle = new DLabel(tr("Device").append(": "), this);
    deviceNameTitle->setContentsMargins(0, 0, 20, 0); //标题右侧预留空间
    infoLayout->addWidget(deviceNameTitle, 0, 0, Qt::AlignTop);
    UsbInfoLabel *deviceNameValue = new UsbInfoLabel(info.sName, this);
    deviceNameValue->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(deviceNameValue, 0, 1, Qt::AlignTop);

    DLabel *vendorIDTitle = new DLabel("VendorID: ", this);
    vendorIDTitle->setContentsMargins(0, 0, 20, 0); //标题右侧预留空间
    infoLayout->addWidget(vendorIDTitle, 1, 0, Qt::AlignTop);
    UsbInfoLabel *vendorIDValue = new UsbInfoLabel(info.sVendorID, this);
    vendorIDValue->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(vendorIDValue, 1, 1, Qt::AlignTop);

    DLabel *productIDTitle = new DLabel("ProductID: ", this);
    productIDTitle->setContentsMargins(0, 0, 20, 0); //标题右侧预留空间
    infoLayout->addWidget(productIDTitle, 2, 0, Qt::AlignTop);
    UsbInfoLabel *productIDValue = new UsbInfoLabel(info.sProductID, this);
    vendorIDValue->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(productIDValue, 2, 1, Qt::AlignTop);

    DLabel *serialTitle = new DLabel("SerialNumber: ", this);
    serialTitle->setContentsMargins(0, 0, 20, 0); //标题右侧预留空间
    infoLayout->addWidget(serialTitle, 3, 0, Qt::AlignTop);
    UsbInfoLabel *serialValue = new UsbInfoLabel(info.sSerial, this);
    serialValue->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(serialValue, 3, 1, Qt::AlignTop);

    dialog->mainLayout()->addSpacing(10);
    // 正常的按钮
    DPushButton *okBtn = new DPushButton(tr("OK"), this);
    dialog->mainLayout()->addWidget(okBtn);

    dialog->show();
    connect(okBtn, &DPushButton::clicked, dialog, &UsbInfoDialog::deleteLater);
}

UsbTableNameItemDelegate::UsbTableNameItemDelegate(UsbTableType tableType, QObject *parent)
    : QItemDelegate(parent)
    , m_tableType(tableType)
    , m_nameItemCol(0)
    , m_dataItemCol(0)
{
    if (UsbTableType::Record == m_tableType) {
        m_nameItemCol = USB_LOG_ITEM_NAME_COL;
        m_dataItemCol = USB_LOG_ITEM_DATA_COL;
    } else if (UsbTableType::Whitelist == m_tableType) {
        m_nameItemCol = USB_WHITELIST_ITEM_NAME_COL;
        m_dataItemCol = USB_WHITELIST_ITEM_DATA_COL;
    }
}

UsbTableNameItemDelegate::~UsbTableNameItemDelegate()
{
}

void UsbTableNameItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == m_nameItemCol) {
        // 获取名称
        QString name = index.model()->data(index.model()->index(index.row(), m_dataItemCol), DATA_ROLE_NAME).toString();

        // 绘画文字
        QRect rect = option.rect;
        int x = rect.x() + 20; // 左右边距
        int rectHeight = rect.height();
        int y = rect.y() + rectHeight / 2 + 5;
        int nameWidth = rect.width() - 20;

        QString sElidedName = painter->fontMetrics().elidedText(name, Qt::ElideRight, nameWidth, Qt::TextShowMnemonic);
        painter->drawText(x, y, sElidedName);
    }
}
