// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "speedwidget.h"
#include "../src/widgets/tipwidget.h"
#include "window/modules/common/common.h"
#include "window/modules/common/gsettingkey.h"
#include "window/modules/common/compixmap.h"

#include <DPinyin>
#include <DComboBox>
#include <DPushButton>
#include <DFloatingMessage>
#include <DMessageManager>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>

// 表格中每列索引
const int AppNameDispCol = 0;
const int DownloadSpeedDispCol = 1;
const int UploadSpeedDispCol = 2;
const int NetContrDispCol = 3;
const int ProcIsRunningCol = 4;
const int PidCol = 5;
const int IsbSysAppCol = 6;
const int ProcNameCol = 7;
const int DesktopCol = 8;
const int AppNameCol = 9;
const int IconCol = 10;
const int DownloadSpeedCol = 11;
const int UploadSpeedCol = 12;
const int NetStatus = 13;

#define COL_APP_NAME 220 // 第一列 appmingc
#define COL_DOWNLOAD_SPEED 215 // 第二列 下载网速
#define COL_UPLOAD_SPEED 150 // 第三列 上传网速
#define COL_NET_CONTROL 150 // 第四列 联网控制
#define COL_LEFT_ADD_SPACE 10 // 左侧增加10间隔，保持与表头标题一致

#define SPLIT_FLAG_1 "|" // 分割符 |
#define SPLIT_FLAG_2 "*" // 分割符 *

SpeedWidget::SpeedWidget(DataUsageModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
    , m_layout(new QVBoxLayout)
    , m_totalSpeedLabel(nullptr)
    , m_speedtableview(new QTableView(this))
    , m_speeditemmodel(new QStandardItemModel(this))
    , m_speedTableWidget(nullptr)
    , m_speeditem(new SpeedItemDelegate(this, this))
    , m_waitSpinner(new DSpinner(this))
{
    this->setAccessibleName("dataUsageWidget_speedWidget");
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(10);
    setLayout(m_layout);

    // 总速度标签
    m_totalSpeedLabel = new DTipLabel("↓" + tr("Total download") + ": 0.00KB/s" + "  " + "↑" + tr("Total upload") + ": 0.00KB/s", this);
    m_totalSpeedLabel->setAccessibleName("speedWidget_totalSpeedLable");
    m_totalSpeedLabel->setAlignment(Qt::AlignVCenter);
    m_layout->addWidget(m_totalSpeedLabel, 0, Qt::AlignLeft);

    // 表格
    m_speedTableWidget = new DefenderTable(this);
    m_speedTableWidget->setAccessibleName("speedWidget_speedTableWidget");
    m_speedTableWidget->setHeadViewSortHide(3);
    m_speedTableWidget->setTableAndModel(m_speedtableview, m_speeditemmodel);
    m_layout->addWidget(m_speedTableWidget, 1 /*, Qt::AlignCenter*/);
    m_speedtableview->setAccessibleName("speedTableWidget_speedTableView");

    // 等待界面
    m_waitSpinner->setFixedSize(30, 30);
    m_waitSpinner->setVisible(false);
    m_layout->addWidget(m_waitSpinner, 1, Qt::AlignCenter);

    SetSpeedAttributes();
    updateTable();

    // 添加流量监控未开启提示栏
    m_tipwidSwitchOff = new DWidget(this);
    m_tipwidSwitchOff->setContentsMargins(0, 0, 0, 0);
    m_tipwidgetSwitchOff = new TipWidget;
    m_tipwidgetSwitchOff->setIcon(QIcon::fromTheme(DIALOG_TIP_RED));
    m_tipwidgetSwitchOff->setTitle(tr("Its functions are disabled, please turn it on in Protection"));
    QHBoxLayout *layoutSwitchOff = new QHBoxLayout;
    layoutSwitchOff->setContentsMargins(30, 0, 30, 0);
    layoutSwitchOff->addWidget(m_tipwidgetSwitchOff, 0, Qt::AlignCenter);
    m_tipwidSwitchOff->setLayout(layoutSwitchOff);
    m_tipwidSwitchOff->setVisible(false);
    m_layout->addWidget(m_tipwidSwitchOff);

    // 初始化判断是否显示流量监控开关未开启提示
    intiSwitchStatus();

    // 初始化表格数据
    recAllProcInfos(m_model->getProcInfoList());
    connect(m_model, &DataUsageModel::sendProcInfoList, this, &SpeedWidget::recAllProcInfos);
}

SpeedWidget::~SpeedWidget()
{
    m_speeditemmodel->clear();
    m_speeditemmodel->deleteLater();
    m_speeditemmodel = nullptr;
}

//更新表格数据
void SpeedWidget::updateTable()
{
    // 总网速
    double dTotalDownloadSpeed = 0.0;
    double dTotalUploadSpeed = 0.0;

    if (m_mergedProcInfos.size() == 0) {
        //设置鼠标等待界面
        m_waitSpinner->setVisible(true);
        m_waitSpinner->start();
        m_speedTableWidget->setVisible(false);
    } else {
        //恢复显示界面
        m_waitSpinner->setVisible(false);
        m_waitSpinner->stop();
        m_speedTableWidget->setVisible(true);
    }

    // 设置当前列表中所有行的 应用是否在运行项 为 "false"
    for (int i = 0; i < m_speeditemmodel->rowCount(); i++) {
        m_speeditemmodel->setData(m_speeditemmodel->index(i, ProcIsRunningCol), "false");
    }

    // 更新已存在进程数据 或 添加新进程数据
    int currentRowCount = m_speeditemmodel->rowCount();
    for (int i = 0; i < currentRowCount; i++) {
        // 获取当前行的进程号
        int nProcPid = m_speeditemmodel->data(m_speeditemmodel->index(i, PidCol), Qt::ItemDataRole::DisplayRole).toInt();
        for (int j = 0; j < m_mergedProcInfos.size(); j++) {
            if (nProcPid == m_mergedProcInfos.at(j).nPid) {
                DefenderProcInfo procInfoTmp = m_mergedProcInfos.at(j);

                // 设置列表中当前行的 应用是否在运行项 为 "true"
                m_speeditemmodel->setData(m_speeditemmodel->index(i, ProcIsRunningCol), "true");
                // 更新是否是系统应用
                m_speeditemmodel->setData(m_speeditemmodel->index(i, IsbSysAppCol), procInfoTmp.isbSysApp);
                // 更新进程名
                m_speeditemmodel->setData(m_speeditemmodel->index(i, ProcNameCol), procInfoTmp.sProcName);
                // 更新desktop
                m_speeditemmodel->setData(m_speeditemmodel->index(i, DesktopCol), procInfoTmp.sPkgName);
                // 更新应用名
                QString appName = procInfoTmp.sAppName.isEmpty() ? procInfoTmp.sProcName : procInfoTmp.sAppName;
                m_speeditemmodel->setData(m_speeditemmodel->index(i, AppNameCol), appName);
                // 更新当前行图标数据
                m_speeditemmodel->setData(m_speeditemmodel->index(i, IconCol), procInfoTmp.sThemeIcon);
                // 更新当前行网速数据
                QString sDownloadSpeed = Utils::changeFlowValueUnit(procInfoTmp.dDownloadSpeed, 2).append("/s");
                QString sUploadSpeed = Utils::changeFlowValueUnit(procInfoTmp.dUploadSpeed, 2).append("/s");
                m_speeditemmodel->setData(m_speeditemmodel->index(i, DownloadSpeedCol), sDownloadSpeed);
                m_speeditemmodel->setData(m_speeditemmodel->index(i, UploadSpeedCol), sUploadSpeed);

                // 设置名称显示列，用于排序
                m_speeditemmodel->setData(m_speeditemmodel->index(i, AppNameDispCol), DTK_CORE_NAMESPACE::Chinese2Pinyin(appName), Qt::ItemDataRole::EditRole);

                // 原始单位为KB/s的网速大小，用于排序
                m_speeditemmodel->item(i, DownloadSpeedDispCol)->setData(procInfoTmp.dDownloadSpeed, Qt::ItemDataRole::EditRole);
                m_speeditemmodel->item(i, UploadSpeedDispCol)->setData(procInfoTmp.dUploadSpeed, Qt::ItemDataRole::EditRole);

                // 找到已存在于表格中的进程项，提取数据后，从列表中移除该项
                m_mergedProcInfos.removeAt(j);
                // 统计总网速
                dTotalDownloadSpeed += procInfoTmp.dDownloadSpeed;
                dTotalUploadSpeed += procInfoTmp.dUploadSpeed;
                break;
            }
        }
    }

    // 移除表格中没有运行的进程对应的行
    for (int i = 0; i < m_speeditemmodel->rowCount();) {
        if ("false" == m_speeditemmodel->index(i, ProcIsRunningCol).data().toString()) {
            m_speeditemmodel->removeRow(i);
        } else {
            i++;
        }
    }

    // 添加新进程数据
    for (DefenderProcInfo procFlowInfo : m_mergedProcInfos) {
        QStandardItem *nameItem = new QStandardItem;
        QStandardItem *downloadSpeedItem = new QStandardItem;
        QStandardItem *uploadSpeedItem = new QStandardItem;
        QStandardItem *settingItem = new QStandardItem;

        QList<QStandardItem *> itemList;
        itemList << nameItem << downloadSpeedItem << uploadSpeedItem << settingItem;
        m_speeditemmodel->insertRow(0, itemList);

        // ==========设置储存数据========
        // 设置进程号
        m_speeditemmodel->setData(m_speeditemmodel->index(0, PidCol), procFlowInfo.nPid);
        // 设置列表中当前行的 应用是否在运行项 为 "true"
        m_speeditemmodel->setData(m_speeditemmodel->index(0, ProcIsRunningCol), "true");
        // 更新是否是系统应用
        m_speeditemmodel->setData(m_speeditemmodel->index(0, IsbSysAppCol), procFlowInfo.isbSysApp);
        // 更新进程名
        m_speeditemmodel->setData(m_speeditemmodel->index(0, ProcNameCol), procFlowInfo.sProcName);
        // 更新desktop
        m_speeditemmodel->setData(m_speeditemmodel->index(0, DesktopCol), procFlowInfo.sPkgName);
        // 更新应用名
        m_speeditemmodel->setData(m_speeditemmodel->index(0, AppNameCol), procFlowInfo.sAppName);
        // 更新当前行图标数据
        m_speeditemmodel->setData(m_speeditemmodel->index(0, IconCol), procFlowInfo.sThemeIcon);
        // 更新当前行网速数据
        QString sDownloadSpeed = Utils::changeFlowValueUnit(procFlowInfo.dDownloadSpeed, 2).append("/s");
        QString sUploadSpeed = Utils::changeFlowValueUnit(procFlowInfo.dUploadSpeed, 2).append("/s");
        m_speeditemmodel->setData(m_speeditemmodel->index(0, DownloadSpeedCol), sDownloadSpeed);
        m_speeditemmodel->setData(m_speeditemmodel->index(0, UploadSpeedCol), sUploadSpeed);
        // ==========设置储存数据-end========

        // ==========设置显示数据=========
        // 设置应用名称，用于排序
        // 如果能够获取到应用名，就取应用名；否则，取进程名
        QString appName = procFlowInfo.sAppName;
        m_speeditemmodel->setData(m_speeditemmodel->index(0, AppNameDispCol), DTK_CORE_NAMESPACE::Chinese2Pinyin(appName), Qt::ItemDataRole::EditRole);
        // 原始单位为KB/s的网速大小，用于排序
        m_speeditemmodel->item(0, DownloadSpeedDispCol)->setData(procFlowInfo.dDownloadSpeed, Qt::ItemDataRole::EditRole);
        m_speeditemmodel->item(0, UploadSpeedDispCol)->setData(procFlowInfo.dUploadSpeed, Qt::ItemDataRole::EditRole);

        // 包名不为空时加载下拉框控制联网状态
        if (!procFlowInfo.sPkgName.isEmpty()) {
            m_appNamefromPkgName[procFlowInfo.sPkgName] = appName;
            // 表格内加载下拉框
            DComboBox *box = new DComboBox;
            box->addItem(tr("Ask"), NET_STATUS_ITEMASK);
            box->addItem(tr("Allow"), NET_STATUS_ITEMALLOW);
            box->addItem(tr("Disable"), NET_STATUS_ITEMDISALLOW);

            // 添加到布局 并将box插入容器保存
            DWidget *boxw = new DWidget;
            QHBoxLayout *boxLayout = new QHBoxLayout;
            boxLayout->setContentsMargins(6, 6, 20, 6);
            boxLayout->addWidget(box);
            boxw->setLayout(boxLayout);
            m_comboToAppName.insert(box, procFlowInfo.sPkgName);
            m_speedtableview->setIndexWidget(m_speeditemmodel->index(0, 3), boxw);

            // 设置流量详情控制初始值
            int nStatus = 0;
            // 获取所有APP联网选项
            QString sNetAppsOption = m_model->GetNetControlAppStatus();

            // 初始化时拆分本地保存项，用QMap接收
            QStringList sNetAppsOptionList = sNetAppsOption.split(SPLIT_FLAG_1);
            for (int i = 0; i < sNetAppsOptionList.count(); ++i) {
                QStringList sNetAppsList = sNetAppsOptionList.at(i).split(SPLIT_FLAG_2);

                // 保证拆分的总数必须为2,否则崩溃
                if ((sNetAppsList.count() >= 2)
                    && (sNetAppsList.at(0) == procFlowInfo.sPkgName)) {
                    nStatus = sNetAppsList.at(1).toInt();
                }
            }

            // 用于自动化标签添加,无其他用途
            m_speeditemmodel->setData(m_speeditemmodel->index(0, NetStatus), nStatus);

            box->setCurrentIndex(nStatus);
            connect(box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SpeedWidget::onAppOptChanged);
        }

        // 统计总网速
        dTotalDownloadSpeed += procFlowInfo.dDownloadSpeed;
        dTotalUploadSpeed += procFlowInfo.dUploadSpeed;
    }

    // 根据当前排序状态排序
    Qt::SortOrder sortOrder = m_speedtableview->horizontalHeader()->sortIndicatorOrder();
    int sortCol = m_speedtableview->horizontalHeader()->sortIndicatorSection();
    m_speedtableview->sortByColumn(sortCol, sortOrder);

    // 更新总网速标签
    QString sTotalDownloadSpeed = Utils::changeFlowValueUnit(dTotalDownloadSpeed, 2).append("/s");
    QString sTotalUploadSpeed = Utils::changeFlowValueUnit(dTotalUploadSpeed, 2).append("/s");
    m_totalSpeedLabel->setText("↓" + tr("Total download") + ": " + sTotalDownloadSpeed + "  " + "↑" + tr("Total upload") + ": " + sTotalUploadSpeed);
}

void SpeedWidget::recAllProcInfos(const DefenderProcInfoList &procInfos)
{
    m_mergedProcInfos.clear();
    // 合并无包名的进程信息为 "其它应用"
    DefenderProcInfo otherAppInfo;
    otherAppInfo.isbSysApp = true;
    otherAppInfo.sAppName = tr("Other apps");
    // 网速信息以上一段时间内统计的为准
    for (DefenderProcInfo procInfo : procInfos) {
        if (procInfo.sPkgName.isEmpty()) {
            otherAppInfo.dDownloadSpeed += procInfo.dDownloadSpeed;
            otherAppInfo.dUploadSpeed += procInfo.dUploadSpeed;
            otherAppInfo.dDownloads += procInfo.dDownloads;
            otherAppInfo.dUploads += procInfo.dUploads;
        } else {
            // 合并相同包名项
            bool isbSamePkgInfo = false;
            for (int i = 0; i < m_mergedProcInfos.size(); ++i) {
                DefenderProcInfo sameProcInfo = m_mergedProcInfos.at(i);
                if (procInfo.sPkgName == sameProcInfo.sPkgName) {
                    sameProcInfo.dDownloadSpeed += procInfo.dDownloadSpeed;
                    sameProcInfo.dUploadSpeed += procInfo.dUploadSpeed;
                    sameProcInfo.dDownloads += procInfo.dDownloads;
                    sameProcInfo.dUploads += procInfo.dUploads;

                    isbSamePkgInfo = true;
                    m_mergedProcInfos.replace(i, sameProcInfo);
                    break;
                }
            }
            if (!isbSamePkgInfo) {
                m_mergedProcInfos.append(procInfo);
            }
        }
    }

    m_mergedProcInfos.append(otherAppInfo);
    updateTable();
}

// 初始化判断是否显示联网管控开关未开启提示
void SpeedWidget::intiSwitchStatus()
{
    // 联网管控总开关关闭时，提示用户请开启总开关
    if (!m_model->GetDataUsageSwitchStatus()) {
        DFloatingMessage *floMsgSwitchOff = new DFloatingMessage(DFloatingMessage::TransientType);
        floMsgSwitchOff->setDuration(2000);
        floMsgSwitchOff->setIcon(QIcon::fromTheme(DIALOG_WARNING_TIP_RED));
        floMsgSwitchOff->setMessage(tr("Its functions are disabled, please turn it on in Protection"));
        DMessageManager::instance()->sendMessage(m_speedtableview, floMsgSwitchOff);
        DMessageManager::instance()->setContentMargens(m_speedtableview, QMargins(0, 0, 0, 20));
    }
}

// 表格内下拉框选项改变
void SpeedWidget::onAppOptChanged(int index)
{
    // 获取改变的包名
    QComboBox *box = qobject_cast<QComboBox *>(sender());
    QString sPkgName = m_comboToAppName.value(box);

    // 将包名以及对应的类型发送给服务
    m_model->SetNetAppStatusChange(sPkgName, index);

    // 添加安全日志
    if (NET_STATUS_ITEMASK == index) {
        m_model->AddSecurityLog(tr("Changed Internet connection of %1 to \"Ask\"").arg(m_appNamefromPkgName[sPkgName]));
    } else if (NET_STATUS_ITEMALLOW == index) {
        m_model->AddSecurityLog(tr("Changed Internet connection of %1 to \"Allow\"").arg(m_appNamefromPkgName[sPkgName]));
    } else {
        m_model->AddSecurityLog(tr("Changed Internet connection of %1 to \"Disable\"").arg(m_appNamefromPkgName[sPkgName]));
    }
}

//设置网速view属性
void SpeedWidget::SetSpeedAttributes()
{
    m_speedtableview->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_speedtableview->setSelectionMode(QAbstractItemView::NoSelection);
    m_speedtableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_speedtableview->setFrameShape(QTableView::NoFrame);
    //自动调整最后一列的宽度使它和表格的右边界对齐
    m_speedtableview->horizontalHeader()->setStretchLastSection(true);
    m_speedtableview->verticalHeader()->setHidden(true);
    m_speedtableview->setShowGrid(false);
    m_speedtableview->setSortingEnabled(true);
    m_speedtableview->verticalHeader()->setDefaultSectionSize(48);

    // 连接表格与模型
    m_speedtableview->setModel(m_speeditemmodel);
    //设置代理
    m_speedtableview->setItemDelegate(m_speeditem);
    // 初始化表格设置
    // 设置表头
    QStandardItem *standaritem0 = new QStandardItem(tr("Name"));
    standaritem0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem1 = new QStandardItem(tr("Download"));
    standaritem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem2 = new QStandardItem(tr("Upload"));
    standaritem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem3 = new QStandardItem(tr("Settings"));
    standaritem3->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_speeditemmodel->setHorizontalHeaderItem(AppNameDispCol, standaritem0);
    m_speeditemmodel->setHorizontalHeaderItem(DownloadSpeedDispCol, standaritem1);
    m_speeditemmodel->setHorizontalHeaderItem(UploadSpeedDispCol, standaritem2);
    m_speeditemmodel->setHorizontalHeaderItem(NetContrDispCol, standaritem3);
    m_speeditemmodel->setHorizontalHeaderItem(ProcIsRunningCol, new QStandardItem(QString("ProcIsRunning")));
    m_speeditemmodel->setHorizontalHeaderItem(PidCol, new QStandardItem(QString("Pid")));
    m_speeditemmodel->setHorizontalHeaderItem(IsbSysAppCol, new QStandardItem(QString("IsbSysApp")));
    m_speeditemmodel->setHorizontalHeaderItem(ProcNameCol, new QStandardItem(QString("ProcName")));
    m_speeditemmodel->setHorizontalHeaderItem(DesktopCol, new QStandardItem(QString("Desktop")));
    m_speeditemmodel->setHorizontalHeaderItem(AppNameCol, new QStandardItem(QString("AppName")));
    m_speeditemmodel->setHorizontalHeaderItem(IconCol, new QStandardItem(QString("Icon")));
    m_speeditemmodel->setHorizontalHeaderItem(DownloadSpeedCol, new QStandardItem(QString("DownloadSpeed")));
    m_speeditemmodel->setHorizontalHeaderItem(UploadSpeedCol, new QStandardItem(QString("UploadSpeed")));
    m_speeditemmodel->setHorizontalHeaderItem(NetStatus, new QStandardItem(QString("NetStatus")));

    m_speedtableview->setColumnHidden(ProcIsRunningCol, true);
    m_speedtableview->setColumnHidden(PidCol, true);
    m_speedtableview->setColumnHidden(IsbSysAppCol, true);
    m_speedtableview->setColumnHidden(ProcNameCol, true);
    m_speedtableview->setColumnHidden(DesktopCol, true);
    m_speedtableview->setColumnHidden(AppNameCol, true);
    m_speedtableview->setColumnHidden(IconCol, true);
    m_speedtableview->setColumnHidden(DownloadSpeedCol, true);
    m_speedtableview->setColumnHidden(UploadSpeedCol, true);
    m_speedtableview->setColumnHidden(NetStatus, true);

    m_speedtableview->setColumnWidth(AppNameDispCol, COL_APP_NAME);
    m_speedtableview->setColumnWidth(DownloadSpeedDispCol, COL_DOWNLOAD_SPEED);
    m_speedtableview->setColumnWidth(UploadSpeedDispCol, COL_UPLOAD_SPEED);
    m_speedtableview->setColumnWidth(NetContrDispCol, COL_NET_CONTROL);
    m_speedtableview->horizontalHeader()->setHighlightSections(false);

    // 默认下载速度排序
    m_speedtableview->sortByColumn(DownloadSpeedDispCol, Qt::SortOrder::DescendingOrder);
}

// 网速代理函数
SpeedItemDelegate::SpeedItemDelegate(SpeedWidget *widget, QObject *parent)
    : QItemDelegate(parent)
{
    m_widget = widget;
}

// 网速重绘操作
void SpeedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 第一列
    if (index.column() == AppNameDispCol) {
        // 绘制图标
        QRect rect = option.rect;
        int pix_x = rect.x() + 4;
        int pix_y = rect.y() + 4;
        int height = rect.height() - 4 * 2;

        // 如果没有图标，则使用默认图标
        QString sThemeIcon = index.model()->index(index.row(), IconCol).data(Qt::DisplayRole).toString();
        if (sThemeIcon.isEmpty()) {
            sThemeIcon = "dcc_application_executable";
        }
        painter->drawPixmap(pix_x + COL_LEFT_ADD_SPACE, pix_y, height, height,
                            QIcon::fromTheme(sThemeIcon).pixmap(height, height));

        // 绘画应用名
        int textWidth = rect.width() - height - 4 * 8;
        int x = pix_x + height + 4 * 2;
        int y = pix_y + rect.height() / 4 + height / 8;
        int y1 = pix_y + rect.height() / 2 + height / 4 + 4;
        QString sName = index.model()->data(index.model()->index(index.row(), AppNameCol), Qt::DisplayRole).toString();
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
        bool isbSysApp = index.model()->data(index.model()->index(index.row(), IsbSysAppCol), Qt::DisplayRole).toBool();
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
    } else if (index.column() == DownloadSpeedDispCol) {
        // 绘制下载速度
        QString download = index.model()->data(index.model()->index(index.row(), DownloadSpeedCol), Qt::DisplayRole).toString();

        // 绘画图片和文字的坐标
        QRect rect = option.rect;
        int x = rect.x() + COL_LEFT_ADD_SPACE;
        int y = rect.y() + rect.height() / 2 + 5;
        int downloadWidth = rect.width() - COL_LEFT_ADD_SPACE;
        QString sElidedDownload = painter->fontMetrics().elidedText(download, Qt::ElideRight, downloadWidth, Qt::TextShowMnemonic);
        painter->drawText(x, y, sElidedDownload);
    } else if (index.column() == UploadSpeedDispCol) {
        // 绘制上传速度
        QString upload = index.model()->data(index.model()->index(index.row(), UploadSpeedCol), Qt::DisplayRole).toString();

        // 绘画文字的坐标
        QRect rect = option.rect;
        int x = rect.x() + COL_LEFT_ADD_SPACE;
        int y = rect.y() + rect.height() / 2 + 5;
        int uploadWidth = rect.width() - COL_LEFT_ADD_SPACE;
        QString sElidedUpload = painter->fontMetrics().elidedText(upload, Qt::ElideRight, uploadWidth, Qt::TextShowMnemonic);
        painter->drawText(x, y, sElidedUpload);
    }
}
