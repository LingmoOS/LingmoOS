// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitylogdialog.h"
#include "widgets/defendertableheaderview.h"
#include "securitylogadaptorinterface.h"
#include "window/modules/common/common.h"

#include <DLabel>
#include <DFrame>
#include <DTipLabel>
#include <DComboBox>
#include <DIconButton>
#include <DHeaderView>
#include <DFontSizeManager>
#include <DBlurEffectWidget>

#include <QDate>
#include <QSqlQuery>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QMap>

#include "qmath.h"

// 每页的固定日志显示数量
#define PAGE_SHOW_SUM 10

SecurityLogDialog::SecurityLogDialog(DDialog *parent, SecurityLogAdaptorInterface *dataInterface)
    : DDialog(parent)
    , m_dataInterface(dataInterface)
    , m_headerDelegate(nullptr)
    , m_tableView(nullptr)
    , m_tableModel(nullptr)
    , m_clearLogBtn(nullptr)
    , m_lbLogCount(nullptr)
    , m_lbLogPage(nullptr)
    , m_nPageShowIndex(1)
{
    this->setAccessibleName("logDialog");

    // 初始化表头
    m_headerDelegate = new DefenderTableHeaderView(Qt::Horizontal, this);

    // 初始化
    initUi();
    initData();
}

SecurityLogDialog::~SecurityLogDialog()
{
    Q_EMIT onDialogClose();
}

// 初始化界面
void SecurityLogDialog::initUi()
{
    // 弹框样式
    this->setWindowModality(Qt::ApplicationModal);
    setCloseButtonVisible(true);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setModal(true);
    this->setFixedSize(QSize(750, 555));
    this->setWindowTitle(tr("Logs"));
    this->setAttribute(Qt::WA_UnderMouse);

    // 日志类型标签
    DLabel *lableTypeFilter = new DLabel(tr("Category: "), this);
    lableTypeFilter->setAccessibleName("topWidget_typeFilterLable");
    lableTypeFilter->setFixedHeight(36);
    DFontSizeManager::instance()->bind(lableTypeFilter, DFontSizeManager::T6);

    // 日志类型下拉框
    DComboBox *combTypeFilter = new DComboBox(this);
    combTypeFilter->setAccessibleName("topWidget_typeFilterCombox");
    combTypeFilter->setFixedSize(QSize(150, 36));
    combTypeFilter->addItem(tr("All"), SECURITY_LOG_TYPE_ALL);
    combTypeFilter->addItem(tr("Overall checking"), SECURITY_LOG_TYPE_HOME);
    combTypeFilter->addItem(tr("Virus scan"), SECURITY_LOG_TYPE_ANTIAV);
    combTypeFilter->addItem(tr("Protection"), SECURITY_LOG_TYPE_PROTECTION);
    combTypeFilter->addItem(tr("Cleanup"), SECURITY_LOG_TYPE_CLEANER);
    combTypeFilter->addItem(tr("Tools"), SECURITY_LOG_TYPE_TOOL);
    combTypeFilter->addItem(tr("Basic settings"), SECURITY_LOG_TYPE_BASIC);
    combTypeFilter->addItem(tr("Security settings"), SECURITY_LOG_TYPE_SAFY);

    // 初始化 过滤类型
    m_securityLogType = combTypeFilter->currentIndex();
    // 日志类型信号槽
    connect(combTypeFilter, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SecurityLogDialog::doLogTypeFilterChanged);

    // 日期标签
    DLabel *labledateFilter = new DLabel(QString("     ") + tr("Period: "), this);
    labledateFilter->setAccessibleName("topWidget_dateFilterLable");
    labledateFilter->setFixedHeight(36);
    DFontSizeManager::instance()->bind(labledateFilter, DFontSizeManager::T6);

    // 日期下拉框
    DComboBox *combDateFilter = new DComboBox(this);
    combDateFilter->setAccessibleName("topWidget_dateFilterCombox");
    combDateFilter->setFixedSize(QSize(150, 36));
    combDateFilter->addItem(tr("Today"), LAST_DATE_NOW);
    combDateFilter->addItem(tr("in 3 days"), LAST_DATE_THREE);
    combDateFilter->addItem(tr("in 7 days"), LAST_DATE_SEVEN);
    combDateFilter->addItem(tr("in 30 days"), LAST_DATE_MONTH);

    // 初始化 过滤日期
    m_securityLogDate = combDateFilter->currentIndex();
    // 日期信号槽
    connect(combDateFilter, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SecurityLogDialog::doLogDateFilterChanged);

    // 加入弹簧
    QSpacerItem *horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // 加入清理全部日志说明
    m_clearLogBtn = new DCommandLinkButton(tr("Clear below logs"), this);
    m_clearLogBtn->setAccessibleName("topWidget_clearButton");
    DFontSizeManager::instance()->bind(m_clearLogBtn, DFontSizeManager::T8);
    m_clearLogBtn->installEventFilter(this);

    // 连接清除记录按钮信号
    connect(m_clearLogBtn, &DCommandLinkButton::clicked, this, &SecurityLogDialog::confirmClearLog);

    // 顶部过滤栏
    DWidget *widget = new DWidget(this);
    widget->setAccessibleName("logDialog_topWidget");
    QHBoxLayout *layoutFilter = new QHBoxLayout(widget);
    layoutFilter->setContentsMargins(0, 0, 0, 0);
    layoutFilter->setSpacing(10);

    // 添加顶部组件到横向布局
    layoutFilter->addWidget(lableTypeFilter);
    layoutFilter->addWidget(combTypeFilter);
    layoutFilter->addWidget(labledateFilter);
    layoutFilter->addWidget(combDateFilter);
    layoutFilter->addItem(horizontalSpacer);
    layoutFilter->addWidget(m_clearLogBtn);
    layoutFilter->setAlignment(m_clearLogBtn, Qt::AlignBottom);

    widget->setLayout(layoutFilter);
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // 表格 - 模型 连接
    m_tableView = new DTableView(this);
    m_tableView->setAccessibleName("centerFrame_tableView");

    // 设置表格样式 (表头 网格线 排序)
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setFrameShape(QTableView::NoFrame);
    m_tableView->setFocusPolicy(Qt::NoFocus);
    m_tableView->setWordWrap(false);

    // 自动调整最后一列的宽度使它和表格的右边界对齐
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setHidden(true);
    m_tableView->setShowGrid(false);
    m_tableView->setSortingEnabled(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(36);

    // 连接代理
    m_delegate = new SecurityLogTableDelegate(this);
    m_tableView->setItemDelegate(m_delegate);

    // 使用代理更换表头
    m_tableView->setHorizontalHeader(m_headerDelegate);

    // 表头属性设置
    m_headerDelegate->setSectionsMovable(true);
    m_headerDelegate->setSectionsClickable(true);
    m_headerDelegate->setSectionResizeMode(DHeaderView::Interactive);
    m_headerDelegate->setStretchLastSection(true);
    m_headerDelegate->setSortIndicatorShown(false);
    m_headerDelegate->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerDelegate->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerDelegate->setFocusPolicy(Qt::TabFocus);

    // 总记录
    m_lbLogCount = new DLabel(tr("Total records: %1").arg(0), this);
    m_lbLogCount->setAccessibleName("bottomWidget_countLable");
    m_lbLogCount->setFixedHeight(36);
    DFontSizeManager::instance()->bind(m_lbLogCount, DFontSizeManager::T8);
    m_lbLogCount->setAlignment(Qt::AlignTop);

    // 页数
    m_lbLogPage = new DLabel("0/0", this);
    m_lbLogPage->setAccessibleName("bottomWidget_pageLable");
    m_lbLogPage->setFixedHeight(36);
    DFontSizeManager::instance()->bind(m_lbLogPage, DFontSizeManager::T7);

    // 上一页
    DIconButton *leftBtn = new DIconButton(this); //上一步按钮
    leftBtn->setAccessibleName("bottomWidget_leftButton");
    leftBtn->setFixedSize(QSize(36, 36));
    leftBtn->setIcon(QStyle::SP_ArrowLeft);
    connect(leftBtn, &DIconButton::clicked, this, &SecurityLogDialog::doBeforePage);

    // 下一页
    DIconButton *rightBtn = new DIconButton(this); //上一步按钮
    rightBtn->setAccessibleName("bottomWidget_rightButton");
    rightBtn->setFixedSize(QSize(36, 36));
    rightBtn->setIcon(QStyle::SP_ArrowRight);
    connect(rightBtn, &DIconButton::clicked, this, &SecurityLogDialog::doAfterPage);

    // 加入弹簧
    QSpacerItem *horizontalPageLeftSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *horizontalPageRightSpacer = new QSpacerItem(299, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // 底部控件
    DWidget *widgetPage = new DWidget(this);
    widgetPage->setAccessibleName("logDialog_bottomWidget");
    QHBoxLayout *layoutPage = new QHBoxLayout(widgetPage);
    layoutPage->setContentsMargins(0, 0, 0, 0);
    layoutPage->setSpacing(10);

    layoutPage->addWidget(m_lbLogCount);
    layoutPage->addItem(horizontalPageLeftSpacer);
    layoutPage->addWidget(m_lbLogPage);
    layoutPage->addWidget(leftBtn);
    layoutPage->addWidget(rightBtn);
    layoutPage->addItem(horizontalPageRightSpacer);

    widgetPage->setLayout(layoutPage);
    widgetPage->setContentsMargins(0, 0, 0, 0);
    widgetPage->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    DFrame *frame = new DFrame(this);
    frame->setAccessibleName("logDialog_centerFrame");
    frame->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_tableView);
    frame->setLayout(layout);

    // 最后将所有控件放入布局
    addContent(widget);
    addContent(frame);
    addContent(widgetPage);
    setSpacing(10);
    setContentLayoutContentsMargins(QMargins(10, 0, 10, 0));
}

// 初始化数据
void SecurityLogDialog::initData()
{
    // 初始化QSqlTableModel
    m_tableModel = new QSqlQueryModel(this);
    m_tableView->setModel(m_tableModel);

    // 刷新安全日志
    refreshSecurityLog();
}

// 刷新安全日志
void SecurityLogDialog::refreshSecurityLog()
{
    if (!m_dataInterface) {
        qDebug() << "SecurityLogDialog : lost dataInterface ";
        return;
    }
    // 打开数据
    if (!m_dataInterface->GetDatabase().open()) {
        qDebug() << "security.db open failed! ";
    }

    // 根据过滤条件查询数据 日期/类型/每一页的显示行数
    m_tableModel->setQuery(getSecurityLogCmd(), m_dataInterface->GetDatabase());
    setTableBackGround();

    // 初始化表头 - 用来替换setTable本身的表头
    m_tableModel->setHeaderData(0, Qt::Horizontal, tr("Date and Time"));
    m_tableModel->setHeaderData(1, Qt::Horizontal, tr("Category"));
    m_tableModel->setHeaderData(2, Qt::Horizontal, tr("Content"));

    // 设置前两列列的宽度
    m_tableView->setColumnWidth(0, 200);
    m_tableView->setColumnWidth(1, 150);

    // 隐藏第一列和第三列
    m_tableView->setColumnHidden(3, true);

    m_dataInterface->GetDatabase().close();
}

// 获取日志刷新需要用到的sql语句
QString SecurityLogDialog::getSecurityLogCmd()
{
    if (!m_dataInterface) {
        qDebug() << "SecurityLogDialog : lost dataInterface ";
        return "";
    }

    // 当前的日期
    QDate date = QDate::currentDate();
    QString sCurrentDate = date.toString("yyyy-MM-dd");

    QString sCmd;
    // 过滤条件为 日志类型为全部/日期为当天
    if (SECURITY_LOG_TYPE_ALL == m_securityLogType) {
        sCmd = QString("select * from SecurityLog WHERE datetimes between '%1 23:59:59' and '%2 23:59:59' ORDER BY datetimes DESC").arg(getBeginDate(date)).arg(sCurrentDate);
    } else {
        sCmd = QString("select * from SecurityLog WHERE datetimes between '%1 23:59:59' and '%2 23:59:59' and type=%3 ORDER BY datetimes DESC").arg(getBeginDate(date)).arg(sCurrentDate).arg(m_securityLogType);
    }

    // 查询过滤条件下 的日志总数
    QSqlQueryModel *securityLogCountModel = new QSqlQueryModel(this);
    securityLogCountModel->setQuery(sCmd, m_dataInterface->GetDatabase());
    // 获取全部数据(QSqlQueryModel每次最多获取256行数据)
    while (securityLogCountModel->canFetchMore()) {
        securityLogCountModel->fetchMore();
    }
    m_nLogCount = securityLogCountModel->rowCount();

    // 标签页信息显示
    showLableInfo();

    return QString(sCmd + " limit %1 OFFSET %2").arg(PAGE_SHOW_SUM).arg(PAGE_SHOW_SUM * (m_nPageShowIndex - 1));
}

// 得到日期过滤 的开始日期
QString SecurityLogDialog::getBeginDate(QDate date)
{
    // 根据日期过滤条件得到 过滤开始时间
    QString sLastDate;
    if (LAST_DATE_NOW == m_securityLogDate) {
        sLastDate = date.addDays(-1).toString("yyyy-MM-dd");
    } else if (LAST_DATE_THREE == m_securityLogDate) {
        sLastDate = date.addDays(-3).toString("yyyy-MM-dd");
    } else if (LAST_DATE_SEVEN == m_securityLogDate) {
        sLastDate = date.addDays(-7).toString("yyyy-MM-dd");
    } else {
        sLastDate = date.addDays(-30).toString("yyyy-MM-dd");
    }
    return sLastDate;
}

// 类型下拉框选中行改变槽
void SecurityLogDialog::doLogTypeFilterChanged(int nType)
{
    m_nPageShowIndex = 1;
    m_securityLogType = nType;
    refreshSecurityLog();
}

// 日期下拉框选中行改变槽
void SecurityLogDialog::doLogDateFilterChanged(int nLastMonth)
{
    m_nPageShowIndex = 1;
    m_securityLogDate = nLastMonth;
    refreshSecurityLog();
}

// 上一页
void SecurityLogDialog::doBeforePage()
{
    if (m_nPageShowIndex > 1) {
        m_nPageShowIndex -= 1;
        refreshSecurityLog();
    }
}

// 下一页
void SecurityLogDialog::doAfterPage()
{
    if (m_nPageShowIndex < ceil(double(m_nLogCount) / PAGE_SHOW_SUM)) {
        m_nPageShowIndex += 1;
        refreshSecurityLog();
    }
}

// 标签显示
void SecurityLogDialog::showLableInfo()
{
    // 显示标签页
    if (m_nLogCount <= 0) {
        m_lbLogPage->setText(QString("0/0"));
    } else {
        m_lbLogPage->setText(QString("%1/%2").arg(m_nPageShowIndex).arg(ceil(double(m_nLogCount) / PAGE_SHOW_SUM)));
    }

    // 显示总记录
    m_lbLogCount->setText(tr("Total records: %1").arg(m_nLogCount));
}

// 删除全部日志槽
void SecurityLogDialog::confirmClearLog()
{
    if (m_dataInterface) {
        if (m_dataInterface->DeleteSecurityLog(m_securityLogDate, m_securityLogType)) {
            refreshSecurityLog();
        }
    }
}

// 每次刷新表格数据时重新添加表格背景交替色
void SecurityLogDialog::setTableBackGround()
{
    m_rowCount = m_tableModel->rowCount();
    // 如果已经存在背景控件，则移除
    int nBackGroundList = m_backgroundList.count();
    if (nBackGroundList > 0) {
        for (int i = 0; i < nBackGroundList; ++i) {
            m_backgroundList.last()->deleteLater();
            m_backgroundList.removeLast();
        }
    }

    // 为表格每行添加背景
    int initY = m_tableView->viewport()->y();
    for (int i = 0; i < m_rowCount; i++) {
        if (1 == i % 2) {
            DFrame *frame = new DFrame(m_tableView);
            frame->setFixedHeight(36);
            frame->setLineWidth(0);
            frame->show();
            frame->setBackgroundRole(DPalette::ItemBackground);

            int rowHeight = m_tableView->rowHeight(i);
            frame->setFixedWidth(680);
            frame->setFixedHeight(rowHeight);
            frame->move(-m_tableView->horizontalScrollBar()->value(), initY + i * rowHeight - m_tableView->verticalScrollBar()->value());
            m_backgroundList.append(frame);
        }
    }
}

// 事件过滤器
bool SecurityLogDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Type::Enter) {
        setCursor(Qt::PointingHandCursor);
    } else if (event->type() == QEvent::Type::Leave) {
        setCursor(Qt::ArrowCursor);
    }

    return QWidget::eventFilter(obj, event);
}

// 代理函数
SecurityLogTableDelegate::SecurityLogTableDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_HOME, tr("Overall checking"));
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_ANTIAV, tr("Virus scan"));
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_PROTECTION, tr("Protection"));
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_CLEANER, tr("Cleanup"));
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_TOOL, tr("Tools"));
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_BASIC, tr("Basic settings"));
    m_logTypeNameMap.insert(SECURITY_LOG_TYPE_SAFY, tr("Security settings"));
}

QString SecurityLogTableDelegate::getLogTypeName(int index) const
{
    if (SECURITY_LOG_TYPE_HOME <= index && index <= SECURITY_LOG_TYPE_SAFY) {
        return m_logTypeNameMap.value(index);
    } else {
        return "type error";
    }
}

// 重绘操作
void SecurityLogTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 获取数据 和 坐标
    QString sDataInfo = index.model()->data(index, Qt::DisplayRole).toString();
    QRect rect = option.rect;
    int pix_x = rect.x() + 10;
    int pix_y = rect.y() + 4;
    int height = rect.height() / 2 + 2;
    QString sElidedDataInfo = painter->fontMetrics().elidedText(sDataInfo, Qt::ElideRight, rect.width() - 20, Qt::TextShowMnemonic);

    if (0 == index.column()) {
        // 设置时间
        painter->drawText(pix_x + 8, pix_y + height, sElidedDataInfo);
    } else if (index.column() == 1) {
        // 设置类型
        QString logTypeName = getLogTypeName(sDataInfo.toInt());
        QString sElidedTypeName = painter->fontMetrics().elidedText(logTypeName, Qt::ElideRight, rect.width() - 20, Qt::TextShowMnemonic);
        // 设置类型
        painter->drawText(pix_x, pix_y + height, sElidedTypeName);
    } else {
        // 设置内容
        painter->drawText(pix_x, pix_y + height, sElidedDataInfo);
    }
}
