// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netcontrolwidget.h"
#include "../src/widgets/tipwidget.h"
#include "window/modules/common/common.h"
#include "window/modules/common/compixmap.h"
#include "window/modules/common/gsettingkey.h"
#include "window/modules/common/invokers/invokerinterface.h"

#include <DMessageManager>
#include <DFrame>
#include <DWidget>
#include <DDialog>
#include <DLabel>
#include <DFontSizeManager>
#include <DTableView>
#include <DPinyin>
#include <DStandardItem>
#include <DComboBox>
#include <DStyle>
#include <DPalette>
#include <DFloatingMessage>

#include <QStandardItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QTimer>

#define LEFT_INTEVAL 10
#define EMPTY_STRING "     "
NetControlWidget::NetControlWidget(const QString &sPkgName, InvokerFactoryInterface *invokerFactory, QWidget *parent)
    : DFrame(parent)
    , m_tableView(nullptr)
    , m_tableModel(nullptr)
    , m_bIniFlag(true)
    , m_tableDelegate(new NetTableViewItemDelegate(this, this))
    , m_sLocatPkgName(sPkgName)
    , m_DataInterFaceServer(nullptr)
    , m_monitorInterFaceServer(nullptr)
    , m_gsetting(nullptr)
    , m_netTableWidget(nullptr)
    , m_lbTotal(nullptr)
    , m_isTipDialogFlag(false)
{
    this->setAccessibleName("rightWidget_netControlWidget");

    // 初始化安全中心gsetting配置
    m_DataInterFaceServer = invokerFactory->CreateInvoker("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", "com.deepin.defender.datainterface", ConnectType::SESSION, this);
    m_monitorInterFaceServer = invokerFactory->CreateInvoker("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", "com.deepin.defender.MonitorNetFlow", ConnectType::SYSTEM, this);
    m_gsetting = invokerFactory->CreateSettings("com.deepin.dde.deepin-defender", QByteArray(), this);
    // 初始化
    initUi();
    initData();

    // 初始化判断是否显示联网管控开关未开启提示
    intiSwitchStatus();
}

NetControlWidget::~NetControlWidget()
{
    m_tableModel->clear();
    m_tableModel->deleteLater();
    m_tableModel = nullptr;
}

// 初始化界面
void NetControlWidget::initUi()
{
    setBackgroundRole(QPalette::Window);
    setLineWidth(0);

    // 添加标题栏
    DLabel *lbTitle = new DLabel(tr("Internet Control"), this);
    lbTitle->setAccessibleName("netControlWidget_titleWidget");
    lbTitle->setContentsMargins(0, 2, 0, 0);
    QFont ft = lbTitle->font();
    // 字体剧中
    lbTitle->setAlignment(Qt::AlignLeft);
    ft.setBold(true);
    lbTitle->setFont(ft);
    DFontSizeManager::instance()->bind(lbTitle, DFontSizeManager::T5);

    // 添加统计栏
    m_lbTotal = new DTipLabel(tr("Allow: %1").arg(0) + EMPTY_STRING
                                  + tr("Ask: %1").arg(0) + EMPTY_STRING
                                  + tr("Disable: %1").arg(0) + EMPTY_STRING
                                  + tr("All: %1").arg(0),
                              this);
    m_lbTotal->setAccessibleName("netControlWidget_totalWidget");
    m_lbTotal->setContentsMargins(0, 0, 0, 0);
    m_lbTotal->setAlignment(Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_lbTotal, DFontSizeManager::T8);

    // 表格 - 模型 连接
    m_tableView = new DTableView(this);
    m_tableView->setAccessibleName("netTableFrame_tableView");
    m_tableModel = new QStandardItemModel(this);
    m_tableView->setModel(m_tableModel);
    m_tableView->setItemDelegate(m_tableDelegate);

    // 设置表格样式 (表头 网格线 排序)
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->horizontalHeader()->setStretchLastSection(true); //自适应调整宽度
    m_tableView->verticalHeader()->setHidden(true);
    m_tableView->setSortingEnabled(true);
    m_tableView->verticalHeader()->setDefaultSectionSize(48);
    m_tableView->horizontalHeader()->setVisible(true);
    m_tableView->setShowGrid(false);
    m_tableView->setFrameShape(QTableView::NoFrame);

    // 表格
    m_netTableWidget = new DefenderTable(this);
    m_netTableWidget->setAccessibleName("netControlWidget_netTableFrame");
    m_netTableWidget->setTableAndModel(m_tableView, m_tableModel);

    QVBoxLayout *layoutNetControl = new QVBoxLayout(this);
    layoutNetControl->addWidget(lbTitle);
    layoutNetControl->addWidget(m_lbTotal);
    layoutNetControl->addWidget(m_netTableWidget);
    layoutNetControl->setContentsMargins(0, 0, 0, 0);
    layoutNetControl->setSpacing(10);
    setLayout(layoutNetControl);
}

// 初始化数据
void NetControlWidget::initData()
{
    // 添加表格列
    QStandardItem *item_0 = new QStandardItem(tr("Name"));
    item_0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *item_1 = new QStandardItem(tr("Status"));
    item_1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_tableModel->setHorizontalHeaderItem(0, item_0);
    m_tableModel->setHorizontalHeaderItem(1, item_1);
    m_tableModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Icon")));
    m_tableModel->setHorizontalHeaderItem(3, new QStandardItem(QString("TypeName")));
    m_tableModel->setHorizontalHeaderItem(4, new QStandardItem(QString("RealNmae")));
    m_tableModel->setHorizontalHeaderItem(5, new QStandardItem(QString("PkgName")));

    // 注册
    registerDefenderProcInfoMetaType();
    registerDefenderProcInfoListMetaType();

    // 连接信号槽
    m_DataInterFaceServer->Connect("requestNetAppsChange", this, SLOT(doNetAppsInfo(DefenderProcInfoList)));

    // 获取联网管控所有APP信息
    m_DataInterFaceServer->Invoke("getNetAppsInfo", QList<QVariant>(), BlockMode::NOBLOCK);
}

// 初始化判断是否显示联网管控开关未开启提示
void NetControlWidget::intiSwitchStatus()
{
    // 联网管控总开关关闭时，提示用户请开启总开关
    if (!m_gsetting->GetValue(NET_CONTROL_ON_OFF).toBool()) {
        DFloatingMessage *floMsgSwitchOff = new DFloatingMessage(DFloatingMessage::TransientType);
        floMsgSwitchOff->setDuration(2000);
        floMsgSwitchOff->setIcon(QIcon::fromTheme(DIALOG_WARNING_TIP_RED));
        floMsgSwitchOff->setMessage(tr("Internet control is disabled, please turn it on in Protection"));
        DMessageManager::instance()->sendMessage(m_tableView, floMsgSwitchOff);
        DMessageManager::instance()->setContentMargens(m_tableView, QMargins(0, 0, 0, 20));
    }
}

// 接收所有APPS的详细信息
void NetControlWidget::doNetAppsInfo(DefenderProcInfoList sNetAppsInfos)
{
    // 表格初始化排序
    int nSortIndicatorSection = 0;
    Qt::SortOrder sSortIndicatorSort = Qt::AscendingOrder;
    if (!m_bIniFlag) {
        nSortIndicatorSection = m_tableView->horizontalHeader()->sortIndicatorSection();
        sSortIndicatorSort = m_tableView->horizontalHeader()->sortIndicatorOrder();
    }

    // 表格加载数据
    int nRow = 0;
    int nAskCount = 0;
    int nAllowCount = 0;
    int nDisableCount = 0;
    m_comboToAppName.clear();
    m_appNamefromPkgName.clear();
    for (DefenderProcInfo defenderprocinfo : sNetAppsInfos) {
        // 获取联网管控默认状态
        m_sDefaultStatus = defenderprocinfo.sDefaultStatus;

        // 加载表格数据
        QString sName = DTK_CORE_NAMESPACE::Chinese2Pinyin(defenderprocinfo.sAppName);
        m_tableModel->setItem(nRow, 0, new QStandardItem(sName));
        m_tableModel->setItem(nRow, 1, new QStandardItem(defenderprocinfo.sStatus));
        m_tableModel->setItem(nRow, 2, new QStandardItem(defenderprocinfo.sThemeIcon));
        m_tableModel->setItem(nRow, 3, new QStandardItem(defenderprocinfo.isbSysApp ? tr("System") : tr("Third-party")));
        m_tableModel->setItem(nRow, 4, new QStandardItem(defenderprocinfo.sAppName));
        m_tableModel->setItem(nRow, 5, new QStandardItem(defenderprocinfo.sPkgName));

        m_appNamefromPkgName[defenderprocinfo.sPkgName] = defenderprocinfo.sAppName;

        // 表格内加载下拉框
        DComboBox *box = new DComboBox(this);
        box->addItem(tr("Ask"), NET_STATUS_ITEMASK);
        box->addItem(tr("Allow"), NET_STATUS_ITEMALLOW);
        box->addItem(tr("Disable"), NET_STATUS_ITEMDISALLOW);

        QString sStatus = defenderprocinfo.sStatus;
        int nStatus = defenderprocinfo.sStatus.toInt();
        box->setCurrentIndex(nStatus);
        connect(box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &NetControlWidget::onAppOptChanged);

        // 统计询问/允许/禁止总数
        if (NET_STATUS_ITEMASK == nStatus) {
            ++nAskCount;
        } else if (NET_STATUS_ITEMALLOW == nStatus) {
            ++nAllowCount;
        } else {
            ++nDisableCount;
        }

        // 添加到布局 并将box插入容器保存
        DWidget *boxw = new DWidget(this);
        QHBoxLayout *boxLayout = new QHBoxLayout;
        boxLayout->setContentsMargins(6, 6, 20, 6);
        boxLayout->addWidget(box);
        boxw->setLayout(boxLayout);
        m_comboToAppName.insert(box, defenderprocinfo.sPkgName);
        m_tableView->setIndexWidget(m_tableModel->index(nRow, 1), boxw);
        ++nRow;
    }

    // 数据统计
    m_lbTotal->setText(tr("Allow: %1").arg(nAllowCount) + EMPTY_STRING + tr("Ask: %1").arg(nAskCount) + EMPTY_STRING + tr("Disable: %1").arg(nDisableCount) + EMPTY_STRING + tr("All: %1").arg(nRow));

    // 设置前两列列的宽度
    m_tableView->setColumnWidth(0, 530);
    m_tableView->setColumnWidth(1, 204);

    // 隐藏多余的列 及设置初始化 排序
    m_tableView->setColumnHidden(2, true);
    m_tableView->setColumnHidden(3, true);
    m_tableView->setColumnHidden(4, true);
    m_tableView->setColumnHidden(5, true);

    m_tableView->horizontalHeader()->setHighlightSections(false);
    if (!m_bIniFlag) {
        if (nSortIndicatorSection == 1) {
            m_tableView->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
        }

        //　再次改变表格数据后，还原改变之前的排序规则
        m_tableView->horizontalHeader()->setSortIndicator(nSortIndicatorSection, sSortIndicatorSort);
        m_tableModel->sort(nSortIndicatorSection, sSortIndicatorSort);
    } else {
        // 初始化排序　默认采用第一列升序
        m_tableView->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
        m_tableModel->sort(nSortIndicatorSection, sSortIndicatorSort);
        m_bIniFlag = false;
    }

    // 如果定位参数有值， 则直接定位到那行
    if (!m_sLocatPkgName.isEmpty()) {
        setNetControlIndex(m_sLocatPkgName);

        // 设置完包名定位，清除某个应用程序改变的包名
        m_sLocatPkgName.clear();
    }
}

// 表格内下拉框选项改变
void NetControlWidget::onAppOptChanged(int index)
{
    // 联网状态选择禁止，弹警告框
    if (NET_STATUS_ITEMDISALLOW == index && !m_isTipDialogFlag) {
        m_isTipDialogFlag = true;
        DFloatingMessage *floMsgDisable = new DFloatingMessage(DFloatingMessage::TransientType, this);
        floMsgDisable->setDuration(3000);
        floMsgDisable->setIcon(QIcon::fromTheme(DIALOG_WARNING_TIP_RED));
        floMsgDisable->setMessage(tr("Note: Disabling Internet access may cause unknown exception of applications"));
        DMessageManager::instance()->sendMessage(m_tableView, floMsgDisable);
        DMessageManager::instance()->setContentMargens(m_tableView, QMargins(0, 0, 0, 20));
    }

    // 获取改变的包名
    QComboBox *box = qobject_cast<QComboBox *>(sender());
    QString sPkgName = m_comboToAppName.value(box);

    // 某个应用程序状态改变，需要将包名保存，以防止排序后能够快速定位该应用程序
    m_sLocatPkgName = sPkgName;
    // 将包名以及对应的类型发送给服务
    m_DataInterFaceServer->Invoke("setNetAppStatusChange", QList<QVariant>() << sPkgName << index, BlockMode::NOBLOCK);

    // 添加安全日志
    if (NET_STATUS_ITEMASK == index) {
        m_monitorInterFaceServer->Invoke("AddSecurityLog", QList<QVariant>() << SECURITY_LOG_TYPE_TOOL << tr("Changed Internet connection of %1 to \"Ask\"").arg(m_appNamefromPkgName[sPkgName]), BlockMode::NOBLOCK);
    } else if (NET_STATUS_ITEMALLOW == index) {
        m_monitorInterFaceServer->Invoke("AddSecurityLog", QList<QVariant>() << SECURITY_LOG_TYPE_TOOL << tr("Changed Internet connection of %1 to \"Allow\"").arg(m_appNamefromPkgName[sPkgName]), BlockMode::NOBLOCK);
    } else {
        m_monitorInterFaceServer->Invoke("AddSecurityLog", QList<QVariant>() << SECURITY_LOG_TYPE_TOOL << tr("Changed Internet connection of %1 to \"Disable\"").arg(m_appNamefromPkgName[sPkgName]), BlockMode::NOBLOCK);
    }
}

// 设置表格选中行
void NetControlWidget::setNetControlIndex(const QString &sPkgName)
{
    m_sLocatPkgName = sPkgName;
    for (int i = 0; i < m_tableModel->rowCount(); ++i) {
        QModelIndex index = m_tableModel->index(i, 1);
        QStandardItem *item = m_tableModel->item(i, 5);
        // 获取表格中每行的 包名, 如果校验成功则切换到这行
        if (item->data(Qt::DisplayRole).toString() == sPkgName) {
            m_tableView->setCurrentIndex(index);
            break;
        }
    }
}

QPixmap NetControlWidget::getPixmap(const QString &appIcon, const QSize &size)
{
    QIcon icon = QIcon::fromTheme(appIcon, QIcon::fromTheme("application-x-desktop"));
    const qreal ratio = devicePixelRatioF();
    QPixmap pixmap = icon.pixmap(size * ratio).scaled(size * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}

// 表格代理内， 实现表格重绘
NetTableViewItemDelegate::NetTableViewItemDelegate(NetControlWidget *widget, QObject *parent)
    : QItemDelegate(parent)
    , m_widget(widget)
{
}

void NetTableViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int Pix_Inteval = 2;
    QRect rect = option.rect;
    // 第一列
    if (index.column() == 0) {
        // 设置图片
        int pix_x = rect.x() + 5;
        int pix_y = rect.y() + rect.height() / 2 - 14;
        int height = rect.height() - Pix_Inteval * 2;

        QString iconName = index.model()->data(index.model()->index(index.row(), 2), Qt::DisplayRole).toString();
        if (iconName.contains(".svg")) {
            // 绘画图片坐标
            QIcon icon = QIcon::fromTheme(iconName);
            QRect rectIcon(pix_x + LEFT_INTEVAL, pix_y, 28, 28);
            icon.paint(painter, rectIcon);
        } else {
            QPixmap pixmap = m_widget->getPixmap(iconName, QSize(28, 28));
            painter->drawPixmap(pix_x + LEFT_INTEVAL, pix_y, pixmap);
        }

        // 绘画文字的坐标
        painter->save();
        QFont ft = painter->font();
        int textWidth = rect.width() - height - Pix_Inteval * 2;
        int x = pix_x + 38;
        int y = pix_y + height / 4;
        QString sName = index.model()->data(index.model()->index(index.row(), 4), Qt::DisplayRole).toString();
        QString sElidedName = painter->fontMetrics().elidedText(sName, Qt::ElideRight, textWidth, Qt::TextShowMnemonic);
        painter->drawText(x + LEFT_INTEVAL, y, sElidedName);
        y += rect.height() / 2;
        ft.setPixelSize(12);
        painter->setFont(ft);
        painter->setPen("#526A7F");

        // 来源
        QString source = index.model()->data(index.model()->index(index.row(), 3), Qt::DisplayRole).toString();
        QString sElidedName1 = painter->fontMetrics().elidedText(source, Qt::ElideRight, textWidth, Qt::TextShowMnemonic);
        painter->drawText(x + LEFT_INTEVAL, y, sElidedName1);
        painter->restore();
    } else {
    }
}
