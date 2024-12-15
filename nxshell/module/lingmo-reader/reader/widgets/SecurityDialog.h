// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYDIALOG_H
#define SECURITYDIALOG_H

#include <DDialog>
#include <DLabel>
DWIDGET_USE_NAMESPACE

struct NewStr {
    QStringList strList;
    QString resultStr;
    int fontHeifht = 0;
};

NewStr autoCutText(const QString &text, DLabel *pDesLbl);

class SecurityDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    explicit SecurityDialog(const QString &urlstr, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    /**
     * @brief autoFeed 自动换行
     */
    void autoFeed();
    /**
     * @brief setLabelColor 设置label颜色
     * @param label
     * @param alphaF 透明度
     */
    void setLabelColor(DLabel *label, qreal alphaF);

private:
    DLabel *NameLabel = nullptr;
    DLabel *ContentLabel = nullptr;
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iLabelOld1Height = 0;
    int m_iDialogOldHeight = 0;
};

#endif // SECURITYDIALOG_H
