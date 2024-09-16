// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTERSSHOWWINDOW_H
#define DPRINTERSSHOWWINDOW_H
#include "dprintermanager.h"
#include "printersearchwindow.h"
#include "renameprinterwindow.h"
#include "qtconvert.h"
#include "util/refreshsnmpbackendtask.h"
#include "printerhelpwindow.h"

#include <DMainWindow>
#include <DListView>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DFontSizeManager>
#include <DFrame>
#include <DBackgroundGroup>
#include <DGroupBox>

#include <QCheckBox>
#include <QVBoxLayout>
#include <QEventLoop>
#include <QDBusMessage>
#include <QCloseEvent>
#include <QMouseEvent>
DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DImageButton;
class DSettingsDialog;
class DDialog;
class DFloatingButton;
class DToolButton;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QLabel;
class QStandardItemModel;
class QMenu;
class QCheckBox;
class QLineEdit;
QT_END_NAMESPACE

#include <QWidgetAction>

class SwitchWidget;
class AdvanceShareWidget;

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent) {
    }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override {
        QLabel::mousePressEvent(event);
        emit clicked();
    }
};

class ServerSettingsWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit ServerSettingsWindow(QWidget *parent = nullptr)
        : DMainWindow(parent)
    {
        initUI();
        initConnections();
    }
    int exec()
    {
        show();
        QEventLoop loop;
        connect(this, &ServerSettingsWindow::finished, &loop, &QEventLoop::quit);
        loop.exec();
        return 1;
    }

private:
    void initUI()
    {
        titlebar()->setMenuVisible(false);
        titlebar()->setTitle("");
        titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
        titlebar()->setAccessibleName("titleBar_settingsWindow");
        setWindowFlags(Qt::Dialog);

        setWindowModality(Qt::ApplicationModal);
        this->setMinimumSize(480, 336);
        this->setMaximumSize(480, 450);

        QLabel *pBaseSettings = new QLabel(tr("Basic Server Settings"));
        pBaseSettings->setAccessibleName("baseSetting_mainSetting");
        DFontSizeManager::instance()->bind(pBaseSettings, DFontSizeManager::T5, QFont::DemiBold);
        QHBoxLayout *pHLayout = new QHBoxLayout();
        pHLayout->setSpacing(0);
        pHLayout->setContentsMargins(0, 0, 0, 0);
        pHLayout->addWidget(pBaseSettings);

        QString checkShared = tr("Publish shared printers connected to this system");
        m_pCheckShared = new QCheckBox();
        m_pCheckIPP = new QCheckBox(tr("Allow printing from the Internet"));
        m_pCheckIPP->setEnabled(false);

        QLabel *shareInfo = new QLabel(tr("To share your printer, turn on these settings"));
        DFontSizeManager::instance()->bind(shareInfo, DFontSizeManager::T8, QFont::Light);
        QPalette pal = shareInfo->palette();
        pal.setColor(QPalette::WindowText, QColor("#00000060"));
        shareInfo->setPalette(pal);

        m_pCheckRemote = new QCheckBox(tr("Allow remote administration"));
        //        m_pCheckCancelJobs = new QCheckBox(tr("Allow users to cancel all tasks (not just their own)"));
        QString checkSaveDebugInfo = tr("Save debugging information for troubleshooting");
        m_pCheckSaveDebugInfo = new QCheckBox();
        QString checkSaveDebugTipsInfo = checkSaveDebugInfo;
        geteElidedText(m_pCheckSaveDebugInfo->font(), checkSaveDebugInfo, m_pCheckSaveDebugInfo->width() - 240);
        m_pCheckSaveDebugInfo->setText(checkSaveDebugInfo);
        m_pCheckSaveDebugInfo->setToolTip(checkSaveDebugTipsInfo);

        QString showCheckShared = checkShared;
        geteElidedText(m_pCheckShared->font(), showCheckShared, m_pCheckShared->width() - 240);
        m_pCheckShared->setText(showCheckShared);
        m_pCheckShared->setToolTip(checkShared);

        m_pCheckShared->setAccessibleName("checkShare_frame1");
        m_pCheckIPP->setAccessibleName("checkIpp_frame1");
        m_pCheckRemote->setAccessibleName("checkRemote_frame2");
        m_pCheckSaveDebugInfo->setAccessibleName("checkSaveInfo_frame3");

        QVBoxLayout *pSettingsVLayout = new QVBoxLayout();
        pSettingsVLayout->setSpacing(0);
        pSettingsVLayout->setContentsMargins(0, 0, 0, 0);

        QVBoxLayout *pSettingsVLayout1 = new QVBoxLayout();
        pSettingsVLayout1->addWidget(m_pCheckShared);
        QHBoxLayout *pSettingsHLayout = new QHBoxLayout();
        pSettingsHLayout->addSpacing(20);
        pSettingsHLayout->addWidget(m_pCheckIPP);
        pSettingsVLayout1->addLayout(pSettingsHLayout);
        QHBoxLayout *pSettingsInfoHLayout = new QHBoxLayout();
        pSettingsInfoHLayout->addSpacing(5);
        pSettingsInfoHLayout->addWidget(shareInfo);
        pSettingsInfoHLayout->setContentsMargins(0, 0, 0, 0);
        pSettingsVLayout1->addLayout(pSettingsInfoHLayout);

        QWidget *pFrame1 = new QWidget();
        pFrame1->setMinimumSize(440, 108);
        pFrame1->setMaximumSize(440, 158);
        pFrame1->setLayout(pSettingsVLayout1);
        pFrame1->setContentsMargins(0, 0, 0, 0);
        pFrame1->setAccessibleName("frame1_SettingWidget");
        pSettingsVLayout->addWidget(pFrame1);

        QVBoxLayout *pSettingsVLayout2 = new QVBoxLayout();
        pSettingsVLayout2->addWidget(m_pCheckRemote);
        QWidget *pFrame2 = new QWidget();
        pFrame2->setLayout(pSettingsVLayout2);
        pFrame2->setMaximumHeight(50);
        pFrame2->setContentsMargins(0, 0, 0, 0);
        pFrame2->setAccessibleName("frame2_SettingWidget");
        pSettingsVLayout->addWidget(pFrame2);

        //        pSettingsVLayout->addWidget(m_pCheckCancelJobs);

        QVBoxLayout *pSettingsVLayout3 = new QVBoxLayout();
        pSettingsVLayout3->addWidget(m_pCheckSaveDebugInfo);
        QWidget *pFrame3 = new QWidget();
        pFrame3->setMaximumHeight(50);
        pFrame3->setLayout(pSettingsVLayout3);
        pFrame3->setContentsMargins(0, 0, 0, 0);
        pFrame3->setAccessibleName("frame3_SettingWidget");
        pSettingsVLayout->addWidget(pFrame3);

        DBackgroundGroup *pSettingWidget = new DBackgroundGroup();
        pSettingWidget->setLayout(pSettingsVLayout);
        pSettingWidget->setItemSpacing(1);
        pSettingWidget->setBackgroundRole(this->backgroundRole());
        pSettingWidget->setAccessibleName("SettingWidget_mainSetting");

        DFrame *pSettingWidget1 = new DFrame(this);
        QVBoxLayout *pMainVlaout1 = new QVBoxLayout();
        pMainVlaout1->addLayout(pHLayout);
        pMainVlaout1->addSpacing(10);
        pMainVlaout1->addWidget(pSettingWidget);
        pMainVlaout1->setContentsMargins(10, 10, 10, 10);
        pSettingWidget1->setLayout(pMainVlaout1);
        pSettingWidget1->setAccessibleName("mainSetting_settingsWindow");

        QHBoxLayout *pMainLayout1 = new QHBoxLayout();
        pMainLayout1->addWidget(pSettingWidget1);
        pMainLayout1->setContentsMargins(10, 10, 10, 10);
        QWidget *pCentralWidget = new QWidget();
        pCentralWidget->setLayout(pMainLayout1);
        pCentralWidget->setAccessibleName("centralWidget_settingsWindow");

        takeCentralWidget();
        setCentralWidget(pCentralWidget);
        moveToCenter(this);
        this->setAccessibleName("settingsWindow_mainWindow");
    }
    void initConnections()
    {
        connect(m_pCheckShared, &QCheckBox::clicked, this, [this](bool checked) {
            if (checked) {
                m_pCheckIPP->setEnabled(true);
            } else {
                m_pCheckIPP->setEnabled(false);
                m_pCheckIPP->setChecked(false);
            }
        });
    }

protected:
    void closeEvent(QCloseEvent *event) override
    {
        emit finished();
        event->accept();
    }

public:
    QCheckBox *m_pCheckShared;
    QCheckBox *m_pCheckIPP;
    QCheckBox *m_pCheckRemote;
    //    QCheckBox *m_pCheckCancelJobs;
    QCheckBox *m_pCheckSaveDebugInfo;
signals:
    void finished();
};

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ItemDelegate(QObject *parent = nullptr);
    virtual ~ItemDelegate() override;


protected:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class PrinterListView : public DListView
{
    Q_OBJECT
public:
    explicit PrinterListView(QWidget *parent = nullptr)
        : DListView(parent)
    {
    }
    virtual ~PrinterListView() override {}

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        QModelIndex index = indexAt(e->pos());
        if (!index.isValid()) {
            if (currentIndex().isValid() && isPersistentEditorOpen(currentIndex())) {
                if (indexWidget(currentIndex()))
                    commitData(indexWidget(currentIndex()));
                closePersistentEditor(currentIndex());
            }
        }
        DListView::mousePressEvent(e);
    }
};

class BackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BackgroundWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class DPrintersShowWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit DPrintersShowWindow(const QString &selectedPrinter = "", QWidget *parent = nullptr);
    virtual ~DPrintersShowWindow() override;

private:
    // 初始化UI
    void initUI();
    // 初始化信号槽连接
    void initConnections();
    /**
    * @projectName   Printer
    * @brief         过滤QListWidget默认代理的右键事件
    * @author        liurui
    * @date          2019-11-08
    */

    void showEvent(QShowEvent *event) override;

    /**
    * @projectName   Printer
    * @brief         选中指定名称的打印机列表项
    * @author        liurui
    * @date          2019-11-07
    */
    void selectPrinterByName(const QString &printerName);
    /**
    * @projectName   Printer
    * @brief         更新打印机列表的默认选项图标
    * @author        liurui
    * @date          2019-11-08
    */
    void updateDefaultPrinterIcon();

    /**
    * @projectName   Printer
    * @brief         按照耗材余量加载相应的图标
    */
    QIcon getSupplyIconByLevel(int iLevel);
private slots:
    // 添加打印机
    void addPrinterClickSlot();
    // 删除打印机
    void deletePrinterClickSlot();
    /**
    * @projectName   Printer
    * @brief         重命名打印机，策略是先复制打印机，然后删除当前打印机，最后启用新打印机
    * @author        liurui
    * @date          2019-11-07
    */
    void renamePrinterSlot(QStandardItem *pItem);
    // 打印机名称可编辑状态
    void setPrinterNameEditable(const QModelIndex &index);

    void printSettingClickSlot();
    void printQueueClickSlot();
    void printTestClickSlot();
    void printFalutClickSlot();
    void printSupplyClickSlot();
    void supplyFreshed(const QString &, bool);

    void printerListWidgetItemChangedSlot(const QModelIndex &previous);
    // 响应列表的右键菜单
    void contextMenuRequested(const QPoint &point);
    // 响应菜单栏的action
    void listWidgetMenuActionSlot(bool checked);

    // 初始化左侧打印机列表
    void refreshPrinterListView(const QString &newPrinterName);
    // 服务器设置
    void serverSettingsSlot();

    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

    void resizeEvent(QResizeEvent *event) override;

    /*已经添加的打印机状态变化*/
    void printerStateChanged(const QDBusMessage &msg);

    /*正在后台自动添加的打印机状态变化*/
    void deviceStatusChanged(const QDBusMessage &msg);
    void changeEvent(QEvent *event) override;

    void showAdminToolsSlot();

private:
    // UI成员变量
    QPushButton *m_pBtnAddPrinter;
    DToolButton *m_pBtnDeletePrinter;
    QLabel *m_pLeftTipLabel;

    QLabel *m_pLabelPrinterName;
    QLabel *m_pLabelLocationShow;
    QLabel *m_pLabelTypeShow;
    QLabel *m_pLabelStatusShow;
    QLabel *m_pLabelPrintTest;
    QLabel *m_pLabelPrintFault;
    QLabel *m_pLabelPrintQueue;

    DIconButton *m_pIBtnSetting;
    DIconButton *m_pIBtnPrintQueue;
    DIconButton *m_pIBtnPrintTest;
    DIconButton *m_pIBtnFault;
    DIconButton *m_pIBtnSupply;

    PrinterListView *m_pPrinterListView;
    DSpinner *m_pLoading;

    QStandardItemModel *m_pPrinterModel;
    QMenu *m_pListViewMenu;
    QAction *m_pPrinterRename;
    QAction *m_pDefaultAction;
    QAction *m_pDeleteAction;
    SwitchWidget *m_pSwitchShareButton;
    AdvanceShareWidget *m_pAdvancedshare;
    AdvanceShareWidget *m_pAdminTool;
    QCheckBox *m_pDefaultPrinter;

    QWidget *m_pPrinterInfoWidget;
    QLabel *m_pPRightTipLabel1;
    QLabel *m_pPRightTipLabel2;
    QLabel *m_pShareIpAddr;
    ClickableLabel *m_pShareCopyIpAddr;
    QWidget *m_pShareWidget;
    BackgroundWidget *m_pRightTopWidget;
    BackgroundWidget *m_pShareConfig;

    PrinterSearchWindow *m_pSearchWindow;
    ServerSettingsWindow *m_pSettingsDialog;
    CustomLabel *m_customLabel;
    QAction *m_pSettings;

private:
    // 数据成员变量
    DPrinterManager *m_pPrinterManager;
    // 当前选中的打印机名称
    QString m_CurPrinterName;
    // 当前正在添加的打印机名称,可能存在多个
    QStringList m_ConfigingPrinterNameList;

    bool m_IsFirstShow;

};

#endif // DPRINTERSSHOWWINDOW_H
