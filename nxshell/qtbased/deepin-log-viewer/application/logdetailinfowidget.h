// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGDETAILINFOWIDGET_H
#define LOGDETAILINFOWIDGET_H
#include "logiconbutton.h"
#include "logdetailedit.h"
#include "structdef.h"

#include <DHorizontalLine>
#include <DLabel>
#include <DWidget>
#include <DTextBrowser>

#include <QModelIndex>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE
class QStandardItemModel;
class QVBoxLayout;
/**
 * @brief The logDetailInfoWidget class 详情页控件
 */
class logDetailInfoWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit logDetailInfoWidget(QWidget *parent = nullptr);

    void cleanText();

    void hideLine(bool isHidden);

private:
    void initUI();
    void setTextCustomSize(QWidget *w);

    void fillDetailInfo(QString deamonName, QString usrName, QString pid, QString dateTime,
                        QModelIndex level, QString msg, QString status = "", QString action = "",
                        QString uname = "", QString event = "");  // modified by Airy
    //其他日志或者自定义日志数据显示
    void fillOOCDetailInfo(const QString &data, const int error = 0);

protected:
    void paintEvent(QPaintEvent *event) override;
public slots:
    void slot_DetailInfo(const QModelIndex &index, QStandardItemModel *pModel, const QString &data, const int error);

private:
    //m_daemonName:进程名显示控件 m_dateTime:时间显示控件 m_userName：用户名显示控件  m_pid：进程号显示控件 m_action：动作显示控件  m_status：状态显示控件 m_name:开关机日志用户名显示控件 m_event: 开关机日志时间类型显示
    Dtk::Widget::DLabel *m_daemonName, *m_dateTime, *m_userName, *m_pid, *m_action, *m_status,
        *m_name, *m_event;  // modified by Airy
    /**
     * @brief m_level 日志等级显示控件
     */
    LogIconButton *m_level;
    Dtk::Widget::DLabel *m_userLabel, *m_pidLabel, *m_statusLabel, *m_actionLabel, *m_nameLabel,
        *m_eventLabel, *m_errorLabel;  // modified by Airy
    /**
     * @brief m_textBrowser 日志信息显示控件
     */
    logDetailEdit *m_textBrowser;
    /**
     * @brief m_hline 中间的分割线
     */
    Dtk::Widget::DHorizontalLine *m_hline;
    /**
     * @brief m_pModel 当前主表的model
     */
    QStandardItemModel *m_pModel;
    /**
     * @brief m_bottomLayer 底部框
     */
    QVBoxLayout *m_bottomLayer;
};

#endif  // LOGDETAILINFOWIDGET_H
