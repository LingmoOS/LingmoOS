// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprintersshowwindow.h"
#include "zdrivermanager.h"
#include "dpropertysetdlg.h"
#include "printerapplication.h"
#include "connectedtask.h"
#include "qtconvert.h"
#include "zjobmanager.h"
#include "uisourcestring.h"
#include "config.h"
#include "cupsconnectionfactory.h"
#include "common.h"

#include "printertestpagedialog.h"
#include "troubleshootdialog.h"
#include "ztroubleshoot_p.h"
#include "dprintersupplyshowdlg.h"
#include "switchwidget.h"
#include "advancedsharewidget.h"

#include <DDialog>
#include <DMessageBox>
#include <DImageButton>
#include <DSettingsDialog>
#include <DTitlebar>
#include <DApplication>
#include <DFloatingButton>
#include <DFrame>
#include <DBackgroundGroup>
#include <DErrorMessage>
#include <DApplicationHelper>
#include <DToolButton>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QDebug>
#include <QMenu>
#include <QLineEdit>
#include <QTimer>
#include <QCheckBox>
#include <QLineEdit>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QProcess>
#include <QNetworkInterface>
#include <QClipboard>

#define FAQDOCUMENT_MESSAGE QObject::tr("Help on adding and using printers")

static const QString canonRemove = "/opt/deepin/dde-printer/printer-drivers/cndrvcups-capt/canonremove";

static bool isCanonCaptPrinter(const QString &printerName)
{
    QString printerUri = getPrinterUri(printerName.toUtf8().data());
    if (printerUri.contains("ccp://")) {
        return true;
    }
    return false;
}

static QStringList getLocalIPv4Addresses()
{
    QStringList ipv4AddressesList;
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface interface, interfaceList) {
        if (interface.flags() & QNetworkInterface::IsLoopBack) {
            continue;
        }

        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        foreach (QNetworkAddressEntry entry, entryList) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                ipv4AddressesList.append(entry.ip().toString());
            }
        }
    }

    return ipv4AddressesList;
}

DPrintersShowWindow::DPrintersShowWindow(const QString &printerName, QWidget *parent)
    : DMainWindow(parent)
    , m_pSearchWindow(nullptr)
    , m_pSettingsDialog(nullptr)
    , m_CurPrinterName(printerName)
    , m_IsFirstShow(true)
{
    m_pPrinterManager = DPrinterManager::getInstance();

    initUI();

    initConnections();
}

DPrintersShowWindow::~DPrintersShowWindow()
{
    if (m_pSearchWindow)
        m_pSearchWindow->deleteLater();
    if (m_pSettingsDialog)
        m_pSettingsDialog->deleteLater();
}

void DPrintersShowWindow::initUI()
{
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    QMenu *pMenu = new QMenu();
    m_pSettings = new QAction(tr("Settings"));
    pMenu->addAction(m_pSettings);
    pMenu->setAccessibleName("menu_titleBar");
    titlebar()->setMenu(pMenu);
    titlebar()->setAccessibleName("titleBar_mainWindow");
    setFixedSize(942, 656);

    // 左边上面的控制栏
    QLabel *pLabel = new QLabel(tr("Printers"));
    pLabel->setFixedHeight(36);
    DFontSizeManager::instance()->bind(pLabel, DFontSizeManager::T5, int(QFont::DemiBold));
    pLabel->setAccessibleName("label_leftWidget");

    QHBoxLayout *pLeftTopHLayout = new QHBoxLayout();
    pLeftTopHLayout->addWidget(pLabel, 6, Qt::AlignLeft);
    pLeftTopHLayout->setContentsMargins(0, 0, 0, 0);
    // 打印机列表
    m_pPrinterListView = new PrinterListView(this);
    m_pPrinterModel = new QStandardItemModel(m_pPrinterListView);
    m_pPrinterListView->setTextElideMode(Qt::ElideRight);
    m_pPrinterListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pPrinterListView->setItemSpacing(10);
    m_pPrinterListView->setModel(m_pPrinterModel);
    m_pPrinterListView->setFocusPolicy(Qt::NoFocus);

    ItemDelegate *pItemDelegate = new ItemDelegate(m_pPrinterListView);
    m_pPrinterListView->setItemDelegate(pItemDelegate);
    m_pPrinterListView->setAccessibleName("printerList_mainWindow");

    /*添加隐藏加载控件*/
    m_pLoading = new DSpinner(m_pPrinterListView);
    m_pLoading->setAutoFillBackground(true);
    m_pLoading->setFixedSize(32, 32);
    m_pLoading->setVisible(false);
    m_pLoading->move(300 - 32 - 10, 10);
    m_pLoading->setAccessibleName("loader_printerList");

    // 列表的右键菜单
    m_pListViewMenu = new QMenu();
    m_pPrinterRename = new QAction(tr("Rename"), m_pListViewMenu);
    m_pDefaultAction = new QAction(tr("Set as default"), m_pListViewMenu);
    m_pDefaultAction->setObjectName("Default");
    m_pDefaultAction->setCheckable(true);
    m_pDeleteAction = new QAction(tr("Delete"), m_pListViewMenu);

    m_pListViewMenu->addAction(m_pPrinterRename);
    m_pListViewMenu->addAction(m_pDeleteAction);
    m_pListViewMenu->addSeparator();
    m_pListViewMenu->addAction(m_pDefaultAction);
    m_pListViewMenu->setAccessibleName("listView_printerList");

    m_pBtnAddPrinter = new QPushButton(tr("Add printer"));
    #ifdef DTKWIDGET_CLASS_DSizeMode
        m_pBtnAddPrinter->setFixedSize(DSizeModeHelper::element(QSize(300, 24), QSize(300, 36)));
        connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, [this]() {
            m_pBtnAddPrinter->setFixedSize(DSizeModeHelper::element(QSize(300, 24), QSize(300, 36)));
        });
    #else
        m_pBtnAddPrinter->setFixedSize(300, 36);
    #endif

    // 没有打印机时的提示
    m_pLeftTipLabel = new QLabel(tr("No Printers"));
    m_pLeftTipLabel->setVisible(false);
    m_pLeftTipLabel->setAccessibleName("tipLabel_leftWidget");
    QPalette pa = m_pLeftTipLabel->palette();
    QColor color = pa.color(QPalette::WindowText);
    pa.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), int(255 * 0.3)));
    m_pLeftTipLabel->setPalette(pa);
    DFontSizeManager::instance()->bind(m_pLeftTipLabel, DFontSizeManager::T5, QFont::DemiBold);
    // 左侧布局
    QVBoxLayout *pLeftVLayout = new QVBoxLayout();
    pLeftVLayout->addLayout(pLeftTopHLayout, 1);
    pLeftVLayout->addWidget(m_pPrinterListView, 4);
    pLeftVLayout->addWidget(m_pLeftTipLabel, 1, Qt::AlignCenter);
    pLeftVLayout->addWidget(m_pBtnAddPrinter, 0, Qt::AlignBottom);
    pLeftVLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *pLeftWidget = new QWidget(this);
    pLeftWidget->setFixedSize(300, 566);
    pLeftWidget->setLayout(pLeftVLayout);
    pLeftWidget->setAccessibleName("leftWidget_mainWindow");

    // 右侧上方
    QLabel *pLabelImage = new QLabel("");
    pLabelImage->setPixmap(QPixmap(":/images/printer_details.svg").scaled(QSize(128, 128), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pLabelImage->setContentsMargins(10, 5, 0, 0);
    m_pBtnDeletePrinter = new DToolButton(this);
    m_pBtnDeletePrinter->setIcon(QIcon::fromTheme("dp_delete_printer"));
    m_pBtnDeletePrinter->setToolTip(tr("Delete printer"));
    m_pBtnDeletePrinter->setIconSize(QSize(20, 20));
    m_pBtnDeletePrinter->setFixedSize(QSize(30, 30));
    m_pBtnDeletePrinter->setContentsMargins(0, 0, 0, 0);

    m_pLabelPrinterName = new QLabel("");
    DFontSizeManager::instance()->bind(m_pLabelPrinterName, DFontSizeManager::T5, QFont::Medium);

    QLabel *pLabelLocation = new QLabel(tr("Location:"));
    DFontSizeManager::instance()->bind(pLabelLocation, DFontSizeManager::T8, QFont::Medium);
    QFontMetrics fm(pLabelLocation->font());
    pLabelLocation->setFixedWidth(fm.width(pLabelLocation->text()) + 10);
    m_pLabelLocationShow = new QLabel(tr(""));
    DFontSizeManager::instance()->bind(m_pLabelLocationShow, DFontSizeManager::T8, QFont::Medium);
    m_pLabelLocationShow->setFixedWidth(255);
    QLabel *pLabelType = new QLabel(tr("Model:"));
    m_pLabelTypeShow = new QLabel(tr(""));
    m_pLabelTypeShow->setFixedWidth(255);
    DFontSizeManager::instance()->bind(pLabelType, DFontSizeManager::T8, QFont::Medium);
    DFontSizeManager::instance()->bind(m_pLabelTypeShow, DFontSizeManager::T8, QFont::Medium);
    QLabel *pLabelStatus = new QLabel(tr("Status:"));
    m_pLabelStatusShow = new QLabel(tr(""));
    DFontSizeManager::instance()->bind(pLabelStatus, DFontSizeManager::T8, QFont::Medium);
    DFontSizeManager::instance()->bind(m_pLabelStatusShow, DFontSizeManager::T8, QFont::Medium);

    QGridLayout *pRightGridLayout = new QGridLayout();

    // 添加AT测试控件标签
    pLabelImage->setAccessibleName("labelImage_printerInfoWidget");
    m_pLabelPrinterName->setAccessibleName("printerName_printerInfoWidget");
    m_pLabelLocationShow->setAccessibleName("locationShow_printerInfoWidget");
    m_pLabelTypeShow->setAccessibleName("typeShow_printerInfoWidget");
    m_pLabelStatusShow->setAccessibleName("statusShow_printerInfoWidget");

    QHBoxLayout *pDeleteTopHLayout = new QHBoxLayout();
    pDeleteTopHLayout->addStretch();
    pDeleteTopHLayout->addWidget(m_pBtnDeletePrinter);
    pDeleteTopHLayout->setContentsMargins(0, 0, 0, 0);

    m_pLabelPrinterName->setFixedHeight(25);
    m_pLabelPrinterName->setContentsMargins(0, 0, 0, 0);
    pRightGridLayout->addWidget(pLabelLocation, 0, 0, Qt::AlignLeft);
    pRightGridLayout->addWidget(m_pLabelLocationShow, 0, 1);
    pRightGridLayout->addWidget(pLabelType, 1, 0, Qt::AlignLeft);
    pRightGridLayout->addWidget(m_pLabelTypeShow, 1, 1);
    pRightGridLayout->addWidget(pLabelStatus, 2, 0, Qt::AlignLeft);
    pRightGridLayout->addWidget(m_pLabelStatusShow, 2, 1);
    pRightGridLayout->setHorizontalSpacing(0);
    pRightGridLayout->setVerticalSpacing(10);
    pRightGridLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *pRightTopVLayout = new QVBoxLayout();
    pRightTopVLayout->addLayout(pDeleteTopHLayout);
    pRightTopVLayout->addWidget(m_pLabelPrinterName);
    pRightTopVLayout->addSpacing(10);
    pRightTopVLayout->addLayout(pRightGridLayout);
    pRightTopVLayout->addStretch();
    pRightTopVLayout->setSpacing(0);
    pRightTopVLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *pPrinterInfoWidget = new QWidget(this);
    pPrinterInfoWidget->setLayout(pRightTopVLayout);
    pPrinterInfoWidget->setFixedWidth(322);
    pPrinterInfoWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *pRightTopHLayout = new QHBoxLayout();
    pRightTopHLayout->addWidget(pLabelImage, 0, Qt::AlignHCenter | Qt::AlignLeft);
    pRightTopHLayout->addSpacing(10);
    pRightTopHLayout->addWidget(pPrinterInfoWidget);
    pRightTopHLayout->setContentsMargins(0, 0, 0, 0);

    m_pRightTopWidget = new BackgroundWidget(this);
    m_pRightTopWidget->setFixedSize(480, 168);
    m_pRightTopWidget->setLayout(pRightTopHLayout);

    // 右侧中部控件
    m_pDefaultPrinter = new QCheckBox();
    m_pDefaultPrinter->setText(tr("Default printer"));
    DFontSizeManager::instance()->bind(m_pDefaultPrinter, DFontSizeManager::T8);
    QHBoxLayout *phLayoutDefaultPrinter = new QHBoxLayout();
    phLayoutDefaultPrinter->addWidget(m_pDefaultPrinter);
    phLayoutDefaultPrinter->setContentsMargins(0, 0, 0, 0);

    m_pSwitchShareButton = new SwitchWidget(tr("Shared printer"), this);
    m_pAdvancedshare = new AdvanceShareWidget();
    m_pAdvancedshare->setLabelText(tr("Advanced sharing options"));
    QLabel *shareIpInfo = new QLabel(tr("Shared address:"));
    DFontSizeManager::instance()->bind(shareIpInfo, DFontSizeManager::T8, QFont::Light);
    QStringList ipv4Addr = getLocalIPv4Addresses();
    m_pShareIpAddr = new QLabel();
    m_pShareIpAddr->setText(ipv4Addr.join(" "));
    DFontSizeManager::instance()->bind(m_pShareIpAddr, DFontSizeManager::T8, QFont::Light);
    m_pShareCopyIpAddr = new ClickableLabel(tr("Copy"));
    QPalette pal = m_pShareCopyIpAddr->palette();
    pal.setColor(QPalette::WindowText, QColor("#0082FA"));
    m_pShareCopyIpAddr->setPalette(pal);
    DFontSizeManager::instance()->bind(m_pShareCopyIpAddr, DFontSizeManager::T8, QFont::Light);

    m_pShareWidget = new QWidget();
    m_pShareWidget->setVisible(true);
    m_pShareWidget->setContentsMargins(0, 0, 0, 0);

    m_pAdminTool = new AdvanceShareWidget();
    m_pAdminTool->setLabelText(tr("Management Tools"));
    m_pAdminTool->setFixedWidth(480);

    QHBoxLayout *phAdminLayout = new QHBoxLayout();
    phAdminLayout->addSpacing(10);
    phAdminLayout->addWidget(m_pAdminTool);

    QHBoxLayout *pSharehLayout = new QHBoxLayout();
    pSharehLayout->setSpacing(10);
    pSharehLayout->addWidget(shareIpInfo, 0);
    pSharehLayout->addWidget(m_pShareIpAddr, 0);
    pSharehLayout->addWidget(m_pShareCopyIpAddr, 1);
    pSharehLayout->setContentsMargins(10, 0, 0, 0);

    m_pShareWidget->setLayout(pSharehLayout);

    QVBoxLayout *pRightShareLayout = new QVBoxLayout();
    pRightShareLayout->addWidget(m_pSwitchShareButton);
    pRightShareLayout->addWidget(m_pAdvancedshare);
    pRightShareLayout->addWidget(m_pShareWidget);
    pRightShareLayout->addStretch();
    pRightShareLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *pRightShareWidget = new QWidget(this);
    pRightShareWidget->setMinimumSize(480, 105);
    pRightShareWidget->setMaximumSize(480, 120);
    pRightShareWidget->setLayout(pRightShareLayout);

    // 右侧下方控件
    m_pIBtnSetting = new DIconButton(this);
    m_pIBtnSetting->setIcon(QIcon::fromTheme("dp_set"));
    m_pIBtnSetting->setIconSize(QSize(24, 24));
    m_pIBtnSetting->setFixedSize(60, 60);
    m_pIBtnSetting->setEnabledCircle(true);
    QLabel *pLabelSetting = new QLabel();
    pLabelSetting->setText(tr("Properties"));
    pLabelSetting->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(pLabelSetting, DFontSizeManager::T8);

    m_pIBtnPrintQueue = new DIconButton(this);
    m_pIBtnPrintQueue->setIcon(QIcon::fromTheme("dp_print_queue"));
    m_pIBtnPrintQueue->setIconSize(QSize(24, 24));
    m_pIBtnPrintQueue->setFixedSize(60, 60);
    m_pIBtnPrintQueue->setEnabledCircle(true);
    m_pLabelPrintQueue = new QLabel();
    m_pLabelPrintQueue->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(m_pLabelPrintQueue, DFontSizeManager::T8);

    m_pIBtnPrintTest = new DIconButton(this);
    m_pIBtnPrintTest->setIcon(QIcon::fromTheme("dp_test_page"));
    m_pIBtnPrintTest->setIconSize(QSize(24, 24));
    m_pIBtnPrintTest->setFixedSize(60, 60);
    m_pIBtnPrintTest->setEnabledCircle(true);
    m_pLabelPrintTest = new QLabel();
    m_pLabelPrintTest->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(m_pLabelPrintTest, DFontSizeManager::T8);

    m_pIBtnFault = new DIconButton(this);
    m_pIBtnFault->setIcon(QIcon::fromTheme("dp_fault"));
    m_pIBtnFault->setIconSize(QSize(24, 24));
    m_pIBtnFault->setFixedSize(60, 60);
    m_pIBtnFault->setEnabledCircle(true);
    m_pLabelPrintFault = new QLabel();
    m_pLabelPrintFault->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(m_pLabelPrintFault, DFontSizeManager::T8);

    m_pIBtnSupply = new DIconButton(this);
    m_pIBtnSupply->setIcon(QIcon::fromTheme("filter_icon_unknown"));
    m_pIBtnSupply->setIconSize(QSize(24, 24));
    m_pIBtnSupply->setFixedSize(60, 60);
    m_pIBtnSupply->setEnabledCircle(true);
    QLabel *pLabelSupply = new QLabel;
    pLabelSupply->setText(tr("Supplies"));
    pLabelSupply->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(pLabelSupply, DFontSizeManager::T8);

    m_pIBtnSetting->setAccessibleName("setting_mainWindow");
    m_pIBtnPrintQueue->setAccessibleName("printQueue_mainWindow");
    m_pIBtnPrintTest->setAccessibleName("printTest_mainWindow");
    m_pIBtnSupply->setAccessibleName("supply_mainWindow");
    m_pIBtnFault->setAccessibleName("fault_mainWindow");
    pLabelSetting->setAccessibleName("labelSetting_printerInfoWidget");
    m_pLabelPrintQueue->setAccessibleName("labelPrintQueue_printerInfoWidget");
    m_pLabelPrintTest->setAccessibleName("labelPrintTest_printerInfoWidget");
    pLabelSupply->setAccessibleName("labelSupply_printerInfoWidget");
    m_pLabelPrintFault->setAccessibleName("labelFault_printerInfoWidget");

    QGridLayout *pRightBottomGLayout = new QGridLayout();
    pRightBottomGLayout->setContentsMargins(0, 0, 0, 0);
    pRightBottomGLayout->addWidget(m_pIBtnSetting, 0, 0, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(pLabelSetting, 1, 0);
    pRightBottomGLayout->addWidget(m_pIBtnPrintQueue, 0, 1, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(m_pLabelPrintQueue, 1, 1);
    pRightBottomGLayout->addWidget(m_pIBtnPrintTest, 0, 2, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(m_pLabelPrintTest, 1, 2);
    pRightBottomGLayout->addWidget(m_pIBtnSupply, 0, 3, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(pLabelSupply, 1, 3);
    pRightBottomGLayout->addWidget(m_pIBtnFault, 0, 4, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(m_pLabelPrintFault, 1, 4);

    QWidget *pRightBottomWidget = new QWidget(this);
    pRightBottomWidget->setFixedSize(520, 85);
    pRightBottomWidget->setLayout(pRightBottomGLayout);
    pRightBottomWidget->setContentsMargins(0, 0, 0, 0);

    m_customLabel = new CustomLabel(this);

    // 右侧上中布局
    QVBoxLayout *pRightTmVLayout = new QVBoxLayout();
    pRightTmVLayout->addSpacing(18);
    pRightTmVLayout->addWidget(m_pRightTopWidget, Qt::AlignCenter);
    pRightTmVLayout->addSpacing(20);
    pRightTmVLayout->addLayout(phLayoutDefaultPrinter, Qt::AlignVCenter);
    pRightTmVLayout->addWidget(pRightShareWidget, Qt::AlignCenter);
    pRightTmVLayout->setContentsMargins(25, 0, 0, 0);

    // 右侧整体布局
    QVBoxLayout *pRightVLayout = new QVBoxLayout();

    pRightVLayout->addLayout(pRightTmVLayout);
    pRightVLayout->addLayout(phAdminLayout);
    pRightVLayout->addSpacing(25);
    pRightVLayout->addWidget(pRightBottomWidget, Qt::AlignCenter);
    pRightVLayout->addSpacing(37);
    pRightVLayout->setContentsMargins(0, 0, 0, 0);
    m_pPrinterInfoWidget = new QWidget();
    m_pPrinterInfoWidget->setLayout(pRightVLayout);
    m_pPrinterInfoWidget->setAccessibleName("printerInfoWidget_rightWidget");
    m_pPRightTipLabel1 = new QLabel(tr("No printer configured"));
    m_pPRightTipLabel1->setAlignment(Qt::AlignCenter);
    m_pPRightTipLabel1->setVisible(false);
    m_pPRightTipLabel1->setPalette(pa);
    m_pPRightTipLabel1->setAccessibleName("rightTipLabel1_rightWidget");
    DFontSizeManager::instance()->bind(m_pPRightTipLabel1, DFontSizeManager::T5, QFont::DemiBold);
    m_pPRightTipLabel2 = new QLabel(tr("Click + to add printers"));
    m_pPRightTipLabel2->setAlignment(Qt::AlignCenter);
    m_pPRightTipLabel2->setVisible(false);
    m_pPRightTipLabel2->setPalette(pa);
    m_pPRightTipLabel2->setAccessibleName("rightTipLabel2_rightWidget");
    DFontSizeManager::instance()->bind(m_pPRightTipLabel2, DFontSizeManager::T6, QFont::DemiBold);
    QVBoxLayout *pRightMainVLayout = new QVBoxLayout();
    pRightMainVLayout->addStretch();
    pRightMainVLayout->addWidget(m_pPrinterInfoWidget, 0, Qt::AlignCenter);
    pRightMainVLayout->addWidget(m_pPRightTipLabel1, 0, Qt::AlignCenter);
    pRightMainVLayout->addWidget(m_pPRightTipLabel2, 0, Qt::AlignCenter);
    pRightMainVLayout->addStretch();
    pRightMainVLayout->addWidget(m_customLabel, 0, Qt::AlignBottom | Qt::AlignRight);
    pRightMainVLayout->setContentsMargins(0, 0, 0, 0);
    //DFrame 会导致上面的QLabel显示颜色不正常
    QWidget *pRightWidgwt = new QWidget();
    pRightWidgwt->setFixedSize(580, 566);
    pRightWidgwt->setLayout(pRightMainVLayout);
    pRightWidgwt->setAccessibleName("rightWidget_centralWidget");
    pRightWidgwt->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *pMainHLayout = new QHBoxLayout();
    pMainHLayout->addWidget(pLeftWidget, 320);
    pMainHLayout->addWidget(pRightWidgwt, 600);

    //阴影分割布局控件
    DBackgroundGroup *pCentralWidget = new DBackgroundGroup();
    pCentralWidget->setLayout(pMainHLayout);
    pCentralWidget->setItemSpacing(2);
    pCentralWidget->setAccessibleName("centralWidget_mainWindow");
    QHBoxLayout *pMainLayout1 = new QHBoxLayout();
    pMainLayout1->addWidget(pCentralWidget);
    pMainLayout1->setContentsMargins(10, 10, 10, 10);
    QWidget *pCentralWidget1 = new QWidget();
    pCentralWidget1->setLayout(pMainLayout1);
    takeCentralWidget();
    setCentralWidget(pCentralWidget1);
}

void DPrintersShowWindow::setPrinterNameEditable(const QModelIndex &index)
{
    // 存在未完成的任务无法进入编辑状态
    if (!index.isValid())
        return;

    m_pPrinterListView->blockSignals(true);
    m_pPrinterModel->blockSignals(true);
    QStandardItem *pItem = m_pPrinterModel->itemFromIndex(index);
    // 判断当前打印机的状态
    if (m_pPrinterManager->hasUnfinishedJob(pItem->text())) {
        DDialog *pDialog = new DDialog();
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        QLabel *pMessage = new QLabel(tr("As print jobs are in process, you cannot rename the printer now, please try later"));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(tr("OK"));
        pDialog->exec();
        pDialog->deleteLater();
        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
    } else {
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
    }

    m_pPrinterListView->blockSignals(false);
    m_pPrinterModel->blockSignals(false);
}

void DPrintersShowWindow::initConnections()
{
    connect(m_pBtnAddPrinter, &DIconButton::clicked, this, &DPrintersShowWindow::addPrinterClickSlot);
    connect(m_pBtnDeletePrinter, &DIconButton::clicked, this, &DPrintersShowWindow::deletePrinterClickSlot);

    connect(m_pIBtnSetting, &DIconButton::clicked, this, &DPrintersShowWindow::printSettingClickSlot);
    connect(m_pIBtnPrintQueue, &DIconButton::clicked, this, &DPrintersShowWindow::printQueueClickSlot);
    connect(m_pIBtnPrintTest, &DIconButton::clicked, this, &DPrintersShowWindow::printTestClickSlot);
    connect(m_pIBtnSupply, &DIconButton::clicked, this, &DPrintersShowWindow::printSupplyClickSlot);
    connect(m_pIBtnFault, &DIconButton::clicked, this, &DPrintersShowWindow::printFalutClickSlot);

    connect(m_pPrinterListView, QOverload<const QModelIndex &>::of(&DListView::currentChanged), this, &DPrintersShowWindow::printerListWidgetItemChangedSlot);
    //    //此处修改文字和修改图标都会触发这个信号，导致bug，修改图标之前先屏蔽信号
    connect(m_pPrinterModel, &QStandardItemModel::itemChanged, this, &DPrintersShowWindow::renamePrinterSlot);
    connect(m_pPrinterListView, &DListView::customContextMenuRequested, this, &DPrintersShowWindow::contextMenuRequested);
    connect(m_pPrinterListView, &DListView::doubleClicked, this, &DPrintersShowWindow::setPrinterNameEditable);
    connect(m_pPrinterRename, &QAction::triggered, this, [this]() {
        QModelIndex index= m_pPrinterListView->currentIndex();
        if (!index.isValid()) {
            return;
        }

        m_pPrinterRename->blockSignals(true);
        setPrinterNameEditable(index);
        m_pPrinterListView->edit(index);
        m_pPrinterRename->blockSignals(false);
    });

    connect(m_pDefaultAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pDeleteAction, &QAction::triggered, this, &DPrintersShowWindow::deletePrinterClickSlot);
    connect(m_pSettings, &QAction::triggered, this, &DPrintersShowWindow::serverSettingsSlot);
    connect(m_pSwitchShareButton, &SwitchWidget::checkedChanged, this, [&](bool check) {
        QString printerName = m_pPrinterListView->currentIndex().data().toString();
        m_pPrinterManager->setPrinterShared(printerName, check);
        if (check && !(m_pPrinterManager->isSharePrintersEnabled() && m_pPrinterManager->isRemoteAnyEnabled())) {
            // 需要开启共享状态，先检查服务器配置是否开启，如果开启，直接退出。没有开启打开共享配置
            m_pPrinterManager->enableSharePrinters(true);
            m_pPrinterManager->enableRemoteAny(true);

            map<string, string> options;
            options.insert({CUPS_SERVER_SHARE_PRINTERS, "1"});
            options.insert({CUPS_SERVER_REMOTE_ANY, "1"});
            m_pPrinterManager->commit(options);
            if (!g_cupsConnection) {
                qCInfo(COMMONMOUDLE) << "Try to restart the cups service";
            }
        }

        QTimer::singleShot(1000, [=]() {
            printerListWidgetItemChangedSlot(QModelIndex());
        });
    });
    connect(m_pDefaultPrinter, &QCheckBox::stateChanged, this, [&](int state) {
        QString printerName = m_pPrinterListView->currentIndex().data().toString();
        if (state == Qt::Checked) {
            m_pPrinterManager->setPrinterDefault(printerName);
            m_pDefaultPrinter->setChecked(true);
            m_pDefaultPrinter->setEnabled(false);
        }
        updateDefaultPrinterIcon();
    });

    connect(m_pShareCopyIpAddr, &ClickableLabel::clicked, [this]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_pShareIpAddr->text());
    });

    connect(m_pAdvancedshare, &AdvanceShareWidget::clicked, this, &DPrintersShowWindow::serverSettingsSlot);
    connect(m_pAdminTool, &AdvanceShareWidget::clicked, this, &DPrintersShowWindow::showAdminToolsSlot);
}

void DPrintersShowWindow::showAdminToolsSlot()
{
    ManagementToolWidget toolWidget;
    toolWidget.exec();
}

void DPrintersShowWindow::showEvent(QShowEvent *event)
{
    DMainWindow::showEvent(event);

    QTimer::singleShot(10, this, [ = ]() {
        CheckCupsServer cups(this);
        if (!cups.isPass()) {
            DDialog dlg("", tr("CUPS server is not running, and can’t manage printers."));

            dlg.setIcon(QIcon(":/images/warning_logo.svg"));
            dlg.addButton(tr("OK"), true);
            dlg.setContentsMargins(10, 15, 10, 15);
            dlg.setModal(true);
            dlg.setMinimumSize(422, 202);
            dlg.setMaximumSize(422, 250);
            dlg.exec();
        } else {
            refreshPrinterListView(m_CurPrinterName);
        }
    });

    if (!m_IsFirstShow)
        return;
    m_IsFirstShow = false;
    QTimer::singleShot(1000, this, [ = ]() {

        /*第一次安装没有启动后台程序，需要手动启动*/
        QDBusInterface interface(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME, QDBusConnection::sessionBus());
        if (!interface.isValid()) {
            qCInfo(COMMONMOUDLE) << "start dde-printer-helper";
            QProcess proces;
            proces.startDetached("bash", QStringList() << "-c" << "dde-printer-helper");
        }

        //连接dbus信号
        if (!QDBusConnection::sessionBus().connect(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME,
                                                   "signalPrinterStateChanged", this, SLOT(printerStateChanged(QDBusMessage)))) {
            qCWarning(COMMONMOUDLE) << "connect to dbus signal(signalPrinterStateChanged) failed";
        }
        if (!QDBusConnection::sessionBus().connect(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME,
                                                   "deviceStatusChanged", this, SLOT(deviceStatusChanged(QDBusMessage)))) {
            qCWarning(COMMONMOUDLE) << "connect to dbus signal(deviceStatusChanged) failed";
        }
    });

}

void DPrintersShowWindow::selectPrinterByName(const QString &printerName)
{
    int rowCount = m_pPrinterModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        if (m_pPrinterModel->item(i)->text() == printerName) {
            m_pPrinterListView->setCurrentIndex(m_pPrinterModel->item(i)->index());
            m_CurPrinterName = printerName;
            return;
        }
    }
}

void DPrintersShowWindow::updateDefaultPrinterIcon()
{
    //防止触发itemChanged信号
    m_pPrinterListView->blockSignals(true);
    m_pPrinterModel->blockSignals(true);
    int count = m_pPrinterModel->rowCount();
    for (int i = 0; i < count; ++i) {
        if (m_pPrinterManager->isDefaultPrinter(m_pPrinterModel->item(i)->text())) {
            m_pPrinterModel->item(i)->setIcon(QIcon::fromTheme("dp_printer_default"));
        } else {
            m_pPrinterModel->item(i)->setIcon(QIcon::fromTheme("dp_printer_list"));
        }
    }
    m_pPrinterListView->blockSignals(false);
    m_pPrinterModel->blockSignals(false);
}

QIcon DPrintersShowWindow::getSupplyIconByLevel(int iLevel)
{
    QIcon icon;

    if (iLevel == 0) {
        icon = QIcon::fromTheme("filter_icon_normal6");
    } else if (iLevel <= 20) {
        icon = QIcon::fromTheme("filter_icon_normal5");
    } else if (iLevel <= 40) {
        icon = QIcon::fromTheme("filter_icon_normal4");
    } else if (iLevel <= 60) {
        icon = QIcon::fromTheme("filter_icon_normal3");
    } else if (iLevel <= 80) {
        icon = QIcon::fromTheme("filter_icon_normal2");
    } else if (iLevel <= 100) {
        icon = QIcon::fromTheme("filter_icon_normal1");
    } else {
        icon = QIcon::fromTheme("filter_icon_unknown");
    }

    return icon;
}

void DPrintersShowWindow::refreshPrinterListView(const QString &newPrinterName)
{
    m_pPrinterManager->updateDestinationList();
    m_pPrinterModel->clear();
    QStringList printerList = m_pPrinterManager->getPrintersList();
    /*先插入正在配置的打印机列表，无法点击*/
    foreach (const QString &name, m_ConfigingPrinterNameList) {
        DStandardItem *pItem = new DStandardItem(name);
        pItem->setData(VListViewItemMargin, Dtk::MarginsRole);
        pItem->setSizeHint(QSize(300, 50));
        pItem->setToolTip(name);
        pItem->setIcon(QIcon::fromTheme("dp_printer_list"));
        pItem->setFlags(pItem->flags() & ~Qt::ItemFlag::ItemIsEnabled);
        m_pPrinterModel->appendRow(pItem);
    }
    if (m_ConfigingPrinterNameList.count() > 0) {
        m_pLoading->setVisible(true);
        m_pLoading->start();
    } else {
        m_pLoading->setVisible(false);
        m_pLoading->stop();
    }

    foreach (const QString &printerName, printerList) {
        DStandardItem *pItem = new DStandardItem(printerName);
        pItem->setData(VListViewItemMargin, Dtk::MarginsRole);
        pItem->setSizeHint(QSize(300, 50));
        pItem->setToolTip(printerName);
        if (m_pPrinterManager->isDefaultPrinter(printerName))
            pItem->setIcon(QIcon::fromTheme("dp_printer_default"));
        else {
            pItem->setIcon(QIcon::fromTheme("dp_printer_list"));
        }
        m_pPrinterModel->appendRow(pItem);
    }

    if (newPrinterName.isEmpty()) {
        m_pLabelPrinterName->setText("");
        m_pLabelLocationShow->setText("");
        m_pLabelTypeShow->setText("");
        m_pLabelStatusShow->setText("");
    }

    m_pPrinterListView->setIconSize(QSize(45, 45));
    if (m_pPrinterListView->count() > 0) {
        m_pPrinterListView->setVisible(true);
        m_pLeftTipLabel->setVisible(false);

        m_pPrinterInfoWidget->setVisible(true);
        m_pPRightTipLabel1->setVisible(false);
        m_pPRightTipLabel2->setVisible(false);

        m_pBtnDeletePrinter->setEnabled(true);
    } else {
        m_pPrinterListView->setVisible(false);
        m_pLeftTipLabel->setVisible(true);
        m_pPrinterInfoWidget->setVisible(false);
        m_pPRightTipLabel1->setVisible(true);
        m_pPRightTipLabel2->setVisible(true);
        m_pBtnDeletePrinter->setEnabled(false);
    }

    if (newPrinterName.isEmpty()) {
        if (m_pPrinterListView->count() > 0) {
            m_pPrinterListView->setCurrentIndex(m_pPrinterModel->index(0, 0));
        }
    } else {
        selectPrinterByName(newPrinterName);
    }

    QTimer::singleShot(1000,  this, [ = ]() {
        RefreshSnmpBackendTask *task = new RefreshSnmpBackendTask;
        connect(task, &RefreshSnmpBackendTask::refreshsnmpfinished,
                this, &DPrintersShowWindow::supplyFreshed);
        connect(task, &RefreshSnmpBackendTask::finished, this, [ = ]() {
            task->deleteLater();
        });
        task->setPrinters(printerList);
        task->beginTask();
    });
}

void DPrintersShowWindow::serverSettingsSlot()
{
    //设置对话框
    if (!m_pSettingsDialog) {
        m_pSettingsDialog = new ServerSettingsWindow(this);
    }

    connect(m_pSettingsDialog, &ServerSettingsWindow::finished, this, [this]() {
        QTimer::singleShot(1000, [=]() {
            printerListWidgetItemChangedSlot(QModelIndex());
        });
    });

    /*打开设置界面之前先更新设置接口数据，避免外部程序修改了cups设置，导致数据不同步*/
    if (!m_pPrinterManager->updateServerSetting())
        return;
    if (m_pPrinterManager->isSharePrintersEnabled()) {
        m_pSettingsDialog->m_pCheckShared->setChecked(true);
        m_pSettingsDialog->m_pCheckIPP->setChecked(m_pPrinterManager->isRemoteAnyEnabled());
        m_pSettingsDialog->m_pCheckIPP->setEnabled(true);
    } else {
        m_pSettingsDialog->m_pCheckShared->setChecked(false);
        m_pSettingsDialog->m_pCheckIPP->setChecked(false);
        m_pSettingsDialog->m_pCheckIPP->setEnabled(false);
    }
    m_pSettingsDialog->m_pCheckRemote->setChecked(m_pPrinterManager->isRemoteAdminEnabled());
    m_pSettingsDialog->m_pCheckSaveDebugInfo->setChecked(m_pPrinterManager->isDebugLoggingEnabled());
    m_pSettingsDialog->exec();
    /*找出变化的选项进行设置*/
    map<string, string> options;
    if (m_pPrinterManager->isSharePrintersEnabled() != m_pSettingsDialog->m_pCheckShared->isChecked()) {
        options.insert({CUPS_SERVER_SHARE_PRINTERS, m_pSettingsDialog->m_pCheckShared->isChecked() ? "1" : "0"});
    }
    if (m_pPrinterManager->isRemoteAnyEnabled() != m_pSettingsDialog->m_pCheckIPP->isChecked()) {
        options.insert({CUPS_SERVER_REMOTE_ANY, m_pSettingsDialog->m_pCheckIPP->isChecked() ? "1" : "0"});
    }
    if (m_pPrinterManager->isRemoteAdminEnabled() != m_pSettingsDialog->m_pCheckRemote->isChecked()) {
        options.insert({CUPS_SERVER_REMOTE_ADMIN, m_pSettingsDialog->m_pCheckRemote->isChecked() ? "1" : "0"});
    }
    if (m_pPrinterManager->isDebugLoggingEnabled() != m_pSettingsDialog->m_pCheckSaveDebugInfo->isChecked()) {
        options.insert({CUPS_SERVER_DEBUG_LOGGING, m_pSettingsDialog->m_pCheckSaveDebugInfo->isChecked() ? "1" : "0"});
    }

    if (m_pSettingsDialog->m_pCheckShared->isChecked()) {
        m_pPrinterManager->enableSharePrinters(true);
        m_pPrinterManager->enableRemoteAny(m_pSettingsDialog->m_pCheckIPP->isChecked());
    } else {
        m_pPrinterManager->enableSharePrinters(false);
        m_pPrinterManager->enableRemoteAny(false);
    }
    m_pPrinterManager->enableRemoteAdmin(m_pSettingsDialog->m_pCheckRemote->isChecked());
    m_pPrinterManager->enableDebugLogging(m_pSettingsDialog->m_pCheckSaveDebugInfo->isChecked());
    if (options.size() > 0) {
        m_pPrinterManager->commit(options);
        //触发本地cups服务启动
        if (!g_cupsConnection) {
            qCInfo(COMMONMOUDLE) << "Try to restart the cups service";
        }
    }
}

void DPrintersShowWindow::closeEvent(QCloseEvent *event)
{
    DMainWindow::closeEvent(event);

    QTimer::singleShot(10, g_printerApplication, &PrinterApplication::slotMainWindowClosed);
}

void DPrintersShowWindow::resizeEvent(QResizeEvent *event)
{
    /*界面缩放时动态调整打印机信息显示的宽度*/
    printerListWidgetItemChangedSlot(QModelIndex());

    /* 20号字体下调整界面图标宽度 */
    QString printQueue = tr("Print Queue");
    geteElidedText(m_pLabelPrintQueue->font(), printQueue, m_pLabelPrintQueue->width() > 600 ? 100 : m_pLabelPrintQueue->width() - 15);
    m_pLabelPrintQueue->setToolTip(tr("Print Queue"));
    m_pLabelPrintQueue->setText(printQueue);

    QString testPage = tr("Print Test Page");
    geteElidedText(m_pLabelPrintTest->font(), testPage, m_pLabelPrintTest->width() > 600 ? 100 : m_pLabelPrintTest->width() - 37);
    m_pLabelPrintTest->setToolTip(tr("Print Test Page"));
    m_pLabelPrintTest->setText(testPage);

    QString printFault = UI_PRINTERSHOW_TROUBLE;
    geteElidedText(m_pLabelPrintFault->font(), printFault, m_pLabelPrintFault->width() > 600 ? 100 : m_pLabelPrintFault->width() - 37);
    m_pLabelPrintFault->setToolTip(UI_PRINTERSHOW_TROUBLE);
    m_pLabelPrintFault->setText(printFault);
    DMainWindow::resizeEvent(event);
}
void DPrintersShowWindow::printerStateChanged(const QDBusMessage &msg)
{
    if (msg.arguments().count() != 3) {
        qCWarning(COMMONMOUDLE) << "printerStateChanged dbus arguments error";
        return;
    }
    const QString printer = msg.arguments().at(0).toString();
    int state = msg.arguments().at(1).toInt();
    if (printer == m_CurPrinterName) {
        QString stateStr;
        if (state == 3) {
            stateStr = tr("Idle");
        } else if (state == 4) {
            stateStr = tr("Printing");
        } else {
            stateStr = tr("Disabled");
        }
        m_pLabelStatusShow->setText(stateStr);
    }
}

void DPrintersShowWindow::deviceStatusChanged(const QDBusMessage &msg)
{
    if (msg.arguments().count() != 2) {
        qCWarning(COMMONMOUDLE) << "deviceStatusChanged dbus arguments error";
        return;
    }
    const QString printer = msg.arguments().at(0).toString();
    int state = msg.arguments().at(1).toInt();
    if (state == 0) {
        if (!m_ConfigingPrinterNameList.contains(printer))
            m_ConfigingPrinterNameList.append(printer);
        refreshPrinterListView("");
    } else if (state == 1) {
        /*刷新打印机列表，选中配置成功的打印机*/
        if (m_ConfigingPrinterNameList.contains(printer))
            m_ConfigingPrinterNameList.removeOne(printer);
        refreshPrinterListView(printer);
    } else if (state == 2) {
        /*刷新打印机列表*/
        if (m_ConfigingPrinterNameList.contains(printer))
            m_ConfigingPrinterNameList.removeOne(printer);
        refreshPrinterListView(m_CurPrinterName);
    }
}

void DPrintersShowWindow::addPrinterClickSlot()
{
    if (!m_pSearchWindow) {
        m_pSearchWindow = new PrinterSearchWindow(this);
        connect(m_pSearchWindow, &PrinterSearchWindow::updatePrinterList, this, &DPrintersShowWindow::refreshPrinterListView);
    }
    m_pSearchWindow->show();
    (void)getCurrentTime(ADD_TIME);
}

void DPrintersShowWindow::deletePrinterClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString printerName = m_pPrinterListView->currentIndex().data().toString();
    if (!printerName.isEmpty()) {
        DDialog *pDialog = new DDialog(this);
        QLabel *pMessage = new QLabel(tr("Are you sure you want to delete the printer \"%1\" ?").arg(printerName));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(UI_PRINTERSHOW_CANCEL);
        pDialog->addSpacing(20);
        pDialog->addButton(tr("Delete"), true, DDialog::ButtonType::ButtonWarning);
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        int ret = pDialog->exec();
        if (ret > 0) {
            if (isCanonCaptPrinter(printerName)) { // 判断是否canon capt打印机
                QFile file(canonRemove);
                if (file.exists()) {
                    int bret = managerCanonPrinter("remove", QStringList{printerName});
                    if (bret != 0) {
                        return;
                    }
                }
            }

            bool suc = m_pPrinterManager->deletePrinterByName(printerName);
            if (suc) {
                // 刷新打印机列表
                m_CurPrinterName = "";
                refreshPrinterListView(m_CurPrinterName);
            }
        }
        pDialog->deleteLater();
    }
}

void DPrintersShowWindow::renamePrinterSlot(QStandardItem *pItem)
{
    //过滤掉空格
    if (!pItem)
        return;

    if (isCanonCaptPrinter(m_CurPrinterName)) { // ccp打印机不支持修改名称
        DDialog *pDialog = new DDialog();
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        QLabel *pMessage = new QLabel(tr("Canon capt printer does not support name modification."));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(tr("Confirm"));
        pDialog->exec();
        pDialog->deleteLater();

        m_pPrinterListView->blockSignals(true);
        m_pPrinterModel->blockSignals(true);
        pItem->setText(m_CurPrinterName);
        m_pPrinterListView->blockSignals(false);
        m_pPrinterModel->blockSignals(false);
        return;
    }

    QString newPrinterName = m_pPrinterManager->validataName(pItem->text());
    if (newPrinterName.isEmpty()) {
        m_pPrinterListView->blockSignals(true);
        m_pPrinterModel->blockSignals(true);
        pItem->setText(m_CurPrinterName);
        m_pPrinterListView->blockSignals(false);
        m_pPrinterModel->blockSignals(false);
        return;
    }
    if (m_pPrinterManager->hasSamePrinter(newPrinterName)) {
        DDialog *pDialog = new DDialog();
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        QLabel *pMessage = new QLabel(tr("The name already exists"));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(tr("OK"));
        pDialog->exec();
        pDialog->deleteLater();
        m_pPrinterListView->blockSignals(true);
        m_pPrinterModel->blockSignals(true);
        pItem->setText(m_CurPrinterName);
        m_pPrinterListView->blockSignals(false);
        m_pPrinterModel->blockSignals(false);
        return;
    }

    try {
        if (m_pPrinterManager->hasFinishedJob()) {
            DDialog *pDialog = new DDialog();
            pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
            QLabel *pMessage = new QLabel(tr("You will not be able to reprint the completed jobs if continue. Are you sure you want to rename the printer?"));
            pMessage->setWordWrap(true);
            pMessage->setAlignment(Qt::AlignCenter);
            pDialog->addContent(pMessage);
            pDialog->addButton(UI_PRINTERSHOW_CANCEL);
            int okIndex = pDialog->addButton(tr("Confirm"));

            int ret = pDialog->exec();
            pDialog->deleteLater();
            if (ret != okIndex) {
                //避免重复触发itemchanged信号
                m_pPrinterListView->blockSignals(true);
                m_pPrinterModel->blockSignals(true);
                pItem->setText(m_CurPrinterName);
                m_pPrinterListView->blockSignals(false);
                m_pPrinterModel->blockSignals(false);
                return;
            }
        }
        QString info, location, deviceURI, ppdFile;
        DDestination *pDest = m_pPrinterManager->getDestinationByName(m_CurPrinterName);
        if (!pDest)
            return;
        bool isDefault = m_pPrinterManager->isDefaultPrinter(m_CurPrinterName);
        bool isAccept = m_pPrinterManager->isPrinterAcceptJob(m_CurPrinterName);
        m_pPrinterManager->setPrinterEnabled(m_CurPrinterName, false);

        pDest->getPrinterInfo(info, location, deviceURI, ppdFile);
        //添加打印机失败
        if (!m_pPrinterManager->addPrinter(newPrinterName, info, location, deviceURI, ppdFile)) {
            m_pPrinterListView->blockSignals(true);
            m_pPrinterModel->blockSignals(true);
            pItem->setText(m_CurPrinterName);
            m_pPrinterListView->blockSignals(false);
            m_pPrinterModel->blockSignals(false);
            qCWarning(COMMONMOUDLE) << "add printer failed";
            return;
        }
        m_pPrinterManager->setPrinterEnabled(newPrinterName, true);
        if (isDefault)
            m_pPrinterManager->setPrinterDefault(newPrinterName);
        if (isAccept)
            m_pPrinterManager->setPrinterAcceptJob(newPrinterName, true);
        m_pPrinterManager->deletePrinterByName(m_CurPrinterName);

        m_CurPrinterName = newPrinterName;
        // 刷新完成选中重命名的打印机
        refreshPrinterListView(m_CurPrinterName);
    } catch (const std::runtime_error &e) {
        qCWarning(COMMONMOUDLE) << e.what();
    }
}

void DPrintersShowWindow::printSettingClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString strPrinterName = m_pPrinterListView->currentIndex().data().toString();
    DPropertySetDlg dlg(strPrinterName, this);

    if (dlg.isDriveBroken()) {
        DDialog dialog;
        dialog.setMinimumSize(400, 150);
        dialog.setMaximumSize(400, 230);
        dialog.setMessage(tr("The driver is damaged, please install it again."));
        dialog.addSpacing(10);
        dialog.addButton(UI_PRINTERSHOW_CANCEL);
        int iIndex = dialog.addButton(tr("Install Driver"));

        dialog.setIcon(QIcon(":/images/warning_logo.svg"));
        QAbstractButton *pBtn = dialog.getButton(iIndex);
        connect(pBtn, &QAbstractButton::clicked, this, &DPrintersShowWindow::addPrinterClickSlot);
        QPoint ptCen1 = this->geometry().center();
        QPoint ptCen2 = dialog.geometry().center();
        QPoint mvPhasor = ptCen1 - ptCen2;
        QRect dialogRc = dialog.geometry();
        dialogRc.moveCenter(dialogRc.center() + mvPhasor);
        dialog.setGeometry(dialogRc);
        dialog.exec();
        dialog.setAccessibleName("driveBroken_mainWindow");
    } else {
        dlg.initUI();
        dlg.initConnection();
        dlg.updateViews();
        dlg.moveToParentCenter();
        dlg.exec();
        this->printerListWidgetItemChangedSlot(m_pPrinterListView->currentIndex());
    }
}

void DPrintersShowWindow::printQueueClickSlot()
{
    g_printerApplication->showJobsWindow();
}

void DPrintersShowWindow::printTestClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    m_pIBtnPrintTest->blockSignals(true);
    QString printerName = m_pPrinterListView->currentIndex().data().toString();

    PrinterTestPageDialog *dlg = new PrinterTestPageDialog(printerName, this);
    connect(dlg, &PrinterTestPageDialog::signalFinished, this, [ = ]() {
        m_pIBtnPrintTest->blockSignals(false);
    });
    dlg->printTestPage();
}

void DPrintersShowWindow::printFalutClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString printerName = m_pPrinterListView->currentIndex().data(Qt::DisplayRole).toString();

    TroubleShootDialog dlg(printerName, this);
    dlg.setModal(true);
    dlg.exec();
}

void DPrintersShowWindow::printSupplyClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString strPrinterName = m_pPrinterListView->currentIndex().data().toString();
    RefreshSnmpBackendTask *task = new RefreshSnmpBackendTask;
    connect(task, &RefreshSnmpBackendTask::refreshsnmpfinished,
            this, &DPrintersShowWindow::supplyFreshed);
    connect(task, &RefreshSnmpBackendTask::finished, this, [ = ]() {
        task->deleteLater();
    });
    task->setPrinters(QStringList(strPrinterName));
    DPrinterSupplyShowDlg *dlg = new DPrinterSupplyShowDlg(task, this);
    dlg->setModal(true);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    dlg->show();
}

void DPrintersShowWindow::supplyFreshed(const QString &strName, bool bVal)
{
    Q_UNUSED(bVal);
    RefreshSnmpBackendTask *task = static_cast<RefreshSnmpBackendTask *>(sender());

    if (!m_pPrinterListView->currentIndex().isValid())
        return;

    DDestination *pDest = m_pPrinterManager->getDestinationByName(strName);
    if (pDest != nullptr) {
        if (PRINTER == pDest->getType()) {
            QVector<SUPPLYSDATA> vecSupplys = task->getSupplyData(strName);
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->SetSupplys(vecSupplys);

            QString printerName = m_pPrinterListView->currentIndex().data(Qt::DisplayRole).toString();

            if (printerName == strName) {
                int iMinValue = pPrinter->getMinMarkerLevel();
                QIcon pix = getSupplyIconByLevel(iMinValue);
                m_pIBtnSupply->setIcon(pix);
                m_pIBtnSupply->update();
            }
        }
    }
}

void DPrintersShowWindow::printerListWidgetItemChangedSlot(const QModelIndex &previous)
{
    Q_UNUSED(previous)
    //在清空QListWidget的时候会触发这个槽函数，需要先判断是否这个item存在，不然会导致段错误
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString printerName = m_pPrinterListView->currentIndex().data(Qt::DisplayRole).toString();
    m_CurPrinterName = printerName;
    QStringList basePrinterInfo = m_pPrinterManager->getPrinterBaseInfoByName(printerName);
    if (basePrinterInfo.count() == 3) {
        QString showPrinter = printerName;
        //如果文字超出了显示范围，那么就用省略号代替，并且设置tip提示
        geteElidedText(m_pLabelPrinterName->font(), showPrinter, 60 + m_pLabelLocationShow->width());
        m_pLabelPrinterName->setText(showPrinter);
        m_pLabelPrinterName->setToolTip(printerName);

        // 共享状态应该为打印机状态与cups服务共享状态相与结果
        m_pPrinterManager->updateServerSetting();
        bool isShared = m_pPrinterManager->isPrinterShared(printerName) && m_pPrinterManager->isSharePrintersEnabled() && m_pPrinterManager->isRemoteAnyEnabled();
        m_pSwitchShareButton->setChecked(isShared);
        m_pShareWidget->setVisible(isShared);

        m_pDefaultPrinter->setEnabled(true);
        bool isDefault = m_pPrinterManager->isDefaultPrinter(printerName);
        if (isDefault) {
            m_pDefaultPrinter->setChecked(true);
            m_pDefaultPrinter->setEnabled(false);
        } else {
            m_pDefaultPrinter->setChecked(false);
        }

        QString location = QObject::tr(basePrinterInfo.at(0).toUtf8());
        m_pLabelLocationShow->setToolTip(location);
        geteElidedText(m_pLabelLocationShow->font(), location, m_pLabelLocationShow->width() - 20);
        m_pLabelLocationShow->setText(location);

        QString model = basePrinterInfo.at(1);
        geteElidedText(m_pLabelTypeShow->font(), model, m_pLabelTypeShow->width() - 20);
        m_pLabelTypeShow->setText(model);
        m_pLabelTypeShow->setToolTip(basePrinterInfo.at(1));
        m_pLabelStatusShow->setText(basePrinterInfo.at(2));
        DDestination *pDest = m_pPrinterManager->getDestinationByName(printerName);

        if (pDest != nullptr) {
            if (PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                int iMinValue = pPrinter->getMinMarkerLevel();
                QIcon pix = getSupplyIconByLevel(iMinValue);
                m_pIBtnSupply->setIcon(pix);
            }
        }
    }
}

void DPrintersShowWindow::contextMenuRequested(const QPoint &point)
{
    if (m_pPrinterListView->currentIndex().isValid()) {
        QString printerName = m_pPrinterListView->currentIndex().data().toString();
        bool isDefault = m_pPrinterManager->isDefaultPrinter(printerName);
        m_pDefaultAction->setChecked(isDefault);
        // 当打印机为默认时，不让用户取消，避免出现没有默认打印机的状态
        m_pDefaultAction->setDisabled(isDefault);
        m_pListViewMenu->popup(mapToParent(point));
    }
}

void DPrintersShowWindow::listWidgetMenuActionSlot(bool checked)
{
    QString printerName = m_pPrinterListView->currentIndex().data().toString();
    if (sender()->objectName() == "Share") {
        m_pPrinterManager->setPrinterShared(printerName, checked);
    } else if (sender()->objectName() == "Enable") {
        m_pPrinterManager->setPrinterEnabled(printerName, checked);
        //更新打印机状态信息
        printerListWidgetItemChangedSlot(m_pPrinterListView->currentIndex());
    } else if (sender()->objectName() == "Default") {
        if (checked) {
            m_pPrinterManager->setPrinterDefault(printerName);
            updateDefaultPrinterIcon();
        }
    } else if (sender()->objectName() == "Accept") {
        m_pPrinterManager->setPrinterAcceptJob(printerName, checked);
    }
    printerListWidgetItemChangedSlot(QModelIndex());
}

void DPrintersShowWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        if (m_pPrinterListView->currentIndex().isValid()) {
            QString printerName = m_pPrinterListView->currentIndex().data(Qt::DisplayRole).toString();
            QStringList basePrinterInfo = m_pPrinterManager->getPrinterBaseInfoByName(printerName);
            if (basePrinterInfo.count() == 3) {
                QString location = QObject::tr(basePrinterInfo.at(0).toUtf8());
                geteElidedText(m_pLabelLocationShow->font(), location, m_pLabelLocationShow->width() - 20);
                m_pLabelLocationShow->setText(location);

                QString model = basePrinterInfo.at(1);
                geteElidedText(m_pLabelTypeShow->font(), model, m_pLabelTypeShow->width() - 20);
                m_pLabelTypeShow->setText(model);
            }
        }
        QWidget::changeEvent(event);
    }
}

BackgroundWidget::BackgroundWidget(QWidget *parent)
{
    Q_UNUSED(parent)
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const DPalette &dp = DApplicationHelper::instance()->palette(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));
    p.drawRoundedRect(rect(), 8, 8);

    return QWidget::paintEvent(event);
}

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

ItemDelegate::~ItemDelegate()
{
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(parent)
    if (!index.isValid())
        return nullptr;
    QLineEdit *pLineEdit = new QLineEdit(parent);
    //打印机名称长度的限制是字节数128,unicode中文占3个字节
    pLineEdit->setMaxLength(40);
    return pLineEdit;
}
