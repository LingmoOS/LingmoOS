// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMELINEDATEWIDGET_H
#define TIMELINEDATEWIDGET_H

#include <DCommandLinkButton>
#include <DLabel>
#include <DCheckBox>
#include <DWidget>
#include <DGuiApplicationHelper>

#include <QStandardItem>
#include <QWheelEvent>
DWIDGET_USE_NAMESPACE

const QColor lightTextColor = QColor("#484848");
const QColor darkTextColor = QColor("#bababa");

/****时间线界面，图片数量、时间、选择按钮统一类****/

class TimeLineDateWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TimeLineDateWidget(QStandardItem *item, const QString &time, const QString &num);

public slots:
    void onCheckBoxCliked();
    void onChangeChooseBtnVisible(bool visible = false);             //1050预留，默认隐藏按钮
    void onTimeLinePicSelectAll(bool selectall = true);              //响应本时间内所有照片的全选状态，改变btn
    QString onGetBtnStatus();                                        //获取按钮状态
    void onThemeChanged(DGuiApplicationHelper::ColorType themeType); //界面主题切换
    void onShowCheckBox(bool bShow);                                 //显隐复选框
signals:
    void sigIsSelectCurrentDatePic(bool isSelectAll, QStandardItem *item);

private:
    DCommandLinkButton *m_chooseBtn;
    DLabel *m_pDate;
    DLabel *m_pNum;
    DCheckBox *m_pNumCheckBox;
    QStandardItem *m_currentItem;
    DCommandLinkButton *m_pbtn;//占位btn
};



/****已导入时间线界面，图片数量、时间、选择按钮统一类****/

class importTimeLineDateWidget : public DWidget
{
    Q_OBJECT
public:
    explicit importTimeLineDateWidget(QStandardItem *item, const QString &time, const QString &num);

public slots:
    void onCheckBoxCliked();
    void onChangeChooseBtnVisible(bool visible = false);             //1050预留，默认隐藏按钮
    void onTimeLinePicSelectAll(bool selectall = true);              //响应本时间内所有照片的全选状态，改变btn
    QString onGetBtnStatus();                                        //获取按钮状态
    void onThemeChanged(DGuiApplicationHelper::ColorType themeType); //界面主题切换
    void onShowCheckBox(bool bShow);                                 //显隐复选框

signals:
    void sigIsSelectCurrentDatePic(bool isSelectAll, QStandardItem *item);

private:
    DCommandLinkButton *m_chooseBtn;
    DLabel *m_pDateandNum;
    DCheckBox *m_pDateandNumCheckBox;
    QStandardItem *m_currentItem;
    DCommandLinkButton *m_pbtn;//占位btn

};

#endif // TIMELINEDATEWIDGET_H
