// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYLOGDIALOG_H
#define SECURITYLOGDIALOG_H

#include "dtkwidget_global.h"
#include "window/namespace.h"

#include <DDialog>
#include <DTableView>
#include <DCommandLinkButton>

#include <QItemDelegate>
#include <QSqlQueryModel>
#include <QMap>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DFrame;
class DTipLabel;
class DFontSizeManager;
DWIDGET_END_NAMESPACE

DEF_NAMESPACE_BEGIN
DEF_NETPROTECTION_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE

//后端DBUS接口与数据库持有
class SecurityLogAdaptorInterface;

/************************* SecurityLogDialog 表格代理 ***********************/
class SecurityLogTableDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SecurityLogTableDelegate(QObject *parent = nullptr);
    virtual ~SecurityLogTableDelegate() {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QString getLogTypeName(int) const;

private:
    QMap<int, QString> m_logTypeNameMap; // 类型与名称映射
};

class DefenderTableHeaderView;
class SecurityLogDialog : public DDialog
{
    Q_OBJECT
public:
    explicit SecurityLogDialog(DDialog *parent = nullptr, SecurityLogAdaptorInterface *dataInterface = nullptr);
    ~SecurityLogDialog() override;

private:
    // 初始化界面
    void initUi();

    // 初始化数据
    void initData();

    // 刷新安全日志
    void refreshSecurityLog();
    // 获取日志刷新需要用到的sql语句
    QString getSecurityLogCmd();
    // 得到日期过滤 的开始日期
    QString getBeginDate(QDate date);

    // 标签显示
    void showLableInfo();

    void setTableBackGround();

public:
    inline int GetRowCount() { return m_rowCount; }
    inline int GetLogCount() { return m_nLogCount; }
    inline int GetPageShowIndex() { return m_nPageShowIndex; }

public Q_SLOTS:
    void confirmClearLog();
    void doLogTypeFilterChanged(int nType);
    void doLogDateFilterChanged(int nLastMonth);

    void doBeforePage();
    void doAfterPage();

Q_SIGNALS:
    void onDialogClose();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    SecurityLogAdaptorInterface *m_dataInterface;

    DefenderTableHeaderView *m_headerDelegate; // 表头代理类
    DTableView *m_tableView; // 表格
    QSqlQueryModel *m_tableModel; // 模型
    SecurityLogTableDelegate *m_delegate; // 表格代理

    DCommandLinkButton *m_clearLogBtn;
    DLabel *m_lbLogCount; // 日志总数
    DLabel *m_lbLogPage; // 日志页数
    QList<DFrame *> m_backgroundList; // 背景列表
    int m_rowCount; // 当前表格总数量
    int m_nLogCount; // 过滤条件下的查询总数量
    int m_securityLogType; // 日志过滤类型
    int m_securityLogDate; // 日志过滤日期
    int m_nPageShowIndex; // 当前页数
};
DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END

#endif // SECURITYLOGDIALOG_H
