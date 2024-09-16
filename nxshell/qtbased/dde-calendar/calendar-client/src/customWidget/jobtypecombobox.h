// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef JOBTYPECOMBOBOX_H
#define JOBTYPECOMBOBOX_H

#include "dschedule.h"
#include "scheduledatamanage.h"
#include "cpushbutton.h"
#include "accountmanager.h"

#include <DComboBox>
#include <DAlertControl>

DWIDGET_USE_NAMESPACE

class JobTypeComboBox : public DComboBox
{
    Q_OBJECT
public:
    explicit JobTypeComboBox(QWidget *parent = nullptr);
    ~JobTypeComboBox() override;
    void updateJobType(const AccountItem::Ptr&);
    QString getCurrentJobTypeNo();
    void setCurrentJobTypeNo(const QString &strJobTypeNo);

    void setAlert(bool isAlert);
    bool isAlert() const;
    void showAlertMessage(const QString &text, int duration = 3000);
    void showAlertMessage(const QString &text, QWidget *follower, int duration = 3000);
    void setAlertMessageAlignment(Qt::Alignment alignment);
    void hideAlertMessage();

    int getCurrentEditPosition() const;

signals:
    void signalAddTypeBtnClicked();
    void alertChanged(bool alert) const;
    void editingFinished();
public slots:

protected slots:
    void slotBtnAddItemClicked();
    void slotEditingFinished();
    void slotEditCursorPositionChanged(int oldPos, int newPos);

protected:
    void showPopup() override;
    bool eventFilter(QObject *, QEvent *) override;

private:
    void initUI();
    void addJobTypeItem(int idx, QString strColorHex, QString strJobType);
    void addCustomWidget(QFrame *);
    void setItemSelectable(bool status);

private:
    QWidget *m_customWidget {nullptr};
    CPushButton *m_addBtn {nullptr};
    DScheduleType::List m_lstJobType;
    int m_hoverSelectedIndex = -1; //鼠标悬停的选项下标
    int m_itemNumIndex = 0; //item数量
    DAlertControl *m_control {nullptr};
    QLineEdit *m_lineEdit {nullptr};
    int m_oldPos = 0;
    int m_newPos = 0;
};

#endif // JOBTYPECOMBOBOX_H
