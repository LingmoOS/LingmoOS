// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanwidget.h"

#include "cleanerdbusadaptorinterface.h"
#include "dtkwidget_global.h"
#include "src/widgets/cleaneritem.h"
#include "src/widgets/titlelabel.h"
#include "src/window/modules/common/common.h"
#include "src/window/modules/common/compixmap.h"
#include "src/window/modules/common/gsettingkey.h"
#include "trashcleanitem.h"
#include "trashcleanwidget.h"
#include "widgets/cleanerresultitemwidget.h"

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

TrashCleanWidget::TrashCleanWidget(QWidget *parent, CleanerDBusAdaptorInterface *interface)
    : QWidget(parent)
    , m_headerFrame(nullptr)
    , m_configFrame(nullptr)
    , m_resultFrame(nullptr)
    , m_bottomFrame(nullptr)
    , m_bottomLine(nullptr)
    , m_leftButton(nullptr)
    , m_rightButton(nullptr)
    , m_configLogo(nullptr)
    , m_headerTitle(nullptr)
    , m_scanProgressBar(nullptr)
    , m_tipTitle(nullptr)
    , m_headerLogo(nullptr)
    , m_treeWidget(nullptr)
    , m_delegate(nullptr)
    , m_isbScan(false)
    , m_scanTime(nullptr)
    , m_dataInterface(interface)
    , m_isFirstShow(false)
{
    this->setAccessibleName("rightWidget_trashCleanWidget");

    // 主题变换 改变部分图标颜色
    connect(DGuiApplicationHelper::instance(),
            &DGuiApplicationHelper::themeTypeChanged,
            this,
            &TrashCleanWidget::setPixmapByTheme);

    // 固定窗口
    setFixedSize(QSize(732, 570));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 0, 10, 10);

    // 上部分标题视图绘制
    m_headerFrame = new DFrame(this);
    m_headerFrame->setAccessibleName("trashCleanWidget_topFrame");
    m_headerFrame->setLineWidth(0);
    m_headerFrame->setFixedSize(QSize(710, 135));
    initHeaderFrame();

    // 中部配置视图绘制
    m_configFrame = new DFrame(this);
    m_configFrame->setAccessibleName("trashCleanWidget_centerConfigFrame");
    m_configFrame->setLineWidth(0);
    m_configFrame->setFixedSize(QSize(710, 422));
    initConfigFrame();

    // 中部结果展示绘制
    m_resultFrame = new DFrame(this);
    m_resultFrame->setAccessibleName("trashCleanWidget_centerResultFrame");
    m_resultFrame->setLineWidth(0);
    m_resultFrame->setFixedSize(QSize(710, 380));
    initResultFrame();

    // 底部按键
    m_bottomFrame = new DFrame(this);
    m_bottomFrame->setAccessibleName("trashCleanWidget_bottomFrame");
    m_bottomFrame->setLineWidth(0);
    m_bottomFrame->setFixedSize(710, 52);
    initBottomFrame();

    // 加入主框架
    // m_configFrame 与 m_resultFrame 不会同时显示
    mainLayout->addWidget(m_headerFrame);
    mainLayout->addWidget(m_configFrame);
    mainLayout->addWidget(m_resultFrame);
    mainLayout->addWidget(m_bottomFrame);
    mainLayout->setAlignment(Qt::AlignmentFlag::AlignCenter);
    setLayout(mainLayout);

    // 由onStageChanged统一处理窗口状态变化
    connect(this, &TrashCleanWidget::stageChanged, this, &TrashCleanWidget::onStageChanged);

    // 在树中插入根检查项
    addRootCheckItems();

    foreach (auto item, m_rootItems) {
        connect(this,
                &TrashCleanWidget::prepare,
                item,
                &TrashCleanItem::prepare); // 移除上一次扫描数据
        connect(this, &TrashCleanWidget::startWork, item, &TrashCleanItem::startScan); // 开始扫描
        connect(this,
                &TrashCleanWidget::showScanResult,
                item,
                &TrashCleanItem::showScanResult); //  扫描完成后展示结果
        connect(this,
                &TrashCleanWidget::stopShowResult,
                item,
                &TrashCleanItem::stopShowResult); // 停止展示，响应“中止扫描”
        connect(this, &TrashCleanWidget::startClean, item, &TrashCleanItem::clean); // 开始清理

        connect(item,
                &TrashCleanItem::showScanResultFinished,
                this,
                &TrashCleanWidget::showScanResultFinished); // 开始展示扫描结果
        connect(item,
                &TrashCleanItem::scanFinished,
                this,
                &TrashCleanWidget::scanDone); // 根项扫描结束
        connect(item,
                &TrashCleanItem::noticeFileName,
                this,
                &TrashCleanWidget::setScanTitleName); // 根项通知主界面当前展示的文件名称
        connect(item, &TrashCleanItem::recounted, this, &TrashCleanWidget::recount);
        connect(item, &TrashCleanItem::cleanFinished, this, &TrashCleanWidget::onCleanFinished);

        // 后台服务文件删除功能
        connect(item,
                &TrashCleanItem::deleteUserFiles,
                this,
                &TrashCleanWidget::DeleteSpecifiedFiles);
        // 后台卸载应用数据库删除
        connect(item,
                &TrashCleanItem::noticeAppName,
                this,
                &TrashCleanWidget::DeleteSpecifiedAppRecord);
    }

    // 全没有选中时需要置灰左按键
    foreach (const CleanerItem *ci, m_configItem) {
        connect(ci, &CleanerItem::itemClicked, this, &TrashCleanWidget::onConfigItemClicked);
    }

    // 由于提供了公有查询方法，变量应当初始化
    m_totalSize = 0;
    m_totalScannedFiles = 0;
    m_cleanedSize = 0;
    m_cleanedAmount = 0;

    // 初始化后，将主界面设为PREPARING状态
    onStageChanged(PREPARING);
}

TrashCleanWidget::~TrashCleanWidget()
{
    foreach (auto root, m_rootItems) {
        if (root) {
            root->removeChildren();
        }
        root->itemWidget()->deleteLater();
    }
    // 注意,指针在list中以值保存,foreach操作时,会以值的形式复制给局部变量
    // 不要在foreach遍历中进行指针删除
    qDeleteAll(m_rootItems.begin(), m_rootItems.end());
    m_rootItems.clear();
}

void TrashCleanWidget::setServerInterface(CleanerDBusAdaptorInterface *interface)
{
    m_dataInterface = interface;
}

void TrashCleanWidget::initHeaderFrame()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
    mainLayout->setContentsMargins(CONTENT_FRAME_LEFT_MARGINS, 25, 30, 0);
    mainLayout->setSpacing(5);
    mainLayout->setGeometry(QRect(0, 0, 730, 135));

    QWidget *wid = new QWidget(this);
    wid->setAccessibleName("topFrame_backGroundWidget");
    wid->setFixedHeight(96);
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->setContentsMargins(0, 0, 0, 0);
    hlayout1->setSpacing(0);
    hlayout1->setGeometry(QRect(0, 0, 640, 96));

    // logo 图片
    m_headerLogo = new DLabel(m_headerFrame);
    m_headerLogo->setAccessibleName("backGroundWidget_headerLogoLable");
    QIcon icon = QIcon::fromTheme(TRASH_CLEAN_HEADER_LOGO);
    m_headerLogo->setPixmap(icon.pixmap(96, 96));
    m_headerLogo->setFixedWidth(125);
    m_headerLogo->setAlignment(Qt::AlignmentFlag::AlignLeft);
    m_headerLogo->setVisible(false);
    hlayout1->addWidget(m_headerLogo);

    // 右侧标题
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 10, 0, 0);
    vLayout->setSpacing(0);

    // 标题
    m_headerTitle = new TitleLabel(m_headerFrame);
    m_headerTitle->setAccessibleName("backGroundWidget_headerTitleLable");
    m_headerTitle->setText(tr("Clean out junk files and free up disk space"));
    m_headerTitle->setObjectName("headerTitle");
    QFont ft = m_headerTitle->font();
    ft.setBold(true);
    m_headerTitle->setFont(ft);

    // 次标题
    m_tipTitle = new DLabel(m_headerFrame);
    m_tipTitle->setAccessibleName("backGroundWidget_tipTitleLable");
    m_tipTitle->setText(tr("Start cleaning now to refresh your computer"));
    m_tipTitle->setObjectName("tipTitle");
    m_tipTitle->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);

    // 字体样式
    QPalette pal;
    pal.setColor(QPalette::Text, QColor("#526A7F"));
    m_tipTitle->setPalette(pal);
    QFont tipFont = m_tipTitle->font();
    tipFont.setWeight(QFont::Medium);
    tipFont.setPixelSize(12);
    m_tipTitle->setFont(tipFont);

    // 扫描时间提示
    m_scanTime = new DLabel(m_headerFrame);
    m_scanTime->setAccessibleName("backGroundWidget_scanTimeLable");
    m_scanTime->setFont(Utils::getFixFontSize(12));
    m_scanTime->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    // 仅扫描开始后可见
    m_scanTime->setVisible(false);

    // 扫描进度条
    m_scanProgressBar = new DProgressBar(m_headerFrame);
    m_scanProgressBar->setAccessibleName("backGroundWidget_scanProgressBar");
    m_scanProgressBar->setContentsMargins(0, 0, 0, 0);
    m_scanProgressBar->setMinimumSize(QSize(0, 8));
    m_scanProgressBar->setMaximumSize(QSize(1000, 8));
    m_scanProgressBar->setVisible(false);

    QGridLayout *titlesLayout = new QGridLayout;

    titlesLayout->setSpacing(10);
    titlesLayout->addWidget(m_headerTitle, 0, 0);
    titlesLayout->addWidget(m_scanTime, 0, 1);
    titlesLayout->addWidget(m_scanProgressBar, 1, 0, 1, -1);
    titlesLayout->addWidget(m_tipTitle, 2, 0, 1, -1);

    vLayout->addLayout(titlesLayout);
    vLayout->addStretch();
    hlayout1->addLayout(vLayout);
    wid->setLayout(hlayout1);

    mainLayout->addWidget(wid);
    // 添加分割线
    QFrame *line = new QFrame(this);
    line->setAccessibleName("topFrame_lineFrame");
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setFixedWidth(640);

    mainLayout->addSpacing(15);
    mainLayout->addWidget(line);
    m_headerFrame->setLayout(mainLayout);
    m_lastScannedSize = "";
    m_lastCleanedSize = "";
}

// 配置界面，包含四个配置项
void TrashCleanWidget::initConfigFrame()
{
    // 清理的配置项
    QVBoxLayout *configLayout = new QVBoxLayout;
    configLayout->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
    configLayout->setContentsMargins(26, 0, 0, 0);
    configLayout->setSpacing(10);

    QStringList itemNames = {
        tr("System junk"),      tr("Useless files created by the system"),
        tr("Application junk"), tr("Unneeded files created by applications"),
        tr("Traces"),           tr("Activity traces of the system and applications"),
        tr("Cookies"),          tr("Cookies from Internet browsers")
    };
    QStringList itemAccessibleNames = { "systemJunkFrame",
                                        "appJunkFrame",
                                        "tracesFrame",
                                        "cookiesFrame" };

    for (int i = 0; i < itemNames.length(); i += 2) {
        CleanerItem *ci = new CleanerItem(this);
        ci->setAccessibleName("centerConfigFrame_" + itemAccessibleNames.at(i / 2));
        ci->setAccessibleParentText(itemAccessibleNames.at(i / 2));
        ci->setFixedSize(QSize(330, 64));
        ci->setLevelText(itemNames.at(i));
        ci->setTipText(itemNames.at(i + 1));
        configLayout->addWidget(ci);
        m_configItem.push_back(ci);
    }

    for (int i = 0; i < m_configItem.size(); i++) {
        m_configItem[i]->setObjectName(QString("cleanerItem%1").arg(i));
    }

    m_configItem[SYS_CHECK_INDEX]->setCheckBoxStatus(true);
    m_configItem[APP_CHECK_INDEX]->setCheckBoxStatus(true);
    // 因为用户信息的敏感性和便利性
    // 历史痕迹和浏览器cookies用户决定是否清理
    m_configItem[HISTORY_CHECK_INDEX]->setCheckBoxStatus(false);
    m_configItem[BROWSER_CACHE_CHECK_INDEX]->setCheckBoxStatus(false);

    // 右侧大图标
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setAlignment(Qt::AlignmentFlag::AlignRight);
    m_configLogo = new DLabel(m_configFrame);
    m_configLogo->setAccessibleName("centerConfigFrame_configLogoLable");
    m_configLogo->setAlignment(Qt::AlignmentFlag::AlignRight);
    m_configLogo->setGeometry(405, 0, 300, 300);

    // 跟随系统主题设置
    if (DGuiApplicationHelper::instance()->themeType()
        == DGuiApplicationHelper::ColorType::LightType) {
        m_configLogo->setPixmap(
            QIcon::fromTheme(TRASH_CLEAN_CONFIG_BIG_LOGO_LIGHT).pixmap(300, 300));
    } else {
        m_configLogo->setPixmap(
            QIcon::fromTheme(TRASH_CLEAN_CONFIG_BIG_LOGO_DARK).pixmap(300, 300));
    }

    vLayout->addWidget(m_configLogo);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->insertLayout(0, configLayout);
    mainLayout->insertLayout(1, vLayout);
    m_configFrame->setLayout(mainLayout);
}

void TrashCleanWidget::initResultFrame()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(CONTENT_FRAME_LEFT_MARGINS, 0, 0, 5);

    m_treeWidget = new DTreeWidget(this);
    m_treeWidget->setAccessibleName("centerResultFrame_treeWidget");
    m_treeWidget->setFixedWidth(640);
    m_treeWidget->setFrameShape(QFrame::NoFrame);
    m_treeWidget->setIndentation(16);
    m_treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    m_treeWidget->setBackgroundRole(QPalette::Window);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setFocusPolicy(Qt::NoFocus);
    m_delegate = new MyItemDelegateTree(this);
    m_treeWidget->setItemDelegate(m_delegate);

    QStringList strList;
    strList << "treeWidget_headerLable";
    m_treeWidget->setHeaderLabels(strList);
    m_treeWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_treeWidget);

    m_resultFrame->setLayout(mainLayout);
    m_resultFrame->setBackgroundRole(DPalette::NoType);
}

void TrashCleanWidget::initBottomFrame()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(CONTENT_FRAME_LEFT_MARGINS, 0, 0, 0);

    // 添加下部分割线
    m_bottomLine = new DFrame(this);
    m_bottomLine->setAccessibleName("bottomFrame_lineFrame");
    m_bottomLine->setFrameShape(QFrame::HLine);
    m_bottomLine->setFrameShadow(QFrame::Sunken);
    m_bottomLine->setFixedWidth(640);
    m_bottomLine->setVisible(false);
    mainLayout->addWidget(m_bottomLine);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignBottom);
    layout->setSpacing(20);
    layout->setContentsMargins(0, 0, 0, 5);

    m_leftButton = new DPushButton(this);
    m_leftButton->setFont(Utils::getFixFontSize(14));
    m_leftButton->setAccessibleName("bottomFrame_leftButton");
    m_leftButton->setObjectName("leftButton");
    m_leftButton->setFixedSize(QSize(160, 40));
    m_leftButton->setText(tr("Scan Now"));

    // 右按键背景色
    m_rightButton = new DSuggestButton(this);
    m_rightButton->setFont(Utils::getFixFontSize(14));
    m_rightButton->setAccessibleName("bottomFrame_rightButton");
    m_rightButton->setObjectName("rightButton");
    m_rightButton->setFixedSize(QSize(160, 40));
    m_rightButton->setText(tr("Scan Now"));
    m_rightButton->setVisible(false);

    layout->addWidget(m_leftButton);
    layout->addWidget(m_rightButton);

    mainLayout->addLayout(layout);

    m_bottomFrame->setLayout(mainLayout);

    // 下部左按键和右按键分开处理
    // 左按键为扫描功能相关，右按键为清理相关
    connect(m_leftButton, &QPushButton::clicked, this, &TrashCleanWidget::changeStageByLeftButton);
    connect(m_rightButton,
            &QPushButton::clicked,
            this,
            &TrashCleanWidget::changeStageByRightButton);
}

// 在界面上添加固定的根检查项
void TrashCleanWidget::addRootCheckItems()
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
void TrashCleanWidget::addChildrenCheckItems()
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
void TrashCleanWidget::addAppCheckItems()
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

void TrashCleanWidget::addBrowserCookies()
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
void TrashCleanWidget::addChildItem(int rootIndex,
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

void TrashCleanWidget::addChildItem(int rootIndex,
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
void TrashCleanWidget::onStageChanged(ScanStages stage)
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
void TrashCleanWidget::changeStageByLeftButton()
{
    m_scanTime->setVisible(false);
    // m_state为当前状态
    // 开始进入下一状态
    switch (m_stage) {
    // 由配置界面，开始扫描
    case PREPARING:
        m_isbScan = true;
        Q_EMIT stageChanged(SCAN_STARTED);
        break;
    // 扫描过程中要求中止
    case SCAN_STARTED:
        Q_EMIT stopShowResult();
        Q_EMIT stageChanged(SCAN_FINISHED);
        break;
    // 扫描完成要求返回到配置页
    case SCAN_FINISHED:
        Q_EMIT stageChanged(PREPARING);
        break;
    // 清理过程中要求中止
    case CLEAN_STARTED:
        Q_EMIT stageChanged(CLEAN_FINISHED);
        break;
    // 清理结束后返回配置界面
    case CLEAN_FINISHED:
        m_isbScan = true;
        Q_EMIT stageChanged(SCAN_STARTED);
        break;
    }
}

// 右按键引发的状态变化
void TrashCleanWidget::changeStageByRightButton()
{
    m_scanTime->setVisible(false);
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
        Q_EMIT stageChanged(PREPARING);
        break;
    default:
        break;
    }
}

// 需要顺序执行
void TrashCleanWidget::startScan()
{
    // 启动扫描
    // 获取所有项目的文件列表和大小数据

    // 待重构
    // 将扫描功能移动到后端服务

    Q_EMIT startWork();
}

void TrashCleanWidget::scanDone()
{
    // 扫描并获得全部文件数量
    m_doneItems--;
    if (m_doneItems == 0) {
        int totalAmount = 0;
        foreach (auto item, m_rootItems) {
            if (item->isSelected()) {
                totalAmount += item->totalFileAmount();
            }
        }
        m_scanProgressBar->setRange(0, totalAmount);

        // 开始延时显示扫描结果
        m_totalSize = 0;
        Q_EMIT showScanResult();
    }
}

QString TrashCleanWidget::getElidedScanTitle(const QString &fileName) const
{
    QString info = tr("Scanning: %1").arg(fileName);
    QFontMetrics tipFontmet(m_tipTitle->font());

    return tipFontmet.elidedText(info, Qt::TextElideMode::ElideMiddle, 480);
}

// 接收工作线程发送的信号，以同步标题上的扫描文件名
// 展示当前扫描进度
void TrashCleanWidget::setScanTitleName(QString fileName)
{
    if (!m_leftButton->isEnabled()) {
        m_leftButton->setDisabled(false);
    }
    m_tipTitle->setText(getElidedScanTitle(fileName));

    m_currentScanProgressValue += PROGRESS_NOTICE_INTERVAL;
    m_scanProgressBar->setValue(m_currentScanProgressValue);
}

// 根检查项通知过程结束
void TrashCleanWidget::showScanResultFinished()
{
    if (!m_leftButton->isEnabled()) {
        m_leftButton->setDisabled(false);
    }
    m_showItems--;
    if (m_showItems == 0) {
        stageChanged(SCAN_FINISHED);
    }
}

// 因界面变动要求重新统计总计文件大小
void TrashCleanWidget::recount()
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
        setHeadTextWithElidedText(m_scanResultStr);
        m_tipTitle->setText(tr("Scanned: %1 files").arg(m_totalScannedFiles));
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
void TrashCleanWidget::stopScan()
{
    onStageChanged(SCAN_FINISHED);
}

void TrashCleanWidget::setWidgetPrepare()
{
    // 回到配置界面，清空历史数据
    // 隐藏检查树
    // 隐藏右按键
    m_resultFrame->setVisible(false);
    m_configFrame->setVisible(true);
    m_headerLogo->setVisible(false);
    m_scanProgressBar->setVisible(false);
    m_leftButton->setText(tr("Scan Now"));
    m_rightButton->setVisible(false);
    m_configLogo->setVisible(true);
    m_isCleaned = false;
    m_bottomLine->setVisible(false);

    // 隐藏扫描计时
    m_scanTime->setVisible(false);

    // 由gsetting取上一次清理结果
    // 如果不为空，说明存在上一次清理
    // 将其写上界面
    if (m_dataInterface) {
        m_lastCleanedSize = m_dataInterface->GetValueFromeGSettings(CLEANER_LAST_TIME_CLEANED);
    }
    if (!m_lastCleanedSize.isEmpty()) {
        m_tipTitle->setText(tr("Last clean: %1").arg(m_lastCleanedSize));
    } else {
        m_tipTitle->setText(tr("Start cleaning now to refresh your computer"));
    }
    m_headerTitle->setText(tr("Clean out junk files and free up disk space"));
    Q_EMIT prepare();
}

void TrashCleanWidget::setWidgetScanStart()
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
    m_currentScanProgressValue = 0;
    // 有可能在scanfinish时将按键设为disable
    // 此处还原
    m_rightButton->setDisabled(false);

    int i = 0;
    int hiddens = 0;

    foreach (auto item, m_configItem) {
        if (item->getCheckBoxStatus()) {
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
        m_configFrame->setVisible(false);
        m_leftButton->setDisabled(true);
        m_headerLogo->setVisible(true);
        m_tipTitle->setText(tr("Initializing"));
        m_scanProgressBar->setVisible(true);
        m_resultFrame->setVisible(true);
        m_leftButton->setText(tr("Cancel", "button"));
        m_rightButton->setVisible(false);
        m_bottomLine->setVisible(true);

        // 扫描计时
        // 扫描计时初始化
        m_scanTime->setText(tr("Time elapsed %1:%2:%3").arg("00").arg("00").arg("00"));
        m_scanTime->setVisible(true);

        QTimer scanTimer;
        QDateTime currentTime = QDateTime::currentDateTime();
        connect(&scanTimer, &QTimer::timeout, this, [=] {
            QDateTime duration =
                QDateTime::fromSecsSinceEpoch(QDateTime::currentDateTime().toSecsSinceEpoch()
                                              - currentTime.toSecsSinceEpoch())
                    .toUTC();
            // 格式来自于需求，可能是翻译要求
            m_scanTime->setText(tr("Time elapsed %1:%2:%3")
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

        m_scanProgressBar->setValue(0);

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

void TrashCleanWidget::setWidgetScanFinish()
{
    // 取消扫描
    // 显示扫描完成页面
    // 左按键重命名为“返回”
    // 显示右按键，命名“立即清理”
    m_leftButton->setText(tr("Back", "button"));
    m_rightButton->setVisible(true);
    m_rightButton->setText(tr("Clean Up"));
    m_scanProgressBar->setVisible(false);
    // 要求统计所有项目并在界面上写对应信息
    recount();

    m_isFirstShow = false;

    // 由于在中止扫描的情况下有两次调用setWidgetFinished
    // 此处应注意不被上一次调用影响
    m_rightButton->setDisabled(!m_totalSize);

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
}

void TrashCleanWidget::setWidgetCleanStart()
{
    // 有可能在scanfinish时将按键设为disable
    // 此处还原
    m_rightButton->setDisabled(false);

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

void TrashCleanWidget::setWidgetCleanFinish()
{
    // 有可能在scanfinish时将按键设为disable
    // 此处还原
    m_rightButton->setDisabled(false);

    m_leftButton->setText(tr("Scan Again"));
    m_rightButton->setText(tr("Done", "button"));
    m_rightButton->setVisible(true);
    m_lastCleanedSize = TrashCleanItem::fileSizeToString(m_cleanedSize);
    m_headerTitle->setText(tr("%1 removed").arg(m_lastCleanedSize));
    m_tipTitle->setText(tr("Cleaned: %1 files").arg(m_cleanedAmount));
    if (m_dataInterface) {
        m_dataInterface->SetValueToGSettings(CLEANER_LAST_TIME_CLEANED, m_lastCleanedSize);
    }
}

void TrashCleanWidget::getScanPathsFromDataInterface()
{
    if (nullptr == m_dataInterface) {
        m_trashInfoList.clear();
        m_cacheInfoLIst.clear();
        m_logInfoList.clear();
        m_historyInfoList.clear();
        m_appJsonStr.clear();
        m_configItem.clear();
        return;
    }

    // GetCacheInfoList 可能存在耗时较长的问题
    // 关联的数据获取集中在此处修改
    QFuture<void> future = QtConcurrent::run([&]() {
        if (m_configItem[SYS_CHECK_INDEX]->getCheckBoxStatus()) {
            m_trashInfoList = m_dataInterface->GetTrashInfoList();
            m_cacheInfoLIst = m_dataInterface->GetCacheInfoList();
            m_logInfoList = m_dataInterface->GetLogInfoList();
        }

        if (m_configItem[APP_CHECK_INDEX]->getCheckBoxStatus()) {
            m_appJsonStr = m_dataInterface->GetAppTrashInfoList();
        }

        if (m_configItem[HISTORY_CHECK_INDEX]->getCheckBoxStatus()) {
            m_historyInfoList = m_dataInterface->GetHistoryInfoList();
        }

        if (m_configItem[BROWSER_CACHE_CHECK_INDEX]->getCheckBoxStatus()) {
            m_cookiesJsonStr = m_dataInterface->GetBrowserCookiesInfoList();
        }
    });
    while (!future.isFinished()) {
        QApplication::processEvents();
    }
}

// 随主题变化，改变图标
void TrashCleanWidget::setPixmapByTheme(DGuiApplicationHelper::ColorType themeType)
{
    // 判断当前主题类型
    if (themeType == DGuiApplicationHelper::ColorType::LightType) {
        QIcon icon = QIcon::fromTheme(TRASH_CLEAN_CONFIG_BIG_LOGO_LIGHT);
        m_configLogo->setPixmap(icon.pixmap(300, 300));
    } else {
        QIcon icon = QIcon::fromTheme(TRASH_CLEAN_CONFIG_BIG_LOGO_DARK);
        m_configLogo->setPixmap(icon.pixmap(300, 300));
    }
}

// 如果所有检查项未勾选，左按键置灰
void TrashCleanWidget::onConfigItemClicked()
{
    foreach (CleanerItem *ci, m_configItem) {
        if (ci->getCheckBoxStatus()) {
            m_leftButton->setDisabled(false);
            return;
        }
    }

    m_leftButton->setDisabled(true);
}

void TrashCleanWidget::onCleanFinished()
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
    m_headerTitle->setText(tr("%1 removed").arg(m_lastCleanedSize));
    m_tipTitle->setText(tr("Cleaned: %1 files").arg(m_cleanedAmount));
}

void TrashCleanWidget::DeleteSpecifiedFiles(QStringList paths)
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

void TrashCleanWidget::DeleteSpecifiedAppRecord(const QString &pkgName)
{
    if (m_dataInterface) {
        m_dataInterface->DeleteSpecifiedAppUninstallInfo(pkgName);
    }
}

// 代理函数
MyItemDelegateTree::MyItemDelegateTree(QObject *parent)
    : QItemDelegate(parent)
{
}

// 重绘操作
void MyItemDelegateTree::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(index);
}

void TrashCleanWidget::paintEvent(QPaintEvent *event)
{
    if (SCAN_FINISHED == m_stage) {
        setHeadTextWithElidedText(m_scanResultStr);
    }

    QWidget::paintEvent(event);
}

// 根据实际长度设置标题内容，右侧发生遮拦时以省略号代替内容
void TrashCleanWidget::setHeadTextWithElidedText(const QString &headStr)
{
    // 因扫描时间控件在可见时造成遮挡
    if (m_scanTime->isVisible()) {
        QFontMetrics scanTimeFontmet(m_scanTime->font());
        int scanTimeWidth = scanTimeFontmet.horizontalAdvance(m_scanTime->text(), -1);
        QFontMetrics headFontmet(m_headerTitle->font());
        QString elidedStr;
        elidedStr = headFontmet.elidedText(headStr,
                                           Qt::TextElideMode::ElideRight,
                                           MAX_HEAD_TITLE_WIDTH - scanTimeWidth - 10);
        m_headerTitle->setText(elidedStr);
    }
}
