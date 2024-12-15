// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERITEM_H
#define DRIVERITEM_H

#include "MacroDefinition.h"

#include <QObject>
#include <DWidget>
#include <DCheckBox>
#include <DLabel>
#include <DSpinner>
#include <DIconButton>
#include <DToolButton>

class BtnLabel;
class TipsLabel;
class TitleLabel;

DWIDGET_USE_NAMESPACE

/**
 * @brief The DriverCheckItem class
 * 单选框
 */
class DriverCheckItem : public DWidget
{
    Q_OBJECT
public:
    explicit DriverCheckItem(DWidget *parent = nullptr, bool header = false);
    /**
     * @brief setChecked 设置选中状态
     * @param checked
     * @param disconnect 是否断开信号曹函数，这里烤炉到，设置状态时不需要发送QCheckBox本身的信号
     */
    void setChecked(bool sigChecked, bool disconnect = false);

    /**
     * @brief checked 获取是否选中
     */
    bool checked();

    /**
     * @brief isEnabled
     * @return
     */
    bool isEnabled();

    /**
     * @brief setCbEnable 设置是否可选
     * @return
     */
    void setCbEnable(bool e);

signals:
    void sigChecked(bool check);
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void slotStateChanged(int state);
private:
    DCheckBox *mp_cb;
};


/**
 * @brief The DriverNameItem class
 * 设备信息item
 */
class DriverNameItem : public DWidget
{
    Q_OBJECT
public:
    explicit DriverNameItem(DWidget *parent = nullptr, DriverType dt = DR_Null);

    /**
     * @brief setName
     * @param name
     */
    void setName(const QString &name);

    /**
     * 设置和获取index
     * @brief setIndex
     * @param index
     */
    void setIndex(int index);
    int index();

private:
    DLabel *mp_Icon;
    DLabel *mp_Type;
    TipsLabel *mp_Name;
    int    m_Index;

    const int ICON_SIZE_WIDTH = 32;
    const int ICON_SIZE_HEIGHT = 32;
};


/**
 * @brief The DriverLabelItem class
 * 文本标签item
 */
class DriverLabelItem : public DWidget
{
    Q_OBJECT
public:
    explicit DriverLabelItem(DWidget *parent = nullptr,  const QString &txt = "");
private:
    TipsLabel *mp_Txt;
};



class DriverStatusItem : public DWidget
{
    Q_OBJECT
public:
    explicit DriverStatusItem(DWidget *parent = nullptr, Status s = ST_CAN_UPDATE);

    /**
     * @brief setStatus 修改状态，不同的状态显示不同的效果
     * @param st 状态值
     */
    void setStatus(Status st);

    /**
     * @brief getStatus
     * @return
     */
    Status getStatus();

    /**
     * @brief setErrorMsg
     * @param msg
     */
    void setErrorMsg(const QString &msg);

private:

    /**
     * @brief showSpinner 显示icon还是spinner
     * @param spin
     */
    void showSpinner(bool spin);
private:
    DSpinner *mp_Spinner;
    DLabel   *mp_Icon;
    BtnLabel *mp_Status;
    Status    m_Status;

    const int ICON_SIZE_WIDTH = 16;
    const int ICON_SIZE_HEIGHT = 16;
};


class DriverOperationItem : public DWidget
{
    Q_OBJECT
public:
    enum Mode {
        INSTALL = 0,
        UPDATE,
        BACKUP,
        RESTORE
    };

    explicit DriverOperationItem(DWidget *parent = nullptr, Mode m_mode = INSTALL);
    void setBtnEnable(bool enable);

    // 设置Btn  Icon
    void setBtnIcon();

    Mode mode() {return m_mode;}

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void clicked(bool checked = false);

private:
    DToolButton  *mp_Btn;
    Mode          m_mode;
};

#endif // DRIVERITEM_H
