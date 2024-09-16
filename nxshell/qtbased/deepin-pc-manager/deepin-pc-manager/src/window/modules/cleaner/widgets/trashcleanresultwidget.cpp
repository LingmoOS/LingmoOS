// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanresultwidget.h"

#include "../trashcleandefinition.h"
#include "../trashcleanitem.h"
#include "cleanerresultitemwidget.h"
#include "dtkwidget_global.h"
#include "trashcleanresultheaderitem.h"
#include "window/modules/cleaner/cleanerdbusadaptorinterface.h"
#include "window/modules/common/common.h"
#include "window/modules/common/gsettingkey.h"

#include <DFontSizeManager>
#include <DFrame>
#include <DLabel>
#include <DProgressBar>
#include <DPushButton>
#include <DSpinner>
#include <DSuggestButton>
#include <DTrashManager>
#include <DTreeWidget>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFuture>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

#define SYS_CHECK_INDEX 0
#define APP_CHECK_INDEX 1
#define HISTORY_CHECK_INDEX 2
#define BROWSER_CACHE_CHECK_INDEX 3

#define APP_UNINSTALL_REMAIN_PATH_INDEX 0
#define APP_CONFIG_PATH_INDEX 0
#define APP_CACHE_PATH_INDEX 1

#define CONTENT_FRAME_LEFT_MARGINS 35

#define MAX_HEAD_TITLE_WIDTH 520

DWIDGET_USE_NAMESPACE

TrashCleanResultWidget::TrashCleanResultWidget(CleanerDBusAdaptorInterface *interface,
                                               QWidget *parent)
    : QWidget(parent)
    , m_leftButton(nullptr)
    , m_rightButton(nullptr)
    , m_treeWidget(nullptr)
    , m_delegate(nullptr)
    , m_isbScan(false)
    , m_dataInterface(interface)
    , m_isFirstShow(false)
    , m_headerItem(nullptr)
{
    this->setAccessibleName("rightWidget_trashCleanWidget");

    // 固定窗口
    setFixedSize(QSize(770, 590));
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 布局
    mainLayout->setRowStretch(0, 146);
    mainLayout->setRowMinimumHeight(0, 146);
    mainLayout->setRowStretch(1, 368);
    mainLayout->setRowMinimumHeight(1, 368);
    mainLayout->setRowStretch(2, 36);
    mainLayout->setColumnStretch(0, 420);
    mainLayout->setColumnMinimumWidth(0, 420);
    mainLayout->setColumnStretch(1, 150);
    mainLayout->setColumnMinimumWidth(1, 150);
    mainLayout->setColumnStretch(2, 150);
    mainLayout->setColumnMinimumWidth(2, 150);

    // 上部分标题视图绘制
    m_headerItem = new TrashCleanResultHeaderItem(this);
    m_headerItem->setAccessibleName("resultWidgetHeader");
    mainLayout->addWidget(m_headerItem, 0, 0, 1, -1, Qt::AlignCenter);

    initTable();
    mainLayout->addWidget(m_treeWidget, 1, 0, 1, -1, Qt::AlignCenter);

    initButtons();
    mainLayout->addWidget(m_leftButton, 2, 1, Qt::AlignCenter);
    mainLayout->addWidget(m_cancelScanButton, 2, 2, Qt::AlignCenter);
    mainLayout->addWidget(m_rightButton, 2, 2, Qt::AlignCenter);
    setLayout(mainLayout);

    // 由onStageChanged统一处理窗口状态变化
    connect(this,
            &TrashCleanResultWidget::stageChanged,
            this,
            &TrashCleanResultWidget::onStageChanged);

    foreach (auto item, m_rootItems) {
        connect(this,
                &TrashCleanResultWidget::prepare,
                item,
                &TrashCleanItem::prepare); // 移除上一次扫描数据
        connect(this,
                &TrashCleanResultWidget::startWork,
                item,
                &TrashCleanItem::startScan); // 开始扫描
        connect(this,
                &TrashCleanResultWidget::showScanResult,
                item,
                &TrashCleanItem::showScanResult); //  扫描完成后展示结果
        connect(this,
                &TrashCleanResultWidget::stopShowResult,
                item,
                &TrashCleanItem::stopShowResult); // 停止展示，响应“中止扫描”
        connect(this,
                &TrashCleanResultWidget::startClean,
                item,
                &TrashCleanItem::clean); // 开始清理

        connect(item,
                &TrashCleanItem::showScanResultFinished,
                this,
                &TrashCleanResultWidget::showScanResultFinished); // 开始展示扫描结果
        connect(item,
                &TrashCleanItem::scanFinished,
                this,
                &TrashCleanResultWidget::scanDone); // 根项扫描结束
        connect(item,
                &TrashCleanItem::noticeFileName,
                this,
                &TrashCleanResultWidget::setScanTitleName); // 根项通知主界面当前展示的文件名称
        connect(item, &TrashCleanItem::recounted, this, &TrashCleanResultWidget::recount);
        connect(item,
                &TrashCleanItem::cleanFinished,
                this,
                &TrashCleanResultWidget::onCleanFinished);

        // 后台服务文件删除功能
        connect(item,
                &TrashCleanItem::deleteUserFiles,
                this,
                &TrashCleanResultWidget::DeleteSpecifiedFiles);
        // 后台卸载应用数据库删除
        connect(item,
                &TrashCleanItem::noticeAppName,
                this,
                &TrashCleanResultWidget::DeleteSpecifiedAppRecord);
    }

    // 由于提供了公有查询方法，变量应当初始化
    m_totalSize = 0;
    m_totalScannedFiles = 0;
    m_cleanedSize = 0;
    m_cleanedAmount = 0;
    onStageChanged(ScanStages::PREPARING);
}

TrashCleanResultWidget::~TrashCleanResultWidget()
{
    Q_EMIT stopScan();
    Q_EMIT stopShowResult();

    foreach (auto root, m_rootItems) {
        if (root) {
            root->removeChildren();
        }
        root->itemWidget()->deleteLater();
        root->deleteLater();
    }
    // 注意,指针在list中以值保存,foreach操作时,会以值的形式复制给局部变量
    // 不要在foreach遍历中进行指针删除
    qDeleteAll(m_rootItems.begin(), m_rootItems.end());
    m_rootItems.clear();
}

void TrashCleanResultWidget::processScan()
{
    // 初始化后，将主界面设为PREPARING状态
    if (m_stage != SCAN_STARTED) {
        onStageChanged(PREPARING);
        onStageChanged(SCAN_STARTED);
    }
}

void TrashCleanResultWidget::haltScan()
{
    Q_EMIT stopScan();
    Q_EMIT stopShowResult();
    Q_EMIT stageChanged(SCAN_FINISHED);
}

void TrashCleanResultWidget::setServerInterface(CleanerDBusAdaptorInterface *interface)
{
    m_dataInterface = interface;
}

void TrashCleanResultWidget::initTable()
{
    m_treeWidget = new DTreeWidget(this);
    m_treeWidget->setAccessibleName("centerResultFrame_treeWidget");
    m_treeWidget->setFixedSize(750, 368);
    m_treeWidget->setFrameShape(QFrame::NoFrame);
    m_treeWidget->setIndentation(16);
    m_treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    m_treeWidget->setBackgroundRole(QPalette::Window);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setFocusPolicy(Qt::NoFocus);
    m_delegate = new TCTableDelegateTree(this);
    m_treeWidget->setItemDelegate(m_delegate);

    QStringList strList;
    strList << "treeWidget_headerLable";
    m_treeWidget->setHeaderLabels(strList);
    m_treeWidget->setContentsMargins(0, 0, 0, 0);

    // 在树中插入根检查项
    addRootCheckItems();
}

void TrashCleanResultWidget::initButtons()
{
    m_leftButton = new DPushButton(this);
    m_leftButton->setFont(Utils::getFixFontSize(14));
    m_leftButton->setAccessibleName("leftButton");
    m_leftButton->setObjectName("leftButton");
    m_leftButton->setFixedSize(QSize(150, 36));
    m_leftButton->setText(tr("Scan Now", "buton"));

    m_cancelScanButton = new DPushButton(this);
    m_cancelScanButton->setFont(Utils::getFixFontSize(14));
    m_cancelScanButton->setAccessibleName("cancelButton");
    m_cancelScanButton->setObjectName("cancelButton");
    m_cancelScanButton->setFixedSize(QSize(150, 36));
    m_cancelScanButton->setText(tr("Cancel", "button"));
    m_cancelScanButton->setVisible(false);

    // 右按键背景色
    m_rightButton = new DSuggestButton(this);
    m_rightButton->setFont(Utils::getFixFontSize(14));
    m_rightButton->setAccessibleName("rightButton");
    m_rightButton->setObjectName("rightButton");
    m_rightButton->setFixedSize(QSize(150, 36));
    m_rightButton->setText(tr("Scan Now", "button"));

    connect(m_cancelScanButton, &QPushButton::clicked, this, [this] {
        Q_EMIT stopShowResult();
        Q_EMIT stageChanged(SCAN_FINISHED);
    });
    connect(m_leftButton,
            &QPushButton::clicked,
            this,
            &TrashCleanResultWidget::changeStageByLeftButton);
    connect(m_rightButton,
            &QPushButton::clicked,
            this,
            &TrashCleanResultWidget::changeStageByRightButton);
}

// 在界面上添加固定的根检查项
void TrashCleanResultWidget::addRootCheckItems()
{
    // 添加根项检查点
    // 根项只有标题，没有说明
    QStringList itemlist = { tr("System junk"),
                             tr("Application junk"),
                             tr("Traces"),
                             tr("Cookies") };
    foreach (auto item, itemlist) {
        TrashCleanItem *trash = new TrashCleanItem(nullptr);
        trash->setTitle(item);
        m_rootItems.push_back(trash);
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(m_treeWidget);
        treeItem->setData(0, Qt::DisplayRole, "treeWidget_rootItem");
        treeItem->setHidden(true);
        m_treeWidget->setItemWidget(treeItem, 0, trash->itemWidget());
    }
}

// 为所有根检查项添加子项检查
void TrashCleanResultWidget::addChildrenCheckItems()
{
    if (!m_dataInterface) {
        return;
    }
    getScanPathsFromDataInterface();

    // 系统扫描项为固定目录
    // 垃圾箱
    addChildItem(SYS_CHECK_INDEX,
                 tr("Trash"),
                 tr("Make sure all files in the trash can be deleted"),
                 m_trashInfoList);
    // 系统缓存
    addChildItem(SYS_CHECK_INDEX,
                 tr("System caches"),
                 tr("Caches created by the system"),
                 m_cacheInfoLIst);
    // 系统日志
    addChildItem(SYS_CHECK_INDEX,
                 tr("System logs"),
                 tr("Log files created by the system"),
                 m_logInfoList);

    // 痕迹扫描项为固定目录
    addChildItem(HISTORY_CHECK_INDEX,
                 tr("System and Application traces"),
                 tr(""),
                 m_historyInfoList);

    // 根据服务返回清单，建立应用扫描项
    addAppCheckItems();

    // 浏览器cookies扫描项
    addBrowserCookies();
}

// 服务返回JSON文件在这里处理，收集文件信息
// 待重构
// 服务返回改为结构体，更有效率
void TrashCleanResultWidget::addAppCheckItems()
{
    if (!m_dataInterface) {
        return;
    }

    QJsonParseError err;
    // 读app.json获取名称和路径信息
    QJsonObject object = QJsonDocument::fromJson(m_appJsonStr.toUtf8(), &err).object();
    // 文件错误检查
    if (QJsonParseError::NoError != err.error) {
        qDebug() << "failed to parse JSON content when addAppCheckItems";
        return;
    }

    QJsonArray appArr = object[CLEANER_APP_JSON_NAME].toArray();
    // 从后台服务传递的JSON中取出对应路径并设置给子检查项
    foreach (auto app, appArr) {
        QStringList configPaths;
        QStringList cachePaths;
        QList<QStringList> paths;
        foreach (auto item, app[CLEANER_APP_CONFIG_PATH].toArray()) {
            configPaths.append(item.toString());
        }
        foreach (auto item, app[CLEANER_APP_CACHE_PATH].toArray()) {
            cachePaths.append(item.toString());
        }
        paths.append(configPaths);
        paths.append(cachePaths);

        // 应用的缷载标志
        bool isInstalled = app[CLEANER_APP_INSTALL_FLAG].toBool();
        QString pkgName = app[CLEANER_APP_COM_NAME].toString();
        addChildItem(APP_CHECK_INDEX,
                     app[CLEANER_APP_NAME].toString(),
                     pkgName,
                     QString(""),
                     paths,
                     isInstalled);
    }
}

void TrashCleanResultWidget::addBrowserCookies()
{
    if (!m_dataInterface) {
        return;
    }

    QJsonParseError err;
    // 读app.json获取名称和路径信息
    QJsonObject object = QJsonDocument::fromJson(m_cookiesJsonStr.toUtf8(), &err).object();
    // 文件错误检查
    if (QJsonParseError::NoError != err.error) {
        qDebug() << "failed to parse JSON content when addBrowserCookies";
        return;
    }

    QJsonArray appArr = object[CLEANER_BROWSER_JSON_NAME].toArray();
    foreach (auto app, appArr) {
        QStringList paths;
        foreach (auto item, app[CLEANER_BROWSER_COOKIES_PATH].toArray()) {
            paths.append(item.toString());
        }
        addChildItem(BROWSER_CACHE_CHECK_INDEX, app[CLEANER_APP_NAME].toString(), tr(""), paths);
    }
}

// 将子项检查内容添加到根项
// 根据需求如果子项大小为0则不显示子项
// 这里根据文件数实现，如果文件数为0,即大小为0
void TrashCleanResultWidget::addChildItem(int rootIndex,
                                          const QString &title,
                                          const QString &pkgName,
                                          const QString &tip,
                                          const QList<QStringList> &paths,
                                          bool isInstalled)
{
    if (rootIndex >= m_rootItems.size() || rootIndex < 0)
        return;

    // 构建应用上级检查项
    // 上级结点是逻辑结点，不要设置扫描路径
    TrashCleanItem *item = new TrashCleanItem(m_rootItems[rootIndex]);

    TrashCleanItem *childItem = nullptr;
    // 构建次级检查项
    // 判断是否应用已经卸载
    if (isInstalled) {
        if (!paths[APP_CACHE_PATH_INDEX].isEmpty()) {
            childItem = new TrashCleanItem(item);
            childItem->setTitle(tr("Cache and log files"));
            childItem->setFilePaths(paths[APP_CACHE_PATH_INDEX]);
        }
    } else {
        // 构建卸载应用残留
        QStringList filePath(paths[APP_CACHE_PATH_INDEX]);
        filePath.append(paths[APP_CONFIG_PATH_INDEX]);
        if (!filePath.isEmpty()) {
            childItem = new TrashCleanItem(item);
            childItem->setPkgName(pkgName);
            childItem->setTitle(tr("Residual files"));
            childItem->setFilePaths(filePath);
        }
    }

    // TrashCleanItem类中，UI在构造时产生，因此只能从上至下的形式构造树
    // 如果是卸载项目，在子项目为空的情况下，不需要构建上级扫描项，此处将父项从根项中删除
    if (!childItem && !isInstalled) {
        m_rootItems[rootIndex]->removeChild(item);
        if (item) {
            delete item;
            item = nullptr;
        }
        return;
    }

    // 父项达成存在条件时设置父项属性并建立界面
    // 1.应用没有被缷载
    // 2.应用被缷载，且存在残留内容
    item->setObjectName(title);
    item->setTitle(title);
    item->setTip(tip.toUtf8());
    QTreeWidgetItem *itemWidget = new QTreeWidgetItem();
    itemWidget->setData(0, Qt::DisplayRole, "treeWidget_childItem");
    m_treeWidget->topLevelItem(rootIndex)->addChild(itemWidget);
    m_treeWidget->setItemWidget(itemWidget, 0, item->itemWidget());

    // 次级项存在内容时建立界面
    if (childItem) {
        QTreeWidgetItem *childWidget = new QTreeWidgetItem;
        childWidget->setData(0, Qt::DisplayRole, "treeWidget_childItem");
        itemWidget->addChild(childWidget);
        m_treeWidget->setItemWidget(childWidget, 0, childItem->itemWidget());
    } else {
        // 次级不存在时，插入一个空的对象以标识父项的身份
        item->addChild(nullptr);
    }
}

void TrashCleanResultWidget::addChildItem(int rootIndex,
                                          const QString &title,
                                          const QString &tip,
                                          const QStringList &paths)
{
    if (rootIndex >= m_rootItems.size() || rootIndex < 0)
        return;
    TrashCleanItem *item = new TrashCleanItem(m_rootItems[rootIndex]);
    item->setObjectName(title);
    item->setTitle(title);
    item->setTip(tip.toUtf8());
    item->setFilePaths(paths);
    QTreeWidgetItem *itemWidget = new QTreeWidgetItem();
    itemWidget->setData(0, Qt::DisplayRole, "treeWidget_childItem");
    m_treeWidget->topLevelItem(rootIndex)->addChild(itemWidget);
    m_treeWidget->setItemWidget(itemWidget, 0, item->itemWidget());
}

// 待重构
// 控制内容重写为函数
void TrashCleanResultWidget::onStageChanged(ScanStages stage)
{
    m_stage = stage;
    switch (m_stage) {
    case SCAN_STARTED:
        setWidgetScanStart();
        break;
    case SCAN_FINISHED:
        setWidgetScanFinish();
        break;
    case PREPARING:
        m_isbScan = true;
        setWidgetPrepare();
        break;
    case CLEAN_STARTED:
        setWidgetCleanStart();
        break;
    case CLEAN_FINISHED:
        setWidgetCleanFinish();
        break;
    }
}

// 左按键引发的状态变化
void TrashCleanResultWidget::changeStageByLeftButton()
{
    // m_state为当前状态
    // 开始进入下一状态
    switch (m_stage) {
    // 由配置界面，开始扫描
    case PREPARING:
        //        m_isbScan = true;
        //        Q_EMIT stageChanged(SCAN_STARTED);
        //        break;
        // 扫描过程中要求中止
    case SCAN_STARTED:
        //        Q_EMIT stopShowResult();
        //        Q_EMIT stageChanged(SCAN_FINISHED);
        //        break;
        // 扫描完成要求返回到配置页
    case SCAN_FINISHED:
        //        Q_EMIT stageChanged(PREPARING);
        //        break;
        // 清理过程中要求中止
    case CLEAN_STARTED:
        //        Q_EMIT stageChanged(CLEAN_FINISHED);
        break;
    // 清理结束后返回配置界面
    case CLEAN_FINISHED:
        m_isbScan = true;
        Q_EMIT stageChanged(SCAN_STARTED);
        break;
    }
}

// 右按键引发的状态变化
void TrashCleanResultWidget::changeStageByRightButton()
{
    // m_state为当前状态
    // 开始进入下一状态
    switch (m_stage) {
    // 扫描结束时，按下开始进行清理
    case SCAN_FINISHED:
        if (m_rootItems[0]->isSelected() || m_rootItems[1]->isSelected()
            || m_rootItems[2]->isSelected() || m_rootItems[3]->isSelected()) {
            Q_EMIT stageChanged(CLEAN_STARTED);
        }
        break;
    // 清理结束时，按下返回到配置界面
    case CLEAN_FINISHED:
        // Q_EMIT stageChanged(PREPARING);
        // 回退到前一页面
        Q_EMIT stageChanged(PREPARING);
        Q_EMIT notifyBackToGreetingWidget();
        break;
    default:
        break;
    }
}

// 需要顺序执行
void TrashCleanResultWidget::startScan()
{
    // 启动扫描
    // 获取所有项目的文件列表和大小数据

    // 待重构
    // 将扫描功能移动到后端服务

    Q_EMIT startWork();
}

void TrashCleanResultWidget::scanDone()
{
    // 扫描并获得全部文件数量
    m_doneItems--;
    if (m_doneItems == 0) {
        // 开始延时显示扫描结果
        m_totalSize = 0;
        Q_EMIT showScanResult();
    }
}

// 接收工作线程发送的信号，以同步标题上的扫描文件名
// 展示当前扫描进度
void TrashCleanResultWidget::setScanTitleName(const QString &fileName)
{
    if (!m_leftButton->isEnabled()) {
        m_leftButton->setDisabled(false);
    }
    m_headerItem->updateScanPath(fileName);
}

// 根检查项通知过程结束
void TrashCleanResultWidget::showScanResultFinished()
{
    if (!m_leftButton->isEnabled()) {
        m_leftButton->setDisabled(false);
    }

    m_cancelScanButton->setDisabled(false);
    m_showItems--;
    if (m_showItems == 0) {
        stageChanged(SCAN_FINISHED);
    }
}

// 因界面变动要求重新统计总计文件大小
void TrashCleanResultWidget::recount()
{
    if (PREPARING == m_stage) {
        // 在PREPARING状态下不响应检查项的recount信号
        return;
    }

    quint64 fileSize = 0;
    int fileAmount = 0;

    foreach (auto item, m_rootItems) {
        if (!item->isSelected() || !item->isShown()) {
            continue;
        }
        fileSize += item->totalFileSize();
        fileAmount += item->totalFileAmount();
    }

    // recount结果会根据项目选中变化，此处应使用qMax统计
    m_totalSize = qMax(m_totalSize, fileSize);
    m_totalScannedFiles = qMax(m_totalScannedFiles, fileAmount);
    m_lastScannedSize = TrashCleanItem::fileSizeToString(m_totalSize);

    if (SCAN_FINISHED == m_stage || SCAN_STARTED == m_stage) {
        m_scanResultStr = tr("%1 junk files, %2 selected")
                              .arg(m_lastScannedSize)
                              .arg(TrashCleanItem::fileSizeToString(fileSize));
        QString tipInfo = tr("Scanned: %1 files").arg(m_totalScannedFiles);
        m_headerItem->setTitle(m_scanResultStr);
        m_headerItem->setTip(tipInfo);
    }

    // 如果在清理前之选择了部分项目
    // 清理完成后选择另外部分项目,则重新回到清理开始状态
    // 根据重新计算的总大小判断是否回退状态到清理完成
    if (CLEAN_FINISHED == m_stage && fileSize) {
        onStageChanged(SCAN_FINISHED);
        return;
    }

    if (SCAN_FINISHED == m_stage && !fileSize && !m_isFirstShow) {
        onStageChanged(CLEAN_FINISHED);
        return;
    }
}

// 通知扫描线程，中断扫描过程
void TrashCleanResultWidget::stopScan()
{
    if (m_stage == SCAN_STARTED) {
        onStageChanged(SCAN_FINISHED);
    }
}

void TrashCleanResultWidget::setWidgetPrepare()
{
    m_headerItem->setTitle(tr("Clean out junk files and free up disk space"));
    Q_EMIT prepare();
}

void TrashCleanResultWidget::setWidgetScanStart()
{
    // 开始扫描
    // 显示header上logo
    // 显示进度条
    // 隐藏所有的配置项与配置区logo
    // 显示树结构
    // 显示“正在扫描”旋转
    // 左按键重命名“取消扫描”
    m_doneItems = 0;
    m_totalSize = 0;
    m_totalScannedFiles = 0;
    m_showItems = 0;
    // 有可能在scanfinish时将按键设为disable
    // 此处还原

    m_headerItem->setLogo(kHeaderScanningIconPath);

    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_cancelScanButton->setVisible(true);
    m_cancelScanButton->setDisabled(true);
    Q_EMIT notifySetBackForwardBtnStatus(false);

    int i = 0;
    int hiddens = 0;

    foreach (auto item, m_scanConfigList) {
        if (item != Qt::Unchecked) {
            // 开始显示选中的根项和子项窗口
            m_treeWidget->topLevelItem(i)->setHidden(false);
            // 默认将项目设为选中状态
            m_rootItems[i]->setConfigToScan();
            Q_EMIT m_rootItems[i]->scanStarted(true);

            m_doneItems++;
            m_showItems++;
        } else {
            // 隐藏未选中的检查项
            hiddens++;
            m_treeWidget->topLevelItem(i)->setHidden(true);
            m_rootItems[i]->setSelected(false);
        }
        i++;
    }
    m_treeWidget->collapseAll();

    // 选中了配置项，可以开始扫描
    if (hiddens < m_rootItems.size()) {
        m_leftButton->setDisabled(true);
        m_headerItem->setTip(tr("Initializing"));

        QString scanTimeInfo = tr("Time elapsed %1:%2:%3").arg("00").arg("00").arg("00");
        m_headerItem->setTimeInfo(scanTimeInfo);
        QTimer scanTimer;
        QDateTime currentTime = QDateTime::currentDateTime();
        connect(&scanTimer, &QTimer::timeout, this, [=] {
            QDateTime duration =
                QDateTime::fromSecsSinceEpoch(QDateTime::currentDateTime().toSecsSinceEpoch()
                                              - currentTime.toSecsSinceEpoch())
                    .toUTC();
            // 格式来自于需求，可能是翻译要求
            m_headerItem->setTimeInfo(tr("Time elapsed %1:%2:%3")
                                          .arg(duration.toString("hh"))
                                          .arg(duration.toString("mm"))
                                          .arg(duration.toString("ss")));
        });
        scanTimer.start(1000);

        m_isCleaned = false;
        foreach (auto rootItem, m_rootItems) {
            if (rootItem) {
                rootItem->removeChildren();
            }
        }

        // 从树上移除所有子项目,等待重新插入
        QList<QTreeWidgetItem *> childrenItem =
            m_treeWidget->topLevelItem(SYS_CHECK_INDEX)->takeChildren();
        childrenItem.append(m_treeWidget->topLevelItem(APP_CHECK_INDEX)->takeChildren());
        childrenItem.append(m_treeWidget->topLevelItem(HISTORY_CHECK_INDEX)->takeChildren());
        childrenItem.append(m_treeWidget->topLevelItem(BROWSER_CACHE_CHECK_INDEX)->takeChildren());
        qDeleteAll(childrenItem.begin(), childrenItem.end());
        childrenItem.clear();

        // 插入所有子项目
        // 增加调用deb文件相关接口后，有明显的延时
        addChildrenCheckItems();

        // 仅扩展树的最顶级检查项
        // 在“应用”相关的检查项时，就只会显示到应用名称
        // 如果客户需要详细信息，再双击点开
        // 否则会因内容过多，列表太长，影响体验
        m_treeWidget->expandToDepth(0);

        m_isFirstShow = true;
        m_cleanedSize = 0;
        m_cleanedAmount = 0;

        Q_EMIT prepare();
        Q_EMIT startWork();
    } else {
        onStageChanged(PREPARING);
    }
}

void TrashCleanResultWidget::setWidgetScanFinish()
{
    // 取消扫描
    // 显示扫描完成页面
    // 左按键重命名为“返回”
    // 显示右按键，命名“立即清理”
    m_leftButton->setVisible(false);
    m_cancelScanButton->setVisible(false);
    m_rightButton->setVisible(true);
    m_rightButton->setText(tr("Clean Up", "button"));
    m_headerItem->setLogo(kHeaderScanedIconPath);
    // 要求统计所有项目并在界面上写对应信息
    recount();

    m_isFirstShow = false;

    // 由于在中止扫描的情况下有两次调用setWidgetFinished
    // 此处应注意不被上一次调用影响
    m_rightButton->setDisabled(!m_totalSize);

    // 针对无垃圾情况额外处理
    if (!m_totalSize) {
        m_cancelScanButton->setVisible(false);
        m_leftButton->setDisabled(false);
        m_rightButton->setDisabled(false);
        m_leftButton->setText(tr("Scan Again", "button"));
        m_rightButton->setText(tr("Done", "button"));
        m_leftButton->setVisible(true);
        m_rightButton->setVisible(true);
        m_headerItem->setLogo(kHeaderCleanedIconPath);
        m_stage = CLEAN_FINISHED;
    }

    if (m_isbScan) {
        // 添加安全日志
        m_isbScan = false;
        // 添加安全日志
        if (m_dataInterface) {
            m_dataInterface->AddSecurityLog(SECURITY_LOG_TYPE_CLEANER,
                                            tr("Scanned: %1 files, junk files: %2")
                                                .arg(m_totalScannedFiles)
                                                .arg(m_lastScannedSize));
        }
    }
    Q_EMIT notifySetBackForwardBtnStatus(true);
}

void TrashCleanResultWidget::setWidgetCleanStart()
{
    // 有可能在scanfinish时将按键设为disable
    // 此处还原
    m_rightButton->setDisabled(false);
    m_cancelScanButton->setVisible(false);

    m_isCleaned = true;
    m_leftButton->setText(tr("Cancel", "button"));
    m_rightButton->setVisible(false);

    m_doneItems = 0;
    m_cleanedSize = 0;
    m_cleanedAmount = 0;
    foreach (TrashCleanItem *root, m_rootItems) {
        if (root) {
            // 记录被选中进行清理的项目数
            if (root->isSelected()) {
                m_doneItems++;
            }
        }
    }
    Q_EMIT startClean();

    // 添加安全日志
    if (m_dataInterface) {
        m_dataInterface->AddSecurityLog(SECURITY_LOG_TYPE_CLEANER,
                                        tr("Removed: %1 junk files").arg(m_lastCleanedSize));
    }
}

void TrashCleanResultWidget::setWidgetCleanFinish()
{
    // 有可能在scanfinish时将按键设为disable
    // 此处还原
    m_rightButton->setDisabled(false);

    m_cancelScanButton->setVisible(false);
    m_leftButton->setText(tr("Scan Again", "button"));
    m_rightButton->setText(tr("Done", "button"));
    m_leftButton->setVisible(true);
    m_rightButton->setVisible(true);
    m_lastCleanedSize = TrashCleanItem::fileSizeToString(m_cleanedSize);
    m_headerItem->setLogo(kHeaderCleanedIconPath);

    QString titleInfo = tr("%1 removed").arg(m_lastCleanedSize);
    QString tipInfo = tr("Cleaned: %1 files").arg(m_cleanedAmount);
    m_headerItem->setCleanFinished(titleInfo, tipInfo);
    if (m_dataInterface) {
        m_dataInterface->SetValueToGSettings(CLEANER_LAST_TIME_CLEANED, m_lastCleanedSize);
    }
    Q_EMIT notifyUpdateLastCleaned(m_lastCleanedSize);
}

void TrashCleanResultWidget::getScanPathsFromDataInterface()
{
    if (nullptr == m_dataInterface) {
        m_trashInfoList.clear();
        m_cacheInfoLIst.clear();
        m_logInfoList.clear();
        m_historyInfoList.clear();
        m_appJsonStr.clear();
        return;
    }

    // GetCacheInfoList 可能存在耗时较长的问题
    // 关联的数据获取集中在此处修改
    QFuture<void> future = QtConcurrent::run([&]() {
        if (m_scanConfigList[SYS_CHECK_INDEX]) {
            m_trashInfoList = m_dataInterface->GetTrashInfoList();
            m_cacheInfoLIst = m_dataInterface->GetCacheInfoList();
            m_logInfoList = m_dataInterface->GetLogInfoList();
        }

        if (m_scanConfigList[APP_CHECK_INDEX]) {
            m_appJsonStr = m_dataInterface->GetAppTrashInfoList();
        }

        if (m_scanConfigList[HISTORY_CHECK_INDEX]) {
            m_historyInfoList = m_dataInterface->GetHistoryInfoList();
        }

        if (m_scanConfigList[BROWSER_CACHE_CHECK_INDEX]) {
            m_cookiesJsonStr = m_dataInterface->GetBrowserCookiesInfoList();
        }
    });
    while (!future.isFinished()) {
        QApplication::processEvents();
    }
}

void TrashCleanResultWidget::onCleanFinished()
{
    m_doneItems--;
    // 未被选中的根项不会有删除文件的内容,所以不需要判断是否选中
    m_cleanedSize = 0;
    m_cleanedAmount = 0;
    foreach (auto item, m_rootItems) {
        m_cleanedSize += item->cleanedSize();
        m_cleanedAmount += item->cleanedAmount();
    }
    // 所有选中项目都已经清理完成
    if (!m_doneItems) {
        onStageChanged(CLEAN_FINISHED);
    }
    m_lastCleanedSize = TrashCleanItem::fileSizeToString(m_cleanedSize);

    QString titleInfo = tr("%1 removed").arg(m_lastCleanedSize);
    QString tipInfo = tr("Cleaned: %1 files").arg(m_cleanedAmount);
    m_headerItem->setCleanFinished(titleInfo, tipInfo);
}

void TrashCleanResultWidget::DeleteSpecifiedFiles(const QStringList &paths)
{
    foreach (auto filePath, paths) {
        if (filePath.contains(CLEANER_SYSTEM_TRASH_PATH)) {
            DTrashManager::instance()->cleanTrash();
            break;
        }
    }

    if (m_dataInterface) {
        m_dataInterface->DeleteSpecifiedFiles(paths);
    }
}

void TrashCleanResultWidget::DeleteSpecifiedAppRecord(const QString &pkgName)
{
    if (m_dataInterface) {
        m_dataInterface->DeleteSpecifiedAppUninstallInfo(pkgName);
    }
}

// 代理函数
TCTableDelegateTree::TCTableDelegateTree(QObject *parent)
    : QItemDelegate(parent)
{
}

// 重绘操作
void TCTableDelegateTree::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(index);
}

void TrashCleanResultWidget::paintEvent(QPaintEvent *event)
{
    if (SCAN_FINISHED == m_stage) {
        m_headerItem->setTitle(m_scanResultStr);
    }

    QWidget::paintEvent(event);
}
