// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printersearchwindow.h"
#include "zdrivermanager.h"
#include "cupsattrnames.h"
#include "addprinter.h"
#include "common.h"
#include "uisourcestring.h"
#include "permissionswindow.h"
#include "printerservice.h"
#include "qtconvert.h"

#include <DIconButton>
#include <DComboBox>
#include <DListWidget>
#include <DSpinner>
#include <DWidgetUtil>
#include <DListView>
#include <DLineEdit>
#include <DTitlebar>
#include <DMessageManager>
#include <DDialog>
#include <DStandardItem>
#include <DFrame>
#include <DBackgroundGroup>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QStackedWidget>
#include <QRegExp>
#include <QRegularExpression>
#include <QScrollArea>

static const QString strHplipName = "com.hp.hplip";
static int netCount = 0;

PrinterSearchWindow::PrinterSearchWindow(QWidget *parent)
    : DMainWindow(parent)
{
    initUi();
    initConnections();
}

PrinterSearchWindow::~PrinterSearchWindow()
{
    if (m_pInstallDriverWindow)
        m_pInstallDriverWindow->deleteLater();
}

void PrinterSearchWindow::initUi()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
	titlebar()->setAccessibleName("titlebar_mainWindow");
    // 去掉最大最小按钮
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(682, 532);

    DFrame *pLeftWidget = new DFrame();
    //自动填充背景之后就去掉了DTK的圆角边框，变成了默认的直角边框（UI设计）
    pLeftWidget->setFrameShape(DFrame::Shape::NoFrame);
    pLeftWidget->setAutoFillBackground(true);
    // 左侧菜单列表
    m_pTabListView = new DListView(this);
    QStandardItemModel *pTabModel = new QStandardItemModel(m_pTabListView);
    m_pWidgetItemAuto = new DStandardItem(QIcon::fromTheme("dp_auto_searching"), tr("Discover printer"));
    m_pWidgetItemAuto->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pWidgetItemAuto->setToolTip(tr("Discover printer"));
    //把Item的颜色和view的颜色设置一致
    m_pWidgetItemAuto->setBackgroundRole(pLeftWidget->backgroundRole());
    m_pWidgetItemManual = new DStandardItem(QIcon::fromTheme("dp_manual_search"), tr("Find printer"));
    m_pWidgetItemManual->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pWidgetItemManual->setToolTip(tr("Find printer"));
    m_pWidgetItemManual->setBackgroundRole(pLeftWidget->backgroundRole());
    m_pWidgetItemURI = new DStandardItem(QIcon::fromTheme("dp_uri"), tr("Enter URI"));
    m_pWidgetItemURI->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pWidgetItemURI->setToolTip(tr("Enter URI"));
    m_pWidgetItemURI->setBackgroundRole(pLeftWidget->backgroundRole());

#ifdef DTKWIDGET_CLASS_DSizeMode
    m_pWidgetItemAuto->setSizeHint(DSizeModeHelper::element(QSize(108, 36),QSize(108, 48)));
    m_pWidgetItemManual->setSizeHint(DSizeModeHelper::element(QSize(108, 36),QSize(108, 48)));
    m_pWidgetItemURI->setSizeHint(DSizeModeHelper::element(QSize(108, 36),QSize(108, 48)));
#else
    m_pWidgetItemAuto->setSizeHint(QSize(108, 48));
    m_pWidgetItemManual->setSizeHint(QSize(108, 48));
    m_pWidgetItemURI->setSizeHint(QSize(108, 48));
#endif

    pTabModel->appendRow(m_pWidgetItemAuto);
    pTabModel->appendRow(m_pWidgetItemManual);
    pTabModel->appendRow(m_pWidgetItemURI);
    m_pTabListView->setModel(pTabModel);
    m_pTabListView->setCurrentIndex(pTabModel->index(0, 0));
    m_pTabListView->setFocusPolicy(Qt::NoFocus);
    m_pTabListView->setTextElideMode(Qt::TextElideMode::ElideRight);
    m_pTabListView->setEditTriggers(DListView::NoEditTriggers);
    m_pTabListView->setFixedWidth(128);
    m_pTabListView->setItemSpacing(0);
    m_pTabListView->setAccessibleName("listView_mainWindow");

    QVBoxLayout *pLeftVLayout = new QVBoxLayout();
    pLeftVLayout->addWidget(m_pTabListView);
    pLeftVLayout->setContentsMargins(10, 10, 10, 0);
    pLeftWidget->setFixedWidth(148);
    pLeftWidget->setLayout(pLeftVLayout);
    pLeftWidget->setAccessibleName("leftWidget_centralWidget");

    m_pStackedWidget = new QStackedWidget();
    // 右侧 自动查找
    m_pLabelPrinter = new QLabel(tr("Select a printer"));
    m_pLabelPrinter->setAccessibleName("label_autoFrame1");
    DFontSizeManager::instance()->bind(m_pLabelPrinter, DFontSizeManager::T5, QFont::DemiBold);

    m_pBtnRefresh = new DIconButton(this);
    m_pBtnRefresh->setIcon(QIcon::fromTheme("dp_refresh"));
    m_pBtnRefresh->setToolTip(tr("Refresh"));
    m_pBtnRefresh->setAccessibleName("btnRefresh_mainWindow");
    QHBoxLayout *pHLayout1 = new QHBoxLayout();
    pHLayout1->setSpacing(0);
    pHLayout1->addSpacing(8);
    pHLayout1->addWidget(m_pLabelPrinter);
    pHLayout1->addWidget(m_pBtnRefresh, 0, Qt::AlignRight);
    pHLayout1->setMargin(0);

    m_pPrinterListViewAuto = new DListView(this);
    m_pPrinterListViewAuto->setEditTriggers(DListView::NoEditTriggers);
    m_pPrinterListViewAuto->setTextElideMode(Qt::ElideRight);
    m_pPrinterListViewAuto->setSelectionMode(QAbstractItemView::NoSelection);
    m_pPrinterListViewAuto->setFocusPolicy(Qt::NoFocus);
    m_pPrinterListViewAuto->setItemSpacing(10);
    m_pPrinterListViewAuto->setIconSize(QSize(52, 52));

    m_pPrinterListModel = new QStandardItemModel(m_pPrinterListViewAuto);
    m_pPrinterListViewAuto->setModel(m_pPrinterListModel);
    m_pPrinterListViewAuto->setAccessibleName("listViewAuto_mainWindow");

    m_pInfoAuto = new QLabel(m_pPrinterListViewAuto);
    m_pInfoAuto->setVisible(false);
    m_pInfoAuto->setText(tr("No printers found"));
    m_pInfoAuto->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_pInfoAuto, DFontSizeManager::T5, int(QFont::DemiBold));
    QPalette pa = m_pInfoAuto->palette();
    QColor color = pa.color(QPalette::Inactive, QPalette::Text);
    pa.setColor(QPalette::Text, QColor(color.red(), color.green(), color.blue(), 150));
    m_pInfoAuto->setPalette(pa);
    m_pInfoAuto->setAccessibleName("infoAuto_listViewAuto");

    QString tipInfo = UI_PRINTER_DRIVER_MESSAGE;
    QString webLinkinfo = UI_PRINTER_DRIVER_WEBSITE;
    m_pAutoDriverWebLink = new QLabel();
    QFontMetrics fm(m_pAutoDriverWebLink->font());
    m_pAutoDriverWebLink->setContentsMargins(10, 0, 0, 0);
    m_pAutoDriverWebLink->setOpenExternalLinks(true);
    geteElidedText(m_pAutoDriverWebLink->font(), webLinkinfo, 480 - fm.width(tipInfo));
    QString webLink = QObject::tr(UI_PRINTER_DRIVER_WEB_LINK).arg(webLinkinfo);
    m_pAutoDriverWebLink->setToolTip(UI_PRINTER_DRIVER_MESSAGE + UI_PRINTER_DRIVER_WEBSITE);
    m_pAutoDriverWebLink->setText(tipInfo + webLink);
    m_pAutoDriverWebLink->setAccessibleName("autoWebLink_autoFrame1");

    QVBoxLayout *pVlayoutAuto1 = new QVBoxLayout();
    pVlayoutAuto1->addLayout(pHLayout1);
    pVlayoutAuto1->addWidget(m_pPrinterListViewAuto);
    pVlayoutAuto1->addWidget(m_pAutoDriverWebLink);
    pVlayoutAuto1->setContentsMargins(10, 10, 10, 10);
    QWidget *pAutoFrame1 = new QWidget();
    pAutoFrame1->setLayout(pVlayoutAuto1);
    pAutoFrame1->setAccessibleName("autoFrame1_autoWidget");

    QLabel *pLabelDriver1 = new QLabel(UI_PRINTERSEARCH_DRIVER);
    pLabelDriver1->setFixedWidth(46);
    pLabelDriver1->setAccessibleName("labelDriver_autoFrame2");
    m_pAutoDriverCom = new QComboBox();
    m_pAutoDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
    m_pAutoDriverCom->setAccessibleName("autoDriver_autoFrame2");
    QHBoxLayout *pHLayout2 = new QHBoxLayout();
    pHLayout2->setSpacing(0);
    pHLayout2->addWidget(pLabelDriver1);
    pHLayout2->addSpacing(14);
    pHLayout2->addWidget(m_pAutoDriverCom, 1);
    pHLayout2->setContentsMargins(0, 0, 0, 0);

    m_pAutoListSpinner = new DSpinner(m_pPrinterListViewAuto);
    m_pAutoListSpinner->setAccessibleName("autoSpinner_listViewAuto");
    m_pAutoSpinner = new DSpinner();
    m_pAutoSpinner->setFixedSize(36, 36);
    m_pAutoSpinner->setAccessibleName("autoSpinner_autoFrame2");
    m_pAutoInstallDriverBtn = new QPushButton(tr("Install Driver"));
    m_pAutoInstallDriverBtn->setEnabled(false);
    m_pAutoInstallDriverBtn->setFixedWidth(200);
    m_pAutoInstallDriverBtn->setAccessibleName("autoInstallBtn_autoFrame2");
    QHBoxLayout *pHLayout3 = new QHBoxLayout();
    pHLayout3->addStretch();
    pHLayout3->addWidget(m_pAutoInstallDriverBtn);
    pHLayout3->addStretch();
    pHLayout3->setContentsMargins(0, 0, 0, 0);

    m_phplipAutoTipLabel = new QLabel("");
    m_phplipAutoTipLabel->setFixedWidth(485);
    m_phplipAutoTipLabel->setWordWrap(true);
    m_phplipAutoTipLabel->setContentsMargins(60, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_phplipAutoTipLabel, DFontSizeManager::T8, QFont::Light);

    QWidget *pAutoFrame2 = new QWidget();
    QVBoxLayout *pVLayoutSub1 = new QVBoxLayout();
    pVLayoutSub1->setSpacing(0);
    pVLayoutSub1->addLayout(pHLayout2);
    pVLayoutSub1->addSpacing(4);
    pVLayoutSub1->addWidget(m_phplipAutoTipLabel);
    pVLayoutSub1->addSpacing(4);
    pVLayoutSub1->addWidget(m_pAutoSpinner, 0, Qt::AlignCenter);
    pVLayoutSub1->addLayout(pHLayout3);
    pVLayoutSub1->setContentsMargins(20, 10, 10, 10);
    pAutoFrame2->setLayout(pVLayoutSub1);
    pAutoFrame2->setAccessibleName("autoFrame2_autoWidget");

    QVBoxLayout *pVLayout1 = new QVBoxLayout();
    pVLayout1->setSpacing(2);
    pVLayout1->addWidget(pAutoFrame1);
    pVLayout1->addWidget(pAutoFrame2);
    pVLayout1->setMargin(0);
    DBackgroundGroup *pWidget1 = new DBackgroundGroup();
    pWidget1->setLayout(pVLayout1);
    pWidget1->setItemSpacing(2);
    pWidget1->setAccessibleName("autoWidget_stackedWidget");
    m_pStackedWidget->addWidget(pWidget1);
    // 右侧 手动查找
    m_pLabelLocation = new QLabel(tr("Address"));
    m_pLabelLocation->setAccessibleName("labelLocation_manFrame1");
    m_pLineEditLocation = new QLineEdit();
    m_pLineEditLocation->setPlaceholderText(tr("Please enter an IP address or hostname"));
    m_pLineEditLocation->setAccessibleName("lineEditLocation_manFrame1");
    m_pBtnFind = new QPushButton(tr("Find", "button"));
    m_pBtnFind->setAccessibleName("btnFind_manFrame1");
    QHBoxLayout *pHLayout4 = new QHBoxLayout();
    pHLayout4->setSpacing(0);
    pHLayout4->addWidget(m_pLabelLocation);
    pHLayout4->addSpacerItem(new QSpacerItem(20, 36, QSizePolicy::Preferred, QSizePolicy::Preferred));
    pHLayout4->addWidget(m_pLineEditLocation);
    pHLayout4->addSpacing(8);
    pHLayout4->addWidget(m_pBtnFind);
    pHLayout4->setContentsMargins(20, 20, 10, 20);
    QWidget *pManFrame1 = new QWidget();
    pManFrame1->setLayout(pHLayout4);
    pManFrame1->setAccessibleName("manFrame1_manWidget");

    m_pPrinterListViewManual = new DListView(this);
    m_pPrinterListViewManual->setEditTriggers(DListView::NoEditTriggers);
    m_pPrinterListViewManual->setTextElideMode(Qt::ElideRight);
    m_pPrinterListViewManual->setSelectionMode(QAbstractItemView::NoSelection);
    m_pPrinterListViewManual->setFocusPolicy(Qt::NoFocus);
    m_pPrinterListViewManual->setItemSpacing(10);
    m_pPrinterListModelManual = new QStandardItemModel(m_pPrinterListViewManual);
    m_pPrinterListViewManual->setModel(m_pPrinterListModelManual);
    m_pPrinterListViewManual->setIconSize(QSize(52, 52));
    m_pPrinterListViewManual->setAccessibleName("listViewManual_mainWindow");

    m_pInfoManual = new QLabel(m_pPrinterListViewManual);
    m_pInfoManual->setVisible(false);
    m_pInfoManual->setText(tr("No printers found"));
    m_pInfoManual->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_pInfoManual, DFontSizeManager::T5, int(QFont::DemiBold));
    pa = m_pInfoManual->palette();
    color = pa.color(QPalette::Inactive, QPalette::Text);
    pa.setColor(QPalette::Text, QColor(color.red(), color.green(), color.blue(), 150));
    m_pInfoManual->setPalette(pa);
    m_pInfoManual->setAccessibleName("infoManual_listViewManual");

    m_pManDriverWebLink = new QLabel();
    m_pManDriverWebLink->setContentsMargins(10, 0, 0, 0);
    m_pManDriverWebLink->setOpenExternalLinks(true);
    m_pManDriverWebLink->setToolTip(UI_PRINTER_DRIVER_MESSAGE + UI_PRINTER_DRIVER_WEBSITE);
    m_pManDriverWebLink->setText(tipInfo + webLink);
    m_pManDriverWebLink->setAccessibleName("manWebLink_manFrame2");

    QVBoxLayout *pVLayoutMan2 = new QVBoxLayout();
    pVLayoutMan2->addWidget(m_pPrinterListViewManual);
    pVLayoutMan2->addWidget(m_pManDriverWebLink);
    pVLayoutMan2->setContentsMargins(10, 10, 10, 10);
    QWidget *pManFrame2 = new QWidget();
    pManFrame2->setLayout(pVLayoutMan2);
    pManFrame2->setAccessibleName("manFrame2_manWidget");

    QLabel *pLabelDriver2 = new QLabel(UI_PRINTERSEARCH_DRIVER);
    pLabelDriver2->setFixedWidth(46);
    m_pManDriverCom = new QComboBox();
    m_pManDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
    m_pManDriverCom->setAccessibleName("manDriverCom_manFrame3");
    QHBoxLayout *pHLayout5 = new QHBoxLayout();
    pHLayout5->setSpacing(0);
    pHLayout5->addWidget(pLabelDriver2);
    pHLayout5->addSpacing(14);
    pHLayout5->addWidget(m_pManDriverCom, 1);
    pHLayout5->setContentsMargins(0, 0, 0, 0);

    m_pManListSpinner = new DSpinner(m_pPrinterListViewManual);
    m_pManListSpinner->setVisible(false);
    m_pManListSpinner->setAccessibleName("manListSpinner_listViewManual");
    m_pManSpinner = new DSpinner();
    m_pManSpinner->setFixedSize(36, 36);
    m_pManSpinner->setVisible(false);
    m_pManSpinner->setAccessibleName("manSpinner_manFrame3");
    m_pManInstallDriverBtn = new QPushButton(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
    m_pManInstallDriverBtn->setEnabled(false);
    m_pManInstallDriverBtn->setFixedWidth(200);
    m_pManInstallDriverBtn->setAccessibleName("manInstallBtn_manFrame3");

    QHBoxLayout *pHLayout6 = new QHBoxLayout();
    pHLayout6->addStretch();
    pHLayout6->addWidget(m_pManInstallDriverBtn);
    pHLayout6->addStretch();
    pHLayout6->setContentsMargins(0, 0, 0, 0);

    m_phplipTipLabel = new QLabel("");
    m_phplipTipLabel->setFixedWidth(485);
    m_phplipTipLabel->setWordWrap(true);
    m_phplipTipLabel->setContentsMargins(60, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_phplipTipLabel, DFontSizeManager::T8, QFont::Light);

    QVBoxLayout *pVLayoutSub2 = new QVBoxLayout();
    pVLayoutSub2->setSpacing(0);
    pVLayoutSub2->addLayout(pHLayout5);
    pVLayoutSub2->addSpacing(4);
    pVLayoutSub2->addWidget(m_phplipTipLabel);
    pVLayoutSub2->addSpacing(4);
    pVLayoutSub2->addWidget(m_pManSpinner, 0, Qt::AlignCenter);
    pVLayoutSub2->addLayout(pHLayout6);
    pVLayoutSub2->setContentsMargins(20, 10, 10, 10);
    QWidget *pManFrame3 = new QWidget();
    pManFrame3->setLayout(pVLayoutSub2);
    pManFrame3->setAccessibleName("manFrame3_manWidget");

    QVBoxLayout *pVLayout2 = new QVBoxLayout();
    pVLayout2->setSpacing(2);
    pVLayout2->addWidget(pManFrame1);
    pVLayout2->addWidget(pManFrame2);
    pVLayout2->addWidget(pManFrame3);
    pVLayout2->setMargin(0);
    DBackgroundGroup *pWidget2 = new DBackgroundGroup();
    pWidget2->setLayout(pVLayout2);
    pWidget2->setItemSpacing(2);
    pWidget2->setAccessibleName("manWidget_stackedWidget");
    m_pStackedWidget->addWidget(pWidget2);

    // 右侧 URI
    m_pLabelURI = new QLabel(tr("URI"));
    m_pLabelURI->setFixedWidth(43);
    m_pLabelURI->setAlignment(Qt::AlignCenter);
    m_pLabelURI->setAccessibleName("label_uriFrame1");
    m_pLineEditURI = new QLineEdit();
    m_pLineEditURI->setPlaceholderText(tr("Enter device URI"));
    m_pLineEditURI->setAccessibleName("uri_uriFrame1");

    QHBoxLayout *pHLayoutURI1 = new QHBoxLayout();
    pHLayoutURI1->setSpacing(0);
    pHLayoutURI1->addWidget(m_pLabelURI);
    pHLayoutURI1->addSpacing(20);
    pHLayoutURI1->addWidget(m_pLineEditURI);
    pHLayoutURI1->setContentsMargins(20, 20, 10, 20);
    QWidget *pURIFrame1 = new QWidget();
    pURIFrame1->setLayout(pHLayoutURI1);
    pURIFrame1->setFixedHeight(76);

    QScrollArea *tipScrollArea = new QScrollArea;
    tipScrollArea->setContentsMargins(0, 0, 0, 0);
    tipScrollArea->setWidgetResizable(true);
    tipScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    tipScrollArea->setWidgetResizable(true);
    tipScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    // 设置滑动条区域背景颜色为透明
    QPalette pa1 = tipScrollArea->palette();
    pa1.setBrush(QPalette::Window, Qt::transparent);
    tipScrollArea->setPalette(pa1);

    m_pLabelTip = new QLabel(tr("Examples:") + "\n" + UI_PRINTERSEARCH_URITIP);
    DFontSizeManager::instance()->bind(m_pLabelTip, DFontSizeManager::T8);
    QPalette pe;
    pe.setColor(QPalette::WindowText, QColor("#92A8BA"));
    m_pLabelTip->setPalette(pe);
    m_pLabelTip->setWordWrap(true);
    m_pLabelTip->setContentsMargins(70, 0, 0, 0);
    m_pLabelTip->setAccessibleName("labelTip_uriFrame2");
    tipScrollArea->setWidget(m_pLabelTip);
    m_pURIDriverWebLink = new QLabel();
    m_pURIDriverWebLink->setContentsMargins(10, 0, 0, 20);
    m_pURIDriverWebLink->setOpenExternalLinks(true);
    m_pURIDriverWebLink->setToolTip(UI_PRINTER_DRIVER_MESSAGE + UI_PRINTER_DRIVER_WEBSITE);
    m_pURIDriverWebLink->setText(tipInfo + webLink);
    m_pURIDriverWebLink->setAccessibleName("webLink_uriFrame2");

    QVBoxLayout *pURIVLayout2 = new QVBoxLayout();
    pURIVLayout2->addSpacing(10);
    pURIVLayout2->addWidget(tipScrollArea);
    pURIVLayout2->addSpacing(10);
    pURIVLayout2->addWidget(m_pURIDriverWebLink);
    pURIVLayout2->setContentsMargins(10, 10, 10, 10);
    QWidget *pURIFrame2 = new QWidget();
    pURIFrame2->setLayout(pURIVLayout2);
    pURIFrame2->setFixedHeight(284);

    QLabel *pLabelDriver3 = new QLabel(UI_PRINTERSEARCH_DRIVER);
    pLabelDriver3->setFixedWidth(46);
    m_pURIDriverCom = new QComboBox();
    m_pURIDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
    pLabelDriver3->setAccessibleName("labelDriver_uriFrame3");
    m_pURIDriverCom->setAccessibleName("uriDriver_uriFrame3");

    QHBoxLayout *pHLayout7 = new QHBoxLayout();
    pHLayout7->setSpacing(0);
    pHLayout7->addWidget(pLabelDriver3);
    pHLayout7->addSpacing(14);
    pHLayout7->addWidget(m_pURIDriverCom, 1);
    pHLayout7->setContentsMargins(0, 0, 0, 0);

    m_pURIInstallDriverBtn = new QPushButton(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
    m_pURIInstallDriverBtn->setEnabled(false);
    m_pURIInstallDriverBtn->setFixedWidth(200);
    m_pURIInstallDriverBtn->setAccessibleName("uriInstallBtn_uriFrame3");

    QHBoxLayout *pHLayout8 = new QHBoxLayout();
    pHLayout8->addStretch();
    pHLayout8->addWidget(m_pURIInstallDriverBtn);
    pHLayout8->addStretch();
    pHLayout8->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *pURIVLayoutSub = new QVBoxLayout();
    pURIVLayoutSub->setSpacing(0);
    pURIVLayoutSub->addLayout(pHLayout7);
    pURIVLayoutSub->addSpacing(26);
    pURIVLayoutSub->addLayout(pHLayout8);
    pURIVLayoutSub->setContentsMargins(20, 10, 10, 10);
    QWidget *pURIFrame3 = new QWidget();
    pURIFrame3->setLayout(pURIVLayoutSub);

    pURIFrame1->setAccessibleName("uriFrame1_uriWidget");
    pURIFrame2->setAccessibleName("uriFrame2_uriWidget");
    pURIFrame3->setAccessibleName("uriFrame3_uriWidget");

    QVBoxLayout *pURIVLayout = new QVBoxLayout();
    pURIVLayout->setSpacing(2);
    pURIVLayout->addWidget(pURIFrame1);
    pURIVLayout->addWidget(pURIFrame2);
    pURIVLayout->addWidget(pURIFrame3);
    pURIVLayout->setMargin(0);

    DBackgroundGroup *pWidget3 = new DBackgroundGroup();
    pWidget3->setLayout(pURIVLayout);
    pWidget3->setItemSpacing(2);
    pWidget3->setAccessibleName("uriWidget_stackedWidget");

    m_pStackedWidget->addWidget(pWidget3);
    m_pStackedWidget->setAccessibleName("stackedWidget_centralWidget");

    // 右侧整体布局
    QVBoxLayout *pRightVLayout = new QVBoxLayout();
    pRightVLayout->addWidget(m_pStackedWidget);
    pRightVLayout->setContentsMargins(0, 10, 10, 10);

    // 整体布局
    QHBoxLayout *m_pMainHLayout = new QHBoxLayout();
    m_pMainHLayout->setSpacing(10);
    m_pMainHLayout->addWidget(pLeftWidget, 1);
    //    m_pMainHLayout->addWidget(pRightWidget, 2);
    m_pMainHLayout->addLayout(pRightVLayout, 2);
    m_pMainHLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *pCentralWidget = new QWidget();
    pCentralWidget->setLayout(m_pMainHLayout);
    pCentralWidget->setAccessibleName("centralWidget_searchWindow");
    takeCentralWidget();
    setCentralWidget(pCentralWidget);

    // 手动安装驱动
    m_pInstallDriverWindow = new InstallDriverWindow();
    m_pInstallDriverWindow->setAccessibleName("installDriver_searchWindow");
    m_pInstallDriverWindow->setPreWidget(this);
    Dtk::Widget::moveToCenter(this);
    this->setAccessibleName("searchWindow_mainWindow");
}

void PrinterSearchWindow::initConnections()
{
    connect(m_pTabListView, QOverload<const QModelIndex &>::of(&DListView::currentChanged), this, &PrinterSearchWindow::listWidgetClickedSlot);

    connect(m_pPrinterListViewAuto, &DListView::clicked, this, &PrinterSearchWindow::printerListClickedSlot);

    connect(m_pPrinterListViewManual, &DListView::clicked, this, &PrinterSearchWindow::printerListClickedSlot);

    connect(m_pBtnRefresh, &DIconButton::clicked, this, &PrinterSearchWindow::refreshPrinterListSlot);

    connect(m_pBtnFind, &QPushButton::clicked, this, &PrinterSearchWindow::searchPrintersByManual);

    connect(m_pAutoInstallDriverBtn, &QPushButton::clicked, this, &PrinterSearchWindow::installDriverSlot);
    connect(m_pManInstallDriverBtn, &QPushButton::clicked, this, &PrinterSearchWindow::installDriverSlot);
    connect(m_pURIInstallDriverBtn, &QPushButton::clicked, this, &PrinterSearchWindow::installDriverSlot);

    connect(m_pAutoDriverCom, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PrinterSearchWindow::driverChangedSlot);
    connect(m_pManDriverCom, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PrinterSearchWindow::driverChangedSlot);
    connect(m_pURIDriverCom, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PrinterSearchWindow::driverChangedSlot);

    connect(m_pLineEditURI, &QLineEdit::textChanged, this, &PrinterSearchWindow::lineEditURIChanged);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, [&]() {
        m_pWidgetItemAuto->setSizeHint(DSizeModeHelper::element(QSize(108, 36),QSize(108, 48)));
        m_pWidgetItemManual->setSizeHint(DSizeModeHelper::element(QSize(108, 36),QSize(108, 48)));
        m_pWidgetItemURI->setSizeHint(DSizeModeHelper::element(QSize(108, 36),QSize(108, 48)));
    });
#endif

    connect(m_pLineEditLocation, &QLineEdit::editingFinished, this, [this]() {
        // 按下enter会触发两次信号，需要过滤掉失去焦点之后的信号 并且判断校验结果
        if (m_pLineEditLocation->hasFocus())
            searchPrintersByManual();
    });
}

void PrinterSearchWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    //启动自动刷新打印机列表
    if (m_pTabListView->currentIndex().row() == 0 && m_pPrinterListViewAuto->count() == 0)
        refreshPrinterListSlot();
}

QStandardItem *PrinterSearchWindow::getCheckedItem(int type)
{
    QStandardItem *pCheckedItem = nullptr;
    if (type == 0) {
        int count = m_pPrinterListModel->rowCount();
        for (int i = 0; i < count; ++i) {
            QStandardItem *pTemp = m_pPrinterListModel->item(i);
            if (pTemp->checkState() == Qt::Checked) {
                pCheckedItem = pTemp;
                break;
            }
        }
    } else if (type == 1) {
        int count = m_pPrinterListModelManual->rowCount();
        for (int i = 0; i < count; ++i) {
            QStandardItem *pTemp = m_pPrinterListModelManual->item(i);
            if (pTemp->checkState() == Qt::Checked) {
                pCheckedItem = pTemp;
                break;
            }
        }
    }
    return pCheckedItem;
}

bool PrinterSearchWindow::autoInstallPrinter(int type, const TDeviceInfo &device)
{
    QComboBox *pDriverCombo = nullptr;
    if (type == 0) {
        pDriverCombo = m_pAutoDriverCom;
    } else if (type == 1) {
        pDriverCombo = m_pManDriverCom;
    } else {
        pDriverCombo = m_pURIDriverCom;
    }
    QMap<QString, QVariant> solution = pDriverCombo->currentData().value<QMap<QString, QVariant>>();
    if (solution.isEmpty())
        return false;

    AddPrinterTask *task = g_addPrinterFactoty->createAddPrinterTask(device, solution);
    close();
    m_pInstallPrinterWindow = new InstallPrinterWindow(this);
    connect(m_pInstallPrinterWindow, &InstallPrinterWindow::showParentWindows, this, [&]() {
        this->show();
    });
    m_pInstallPrinterWindow->show();
    m_pInstallPrinterWindow->setTask(task);
    m_pInstallPrinterWindow->setDevice(device);
    QMap<QString, QVariant> dataMap;
    int driverCount = pDriverCombo->count();
    for (int i = 0; i < driverCount; ++i) {
        if (i != pDriverCombo->currentIndex() && i != pDriverCombo->count() - 1) {
            dataMap.insert(pDriverCombo->itemText(i), pDriverCombo->itemData(i));
        }
    }
    m_pInstallPrinterWindow->copyDriverData(dataMap);
    connect(task, &AddPrinterTask::signalStatus, m_pInstallPrinterWindow, &InstallPrinterWindow::receiveInstallationStatusSlot);
    connect(m_pInstallPrinterWindow, &InstallPrinterWindow::updatePrinterList, this, &PrinterSearchWindow::updatePrinterList);
    task->doWork();
    return true;
}

void PrinterSearchWindow::listWidgetClickedSlot(const QModelIndex &previous)
{
    Q_UNUSED(previous)
    // 0 自动查找 ;1 手动；2 URI
    if (m_pStackedWidget->count() > m_pTabListView->currentIndex().row()) {
        m_pStackedWidget->setCurrentIndex(m_pTabListView->currentIndex().row());
        if (m_pTabListView->currentIndex().row() == 0) {
        }
        if (m_pTabListView->currentIndex().row() == 1) {
        } else if (m_pTabListView->currentIndex().row() == 2) {
        }
    }
}

QString PrinterSearchWindow::printerDescription(const TDeviceInfo &info, bool manual)
{
    QString strDesc = !info.strName.isEmpty() ? info.strName : !info.strInfo.isEmpty() ? info.strInfo + " " : info.strMakeAndModel;
    QString strUri = info.uriList[0];
    if (strDesc.isEmpty())
        strDesc = strUri;
    else {
        /*
         *网络打印机在局域网中存在多台同型号时，普通用户无法通过ip区分打印机，如果用户设置了打印机位置属性
         * 比如办公室等，显示在查找界面方便用户区分打印机。
        */
        QString protocol = strUri.left(strUri.indexOf(":/"));
        if (protocol == "socket" && !info.strLocation.isEmpty() && !isIpv4Address(info.strLocation)) {
            strDesc += "-" + info.strLocation;
        }
        if (!manual) {
            QString strHost = getHostFromUri(strUri);
            if (!strHost.isEmpty())
                strDesc += QString(" (%1)").arg(strHost);
        }

        if (!protocol.isEmpty())
            strDesc += QObject::tr("(use %1 protocol)").arg(protocol);

    }

    return strDesc;
}

void PrinterSearchWindow::getDeviceResultSlot(int id, int state)
{
    Q_UNUSED(id)
    if (state == ETaskStatus::TStat_Suc || state == ETaskStatus::TStat_Fail) {
        sender()->deleteLater();
        m_pAutoListSpinner->stop();
        m_pAutoListSpinner->setVisible(false);
        m_pAutoInstallDriverBtn->setVisible(true);
        if (m_pPrinterListViewAuto->count() > 0) {
            m_pAutoInstallDriverBtn->setEnabled(true);
            m_pInfoAuto->setVisible(false);
        } else {
            m_pAutoInstallDriverBtn->setEnabled(false);
            m_pInfoAuto->resize(m_pPrinterListViewAuto->size());
            m_pInfoAuto->setVisible(true);
        }
        m_pBtnRefresh->blockSignals(false);
        m_pPrinterListViewAuto->blockSignals(false);
    } else if (state == ETaskStatus::TStat_Update) {
        RefreshDevicesByBackendTask *task = static_cast<RefreshDevicesByBackendTask *>(sender());
        if (!task)
            return;
        QList<TDeviceInfo> deviceList = task->getResult();
        int index = m_pPrinterListViewAuto->count();
        //更新打印机数据
        for (int i = 0; i < index; i++) {
            const TDeviceInfo &info = deviceList[i];
            QAbstractItemModel *model = m_pPrinterListViewAuto->model();
            if (model) {
                QModelIndex itemIndex = model->index(i, 0);
                model->setData(itemIndex, QVariant::fromValue(info), Dtk::UserRole + 1);
            }
        }
        for (; index < deviceList.count(); index++) {
            const TDeviceInfo &info = deviceList[index];
            DStandardItem *pItem = new DStandardItem(info.strName);
            // 设置item的margin，这两个dataRole的枚举值一样，需要区分开
            pItem->setData(VListViewItemMargin, Dtk::MarginsRole);
            pItem->setText(printerDescription(info));
            pItem->setIcon(QIcon::fromTheme("dp_printer_list"));
            // 将结构体转化为QVariant,需要再转回来
            pItem->setData(QVariant::fromValue(info), Dtk::UserRole + 1);
            pItem->setSizeHint(QSize(480, 60));
            pItem->setToolTip(info.strName);
            m_pPrinterListModel->appendRow(pItem);
        }
    }
}

void PrinterSearchWindow::getDeviceResultByManualSlot(int id, int state)
{
    Q_UNUSED(id)
    if (state == ETaskStatus::TStat_Suc || state == ETaskStatus::TStat_Fail) {
        sender()->deleteLater();
        m_pManListSpinner->stop();
        m_pManListSpinner->setVisible(false);
        m_pManInstallDriverBtn->setVisible(true);
        if (m_pPrinterListViewManual->count() > 0) {
            m_pManInstallDriverBtn->setEnabled(true);
            m_pInfoManual->setVisible(false);
        } else {
            m_pManInstallDriverBtn->setEnabled(false);
            m_pInfoManual->resize(m_pPrinterListViewManual->size());
            m_pInfoManual->setVisible(true);
        }
        //解除阻塞
        m_pBtnFind->blockSignals(false);
        m_pLineEditLocation->blockSignals(false);
        m_pPrinterListViewManual->blockSignals(false);
        if (TStat_Fail == state || m_pInfoManual->isVisible() == true) {
            DDialog dlg(this);
            QLabel *pLabelTip = new QLabel();
            pLabelTip->setWordWrap(true);
            pLabelTip->setAlignment(Qt::AlignCenter);
            pLabelTip->setContentsMargins(0, 0, 0, 0);

            if (TStat_Fail == state) {
            TaskInterface *task = static_cast<TaskInterface *>(sender());
                pLabelTip->setText(task->getErrorString());
            } else {
                pLabelTip->setText(UI_PRINTER_MANUAL_TIPINFO);
                pLabelTip->setToolTip(UI_PRINTER_MANUAL_TIPINFO);
            }

            QScrollArea *tipScrollArea = new QScrollArea;
            tipScrollArea->setContentsMargins(0, 0, 0, 0);
            tipScrollArea->setWidgetResizable(true);
            tipScrollArea->setFrameShape(QFrame::Shape::NoFrame);
            tipScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
            tipScrollArea->setWidget(pLabelTip);
            QPalette pa1 = tipScrollArea->palette();
            pa1.setBrush(QPalette::Window, Qt::transparent);
            tipScrollArea->setPalette(pa1);
            dlg.addContent(tipScrollArea);
            dlg.setIcon(QIcon(":/images/warning_logo.svg"));
            dlg.addButton(tr("OK"));
            dlg.setContentsMargins(10, 15, 10, 15);
            dlg.setModal(true);
            dlg.setFixedSize(452, 202);
            dlg.exec();
        }
    } else if (state == ETaskStatus::TStat_Update) {
        RefreshDevicesByHostTask *task = static_cast<RefreshDevicesByHostTask *>(sender());
        if (!task)
            return;
        QList<TDeviceInfo> deviceList = task->getResult();
        int index = m_pPrinterListViewManual->count();
        for (; index < deviceList.count(); index++) {
            const TDeviceInfo &info = deviceList[index];
            DStandardItem *pItem = new DStandardItem(info.strName);
            // 设置item的margin，这两个dataRole的枚举值一样，需要区分开
            pItem->setData(VListViewItemMargin, Dtk::MarginsRole);
            pItem->setText(printerDescription(info, true));
            pItem->setIcon(QIcon::fromTheme("dp_printer_list"));
            // 将结构体转化为QVariant,需要再转回来
            pItem->setData(QVariant::fromValue(info), Dtk::UserRole + 1);
            pItem->setSizeHint(QSize(480, 60));
            pItem->setToolTip(info.strName);
            m_pPrinterListModelManual->appendRow(pItem);
        }
    }
}

void PrinterSearchWindow::printerListClickedSlot(const QModelIndex &index)
{
    DriverManager *pDManager = DriverManager::getInstance();
    QList<QMap<QString, QString>> driverList;
    QVariant temp;
    (void)getCurrentTime(START_TIME);
    if (sender() == m_pPrinterListViewAuto) {
        temp = m_pPrinterListViewAuto->currentIndex().data(Dtk::UserRole + 1);
        for (int row = 0; row < m_pPrinterListModel->rowCount(); ++row) {
            if (row != index.row())
                m_pPrinterListModel->item(row)->setCheckState(Qt::Unchecked);
            else {
                m_pPrinterListModel->item(row)->setCheckState(Qt::Checked);
            }
        }
        m_pPrinterListViewAuto->blockSignals(true);
        m_pAutoInstallDriverBtn->setVisible(false);
        m_pAutoSpinner->setVisible(true);
        m_pAutoSpinner->start();
        TDeviceInfo device = temp.value<TDeviceInfo>();
        m_phplipAutoTipLabel->setText("");
        if (device.strMakeAndModel.startsWith("hp", Qt::CaseInsensitive) &&
            isPackageInstalled(strHplipName)) {
            m_phplipAutoTipLabel->setText(UI_PRINTER_HPLIP_TIPINFO);
        }

        DriverSearcher *pDriverSearcher = pDManager->createSearcher(device);
        connect(pDriverSearcher, &DriverSearcher::signalDone, this, &PrinterSearchWindow::driverAutoSearchedSlot);
        pDriverSearcher->startSearch();
    } else {
        temp = m_pPrinterListViewManual->currentIndex().data(Dtk::UserRole + 1);
        for (int row = 0; row < m_pPrinterListModelManual->rowCount(); ++row) {
            if (row != index.row())
                m_pPrinterListModelManual->item(row)->setCheckState(Qt::Unchecked);
            else {
                m_pPrinterListModelManual->item(row)->setCheckState(Qt::Checked);
            }
        }
        //驱动下拉列表的数据是异步获取的，在获取到数据之前先阻塞当前列表的信号，防止本次数据获取之前，用户又开始点击另外的列
        // 导致驱动和打印机数据无法对应，安装的时候崩溃
        m_pPrinterListViewManual->blockSignals(true);
        m_pManInstallDriverBtn->setVisible(false);
        m_pManSpinner->setVisible(true);
        m_pManSpinner->start();
        TDeviceInfo device = temp.value<TDeviceInfo>();

        m_phplipTipLabel->setText("");
        if (device.strMakeAndModel.startsWith("hp", Qt::CaseInsensitive) &&
            isPackageInstalled(strHplipName)) {
            m_phplipTipLabel->setText(UI_PRINTER_HPLIP_TIPINFO);
        }
        DriverSearcher *pDriverSearcher = pDManager->createSearcher(device);
        connect(pDriverSearcher, &DriverSearcher::signalDone, this, &PrinterSearchWindow::driverManSearchedSlot);
        pDriverSearcher->startSearch();
    }
}

QString PrinterSearchWindow::driverDescription(const QMap<QString, QVariant> &driver)
{
    QString strDesc;
    if (driver.value("ppd-make-and-model").isNull())
        strDesc = driver.value("ppd-make").toString() + " " + driver.value("ppd-model").toString();
    else
        strDesc = driver.value("ppd-make-and-model").toString();

    //去掉自带的recommended字段
    strDesc.replace("(recommended)", "");

    if (driver.contains(SD_KEY_recommended) && driver[SD_KEY_recommended].toBool()) {
        strDesc += tr(" (recommended)");
    }

    return strDesc;
}

void PrinterSearchWindow::driverSearchNoMatchDialog(bool isExist)
{
    DDialog pDialog;
    QLabel *pMessage1, *pMessage2;
    if (isExist) {
        pMessage1 = new QLabel(tr("The driver was found"));
        pMessage2 = new QLabel(tr("Connect to the network and try again"));
    } else {
        pMessage1 = new QLabel(tr("Cannot find the driver"));
        pMessage2 = new QLabel(tr("Please try to install it manually or search its official website"));
    }

    DFontSizeManager::instance()->bind(pMessage1, DFontSizeManager::T6);
    DFontSizeManager::instance()->bind(pMessage2, DFontSizeManager::T7);

    pMessage1->setAlignment(Qt::AlignCenter);
    pMessage2->setAlignment(Qt::AlignCenter);
    pDialog.addContent(pMessage1);
    pDialog.addContent(pMessage2);
    pDialog.addButton(tr("OK", "button"), false, DDialog::ButtonRecommend);
    pDialog.setIcon(QIcon(":/images/warning_logo.svg"));

    QRect rect = this->geometry();
    // 计算显示原点
    int x = rect.x() + rect.width()/2 - pDialog.width() /2;
    int y = rect.y() + rect.height()/2 - pDialog.height()/2;
    pDialog.move(x, y);

    pDialog.exec();
}

void PrinterSearchWindow::driverAutoSearchedSlot()
{
    netCount = 0;
    DriverSearcher *pDriverSearcher = static_cast<DriverSearcher *>(sender());
    if (pDriverSearcher) {
        m_pAutoDriverCom->clear();
        m_drivers = pDriverSearcher->getDrivers();
        pDriverSearcher->deleteLater();

        bool isInsert = false;
        int indexSelect = -1;

        int index = 0;
        foreach (const auto &driver, m_drivers) { // 驱动已排序，网络驱动前ipp居中本地在后
            QString driverdesc = driverDescription(driver);
            if (driver[SD_KEY_recommended].toBool() && (indexSelect == -1)) {
                indexSelect = index;
                ++netCount;
            } else if (!driver.contains(SD_KEY_recommended)) { // 处理本地，网络驱动携带is_recommend字段
                if (netCount >= 1 && !isInsert) {
                    m_pAutoDriverCom->insertSeparator(m_pAutoDriverCom->count());
                    isInsert = true;
                    if ((driver[SD_KEY_from].toInt() == PPDFrom_EveryWhere) && (indexSelect == -1)) {
                        indexSelect = index;
                    } else if (indexSelect == -1) {
                        indexSelect = index;
                    }
                }
            } else {
                ++netCount;
            }
            ++index;
            m_pAutoDriverCom->addItem(driverdesc, QVariant::fromValue(driver));
        }
        if (indexSelect != -1) {
            if (isInsert && (indexSelect != 0)) {
                ++indexSelect;
            }
            m_pAutoDriverCom->setCurrentIndex(indexSelect);
            showHplipDriverInfo(indexSelect);
        }
        m_pAutoDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
    }
    m_pAutoSpinner->stop();
    m_pAutoSpinner->setVisible(false);
    m_pAutoInstallDriverBtn->setVisible(true);
    m_pAutoInstallDriverBtn->setEnabled(true);
    if (m_pAutoDriverCom->count() >= 2) {
        m_pAutoInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_AUTO);
    } else {
        m_pAutoInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
        driverSearchNoMatchDialog(pDriverSearcher->hasOfflineDriver());
    }
    //解除信号阻塞，重新响应列表的点击事件

    m_pPrinterListViewAuto->blockSignals(false);
}

void PrinterSearchWindow::driverManSearchedSlot()
{
    netCount = 0;
    DriverSearcher *pDriverSearcher = static_cast<DriverSearcher *>(sender());
    if (pDriverSearcher) {
        m_pManDriverCom->clear();
        m_drivers = pDriverSearcher->getDrivers();
        pDriverSearcher->deleteLater();

        bool isInsert = false;
        int indexSelect = -1;
        int index = 0;
        foreach (const auto &driver, m_drivers) { // 驱动已排序，网络驱动前ipp居中本地在后
            QString driverdesc = driverDescription(driver);
            if (driver[SD_KEY_recommended].toBool() && (indexSelect == -1)) {
                indexSelect = index;
                ++netCount;
            } else if (!driver.contains(SD_KEY_recommended)) { // 处理本地，网络驱动携带is_recommend字段
                if (netCount >= 1 && !isInsert) {
                    m_pManDriverCom->insertSeparator(m_pManDriverCom->count());
                    isInsert = true;
                    if ((driver[SD_KEY_from].toInt() == PPDFrom_EveryWhere) && (indexSelect == -1)) {
                        indexSelect = index;
                    } else if (indexSelect == -1) {
                        indexSelect = index;
                    }
                }
            } else {
                ++netCount;
            }
            ++index;
            m_pManDriverCom->addItem(driverdesc, QVariant::fromValue(driver));
        }
        if (indexSelect != -1) {
            if (isInsert && (indexSelect != 0)) {
                ++indexSelect;
            }
            m_pManDriverCom->setCurrentIndex(indexSelect);
            showHplipDriverInfo(indexSelect);
        }
        m_pManDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
    }
    m_pManSpinner->stop();
    m_pManSpinner->setVisible(false);
    m_pManInstallDriverBtn->setVisible(true);
    m_pManInstallDriverBtn->setEnabled(true);
    if (m_pManDriverCom->count() >= 2) {
        m_pManInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_AUTO);
    } else {
        m_pManInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
        driverSearchNoMatchDialog(pDriverSearcher->hasOfflineDriver());
    }
    //解除信号阻塞，重新响应列表的点击事件

    m_pPrinterListViewManual->blockSignals(false);
}

void PrinterSearchWindow::refreshPrinterListSlot()
{
    m_phplipAutoTipLabel->setText("");
    m_pBtnRefresh->blockSignals(true);
    m_pBtnRefresh->setVisible(true);

    m_pAutoInstallDriverBtn->setEnabled(false);
    //屏蔽点击信号，避免未刷新完打印机就开始刷新驱动，目前只有一个loading动画
    m_pPrinterListViewAuto->blockSignals(true);
    RefreshDevicesByBackendTask *task = new RefreshDevicesByBackendTask();
    //lambda表达式使用directConnection,这里需要在UI线程执行槽函数
    connect(task, &RefreshDevicesByBackendTask::signalStatus, this, &PrinterSearchWindow::getDeviceResultSlot);
    task->start();

    m_pAutoListSpinner->move((m_pPrinterListViewAuto->width() - m_pAutoListSpinner->width()) / 2,
                             (m_pPrinterListViewAuto->height() - m_pAutoListSpinner->height()) / 2);
    m_pAutoListSpinner->setVisible(true);
    m_pAutoListSpinner->start();
    m_pInfoAuto->setVisible(false);
    m_pAutoSpinner->setVisible(false);
    m_pPrinterListModel->clear();
    m_pAutoDriverCom->clear();
    m_pAutoDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
}

void PrinterSearchWindow::searchPrintersByManual()
{
    QString location = m_pLineEditLocation->text();
    m_phplipTipLabel->setText("");
    if (location.isEmpty())
        return;
    // 阻塞信号防止查询完成之前重复查询打印机
    m_pBtnFind->blockSignals(true);
    m_pLineEditLocation->blockSignals(true);
    m_pPrinterListViewManual->blockSignals(true);
    RefreshDevicesByHostTask *task = new RefreshDevicesByHostTask(location);
    connect(task, &RefreshDevicesByHostTask::signalStatus, this, &PrinterSearchWindow::getDeviceResultByManualSlot);
    // 当使用ip查找打印机的时候，smb协议流程会出现用户名和密码的输入提示框
    connect(task, &RefreshDevicesByHostTask::signalSmbPassWord, this, &PrinterSearchWindow::smbInfomationSlot, Qt::BlockingQueuedConnection);
    task->start();

    m_pManListSpinner->move((m_pPrinterListViewManual->width() - m_pManListSpinner->width()) / 2,
                            (m_pPrinterListViewManual->height() - m_pManListSpinner->height()) / 2);
    m_pManListSpinner->setVisible(true);
    m_pManListSpinner->start();
    m_pInfoManual->setVisible(false);
    m_pPrinterListModelManual->clear();
    m_pManDriverCom->clear();
    m_pManDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
}

void PrinterSearchWindow::lineEditURIChanged(const QString &uri)
{
    //QRegExp reg("(\\S+)(://)(\\S+)");
    QString strVal = uri.trimmed();
    QRegExp reg("([^# ]{1,})(:[/]{1,})([^# ]{1,})");
    QRegExpValidator v(reg);
    int pos = 0;
    QValidator::State state = v.validate(strVal, pos);
    if (state == QValidator::Acceptable) {
        QMap<QString, QVariant> driver = g_driverManager->getEveryWhereDriver(uri);
        if (m_pURIDriverCom->count() < 2 && !driver.isEmpty()) {
            //手动屏蔽m_pURIDriverCom信号处理，避免清空和插入操作触发currentIndexChanged对应的槽函数，添加完成之后在手动调用修改对应的按钮文案
            m_pURIDriverCom->blockSignals(true);
            m_pURIDriverCom->clear();
            m_pURIDriverCom->addItem(driver[CUPS_PPD_MAKE_MODEL].toString(), QVariant::fromValue(driver));
            m_pURIDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
            m_pURIDriverCom->blockSignals(false);
            driverChangedSlot(0);
        } else if (m_pURIDriverCom->count() > 1 && driver.isEmpty()) {
            m_pURIDriverCom->clear();
            m_pURIDriverCom->addItem(UI_PRINTERSEARCH_MANUAL);
        }
        m_pURIInstallDriverBtn->setEnabled(true);
    } else {
        m_pURIInstallDriverBtn->setEnabled(false);
    }
}

void PrinterSearchWindow::installDriverSlot()
{
    TDeviceInfo device;
    if (m_pTabListView->currentIndex().row() == 0) { // 自动
        // 当前选中的row和checkeded的row已经不一致，按照checked的为准
        QStandardItem *pCheckedItem = getCheckedItem(0);
        if (!pCheckedItem) {
            return;
        }
        QVariant temp = pCheckedItem->data(Dtk::UserRole + 1);
        device = temp.value<TDeviceInfo>();
    } else if (m_pTabListView->currentIndex().row() == 1) { //ip
        QStandardItem *pCheckedItem = getCheckedItem(1);
        if (!pCheckedItem) {
            return;
        }
        QVariant temp = pCheckedItem->data(Dtk::UserRole + 1);
        device = temp.value<TDeviceInfo>();
    } else if (m_pTabListView->currentIndex().row() == 2) { //URI 手动构造设备结构体
        device.uriList.append(m_pLineEditURI->text());
        device.iType = InfoFrom_Manual;
    }

    if (!autoInstallPrinter(m_pTabListView->currentIndex().row(), device)) {
        close();
        m_pInstallDriverWindow->setDeviceInfo(device);
        m_pInstallDriverWindow->show();
    }
}

void PrinterSearchWindow::driverChangedSlot(int index)
{
    if (m_pTabListView->currentIndex().row() == 0) {
        if (index == m_pAutoDriverCom->count() - 1) {
            // 手动选择驱动
            m_pAutoInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
        } else {
            // 自动安装驱动
            showHplipDriverInfo(m_pAutoDriverCom->currentIndex());
            m_pAutoInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_AUTO);
        }
    } else if (m_pTabListView->currentIndex().row() == 1) {
        if (index == m_pManDriverCom->count() - 1) {
            // 手动选择驱动
            m_pManInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
        } else {
            showHplipDriverInfo(m_pManDriverCom->currentIndex());
            // 自动安装驱动
            m_pManInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_AUTO);
        }
    } else if (m_pTabListView->currentIndex().row() == 2) {
        if (index == m_pURIDriverCom->count() - 1) {
            // 手动选择驱动
            m_pURIInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_NEXT);
        } else {
            // 自动安装驱动
            m_pURIInstallDriverBtn->setText(UI_PRINTERSEARCH_INSTALLDRIVER_AUTO);
        }
    }
}

void PrinterSearchWindow::smbInfomationSlot(int &ret, const QString &host, QString &group, QString &user, QString &password)
{
    PermissionsWindow *pPermissionsWindow = new PermissionsWindow();
    pPermissionsWindow->setHost(host);
    int result = pPermissionsWindow->exec();
    if (result > 0) {
        ++ret;
        user = pPermissionsWindow->getUser();
        group = pPermissionsWindow->getGroup();
        password = pPermissionsWindow->getPassword();
    }
    pPermissionsWindow->deleteLater();
}

void PrinterSearchWindow::changeEvent(QEvent *event)
{
    /* 字体改变时，调整显示 */
    if (event->type() == QEvent::FontChange) {
        QString tipInfo = UI_PRINTER_DRIVER_MESSAGE;
        QString webLinkinfo = UI_PRINTER_DRIVER_WEBSITE;
        QFontMetrics fm(m_pAutoDriverWebLink->font());
        geteElidedText(m_pAutoDriverWebLink->font(), webLinkinfo, 480 - fm.width(tipInfo));
        QString webLink = QObject::tr(UI_PRINTER_DRIVER_WEB_LINK).arg(webLinkinfo);
        m_pAutoDriverWebLink->setText(tipInfo + webLink);

        m_pManDriverWebLink->setText(tipInfo + webLink);
        QWidget::changeEvent(event);
    }
}

void PrinterSearchWindow::showHplipDriverInfo(int index)
{
    // 存在网络推荐驱动并且索引非网络驱动时，index需-1
    int realIndex = (netCount == 0 || (index <= netCount - 1)) ? index : (index - 1);
    if (realIndex <= m_drivers.count() && m_drivers[realIndex][SD_KEY_driver].toString() == strHplipName) {
        m_phplipTipLabel->setText(UI_PRINTER_HPLIP_TIPINFO);
        m_phplipAutoTipLabel->setText(UI_PRINTER_HPLIP_TIPINFO);
    }
}
