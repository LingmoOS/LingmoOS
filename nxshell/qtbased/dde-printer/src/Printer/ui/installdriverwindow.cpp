// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "installdriverwindow.h"
#include "zdrivermanager.h"
#include "addprinter.h"
#include "installprinterwindow.h"
#include "cupsattrnames.h"
#include "printerservice.h"
#include "printersearchwindow.h"
#include "uisourcestring.h"
#include "common.h"
#include "qtconvert.h"

#include <DListView>
#include <DLineEdit>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DFileDialog>
#include <DSpinner>
#include <DFrame>
#include <DBackgroundGroup>
#include <DComboBox>
#include <DButtonBox>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QStandardItemModel>
#include <QStandardItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>

QtCompleterDelegate::QtCompleterDelegate(QObject *pParent)
    : QStyledItemDelegate(pParent)
{
}

QSize QtCompleterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(400, 30);
}

InstallDriverWindow::InstallDriverWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_pManufactureCompleter(nullptr)
    , m_pTypeCompleter(nullptr)
    , m_pDriverCompleter(nullptr)
{
    initUI();
    initConnections();
}

InstallDriverWindow::~InstallDriverWindow()
{
    unloadEventLib();
}

void InstallDriverWindow::initUI()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    m_pPreBtn = new DButtonBoxButton(QIcon::fromTheme("dp_arrow_left"), "", this);
    m_pPreBtn->setIconSize(QSize(7, 10));
    m_pPreBtn->setAccessibleName("preBtn_installDriver");
    DButtonBox *pBtnBox = new DButtonBox(titlebar());
    QList<DButtonBoxButton *> btnList;
    btnList.append(m_pPreBtn);
    pBtnBox->setButtonList(btnList, false);
    titlebar()->addWidget(pBtnBox, Qt::AlignLeft);
    titlebar()->setAccessibleName("titleBar_installDriver");
    // 去掉最大最小按钮
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(682, 532);
    // 左侧
    QString labelTitle = tr("Select a driver from");
    QLabel *pLabelTitle1 = new QLabel(this);
    geteElidedText(pLabelTitle1->font(), labelTitle, 90);
    pLabelTitle1->setText(labelTitle);
    pLabelTitle1->setToolTip(tr("Select a driver from"));
    pLabelTitle1->setAccessibleName("leftTitle_installDriver");
    DFontSizeManager::instance()->bind(pLabelTitle1, DFontSizeManager::T5, QFont::DemiBold);
    m_pTabListView = new DListView(this);
    m_pTabListModel = new QStandardItemModel(m_pTabListView);

    m_pLocalDriver = new DStandardItem(tr("Local driver"));
    m_pLocalDriver->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pLocalDriver->setToolTip(tr("Local driver"));

    m_pPpdFile = new DStandardItem(tr("Local PPD file"));
    m_pPpdFile->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pPpdFile->setToolTip(tr("Local PPD file"));

    m_pSearchDriver = new DStandardItem(tr("Search for a driver"));
    m_pSearchDriver->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pSearchDriver->setToolTip(tr("Search for a driver"));

#ifdef DTKWIDGET_CLASS_DSizeMode
    m_pLocalDriver->setSizeHint(DSizeModeHelper::element(QSize(108, 36), QSize(108, 48)));
    m_pPpdFile->setSizeHint(DSizeModeHelper::element(QSize(108, 36), QSize(108, 48)));
    m_pSearchDriver->setSizeHint(DSizeModeHelper::element(QSize(108, 36), QSize(108, 48)));
#else
    m_pLocalDriver->setSizeHint(QSize(108, 48));
    m_pPpdFile->setSizeHint(QSize(108, 48));
    m_pSearchDriver->setSizeHint(QSize(108, 48));
#endif

    m_pTabListModel->appendRow(m_pLocalDriver);
    m_pTabListModel->appendRow(m_pPpdFile);
    m_pTabListModel->appendRow(m_pSearchDriver);
    m_pTabListView->setModel(m_pTabListModel);
    m_pTabListView->setCurrentIndex(m_pTabListModel->index(0, 0));
    m_pTabListView->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    m_pTabListView->setItemSpacing(4);
    QVBoxLayout *pLeftVBoxlayout = new QVBoxLayout();
    pLeftVBoxlayout->setContentsMargins(10, 20, 10, 0);
    pLeftVBoxlayout->addWidget(pLabelTitle1);
    pLeftVBoxlayout->addWidget(m_pTabListView);
    QWidget *pLeftFrame = new QWidget(this);
    pLeftFrame->setFixedWidth(150);
    pLeftFrame->setLayout(pLeftVBoxlayout);
    pLeftFrame->setAccessibleName("leftFrame_installDriver");

    // 切换控件
    m_pStackWidget = new QStackedWidget(this);
    m_pRightTitleLabel = new QLabel(tr("Choose a local driver"), this); // 这个标题栏三个界面公用
    m_pRightTitleLabel->setAccessibleName("rightTitle_installDriver");
    DFontSizeManager::instance()->bind(m_pRightTitleLabel, DFontSizeManager::T5, QFont::DemiBold);
    //右侧 本地
    QLabel *pLabelManufacturer = new QLabel(tr("Vendor"), this);
    m_pManufacturerCombo = new DComboBox(this);
    m_pManufacturerCombo->setMaxVisibleItems(10);
    m_pManufacturerCombo->setEditable(true);
    m_pManufacturerCombo->setInsertPolicy(QComboBox::NoInsert);
    pLabelManufacturer->setAccessibleName("mfgLabel_installDriver");
    m_pManufacturerCombo->setAccessibleName("mfgCombo_installDriver");
    QHBoxLayout *pLocalHL1 = new QHBoxLayout();
    pLocalHL1->addWidget(pLabelManufacturer, 1);
    pLocalHL1->addWidget(m_pManufacturerCombo, 3);
    pLocalHL1->setContentsMargins(20, 8, 10, 8);
    QWidget *pLocalWidget1 = new QWidget(this);
    pLocalWidget1->setLayout(pLocalHL1);

    QLabel *pLabelType = new QLabel(tr("Model"), this);
    pLabelType->setAccessibleName("labelType_installDriver");
    m_pTypeCombo = new DComboBox(this);
    m_pTypeCombo->setEditable(true);
    m_pTypeCombo->setInsertPolicy(QComboBox::NoInsert);
    m_pTypeCombo->setAccessibleName("typeCombo_installDriver");
    QHBoxLayout *pLocalHL2 = new QHBoxLayout();
    pLocalHL2->addWidget(pLabelType, 1);
    pLocalHL2->addWidget(m_pTypeCombo, 3);
    pLocalHL2->setContentsMargins(20, 8, 10, 8);
    QWidget *pLocalWidget2 = new QWidget(this);
    pLocalWidget2->setLayout(pLocalHL2);

    QLabel *pLabelDriver = new QLabel(tr("Driver"), this);
    m_pDriverCombo = new DComboBox(this);
    m_pDriverCombo->setEditable(true);
    m_pDriverCombo->setInsertPolicy(QComboBox::NoInsert);
    m_pDriverCombo->setAccessibleName("driverCombo_installDriver");
    QHBoxLayout *pLocalHL3 = new QHBoxLayout();
    pLocalHL3->addWidget(pLabelDriver, 1);
    pLocalHL3->addWidget(m_pDriverCombo, 3);
    pLocalHL3->setContentsMargins(20, 8, 10, 8);
    QWidget *pLocalWidget3 = new QWidget(this);
    pLocalWidget3->setLayout(pLocalHL3);

    pLocalWidget1->setAccessibleName("localWidget1_installDriver");
    pLocalWidget2->setAccessibleName("localWidget2_installDriver");
    pLocalWidget3->setAccessibleName("localWidget3_installDriver");

    QVBoxLayout *pLocalVLayout = new QVBoxLayout();
    pLocalVLayout->addWidget(pLocalWidget1);
    pLocalVLayout->addWidget(pLocalWidget2);
    pLocalVLayout->addWidget(pLocalWidget3);
    pLocalVLayout->setContentsMargins(0, 0, 0, 0);

    DBackgroundGroup *pLocalWidget = new DBackgroundGroup(pLocalVLayout, this);
    pLocalWidget->setLayout(pLocalVLayout);
    pLocalWidget->setItemSpacing(1);
    pLocalWidget->setItemMargins(QMargins(0, 0, 0, 0));
    pLocalWidget->setBackgroundRole(this->backgroundRole());
    pLocalWidget->setAccessibleName("localWidget_installDriver");

    QVBoxLayout *pVLayoutLocal = new QVBoxLayout();
    pVLayoutLocal->addWidget(pLocalWidget, 0);
    pVLayoutLocal->addStretch();
    pVLayoutLocal->setMargin(0);
    QWidget *pLocalWidget4 = new QWidget(this);
    pLocalWidget4->setLayout(pVLayoutLocal);
    pLocalWidget4->setAccessibleName("localWidget4_installDriver");
    //将中间控件设置为DMainWindow的背景颜色
    m_pStackWidget->addWidget(pLocalWidget4);

    // PPD
    m_pPPDPath = new QLabel(UI_PRINTERDRIVER_PPDLABEL_NORMAL, this);
    DFontSizeManager::instance()->bind(m_pPPDPath, DFontSizeManager::T8, QFont::Normal);
    m_pPPDPath->installEventFilter(this);
    m_pPPDPath->setAcceptDrops(true);
    m_pPPDPath->setAlignment(Qt::AlignCenter);
    m_pPPDPath->setWordWrap(true);
    m_pPPDPath->setMinimumSize(200, 60);
    m_pPPDPath->setMaximumSize(300, 90);
    m_pPPDPath->setAccessibleName("ppdPath_installDriver");
    m_pSelectPPDBtn = new QPushButton(UI_PRINTERDRIVER_PPDBTN_NORMAL, this);
    m_pSelectPPDBtn->setAccessibleName("ppdBtn_installDriver");
    DFontSizeManager::instance()->bind(m_pPPDPath, DFontSizeManager::T6, QFont::Medium);
    QVBoxLayout *pVLayout = new QVBoxLayout(this);
    pVLayout->setContentsMargins(0, 80, 0, 80);
    pVLayout->addWidget(m_pPPDPath, 1, Qt::AlignCenter);
    pVLayout->addWidget(m_pSelectPPDBtn, 1, Qt::AlignCenter);

    DFrame *pPPDWidget = new DFrame(this);
    pPPDWidget->setObjectName("ppdWidget");
    pPPDWidget->setLayout(pVLayout);
    pPPDWidget->setBackgroundRole(this->backgroundRole());
    m_pStackWidget->addWidget(pPPDWidget);

    //设置打印信息搜索
    QLabel *pMakerAndTypeLabel = new QLabel(tr("Vendor and Model"), this);
    m_pManuAndTypeLineEdit = new QLineEdit(this);
    m_pManuAndTypeLineEdit->setToolTip(tr("Enter a complete vendor and model (Only letters, numbers and whitespaces)"));
    m_pManuAndTypeLineEdit->setValidator(new QRegExpValidator(QRegExp("^[a-zA-Z0-9 ]*$")));
    m_pManuAndTypeLineEdit->setAccessibleName("manuBtn_installDriver");
    m_pSearchBtn = new QPushButton(tr("Search", "button"), this);
    m_pSearchBtn->setMaximumWidth(105);
    m_pSearchBtn->setAccessibleName("searchBtn_installDriver");
    QHBoxLayout *pMakerHL1 = new QHBoxLayout();
    pMakerHL1->addWidget(pMakerAndTypeLabel, 1);
    pMakerHL1->addWidget(m_pManuAndTypeLineEdit, 3);
    pMakerHL1->addWidget(m_pSearchBtn, 1);
    pMakerHL1->setContentsMargins(20, 10, 10, 10);
    QWidget *pMakerWidget1 = new QWidget(this);
    pMakerWidget1->setLayout(pMakerHL1);
    pMakerWidget1->setFixedHeight(56);

    QLabel *pDriverLabel = new QLabel(tr("Driver"), this);
    pDriverLabel->setAccessibleName("driverLabel_installDriver");
    m_pDriverManualCombo = new DComboBox(this);
    m_pDriverManualCombo->setAccessibleName("manualCombo_installDriver");
    QHBoxLayout *pMakerHL2 = new QHBoxLayout();
    pMakerHL2->setSpacing(0);
    pMakerHL2->addWidget(pDriverLabel, 1);
    pMakerHL2->addSpacing(10);
    pMakerHL2->addWidget(m_pDriverManualCombo, 4);
    pMakerHL2->setContentsMargins(20, 10, 10, 10);
    QWidget *pMakerWidget2 = new QWidget(this);
    pMakerWidget2->setLayout(pMakerHL2);

    pMakerWidget1->setAccessibleName("makerWidget1_installDriver");
    pMakerWidget2->setAccessibleName("makerWidget2_installDriver");

    QVBoxLayout *pMakerVLayout1 = new QVBoxLayout();
    pMakerVLayout1->addWidget(pMakerWidget1);
    pMakerVLayout1->addWidget(pMakerWidget2);
    pMakerVLayout1->setContentsMargins(0, 0, 0, 0);

    DBackgroundGroup *pSettingWidget = new DBackgroundGroup(pMakerVLayout1, this);
    pSettingWidget->setLayout(pMakerVLayout1);
    pSettingWidget->setItemSpacing(1);
    pSettingWidget->setItemMargins(QMargins(0, 0, 0, 0));
    pSettingWidget->setBackgroundRole(this->backgroundRole());
    pSettingWidget->setAccessibleName("backGroup_installDriver");
    QVBoxLayout *pVLayoutMaker = new QVBoxLayout();
    pVLayoutMaker->addWidget(pSettingWidget, 0);
    pVLayoutMaker->addStretch();
    pVLayoutMaker->setMargin(0);
    QWidget *pSettingWidget1 = new QWidget(this);
    pSettingWidget1->setLayout(pVLayoutMaker);
    pSettingWidget1->setAccessibleName("settingWidget1_installDriver");
    m_pStackWidget->addWidget(pSettingWidget1);

    //安装按钮
    m_pInstallBtn = new QPushButton(tr("Install Driver", "button"), this);
    m_pInstallBtn->setFixedWidth(200);
    m_pSpinner = new DSpinner();
    m_pSpinner->setFixedSize(32, 32);

    m_pSearchSpinner = new DSpinner(m_pStackWidget);
    m_pSearchSpinner->setFixedSize(36, 36);
    m_pSearchSpinner->setVisible(false);
    m_pSearchSpinner->setAccessibleName("searchSpinner_stackWidget");

    m_pRightTitleLabel->setAccessibleName("titleLabel_installDriver");
    m_pStackWidget->setAccessibleName("stackWidget_installDriver");
    m_pInstallBtn->setAccessibleName("installBtn_installDriver");
    m_pSpinner->setAccessibleName("spinner_rightFrame");

    // 右侧整体布局
    QVBoxLayout *pRightVLayout = new QVBoxLayout();
    pRightVLayout->addWidget(m_pRightTitleLabel);
    pRightVLayout->addWidget(m_pStackWidget);
    pRightVLayout->addSpacing(20);
    pRightVLayout->addWidget(m_pInstallBtn, 0, Qt::AlignCenter);
    pRightVLayout->addWidget(m_pSpinner, 0, Qt::AlignCenter);
    pRightVLayout->setContentsMargins(10, 20, 10, 10);
    QWidget *pRightFrame1 = new QWidget(this);
    pRightFrame1->setLayout(pRightVLayout);
    pRightFrame1->setAccessibleName("rightFrame_installDriver");

    // 整体布局
    QHBoxLayout *pMainLayout = new QHBoxLayout();
    pMainLayout->addWidget(pLeftFrame, 1);
    pMainLayout->addWidget(pRightFrame1, 2);
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    DBackgroundGroup *pCentralWidget = new DBackgroundGroup;
    pCentralWidget->setAccessibleName("backWidget_centralWidget");
    pCentralWidget->setLayout(pMainLayout);
    pCentralWidget->setItemSpacing(2);
    QHBoxLayout *pMainLayout1 = new QHBoxLayout();
    pMainLayout1->addWidget(pCentralWidget);
    pMainLayout1->setContentsMargins(10, 10, 10, 10);
    QWidget *pCentralWidget1 = new QWidget(this);
    pCentralWidget1->setLayout(pMainLayout1);
    pCentralWidget1->setAccessibleName("centralWidget_installDriver");

    takeCentralWidget();
    setCentralWidget(pCentralWidget1);

    moveToCenter(this);
}

void InstallDriverWindow::initConnections()
{
    connect(m_pPreBtn, &DIconButton::clicked, this, [this]() {
        close();
        if (m_pParentWidget)
            m_pParentWidget->show();
    });
    connect(m_pTabListView, QOverload<const QModelIndex &>::of(&DListView::currentChanged), this, &InstallDriverWindow::tabCurrentIndexChanged);
    connect(m_pSelectPPDBtn, &QPushButton::clicked, this, &InstallDriverWindow::getPPDFileFromLocalSlot);
    connect(m_pInstallBtn, &QPushButton::clicked, this, &InstallDriverWindow::installDriverSlot);
    connect(m_pManufacturerCombo, &QComboBox::currentTextChanged, this, &InstallDriverWindow::currentMakerChangedSlot);
    connect(m_pTypeCombo, &QComboBox::currentTextChanged, this, &InstallDriverWindow::currentModelChangedSlot);
    connect(m_pSearchBtn, &QPushButton::clicked, this, &InstallDriverWindow::searchDriverSlot);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, [&]() {
        m_pLocalDriver->setSizeHint(DSizeModeHelper::element(QSize(108, 36), QSize(108, 48)));
        m_pPpdFile->setSizeHint(DSizeModeHelper::element(QSize(108, 36), QSize(108, 48)));
        m_pSearchDriver->setSizeHint(DSizeModeHelper::element(QSize(108, 36), QSize(108, 48)));
    });
#endif

    connect(m_pManuAndTypeLineEdit, &QLineEdit::editingFinished, this, [this]() {
        // 按下enter会触发两次信号，需要过滤掉失去焦点之后的信号 并且判断校验结果
        if (m_pManuAndTypeLineEdit->hasFocus())
            searchDriverSlot();
    });
}

void InstallDriverWindow::initMakerAndType()
{
    QStringList makerList = g_driverManager->getAllMakes();
    if (!makerList.isEmpty()) {
        int index = -1;
        makerList.sort(Qt::CaseInsensitive);
        m_pManufacturerCombo->clear();
        m_pManufacturerCombo->addItems(makerList);
        initCompleter(MANUFACTURER, makerList);
        //优先显示所选打印机对应的驱动
        if (!m_device.strMakeAndModel.isEmpty()) {
            QString strMake, strModel;
            ppdMakeModelSplit(m_device.strMakeAndModel, strMake, strModel);
            strMake = toNormalName(strMake);
            index = makerList.indexOf(strMake);
        }

        //如果没有和打印机匹配的就选择通用驱动
        if (0 > index) {
            QMap<QString, QString> textdriver = g_driverManager->getTextPPD();
            index = makerList.indexOf(textdriver[CUPS_PPD_MAKE]);
        }

        index = index > -1 ? index : 0;
        m_pManufacturerCombo->setCurrentIndex(index);
    }
}

void InstallDriverWindow::clearUserInfo()
{
    m_pPPDPath->setText(UI_PRINTERDRIVER_PPDLABEL_NORMAL);
    m_pSelectPPDBtn->setText(UI_PRINTERDRIVER_PPDBTN_NORMAL);

    m_pManuAndTypeLineEdit->clear();
    m_pDriverManualCombo->clear();
}

void InstallDriverWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    clearUserInfo();
    if (g_driverManager->getStatus() < TStat_Suc) {
        //提示本地驱动没有初始化完成
        connect(g_driverManager, &DriverManager::signalStatus, this, &InstallDriverWindow::driverRefreshSlot);
        m_pInstallBtn->setVisible(false);
        m_pSpinner->setVisible(true);
        m_pSpinner->start();
    } else {
        m_pSpinner->setVisible(false);
        m_pInstallBtn->setVisible(true);
        initMakerAndType();
    }
}

bool InstallDriverWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pPPDPath) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *pDragEE = static_cast<QDragEnterEvent *>(event);
            if (pDragEE) {
                if (pDragEE->mimeData()->hasFormat("text/uri-list")) {
                    if (pDragEE->mimeData()->urls().count() == 1) {
                        QString filePath = pDragEE->mimeData()->urls().at(0).toLocalFile();
                        QFileInfo info(filePath);
                        if (info.suffix() == "ppd") {
                            pDragEE->acceptProposedAction();
                            return true;
                        }
                    }
                }
            }
        } else if (event->type() == QEvent::Drop) {
            QDropEvent *pDropE = static_cast<QDropEvent *>(event);
            if (pDropE) {
                if (pDropE->mimeData()->hasFormat("text/uri-list")) {
                    QList<QUrl> urls = pDropE->mimeData()->urls();
                    if (urls.count() == 1) {
                        QString ppdFilePath = urls.at(0).toLocalFile();
                        if (!ppdFilePath.isEmpty()) {
                            m_pPPDPath->setText(ppdFilePath);
                            m_pSelectPPDBtn->setText(tr("Reselect"));
                            m_pInstallBtn->setEnabled(true);
                        } else {
                            m_pInstallBtn->setEnabled(false);
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void InstallDriverWindow::tabCurrentIndexChanged()
{
    if (m_pTabListView->currentIndex().row() == 0) {
        m_pStackWidget->setCurrentIndex(0);
        m_pRightTitleLabel->setText(tr("Choose a local driver"));
        if (m_pDriverCombo->count() == 0)
            m_pInstallBtn->setEnabled(false);
        else {
            m_pInstallBtn->setEnabled(true);
        }
    } else if (m_pTabListView->currentIndex().row() == 1) {
        m_pStackWidget->setCurrentIndex(1);
        m_pRightTitleLabel->setText(tr("Select a PPD file"));
        QFileInfo info(m_pPPDPath->text());
        if (info.exists()) {
            m_pInstallBtn->setEnabled(true);
        } else {
            m_pInstallBtn->setEnabled(false);
        }
    } else if (m_pTabListView->currentIndex().row() == 2) {
        m_pStackWidget->setCurrentIndex(2);
        m_pRightTitleLabel->setText(tr("Search for printer driver"));
        if (m_pDriverManualCombo->count() == 0)
            m_pInstallBtn->setEnabled(false);
        else {
            m_pInstallBtn->setEnabled(true);
        }
    }
}

void InstallDriverWindow::currentMakerChangedSlot(const QString &maker)
{
    const QMap<QString, QString> *modelset = g_driverManager->getModelsByMake(maker);
    if (modelset) {
        m_pTypeCombo->clear();
        // 去掉重复项
        QStringList modelList = modelset->keys().toSet().toList();
        modelList.sort(Qt::CaseInsensitive);
        m_pTypeCombo->addItems(modelList);
        initCompleter(TYPE, modelList);
        int index = -1;
        //优先选择打印机对应的驱动
        if (!m_device.strMakeAndModel.isEmpty()) {
            QString strMake, strModel;
            ppdMakeModelSplit(m_device.strMakeAndModel, strMake, strModel);
            index = modelList.indexOf(strModel);
        }

        /* 如果没有匹配的，则保留厂商，型号及驱动为空白，上面处理strModel时其实有问题的，
           大小写可能和modelList中不一样，导致index为-1。
        */

        m_pTypeCombo->setCurrentIndex(index);
    }
}

void InstallDriverWindow::currentModelChangedSlot(const QString &model)
{
    const QMap<QString, QString> *modelset = g_driverManager->getModelsByMake(m_pManufacturerCombo->currentText());
    if (modelset) {
        m_pDriverCombo->clear();

        QStringList ppdKeys = modelset->values(model);
        const QMap<QString, QMap<QString, QString>> *ppds = g_driverManager->getPPDs();
        QStringList strValues;

        foreach (QString key, ppdKeys) {
            QList<QMap<QString, QString>> list = ppds->values(key.toLower());
            for (int i = 0; i < list.count(); i++) {
                QString strPpd = list[i].value("ppd-name");
                QString ppdname = list[i].value("ppd-make-and-model");
                if (ppdname.isEmpty())
                    ppdname = strPpd;
                if (ppdname.contains("(recommended)")) {
                    ppdname.remove("(recommended)");
                }
                m_pDriverCombo->addItem(ppdname, QVariant::fromValue(list[i]));
                strValues.push_back(ppdname);
            }
        }

        initCompleter(DRIVER, strValues);
    }
    if (m_pDriverCombo->count() > 0) {
        m_pInstallBtn->setEnabled(true);
    } else {
        m_pInstallBtn->setEnabled(false);
    }
}

void InstallDriverWindow::getPPDFileFromLocalSlot()
{
    QString ppdFilePath = DFileDialog::getOpenFileName(this, tr("Select a PPD file"), "/home", "*.ppd");
    if (!ppdFilePath.isEmpty()) {
        m_pPPDPath->setText(ppdFilePath);
        m_pSelectPPDBtn->setText(tr("Reselect"));
    }
    m_pInstallBtn->setEnabled(!m_pPPDPath->text().isEmpty());
}

void InstallDriverWindow::installDriverSlot()
{
    QMap<QString, QVariant> solution;
    QMap<QString, QVariant> dataMap;

    if (m_pTabListView->currentIndex().row() == 0) {
        if (m_pDriverCombo->count() <= 0)
            return;
        solution = m_pDriverCombo->currentData().value<QMap<QString, QVariant>>();
        int driverCount = m_pDriverCombo->count();
        for (int i = 0; i < driverCount; ++i) {
            if (i != m_pDriverCombo->currentIndex()) {
                dataMap.insert(m_pDriverCombo->itemText(i), m_pDriverCombo->itemData(i));
            }
        }
    } else if (m_pTabListView->currentIndex().row() == 1) {
        QFileInfo info(m_pPPDPath->text());
        if (!info.exists()) {
            return;
        }
        solution.insert(SD_KEY_from, PPDFrom_File);
        solution.insert(CUPS_PPD_NAME, m_pPPDPath->text());
    } else if (m_pTabListView->currentIndex().row() == 2) {
        if (m_pDriverManualCombo->count() <= 0)
            return;
        solution = m_pDriverManualCombo->currentData().value<QMap<QString, QVariant>>();
        m_device.strMakeAndModel = m_pManuAndTypeLineEdit->text();
        int driverCount = m_pDriverManualCombo->count();
        for (int i = 0; i < driverCount; ++i) {
            if (i != m_pDriverManualCombo->currentIndex()) {
                dataMap.insert(m_pDriverManualCombo->itemText(i), m_pDriverManualCombo->itemData(i));
            }
        }
    }

    AddPrinterTask *task = g_addPrinterFactoty->createAddPrinterTask(m_device, solution);
    InstallPrinterWindow *pInstallPrinterWindow = new InstallPrinterWindow(this);
    connect(pInstallPrinterWindow, &InstallPrinterWindow::showParentWindows, this, [&]() {
        this->show();
    });
    pInstallPrinterWindow->setTask(task);
    pInstallPrinterWindow->setDevice(m_device);

    pInstallPrinterWindow->copyDriverData(dataMap);
    close();
    pInstallPrinterWindow->show();
    connect(task, &AddPrinterTask::signalStatus, pInstallPrinterWindow, &InstallPrinterWindow::receiveInstallationStatusSlot);
    PrinterSearchWindow *pSearch = static_cast<PrinterSearchWindow *>(m_pParentWidget);
    if (pSearch)
        connect(pInstallPrinterWindow, &InstallPrinterWindow::updatePrinterList, pSearch, &PrinterSearchWindow::updatePrinterList);
    task->doWork();
}

void InstallDriverWindow::searchDriverSlot()
{
    m_pSearchSpinner->move((m_pStackWidget->width() - m_pSearchSpinner->width()) / 2,
                            (m_pStackWidget->height() - m_pSearchSpinner->height()) / 2 + 65);
    m_pSearchSpinner->setVisible(true);
    m_pSearchSpinner->start();

    if (m_pManuAndTypeLineEdit->text().isEmpty()) {
        m_pSearchSpinner->stop();
        m_pSearchSpinner->setVisible(false);
        return;
    }

    m_pSearchBtn->blockSignals(true);
    // 拷贝结构体数据，避免修改原始数据
    TDeviceInfo device = m_device;
    device.strMakeAndModel = m_pManuAndTypeLineEdit->text();
    device.strDeviceId.clear();
    DriverSearcher *pDriverSearcher = g_driverManager->createSearcher(device);
    connect(pDriverSearcher, &DriverSearcher::signalDone, this, &InstallDriverWindow::driverSearchedSlot);
    pDriverSearcher->startSearch();
}

void InstallDriverWindow::driverRefreshSlot(int id, int iState)
{
    Q_UNUSED(id)
    if (iState == TStat_Suc) {
        m_pSpinner->stop();
        m_pSpinner->setVisible(false);
        m_pInstallBtn->setVisible(true);
        initMakerAndType();
    }
}

QCompleter *InstallDriverWindow::initCompleter(COMPLETERNAME name, const QStringList &strList)
{
    QCompleter *pCompleter = new QCompleter(strList, this);
    pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    pCompleter->setFilterMode(Qt::MatchContains);
    pCompleter->setCompletionMode(QCompleter::PopupCompletion);
    pCompleter->popup()->setItemDelegate(new QtCompleterDelegate(pCompleter));

    if (MANUFACTURER == name) {
        m_pManufacturerCombo->setCompleter(pCompleter);

        if (m_pManufactureCompleter != nullptr) {
            delete m_pManufactureCompleter;
        }

        m_pManufactureCompleter = pCompleter;
    } else if (TYPE == name) {
        m_pTypeCombo->setCompleter(pCompleter);

        if (m_pTypeCompleter != nullptr) {
            delete m_pTypeCompleter;
        }

        m_pTypeCompleter = pCompleter;
    } else if (DRIVER == name) {
        m_pDriverCombo->setCompleter(pCompleter);

        if (m_pDriverCompleter != nullptr) {
            delete m_pDriverCompleter;
        }

        m_pDriverCompleter = pCompleter;
    }

    return pCompleter;
}

void InstallDriverWindow::driverSearchedSlot()
{
    m_pDriverManualCombo->clear();
    DriverSearcher *pDriverSearcher = static_cast<DriverSearcher *>(sender());
    if (pDriverSearcher) {
        QList<QMap<QString, QVariant>> drivers = pDriverSearcher->getDrivers();
        pDriverSearcher->deleteLater();
        foreach (auto driver, drivers) {
            //将驱动结构体存在item中，方便后续安装打印机
            QString strDesc;
            if (driver.value("ppd-make-and-model").isNull())
                strDesc = driver.value("ppd-make").toString() + " " + driver.value("ppd-model").toString();
            else
                strDesc = driver.value("ppd-make-and-model").toString();

            bool isExcat = true;
            QString strReplace;
            //本地都是精确匹配，服务器获取可能不是精确匹配
            if (driver.value(SD_KEY_from).toInt() == PPDFrom_Server) {
                isExcat = driver.value("excat").toBool();
            }
            if (isExcat)
                strReplace = tr(" (recommended)");
            //去掉自带的recommended字段
            strDesc.replace("(recommended)", "");
            //如果是精确匹配添加 推荐 字段
            strDesc += strReplace;
            m_pDriverManualCombo->addItem(strDesc, QVariant::fromValue(driver));
        }
        if (m_pDriverManualCombo->count() == 0) {
            m_pInstallBtn->setEnabled(false);
        } else {
            m_pInstallBtn->setEnabled(true);
        }
    }

    m_pSearchSpinner->stop();
    m_pSearchSpinner->setVisible(false);
    m_pSearchBtn->blockSignals(false);
}

void InstallDriverWindow::setDeviceInfo(const TDeviceInfo &device)
{
    m_device = device;
}

void InstallDriverWindow::setPreWidget(QWidget *parent)
{
    m_pParentWidget = parent;
}
