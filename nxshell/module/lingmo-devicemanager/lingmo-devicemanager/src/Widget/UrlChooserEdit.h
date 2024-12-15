// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URLCHOOSEREDIT_H
#define URLCHOOSEREDIT_H

//#include <DLineEdit>
#include <DWidget>

DWIDGET_BEGIN_NAMESPACE
class DSuggestButton;
class DLineEdit;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class UrlChooserEdit : public DWidget
{
    Q_OBJECT
public:
    explicit UrlChooserEdit(QWidget *parent = nullptr);

    /**
     * @brief text
     */
    QString text() const;

private:
    /**
     * @brief initUI
     */
    void initUI();

    /**
     * @brief initConnections
     */
    void initConnections();

    /**
     * @brief checkLocalFolder 检测是否本地路径
     */
    void checkLocalFolder(const QString &path);

signals:

    /**
     * @brief signalNotLocalFolder
     */
    void signalNotLocalFolder(bool isLocal);

public slots:

    /**
     * @brief slotChooseUrl
     */
    void slotChooseUrl();


private:
    Dtk::Widget::DLineEdit *mp_urlEdit;
    Dtk::Widget::DSuggestButton *mp_urlBtn;
    QString mp_folderPath;
    QString mp_elidParh;
};
#endif // URLCHOOSEREDIT_H
