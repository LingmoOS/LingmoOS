// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BUTTONEDITWIDGET_H
#define BUTTONEDITWIDGET_H

#include "widgets/settingsitem.h"

#include <DArrowRectangle>


QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QLabel;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DLineEdit;
class DIconButton;
DWIDGET_END_NAMESPACE

namespace dcc {
namespace network {

class ErrorTip : public DTK_WIDGET_NAMESPACE::DArrowRectangle {
    Q_OBJECT
public:
    explicit ErrorTip(QWidget *parent = nullptr);

    void setText(QString text);
    void clear();
    bool isEmpty() const;

public Q_SLOTS:
    void appearIfNotEmpty();

private:
    QLabel *m_label;
};

class ButtonEditWidget : public DCC_NAMESPACE:: SettingsItem
{
    Q_OBJECT

public:
    explicit ButtonEditWidget(QFrame *parent = nullptr);
    ~ButtonEditWidget();

    inline DTK_WIDGET_NAMESPACE::DLineEdit *dTextEdit() const { return m_edit; }
    inline DTK_WIDGET_NAMESPACE::DIconButton *addBtn() const { return m_addBtn; }
    inline DTK_WIDGET_NAMESPACE::DIconButton *reduceBtn() const { return m_reduceBtn; }

    void initConnect();
    void hideIconBtn();
    void showIconBtn();
    void hideAlertMessage();

Q_SIGNALS:
    void addNewDnsEdit() const;
    void deleteCurrentDnsEdit() const;

public Q_SLOTS:
    void setTitle(const QString &title);
    void setText(const QString &text);

protected:
    QHBoxLayout *m_mainLayout;

private:
    QLabel *m_title;
    dcc::network::ErrorTip *m_errTip;
    DTK_WIDGET_NAMESPACE::DLineEdit *m_edit;
    DTK_WIDGET_NAMESPACE::DIconButton *m_addBtn;
    DTK_WIDGET_NAMESPACE::DIconButton *m_reduceBtn;
};

}
}

#endif // BUTTONEDITWIDGET_H
